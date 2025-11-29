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