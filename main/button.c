#include "button.h"

static volatile uint8_t button_count = 0;
static volatile uint32_t last_interrupt_time = 0;

static void IRAM_ATTR button_isr_handler (void *arg) {
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if ((now - last_interrupt_time) > DEBOUNCE_TIME_MS) {
        button_count++;
        if (button_count > 2) {
            button_count = 0;
        }
        last_interrupt_time = now;
    }
}

void button_init () {
    gpio_config_t io_button = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_button);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
}

uint8_t button_get_mode () {
    return button_count;
}

