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

    ldr r12,= gGbaSioShared + 0xA
    mov r4, #0
    mcr p15, 0, r4, c7, c10, 4 // drain write buffer
    mcr p15, 0, r12, c7, c6, 1 // invalidate range
    swpb lr, r4, [r12]
    cmp lr, #0
        blne sio_finishTransfer
#endif
    pop {r0-r3,r12}
    b emu_arm7IrqReturn
