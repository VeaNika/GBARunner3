#include "common.h"
#include <libtwl/mem/memSwap.h>
#include "RomGpio.h"
#include "RomGpioRtc.h"

#define ROM_GPIO_PIN_SCK            0
#define ROM_GPIO_PIN_SIO            1
#define ROM_GPIO_PIN_CS             2

#define RIO_RTC_COMMAND_RESET       0
#define RIO_RTC_COMMAND_STATUS      1
#define RIO_RTC_COMMAND_DATE_TIME   2
#define RIO_RTC_COMMAND_TIME        3
#define RIO_RTC_COMMAND_ALARM1      4
#define RIO_RTC_COMMAND_ALARM2      5
#define RIO_RTC_COMMAND_TEST_START  6
#define RIO_RTC_COMMAND_TEST_END    7

#define RIO_RTC_STATUS_INTFE        0x02
#define RIO_RTC_STATUS_INTME        0x08
#define RIO_RTC_STATUS_INTAE        0x20
#define RIO_RTC_STATUS_24H          0x40
#define RIO_RTC_STATUS_POWER        0x80

#define RIO_RTC_STATUS_WRITE_MASK   0b01101010

void RomGpioRtc::Update(RomGpio& romGpio)
{
    if (!romGpio.GetPinState(ROM_GPIO_PIN_CS))
    {
        _state = RtcTransferState::CommandWaitFallingEdge;
        _shiftRegister = 0;
        _bitCount = 0;
    }
    else
    {
        switch (_state)
        {
            case RtcTransferState::CommandWaitFallingEdge:
            {
                if (!romGpio.GetPinState(ROM_GPIO_PIN_SCK))
                {
                    _state = RtcTransferState::CommandWaitRisingEdge;
                }
                break;
            }
            case RtcTransferState::CommandWaitRisingEdge:
            {
                CommandWaitRisingEdge(romGpio);
                break;
            }
            case RtcTransferState::InDataWaitFallingEdge:
            {
                if (!romGpio.GetPinState(ROM_GPIO_PIN_SCK))
                {
                    _state = RtcTransferState::InDataWaitRisingEdge;
                }
                break;
            }
            case RtcTransferState::InDataWaitRisingEdge:
            {
                HandleInDataWaitRisingEdge(romGpio);
                break;
            }
            case RtcTransferState::OutDataWaitFallingEdge:
            {
                HandleOutDataWaitFallingEdge(romGpio);
                break;
            }
            case RtcTransferState::OutDataWaitRisingEdge:
            {
                if (romGpio.GetPinState(ROM_GPIO_PIN_SCK))
                {
                    _state = RtcTransferState::OutDataWaitFallingEdge;
                }
                break;
            }
            case RtcTransferState::Done:
            {
                break;
            }
        }
    }
}

void RomGpioRtc::CommandWaitRisingEdge(RomGpio& romGpio)
{
    if (!romGpio.GetPinState(ROM_GPIO_PIN_SCK))
    {
        return;
    }

    _shiftRegister = (_shiftRegister << 1) | romGpio.GetPinState(ROM_GPIO_PIN_SIO);
    if (++_bitCount == 8)
    {
        if ((_shiftRegister >> 4) != 0b0110)
        {
            _state = RtcTransferState::Done;
        }
        else
        {
            _command = (_shiftRegister >> 1) & 7;
            if (_command == RIO_RTC_COMMAND_RESET)
            {
                RtcReset();
                _state = RtcTransferState::Done;
            }
            else if (_command >= RIO_RTC_COMMAND_TEST_START)
            {
                _state = RtcTransferState::Done;
            }
            else
            {
                _state = _shiftRegister & 1
                    ? RtcTransferState::OutDataWaitFallingEdge
                    : RtcTransferState::InDataWaitFallingEdge;
                _shiftRegister = 0;
                _bitCount = 0;
                _byteIndex = 0;
            }
        }
    }
    else
    {
        _state = RtcTransferState::CommandWaitFallingEdge;
    }
}

