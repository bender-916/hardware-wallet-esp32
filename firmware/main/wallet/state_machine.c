/**
 * @file state_machine.c
 * @brief Hardware Wallet Secure State Machine Implementation
 * @details States: BOOT -> LOCKED -> PIN_ENTRY -> UNLOCKED -> SIGNING
 * With rate limiting, secure transitions, and timeout handling
 */

#include <string.h>
#include "state_machine.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "STATE_MACHINE";

// Rate limiting: delay progression (exponential)
static const uint32_t PIN_DELAY_SEQUENCE[] = {
    0,      // First attempt - no delay
    100,    // 100ms
    1000,   // 1 second
    5000,   // 5 seconds
    30000,  // 30 seconds
    60000,  // 1 minute
    120000, // 2 minutes
    300000, // 5 minutes
    600000  // 10 minutes
};

#define MAX_PIN_ATTEMPTS 10
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// State machine context
static state_machine_ctx_t s_ctx = {0};
static bool s_initialized = false;

// Transition matrix: valid (from, to) pairs
static const bool TRANSITION_MATRIX[STATE_ERROR + 1][STATE_ERROR + 1] = {
    //                    BOOT   LOCKED  PIN     UNLOCK  SIGNING WIPE    ERROR
    /* BOOT */        {  false, true,   false,  false,  false,  false,  true  },
    /* LOCKED */      {  false, false,  true,   false,  false,  false,  true  },
    /* PIN_ENTRY */   {  false, true,   false,  true,   false,  false,  true  },
    /* UNLOCKED */    {  false, true,   false,  false,  true,   true,   true  },
    /* SIGNING */     {  false, false,  false,  true,   false,  false,  true  },
    /* WIPE_REQ */    {  true,  false,  false,  false,  false,  false,  true  },
    /* ERROR */       {  true,  false,  false,  false,  false,  false,  true  }
};

static esp_err_t transition_to(wallet_state_t new_state);
static void update_timeout(void);

esp_err_t state_machine_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing secure state machine...");

    memset(&s_ctx, 0, sizeof(s_ctx));
    
    // Start in BOOT state
    s_ctx.current_state = STATE_BOOT;
    s_ctx.previous_state = STATE_BOOT;
    s_ctx.pin_security.delay_ms = 0;
    s_ctx.pin_security.locked_out = false;
    s_ctx.pin_security.attempts = 0;
    s_ctx.unlock_timeout_ms = 120000; // 2 minutes default
    s_ctx.initialized = true;
    s_ctx.state_entry_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    ESP_LOGI(TAG, "State machine initialized");
    ESP_LOGI(TAG, "Auto-lock timeout: %lu ms", s_ctx.unlock_timeout_ms);

    s_initialized = true;

    // Transition to LOCKED after boot
    esp_err_t ret = transition_to(STATE_LOCKED);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transition to LOCKED");
        return ret;
    }

    return ESP_OK;
}

esp_err_t state_machine_process_event(wallet_event_t event)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "State machine not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    wallet_state_t current = s_ctx.current_state;
    wallet_state_t next = current;

    ESP_LOGD(TAG, "Processing event %d in state %d", event, current);

    switch (current) {
        case STATE_BOOT:
            if (event == EVENT_BOOT_COMPLETE) {
                next = STATE_LOCKED;
            }
            break;

        case STATE_LOCKED:
            if (event == EVENT_WAKE_BUTTON) {
                next = STATE_PIN_ENTRY;
            }
            break;

        case STATE_PIN_ENTRY:
            if (event == EVENT_PIN_OK) {
                if (state_machine_pin_entry_allowed()) {
                    next = STATE_UNLOCKED;
                    state_machine_reset_pin_security();
                } else {
                    ESP_LOGW(TAG, "PIN entry blocked by rate limiting");
                }
            } else if (event == EVENT_PIN_FAIL) {
                state_machine_record_pin_attempt();
                next = STATE_LOCKED;
            } else if (event == EVENT_PIN_TIMEOUT || event == EVENT_TIMEOUT) {
                next = STATE_LOCKED;
            }
            break;

        case STATE_UNLOCKED:
            if (event == EVENT_LOCK_REQUEST || event == EVENT_TIMEOUT) {
                next = STATE_LOCKED;
            } else if (event == EVENT_SIGN_REQUEST) {
                next = STATE_SIGNING;
            } else if (event == EVENT_WIPE_CONFIRM) {
                next = STATE_WIPE_REQUESTED;
            }
            break;

        case STATE_SIGNING:
            if (event == EVENT_SIGN_CONFIRM) {
                next = STATE_UNLOCKED;
            } else if (event == EVENT_SIGN_CANCEL || event == EVENT