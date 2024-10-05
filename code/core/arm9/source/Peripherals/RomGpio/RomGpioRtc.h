#pragma once

class RomGpio;

class RomGpioRtc
{
private:
    enum class RtcTransferState
    {
        CommandWaitFallingEdge,
        CommandWaitRisingEdge,
        InDataWaitFallingEdge,
        InDataWaitRisingEdge,
        OutDataWaitFallingEdge,
        OutDataWaitRisingEdge,
        Done
    };

    typedef struct
    {
        u8 year;
        u8 month;
        u8 monthDay;
        u8 weekDay;
    } rio_rtc_date_t;

    typedef struct
    {
        u8 hour;
        u8 minute;
        u8 second;
    } rio_rtc_time_t;

    typedef struct
    {
        rio_rtc_date_t date;
        rio_rtc_time_t time;
    } rio_rtc_datetime_t;

public:
    RomGpioRtc()
        : _state(RtcTransferState::CommandWaitFallingEdge), _shiftRegister(0), _bitCount(0)
        , _statusRegister(0x40), _intRegister(0)
    {
        *(u32*)&_dateTime.date = 0x030A0224;
        *(u32*)&_dateTime.time = 0x00300910;
    }

    void Update(RomGpio& romGpio);

private:
    RtcTransferState _state;
    u32 _shiftRegister;
    u32 _bitCount;
    u32 _command;
    u32 _byteIndex;
    u16 _statusRegister;
    u16 _intRegister;
    rio_rtc_datetime_t _dateTime;
    u8 _padding;

    void CommandWaitRisingEdge(RomGpio& romGpio);
    void HandleInDataWaitRisingEdge(RomGpio& romGpio);
    void HandleOutDataWaitFallingEdge(RomGpio& romGpio);

    void RtcReset();

    void SetYear(u8 value);
    void SetMonth(u8 value);
    void SetDayOfMonth(u8 value);
    void SetDayOfWeek(u8 value);
    void SetHour(u8 value);
    void SetMinute(u8 value);
    void SetSecond(u8 value);

    u8 FromBcd(u8 bcdValue);
    u8 ToBcd(u8 value);
};
