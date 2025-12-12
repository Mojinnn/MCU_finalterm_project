#ifndef CLOCK_H
#define CLOCK_H

#include "main.h"

// ------- Time struct -------
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

// ------- Pomodoro struct -------
typedef enum {
    POMODORO_IDLE,
    POMODORO_WORK,
    POMODORO_BREAK,
    POMODORO_LONG_BREAK
} pomodoro_state_t;

typedef struct {
    pomodoro_state_t state;
    bool  is_running;
    uint16_t time_left;
    uint8_t pomodoro_count;
} pomodoro_t;

static pomodoro_t pomodoro = {
    .state = POMODORO_WORK,
    .is_running = false,
    .time_left = POMODORO_WORK_DURATION,
    .pomodoro_count = 0
};
static rtc_time_t current_time;
// static httpd_handle_t server = NULL;
static bool time_synced = false;

esp_err_t i2c_master_init(void);
esp_err_t ds3231_get_time(rtc_time_t *time);
esp_err_t ds3231_set_time(rtc_time_t *time);


#endif CLOCK_H