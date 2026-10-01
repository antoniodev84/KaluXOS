#include "irq.h"
#include "pic.h"
#include "pit.h"
#include "uart.h"

void irq_handler(uint64_t irq) {
    switch (irq) {
        case 0:
            pit_tick();

            if ((pit_get_ticks() % 100) == 0) {
                uart_puts("[ IRQ ] Timer tick: ");
                uart_put_dec(pit_get_ticks());
                uart_puts("\n");
            }
            break;

        default:
            break;
    }

    pic_send_eoi((uint8_t)irq);
}

void irq_init(void) {
    uart_puts("[ OK ] IRQ subsystem initialized\n");
}
