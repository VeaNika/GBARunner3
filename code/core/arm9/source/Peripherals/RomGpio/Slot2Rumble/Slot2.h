// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the GBARunner3 project.
//
// Copyright (C) 2023 Adrian "asie" Siekierka

// TODO: Replace this header with the official BlocksDS version once the project is ported.

#ifndef LIBNDS_ARM9_PERIPHERALS_SLOT2_H__
#define LIBNDS_ARM9_PERIPHERALS_SLOT2_H__

#ifdef __cplusplus
extern "C" {
#endif

/// @file nds/arm9/peripherals/slot2.h
///
/// @brief Slot-2 peripheral detection, external RAM.

#include <stdbool.h>
#include <stdint.h>
#include <nds/ndstypes.h>

#define COMPILER_MEMORY_BARRIER() asm volatile("" ::: "memory")

// TODO: Peripherals marked as "TODO" are not currently detected.
// In addition, the following are not listed due to insufficient information:
// - Activity Meters

#define SLOT2_PERIPHERAL_NONE            0x00000000
#define SLOT2_PERIPHERAL_EXTRAM          0x00000001 ///< External RAM
#define SLOT2_PERIPHERAL_PADDLE          0x00000002 ///< Paddle controller (Taito)
#define SLOT2_PERIPHERAL_PIANO           0x00000004 ///< Piano keyboard (Easy Piano)
#define SLOT2_PERIPHERAL_GUITAR_GRIP     0x00000008 ///< Guitar grip
#define SLOT2_PERIPHERAL_RUMBLE_GPIO     0x00000010 ///< Rumble (GPIO; WarioWare/Drill Dozer)
#define SLOT2_PERIPHERAL_RUMBLE_PAK      0x00000020 ///< DS Rumble Pak
#define SLOT2_PERIPHERAL_SLIDE_MAGKID    0x00000040 ///< TODO: Slide controller (MagKid)
#define SLOT2_PERIPHERAL_RUMBLE_EZ       0x00000080 ///< Rumble (EZ)
#define SLOT2_PERIPHERAL_GYRO_GPIO       0x00000100 ///< Gyro sensor (GPIO; WarioWare)
#define SLOT2_PERIPHERAL_TILT            0x00000200 ///< Tilt sensor (Yoshi)
#define SLOT2_PERIPHERAL_SOLAR_GPIO      0x00000400 ///< Solar sensor (GPIO; Boktai)
#define SLOT2_PERIPHERAL_FACE_SCAN       0x00008000 ///< TODO: Facening Scan
#define SLOT2_PERIPHERAL_GPS_RANGER      0x00001000 ///< TODO: Ranger GPS
#define SLOT2_PERIPHERAL_ANY             0x00001FFF
#define SLOT2_PERIPHERAL_RUMBLE_ANY      (SLOT2_PERIPHERAL_RUMBLE_GPIO | SLOT2_PERIPHERAL_RUMBLE_PAK | SLOT2_PERIPHERAL_SLIDE_MAGKID | SLOT2_PERIPHERAL_RUMBLE_EZ)

/// Checks whether the application is running in a debugger or retail console.
///
/// It works on DS and DSi consoles.
///
/// @note
///     swiIsDebugger() only works with the cache disabled, this function
///     doesn't have any restrictions.
///
/// @return
///     Returns true if running on a debugger unit, false on retail units.
static inline bool isHwDebugger(void)
{
    extern bool __debugger_unit;
    return __debugger_unit;
}

// GBA_BUS is volatile, while GBAROM is not
//! 16 bit volatile pointer to the GBA slot bus.
#define GBA_BUS       ((vu16 *)(0x08000000))
//! 16 bit pointer to the GBA slot ROM.
#define GBAROM        ((u16*)0x08000000)

/// GBA file header format.
///
/// See gbatek for more info.
typedef struct sGBAHeader
{
    u32 entryPoint;   ///< 32 bits ARM opcode to jump to executable code
    u8 logo[156];     ///< Nintendo logo needed for booting the game
    char title[12];   ///< Game title
    char gamecode[4]; ///< Game code
    u16 makercode;    ///< Identifies the (commercial) developer
    u8 is96h;         ///< Fixed value that is always 96h
    u8 unitcode;      ///< Identifies the required hardware
    u8 devicecode;    ///< Used by Nintedo's hardware debuggers. Normally 0
    u8 unused[7];
    u8 version;       ///< The version of the game.
    u8 complement;    ///< Complement checksum of the gba header
    u16 checksum;     ///< A 16 bit checksum? (gbatek says its unused/reserved)
} tGBAHeader;

#define GBA_HEADER  (*(tGBAHeader *)0x08000000)

/// Initialize a Slot-2 peripheral.
///
/// Note that this method can take up to a few blocking frames to complete.
///
/// @param peripheral_mask
///     The peripheral mask to use. This allows narrowing the cartridge search
///     query down to specific cartridges for faster detection and minimizing
///     false positives.
///
/// @return
///     True if a peripheral was detected, false otherwise.
bool peripheralSlot2Init(uint32_t peripheral_mask);

/// Initialize any Slot-2 peripheral.
///
/// @return
///     True if a peripheral was detected, false otherwise.
static inline bool peripheralSlot2InitDefault(void)
{
    return peripheralSlot2Init(SLOT2_PERIPHERAL_ANY);
}

/// Un-initialize any previously detected peripheral.
void peripheralSlot2Exit(void);

/// Check if a Slot-2 peripheral has been detected.
///
/// @return
///     True if a peripheral was detected, false otherwise.
bool peripheralSlot2IsDetected(void);

/// Get the name of the detected Slot-2 peripheral, or "None".
///
/// @return
///     Pointer to the string. Don't call free() with this pointer.
const char *peripheralSlot2GetName(void);

/// Get the mask of SLOT2_PERIPHERALs supported by this device.
///
/// @returns
///     The mask.
uint32_t peripheralSlot2GetSupportMask(void);

/// Open (unlock) the specific Slot-2 peripheral.
///
/// This is necessary for some cartridges which may have multiple functions
/// (for example, external RAM and rumble in a conflicting address space).
///
/// @param peripheral_mask
///     The peripheral mask to unlock.
///
/// @return
///     True on success, false on failure (no peripheral of type present).
bool peripheralSlot2Open(uint32_t peripheral_mask);

/// Close (lock) the detected Slot-2 peripheral.
void peripheralSlot2Close(void);

/// Return the beginning of Slot-2 RAM space; NULL if not detected.
///
/// @return
///     A pointer to the start of the RAM space, or NULL.
uint16_t *peripheralSlot2RamStart(void);

/// Return the size, in bytes, of Slot-2 RAM space; 0 if not detected.
///
/// @return
///     The size in bytes.
uint32_t peripheralSlot2RamSize(void);

/// Return the number of Slot-2 RAM banks; 0 if not detected.
///
/// @return
///     The number of banks.
uint32_t peripheralSlot2RamBanks(void);

/// Switch to a different Slot-2 RAM bank.
///
/// @param bank
///     The bank to switch to.
void peripheralSlot2RamSetBank(uint32_t bank);

#ifdef __cplusplus
}
#endif

#endif // LIBNDS_ARM9_PERIPHERALS_SLOT2_H__