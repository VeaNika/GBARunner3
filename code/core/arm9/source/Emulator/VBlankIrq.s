.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_vblankIrq
    // For center and mask display capture has to be enabled every frame
    // and the buffers need to be swapped
jumpToCaptureUpdate:
    nop

updateDisplayCaptureVramC:
    ldr lr,= 0x84808036
    mov r13, #0x04000000
    strh lr, [r13, #0x66]! // REG_DISPCAPCNT
    mov lr, lr, lsr #16
    add r13, r13, #(0x242 - 0x66)
    strh lr, [r13]
    ldr r13, jumpToUpdateDisplayCaptureVramDInstruction
    b checkSaveWrite

updateDisplayCaptureVramD:
    ldr lr,= 0x80848037
    mov r13, #0x04000000
    strh lr, [r13, #0x66]! // REG_DISPCAPCNT
    mov lr, lr, lsr #16
    add r13, r13, #(0x242 - 0x66)
    strh lr, [r13], -r13 // r13 becomes 0

checkSaveWrite:
    str r13, jumpToCaptureUpdate

    // This is replaced by a nop when no vblank dma is in use
.global emu_vblankDmaJumpInstruction
emu_vblankDmaJumpInstruction:
    b vblankDma

    // This is replaced by a nop when save needs to be checked
.global emu_vblankIrqSkipSaveCheckInstruction
emu_vblankIrqSkipSaveCheckInstruction:
    b emu_vblankIrqReturn
#ifndef GBAR3_TEST
    ldr r13,= gGbaSaveShared
    mcr p15, 0, r13, c7, c6, 1 // invalidate range
    ldrb lr, [r13]
    cmp lr, #3 // GBA_SAVE_STATE_WRITE
    bne emu_vblankIrqReturn

    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r12}
    bl sav_writeSaveToFile
    pop {r0-r3,r12}
    b emu_vblankIrqReturn
#endif

jumpToUpdateDisplayCaptureVramDInstruction:
    add pc, pc, #(updateDisplayCaptureVramD - jumpToCaptureUpdate - 8)

vblankDma:
    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r12}
#ifndef GBAR3_TEST
    // These are replaced by nops when not active
.global emu_vblankDmaJumpInstructions
emu_vblankDmaJumpInstructions:
    bl dma_dma0Transfer
    bl dma_dma1Transfer
    bl dma_dma2Transfer
    bl dma_dma3Transfer
#endif
    pop {r0-r3,r12}
    b emu_vblankIrqSkipSaveCheckInstruction

.pool
.end
