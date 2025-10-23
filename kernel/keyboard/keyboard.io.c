#include "keyboard.h"

uint8_t kb_status_ready(void) {
    return (inb(KEYBOARD_STATUS_PORT) & 1);
}

uint8_t kb_is_mouse_data(void) {
    return (inb(KEYBOARD_STATUS_PORT) & 0x20);
}

uint8_t kb_read(void) {
    return inb(KEYBOARD_DATA_PORT);
}
