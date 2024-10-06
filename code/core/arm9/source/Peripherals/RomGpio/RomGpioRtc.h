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
        , _statusRegister(0x40), _intRegister(0), _rtcOffset(0), _weekDayOffset(0), _offsetUpdateRequired(false) { }

    void Update(RomGpio& romGpio);

private:
    static rio_rtc_datetime_t sDSRtcDateTime;

    RtcTransferState _state;
    u32 _shiftRegister;
    u32 _bitCount;
    u32 _command;
    u32 _byteIndex;
    u16 _statusRegister;
    u16 _intRegister;
    rio_rtc_datetime_t _dateTime;
    u8 _padding;
    s32 _rtcOffset;
    s16 _weekDayOffset;
    u16 _offsetUpdateRequired;

    void CommandWaitRisingEdge(RomGpio& romGpio);
    void HandleInDataWaitRisingEdge(RomGpio& romGpio);
    void HandleOutDataWaitFallingEdge(RomGpio& romGpio);

    void RtcReset();
    void UpdateDSDateTime();
    void UpdateDateTime();
    void UpdateRtcOffset();

    void SetYear(u8 value);
    void SetMonth(u8 value);
    void SetDayOfMonth(u8 value);
    void SetDayOfWeek(u8 value);
    void SetHour(u8 value);
    void SetMinute(u8 value);
    void SetSecond(u8 value);

    u32 FromBcd(u32 bcdValue) const;
    u32 ToBcd(u32 value) const;

    u32 ToSecondsSinceJanuary2000(const rio_rtc_datetime_t& dateTime, bool time24h) const;
    void FromSecondsSinceJanuary2000(u32 secondsSinceJanuary2000, rio_rtc_datetime_t& dateTime, bool time24h) const;
    u32 GetNumberOfDaysInMonth(u32 year, u32 month) const;
};
