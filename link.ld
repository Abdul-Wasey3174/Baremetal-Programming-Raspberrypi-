/*
 * link.ld - Linker script for Raspberry Pi 4 bare-metal kernel
 *
 * The GPU loads kernel8.img and jumps to physical address 0x80000.
 * We place .text.boot first so _start is at exactly that address.
 */

SECTIONS
{
    . = 0x80000;

    /* Entry point must come first */
    .text : {
        KEEP(*(.text.boot))
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata : { *(.rodata) *(.rodata.*) }

    /* Initialized data */
    .data : { *(.data) *(.data.*) }

    /* BSS - zero-initialized by boot.S */
    . = ALIGN(8);
    __bss_start = .;
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    . = ALIGN(8);
    __bss_end = .;
}
