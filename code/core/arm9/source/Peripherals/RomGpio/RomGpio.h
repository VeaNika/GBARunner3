#pragma once

#define RIO_GBA_ADDRESS             0x080000C4

#define RIO_PIN_MASK                0xF

#define RIO_CONTROL_READ_DISABLE    0
#define RIO_CONTROL_READ_ENABLE     1
#define RIO_CONTROL_MASK            1

struct rio_registers_t
{
    u16 data;
    u16 direction;
    u16 control;
};

class RomGpio
{
public:
    void Initialize(rio_registers_t* romGpioRegisters);
    void Reset();
    void UpdateRomRegisters();

    void WriteDataRegister(u16 value)
    {
        _outputData = value & RIO_PIN_MASK;
        UpdateRomRegisters();
    }

    void WriteDirectionRegister(u16 value)
    {
        _direction = value & RIO_PIN_MASK;
        UpdateRomRegisters();
    }

    void WriteControlRegister(u16 value)
    {
        _control = value & RIO_CONTROL_MASK;
        UpdateRomRegisters();
    }

    bool GetPinState(u32 pin)
    {
        return (GetGpioState() >> pin) & 1;
    }

    void SetPinState(u32 pin, bool isHigh)
    {
        u32 mask = (1 << pin) & RIO_PIN_MASK;
        if (isHigh)
        {
            _inputData |= mask;
        }
        else
        {
            _inputData &= ~mask;
        }

        UpdateRomRegisters();
    }

private:
    /// @brief Pointer to the actual registers readable by the GBA side.
    rio_registers_t* _registers;

    /// @brief The rom data behind the gpio registers.
    rio_registers_t _registersRomData;

    /// @brief The input data from the gpio pins.
    u16 _inputData;

    /// @brief The internal output data register.
    u16 _outputData;

    /// @brief The internal direction register.
    u16 _direction;

    /// @brief The internal control register.
    u16 _control;

    u32 GetGpioState()
    {
        return (_inputData & ~_direction) | (_outputData & _direction);
    }
};

extern RomGpio gRomGpio;
