#include "uart.h"
#include "gdt.h"
#include "idt.h"
#include "panic.h"

void kmain(void) {
    uart_init();

    uart_puts("\n");
    uart_puts("================================\n");
    uart_puts("             KaluXOS\n");
    uart_puts("================================\n");

    uart_puts("[ OK ] UART initialized\n");

    gdt_init();
    uart_puts("[ OK ] GDT initialized\n");

    idt_init();
    uart_puts("[ OK ] IDT initialized\n");

    __asm__ volatile ("sti");

    uart_puts("[ OK ] Interrupts enabled\n");
    uart_puts("[ OK ] Kernel initialized\n");
    uart_puts("\nKaluXOS is running.\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

