#include "common.h"
#include "ColorLut.h"
#include "GammaLut.h"

// Based on https://gist.github.com/profi200/bfa7be60b3eecb8c43f59000f626c743

u16 gColorLut[COLOR_LUT_SIZE] __attribute__((section(".lutram")));

const ColorProfile* gCurrentPreset = &Agb001; // Default is AGB_001

// When the 2d engine converts from 5 to 6 bit, the lsb bit will always be zero (i.e. 31 -> 62)
inline constexpr u32 rgb8ToRgb5(u32 value8)
{
    u32 value5 = (value8 * 63 + 255) / (255 * 2);
    if (value5 > 31)
        return 31;
    return value5;
}

// Convert RGB8 to RGB6 (for the 6-bit green)
inline constexpr u32 rgb8ToRgb6(u32 value8)
{
    return (value8 * 63 + 128) / 255;
}

// Simple and optimal clamping
inline constexpr u8 clamp255(int val) 
{
    return val < 0 ? 0 : (val > 255 ? 255 : val);
}

// Apply correction matrix from selected color profile
inline void applyColorMatrix(const fix32<12> matrix[3][3], fix32<12> r, fix32<12> g, fix32<12> b, fix32<12>& outR, fix32<12>& outG, fix32<12>& outB)
{
    fix32<12> newR = (matrix[0][0] * r + matrix[0][1] * g + matrix[0][2] * b);
    fix32<12> newG = (matrix[1][0] * r + matrix[1][1] * g + matrix[1][2] * b);
    fix32<12> newB = (matrix[2][0] * r + matrix[2][1] * g + matrix[2][2] * b);

    // We need to clamp specifically at this step or else color will get ruined
    outR = clamp255(newR.Int());
    outG = clamp255(newG.Int());
    outB = clamp255(newB.Int());
}

// Convert corrected RGB8 channels to RGB555 values (with the extra green bit)
inline constexpr u16 packToRGB5(fix32<12> r, fix32<12> g, fix32<12> b)
{
    u16 r5 = rgb8ToRgb5(r.Int());
    u16 g6 = rgb8ToRgb6(g.Int()); // 6-bit green
    u16 b5 = rgb8ToRgb5(b.Int());

    return (b5 << 10) | ((g6 >> 1) << 5) | r5 | (g6 << 15); // bit 15 = extra green bit
}

// Main Function
static u16 applyColorCorrection(const u16 rgb5)
{
    if (!gCurrentPreset) 
        return rgb5; // fallback

    // Extract RGB chanels
    fix32<12> r5 = (rgb5 & 0x1F);
    fix32<12> g5 = (rgb5 >> 5) & 0x1F;
    fix32<12> b5 = (rgb5 >> 10) & 0x1F;

    // Scale from 5 bits to 8 bits (0–255)
    fix32<12> r8 = (r5 * 255) / 31;
    fix32<12> g8 = (g5 * 255) / 31;
    fix32<12> b8 = (b5 * 255) / 31;

    // Convert to linear gamma (encode)
    fix32<12> rLin = encodeGamma(r8.Int());
    fix32<12> gLin = encodeGamma(g8.Int());
    fix32<12> bLin = encodeGamma(b8.Int());

    // Apply luminance
    rLin = (rLin * gCurrentPreset->luminance);
    gLin = (gLin * gCurrentPreset->luminance);
    bLin = (bLin * gCurrentPreset->luminance);

    // Apply color correction matrix
    fix32<12> outR, outG, outB;
    applyColorMatrix(gCurrentPreset->matrix, rLin, gLin, bLin, outR, outG, outB);

    // Convert to display gamma (decode).
    outR = decodeGamma(outR.Int());
    outG = decodeGamma(outG.Int());
    outB = decodeGamma(outB.Int());

    // Denormalize and convert to RGB8.
    return packToRGB5(outR, outG, outB);
}

// Generate LUT using current color preset and gamma
void clut_generateColorLut()
{
    for (u32 i = 0; i < COLOR_LUT_SIZE; ++i)
    {
        gColorLut[i] = applyColorCorrection(i);
    }
}

// To start color calculation using the selected color profile
void clut_initColorCorrection(const ColorProfile* preset)
{
    gCurrentPreset = preset;
    clut_generateColorLut();
}

void clut_disableColorCorrection()
{
    for (u32 i = 0; i < COLOR_LUT_SIZE; ++i)
    {
        gColorLut[i] = i;
    }
}
