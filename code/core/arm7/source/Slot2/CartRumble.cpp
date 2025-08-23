#include "common.h"
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include "IpcChannels.h"
#include "CartRumble.h"

static RumbleDevice detectedRumbleDevice;

// EZFlash control sequence
// Compatible with every known 3in1 revision (V1, V2, V3) and OMEGA editions
void CartRumble::EzFlashCommand(u32 command,u16 value)
{
    // Command Start Routine (Every command sent to the EZ starts with this)
    *(vu16 *)0x09FE0000 = 0xD200;
    *(vu16 *)0x08000000 = 0x1500;
    *(vu16 *)0x08020000 = 0xD200;
    *(vu16 *)0x08040000 = 0x1500;
    // Command Body
    *((vu16 *)command) = value;
    // Command End
    *(vu16 *)0x09FC0000 = 0x1500;
}

// We need to send a dummy write cmd to EZ Omega before we enable rumble
void CartRumble::TryToWakeEzFlashOde()
{
    EzFlashCommand(EZ_SET_ROM_PAGE, 381);
    EzFlashCommand(EZ_OPEN_NOR_WRITE, 0x1500);
    EzFlashCommand(EZ_SET_SERIAL_MODE, 0xE200);
    *(u16 *)(0x020000C0) = 0x5A45;
    *(vu16 *)(0x08000000) = 0x4D54;
}

void CartRumble::sc_change_mode(uint32_t mode)
{
    SC_REG_ENABLE = SC_ENABLE_MAGIC;
    SC_REG_ENABLE = SC_ENABLE_MAGIC;
    SC_REG_ENABLE = mode;
    SC_REG_ENABLE = mode;
}

RumbleDevice CartRumble::DetectDevice()
{
    // 1. Detect Supercard
    sc_change_mode(SC_ENABLE_CARD);
    uint16_t val = *(volatile uint16_t*)0x09800000;
    if ((val & 0xE300) == 0xC000)
    {
        return RUMBLE_SUPERCARD;
    }
    // 2. Detect RUMBLE_OTHER or NDS_RUMBLE_PAK
    for (int i = 0; i < 0x80; i++)
    {
        if (GBA_BUS[i] & BIT(1))
        {
            return RUMBLE_NONE;
        }
    }
    return RUMBLE_NDS_RUMBLE_PAK;
}

void CartRumble::DoRumble(bool enable)
{
    if (detectedRumbleDevice != RUMBLE_NDS_RUMBLE_PAK && detectedRumbleDevice != RUMBLE_SUPERCARD)
    {
        GBA_GPIO_DIRECTION = GBA_GPIO_ENABLE_CMD;
        GBA_GPIO_DATA = enable ? GBA_GPIO_ENABLE_CMD : GBA_GPIO_DISABLE_CMD;
    }
    if(detectedRumbleDevice == RUMBLE_SUPERCARD)
        sc_change_mode(SC_ENABLE_RUMBLE);
    DS_RUMBLE_PAK_CTRL = enable ? DS_RUMBLE_PAK_ON : DS_RUMBLE_PAK_OFF;
}

void CartRumble::InitSlot2(u16 strength)
{
    if (!isDSiMode())
    {
        detectedRumbleDevice = CartRumble::DetectDevice();

        if (detectedRumbleDevice == RUMBLE_SUPERCARD)
        {
            CartRumble::sc_change_mode(SC_ENABLE_CARD);
        }
        else
        {
            CartRumble::TryToWakeEzFlashOde();
            CartRumble::EzFlashCommand(EZ_UNLOCK_RUMBLE, strength);
        }
    }
}
