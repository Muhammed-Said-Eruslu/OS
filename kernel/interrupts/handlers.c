#include "kernel.h"
#include <stdint.h>

// CPU hataları (örneğin divide by zero, GPF vs)
void isr_handler(void)
{
    print("[INTERRUPT] CPU Exception!\n", 0x0C);
}

// Donanım kesmeleri (örneğin klavye, timer vs)
void irq_handler(void)
{
    uint8_t irq = inb(0x20); // PIC durumunu okuyabiliriz ama gerek yok
    // Timer (IRQ0)
    timer_handler();

    // EOI (End Of Interrupt)
    outb(0x20, 0x20);
}
