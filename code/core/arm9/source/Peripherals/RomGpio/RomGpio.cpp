#include "common.h"
#include "RomGpioRtc.h"
#include "RomGpio.h"

RomGpio gRomGpio;
static RomGpioRtc sRomGpioRtc;

void RomGpio::Initialize(rio_registers_t* romGpioRegisters)
{
    _registers = romGpioRegisters;
    _registersRomData = *romGpioRegisters;
    Reset();
}

void RomGpio::Reset()
{
    _inputData = 0;
    _outputData = 0;
    _direction = 0;
    _control = RIO_CONTROL_READ_DISABLE;
    UpdateRomRegisters();
}

void RomGpio::UpdateRomRegisters()
{
    if (_control == RIO_CONTROL_READ_DISABLE)
    {
        // When reading of the registers is disabled, the original rom data is read.
        *_registers = _registersRomData;
    }
    else
    {
        _registers->data = GetGpioState();
        _registers->direction = _direction;
        _registers->control = RIO_CONTROL_READ_ENABLE;
    }
}

static void updateRomGpioPeripherals()
{
    sRomGpioRtc.Update(gRomGpio);
}

extern "C" void rio_write(u32 offset, u16 value)
{
    switch (offset)
    {
        case offsetof(rio_registers_t, data):
        {
            gRomGpio.WriteDataRegister(value);
            updateRomGpioPeripherals();
            break;
        }
        case offsetof(rio_registers_t, direction):
        {
            gRomGpio.WriteDirectionRegister(value);
            updateRomGpioPeripherals();
            break;
        }
        case offsetof(rio_registers_t, control):
        {
            gRomGpio.WriteControlRegister(value);
            break;
        }
    }
}