#ifndef DRIVERS_H
#define DRIVERS_H

#include "types.h"

// ============================
// VGA
// ============================
#define VGA_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

void print(const char *msg, uint8_t color);
void clear_screen(void);
void scroll(void);
void put_char(char c, uint8_t color);

// ============================
// Keyboard
// ============================
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

char read_key(void);
void kb_set_layout_tr(void);
void kb_set_layout_us(void);

// ============================
// Disk
// ============================
#define MAX_SECTORS 1024
#define SECTOR_SIZE 512

void disk_init(void);
void disk_write_sector(int sector, const uint8_t *data);
void disk_read_sector(int sector, uint8_t *out);

// ============================
// RTC (Real Time Clock)
// ============================
void rtc_read_time(uint8_t *hour, uint8_t *min, uint8_t *sec);

// ============================
// Ports (I/O erişimi)
// ============================
static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val)
{
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void insw(unsigned short port, void *addr, unsigned long count)
{
    __asm__ __volatile__("rep insw"
                         : "+D"(addr), "+c"(count)
                         : "d"(port)
                         : "memory");
}

static inline void outsw(unsigned short port, const void *addr, unsigned long count)
{
    __asm__ __volatile__("rep outsw"
                         : "+S"(addr), "+c"(count)
                         : "d"(port));
}

// ============================
// Mouse
// ============================
void mouse_init(void);
int mouse_poll(int *dx, int *dy, unsigned char *buttons);

// ============================
// UI (grafiksel arayüz)
// ============================
void ui_draw_desktop(void);
void ui_loop(void);


void init_drivers(void);

void vga_init(void);
void keyboard_init(void);
void rtc_init(void);
void disk_init(void);
void cpu_init(void);

#endif
