#pragma once

#define GBA_SYS_CLOCK           16777216

#define GBA_CYCLES_PER_PIXEL    4

#define GBA_LCD_WIDTH           240
#define GBA_LCD_HBLANK          68
#define GBA_LCD_COLUMNS         (GBA_LCD_WIDTH + GBA_LCD_HBLANK)

#define GBA_LCD_HEIGHT          160
#define GBA_LCD_VBLANK          68
#define GBA_LCD_LINES           (GBA_LCD_HEIGHT + GBA_LCD_VBLANK)

#define GBA_CYCLES_PER_LINE     (GBA_LCD_COLUMNS * GBA_CYCLES_PER_PIXEL)
#define GBA_CYCLES_PER_FRAME    (GBA_LCD_COLUMNS * GBA_LCD_LINES * GBA_CYCLES_PER_PIXEL)