#include "drivers.h"

void init_drivers(void)
{
    vga_init();
    keyboard_init();
    rtc_init();
    disk_init();
    cpu_init();
}
