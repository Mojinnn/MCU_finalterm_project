#include "led.h"

led_strip_handle_t led_strip;

void led_init() {

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = LED_NUM,
        .led_model = LED_MODEL_WS2812,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 40 * 1000 * 1000,
        .mem_block_symbols = 64,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
}

void led_off () {
    for (int i = 0; i < LED_NUM; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }
    led_strip_refresh(led_strip);
}

void white_led () {
    for (int i = 0; i < LED_NUM; i++) {
        led_strip_set_pixel(led_strip, i, 255, 255, 100);
    }
    led_strip_refresh(led_strip);
}

void yellow_led() {
    for (int i = 0; i < LED_NUM; i++) {
        led_strip_set_pixel(led_strip, i, 255, 200, 18);
    }
    led_strip_refresh(led_strip);
}

void blue_led() {
    for (int i = 0; i < LED_NUM; i++) {
        led_strip_set_pixel(led_strip, i, 255, 255, 50);
    }
    led_strip_refresh(led_strip);
}