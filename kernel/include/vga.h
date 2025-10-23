#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include "drivers.h"

#define VGA_ADDRESS 0xB8000
#define MAX_COLS 80
#define MAX_ROWS 25

// Renk kodları (isteğe bağlı)
#define VGA_COLOR_BLACK         0x00
#define VGA_COLOR_BLUE          0x01
#define VGA_COLOR_GREEN         0x02
#define VGA_COLOR_CYAN          0x03
#define VGA_COLOR_RED           0x04
#define VGA_COLOR_MAGENTA       0x05
#define VGA_COLOR_BROWN         0x06
#define VGA_COLOR_LIGHT_GRAY    0x07
#define VGA_COLOR_DARK_GRAY     0x08
#define VGA_COLOR_LIGHT_BLUE    0x09
#define VGA_COLOR_LIGHT_GREEN   0x0A
#define VGA_COLOR_LIGHT_CYAN    0x0B
#define VGA_COLOR_LIGHT_RED     0x0C
#define VGA_COLOR_LIGHT_MAGENTA 0x0D
#define VGA_COLOR_YELLOW        0x0E
#define VGA_COLOR_WHITE         0x0F

// Fonksiyon prototipleri
void clear_screen(void);
void scroll(void);
void put_char(char c, uint8_t color);
void print(const char *msg, uint8_t color);
void move_cursor(int row, int col);
void vga_init(void);

extern int cursor_row;
extern int cursor_col;
void draw_at(int row, int col, const char *str, uint8_t color);

#endif