void RomGpioRtc::HandleInDataWaitRisingEdge(RomGpio& romGpio)
{
    if (!romGpio.GetPinState(ROM_GPIO_PIN_SCK))
    {
        return;
    }

    _shiftRegister |= romGpio.GetPinState(ROM_GPIO_PIN_SIO) << _bitCount;
    _state = RtcTransferState::InDataWaitFallingEdge;
    if (++_bitCount == 8)
    {
        _bitCount = 0;
        switch (_command)
        {
            case RIO_RTC_COMMAND_STATUS:
            {
                _statusRegister = (_statusRegister & RIO_RTC_STATUS_POWER) | (_shiftRegister & RIO_RTC_STATUS_WRITE_MASK);
                break;
            }
            case RIO_RTC_COMMAND_DATE_TIME:
            {
                switch (_byteIndex)
                {
                    case 0:
                        SetYear(_shiftRegister);
                        break;
                    case 1:
                        SetMonth(_shiftRegister);
                        break;
                    case 2:
                        SetDayOfMonth(_shiftRegister);
                        break;
                    case 3:
                        SetDayOfWeek(_shiftRegister);
                        break;
                    case 4:
                        SetHour(_shiftRegister);
                        break;
                    case 5:
                        SetMinute(_shiftRegister);
                        break;
                    case 6:
                        SetSecond(_shiftRegister);
                        break;
                }
                if (_byteIndex++ == 7)
                {
                    _state = RtcTransferState::Done;
                }
                break;
            }
            case RIO_RTC_COMMAND_TIME:
            {
                switch (_byteIndex)
                {
                    case 0:
                        SetHour(_shiftRegister);
                        break;
                    case 1:
                        SetMinute(_shiftRegister);
                        break;
                    case 2:
                        SetSecond(_shiftRegister);
                        break;
                }
                if (++_byteIndex == 3)
                {
                    _state = RtcTransferState::Done;
                }
                break;
            }
            case RIO_RTC_COMMAND_ALARM1:
            {
                mem_swapByte(_shiftRegister, &((u8*)&_intRegister)[_byteIndex]);
                if (++_byteIndex == 2)
                {
                    _state = RtcTransferState::Done;
                }
                break;
            }
            case RIO_RTC_COMMAND_ALARM2:
            {
                mem_swapByte(_shiftRegister, &((u8*)&_intRegister)[1]);
                _state = RtcTransferState::Done;
                break;
            }
        }
        _shiftRegister = 0;
    }
}

void RomGpioRtc::HandleOutDataWaitFallingEdge(RomGpio& romGpio)
{
    if (romGpio.GetPinState(ROM_GPIO_PIN_SCK))
    {
        return;
    }

    _state = RtcTransferState::OutDataWaitRisingEdge;
    u32 outputBit = 0;
    switch (_command)
    {
        case RIO_RTC_COMMAND_STATUS:
        {
            outputBit = (_statusRegister >> _bitCount) & 1;
            if (++_bitCount == 8)
            {
                _state = RtcTransferState::Done;
            }
            break;
        }
        case RIO_RTC_COMMAND_DATE_TIME:
        {
            outputBit = (((u8*)&_dateTime)[_bitCount >> 3] >> (_bitCount & 7)) & 1;
            if (++_bitCount == 7 * 8)
            {
                _state = RtcTransferState::Done;
            }
            break;
        }
        case RIO_RTC_COMMAND_TIME:
        {
            outputBit = (((u8*)&_dateTime.time)[_bitCount >> 3] >> (_bitCount & 7)) & 1;
            if (++_bitCount == 3 * 8)
            {
                _state = RtcTransferState::Done;
            }
            break;
        }
        case RIO_RTC_COMMAND_ALARM1:
        {
            outputBit = (_intRegister >> _bitCount) & 1;
            if (++_bitCount == 16)
            {
                _state = RtcTransferState::Done;
            }
            break;
        }
        case RIO_RTC_COMMAND_ALARM2:
        {
            outputBit = (_intRegister >> (_bitCount + 8)) & 1;
            if (++_bitCount == 8)
            {
                _state = RtcTransferState::Done;
            }
            break;
        }
    }

    romGpio.SetPinState(ROM_GPIO_PIN_SIO, outputBit);
}

