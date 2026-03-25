// uart.c - Mini UART (UART1) driver for Raspberry Pi 4 bare-metal
//
// The mini UART clock is derived from core_freq (set to 500 MHz in config.txt).
// Baud divisor = (core_freq / (8 * baud)) - 1 = (500000000 / (8 * 115200)) - 1 ≈ 541

#include "uart.h"
#include <stdint.h>

// ---- BCM2711 peripheral base -----------------------------------------------
#define MMIO_BASE       0xFE000000UL

// GPIO
#define GPIO_BASE       (MMIO_BASE + 0x200000)
#define GPFSEL1         (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPPUD           (*(volatile uint32_t *)(GPIO_BASE + 0x94))
#define GPPUDCLK0       (*(volatile uint32_t *)(GPIO_BASE + 0x98))

// GPIO Pull-up/down (BCM2711 new-style)
#define GPIO_PUP_PDN_CNTRL_REG0 (*(volatile uint32_t *)(GPIO_BASE + 0xE4))

// Auxiliary peripherals (mini UART = AUX UART1)
#define AUX_BASE        (MMIO_BASE + 0x215000)
#define AUX_ENABLES     (*(volatile uint32_t *)(AUX_BASE + 0x04))
#define AUX_MU_IO_REG   (*(volatile uint32_t *)(AUX_BASE + 0x40))
#define AUX_MU_IER_REG  (*(volatile uint32_t *)(AUX_BASE + 0x44))
#define AUX_MU_IIR_REG  (*(volatile uint32_t *)(AUX_BASE + 0x48))
#define AUX_MU_LCR_REG  (*(volatile uint32_t *)(AUX_BASE + 0x4C))
#define AUX_MU_MCR_REG  (*(volatile uint32_t *)(AUX_BASE + 0x50))
#define AUX_MU_LSR_REG  (*(volatile uint32_t *)(AUX_BASE + 0x54))
#define AUX_MU_CNTL_REG (*(volatile uint32_t *)(AUX_BASE + 0x60))
#define AUX_MU_BAUD_REG (*(volatile uint32_t *)(AUX_BASE + 0x68))

// Simple delay loop
static void delay(volatile int count)
{
    while (count-- > 0)
        __asm__ volatile("nop");
}

void uart_init(void)
{
    // 1. Enable mini UART
    AUX_ENABLES = 1;
    AUX_MU_CNTL_REG = 0;   // Disable TX/RX while configuring
    AUX_MU_IER_REG  = 0;   // No interrupts
    AUX_MU_IIR_REG  = 0xC6; // Clear FIFOs
    AUX_MU_LCR_REG  = 3;   // 8-bit mode
    AUX_MU_MCR_REG  = 0;
    // Baud rate = core_freq / (8 * (BAUD_REG + 1))
    // 500 000 000 / (8 * 115200) - 1 = 541
    AUX_MU_BAUD_REG = 541;

    // 2. Configure GPIO14 (TXD1) and GPIO15 (RXD1) to ALT5
    uint32_t r = GPFSEL1;
    r &= ~(7 << 12); // GPIO14: clear bits [14:12]
    r |=  (2 << 12); // ALT5
    r &= ~(7 << 15); // GPIO15: clear bits [17:15]
    r |=  (2 << 15); // ALT5
    GPFSEL1 = r;

    // 3. Disable pull-up/down on GPIO14 & GPIO15 (BCM2711 method)
    r = GPIO_PUP_PDN_CNTRL_REG0;
    r &= ~(3 << 28); // GPIO14 bits [29:28] = 0 (no pull)
    r &= ~(3 << 30); // GPIO15 bits [31:30] = 0 (no pull)
    GPIO_PUP_PDN_CNTRL_REG0 = r;

    delay(150);

    // 4. Enable TX and RX
    AUX_MU_CNTL_REG = 3;
}

void uart_putc(char c)
{
    // Wait until the TX FIFO has space (bit 5 of LSR)
    while (!(AUX_MU_LSR_REG & 0x20))
        __asm__ volatile("nop");
    AUX_MU_IO_REG = (uint32_t)c;
}

char uart_getc(void)
{
    // Wait until data is available (bit 0 of LSR)
    while (!(AUX_MU_LSR_REG & 0x01))
        __asm__ volatile("nop");
    return (char)(AUX_MU_IO_REG & 0xFF);
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

void uart_hex(unsigned long n)
{
    uart_puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        unsigned int d = (n >> i) & 0xF;
        uart_putc(d < 10 ? '0' + d : 'a' + d - 10);
    }
}
