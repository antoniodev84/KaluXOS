#include "kernel_init.h"
#include "uart.h"
#include "gdt.h"
#include "idt.h"

void kernel_init(void) {
    uart_init();
    uart_puts("KaluXOS\n");
    uart_puts("Initializing GDT...\n");

    gdt_init();

    uart_puts("Initializing IDT...\n");

    idt_init();

    uart_puts("GDT: OK\n");
    uart_puts("IDT: OK\n");
    uart_puts("UART: OK\n");
    uart_puts("KaluXOS kernel initialized.\n");

    __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
