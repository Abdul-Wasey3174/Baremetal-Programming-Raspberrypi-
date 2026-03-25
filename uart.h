#ifndef UART_H
#define UART_H

// Initialize the mini UART (UART1) at 115200 baud
void uart_init(void);

// Send a single character
void uart_putc(char c);

// Send a null-terminated string
void uart_puts(const char *s);

// Send an unsigned long as hex with '0x' prefix
void uart_hex(unsigned long n);

// Receive a single character (blocking)
char uart_getc(void);

#endif // UART_H
