#include "main.h"
#include "manual_switch.h"
#include "rtc.h"
#include "display.h"
#include "wifi_manager.h"
#include "buzzer.h"
#include "change_pomo.h"

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  ESP32-C3 Desk Table");
    ESP_LOGI(TAG, "========================================");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize
    led_init();
    touch_init();
    change_pomo_init();
    buzzer_init();
    
    ESP_ERROR_CHECK(clock_init());
    display_init();
    wifi_manager_init();
    
    // Create task
    manual_switch_start();
    manual_pomo_start();
    display_start();

    
    ESP_LOGI(TAG, "System started successfully!");
}