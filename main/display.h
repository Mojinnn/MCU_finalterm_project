#ifndef DISPLAY_H
#define DISPLAY_H

#include "main.h"
#include "oled.h"
#include "clock.h"
#include "wifi_manager.h"

extern rtc_time_t g_current_time;

void display_init(void);
void display_start(void);

void display_time_only(rtc_time_t *time);
void display_date_only(rtc_time_t *time);
void display_pomodoro_fullscreen(pomodoro_t *pomo);

pomodoro_t* get_pomodoro(void);
pomodoro_config_t* pomodoro_get_config(void);
esp_err_t pomodoro_set_durations(uint16_t work, uint16_t brk);
void pomodoro_start_stop(void);
void pomodoro_reset(void);
void pomodoro_tick(void);


#endif