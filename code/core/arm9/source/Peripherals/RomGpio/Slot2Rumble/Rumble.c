// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the GBARunner3 project.
//
// Copyright (c) 2023 Adrian "asie" Siekierka

// TODO: Replace this header with the official BlocksDS version once the project is ported.

#include <nds/ndstypes.h>
#include <nds/system.h>
#include "Rumble.h"

bool isRumbleInserted(void)
{
    if (!peripheralSlot2IsDetected())
        rumbleInit();
    return peripheralSlot2GetSupportMask() & SLOT2_PERIPHERAL_RUMBLE_ANY;
}

uint8_t rumbleGetMaxRawStrength(void)
{
    uint32_t mask = peripheralSlot2GetSupportMask();
    if (mask & SLOT2_PERIPHERAL_RUMBLE_EZ)
        return 3;
    if (mask & SLOT2_PERIPHERAL_RUMBLE_ANY)
        return 1;
    return 0;
}

bool rumbleIsEdgeActivated(void)
{
    return peripheralSlot2GetSupportMask() & SLOT2_PERIPHERAL_RUMBLE_PAK;
}

#define RUMBLE_PAK_CTRL (*(vuint16 *)0x08001000)
#define GPIO_DATA       (*(vuint16 *)0x080000C4)
extern void __libnds_slot2EzCommand(uint32_t address, uint16_t value);
static const uint8_t ez_rumble_table[] = {0x08, 0xF0, 0xF2, 0xF1};

void setRumble(uint8_t strength)
{
    uint32_t mask = peripheralSlot2GetSupportMask();
    peripheralSlot2Open(SLOT2_PERIPHERAL_RUMBLE_ANY);

    if (mask & SLOT2_PERIPHERAL_RUMBLE_GPIO)
    {
        GPIO_DATA = (GPIO_DATA & ~0x8) | (strength ? 0x8 : 0x0);
    }
    else if (mask & SLOT2_PERIPHERAL_RUMBLE_PAK)
    {
        if (mask & SLOT2_PERIPHERAL_RUMBLE_EZ)
        {
            __libnds_slot2EzCommand(0x9E20000, ez_rumble_table[strength > 3 ? 3 : strength]);
        }
        RUMBLE_PAK_CTRL = strength ? 0x2 : 0x0;
    }
    else if (mask & SLOT2_PERIPHERAL_SLIDE_MAGKID)
    {
        // TODO: Untested.
        RUMBLE_PAK_CTRL = strength ? 0x100 : 0x000;
    }
}