/**
 * @file pin_entry.c
 * @brief Secure PIN Entry UI Implementation
 * @details Rate limiting, secure display, zero after verification
 */

#include <string.h>
#include "pin_entry.h"
#include "wallet/pin.h"
#include "drivers/oled.h"
#include "drivers/buttons.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PIN_ENTRY";

static char s_pin_buffer[PIN_ENTRY_MAX_LEN + 1] = {0};
static uint8_t s_pin_length = 0;
static bool s_initialized = false;

esp_err_t pin_entry_init(void)
{
    if (s_initialized) return ESP_OK;
    
    memset(s_pin_buffer, 0, sizeof(s_pin_buffer));
    s_pin_length = 0;
    s_initialized = true;
    
    ESP_LOGI(TAG, "PIN entry UI initialized");
    return ESP_OK;
}

static void pin_secure_zero(void *ptr, size_t len)
{
    volatile uint8_t *p = ptr;
    while (len--) *p++ = 0;
}

esp_err_t pin_entry_get(char *pin_out, size_t max_len)
{
    if (!s_initialized) pin_entry_init();
    
    memset(s_pin_buffer, 0, sizeof(s_pin_buffer));
    s_pin_length = 0;
    
    // Check rate limiting
    if (pin_is_rate_limited()) {
        oled_clear();
        oled_draw_string(0, 0, "RATE LIMITED");
        oled_draw_string(0, 2, "Wait %lu s", pin_get_delay_ms() / 1000);
        oled_refresh();
        return ESP_ERR_TIMEOUT;
    }
    
    oled_clear();
    oled_draw_string(0, 0, "Enter PIN:");
    
    // Display masked entry
    char masked[PIN_ENTRY_MAX_LEN + 1] = {0};
    
    while (1) {
        // Show masked PIN
        memset(masked, '*', s_pin_length);
        masked[s_pin_length] = '\0';
        oled_draw_string(0, 2, masked);
        oled_refresh();
        
        button_event_t event = buttons_wait_for_event(pdMS_TO_TICKS(60000));
        
        if (event == BTN_UP || event == BTN_DOWN) {
            // Digit entry (simplified)
            if (s_pin_length < PIN_ENTRY_MAX_LEN) {
                s_pin_buffer[s_pin_length++] = '0' + (event == BTN_UP ? 1 : 0);
            }
        } else if (event == BTN_CONFIRM) {
            if (s_pin_length >= 4) {
                break; // PIN complete
            }
        } else if (event == BTN_CANCEL) {
            pin_entry_clear();
            return ESP_ERR_INVALID_STATE;
        } else if (event == BTN_NONE) {
            // Timeout
            pin_entry_clear();
            return ESP_ERR_TIMEOUT;
        }
    }
    
    // Copy PIN to output
    strncpy(pin_out, s_pin_buffer, max_len - 1);
    pin_out[max_len - 1] = '\0';
    
    return ESP_OK;
}

void pin_entry_clear(void)
{
    pin_secure_zero(s_pin_buffer, sizeof(s_pin_buffer));
    s_pin_length = 0;
}

esp_err_t pin_entry_confirm(const char *message)
{
    oled_clear();
    oled_draw_string(0, 0, message);
    oled_draw_string(0, 4, "OK to confirm");
    oled_draw_string(0, 6, "Cancel to abort");
    oled_refresh();
    
    button_event_t event = buttons_wait_for_event(pdMS_TO_TICKS(30000));
    return (event == BTN_CONFIRM) ? ESP_OK : ESP_FAIL;
}

uint8_t pin_entry_get_length(void)
{
    return s_pin_length;
}

bool pin_entry_is_rate_limited(void)
{
    return pin_is_rate_limited();
}

uint32_t pin_entry_get_delay_ms(void)
{
    return pin_get_delay_ms();
}

void pin_entry_mask_display(void)
{
    // Display asterisks instead of PIN digits
}
