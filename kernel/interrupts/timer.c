#include "kernel.h"
#include <stdint.h>

static volatile uint32_t tick = 0;  // Her clock interrupt'ında artacak sayaç

// PIT (Programmable Interval Timer) frekans ayarlama
void timer_phase(int hz)
{
    int divisor = 1193180 / hz;  // 1.193180 MHz / hz
    outb(0x43, 0x36);            // Komut portu (channel 0, rate generator)
    outb(0x40, divisor & 0xFF);  // Low byte
    outb(0x40, divisor >> 8);    // High byte
}

// IRQ0 handler — her clock interrupt'ında çağrılır
void timer_handler(void)
{
    tick++;

    // PIC'e End of Interrupt (EOI)
    outb(0x20, 0x20);
}

// 🔹 Sayaç değerini döndür
uint32_t timer_get_ticks(void)
{
    return tick;
}

// 🔹 Kurulum — IRQ0’ı IDT’ye ekler
void timer_install(void)
{
    print("[Timer] Installing...\n", 0x0B);

    // IRQ0 (timer interrupt) vektörünü bağla
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);

    // 100 Hz frekansında PIT ayarla
    timer_phase(100);

    print("[Timer] Initialized (100 Hz)\n", 0x0B);
}

// 🔹 Script dili (veya kernel) içinde bekleme fonksiyonu
void timer_wait(unsigned int ms)
{
    // 100 Hz olduğundan her tick ≈ 10 ms
    uint32_t start = tick;
    uint32_t target_ticks = ms / 10;

    while ((tick - start) < target_ticks)
        __asm__ __volatile__("hlt");
}
