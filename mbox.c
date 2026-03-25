// mbox.c - Mailbox interface for ARM <-> VideoCore communication on RPi4
//
// Follows the BCM2711 mailbox protocol (channel 8, property tags interface).

#include "mbox.h"
#include <stdint.h>

#define MMIO_BASE       0xFE000000UL

// Mailbox registers
#define MBOX_BASE       (MMIO_BASE + 0xB880)
#define MBOX_READ       (*(volatile uint32_t *)(MBOX_BASE + 0x00))
#define MBOX_STATUS     (*(volatile uint32_t *)(MBOX_BASE + 0x18))
#define MBOX_WRITE      (*(volatile uint32_t *)(MBOX_BASE + 0x20))

#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

// 16-byte aligned mailbox buffer (shared with GPU)
volatile uint32_t __attribute__((aligned(16))) mbox[36];

int mbox_call(uint8_t ch)
{
    // Bits [3:0] = channel, bits [31:4] = upper 28 bits of buffer address
    uint32_t r = (((uint32_t)(unsigned long)mbox) & ~0xF) | (ch & 0xF);

    // Wait until mailbox is not full
    while (MBOX_STATUS & MBOX_FULL)
        __asm__ volatile("nop");

    // Write the message address + channel
    MBOX_WRITE = r;

    // Wait for the response
    while (1) {
        while (MBOX_STATUS & MBOX_EMPTY)
            __asm__ volatile("nop");
        if (MBOX_READ == r)
            return (mbox[1] == 0x80000000) ? MBOX_SUCCESS : MBOX_ERROR;
    }
}
