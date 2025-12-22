#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"

void buzzer_init(void);
void buzzer_on(uint32_t frequency);
void buzzer_off(void);
void buzzer_beep_5s (void);

#endif