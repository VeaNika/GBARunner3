#pragma once
#include "ColorLut.h"

struct ColorProfile
{
    int matrix[3][3]; // coeficients multiplied by 1000
    int luminance;    // multiplier *100, i.g. 93
};

// libretro shader values. Credits: hunterk and Pokefan531.
// Last updated 2025-12-03.

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
constexpr inline ColorProfile Agb001 =
{
    {
        { 905, 195, -100 },
        { 100, 650, 250 },
        { 158, 143, 700 }
    },
    91
};

// libretro GBA SP (AGS-101) color (sRGB).
constexpr inline ColorProfile Ags101 =
{
    {
        { 960, 110, -70 },
        { 33, 890, 78 },
        { 1, -30, 1029 }
    },
    94
};

// libretro GB micro OXY-001 color (sRGB).
constexpr inline ColorProfile Oxy001 =
{
    {
        { 803, 310, -113 },
        { 100, 688, 213 },
        { 123, 113, 765 }
    },
    90
};

//// libretro NDS Phat NTR-001 color (sRGB).
constexpr inline ColorProfile Ntr001 =
{
    {
        { 835, 270, -105 },
        { 100, 638, 263 },
        { 105, 175, 720 }
    },
    91
};

// libretro NDS lite USG-001 color (sRGB).
constexpr inline ColorProfile Usg001 =
{
    {
        { 930, 140, -70 },
        { 25, 900, 75 },
        { 8, -30, 1022 }
    },
    94
};

// libretro PSP 1000 color (sRGB).
constexpr inline ColorProfile PspO1g =
{
    {
        { 835, 160, -130 },
        { 43, 745, 155 },
        { 23, 15, 963 }
    },
    99
};

// libretro Nintendo Switch GBA Classics color (sRGB).
constexpr inline ColorProfile NswIps =
{
    {
        { 865, 123, 13 },
        { 58, 925, 13 },
        { 58, 123, 820 }
    },
    100
};

// libretro Nintendo Switch GBA Classics Oled color (sRGB).
constexpr inline ColorProfile NswOle =
{
    {
        { 1540, -480, -60 },
        { -45, 1063, -18 },
        { -23, -75, 1103 }
    },
    100
};

// libretro Visual Boy Advance color.
constexpr inline ColorProfile VbaEmu =
{
    {
        { 730, 270, 0 },
        { 85, 675, 240 },
        { 85, 240, 675 }
    },
    100
};

// libretro No$GBA full color.
constexpr inline ColorProfile NoCash =
{
    {
        { 730, 270, 0 },
        { 83, 678, 240 },
        { 83, 240, 678 }
    },
    100
};

// libretro mGBA GBA shader color.
constexpr inline ColorProfile mGba01 =
{
    {
        { 840, 180, 0 },
        { 90, 670, 260 },
        { 150, 100, 730 }
    },
    99
};

constexpr const ColorProfile* colorProfileLut[] =
{
    /* GbaColorCorrection::None     */ nullptr,
    /* GbaColorCorrection::Agb001   */ &Agb001,
    /* GbaColorCorrection::Ags101   */ &Ags101,
    /* GbaColorCorrection::Oxy001   */ &Oxy001,
    /* GbaColorCorrection::Ntr001   */ &Ntr001,
    /* GbaColorCorrection::Usg001   */ &Usg001,
    /* GbaColorCorrection::PspO1g   */ &PspO1g,
    /* GbaColorCorrection::NswIps   */ &NswIps,
    /* GbaColorCorrection::NswOle   */ &NswOle,
    /* GbaColorCorrection::VbaEmu   */ &VbaEmu,
    /* GbaColorCorrection::NoCash   */ &NoCash,
    /* GbaColorCorrection::mGba01   */ &mGba01
};
