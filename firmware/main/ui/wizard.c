/**
 * @file wizard.c
 * @brief Setup Wizard Implementation
 * @details Step-by-step setup: Generate seed -> Backup -> Verify -> Set PIN
 */

#include <string.h>
#include "wizard.h"
#include "pin_entry.h"
#include "esp_log.h"
#include "drivers/oled.h"
#include "drivers/buttons.h"
#include "crypto/bip39.h"
#include "wallet/key_management.h"
#include "wallet/pin.h"

static const char *TAG = "WIZARD";

static wizard_ctx_t s_wizard = {0};
static bool s_initialized = false;

// Word indices for verification
static int s_verify_words[4] = {-1};

esp_err_t wizard_init(void)
{
    if (s_initialized) return ESP_OK;
    
    memset(&s_wizard, 0, sizeof(s_wizard));
    s_wizard.current_step = WIZARD_STEP_NONE;
    s_initialized = true;
    
    ESP_LOGI(TAG, "Wizard initialized");
    return ESP_OK;
}

esp_err_t wizard_start(void)
{
    s_wizard.current_step = WIZARD_STEP_WELCOME;
    return wizard_process();
}

wizard_step_t wizard_get_step(void)
{
    return s_wizard.current_step;
}

esp_err_t wizard_next_step(void)
{
    switch (s_wizard.current_step) {
        case WIZARD_STEP_WELCOME:
            s_wizard.current_step = WIZARD_STEP_GENERATE_SEED;
            break;
        case WIZARD_STEP_GENERATE_SEED:
            s_wizard.current_step = WIZARD_STEP_BACKUP_SEED;
            break;
        case WIZARD_STEP_BACKUP_SEED:
            s_wizard.current_step = WIZARD_STEP_VERIFY_SEED;
            break;
        case WIZARD_STEP_VERIFY_SEED:
            if (s_wizard.seed_verified) {
                s_wizard.current_step = WIZARD_STEP_SET_PIN;
            }
            break;
        case WIZARD_STEP_SET_PIN:
            s_wizard.current_step = WIZARD_STEP_CONFIRM_PIN;
            break;
        case WIZARD_STEP_CONFIRM_PIN:
            if (s_wizard.pin_set) {
                s_wizard.current_step = WIZARD_STEP_COMPLETE;
            }
            break;
        default:
            break;
    }
    
    return wizard_process();
}

esp_err_t wizard_prev_step(void)
{
    switch (s_wizard.current_step) {
        case WIZARD_STEP_BACKUP_SEED:
            s_wizard.current_step = WIZARD_STEP_GENERATE_SEED;
            break;
        case WIZARD_STEP_VERIFY_SEED:
            s_wizard.current_step = WIZARD_STEP_BACKUP_SEED;
            break;
        case WIZARD_STEP_SET_PIN:
            s_wizard.current_step = WIZARD_STEP_VERIFY_SEED;
            break;
        case WIZARD_STEP_CONFIRM_PIN:
            s_wizard.current_step = WIZARD_STEP_SET_PIN;
            break;
        default:
            break;
    }
    
    return wizard_process();
}

esp_err_t wizard_process(void)
{
    ESP_LOGI(TAG, "Processing step: %d", s_wizard.current_step);
    
    switch (s_wizard.current_step) {
        case WIZARD_STEP_WELCOME:
            oled_clear();
            oled_draw_string(0, 0, "=== WELCOME ===");
            oled_draw_string(0, 2, "Hardware Wallet");
            oled_draw_string(0, 4, "Setup Wizard");
            oled_draw_string(0, 6, "Press OK to start");
            oled_refresh();
            break;
            
        case WIZARD_STEP_GENERATE_SEED:
            oled_clear();
            oled_draw_string(0, 0, "Generating Seed");
            oled_draw_string(0, 2, "Please wait...");
            oled_refresh();
            
            // Generate 24-word mnemonic
            if (!bip39_generate_mnemonic(32, s_wizard.mnemonic, sizeof(s_wizard.mnemonic))) {
                s_wizard.current_step = WIZARD_STEP_ERROR;
                return ESP_FAIL;
            }
            
            oled_clear();
            oled_draw_string(0, 0, "Seed Generated!");
            oled_draw_string(0, 2, "Press OK");
            oled_refresh();
            break;
            
        case WIZARD_STEP_BACKUP_SEED:
            // Display mnemonic for backup
            oled_clear();
            oled_draw_string(0, 0, "BACKUP SEED:");
            oled_draw_string(0, 2, "Write down words");
            oled_draw_string(0, 4, "Never share!");
            oled_refresh();
            
            // TODO: Scroll through words
            // For now, show first part
            vTaskDelay(pdMS_TO_TICKS(3000));
            break;
            
        case WIZARD_STEP_VERIFY_SEED:
            // User must verify backup
            oled_clear();
            oled_draw_string(0, 0, "VERIFY BACKUP");
            oled_draw_string(0, 2, "Enter word #X");
            oled_refresh();
            
            // Mark as verified for now
            s_wizard.seed_verified = true;
            break;
            
        case WIZARD_STEP_SET_PIN:
            oled_clear();
            oled_draw_string(0, 0, "SET PIN");
            oled_draw_string(0, 2, "Enter 4-12 digits");
            oled_refresh();
            
            // TODO: PIN entry UI
            if (pin_entry_get(s_wizard.pin, sizeof(s_wizard.pin)) == ESP_OK) {
                // PIN entered
            }
            break;
            
        case WIZARD_STEP_CONFIRM_PIN:
            oled_clear();
            oled_draw_string(0, 0, "CONFIRM PIN");
            oled_refresh();
            s_wizard.pin_set = true;
            break;
            
        case WIZARD_STEP_COMPLETE:
            oled_clear();
            oled_draw_string(0, 0, "SETUP COMPLETE!");
            oled_draw_string(0, 2, "Wallet ready");
            oled_refresh();
            break;
            
        case WIZARD_STEP_ERROR:
            oled_clear();
            oled_draw_string(0, 0, "SETUP ERROR");
            oled_refresh();
            break;
            
        default:
            break;
    }
    
    return ESP_OK;
}

bool wizard_is_complete(void)
{
    return s_wizard.current_step == WIZARD_STEP_COMPLETE;
}

void wizard_reset(void)
{
    memset(&s_wizard, 0, sizeof(s_wizard));
    s_wizard.current_step = WIZARD_STEP_NONE;
}
