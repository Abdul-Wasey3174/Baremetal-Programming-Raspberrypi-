// boot.S - AArch64 bare-metal entry point for Raspberry Pi 4
// Parks cores 1-3, sets up stack and BSS for core 0, then calls kmain()

.section ".text.boot"
.global _start

_start:
    // Read the CPU ID (MPIDR_EL1 bits [1:0] = core number)
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbnz    x1, park_core          // If not core 0, park indefinitely

    // Core 0: set up the stack pointer below the kernel load address
    ldr     x1, =_start
    mov     sp, x1

    // Zero out BSS section
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
zero_bss:
    cmp     x1, x2
    b.ge    bss_done
    str     xzr, [x1], #8
    b       zero_bss
bss_done:

    // Jump into C kernel
    bl      kmain

    // Should never reach here — halt core 0 too
park_core:
    wfe
    b       park_core
