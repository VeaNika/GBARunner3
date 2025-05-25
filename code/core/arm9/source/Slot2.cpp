#include "common.h"
#include "GbaHeader.h"
#include "MemCopy.h"
#include "MemFastSearch.h"
#include <libtwl/mem/memExtern.h>
#include "Save/Save.h"
#include "Slot2.h"

bool gSlot2Active = false;
bool gSaveInternal = false;
//bool gSDRomActive = true;

extern GbaHeader gRomHeader;

// Checks if SLOT2 holds a game cart.
extern "C" bool checkSlot2(){
    //if(gSDRomActive) return !gSDRomActive;
    if(gSlot2Active) return gSlot2Active;
    mem_setGbaCartridgeRamWait(EXMEMCNT_SLOT2_RAM_WAIT_10);
    mem_setGbaCartridgeRomWaits(EXMEMCNT_SLOT2_ROM_WAIT1_10, EXMEMCNT_SLOT2_ROM_WAIT2_6);
    mem_setGbaCartridgePhi(EXMEMCNT_SLOT2_PHI_LOW);
    mem_setGbaCartridgeCpu(EXMEMCNT_SLOT2_CPU_ARM9);

    mem_copy32((GbaHeader*)0x08000000u, &gRomHeader, sizeof(GbaHeader));
    
    gSlot2Active = (gRomHeader.gameCode != 0xFFFFFFFF);

    return gSlot2Active;
}

// Returns if it flags whether the save r/w is internal or SD if the user holds A. 
extern "C" bool checkSaveInternal(){
    if(gSaveInternal) return gSaveInternal;
    else if (!gSlot2Active) return false;

    if((keysHeld() & KEY_A) != 0) gSaveInternal = false;
    return gSaveInternal;
}