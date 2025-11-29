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
#include "led_strip_types.h"
#include "led_strip_rmt.h" 
#include "led_strip.h"

#define LED_PIN             8
#define LED_NUM             9
#define BUTTON_PIN          0
#define TOUCH_PIN           2
#define DEBOUNCE_TIME_MS    50

// Light mode
#define MODE                3
#define OFF                 0
#define WHITE               1
#define YELLOW              2
#define BLUE                3

// Config SPI for OLED
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

#define OLED_MOSI           3
#define OLED_SCK            2
#define OLED_CS             5
#define OLED_DC             4
#define OLED_RST            10

// Config I2C for RTC
#define I2C_MASTER_SCL_IO           7
#define I2C_MASTER_SDA_IO           6
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define DS3231_ADDR                 0x68

#endif