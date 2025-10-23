#ifndef RTC_H
#define RTC_H

#include <stdint.h>

// RTC modülünün dışa açık fonksiyonları
void rtc_init(void);
void rtc_read_time(uint8_t *out_h, uint8_t *out_m, uint8_t *out_s);

#endif
