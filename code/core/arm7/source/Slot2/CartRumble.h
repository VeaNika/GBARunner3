#pragma once

#include "CartDefinitions.h"

enum RumbleDevice
{
    RUMBLE_NONE,
    RUMBLE_NDS_RUMBLE_PAK,
    RUMBLE_SUPERCARD, 
    RUMBLE_GBA_CART,
    RUMBLE_OTHER
};

class CartRumble
{
public:
    void InitSlot2(u16 strength);
    void DoRumble(bool enable);
    void Init();
    
    RumbleDevice DetectDevice();

private:
    void EzFlashCommand(u32 command,u16 value);
    void TryToWakeEzFlashOde();
    void sc_change_mode(uint32_t mode);
};
