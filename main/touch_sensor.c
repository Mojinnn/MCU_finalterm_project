#include "touch_sensor.h"

static volatile uint8_t touch_count = 0;
static volatile uint32_t last_interrupt_time = 0;

static void IRAM_ATTR touch_isr_handler () {
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if ((now - last_interrupt_time) > DEBOUNCE_TIME_MS) {
        touch_count++;
        if (touch_count > MODE) {
            touch_count = 0;
        }
        last_interrupt_time = now;
    }
}

void touch_init () {
    gpio_config_t io_touch = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << TOUCH_PIN),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_touch);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(TOUCH_PIN, touch_isr_handler, NULL);
}

uint8_t touch_get_mode() {
    return touch_count;
}