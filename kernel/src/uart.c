#include "uart.h"

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static int uart_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void uart_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    outb(COM1 + 4, 0x1E);
    outb(COM1 + 0, 0xAE);

    if (inb(COM1 + 0) != 0xAE) {
        return;
    }

    outb(COM1 + 4, 0x0F);
}

void uart_putc(char c) {
    while (!uart_ready()) {
    }

    outb(COM1, (uint8_t)c);
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }

        uart_putc(*s++);
    }
}

void uart_put_hex(uint64_t value) {
    static const char digits[] = "0123456789ABCDEF";

    uart_puts("0x");

    for (int i = 15; i >= 0; i--) {
        uart_putc(digits[(value >> (i * 4)) & 0xF]);
    }
}

void uart_put_dec(uint64_t value) {
    char buffer[21];
    int i = 20;

    buffer[i] = '\0';

    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value > 0) {
        buffer[--i] = '0' + (value % 10);
        value /= 10;
    }

    uart_puts(&buffer[i]);
}
