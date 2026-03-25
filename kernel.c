// kernel.c - Bare-metal kernel entry point for Raspberry Pi 4
//
// Initializes mini UART, queries GPU via mailbox for memory layout
// and board revision, reads exception level, then prints boot info.

#include "uart.h"
#include "mbox.h"
#include <stdint.h>

// ---------- helpers ----------------------------------------------------------

static void print_uint(unsigned long n)
{
    char buf[21];
    int i = 0;
    if (n == 0) { uart_putc('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (--i >= 0) uart_putc(buf[i]);
}

// Read current exception level (bits [3:2] of CurrentEL)
static unsigned int get_el(void)
{
    unsigned long el;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(el));
    return (unsigned int)((el >> 2) & 3);
}

// ---------- mailbox query helpers --------------------------------------------

// Query a 2-value tag (base + size) — used for ARM mem and VC mem
static int query_mem(uint32_t tag, uint32_t *base_out, uint32_t *size_out)
{
    mbox[0]  = 8 * 4;           // total buffer size in bytes
    mbox[1]  = 0;               // request
    mbox[2]  = tag;             // tag
    mbox[3]  = 8;               // value buffer size
    mbox[4]  = 0;               // request indicator
    mbox[5]  = 0;               // value[0]: base address (filled by GPU)
    mbox[6]  = 0;               // value[1]: size         (filled by GPU)
    mbox[7]  = MBOX_TAG_LAST;
    mbox[0]  = 8 * 4;

    if (!mbox_call(MBOX_CH_PROP))
        return 0;

    *base_out = mbox[5];
    *size_out = mbox[6];
    return 1;
}

// Query board revision
static int query_board_rev(uint32_t *rev_out)
{
    mbox[0] = 7 * 4;
    mbox[1] = 0;
    mbox[2] = MBOX_TAG_GETBOARDREV;
    mbox[3] = 4;
    mbox[4] = 0;
    mbox[5] = 0;
    mbox[6] = MBOX_TAG_LAST;

    if (!mbox_call(MBOX_CH_PROP))
        return 0;

    *rev_out = mbox[5];
    return 1;
}

// ---------- kernel entry -----------------------------------------------------

void kmain(void)
{
    uart_init();

    uart_puts("\r\n");
    uart_puts("==================================================\r\n");
    uart_puts("  Bare-Metal Kernel - Raspberry Pi 4B\r\n");
    uart_puts("  CS5600 Operating Systems - Task 1\r\n");
    uart_puts("  Team: Abdul Wasey Mohammed, Ateeq\r\n");
    uart_puts("==================================================\r\n\r\n");

    // Exception level
    unsigned int el = get_el();
    uart_puts("[BOOT] Exception level : EL");
    uart_putc('0' + el);
    uart_puts("\r\n");

    // ARM memory region
    uint32_t arm_base = 0, arm_size = 0;
    if (query_mem(MBOX_TAG_GETARMMEM, &arm_base, &arm_size)) {
        uart_puts("[MEM ] ARM memory base  : ");
        uart_hex(arm_base);
        uart_puts("\r\n");
        uart_puts("[MEM ] ARM memory size  : ");
        uart_hex(arm_size);
        uart_puts(" (");
        print_uint(arm_size / (1024 * 1024));
        uart_puts(" MB)\r\n");
    } else {
        uart_puts("[MEM ] ARM memory query failed\r\n");
    }

    // VideoCore memory region
    uint32_t vc_base = 0, vc_size = 0;
    if (query_mem(MBOX_TAG_GETVCMEM, &vc_base, &vc_size)) {
        uart_puts("[MEM ] VC  memory base  : ");
        uart_hex(vc_base);
        uart_puts("\r\n");
        uart_puts("[MEM ] VC  memory size  : ");
        uart_hex(vc_size);
        uart_puts(" (");
        print_uint(vc_size / (1024 * 1024));
        uart_puts(" MB)\r\n");
    } else {
        uart_puts("[MEM ] VC  memory query failed\r\n");
    }

    // Board revision
    uint32_t rev = 0;
    if (query_board_rev(&rev)) {
        uart_puts("[BREV] Board revision   : ");
        uart_hex(rev);
        uart_puts("\r\n");
    } else {
        uart_puts("[BREV] Board revision query failed\r\n");
    }

    uart_puts("\r\n");
    uart_puts("hello from bare metal\r\n");
    uart_puts("\r\n[HALT] Kernel complete. Core 0 spinning.\r\n");

    // Spin forever
    while (1)
        __asm__ volatile("wfe");
}
