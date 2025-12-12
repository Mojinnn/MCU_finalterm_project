#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include "main.h"
#include "oled.h"
#include "clock.h"

void oled_display_pomodoro(void);
void oled_display_time_only(rtc_time_t *time);
void oled_display_date_only(rtc_time_t *time);

#endif DISPLAY_OLED_H