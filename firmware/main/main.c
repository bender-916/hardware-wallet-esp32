/*
 * Hardware Wallet ESP32 - Main Entry Point
 * Air-gapped Bitcoin signing device
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

#include "app_state.h"
#include "errors.h"
#include "hw_config.h"
#include "crypto/bip39.h"
#include "crypto/bip32.h"
#include "crypto/psbt.h"
#include "drivers/oled.h"
#include "drivers/buttons.h"
#include "drivers/sd_card.h"

static const char* TAG = "HW_WALLET";

// Forward declarations
static void wallet_init(void);
static void wallet_loop(void);
static void show_main_menu(void);

void app_main(void) {
    ESP_LOGI(TAG, "Hardware Wallet ESP32 Starting...");
    
    // Initialize hardware
    wallet_init();
    
    ESP_LOGI(TAG, "Initialization complete");
    
    // Main loop
    wallet_loop();
}

static void wallet_init(void) {
    // Initialize display
    oled_init();
    oled_clear();
    oled_draw_string(0, 0, "Hardware Wallet");
    oled_draw_string(0, 2, "Initializing...");
    oled_refresh();
    
    // Initialize buttons
    buttons_init();
    
    // Initialize SD card
    sd_card_init();
    
    // Initialize crypto RNG
    rand_init();
    
    // Load or create wallet
    app_state_t state = {0};
    if (app_state_load(&state) != ERR_OK) {
        // New wallet - generate seed
        oled_clear();
        oled_draw_string(0, 0, "Create New Wallet");
        oled_draw_string(0, 2, "Press CONFIRM");
        oled_refresh();
        
        // Wait for button
        while (!buttons_confirm_pressed()) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // Generate mnemonic
        char mnemonic[512];
        bip39_generate_mnemonic(12, mnemonic, sizeof(mnemonic));
        
        // Display seed phrase
        oled_clear();
        oled_draw_string(0, 0, "SEED PHRASE:");
        oled_draw_string(0, 2, "WRITE IT DOWN!");
        oled_draw_string(0, 4, mnemonic);
        oled_refresh();
        
        // Save state
        strncpy(state.mnemonic, mnemonic, sizeof(state.mnemonic));
        app_state_save(&state);
    }
    
    ESP_LOGI(TAG, "Wallet initialized");
}

static void wallet_loop(void) {
    while (1) {
        show_main_menu();
        
        // Wait for user input
        button_event_t btn = buttons_wait_for_event(portMAX_DELAY);
        
        switch (btn) {
            case BTN_UP:
                // Navigate up
                break;
            case BTN_DOWN:
                // Navigate down
                break;
            case BTN_CONFIRM:
                // Handle selection
                // For now, just show version
                oled_clear();
                oled_draw_string(0, 0, "HW Wallet v1.0");
                oled_draw_string(0, 2, "Air-gapped BTC");
                oled_refresh();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case BTN_CANCEL:
                // Go back
                break;
            default:
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void show_main_menu(void) {
    oled_clear();
    oled_draw_string(0, 0, "=== MENU ===");
    oled_draw_string(0, 2, "> Show Address");
    oled_draw_string(0, 3, "  Sign Transaction");
    oled_draw_string(0, 4, "  Settings");
    oled_refresh();
}
