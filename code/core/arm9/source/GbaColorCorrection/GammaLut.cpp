#include "common.h"
#include "GammaLut.h"
#include <array>
#include <algorithm>
#include <cmath>

#define GAMMA_TABLE_SIZE     (1 << 8)
#define GAMMA_STEPS          5 // Min 1, Max 5. Default 5

#define TARGET_GAMMA         2.f // Default 2.2f
#define DARKEN_SCREEN        0.5f // Default 0.5f

// [DISPLAY_GAMMA] Display Gamma range can be modified from 0.0f ~ 2.2f, 5 steps max
#define GAMMA_MIN            0.5f // default 0.5f
#define GAMMA_MAX            0.9f // default 0.9f
#define GAMMA_STEP           (GAMMA_MAX - GAMMA_MIN) / (GAMMA_STEPS - 1)

// Lambda LUT generators based from this example https://stackoverflow.com/a/62699172

// Fixed gamma encode table [TARGET_GAMMA]
const std::array<u8, GAMMA_TABLE_SIZE> gamma_encode_table = []
{
    std::array<u8, GAMMA_TABLE_SIZE> table = {};
    for (int i = 0; i < GAMMA_TABLE_SIZE; ++i)
    {
        float x = static_cast<float>(i) / 255.0f;
        table[i] = static_cast<u8>(std::clamp(std::pow(x, TARGET_GAMMA + DARKEN_SCREEN) * 255.0f, 0.0f, 255.0f));
    }
    return table;
}();

// Precomputed gamma decode tables for gamma [DISPLAY_GAMMA]
const std::array<std::array<u8, GAMMA_TABLE_SIZE>, GAMMA_STEPS> precomputed_decode_tables = []
{
    std::array<std::array<u8, GAMMA_TABLE_SIZE>, GAMMA_STEPS> tables = {};
    for (int g = 0; g < GAMMA_STEPS; ++g)
    {
        float gamma = GAMMA_MIN + GAMMA_STEP * g;
        for (int i = 0; i < GAMMA_TABLE_SIZE; ++i)
        {
            float x = static_cast<float>(i) / 255.0f;
            tables[g][i] = static_cast<u8>(std::clamp(std::pow(x, gamma) * 255.0f, 0.0f, 255.0f));
        }
    }
    return tables;
}();

// Runtime table pointer, default is Gamma Step 0 = 0.5
inline const u8* gamma_decode_table = precomputed_decode_tables[0].data(); // default to gamma 0.5 (index 2)

// Assign gamma from precomputed gamma decode tables:
// Index 0 = gamma 0.5f,
// Index 1 = gamma 0.6f,
// Index 2 = gamma 0.7f,
// Index 3 = gamma 0.8f,
// Index 4 = gamma 0.9f
// Default 0
void setDisplayGammaIndex(int index) 
{
    if (index < 0) index = 0;
    if (index >= GAMMA_STEPS) index = GAMMA_STEPS - 1;
    gamma_decode_table = precomputed_decode_tables[index].data();
}

// Apply gamma from the set LUT. 
// True for encode, false for decode.
u8 applyGamma(u8 value, bool encode)
{
    return encode ? gamma_encode_table[value] : gamma_decode_table[value];
}
