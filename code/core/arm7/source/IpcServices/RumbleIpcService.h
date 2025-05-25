#pragma once
#include "ThreadIpcService.h"
#include "RumbleIpcCommand.h"
#include "IpcChannels.h"

class RumbleIpcService : public ThreadIpcService
{
    u32 _threadStack[128];

public:
    RumbleIpcService()
        : ThreadIpcService(IPC_CHANNEL_RUMBLE, 11, _threadStack, sizeof(_threadStack)) { }

    void HandleMessage(u32 data) override;
};
