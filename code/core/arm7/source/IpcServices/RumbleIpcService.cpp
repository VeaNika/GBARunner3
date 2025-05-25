#include "common.h"
#include "RumbleIpcService.h"
#include "Slot2/CartRumble.h"

static CartRumble sRumble;

void RumbleIpcService::HandleMessage(u32 data)
{
    switch (data & 0xF)
    {
        case RUMBLE_IPC_CMD_INIT:
        {
            sRumble.InitSlot2(0xF2);
            SendResponseMessage(0);
            break;
        }
        case RUMBLE_IPC_CMD_ON:
        {
            sRumble.DoRumble(true);
            SendResponseMessage(0);
            break;
        }
        case RUMBLE_IPC_CMD_OFF:
        {
            sRumble.DoRumble(false);
            SendResponseMessage(0);
            break;
        }
    }
}
