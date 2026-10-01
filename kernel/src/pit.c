#include "pit.h"
#include "uart.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQUENCY 1193182

static volatile uint64_t ticks;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void pit_init(uint32_t frequency) {
    if (frequency == 0) {
        frequency = 100;
    }

    uint32_t divisor = PIT_FREQUENCY / frequency;

    if (divisor > 65535) {
        divisor = 65535;
    }

    if (divisor < 1) {
        divisor = 1;
    }

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xff);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xff);

    ticks = 0;

    uart_puts("[ OK ] PIT initialized at ");
    uart_put_dec(frequency);
    uart_puts(" Hz\n");
}

void pit_tick(void) {
    ticks++;
}

uint64_t pit_get_ticks(void) {
    return ticks;
}
