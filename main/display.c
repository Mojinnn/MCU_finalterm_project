#include "display.h"
#include "buzzer.h"

static const char *TAG = "DISPLAY";

// Global variables
rtc_time_t g_current_time;

static pomodoro_t pomodoro = {
    .state = POMODORO_WORK,
    .is_running = false,
    .is_break = false,
    .time_left = POMODORO_WORK_DURATION,
};

static pomodoro_config_t pomo_config = {
    .work_duration = POMODORO_WORK_DURATION,
    .break_duration = POMODORO_BREAK_DURATION,
};



// Initialization
void display_init(void) {
    oled_init();
    oled_clear();
    ESP_LOGI(TAG, "Display initialized");
}

// Display functions
void display_time_only(rtc_time_t *time) {
    int add = 15;
    oled_draw_large_char(3, add, time->hours / 10 + '0');
    oled_draw_large_char(3, 16 + add, time->hours % 10 + '0');
    oled_draw_large_char(3, 28 + add, ':');
    oled_draw_large_char(3, 40 + add, time->minutes / 10 + '0');
    oled_draw_large_char(3, 52 + add, time->minutes % 10 + '0');
    oled_draw_large_char(3, 64 + add, ':');
    oled_draw_large_char(3, 76 + add, time->seconds / 10 + '0');
    oled_draw_large_char(3, 88 + add, time->seconds % 10 + '0');
}

void display_date_only(rtc_time_t *time) {
    int add = 25;
    oled_draw_large_char(2, add, time->date / 10 + '0');
    oled_draw_large_char(2, 16 + add, time->date % 10 + '0');
    oled_draw_large_char(2, 32 + add, '/');
    oled_draw_large_char(2, 48 + add, time->month / 10 + '0');
    oled_draw_large_char(2, 64 + add, time->month % 10 + '0');
    
    int offset = 30;
    uint16_t year = time->year;
    oled_draw_large_char(5, offset, (year / 1000) + '0');
    oled_draw_large_char(5, 16 + offset, ((year / 100) % 10) + '0');
    oled_draw_large_char(5, 32 + offset, ((year / 10) % 10) + '0');
    oled_draw_large_char(5, 48 + offset, (year % 10) + '0');
}

void display_pomodoro_fullscreen(pomodoro_t *pomo) {
    uint8_t minutes = pomo->time_left / 60;
    uint8_t seconds = pomo->time_left % 60;
    
    int offset = 25;
    
    oled_draw_large_char(3, offset, minutes / 10 + '0');
    oled_draw_large_char(3, 16 + offset, minutes % 10 + '0');
    oled_draw_large_char(3, 28 + offset, ':');
    oled_draw_large_char(3, 40 + offset, seconds / 10 + '0');
    oled_draw_large_char(3, 52 + offset, seconds % 10 + '0');
}

// Pomodoro control
pomodoro_t* get_pomodoro(void) {
    return &pomodoro;
}

pomodoro_config_t* pomodoro_get_config(void) {
    return &pomo_config;
}

esp_err_t pomodoro_set_durations(uint16_t work, uint16_t brk) {
    if (work == 0 || brk == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    pomo_config.work_duration = work;
    pomo_config.break_duration = brk;
    
    if (!pomodoro.is_running && !pomodoro.is_break) {
        if (pomodoro.state == POMODORO_WORK) {
            pomodoro.time_left = work;
        } else if (pomodoro.state == POMODORO_BREAK) {
            pomodoro.time_left = brk;
        }
    }
    
    ESP_LOGI(TAG, "Pomodoro durations updated: Work=%ds, Break=%ds", work, brk);
    return ESP_OK;
}



void pomodoro_start_stop(void) {
    pomodoro.is_running = true;
    pomodoro.is_break = false;
    buzzer_beep_5s();
    ESP_LOGI(TAG, "Pomodoro Started");
}

void pomodoro_reset(void) {
    pomodoro.is_running = false;
    pomodoro.is_break = false;
    pomodoro.state = POMODORO_WORK;
    pomodoro.time_left = pomo_config.work_duration;
    ESP_LOGI(TAG, "Pomodoro Reset");
}

void pomodoro_tick(void) {
    if (!pomodoro.is_running && !pomodoro.is_break) return;
    
    if (pomodoro.time_left > 0) {
        pomodoro.time_left--;
    } else {
        buzzer_beep_5s();
        
        if (pomodoro.state == POMODORO_WORK) {
            pomodoro.state = POMODORO_BREAK;
            pomodoro.time_left = pomo_config.break_duration;
            pomodoro.is_break = true;
            pomodoro.is_running = false;
        } else {
            pomodoro.state = POMODORO_WORK;
            pomodoro.time_left = pomo_config.work_duration;
            pomodoro.is_break = false;
            pomodoro.is_running = true;
        }
    }
}

// Display task
static void display_task(void *pvParameter) {
    bool show_time = true;
    uint8_t counter = 0;
    
    int wait_count = 0;
    while (!wifi_is_time_synced() && wait_count < 30) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d/30)", wait_count + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        wait_count++;
    }
    
    while (1) {
        oled_clear();
        
        if (clock_get_time(&g_current_time) == ESP_OK) {
            if (pomodoro.is_running || pomodoro.is_break ) {
                display_pomodoro_fullscreen(&pomodoro);
            } else {
            
                if (show_time) {
                    display_time_only(&g_current_time);
                } else {
                    display_date_only(&g_current_time);
                }
                
                counter++;
                if (counter >= 3) { 
                    counter = 0;
                    show_time = !show_time;
                }
            }
        }
        
        pomodoro_tick();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void display_start(void) {
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Display task started");
}