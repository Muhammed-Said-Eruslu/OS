#include "kernel.h"
#include <stdint.h>

static uint32_t tick = 0;  // Her clock interrupt'ında artacak sayaç

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

    // Her 100 tick’te bir ekrana debug mesajı (isteğe bağlı)
    // if (tick % 100 == 0)
    // {
    //     print("[Timer] Tick: ", 0x0A);
    //     char buf[16];
    //     int_to_str(tick, buf);
    //     print(buf, 0x0A);
    //     print("\n", 0x0A);
    // }

    // PIC'e End of Interrupt (EOI)
    outb(0x20, 0x20);
}

// 🔹 Bu fonksiyon terminal komutları tarafından çağrılabilir
uint32_t timer_get_ticks(void)
{
    return tick;
}

// Kurulum — IRQ0’ı IDT’ye ekler
void timer_install(void)
{
    print("[Timer] Installing...\n", 0x0B);

    // IRQ0 (timer interrupt) vektörünü bağla
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);

    // 100 Hz frekansında PIT ayarla
    timer_phase(100);

    print("[Timer] Initialized (100 Hz)\n", 0x0B);
}
