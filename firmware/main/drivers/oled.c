#include "oled.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

int oled_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 8,
        .scl_io_num = 9,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    
    // Init sequence
    uint8_t init_cmds[] = {0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
    
    for (int i = 0; i < sizeof(init_cmds); i++) {
        uint8_t cmd[2] = {0x00, init_cmds[i]};
        i2c_master_send(I2C_MASTER_NUM, cmd, 2, OLED_ADDR, 100);
    }
    
    return 0;
}

void oled_clear(void) {
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

void oled_refresh(void) {
    uint8_t cmd[] = {0x00, 0xB0, 0x00, 0x10};
    for (int p = 0; p < 8; p++) {
        cmd[1] = 0xB0 | p;
        i2c_master_send(I2C_MASTER_NUM, cmd, 4, OLED_ADDR, 100);
        uint8_t data[129] = {0x40};
        memcpy(&data[1], &oled_buffer[p * 128], 128);
        i2c_master_send(I2C_MASTER_NUM, data, 129, OLED_ADDR, 100);
    }
}

void oled_draw_string(int x, int y, const char* str) {
    // Simplified - in real implementation, use font table
    int idx = y * 16 + x;
    while (*str && idx < sizeof(oled_buffer)) {
        oled_buffer[idx++] = *str++;
    }
}
