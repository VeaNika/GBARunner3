#pragma once

#include "common.h"
#include <algorithm>
#include <array>

class GammaLut {
public:
    static u16 encode(u16 value);
    static u16 decode(u16 value, int index);

private:
    static constexpr int TableSize = (1 << 8);
    static constexpr int GammaSteps = 5; // Min 1, Max 5. Default 5

    static constexpr float TargetGamma = 2.f; // Default 2.f
    static constexpr float DarkenScreen = 0.f; // Default 0.f

    // [DISPLAY_GAMMA] Display Gamma range can be modified from 0.f ~ 2.2f, 5 steps max
    static constexpr float GammaMin = 0.5f; // default 0.5f
    static constexpr float GammaMax = 0.9f; // default 0.9f
    static constexpr float GammaStep = (GammaMax - GammaMin) / (GammaSteps - 1);

    static const std::array<u16, TableSize> encodeTable;
    static const std::array<std::array<u16, TableSize>, GammaSteps> decodeTables;
};
