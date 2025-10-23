#include "keyboard.h"

// ======================================================
// 🧠 Klavye Durum Değişkenleri
// ======================================================
static int current_layout = LAYOUT_TR;
static int shift_pressed  = 0;
static int caps_lock      = 0;
static int altgr_pressed  = 0;
static int e0_prefix      = 0;
static uint8_t ctrl_pressed = 0;

// ======================================================
// 🧩 Ana Okuma Fonksiyonu (Ham Mod)
// ======================================================
char read_key(void)
{
    if (!kb_status_ready())
        return 0;
    if (kb_is_mouse_data()) { (void)kb_read(); return 0; }

    uint8_t sc = kb_read();

    // E0 öneki geldiyse flag ata
    if (sc == 0xE0) { e0_prefix = 1; return 0; }

    // Tuş bırakıldıysa (break code)
    if (sc & 0x80)
    {
        uint8_t code = sc & 0x7F;
        if (!e0_prefix) {
            if (code == SC_LSHIFT || code == SC_RSHIFT)
                shift_pressed = 0;
        } else if (code == SC_LALT)
            altgr_pressed = 0;

        e0_prefix = 0;
        return 0;
    }

    uint8_t code = sc;

    // Tuş basıldı
    if (!e0_prefix)
    {
        if (code == SC_LSHIFT || code == SC_RSHIFT) { shift_pressed = 1; return 0; }
        if (code == SC_CAPS)  { caps_lock ^= 1; return 0; }
        if (code == SC_BACKSP) return '\b';
        if (code == SC_ENTER)  return '\n';
        if (code == SC_SPACE)  return ' ';
        if (code == SC_TAB)    return '\t';
        if (code == SC_LALT)   return 0;
    }
    else
    {
        // Yön tuşları
        if (code == 0x48) { e0_prefix = 0; return 1; } // ↑
        if (code == 0x50) { e0_prefix = 0; return 2; } // ↓
        if (code == 0x4B) { e0_prefix = 0; return 3; } // ←
        if (code == 0x4D) { e0_prefix = 0; return 4; } // →
        if (code == 0x38) { altgr_pressed = 1; e0_prefix = 0; return 0; } // AltGr
        e0_prefix = 0;
        return 0;
    }

    // Aktif layout'u al
    const kb_layout_t *layout = get_keyboard_layout(current_layout);
    char c = 0;

    // AltGr → Shift → Normal sırası
    if (altgr_pressed && layout->altgr_map[code])
        c = layout->altgr_map[code];
    else if (shift_pressed && layout->shift_map[code])
        c = layout->shift_map[code];
    else
        c = layout->base_map[code];

    // Harflerde CapsLock etkisi
    if (c && caps_lock)
    {
        if (c >= 'a' && c <= 'z') c -= 32;
        else if (c >= 'A' && c <= 'Z') c += 32;
    }

    return c;
}

// ======================================================
// 🧩 Klavye Başlatma
// ======================================================
void keyboard_init(void)
{
    kb_set_layout_tr();
    print("[Keyboard] Initialized (TR Layout)\n", 0x0A);
}

// ======================================================
// Layout yönetimi
// ======================================================
void kb_set_layout_tr(void) { current_layout = LAYOUT_TR; }
void kb_set_layout_us(void) { current_layout = LAYOUT_US; }
int  kb_get_layout(void)    { return current_layout; }

void kb_toggle_layout(void)
{
    current_layout = (current_layout == LAYOUT_TR) ? LAYOUT_US : LAYOUT_TR;
    print("[Keyboard] Layout -> ", 0x0F);
    print(get_keyboard_layout(current_layout)->name, 0x0F);
    print("\n", 0x0F);
}

// ======================================================
// getch() — bloklayıcı karakter okuma
// ======================================================
char getch(void)
{
    char c = 0;
    while (c == 0)
        c = read_key();
    return c;
}

// ======================================================
// Donanımdan scancode okuma (Set-1)
// ======================================================
uint16_t read_scancode(void)
{
    uint8_t sc;

    while ((inb(0x64) & 1) == 0) { /* veri bekle */ }

    sc = inb(0x60);

    if (sc == 0xE0) {
        while ((inb(0x64) & 1) == 0) { /* ikinci byte */ }
        uint8_t ext = inb(0x60);
        return (uint16_t)(0xE000 | ext);
    }

    return (uint16_t)sc;
}

// ======================================================
// ASCII mod — sadece karakter tuşları
// ======================================================
char read_ascii(void)
{
    static uint8_t last_sc = 0;
    uint8_t sc = read_scancode();

    if (sc == last_sc)
        return 0;

    last_sc = sc;

    if (sc & 0x80) {
        last_sc = 0;
        return 0;
    }

    return scancode_to_ascii(sc);
}

// ======================================================
// Scancode → ASCII çevirimi
// ======================================================
char scancode_to_ascii(uint16_t scancode)
{
    // Modifiye tuş takibi
    if (scancode == 0x001D || scancode == 0xE01D) { ctrl_pressed = 1; return 0; }
    if (scancode == 0x009D || scancode == 0xE09D) { ctrl_pressed = 0; return 0; }
    if (scancode == 0x002A || scancode == 0x0036) { shift_pressed = 1; return 0; }
    if (scancode == 0x00AA || scancode == 0x00B6) { shift_pressed = 0; return 0; }
if (scancode == 0xE038) { 
    altgr_pressed = 1;
    return 0;
}

// Sağ Alt bırakıldı (E0 B8)
if (scancode == 0xE0B8) {
    altgr_pressed = 0;
    return 0;
}

// Bazı BIOS / QEMU sürümleri AltGr yerine LeftCtrl + LeftAlt yollarını kullanır
if (scancode == 0x001D) {  // Ctrl basıldı
    ctrl_pressed = 1;
    return 0;
}
if (scancode == 0x009D) {  // Ctrl bırakıldı
    ctrl_pressed = 0;
    altgr_pressed = 0;     // güvenlik: bazen aynı anda bırakılıyor
    return 0;
}
if (scancode == 0x0038 && ctrl_pressed) { // Ctrl basılıyken Alt basılırsa -> AltGr
    altgr_pressed = 1;
    return 0;
}
if (scancode == 0x00B8) { // Alt bırakıldıysa
    altgr_pressed = 0;
    return 0;
}

    if (scancode == 0x003A) { caps_lock = !caps_lock; return 0; }

    // Break kodlarını yok say
    if ((uint8_t)(scancode & 0xFF) & 0x80)
        return 0;

    uint8_t code = scancode & 0x7F;
    const kb_layout_t *layout = get_keyboard_layout(current_layout);
    char c = 0;

    // AltGr → Shift → Base
    if (altgr_pressed && layout->altgr_map[code])
        c = layout->altgr_map[code];
    else if (shift_pressed && layout->shift_map[code])
        c = layout->shift_map[code];
    else
        c = layout->base_map[code];

    if (!c) return 0;

    // CapsLock etkisi
    if (caps_lock && c >= 'a' && c <= 'z')
        c -= 32;
    else if (caps_lock && c >= 'A' && c <= 'Z')
        c += 32;

    // Ctrl + harf → kontrol karakteri
    if (ctrl_pressed && c >= 'a' && c <= 'z')
        c = (char)(c - 'a' + 1);

    return c;
}

// ======================================================
// Yardımcılar
// ======================================================
int is_ctrl_pressed(void) { return ctrl_pressed; }
