#pragma once
#include "Core/Math/fixed.h"

struct ColorProfile
{
    fix32<12> matrix[3][3]; // 3x3 correction matrix
    fix32<12> luminance;    // Luminance factor
};

// Libretro shader values. Credits: hunterk and Pokefan531.
// https://forums.libretro.com/t/real-gba-and-ds-phat-colors/1540/238
// Last updated 10-05-2025.

// const ColorProfile PRESET_NAME = 
//{
//    {
//        { [r], [gr], [br] },
//        { [rg], [g], [bg] },
//        { [rb], [gb], [b] }
//    },
//    luminance
//};

// libretro GBA AGB-001 color (sRGB).
const ColorProfile Agb001 =
{
    {
        { fix32<12>(0.905f), fix32<12>(0.195f), fix32<12>(-0.1f) },
        { fix32<12>(0.1f), fix32<12>(0.65f), fix32<12>(0.25f) },
        { fix32<12>(0.1575f), fix32<12>(0.1425f), fix32<12>(0.7f) }
    },
    fix32<12>(0.91f)
};

// libretro GBA SP (AGS-101) color (sRGB).
const ColorProfile Ags101 =
{
    {
        { fix32<12>(0.96f), fix32<12>(0.11f), fix32<12>(-0.07f) },
        { fix32<12>(0.0325f), fix32<12>(0.89f), fix32<12>(0.0775f) },
        { fix32<12>(0.001f), fix32<12>(-0.03f), fix32<12>(1.029f) }
    },
    fix32<12>(0.935f)
};

// libretro GB micro OXY-001 color (sRGB).
const ColorProfile Oxy001 =
{
    {
        { fix32<12>(0.8025f), fix32<12>(0.31f), fix32<12>(-0.1125f) },
        { fix32<12>(0.1f), fix32<12>(0.6875f), fix32<12>(0.2125f) },
        { fix32<12>(0.1225f), fix32<12>(0.1125f), fix32<12>(0.765f) }
    },
    fix32<12>(0.9f)
};

//// libretro NDS Phat NTR-001 color (sRGB).
const ColorProfile Ntr001 =
{
    {
        { fix32<12>(0.835f), fix32<12>(0.27f), fix32<12>(-0.105f) },
        { fix32<12>(0.1f), fix32<12>(0.6375f), fix32<12>(0.2625f) },
        { fix32<12>(0.105f), fix32<12>(0.175f), fix32<12>(0.72f) }
    },
    fix32<12>(0.905f)
};

// libretro NDS lite USG-001 color (sRGB).
const ColorProfile Usg001 =
{
    {
        { fix32<12>(0.93f), fix32<12>(0.14f), fix32<12>(-0.07f) },
        { fix32<12>(0.025f), fix32<12>(0.9f), fix32<12>(0.075f) },
        { fix32<12>(0.008f), fix32<12>(-0.03f), fix32<12>(1.022f) }
    },
    fix32<12>(0.935f)
};

// libretro PSP 1000 color (sRGB).
const ColorProfile Psp01g =
{
    {
        { fix32<12>(0.835f), fix32<12>(0.155f), fix32<12>(-0.135f) },
        { fix32<12>(0.0425f), fix32<12>(0.745f), fix32<12>(0.155f) },
        { fix32<12>(0.0225f), fix32<12>(0.0145f), fix32<12>(0.963f) }
    },
    fix32<12>(0.99f)
};

// libretro Nintendo Switch GBA Classics color (sRGB).
const ColorProfile NswIps =
{
    {
        { fix32<12>(0.865f), fix32<12>(0.1225f), fix32<12>(0.0125f) },
        { fix32<12>(0.0575f), fix32<12>(0.925f), fix32<12>(0.0125f) },
        { fix32<12>(0.0575f), fix32<12>(0.1225f), fix32<12>(0.82f) }
    },
    fix32<12>(1.f)
};

// libretro Nintendo Switch GBA Classics Oled color (sRGB).
const ColorProfile NswOle =
{
    {
        { fix32<12>(1.54f), fix32<12>(-0.48f), fix32<12>(-0.06f) },
        { fix32<12>(-0.045f), fix32<12>(1.0625f), fix32<12>(-0.0175f) },
        { fix32<12>(-0.0225f), fix32<12>(-0.075f), fix32<12>(1.1025f) }
    },
    fix32<12>(1.f)
};

// libretro Visual Boy Advance color.
const ColorProfile VbaEmu =
{
    {
        { fix32<12>(0.73f), fix32<12>(0.27f), fix32<12>(0.f) },
        { fix32<12>(0.0825f), fix32<12>(0.6775f), fix32<12>(0.24f) },
        { fix32<12>(0.0825f), fix32<12>(0.24f), fix32<12>(0.6775f) }
    },
    fix32<12>(1.f)
};

// libretro No$GBA full color.
const ColorProfile NoCash =
{
    {
        { fix32<12>(0.62f), fix32<12>(0.1913f), fix32<12>(-0.0917f) },
        { fix32<12>(0.004f), fix32<12>(0.56f), fix32<12>(0.167f) },
        { fix32<12>(0.004f), fix32<12>(0.167f), fix32<12>(0.56f) },
    },
    fix32<12>(0.649f)
};

// libretro mGBA GBA shader color.
const ColorProfile mGba01 =
{
    {
        { fix32<12>(0.84f), fix32<12>(0.18f), fix32<12>(0.f) },
        { fix32<12>(0.09f), fix32<12>(0.67f), fix32<12>(0.26f) },
        { fix32<12>(0.15f), fix32<12>(0.10f), fix32<12>(0.73f) }
    },
    fix32<12>(0.99f)
};

constexpr const ColorProfile* colorProfileLut[] =
{
    /* GbaColorCorrection::None     */ nullptr,
    /* GbaColorCorrection::Agb001   */ &Agb001,
    /* GbaColorCorrection::Ags101   */ &Ags101,
    /* GbaColorCorrection::Oxy001   */ &Oxy001,
    /* GbaColorCorrection::Ntr001   */ &Ntr001,
    /* GbaColorCorrection::Usg001   */ &Usg001,
    /* GbaColorCorrection::Psp01g   */ &Psp01g,
    /* GbaColorCorrection::NswIps   */ &NswIps,
    /* GbaColorCorrection::NswOle   */ &NswOle,
    /* GbaColorCorrection::VbaEmu   */ &VbaEmu,
    /* GbaColorCorrection::NoCash   */ &NoCash,
    /* GbaColorCorrection::mGba01   */ &mGba01
};
