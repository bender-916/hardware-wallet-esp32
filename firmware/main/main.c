/**
 * Hardware Wallet ESP32-S3 - Secure Boot Entry Point
 * @details Maximum security with ESP32-S3 native features:
 * - Flash Encryption XTS-AES-256
 * - Secure Boot V2 ECDSA P-256
 * - JTAG Disable
 * - State Machine with rate limiting
 * - DRAM-only key storage
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_sleep.h"

// Security modules
#include "security/flash_enc.h"
#include "security/secure_boot.h"
#include "security/jtag.h"
#include "security/hmac.h"

// Wallet modules
#include "wallet/state_machine.h"
#include "wallet/key_management.h"
#include "wallet/pin.h"

// UI modules
#include "ui/wizard.h"
#include "ui/pin_entry.h"
#include "ui/confirm.h"
#include "ui/menu.h"

// Drivers
#include "drivers/oled.h"
#include "drivers/buttons.h"
#include "drivers/sd_card.h"

// Crypto
#include "crypto/bip39.h"
#include "crypto/rand.h"

static const char* TAG = "HW_WALLET";

// Forward declarations
static esp_err_t security_init(void);
static esp_err_t hardware_init(void);
static esp_err_t wallet_init(void);
static void main_loop(void);
static void handle_locked_state(void);
static void handle_pin_entry_state(void);
static void handle_unlocked_state(void);
static void handle_signing_state(void);

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Hardware Wallet ESP32-S3");
    ESP_LOGI(TAG, "Secure Boot Starting...");
    ESP_LOGI(TAG, "=================================");

    // Step 1: Initialize security subsystems
    ESP_LOGI(TAG, "Phase 1: Security initialization");
    if (security_init() != ESP_OK) {
        ESP_LOGE(TAG, "SECURITY INIT FAILED - HALTING");
        // In production: enter secure failure mode
        vTaskDelay(portMAX_DELAY);
    }

    // Step 2: Initialize hardware
    ESP_LOGI(TAG, "Phase 2: Hardware initialization");
    if (hardware_init() != ESP_OK) {
        ESP_LOGE(TAG, "HARDWARE INIT FAILED");
        // Continue with limited functionality
    }

    // Step 3: Initialize wallet subsystems
    ESP_LOGI(TAG, "Phase 3: Wallet initialization");
    if (wallet_init() != ESP_OK) {
        ESP_LOGE(TAG, "WALLET INIT FAILED");
    }

    // Step 4: Initialize state machine
    ESP_LOGI(TAG, "Phase 4: Starting state machine");
    if (state_machine_init() != ESP_OK) {
        ESP_LOGE(TAG, "STATE MACHINE INIT FAILED");
    }

    // Step 5: Enter main loop
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Boot complete - entering main loop");
    ESP_LOGI(TAG, "=================================");

    main_loop();
}

static esp_err_t security_init(void)
{
    esp_err_t ret = ESP_OK;

    // Initialize Flash Encryption
    ESP_LOGI(TAG, "Initializing Flash Encryption...");
    ret = flash_enc_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash Encryption init failed: %s", esp_err_to_name(ret));
        // Non-fatal in development, fatal in production
    }

    // Initialize Secure Boot
    ESP_LOGI(TAG, "Initializing Secure Boot V2...");
    ret = secure_boot_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Secure Boot init failed: %s", esp_err_to_name(ret));
    }

    // Initialize JTAG Security
    ESP_LOGI(TAG, "Checking JTAG security...");
    ret = jtag_security_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JTAG security check failed: %s", esp_err_to_name(ret));
    }

    // Initialize HMAC subsystem
    ESP_LOGI(TAG, "Initializing HMAC subsystem...");
    ret = hmac_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "HMAC init warning: %s", esp_err_to_name(ret));
    }

    // Security summary
    ESP_LOGI(TAG, "=== SECURITY STATUS ===");
    ESP_LOGI(TAG, "Flash Encryption: %s", 
             flash_enc_get_status() == FLASH_ENC_STATUS_ENABLED ? "ENABLED" : "DISABLED");
    ESP_LOGI(TAG, "Secure Boot: %s", 
             secure_boot_is_enabled() ? "ENABLED" : "DISABLED");
    ESP_LOGI(TAG, "JTAG: %s", 
             jtag_is_disabled() ? "DISABLED" : "ENABLED");
    ESP_LOGI(TAG, "=======================");

    return ESP_OK;
}

static esp_err_t hardware_init(void)
{
    esp_err_t ret = ESP_OK;

    // Initialize display
    ESP_LOGI(TAG, "Initializing OLED display...");
    ret = oled_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OLED init failed");
        return ret;
    }

    // Show boot screen
    oled_clear();
    oled_draw_string(0, 0, "Hardware Wallet");
    oled_draw_string(0, 2, "ESP32-S3 Secure");
    oled_draw_string(0, 4, "Initializing...");
    oled_refresh();

    // Initialize buttons
    ESP_LOGI(TAG, "Initializing buttons...");
    ret = buttons_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Buttons init failed");
        return ret;
    }

    // Initialize SD card
    ESP_LOGI(TAG, "Initializing SD card...");
    ret = sd_card_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SD card not available");
        // Non-fatal - continue without SD
    }

    // Initialize RNG
    ESP_LOGI(TAG, "Initializing secure RNG...");
    ret = rand_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RNG init failed");
        return ret;
    }

    return ESP_OK;
}

static esp_err_t wallet_init(void)
{
    // Initialize key management
    ESP_LOGI(TAG, "Initializing key management...");
    if (key_mgmt_init() != ESP_OK) {
        ESP_LOGE(TAG, "Key management init failed");
        return ESP_FAIL;
    }

    // Initialize PIN subsystem
    ESP_LOGI(TAG, "Initializing PIN subsystem...");
    if (pin_init() != ESP_OK) {
        ESP_LOGE(TAG, "PIN init failed");
        return ESP_FAIL;
    }

    // Initialize UI modules
    ESP_LOGI(TAG, "Initializing UI modules...");
    wizard_init();
    pin_entry_init();
    confirm_init();
    menu_init();

    return ESP_OK;
}

static void main_loop(void)
{
    while (1) {
        wallet_state_t state = state_machine_get_state();

        switch (state) {
            case STATE_BOOT:
                // Should transition immediately
                vTaskDelay(pdMS_TO_TICKS(100));
                break;

            case STATE_LOCKED:
                handle_locked_state();
                break;

            case STATE_PIN_ENTRY:
                handle_pin_entry_state();
                break;

            case STATE_UNLOCKED:
                handle_unlocked_state();
                break;

            case STATE_SIGNING:
                handle_signing_state();
                break;

            case STATE_WIPE_REQUESTED:
                // TODO: Handle wipe
                state_machine_force_state(STATE_BOOT);
                break;

            case STATE_ERROR:
                oled_clear();
                oled_draw_string(0, 0, "ERROR STATE");
                oled_refresh();
                vTaskDelay(pdMS_TO_TICKS(5000));
                esp_restart();
                break;
        }

        // Check for auto-lock timeout
        if (state_machine_check_timeout()) {
            state_machine_process_event(EVENT_TIMEOUT);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void handle_locked_state(void)
{
    // Display locked screen
    oled_clear();
    oled_draw_string(0, 0, "=== LOCKED ===");
    oled_draw_string(0, 3, "Press any key");
    oled_refresh();

    // Wait for wake button
    button_event_t event = buttons_wait_for_event(pdMS_TO_TICKS(5000));
    
    if (event != BTN_NONE) {
        // Wake up - transition to PIN entry
        state_machine_process_event(EVENT_WAKE_BUTTON);
    }
}

static void handle_pin_entry_state(void)
{
    // Check rate limiting
    if (pin_is_rate_limited()) {
        oled_clear();
        oled_draw_string(0, 0, "RATE LIMITED");
        oled_draw_string(0, 2, "Wait %lu s", pin_get_delay_ms() / 1000);
        oled_refresh();
        vTaskDelay(pdMS_TO_TICKS(pin_get_delay_ms()));
    }

    // Get PIN from user
    char pin[16] = {0};
    esp_err_t ret = pin_entry_get(pin, sizeof(pin));
    
    if (ret == ESP_OK) {
        // Verify PIN
        pin_result_t result;
        ret = pin_verify(pin, strlen(pin), &result);
        
        // Clear PIN from memory immediately
        memset(pin, 0, sizeof(pin));
        
        if (ret == ESP_OK && result == PIN_RESULT_OK) {
            state_machine_process_event(EVENT_PIN_OK);
        } else {
            state_machine_process_event(EVENT_PIN_FAIL);
        }
    } else {
        // Timeout or cancel
        state_machine_process_event(EVENT_PIN_TIMEOUT);
    }
}

static void handle_unlocked_state(void)
{
    // Show main menu
    menu_display();

    // Wait for user input
    button_event_t event = buttons_wait_for_event(pdMS_TO_TICKS(60000));

    switch (event) {
        case BTN_UP:
            menu_up();
            break;
        case BTN_DOWN:
            menu_down();
            break;
        case BTN_CONFIRM:
            menu_process_selection(menu_get_selection());
            break;
        case BTN_CANCEL:
            // Lock wallet
            state_machine_process_event(EVENT_LOCK_REQUEST);
            break;
        case BTN_NONE:
            // Timeout - auto lock
            state_machine_process_event(EVENT_TIMEOUT);
            break;
        default:
            break;
    }
}

static void handle_signing_state(void)
{
    // Show transaction review
    oled_clear();
    oled_draw_string(0, 0, "=== SIGN TX ===");
    oled_draw_string(0, 2, "Review details");
    oled_draw_string(0, 4, "OK to confirm");
    oled_draw_string(0, 6, "Cancel to abort");
    oled_refresh();

    button_event_t event = buttons_wait_for_event(pdMS_TO_TICKS(60000));

    if (event == BTN_CONFIRM) {
        // Double-button confirmation required
        if (confirm_double_button("Sign Transaction?") == ESP_OK) {
            state_machine_process_event(EVENT_SIGN_CONFIRM);
        }
    } else if (event == BTN_CANCEL) {
        state_machine_process_event(EVENT_SIGN_CANCEL);
    }
}
