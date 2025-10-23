#include "lvgl.h"
#include "lv_drivers/sdl/sdl.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

/* --- Buton tıklama olayı --- */
static void open_terminal_event(lv_event_t * e)
{
    system("make run");   /* Tıklandığında MyOS başlat (QEMU) */
}

int main(void)
{
    /* LVGL başlat */
    lv_init();

    /* SDL penceresi oluştur (LVGL 9 sürümü) */
    lv_display_t * disp = lv_sdl_window_create(480, 320);
    if (!disp) {
        printf("SDL ekran oluşturulamadı!\n");
        return -1;
    }

    /* Basit tema ayarı (isteğe bağlı) */
    lv_theme_t * th = lv_theme_default_init(disp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        false, LV_FONT_DEFAULT);
    lv_display_set_theme(disp, th);

    /* Ekran oluştur */
    lv_obj_t * screen = lv_screen_active();

    /* Buton oluştur */
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_center(btn);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Open Terminal");

    /* Butona olay (callback) ekle */
    lv_obj_add_event_cb(btn, open_terminal_event, LV_EVENT_CLICKED, NULL);

    /* Ana döngü */
    while (1) {
        lv_timer_handler();
        SDL_Delay(5);
    }

    return 0;
}
