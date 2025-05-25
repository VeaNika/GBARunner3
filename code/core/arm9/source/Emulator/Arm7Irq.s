.section ".itcm", "ax"
.altmacro

#include "AsmMacros.inc"

arm_func emu_arm7Irq
    ldr sp,= dtcmIrqStackEnd
    push {r0-r3,r12}
#ifndef GBAR3_TEST
    mov r0, #1
    // This is replaced by a nop when not active
.global emu_soundDma1JumpInstruction
emu_soundDma1JumpInstruction:
    bl dma_dmaSound

    mov r0, #2
    // This is replaced by a nop when not active
.global emu_soundDma2JumpInstruction
emu_soundDma2JumpInstruction:
    bl dma_dmaSound
#endif
    pop {r0-r3,r12}
    b emu_arm7IrqReturn
