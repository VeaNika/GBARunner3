#include "common.h"
#include "ColorLut.h"
#include "GammaLut.h"

// Based on https://gist.github.com/profi200/bfa7be60b3eecb8c43f59000f626c743

u16 gColorLut[COLOR_LUT_SIZE]; // Dinamically generated color LUT

const ColorProfile* gCurrentPreset = &Agb001; // Initialize the color matrix preset, default is AGB_001

// Scale from 5 bits to 8 bits (0–255)
inline u8 rgb5ToRgb8(u8 val5)
{
    return (val5 * 255) / 31;
}

// Convert RGB8 to RGB5
inline u32 rgb8ToRgb5(u32 value8)
{
    return (value8 * 31 + 127) / 255; // More precise and commonly used for rgb8 → rgb5
}

// Convert RGB8 to RGB6 (for the 6-bit green)
inline u32 rgb8ToRgb6(u32 value8)
{
    return (value8 * 63 + 128) / 255;
}

// Simple and optimal clamping
inline u8 clamp255(int val) 
{
    return val < 0 
    ? 0 : (val > 255 ? 255 : val);
}

// Apply Luminance from the selected color profile (0–100 values)
inline u8 applyLuminance(u8 val, int luminance)
{
    return clamp255((val * luminance) / 100);
}

// Apply correction matrix from selected color profile
inline void applyColorMatrix(const int matrix[3][3], u8 r, u8 g, u8 b, u8& outR, u8& outG, u8& outB)
{
    // Assuming no alpha channel in original calculation.
    int newR = (matrix[0][0] * r + matrix[0][1] * g + matrix[0][2] * b) / 1000;
    int newG = (matrix[1][0] * r + matrix[1][1] * g + matrix[1][2] * b) / 1000;
    int newB = (matrix[2][0] * r + matrix[2][1] * g + matrix[2][2] * b) / 1000;

    outR = clamp255(newR);
    outG = clamp255(newG);
    outB = clamp255(newB);
}

// Convert corrected RGB8 channels to RGB555 values (with the extra green bit)
inline u16 packToRGB5(u8 r, u8 g, u8 b)
{
    u16 r5 = rgb8ToRgb5(r);
    u16 g6 = rgb8ToRgb6(g); // 6-bit green
    u16 b5 = rgb8ToRgb5(b);

    return (b5 << 10) | ((g6 >> 1) << 5) | r5 | (g6 << 15); // bit 15 = extra green bit
}

// Main Function
static u16 applyColorCorrection(const u16 rgb5)
{
    if (!gCurrentPreset) 
        return rgb5; // fallback

    // Extract RGB chanels
    u8 r5, g5, b5;
    r5 = (rgb5 & 0x1F);
    g5 = (rgb5 >> 5) & 0x1F;
    b5 = (rgb5 >> 10) & 0x1F;

    u8 r8 = rgb5ToRgb8(r5);
    u8 g8 = rgb5ToRgb8(g5);
    u8 b8 = rgb5ToRgb8(b5);

    // Convert to linear gamma (encode)
    u8 rLin = encodeGamma(r8);
    u8 gLin = encodeGamma(g8);
    u8 bLin = encodeGamma(b8);

    // Apply luminance
    int luminance = gCurrentPreset->luminance;
    rLin = applyLuminance(rLin, luminance);
    gLin = applyLuminance(gLin, luminance);
    bLin = applyLuminance(bLin, luminance);

    // Apply color correction
    u8 outR, outG, outB;
    applyColorMatrix(gCurrentPreset->matrix, rLin, gLin, bLin, outR, outG, outB);

    // Convert to display gamma (decode).
    outR = decodeGamma(outR);
    outG = decodeGamma(outG);
    outB = decodeGamma(outB);

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
