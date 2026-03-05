#include "buttons.h"
#include "driver/gpio.h"

#define BTN_UP_PIN     10
#define BTN_DOWN_PIN   11
#define BTN_CONFIRM_PIN 12
#define BTN_CANCEL_PIN 13

int buttons_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTN_UP_PIN) | (1ULL << BTN_DOWN_PIN) | 
                        (1ULL << BTN_CONFIRM_PIN) | (1ULL << BTN_CANCEL_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    return 0;
}

int buttons_confirm_pressed(void) {
    return gpio_get_level(BTN_CONFIRM_PIN) == 0;
}
