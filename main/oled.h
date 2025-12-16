#ifndef OLED_H
#define OLED_H

#include "main.h"

void oled_init(void);
void oled_clear(void);
void oled_draw_large_char(uint8_t page, uint8_t col, uint8_t c);
void oled_draw_small_char(uint8_t page, uint8_t col, uint8_t c);
void oled_write_data(uint8_t *data, size_t len);
void oled_write_cmd(uint8_t *cmd);

#endif