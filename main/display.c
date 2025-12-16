#include "display.h"

static const char *TAG = "DISPLAY";

// ===== GLOBAL VARIABLES =====
rtc_time_t g_current_time;  // Shared with wifi_manager

static pomodoro_t pomodoro = {
    .state = POMODORO_WORK,
    .is_running = false,
    .time_left = POMODORO_WORK_DURATION,
    .pomodoro_count = 0
};

// ===== INITIALIZATION =====
void display_init(void) {
    oled_init();
    oled_clear();
    ESP_LOGI(TAG, "Display initialized");
}

// ===== DISPLAY FUNCTIONS =====
void display_time_only(rtc_time_t *time) {
    int add = 15;
    oled_draw_large_char(2, add, time->hours / 10 + '0');
    oled_draw_large_char(2, 16 + add, time->hours % 10 + '0');
    oled_draw_large_char(2, 28 + add, ':');
    oled_draw_large_char(2, 40 + add, time->minutes / 10 + '0');
    oled_draw_large_char(2, 52 + add, time->minutes % 10 + '0');
    oled_draw_large_char(2, 64 + add, ':');
    oled_draw_large_char(2, 76 + add, time->seconds / 10 + '0');
    oled_draw_large_char(2, 88 + add, time->seconds % 10 + '0');
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

void display_pomodoro(pomodoro_t *pomo) {
    uint8_t minutes = pomo->time_left / 60;
    uint8_t seconds = pomo->time_left % 60;
    
    int offset = 30;
    
    oled_draw_small_char(5, offset, minutes / 10 + '0');
    oled_draw_small_char(5, offset + 8, minutes % 10 + '0');
    oled_draw_small_char(5, offset + 16, ':');
    oled_draw_small_char(5, offset + 24, seconds / 10 + '0');
    oled_draw_small_char(5, offset + 32, seconds % 10 + '0');
    
    oled_write_cmd(0xB0 + 5);
    oled_write_cmd(0x00 + ((offset + 45) & 0x0F));
    oled_write_cmd(0x10 + ((offset + 45) >> 4));
    
    if (pomo->state == POMODORO_WORK) {
        uint8_t w[] = {0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00};
        oled_write_data(w, 6);
    } else if (pomo->state == POMODORO_BREAK || pomo->state == POMODORO_LONG_BREAK) {
        uint8_t b[] = {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00};
        oled_write_data(b, 6);
    }
    
    if (pomo->pomodoro_count < 10) {
        oled_draw_small_char(5, offset + 55, pomo->pomodoro_count + '0');
    }
}

// ===== POMODORO CONTROL =====
pomodoro_t* get_pomodoro(void) {
    return &pomodoro;
}

void pomodoro_start_stop(void) {
    pomodoro.is_running = !pomodoro.is_running;
    ESP_LOGI(TAG, "Pomodoro %s", pomodoro.is_running ? "Started" : "Paused");
}

void pomodoro_reset(void) {
    pomodoro.is_running = false;
    pomodoro.state = POMODORO_WORK;
    pomodoro.time_left = POMODORO_WORK_DURATION;
    ESP_LOGI(TAG, "Pomodoro Reset");
}

void pomodoro_tick(void) {
    if (!pomodoro.is_running) return;
    
    if (pomodoro.time_left > 0) {
        pomodoro.time_left--;
    } else {
        if (pomodoro.state == POMODORO_WORK) {
            pomodoro.pomodoro_count++;
            
            if (pomodoro.pomodoro_count % 4 == 0) {
                pomodoro.state = POMODORO_LONG_BREAK;
                pomodoro.time_left = POMODORO_LONG_BREAK_DURATION;
            } else {
                pomodoro.state = POMODORO_BREAK;
                pomodoro.time_left = POMODORO_BREAK_DURATION;
            }
        } else {
            pomodoro.state = POMODORO_WORK;
            pomodoro.time_left = POMODORO_WORK_DURATION;
        }
        pomodoro.is_running = false;
    }
}

// ===== DISPLAY TASK =====
static void display_task(void *pvParameter) {
    bool show_time = true;
    uint8_t counter = 0;
    
    // Wait for time sync
    int wait_count = 0;
    while (!wifi_is_time_synced() && wait_count < 30) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d/30)", wait_count + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        wait_count++;
    }
    
    while (1) {
        if (clock_get_time(&g_current_time) == ESP_OK) {
            oled_clear();
            
            if (show_time) {
                display_time_only(&g_current_time);
                display_pomodoro(&pomodoro);
            } else {
                display_date_only(&g_current_time);
            }
            
            counter++;
            if (counter >= 3) {
                counter = 0;
                show_time = !show_time;
            }
        }
        
        pomodoro_tick();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void display_start_task(void) {
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Display task started");
}