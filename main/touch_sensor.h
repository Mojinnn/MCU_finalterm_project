#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include "main.h"

void touch_init();
uint8_t touch_get_mode();
void touch_set_mode(uint8_t mode);

uint8_t pomo_get_state();
void change_pomo_init();
#endif