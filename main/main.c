// #include <stdio.h>
// #include "driver/rmt_tx.h"
// #include "led_strip.h"
// #include "esp_log.h"

// static const char *TAG = "WS2812";

// #define LED_PIN 8
// #define LED_NUM 9

// void app_main(void)
// {
//     ESP_LOGI(TAG, "Initializing WS2812...");

//     led_strip_handle_t led_strip;

//     led_strip_config_t strip_config = {
//         .strip_gpio_num = LED_PIN,
//         .max_leds = LED_NUM,
//         .led_model = LED_MODEL_WS2812,
//     };

//     led_strip_rmt_config_t rmt_config = {
//         .clk_src = RMT_CLK_SRC_DEFAULT,
//         .resolution_hz = 40 * 1000 * 1000,
//         .mem_block_symbols = 64,
//     };

//     ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
//     ESP_ERROR_CHECK(led_strip_clear(led_strip));

//     ESP_LOGI(TAG, "Start blinking!");

//     while (1) {
//         led_strip_clear(led_strip);

//         led_strip_set_pixel(led_strip, 0, 255, 0, 0); // Red
//         led_strip_refresh(led_strip);
//         vTaskDelay(500 / portTICK_PERIOD_MS);

//         led_strip_set_pixel(led_strip, 0, 0, 255, 0); // Green
//         led_strip_refresh(led_strip);
//         vTaskDelay(500 / portTICK_PERIOD_MS);

//         led_strip_set_pixel(led_strip, 0, 0, 0, 255); // Blue
//         led_strip_refresh(led_strip);
//         vTaskDelay(500 / portTICK_PERIOD_MS);
//     }
// }

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h" 
#include "driver/rmt_tx.h"
#include "led_strip.h"

static const char *TAG = "WS2812";

#define LED_PIN             8
#define LED_NUM             9
#define BUTTON_PIN          0
#define DEBOUNCE_TIME_MS    50

static volatile int button_count = 0;
static volatile uint32_t last_interrupt_time = 0;

//--- ISR ----
static void IRAM_ATTR button_isr_handler (void *arg) {
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    
    if ((current_time - last_interrupt_time) > DEBOUNCE_TIME_MS) {
        if (button_count < 2) {
            button_count++;
        }
        else {
            button_count = 0;
        }
        last_interrupt_time = current_time;
    }
}


void app_main(void)
{
    ESP_LOGI(TAG, "Initializing WS2812...");

    led_strip_handle_t led_strip;

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

    gpio_config_t io_btn = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_btn);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

    ESP_LOGI(TAG, "Start blinking!");
    int last_button_count = -1;

    while (1) {
        if (button_count != last_button_count) {
            ESP_LOGI(TAG, "Mode: %d", button_count);
            last_button_count = button_count;

            switch (button_count)
            {
            case 0:
                for (int i = 0; i < LED_NUM; i++) {
                    led_strip_set_pixel(led_strip, i, 255, 255, 100);
                }
                led_strip_refresh(led_strip);
                break;

            case 1:
                for (int i = 0; i < LED_NUM; i++) {
                    led_strip_set_pixel(led_strip, i, 255, 200, 18);
                }
                led_strip_refresh(led_strip);
                break;

            case 2:
                for (int i = 0; i < LED_NUM; i++) {
                    led_strip_set_pixel(led_strip, i, 255, 255, 50);
                }
                led_strip_refresh(led_strip);
                break;
            
            default:
                for (int i = 0; i < LED_NUM; i++) {
                    led_strip_set_pixel(led_strip, i, 255, 255, 50);
                }
                led_strip_refresh(led_strip);
                break;
            }

        }
        vTaskDelay(10 / portTICK_PERIOD_MS);

        // // --- WHITE ---
        // for (int i = 0; i < LED_NUM; i++) {
        //     led_strip_set_pixel(led_strip, i, 255, 255, 100);
        // }
        // led_strip_refresh(led_strip);
        // vTaskDelay(10000 / portTICK_PERIOD_MS);

        // // --- YELLOW ---
        // for (int i = 0; i < LED_NUM; i++) {
        //     led_strip_set_pixel(led_strip, i, 255, 200, 18);
        // }
        // led_strip_refresh(led_strip);
        // vTaskDelay(10000 / portTICK_PERIOD_MS);

        // // --- BLUE ---
        // for (int i = 0; i < LED_NUM; i++) {
        //     led_strip_set_pixel(led_strip, i, 255, 255, 50);
        // }
        // led_strip_refresh(led_strip);
        // vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

