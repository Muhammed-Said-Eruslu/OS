#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

// IDT & IRQ setup
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void isr_install(void);
void irq_install(void);
void timer_install(void);

// IRQ/ISR handler
void timer_handler(void);
void isr_handler(void);
void irq_handler(void);
void timer_wait(unsigned int ticks);


// IRQ ASM stubs
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// Timer
uint32_t timer_get_ticks(void);

#endif
