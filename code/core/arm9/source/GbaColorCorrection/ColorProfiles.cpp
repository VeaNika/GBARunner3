#include "ColorLut.h"

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
        { 0.905f, 0.195f, -0.1f },
        { 0.1f, 0.65f, 0.25f },
        { 0.1575f, 0.1425f, 0.7f }
    },
    0.91f
};

// libretro GBA SP (AGS-101) color (sRGB).
const ColorProfile Ags101 =
{
    {
        { 0.96f, 0.11f, -0.07f },
        { 0.0325f, 0.89f, 0.0775f },
        { 0.001f, -0.03f, 1.029f }
    },
    0.935f
};

// libretro GB micro OXY-001 color (sRGB).
const ColorProfile Oxy001 =
{
    {
        { 0.8025f, 0.31f, -0.1125f },
        { 0.1f, 0.6875f, 0.2125f },
        { 0.1225f, 0.1125f, 0.765f }
    },
    0.9f
};

//// libretro NDS Phat NTR-001 color (sRGB).
const ColorProfile Ntr001 =
{
    {
        { 0.835f, 0.27f, -0.105f },
        { 0.1f, 0.6375f, 0.2625f },
        { 0.105f, 0.175f, 0.72f }
    },
    0.905f
};

// libretro NDS lite USG-001 color (sRGB).
const ColorProfile Usg001 =
{
    {
        { 0.93f, 0.14f, -0.07f },
        { 0.025f, 0.9f, 0.075f },
        { 0.008f, -0.03f, 1.022f }
    },
    0.935f
};

// libretro PSP 1000 color (sRGB).
const ColorProfile Psp01g =
{
    {
        { 0.835f, 0.155f, -0.135f },
        { 0.0425f, 0.745f, 0.155f },
        { 0.0225f, 0.0145f, 0.963f }
    },
    0.99f
};

// libretro Nintendo Switch GBA Classics color (sRGB).
const ColorProfile NswIps =
{
    {
        { 0.865f, 0.1225f, 0.0125f },
        { 0.0575f, 0.925f, 0.0125f },
        { 0.0575f, 0.1225f, 0.82f }
    },
    1.f
};

// libretro Nintendo Switch GBA Classics Oled color (sRGB).
const ColorProfile NswOle =
{
    {
        { 1.54f, -0.48f, -0.06f },
        { -0.045f, 1.0625f, -0.0175f },
        { -0.0225f, -0.075f, 1.1025f }
    },
    1.f
};

// libretro Visual Boy Advance color.
const ColorProfile VbaEmu =
{
    {
        { 0.73f, 0.27f, 0.f },
        { 0.0825f, 0.6775f, 0.24f },
        { 0.0825f, 0.24f, 0.6775f }
    },
    1.f
};

// libretro No$GBA full color.
const ColorProfile NoCash =
{
    {
        { 0.62f, 0.1913f, -0.0917f },
        { 0.004f, 0.56f, 0.167f },
        { 0.004f, 0.167f, 0.56f },
    },
    0.949f
};

// libretro mGBA GBA shader color.
const ColorProfile mGba01 =
{
    {
        { 0.84f, 0.18f, 0.f },
        { 0.09f, 0.67f, 0.26f },
        { 0.15f, 0.10f, 0.73f }
    },
    0.99f
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
