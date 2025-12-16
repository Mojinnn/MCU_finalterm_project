#ifndef MANUAL_SWITCH_H
#define MANUAL_SWITCH_H

#include "main.h"
#include "touch_sensor.h"
#include "led.h"

void manual_switch();
static void manual_switch_task(void *arg);
void manual_switch_start(void);

#endif