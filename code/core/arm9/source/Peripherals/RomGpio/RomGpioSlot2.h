#pragma once

#include "RomGpio.h"
#include "RumbleIpcCommand.h"

class RomGpioSlot2
{
public:
    void Init(RomGpio& romGpio);
    void Update(RomGpio& romGpio);
    void Rumble(RomGpio& romGpio);
    void SendIpcCommand(RumbleIpcCommand cmd);

private:
    bool _initialized = false;
    bool _lastGpioState = false;
};
//extern RomGpioSlot2 gRomGpioSlot2;
