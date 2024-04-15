#include "common.h"
#include <algorithm>
#include <cmath>
#include "ColorLut.h"

// based on https://gist.github.com/profi200/bfa7be60b3eecb8c43f59000f626c743

#define TARGET_GAMMA  (2.f)
#define DISPLAY_GAMMA (1.6f)
#define DARKEN_SCREEN (0.5f)
#define LUMINANCE     (0.89f) // 0.99f for mGBA.

// https://stackoverflow.com/a/62699172
template <typename T, std::size_t N, typename Generator>
static constexpr std::array<T, N> makeArray(Generator fn)
{
    std::array<T, N> table = {};
    for (std::size_t i = 0; i != N; ++i) {
        table[i] = fn(i);
    }
    return table;
}

static constexpr u32 rgb8ToRgb5(u32 value8)
{
    u32 value5 = (value8 * 63 + 255) / (255 * 2);
    if (value5 > 31)
        return 31;
    return value5;
}

static constexpr u32 rgb8ToRgb6(u32 value8)
{
    return (value8 * 63 + 128) / 255;
}

static constexpr u16 calculateColor(const u16 i)
{
    double r = (double)(i & 0x1F) / 31;
    double g = (double)((i >> 5) & 0x1F) / 31;
    double b = (double)((i >> 10) & 0x1F) / 31;

    // Convert to linear gamma.
    r = std::pow(r, TARGET_GAMMA + DARKEN_SCREEN);
    g = std::pow(g, TARGET_GAMMA + DARKEN_SCREEN);
    b = std::pow(b, TARGET_GAMMA + DARKEN_SCREEN);

    // Luminance.
    r = std::clamp(r * LUMINANCE, 0.0, 1.0);
    g = std::clamp(g * LUMINANCE, 0.0, 1.0);
    b = std::clamp(b * LUMINANCE, 0.0, 1.0);

    /*
    *               Input
    *                [r]
    *                [g]
    *                [b]
    *
    * Correction    Output
    * [ r][gr][br]   [r]
    * [rg][ g][bg]   [g]
    * [rb][gb][ b]   [b]
    */

#if 1
    // AGB-001
    // double newR = 0.8 * r + 0.275 * g + -0.075 * b;
    // double newG = 0.135 * r + 0.64 * g + 0.225 * b;
    // double newB = 0.195 * r + 0.155 * g + 0.65 * b;

    // NDS-Phat-W
    double newR = 0.815 * r + 0.275 * g + -0.09 * b;
    double newG = 0.1 * r + 0.64 * g + 0.26 * b;
    double newB = 0.1075 * r + 0.1725 * g + 0.72 * b;

    // // NSO-GBA
    // double newR = 0.865 * r + 0.1225 * g + 0.0125 * b;
    // double newG = 0.0575 * r + 0.925 * g + 0.0125 * b;
    // double newB = 0.0575 * r + 0.1225 * g + 0.82 * b;

    // // PSP-Phat
    // double newR = 0.835 * r + 0.16 * g + -0.13 * b;
    // double newG = 0.0425 * r + 0.745 * g + 0.155 * b;
    // double newB = 0.0225 * r + 0.015 * g + 0.9625 * b;

    // // AGS_101
    // double newR = 0.955 * r + 0.11 * g + -0.065 * b;
    // double newG = 0.0375 * r + 0.885 * g + 0.0775 * b;
    // double newB = 0.0025 * r + -0.03 * g + 1.0275 * b;

    // // OLED_NSO
    // double newR = 1.54 * r + -0.48 * g + -0.06 * b;
    // double newG = -0.045 * r + 1.0625 * g + -0.0175 * b;
    // double newB = -0.0225 * r + -0.075 * g + 1.1025 * b;

    // // VBA
    // double newR = 0.73 * r + 0.27 * g + 0.0 * b;
    // double newG = 0.085 * r + 0.675 * g + 0.24 * b;
    // double newB = 0.085 * r + 0.24 * g + 0.675 * b;

    // // No$GBA
    // double newR = 0.73 * r + 0.27 * g + 0.0 * b;
    // double newG = 0.08 * r + 0.67 * g + 0.2 * b;
    // double newB = 0.08 * r + 0.2 * g + 0.67 * b;

    // mGBA.
    // double newR = 0.84 * r + 0.18 * g + 0.0 * b;
    // double newG = 0.09 * r + 0.67 * g + 0.26 * b;
    // double newB = 0.15 * r + 0.10 * g + 0.73 * b;
#else
    // mGBA.
    double newR = 0.84 * r + 0.18 * g + 0.0 * b;
    double newG = 0.09 * r + 0.67 * g + 0.26 * b;
    double newB = 0.15 * r + 0.10 * g + 0.73 * b;
#endif

    if (newR < 0)
        newR = 0;
    if (newG < 0)
        newG = 0;
    if (newB < 0)
        newB = 0;  

    // Convert to display gamma.
    newR = std::pow(newR, 1.0 / DISPLAY_GAMMA);
    newG = std::pow(newG, 1.0 / DISPLAY_GAMMA);
    newB = std::pow(newB, 1.0 / DISPLAY_GAMMA);

    // Denormalize, clamp and convert to RGB8.
    u32 outR = rgb8ToRgb5(std::clamp<int>(newR * 255, 0, 255));
    u32 outG = rgb8ToRgb6(std::clamp<int>(newG * 255, 0, 255));
    u32 outB = rgb8ToRgb5(std::clamp<int>(newB * 255, 0, 255));
    return (outB << 10) | ((outG >> 1) << 5) | (outR) | (outG << 15);
}

std::array<u16, COLOR_LUT_SIZE> gColorLut = makeArray<u16, COLOR_LUT_SIZE>(calculateColor);

void clut_disableColorCorrection()
{
    for (u32 i = 0; i < COLOR_LUT_SIZE; ++i)
    {
        gColorLut[i] = i;
    }
}
