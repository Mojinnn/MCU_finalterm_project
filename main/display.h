#ifndef DISPLAY_H
#define DISPLAY_H

#include "main.h"
#include "oled.h"
#include "clock.h"
#include "wifi_manager.h"

// ===== GLOBAL CURRENT TIME (shared with wifi_manager) =====
extern rtc_time_t g_current_time;

// ===== FUNCTION DECLARATIONS =====
void display_init(void);
void display_start_task(void);
void display_time_only(rtc_time_t *time);
void display_date_only(rtc_time_t *time);
void display_pomodoro(pomodoro_t *pomodoro);
pomodoro_t* get_pomodoro(void);
void pomodoro_start_stop(void);
void pomodoro_reset(void);
void pomodoro_tick(void);


#endif