#include "common.h"
#include <string.h>
#include "Fat/ff.h"
#include "Fat/diskio.h"
#include "Fat/FsIpc.h"
#include "VirtualMachine/VMNestedIrq.h"
#include "cp15.h"
#include "Cpsr.h"
#include "SdCache.h"
#include "MemCopy.h"
#include "Slot2.h"

typedef struct
{
    vu16 cacheBlock;
    vu32 romBlock;
} SdcFetch;

static SdcFetch sCurrentFetch;

[[gnu::section(".vramhi.bss")]]
void* sdc_romBlockToCacheBlock[SDC_ROM_BLOCK_COUNT];

/// @brief Random generator state for random cache replacement.
static u32 sRandomState;

/// @brief Maps sd cache blocks to rom blocks.
static u16 sCacheBlockToRomBlock[SDC_BLOCK_COUNT];

/// @brief The number of usable blocks in the cache. This can be less than the
///        total number of cache blocks when some blocks are permanently loaded.
static u32 sBlockCount;

static u32 sTabuBlock;
vu32 gSdCacheIrqForbiddenRomBlockReplacementRange;

static DWORD sClusterTable[512];

// temporarily
extern FIL gFile;

extern bool gSlot2Active;

/// @brief Returns a cache block to replace.
/// @return The index of the cache block to replace.
static u32 getBlockToReplace(void)
{
    sRandomState = sRandomState * 1566083941u + 2531011u;
    u32 maxPlusOne = sBlockCount;
    if (sTabuBlock != SDC_BLOCK_INVALID)
    {
        maxPlusOne--;
    }
    u32 block = ((sRandomState >> 16) * maxPlusOne) >> 16;
    return block == sTabuBlock ? (sBlockCount - 1) : block;
}

static bool isCurrentlyFetching(void)
{
    return sCurrentFetch.cacheBlock != SDC_BLOCK_INVALID;
}

static void finishFetch()
{
    sCacheBlockToRomBlock[sCurrentFetch.cacheBlock] = sCurrentFetch.romBlock;
    sdc_romBlockToCacheBlock[sCurrentFetch.romBlock] = &sdc_cache[sCurrentFetch.cacheBlock][0];
    sCurrentFetch.romBlock = SDC_ROM_BLOCK_INVALID;
    sCurrentFetch.cacheBlock = SDC_BLOCK_INVALID;
    dc_drainWriteBuffer();
}

static u32 getSdSectorOfRomBlock(u32 romBlock)
{
    u32 romOffset = romBlock * SDC_BLOCK_SIZE;
    if (romOffset >= f_size(&gFile))
    {
        return 0;
    }

    FATFS* fs = gFile.obj.fs;
    u32* tbl = gFile.cltbl + 1;
    u32 csect = (UINT)(romOffset / 512 & (fs->csize - 1));
    u32 cshift = __builtin_ctz(fs->csize) + 9;
    u32 cl = (DWORD)(romOffset >> cshift);
    while (true)
    {
        u32 ncl = *tbl++;
        if (cl < ncl)
        {
            break;
        }
        cl -= ncl;
        tbl++;
    }
    u32 cluster = cl + *tbl - 2;
    u32 sector = fs->database + fs->csize * cluster;
    sector += csect;
    return sector;
}

static void fillOutOfBoundsCacheBlock(u32 romBlock, u32 cacheBlock)
{
    u32 romSize = f_size(&gFile);
    u32 powerOf2RomSize = romSize < 0x100000 ? 0x100000 : (1 << (32 - __builtin_clz(romSize - 1)));
    if (romBlock * SDC_BLOCK_SIZE < powerOf2RomSize)
    {
        memset(&sdc_cache[cacheBlock][0], 0xFF, SDC_BLOCK_SIZE);
    }
    else
    {
        for (u32 i = 0; i < SDC_BLOCK_SIZE; i += 4)
        {
            u32 address = romBlock * SDC_BLOCK_SIZE + i;
            u32 oobValue = (address >> 1) & 0xFFFF;
            *(u32 *)&sdc_cache[cacheBlock][i] = oobValue | ((oobValue + 1) << 16);
        }
    }

    sCacheBlockToRomBlock[cacheBlock] = romBlock;
    sdc_romBlockToCacheBlock[romBlock] = &sdc_cache[cacheBlock][0];
    dc_drainWriteBuffer();
}

