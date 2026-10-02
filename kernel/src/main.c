#include "uart.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "irq.h"
#include "pit.h"
#include "irq_idt.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "syscall.h"
#include "scheduler.h"

void kmain(void) {
    __asm__ volatile ("cli");

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

    pic_init();
    uart_puts("[ OK ] PIC initialized\n");

    irq_idt_install();

    irq_init();

    pmm_init();
    uart_puts("[ OK ] PMM initialized, limit: 4 GiB, free pages: ");
    uart_put_dec(pmm_free_pages());
    uart_puts("\n");

    vmm_init();
    uart_puts("[ OK ] VMM initialized, page size: 4 KiB\n");

    heap_init();
    uart_puts("[ OK ] HEAP initialized, limit: 64 MiB, alignment: 16 bytes\n");

    syscall_init();
    scheduler_init();

    pit_init(100);

    __asm__ volatile ("sti");

    uart_puts("[ OK ] Interrupts enabled\n");
    uart_puts("[ OK ] Kernel initialized\n");
    uart_puts("\nKaluXOS is running.\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
