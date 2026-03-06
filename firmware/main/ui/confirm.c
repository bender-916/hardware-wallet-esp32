/**
 * @file confirm.c
 * @brief Transaction Confirmation UI Implementation
 * @details Physical double-button confirmation required
 */

#include <string.h>
#include "confirm.h"
#include "drivers/oled.h"
#include "drivers/buttons.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CONFIRM";

static bool s_initialized = false;

esp_err_t confirm_init(void)
{
    if (s_initialized) return ESP_OK;
    s_initialized = true;
    
    ESP_LOGI(TAG, "Confirmation UI initialized");
    return ESP_OK;
}

esp_err_t confirm_transaction(const tx_display_info_t *tx_info)
{
    if (!tx_info) return ESP_ERR_INVALID_ARG;
    
    ESP_LOGI(TAG, "Confirming transaction...");
    
    // Show summary
    oled_clear();
    oled_draw_string(0, 0, "=== TX REVIEW ===");
    oled_draw_string(0, 2, "Amount: %llu sat", tx_info->amount_satoshis);
    oled_draw_string(0, 4, "Fee: %llu sat", tx_info->fee_satoshis);
    oled_draw_string(0, 6, "OK for details");
    oled_refresh();
    
    // Wait for user navigation
    while (1) {
        button_event_t event = buttons_wait_for_event(pdMS_TO_TICKS(60000));
        
        if (event == BTN_CANCEL) {
            return ESP_ERR_INVALID_STATE;
        } else if (event == BTN_CONFIRM) {
            // Show details
            confirm_show_outputs(tx_info);
        } else if (event == BTN_NONE) {
            return ESP_ERR_TIMEOUT;
        }
    }
    
    return ESP_OK;
}

void confirm_show_outputs(const tx_display_info_t *tx_info)
{
    oled_clear();
    oled_draw_string(0, 0, "OUTPUTS: %u", tx_info->output_count);
    oled_draw_string(0, 2, "To:");
    // Truncate address for display
    if (tx_info->recipient_address) {
        char addr_short[20] = {0};
        strncpy(addr_short, tx_info->recipient_address, 16);
        oled_draw_string(0, 3, addr_short);
    }
    oled_draw_string(0, 5, "Amount: %llu", tx_info->amount_satoshis);
    oled_refresh();
    
    vTaskDelay(pdMS_TO_TICKS(3000));
}

void confirm_show_inputs(const tx_display_info_t *tx_info)
{
    oled_clear();
    oled_draw_string(0, 0, "INPUTS: %u", tx_info->input_count);
    oled_refresh();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

esp_err_t confirm_double_button(const char *message)
{
    oled_clear();
    oled_draw_string(0, 0, "=== CONFIRM ===");
    if (message) {
        oled_draw_string(0, 2, message);
    }
    oled_draw_string(0, 5, "HOLD BOTH");
    oled_draw_string(0, 6, "buttons 3s");
    oled_refresh();
    
    // Wait for double-button press
    // Both CONFIRM and CANCEL must be held
    uint32_t start = xTaskGetTickCount();
    bool both_held = false;
    
    while (1) {
        if (buttons_confirm_pressed() && buttons_cancel_pressed()) {
            if (!both_held) {
                both_held = true;
                start = xTaskGetTickCount();
            } else {
                // Check 3 second hold
                uint32_t elapsed = (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
                if (elapsed >= 3000) {
                    return ESP_OK; // Confirmed!
                }
                // Progress indicator
                oled_draw_string(0, 7, "Confirming...");
                oled_refresh();
            }
        } else {
            both_held = false;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    return ESP_FAIL;
}
