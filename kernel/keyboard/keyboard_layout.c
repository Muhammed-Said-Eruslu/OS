#include "keyboard.h"

// ========================= US =========================
static const kb_layout_t layout_us = {
    .name = "US",
    .letter_map = {0}, // (unused)
    .base_map = {
        [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
        [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
        [0x0C] = '-', [0x0D] = '=',
        [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
        [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
        [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
        [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
        [0x27] = ';', [0x28] = '\'', [0x29] = '`',
        [0x2B] = '\\',
        [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
        [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
        [0x33] = ',', [0x34] = '.', [0x35] = '/',
        [0x56] = 0,    // US ANSI keyboard doesn't have this key (ISO extra key)
        [SC_SPACE] = ' ', [SC_TAB] = '\t'
    },
    .shift_map = {
        [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$', [0x06] = '%',
        [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(', [0x0B] = ')',
        [0x0C] = '_', [0x0D] = '+',
        [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R', [0x14] = 'T',
        [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
        [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
        [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
        [0x27] = ':', [0x28] = '\"', [0x29] = '~',
        [0x2B] = '|',
        [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
        [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',
        [0x33] = '<', [0x34] = '>', [0x35] = '?'
    },
    .altgr_map = {0}  // US layout has no AltGr mappings
};

// ========================= TR-Q (Turkish QWERTY) =========================
// Turkish-specific letters are mapped using their ISO-8859-9 codes (Latin-5), 
// since they are outside standard ASCII. This ensures keys like Ş, Ğ, İ, etc., output correctly.
static const kb_layout_t layout_tr = {
    .name = "TR",
    .letter_map = {0},  // not used in this context
    .base_map = {
        // Number row (no modifier)
        [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
        [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
        [0x0C] = '*',    // VK_OEM_8 in TR (prints '*')
        [0x0D] = '-',    // VK_OEM_MINUS in TR

        // Top letter row (Q, W, E, R, T, Y, U, I, O, P, Ğ, Ü)
        [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
        [0x15] = 'y', [0x16] = 'u', 
        [0x17] = '\xFD', // ı (dotless i)
        [0x18] = 'o', [0x19] = 'p',
        [0x1A] = '\xF0', // ğ 
        [0x1B] = '\xFC', // ü

        // Home row (A, S, D, F, G, H, J, K, L, Ş, İ)
        [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
        [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
        [0x27] = '\xFE', // ş 
        [0x28] = 'i',    // i (dotted i)

        // Bottom row (Z, X, C, V, B, N, M, Ö, Ç, .) plus extra key
        [0x56] = '<',    // ISO extra key (prints '<')
        [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
        [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
        [0x33] = '\xF6', // ö 
        [0x34] = '.',    // . (period) 
        [0x35] = '\xE7', // ç 

        // Miscellaneous keys
        [0x29] = '\"',   // VK_OEM_3 (the key under ESC) prints double-quote (")
        [0x2B] = ',',    // VK_OEM_COMMA key prints comma (,)
        [SC_SPACE] = ' ', [SC_TAB] = '\t'
    },
    .shift_map = {
        // Number row (with Shift)
        [0x02] = '!', [0x03] = '\'', [0x04] = '^', [0x05] = '+', [0x06] = '%',
        [0x07] = '&', [0x08] = '/', [0x09] = '(', [0x0A] = ')', [0x0B] = '=',
        [0x0C] = '?',    // Shift + '*' (VK_OEM_8) -> '?'
        [0x0D] = '_',    // Shift + '-' -> '_'

        // Top letter row (Shifted: Q, W, E, R, T, Y, U, I, O, P, Ğ, Ü)
        [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R', [0x14] = 'T',
        [0x15] = 'Y', [0x16] = 'U', 
        [0x17] = 'I',      // I (capital dotless I)
        [0x18] = 'O', [0x19] = 'P',
        [0x1A] = '\xD0', // Ğ 
        [0x1B] = '\xDC', // Ü

        // Home row (Shifted: A, S, D, F, G, H, J, K, L, Ş, İ)
        [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
        [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
        [0x27] = '\xDE', // Ş 
        [0x28] = '\xDD', // İ 

        // Bottom row & extra (Shifted: < >, Ö, Ç, etc.)
        [0x2B] = ';',    // Shift + comma key -> ';'
        [0x33] = '\xD6', // Ö 
        [0x34] = ':',    // Shift + '.' -> ':'
        [0x35] = '\xC7', // Ç 
        [0x56] = '>'     // Shift + extra key -> '>'
    },
    .altgr_map = {
        // Number row (AltGr combinations)
        [0x02] = '>',        // AltGr+1 -> '>'
        [0x04] = '#',        // AltGr+3 -> '#'
        [0x05] = '$',        // AltGr+4 -> '$'
        // [0x06] = ½ (AltGr+5 -> 1/2 symbol, not ASCII, omitted)
        [0x07] = '{',        // AltGr+7 -> '{'
        [0x08] = '[',        // AltGr+8 -> '['
        [0x09] = ']',        // AltGr+9 -> ']'
        [0x0A] = '}',        // AltGr+0 -> '}'
        [0x0C] = '\\',       // AltGr+* -> backslash '\'
        [0x0D] = '|',        // AltGr+- -> vertical bar '|'

        // Letter keys (AltGr)
        [0x10] = '@',        // AltGr+Q -> '@'
        // [0x11] = '€',     // AltGr+W -> '€' (Euro sign, not in Latin-5; omitted)
        // [0x14] = '₺',     // AltGr+T -> '₺' (Turkish Lira sign, requires Unicode)
        // [0x12] = '~',    // AltGr+E -> '~' (In official TR-Q, AltGr+E is €; we use ~ on another key)
        [0x1B] = '~',        // AltGr+']' -> '~'  (Tilde on the Ü key with AltGr)
        [0x29] = '<',        // AltGr+key under ESC -> '<'
        [0x2B] = '`',        // AltGr+comma key -> backtick '`'
        [0x56] = '|'         // AltGr+ISO extra key (OEM_102) -> '|'
    }
};

const kb_layout_t *keyboard_layouts[] = { &layout_us, &layout_tr };

const kb_layout_t *get_keyboard_layout(int layout_id)
{
    if (layout_id < 0 || layout_id > 1) {
        // Default to Turkish layout if out of range
        return keyboard_layouts[LAYOUT_TR];
    }
    return keyboard_layouts[layout_id];
}
