#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h" 
#include "led_strip.h"
#include <time.h>
#include <sys/time.h>
#include "driver/spi_master.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include <driver/ledc.h>


// ===== Config WiFi =====
#define WIFI_SSID      "TTHL&TT-DHBK"
#define WIFI_PASS      "viettenbadualara"

#define LED_PIN             8
#define LED_NUM             25
#define CHANGE_POMO_PIN     0
#define TOUCH_PIN           1
#define DEBOUNCE_TIME_MS    50

// Light mode
#define MODE                3
#define OFF                 0
#define WHITE               1
#define YELLOW              2
#define BLUE                3


// ===== Config TIMEZONE =====
#define TIMEZONE       "UTC-7"  // Vietnam (UTC+7)
// Other timezone:
// "UTC-7" = Vietnam (UTC+7)
// "UTC-8" = China, Singapore (UTC+8)
// "UTC-9" = Japan (UTC+9)  
// "PST8PDT" = US Pacific
// "EST5EDT" = US Eastern


// Config SPI for OLED
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

#define OLED_MOSI           2
#define OLED_SCK            3
#define OLED_CS             5
#define OLED_DC             4
#define OLED_RST            10

// Config I2C for RTC_DS3231    
#define I2C_MASTER_SCL_IO           7
#define I2C_MASTER_SDA_IO           6
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define DS3231_ADDR                 0x68


// ===== POMODORO SETTINGS =====
#define POMODORO_WORK_DURATION          (25 * 60)
#define POMODORO_BREAK_DURATION         (5 * 60)
#define POMODORO_LONG_BREAK_DURATION    (15 * 60)


// ===== BUZZER SETTING =====
#define BUZZER_PIN          GPIO_NUM_18
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT
#define LEDC_DUTY           (4096)              // 50% duty cycle (8192 / 2)
#define LEDC_FREQUENCY      (2000)              // Tần số 2kHz

#endif