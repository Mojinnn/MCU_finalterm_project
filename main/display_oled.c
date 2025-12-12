#include "display_oled.h"

// ===== Display POMODORO TIMER =====
void oled_display_pomodoro(void) {
    uint8_t minutes = pomodoro.time_left / 60;
    uint8_t seconds = pomodoro.time_left % 60;
    
    int offset = 30;
    
    oled_draw_small_char(5, offset, minutes / 10 + '0');
    oled_draw_small_char(5, offset + 8, minutes % 10 + '0');
    oled_draw_small_char(5, offset + 16, ':');
    oled_draw_small_char(5, offset + 24, seconds / 10 + '0');
    oled_draw_small_char(5, offset + 32, seconds % 10 + '0');
    
    oled_write_cmd(0xB0 + 5);
    oled_write_cmd(0x00 + ((offset + 45) & 0x0F));
    oled_write_cmd(0x10 + ((offset + 45) >> 4));
    
    if (pomodoro.state == POMODORO_WORK) {
        uint8_t w[] = {0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00};
        oled_write_data(w, 6);
    } else if (pomodoro.state == POMODORO_BREAK || pomodoro.state == POMODORO_LONG_BREAK) {
        uint8_t b[] = {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00};
        oled_write_data(b, 6);
    }
    
    if (pomodoro.pomodoro_count < 10) {
        oled_draw_small_char(5, offset + 55, pomodoro.pomodoro_count + '0');
    }
}

// ===== Display time only =====
void oled_display_time_only(rtc_time_t *time) {
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

// ===== Display date only =====
void oled_display_date_only(rtc_time_t *time) {
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