/// @brief Loads a rom block to the given buffer.
/// @param romBlock Rom block index to load.
/// @param dst The destination buffer.
static void* loadRomBlock(u32 romBlock, u32 cacheBlock)
{
    // If using SLOT2, we don't need to unscramble our romblock cache. 
    u32 sector = 0;
    if(!gSlot2Active) {
        sector = getSdSectorOfRomBlock(romBlock);
        if (sector == 0)
        {
            return &sdc_cache[0][0];
        }
    }

    u32 irqs = fs_waitForCompletionOfCurrentTransaction(true);
    if (isCurrentlyFetching())
    {
        finishFetch();
    }

    void* currentCacheBlock = sdc_romBlockToCacheBlock[romBlock];
    if (currentCacheBlock)
    {
        arm_restoreIrqs(irqs);
        return currentCacheBlock;
    }

    if (cacheBlock == SDC_BLOCK_INVALID)
    {
        cacheBlock = getBlockToReplace();
        if ((arm_getCpsr() & 0x1F) == 0x12)
        {
            u32 forbiddenReplacementRange = gSdCacheIrqForbiddenRomBlockReplacementRange;
            if (forbiddenReplacementRange != 0)
            {
                u32 forbiddenReplacementRangeStart = forbiddenReplacementRange & 0xFFFF;
                u32 forbiddenReplacementRangeEnd = forbiddenReplacementRange >> 16;
                while (true)
                {
                    u32 oldRomBlock = sCacheBlockToRomBlock[cacheBlock];
                    if (oldRomBlock == SDC_ROM_BLOCK_INVALID ||
                        !(forbiddenReplacementRangeStart <= oldRomBlock && oldRomBlock < forbiddenReplacementRangeEnd))
                    {
                        break;
                    }
                    cacheBlock = getBlockToReplace();
                }
            }
        }
    }
    u32 oldRomBlock = sCacheBlockToRomBlock[cacheBlock];
    if (oldRomBlock != SDC_ROM_BLOCK_INVALID)
    {
        sdc_romBlockToCacheBlock[oldRomBlock] = NULL;
        sCacheBlockToRomBlock[cacheBlock] = SDC_ROM_BLOCK_INVALID;
    }
    
    // SLOT2 copies the block afterwards, rather than pulling from the SD Cache here.
    FsWaitToken waitToken;
    if(!gSlot2Active)
    {
        fs_readCacheAlignedSectorsAsync(
            gFile.obj.fs->pdrv == DEV_FAT ? FS_DEVICE_DLDI : FS_DEVICE_DSI_SD,
            &sdc_cache[cacheBlock][0], sector,
            SDC_BLOCK_SIZE / 512, &waitToken);
    } //else memset(&waitToken, 0, sizeof(FsWaitToken));
    sCurrentFetch.romBlock = romBlock;
    sCurrentFetch.cacheBlock = cacheBlock;

    if ((arm_getCpsr() & 0x1F) != 0x12)
    {
        if(gSlot2Active) arm_disableIrqs();
        sTabuBlock = cacheBlock;
    }
    arm_restoreIrqs(irqs);
    if(gSlot2Active) 
    {
        mem_copy32((void*)(0x08000000 + (romBlock * SDC_BLOCK_SIZE)), &sdc_cache[cacheBlock][0], SDC_BLOCK_SIZE);
    }
    else 
    {
        irqs = fs_waitForCompletion(&waitToken, true);
    }
    if(gSlot2Active)
    {
        arm_restoreIrqs(irqs);
    }
    if (sCurrentFetch.romBlock == romBlock)
    {
        finishFetch();
    }

    arm_restoreIrqs(irqs);
    if (sector = 0)
    {
        fillOutOfBoundsCacheBlock(romBlock, cacheBlock);
    }
    return &sdc_cache[cacheBlock][0];
}
extern void logAddress(u32 address);

const void* sdc_loadRomBlockDirect(u32 romAddress)
{
    vm_enableNestedIrqs();
    // logAddress(romAddress);
    u32 romBlock = ((romAddress << 7) >> 7) / SDC_BLOCK_SIZE;
    void* cacheBlock = loadRomBlock(romBlock, SDC_BLOCK_INVALID);
    vm_disableNestedIrqs();
    return cacheBlock;
}

void* sdc_loadRomBlockForPatching(u32 romAddress)
{
    u32 romBlock = ((romAddress << 7) >> 7) / SDC_BLOCK_SIZE;
    void* data = sdc_romBlockToCacheBlock[romBlock];
    // if not loaded at all yet, or not permanent
    if (!data || (u32)data < (u32)&sdc_cache[sBlockCount][0])
    {
        if (data)
        {
            // if already loaded, but not permanent, invalidate block
            sdc_romBlockToCacheBlock[romBlock] = NULL;
            sCacheBlockToRomBlock[((u32)data - (u32)&sdc_cache[0][0]) / SDC_BLOCK_SIZE] = SDC_ROM_BLOCK_INVALID;
        }

        data = loadRomBlock(romBlock, --sBlockCount);
    }
    return (void*)((u32)data + (romAddress & SDC_BLOCK_MASK));
}

void sdc_init(void)
{
    sRandomState = 0xA512ED48; // initial random seed
    sBlockCount = SDC_BLOCK_COUNT;
    for (u32 i = 0; i < SDC_ROM_BLOCK_COUNT; i++)
    {
        sdc_romBlockToCacheBlock[i] = NULL;
    }
    for (u32 i = 0; i < SDC_BLOCK_COUNT; i++)
    {
        sCacheBlockToRomBlock[i] = SDC_ROM_BLOCK_INVALID;
    }

    sCurrentFetch.cacheBlock = SDC_BLOCK_INVALID;
    sCurrentFetch.romBlock = SDC_ROM_BLOCK_INVALID;
    gSdCacheIrqForbiddenRomBlockReplacementRange = 0;

    sClusterTable[0] = sizeof(sClusterTable) / sizeof(DWORD);

    if(!gSlot2Active){
        gFile.cltbl = sClusterTable;
        u32 result = f_lseek(&gFile, CREATE_LINKMAP);
        if (result != FR_OK)
        {
            logAddress(0xDEADBEEF);
            logAddress(result);
        }
    }
}
