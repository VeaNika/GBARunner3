#pragma once

#include <nds/ndstypes.h>
#include "Core/Math/fixed.h"

struct ColorProfile
{
    fix32<12> matrix[3][3]; // 3x3 correction matrix
    fix32<12> luminance;    // Luminance factor
};

extern const ColorProfile Agb001;
extern const ColorProfile Ags101;
extern const ColorProfile Oxy001;
extern const ColorProfile Ntr001;
extern const ColorProfile Usg001;
extern const ColorProfile Psp01g;
extern const ColorProfile NswIps;
extern const ColorProfile NswOle;
extern const ColorProfile VbaEmu;
extern const ColorProfile NoCash;
extern const ColorProfile mGba01;

extern const ColorProfile* const colorProfileLut[];
