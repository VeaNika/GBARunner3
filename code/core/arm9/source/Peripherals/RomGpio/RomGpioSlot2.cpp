#include "common.h"
#include <nds/system.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcFifoSystem.h>
#include <libtwl/mem/memExtern.h>
#include "Core/Environment.h"
#include "IpcChannels.h"
#include "RomGpioSlot2.h"

#define ROM_GPIO_PIN_MOT    3

void RomGpioSlot2::SendIpcCommand(RumbleIpcCommand cmd)
{
    ipc_sendWordDirect(cmd << IPC_FIFO_MSG_CHANNEL_BITS | IPC_CHANNEL_RUMBLE);
    while (ipc_isRecvFifoEmpty());
    ipc_recvWordDirect();
}

void RomGpioSlot2::Init(RomGpio& romGpio)
{
    if (!Environment::IsDsiMode())
    {
        mem_setGbaCartridgeCpu(EXMEMCNT_SLOT2_CPU_ARM7);
        SendIpcCommand(RUMBLE_IPC_CMD_INIT);
    }
}

void RomGpioSlot2::Rumble(RomGpio& romGpio)
{
    bool gpio3State = romGpio.GetPinState(ROM_GPIO_PIN_MOT);
    if (gpio3State != _lastGpioState)
    {
        SendIpcCommand(gpio3State ? RUMBLE_IPC_CMD_ON : RUMBLE_IPC_CMD_OFF);
        _lastGpioState = gpio3State;
    }
}

void RomGpioSlot2::Update(RomGpio& romGpio)
{
    if (!Environment::IsDsiMode())
    {
        Rumble(romGpio);
    }
}