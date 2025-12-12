#include "oled.h"

// ===== init SPI =====
void spi_master_init(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = OLED_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = OLED_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = OLED_CS,
        .queue_size = 7,
        .flags = 0,
    };
    
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device));
    
    gpio_config_t io_conf_dc = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_DC),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_dc);
    
    gpio_config_t io_conf_rst = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << OLED_RST),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_rst);
}

// ===== init OLED =====
void oled_init(void) {
    oled_reset();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    oled_write_cmd(0xAE);
    oled_write_cmd(0xD5); oled_write_cmd(0x80);
    oled_write_cmd(0xA8); oled_write_cmd(0x3F);
    oled_write_cmd(0xD3); oled_write_cmd(0x00);
    oled_write_cmd(0x40);
    oled_write_cmd(0x8D); oled_write_cmd(0x14);
    oled_write_cmd(0x20); oled_write_cmd(0x00);
    oled_write_cmd(0xA1);
    oled_write_cmd(0xC8);
    oled_write_cmd(0xDA); oled_write_cmd(0x12);
    oled_write_cmd(0x81); oled_write_cmd(0xCF);
    oled_write_cmd(0xD9); oled_write_cmd(0xF1);
    oled_write_cmd(0xDB); oled_write_cmd(0x40);
    oled_write_cmd(0xA4);
    oled_write_cmd(0xA6);
    oled_write_cmd(0x2E);
    oled_write_cmd(0xAF);
    
    vTaskDelay(pdMS_TO_TICKS(100));
}

// ===== write command OLED SPI =====
void oled_write_cmd(uint8_t cmd) {
    gpio_set_level(OLED_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
        .flags = 0,
    };
    spi_device_polling_transmit(spi_device, &t);
}

// ===== write data OLED SPI =====
void oled_write_data(uint8_t *data, size_t len) {
    gpio_set_level(OLED_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
        .flags = 0,
    };
    spi_device_polling_transmit(spi_device, &t);
}

// ===== reset OLED =====
void oled_reset(void) {
    gpio_set_level(OLED_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(OLED_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// ===== clear oled =====
void oled_clear(void) {
    uint8_t clear[128] = {0};
    for (int page = 0; page < 8; page++) {
        oled_write_cmd(0xB0 + page);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        oled_write_data(clear, 128);
    }
}

// ===== draw large char =====
void oled_draw_large_char(uint8_t page, uint8_t col, uint8_t c) {
    if (c == ':') c = 10;
    else if (c == '/') c = 11;
    else if (c >= '0' && c <= '9') c = c - '0';
    else return;
    
    for (int p = 0; p < 2; p++) {
        oled_write_cmd(0xB0 + page + p);
        oled_write_cmd(0x00 + (col & 0x0F));
        oled_write_cmd(0x10 + (col >> 4));
        
        for (int i = 0; i < 8; i++) {
            uint8_t data = font_8x8[c][i];
            uint8_t byte = 0;
            
            if (p == 0) {
                for (int b = 0; b < 4; b++) {
                    if (data & (1 << b)) byte |= (3 << (b * 2));
                }
            } else {
                for (int b = 0; b < 4; b++) {
                    if (data & (1 << (b + 4))) byte |= (3 << (b * 2));
                }
            }
            oled_write_data(&byte, 1);
            oled_write_data(&byte, 1);
        }
    }
}

// ===== draw small char =====
void oled_draw_small_char(uint8_t page, uint8_t col, uint8_t c) {
    if (c >= '0' && c <= '9') c = c - '0';
    else if (c == ':') c = 10;
    else return;
    
    oled_write_cmd(0xB0 + page);
    oled_write_cmd(0x00 + (col & 0x0F));
    oled_write_cmd(0x10 + (col >> 4));
    
    for (int i = 0; i < 8; i++) {
        uint8_t byte = font_8x8[c][i];
        oled_write_data(&byte, 1);
    }
}
