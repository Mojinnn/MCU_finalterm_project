#ifndef LED_H
#define LED_H

#include "main.h"

extern led_strip_handle_t led_strip;

void led_init();
void led_off();
void white_led();
void yellow_led();
void blue_led();

#endif