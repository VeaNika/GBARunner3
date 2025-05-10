#include "common.h"
#include "GammaLut.h"
#include <array>
#include <algorithm>
#include <cmath>

constexpr int GAMMA_TABLE_SIZE = 256;
constexpr int GAMMA_STEPS = 5; // Min 1, Max 5. Default 5

constexpr float TARGET_GAMMA = 2.0; // Default 2.0
constexpr float DARKEN_SCREEN = 0; // Default 0.0

// [DISPLAY_GAMMA] Display Gamma range can be modified from 0.0f ~ 2.2f, 5 steps max
constexpr float GAMMA_MIN = 0.5; // default 0.5
constexpr float GAMMA_MAX = 0.9; // default 0.9
constexpr float GAMMA_STEP = (GAMMA_MAX - GAMMA_MIN) / (GAMMA_STEPS - 1);

// Lambda LUT generators based from this example https://stackoverflow.com/a/62699172

// Fixed gamma encode table [TARGET_GAMMA]
constexpr std::array<u8, GAMMA_TABLE_SIZE> gamma_encode_table = []
{
    std::array<u8, GAMMA_TABLE_SIZE> table = {};
    for (int i = 0; i < GAMMA_TABLE_SIZE; ++i) 
    {
        double x = static_cast<double>(i) / 255.0;
        table[i] = static_cast<u8>(std::clamp(std::pow(x, TARGET_GAMMA + DARKEN_SCREEN) * 255.0, 0.0, 255.0));
    }
    return table;
}();

// Precomputed gamma decode tables for gamma [DISPLAY_GAMMA]
constexpr std::array<std::array<u8, GAMMA_TABLE_SIZE>, GAMMA_STEPS> precomputed_decode_tables = []
{
    std::array<std::array<u8, GAMMA_TABLE_SIZE>, GAMMA_STEPS> tables = {};
    for (int g = 0; g < GAMMA_STEPS; ++g) 
    {
        double gamma = GAMMA_MIN + GAMMA_STEP * g;
        for (int i = 0; i < GAMMA_TABLE_SIZE; ++i) 
        {
            double x = static_cast<double>(i) / 255.0;
            tables[g][i] = static_cast<u8>(std::clamp(std::pow(x, gamma) * 255.0, 0.0, 255.0));
        }
    }
    return tables;
}();

// Runtime table pointer, default is Gamma Step 0 = 0.5
inline const u8* gamma_decode_table = precomputed_decode_tables[0].data();

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
u8 encodeGamma(u8 value)
{
    return gamma_encode_table[value];
}

u8 decodeGamma(u8 value)
{
    return gamma_decode_table[value];
}
