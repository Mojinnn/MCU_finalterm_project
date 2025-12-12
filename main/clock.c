#include "clock.h"

uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// ===== get time rtc =====
esp_err_t ds3231_get_time(rtc_time_t *time) {
    uint8_t reg = 0x00;
    uint8_t data[7];
    
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_NUM, DS3231_ADDR,
                                                &reg, 1, data, 7, 1000 / portTICK_PERIOD_MS);
    if (err != ESP_OK) return err;
    
    time->seconds = bcd_to_dec(data[0] & 0x7F);
    time->minutes = bcd_to_dec(data[1] & 0x7F);
    time->hours   = bcd_to_dec(data[2] & 0x3F);
    time->day     = bcd_to_dec(data[3] & 0x07);
    time->date    = bcd_to_dec(data[4] & 0x3F);
    time->month   = bcd_to_dec(data[5] & 0x1F);
    time->year    = bcd_to_dec(data[6]) + 2000;
    
    return ESP_OK;
}

// ===== set time rtc DS3231 =====
esp_err_t ds3231_set_time(rtc_time_t *time) {
    uint8_t data[8];
    
    data[0] = 0x00; // Register address
    data[1] = dec_to_bcd(time->seconds);
    data[2] = dec_to_bcd(time->minutes);
    data[3] = dec_to_bcd(time->hours);
    data[4] = dec_to_bcd(time->day);
    data[5] = dec_to_bcd(time->date);
    data[6] = dec_to_bcd(time->month);
    data[7] = dec_to_bcd(time->year - 2000);
    
    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_NUM, DS3231_ADDR, data, 8, 1000 / portTICK_PERIOD_MS);
    return err;
}