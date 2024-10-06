#include "common.h"
#include <libtwl/mem/memSwap.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/ipc/ipcFifo.h>
#include "cp15.h"
#include "IpcChannels.h"
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

[[gnu::section(".ewram.bss")]]
RomGpioRtc::rio_rtc_datetime_t RomGpioRtc::sDSRtcDateTime alignas(32);

void RomGpioRtc::Update(RomGpio& romGpio)
{
    if (!romGpio.GetPinState(ROM_GPIO_PIN_CS))
    {
        _state = RtcTransferState::CommandWaitFallingEdge;
        _shiftRegister = 0;
        _bitCount = 0;
        if (_offsetUpdateRequired)
        {
            _offsetUpdateRequired = false;
            UpdateRtcOffset();
        }
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
                bool isRead = _shiftRegister & 1;
                if (isRead &&
                    (_command == RIO_RTC_COMMAND_DATE_TIME || _command == RIO_RTC_COMMAND_TIME))
                {
                    UpdateDateTime();
                }
                _state = isRead
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
                _offsetUpdateRequired = true;
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
                _offsetUpdateRequired = true;
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
    UpdateRtcOffset();
}

void RomGpioRtc::UpdateDSDateTime()
{
    dc_invalidateRange(&sDSRtcDateTime, sizeof(sDSRtcDateTime));
    ipc_sendWordDirect(
        ((((u32)&sDSRtcDateTime) >> 2) << IPC_FIFO_MSG_CHANNEL_BITS) |
        IPC_CHANNEL_RTC);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

void RomGpioRtc::UpdateDateTime()
{
    UpdateDSDateTime();
    u32 dsRtcSecondsSince2000 = ToSecondsSinceJanuary2000(sDSRtcDateTime, true);
    u32 secondsSince2000 = dsRtcSecondsSince2000 + _rtcOffset;
    FromSecondsSinceJanuary2000(secondsSince2000, _dateTime, _statusRegister & RIO_RTC_STATUS_24H);
    int weekDay = (_dateTime.date.weekDay + _weekDayOffset) % 7;
    _dateTime.date.weekDay = weekDay < 0 ? weekDay + 7 : weekDay;
}

void RomGpioRtc::UpdateRtcOffset()
{
    UpdateDSDateTime();
    u32 dsRtcSecondsSince2000 = ToSecondsSinceJanuary2000(sDSRtcDateTime, true);
    u32 gbaRtcSecondsSince2000 = ToSecondsSinceJanuary2000(_dateTime, _statusRegister & RIO_RTC_STATUS_24H);
    rio_rtc_datetime_t newDateTime;
    FromSecondsSinceJanuary2000(gbaRtcSecondsSince2000, newDateTime, true);
    _rtcOffset = gbaRtcSecondsSince2000 - dsRtcSecondsSince2000;
    _weekDayOffset = (_dateTime.date.weekDay - newDateTime.date.weekDay) % 7;
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

    u32 year = FromBcd(_dateTime.date.year);
    u32 month = FromBcd(_dateTime.date.month);
    u32 daysInMonth = GetNumberOfDaysInMonth(2000 + year, month);
    if (FromBcd(value) > daysInMonth)
    {
        value = 1;
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

u32 RomGpioRtc::FromBcd(u32 bcdValue) const
{
    return bcdValue - 6 * (bcdValue >> 4);
}

u32 RomGpioRtc::ToBcd(u32 value) const
{
    static const u8 sToBcd[100] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99
    };

    return sToBcd[value];
}

u32 RomGpioRtc::ToSecondsSinceJanuary2000(const rio_rtc_datetime_t& dateTime, bool time24h) const
{
    u32 yearsSince2000 = FromBcd(dateTime.date.year);
    u32 leapDays = yearsSince2000 / 4;
    if ((yearsSince2000 & 3) != 0)
    {
        leapDays++;
    }

    u32 days = leapDays + yearsSince2000 * 365 + FromBcd(dateTime.date.monthDay) - 1;
    u32 month = FromBcd(dateTime.date.month);
    for (u32 i = 1; i < month; i++)
    {
        days += GetNumberOfDaysInMonth(2000 + yearsSince2000, i);
    }

    u32 hours = FromBcd(dateTime.time.hour & 0x1F);
    if (!time24h && (dateTime.time.hour & 0x80))
    {
        hours += 12;
    }

    return ((days * 24u + hours) * 60u
        + FromBcd(dateTime.time.minute & 0x3F)) * 60u
        + FromBcd(dateTime.time.second & 0x3F);
}

void RomGpioRtc::FromSecondsSinceJanuary2000(u32 secondsSinceJanuary2000, rio_rtc_datetime_t& dateTime, bool time24h) const
{
    dateTime.time.second = ToBcd(secondsSinceJanuary2000 % 60);
    u32 minutesSinceJanuary2000 = secondsSinceJanuary2000 / 60;
    dateTime.time.minute = ToBcd(minutesSinceJanuary2000 % 60);
    u32 hoursSinceJanuary2000 = minutesSinceJanuary2000 / 60;

    u32 hours = hoursSinceJanuary2000 % 24;
    u32 amPmFlag = hours >= 12 ? 0x80 : 0;
    if (!time24h && hours >= 12)
    {
        hours -= 12;
    }
    dateTime.time.hour = ToBcd(hours) | amPmFlag;

    u32 daysSinceJanuary2000 = hoursSinceJanuary2000 / 24;
    dateTime.date.weekDay = ((daysSinceJanuary2000 + 5) % 7); // 1 January 2000 was a Saturday

    u32 year = 2000 + daysSinceJanuary2000 * 4 / 1461;
    u32 remainingDays = ((daysSinceJanuary2000 * 4) % 1461) / 4;

    dateTime.date.year = ToBcd(year - 2000);

    u32 month = 1;
    u32 daysInMonth = GetNumberOfDaysInMonth(year, month);
    if (remainingDays >= daysInMonth)
    {
        remainingDays -= daysInMonth;
        month++;
        daysInMonth = GetNumberOfDaysInMonth(year, month);
        if (remainingDays >= daysInMonth)
        {
            remainingDays -= daysInMonth;
            month++;
            for (; month < 12; month++)
            {
                u32 daysInMonth = GetNumberOfDaysInMonth(year, month);
                if (remainingDays < daysInMonth)
                {
                    break;
                }
                remainingDays -= daysInMonth;
            }
        }
    }

    dateTime.date.month = ToBcd(month);
    dateTime.date.monthDay = ToBcd(remainingDays + 1);
}

u32 RomGpioRtc::GetNumberOfDaysInMonth(u32 year, u32 month) const
{
    static const u8 sDaysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    u32 result = sDaysPerMonth[month - 1];
    if (month == 2)
    {
        // It is sufficient here to check if the year is divisible by 4
        result += ((year & 3) == 0);
    }

    return result;
}
