#include "drivers.h"

void move_cursor(int row, int col);


#define VGA_CTRL 0x3D4
#define VGA_DATA 0x3D5

int cursor_row = 0;
int cursor_col = 0;

void clear_screen()
{
    char *video = (char*)VGA_ADDRESS;
    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0F;
    }
    cursor_row = 0;
    cursor_col = 0;
}

void scroll()
{
    char *video = (char*)VGA_ADDRESS;
    for (int y = 1; y < MAX_ROWS; y++) {
        for (int x = 0; x < MAX_COLS; x++) {
            int from = (y * MAX_COLS + x) * 2;
            int to = ((y - 1) * MAX_COLS + x) * 2;
            video[to] = video[from];
            video[to + 1] = video[from + 1];
        }
    }
    for (int x = 0; x < MAX_COLS; x++) {
        int pos = ((MAX_ROWS - 1) * MAX_COLS + x) * 2;
        video[pos] = ' ';
        video[pos + 1] = 0x0F;
    }
    cursor_row = MAX_ROWS - 1;
}

void put_char(char c, uint8_t color)
{
    char *video = (char*)VGA_ADDRESS;

    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            int pos = (cursor_row * MAX_COLS + cursor_col) * 2;
            video[pos] = ' ';
            video[pos + 1] = 0x0F;
        }
    } else {
        int pos = (cursor_row * MAX_COLS + cursor_col) * 2;
        video[pos] = c;
        video[pos + 1] = color;
        cursor_col++;
    }

    if (cursor_col >= MAX_COLS) {
        cursor_col = 0;
        cursor_row++;
    }

    if (cursor_row >= MAX_ROWS)
        scroll();
    move_cursor(cursor_row, cursor_col);
}

void print(const char *msg, uint8_t color)
{
    for (int i = 0; msg[i]; i++)
        put_char(msg[i], color);
}

void draw_char(int row, int col, char c, uint8_t attr) {
    uint8_t *vidmem = (uint8_t*)0xB8000;
    int offset = (row * 80 + col) * 2;
    vidmem[offset] = c;
    vidmem[offset + 1] = attr;
}

void move_cursor(int row, int col)
{
    uint16_t pos = row * MAX_COLS + col;
    outb(VGA_CTRL, 14);
    outb(VGA_DATA, (pos >> 8) & 0xFF);
    outb(VGA_CTRL, 15);
    outb(VGA_DATA, pos & 0xFF);
}

void vga_init(void)
{
    clear_screen();
    print("[VGA] Initialized (Text Mode 80x25)\n", 0x0A);
}

void draw_at(int row, int col, const char *str, uint8_t color)
{
    volatile unsigned char *vga = (unsigned char*)0xB8000;
    int pos = (row * 80 + col) * 2;

    for (int i = 0; str[i] && i < 80 - col; i++)
    {
        vga[pos++] = str[i];
        vga[pos++] = color;
    }
}
