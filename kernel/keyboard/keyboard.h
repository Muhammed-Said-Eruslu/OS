#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "../include/drivers.h"  // inb/outb, print, port erişimleri

// ============================================================
// 🧩 Klavye Layout Sabitleri
// ============================================================
#define LAYOUT_US 0
#define LAYOUT_TR 1

// ============================================================
// 🎹 Set-1 Scancode Sabitleri (yaygın tuşlar)
// ============================================================
#define SC_LSHIFT   0x2A
#define SC_RSHIFT   0x36
#define SC_CAPS     0x3A
#define SC_CTRL     0x1D
#define SC_CTRL_B   0x9D
#define SC_LALT     0x38
#define SC_ENTER    0x1C
#define SC_BACKSP   0x0E
#define SC_SPACE    0x39
#define SC_TAB      0x0F

// E0 önekiyle gelen özel tuşlar (ok tuşları vb.)
#define SC_UP_E0     0xE048
#define SC_DOWN_E0   0xE050
#define SC_LEFT_E0   0xE04B
#define SC_RIGHT_E0  0xE04D

// Sağ Alt (AltGr) tuşu — yalnızca E0 önekiyle gelir
#define SC_RALT_MAKE   0xE038
#define SC_RALT_BREAK  0xE0B8

// ============================================================
// 🧠 Donanım G/Ç Fonksiyonları (ps/2 controller)
// ============================================================
uint8_t kb_status_ready(void);
uint8_t kb_is_mouse_data(void);
uint8_t kb_read(void);

// ============================================================
// 🌍 Layout Yönetimi
// ============================================================
void kb_set_layout_tr(void);
void kb_set_layout_us(void);
int  kb_get_layout(void);
void kb_toggle_layout(void);
const char *kb_layout_name(void);

// ============================================================
// 🧩 Ana Klavye API Fonksiyonları
// ============================================================
void keyboard_init(void);
char read_key(void);
uint16_t read_scancode(void);
char scancode_to_ascii(uint16_t scancode);
int  is_ctrl_pressed(void);
char getch(void);
char read_ascii(void);

// ============================================================
// 🧱 Layout Yapısı Tanımı
// ============================================================
typedef struct s_kb_layout {
    const char *name;        // "US" veya "TR"
    char letter_map[128];    // Harf tablosu (opsiyonel)
    char base_map[128];      // Normal karakterler
    char shift_map[128];     // Shift kombinasyonları
    char altgr_map[128];     // AltGr kombinasyonları
} kb_layout_t;

// Aktif layout'u getiren fonksiyon
extern const kb_layout_t *get_keyboard_layout(int layout_id);

#endif // KEYBOARD_H
