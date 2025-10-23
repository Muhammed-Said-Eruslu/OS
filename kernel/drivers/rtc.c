#include "kernel.h"
#include "ports.h"
#include "drivers.h"

// ================================
// RTC (Real Time Clock) Driver
// ================================

// İstanbul için UTC+3 (isteğe göre 0 yapabilirsin)
#define RTC_TZ_OFFSET_HOURS  (+3)

// CMOS’tan veri okuma
static inline uint8_t cmos_read(uint8_t reg)
{
    // NMI kapatmadan da okunabilir, ama 0x80 | reg kararlılığı artırır
    outb(0x70, 0x80 | reg);
    return inb(0x71);
}

// Update in progress (UIP) kontrolü
static inline int is_update_in_progress(void)
{
    return (cmos_read(0x0A) & 0x80) != 0;   // Register A, bit7 = UIP
}

// BCD’yi normal sayıya çevirme
static inline uint8_t bcd_to_bin(uint8_t x)
{
    return (uint8_t)((x & 0x0F) + ((x >> 4) * 10));
}

// Güncel RTC değerlerini güvenli şekilde okur (UIP geçişinde sapma olmasın)
static void rtc_read_raw(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *reg_b)
{
    uint8_t last_s, last_m, last_h, rb;

    // Güncellenme bitinin sıfırlanmasını bekle
    while (is_update_in_progress()) {}

    last_s = cmos_read(0x00);
    last_m = cmos_read(0x02);
    last_h = cmos_read(0x04);
    rb     = cmos_read(0x0B);

    // Değerler sabitlenene kadar oku
    for (;;)
    {
        while (is_update_in_progress()) {}

        uint8_t s = cmos_read(0x00);
        uint8_t m = cmos_read(0x02);
        uint8_t h = cmos_read(0x04);
        uint8_t b = cmos_read(0x0B);

        if (s == last_s && m == last_m && h == last_h && b == rb)
        {
            *sec = s; *min = m; *hour = h; *reg_b = b;
            return;
        }

        last_s = s;
        last_m = m;
        last_h = h;
        rb     = b;
    }
}

// ================================
// Ana okuma fonksiyonu
// ================================
void rtc_read_time(uint8_t *out_h, uint8_t *out_m, uint8_t *out_s)
{
    uint8_t sec, min, hour, regb;
    rtc_read_raw(&sec, &min, &hour, &regb);

    // regB bit1: 1=binary, 0=BCD
    int is_binary = (regb & 0x04) != 0;
    // regB bit2: 1=24h, 0=12h
    int is_24h    = (regb & 0x02) != 0;

    if (!is_binary)
    {
        sec  = bcd_to_bin(sec);
        min  = bcd_to_bin(min);

        if (!is_24h)
        {
            uint8_t pm = hour & 0x80;
            hour = bcd_to_bin((uint8_t)(hour & 0x7F));
            if (pm)
            {
                if (hour != 12) hour = (uint8_t)(hour + 12);
            }
            else
            {
                if (hour == 12) hour = 0;
            }
        }
        else
        {
            hour = bcd_to_bin(hour);
        }
    }
    else
    {
        if (!is_24h)
        {
            uint8_t pm = hour & 0x80;
            hour = (uint8_t)(hour & 0x7F);
            if (pm)
            {
                if (hour != 12) hour = (uint8_t)(hour + 12);
            }
            else
            {
                if (hour == 12) hour = 0;
            }
        }
    }

    // Zaman dilimi ofseti uygula
    int h = (int)hour + RTC_TZ_OFFSET_HOURS;
    if (h < 0)      h += 24;
    else if (h > 23) h -= 24;

    *out_h = (uint8_t)h;
    *out_m = min;
    *out_s = sec;
}

// ================================
// RTC Başlatma (opsiyonel)
// ================================
void rtc_init(void)
{
    // RTC’ye özel bir donanım ayarı gerekmiyor — sadece bilgilendirme
    print("[RTC] Real Time Clock initialized\n", 0x0B);
}
