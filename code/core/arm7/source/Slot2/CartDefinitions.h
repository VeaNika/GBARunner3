#pragma once

#include <nds/memory.h>
#include <nds/system.h>
#include "common.h"

// Nintendo DS Oficial "Rumble Pak" and some third party rumble devices
#define DS_RUMBLE_PAK_CTRL          (*(vu16 *)0x08001000)

#define DS_RUMBLE_PAK_ON            (1 << 1)
#define DS_RUMBLE_PAK_OFF           0

// GBA Games with motor rumble (Wario Ware Twisted and Drill Dozer)
#define GBA_GPIO_DATA	            (*(vu16 *)0x080000C4)
#define GBA_GPIO_DIRECTION          (*(vu16 *)0x080000C6)

#define GBA_GPIO_ENABLE_CMD         (1 << 3)
#define GBA_GPIO_DISABLE_CMD        0

// EZFlash Rumble
#define EZ_UNLOCK_RUMBLE            0x09E20000 // Sets EZFlash in DS Rumble Pak mode
#define EZ_SET_ROM_PAGE             0x09880000 // Select EZFlash Rom Page
#define EZ_OPEN_NOR_WRITE           0x09C40000 // Unlock EZFlash NOR writes
#define EZ_SET_SERIAL_MODE          0x09A40000 // Set EZFlash in serial bus mode

// SC Rumble
#define SC_REG_ENABLE               (*(vu16 *)0x9FFFFFE)
#define SC_ENABLE_MAGIC             0xA55A

#define SC_ENABLE_RAM               (1 << 0)
#define SC_ENABLE_CARD              (1 << 1)
#define SC_ENABLE_WRITE             (1 << 2) // To be used with SC_ENABLE_RAM
#define SC_ENABLE_RUMBLE            (1 << 3)

// TODO: Add Definitions for Gyro, Tilt and Solar Sensor GPIO carts.