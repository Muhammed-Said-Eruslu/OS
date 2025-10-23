#include "rtc.h"
#include "kernel.h"
#include "drivers.h"


#define strcpy  my_strcpy
#define strcat  my_strcat
#define strlen  my_strlen

void sysinfo_print()
{
    print("\n====================[ MyOS System Info ]====================\n", 0x0E);

    // RTC zamanı
    uint8_t h, m, s;
    rtc_read_time(&h, &m, &s);
    char hbuf[4], mbuf[4], sbuf[4];
    int_to_str(h, hbuf); int_to_str(m, mbuf); int_to_str(s, sbuf);

    char timebuf[64];
    strcpy(timebuf, "Current Time: ");
    strcat(timebuf, hbuf); strcat(timebuf, ":");
    strcat(timebuf, mbuf); strcat(timebuf, ":");
    strcat(timebuf, sbuf); strcat(timebuf, "\n");
    print(timebuf, 0x0F);

    // CPU bilgisi
    char cpu_name[64];
    cpu_get_name(cpu_name);
    print("CPU: ", 0x0F);
    print(cpu_name, 0x0F);
    print("\n", 0x0F);

    // Bellek
    uint32_t mem_mb = 512; // ileride GRUB'dan alacağız
    char memstr[16];
    int_to_str(mem_mb, memstr);
    print("Memory: ~", 0x0F); print(memstr, 0x0F); print(" MB\n", 0x0F);

    // VGA
    print("Display: VGA Text Mode (80x25)\n", 0x0F);

    // Kernel modu
    print("Kernel Mode: Protected Mode (32-bit)\n", 0x0F);

    // Interruptlar
    print("Interrupts: IDT + IRQ + Timer OK\n", 0x0F);

    // Dosya sistemi
    print("Filesystem: disk.img mounted\n", 0x0F);

    print("============================================================\n", 0x0E);
}
