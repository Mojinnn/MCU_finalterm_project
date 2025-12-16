#include "manual_switch.h"

static const char* TAG = "SWITCH MODE";
static int last_mode = -1;

static void mode_switch(int mode) {
    switch (mode)
    {
    case OFF:
        led_off();
        break;
    case WHITE:
        white_led();
        break;
    case YELLOW:
        yellow_led();
        break;
    case BLUE:
        blue_led();
        break;
    default:
        led_off();
        break;
    }
}

void manual_switch() {
    int mode = touch_get_mode();
    if (mode != last_mode){
        ESP_LOGI(TAG, "Mode: %d", mode);
        last_mode = mode;
        mode_switch(mode);
    }
}

static void manual_switch_task(void *arg) {
    while (1) {
        manual_switch();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void manual_switch_start(void) {
    xTaskCreate(manual_switch_task, "Manual Switch", 2048, NULL, 5, NULL);
}