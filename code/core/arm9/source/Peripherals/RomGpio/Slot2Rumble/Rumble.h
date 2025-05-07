// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the GBARunner3 project.
//
// Copyright (C) 2023 Adrian "asie" Siekierka

// TODO: Replace this header with the official BlocksDS version once the project is ported.

#ifndef LIBNDS_NDS_ARM9_RUMBLE_H__
#define LIBNDS_NDS_ARM9_RUMBLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/// @file nds/arm9/rumble.h
///
/// @brief Slot-2 Rumble.

#include <stdbool.h>
#include <stdint.h>

#include "slot2.h"

/// Initialize the rumble device.
///
/// @deprecated
///     Use of peripheralSlot2Init() is recommended instead.
static inline void rumbleInit(void)
{
    peripheralSlot2InitDefault();
}

/// Check if a rumble device has been inserted.
///
/// @return
///     True if a rumble device has been inserted, false otherwise.
bool isRumbleInserted(void);

/// Check if a rumble device is edge-activated.
///
/// An edge-activated rumble device triggers its actuator when calling
/// setRumble(1) if setRumble(0) was called prior.
///
/// A non-edge-activated rumble device triggers its motor starting from
/// when setRumble(1) was called, until setRumble(0) is called.
///
/// @return
///     It returns true if the device is edge-activated.
bool rumbleIsEdgeActivated(void);

/// Get the maximum rumble strength.
///
/// @return
///     The maximum rumble strength for this device.
uint8_t rumbleGetMaxRawStrength(void);

/// Set the rumble device enable/disable pin.
///
/// Note that rumbleEnable() and rumbleDisable() are more user-friendly.
///
/// @param rawStrength
///     The raw rumble strength (rumbleGetMaxRawStrength()).
///
/// @see rumbleIsEdgeActivated
void setRumble(uint8_t rawStrength);
#define RUMBLE_STRENGTH_HIGHEST 0xFF

/// Emit a single, fast rumble tick.
///
/// @param rawStrength
///     The raw rumble strength (rumbleGetMaxRawStrength()).
static inline void rumbleTick(uint8_t rawStrength)
{
    setRumble(rawStrength);
    setRumble(0);
}

#ifdef __cplusplus
}
#endif

#endif // LIBNDS_NDS_ARM9_RUMBLE_H__