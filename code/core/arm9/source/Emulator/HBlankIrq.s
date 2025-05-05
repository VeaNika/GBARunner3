.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

.extern dma_state

/// @brief Performs hblank irq tasks for the emulator.
///        - Emulates HDMA
/// @param r0-r12 Preserved
/// @param r13 Trashed.
/// @param lr Return address.
arm_func emu_hblankIrq
    // Don't tigger hblank irq on scanlines beyond the gba screen
    mov r13, #0x04000000
    ldrh lr, [r13, #6]
    cmp lr, #260
    sublo r13, lr, #160
    rsblos r13, r13, #31
    bichs r4, r4, #2 // HBLANK IRQ

    cmp lr, #160
    // This is replaced by a b instruction when no hblank dma is in use
.global emu_hblankDmaSkipInstruction
emu_hblankDmaSkipInstruction:
    bge emu_hblankIrqReturn

    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r12}
#ifndef GBAR3_TEST
    // These are replaced by nops when not active
.global emu_hblankDmaJumpInstructions
emu_hblankDmaJumpInstructions:
    bl dma_dma0Transfer
    bl dma_dma1Transfer
    bl dma_dma2Transfer
    bl dma_dma3Transfer
#endif
    pop {r0-r3,r12}
    b emu_hblankIrqReturn