void RomGpioRtc::RtcReset()
{
    mem_swapByte(0, &_dateTime.date.year);
    mem_swapByte(1, &_dateTime.date.month);
    mem_swapByte(1, &_dateTime.date.monthDay);
    mem_swapByte(0, &_dateTime.date.weekDay);
    mem_swapByte(0, &_dateTime.time.hour);
    mem_swapByte(0, &_dateTime.time.minute);
    mem_swapByte(0, &_dateTime.time.second);
    _statusRegister = 0;
    _intRegister = 0;
}

void RomGpioRtc::SetYear(u8 value)
{
    if ((value & 0xF) > 9 ||
        ((value >> 4) & 0xF) > 9)
    {
        value = 0;
    }
    mem_swapByte(value, &_dateTime.date.year);
}

void RomGpioRtc::SetMonth(u8 value)
{
    value &= 0x1F;
    if (value == 0 ||
        (value >= 0x13 && value <= 0x19) ||
        (value & 0xF) > 9)
    {
        value = 1;
    }
    mem_swapByte(value, &_dateTime.date.month);
}

void RomGpioRtc::SetDayOfMonth(u8 value)
{
    value &= 0x3F;
    if (value == 0 ||
        (value >= 0x32 && value <= 0x39) ||
        (value & 0xF) > 9)
    {
        value = 1;
    }

    u32 daysInMonth = 0;
    switch (_dateTime.date.month)
    {
        case 0x01:
        case 0x03:
        case 0x05:
        case 0x07:
        case 0x08:
        case 0x10:
        case 0x12:
        {
            daysInMonth = 31;
            break;
        }
        case 0x04:
        case 0x06:
        case 0x09:
        case 0x11:
        {
            daysInMonth = 30;
            break;
        }
        case 0x02:
        {
            u32 year = 2000 + FromBcd(_dateTime.date.year);
            // It is sufficient here to check if the year is divisible by 4
            if ((year & 3) == 0)
            {
                daysInMonth = 29;
            }
            else
            {
                daysInMonth = 28;
            }
            break;
        }
    }
    if (FromBcd(value) > daysInMonth)
    {
        value = 1;
        u32 month = FromBcd(_dateTime.date.month);
        if (++month == 13)
        {
            month = 1;
        }

        mem_swapByte(ToBcd(month), &_dateTime.date.month);
    }

    mem_swapByte(value, &_dateTime.date.monthDay);
}

void RomGpioRtc::SetDayOfWeek(u8 value)
{
    value &= 7;
    if (value == 7)
    {
        value = 0;
    }
    mem_swapByte(value, &_dateTime.date.weekDay);
}

void RomGpioRtc::SetHour(u8 value)
{
    if (_statusRegister & RIO_RTC_STATUS_24H)
    {
        value &= 0x3F;
        if ((value >= 0x24 && value <= 0x29) ||
            (value & 0xF) > 9)
        {
            value = 0;
        }

        if (FromBcd(value) >= 12)
        {
            value |= 0x80; // PM flag
        }
    }
    else
    {
        value &= 0xBF;
        if (((value & ~0x80) >= 0x12 && (value & ~0x80) <= 0x19) ||
            (value & 0xF) > 9)
        {
            value = 0;
        }
    }
    mem_swapByte(value, &_dateTime.time.hour);
}

void RomGpioRtc::SetMinute(u8 value)
{
    value &= 0x7F;
    if ((value >= 0x60 && value <= 0x79) ||
        (value & 0xF) > 9)
    {
        value = 0;
    }
    mem_swapByte(value, &_dateTime.time.minute);
}

void RomGpioRtc::SetSecond(u8 value)
{
    value &= 0x7F;
    if ((value >= 0x60 && value <= 0x79) ||
        (value & 0xF) > 9)
    {
        value = 0;
    }
    mem_swapByte(value, &_dateTime.time.second);
}

u8 RomGpioRtc::FromBcd(u8 bcdValue)
{
    return (bcdValue >> 4) * 10 + (bcdValue & 0xF);
}

u8 RomGpioRtc::ToBcd(u8 value)
{
    u32 lowDigit = value % 10;
    u32 highDigit = value / 10;
    return (highDigit << 4) | lowDigit;
}
