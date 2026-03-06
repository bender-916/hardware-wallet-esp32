/**
 * @file state_machine.h
 * @brief Hardware Wallet Secure State Machine
 * @details Secure state machine with rate limiting and safe transitions
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wallet states
 */
typedef enum {
    STATE_BOOT = 0,
    STATE_LOCKED,
    STATE_PIN_ENTRY,
    STATE_UNLOCKED,
    STATE_SIGNING,
    STATE_WIPE_REQUESTED,
    STATE_ERROR
} wallet_state_t;

/**
 * @brief State events
 */
typedef enum {
    EVENT_NONE = 0,
    EVENT_BOOT_COMPLETE,
    EVENT_WAKE_BUTTON,
    EVENT_PIN_DIGIT,
    EVENT_PIN_OK,
    EVENT_PIN_FAIL,
    EVENT_PIN_TIMEOUT,
    EVENT_SIGN_REQUEST,
    EVENT_SIGN_CONFIRM,
    EVENT_SIGN_CANCEL,
    EVENT_LOCK_REQUEST,
    EVENT_WIPE_CONFIRM,
    EVENT_TIMEOUT
} wallet_event_t;

/**
 * @brief PIN entry security context
 */
typedef struct {
    uint8_t attempts;
    uint32_t last_attempt_time;
    uint32_t delay_ms;
    bool locked_out;
} pin_security_t;

/**
 * @brief State machine context
 */
typedef struct {
    wallet_state_t current_state;
    wallet_state_t previous_state;
    pin_security_t pin_security;
    uint32_t state_entry_time;
    uint32_t unlock_timeout_ms;
    bool initialized;
} state_machine_ctx_t;

/**
 * @brief Initialize state machine
 * @return ESP_OK on success
 */
esp_err_t state_machine_init(void);

/**
 * @brief Process event and transition state
 * @param event Event to process
 * @return ESP_OK on valid transition, error otherwise
 */
esp_err_t state_machine_process_event(wallet_event_t event);

/**
 * @brief Get current state
 * @return Current state
 */
wallet_state_t state_machine_get_state(void);

/**
 * @brief Check if state transition is valid
 * @param from Source state
 * @param to Target state
 * @return true if valid transition
 */
bool state_machine_is_valid_transition(wallet_state_t from, wallet_state_t to);

/**
 * @brief Force state change (for emergency/wipe)
 * @param new_state State to force
 */
void state_machine_force_state(wallet_state_t new_state);

/**
 * @brief Check if PIN entry is allowed (rate limiting)
 * @return true if PIN entry allowed
 */
bool state_machine_pin_entry_allowed(void);

/**
 * @brief Record PIN attempt
 */
void state_machine_record_pin_attempt(void);

/**
 * @brief Get current PIN delay
 * @return Delay in milliseconds
 */
uint32_t state_machine_get_pin_delay(void);

/**
 * @brief Reset PIN security (after successful auth)
 */
void state_machine_reset_pin_security(void);

/**
 * @brief Check for auto-lock timeout
 * @return true if should auto-lock
 */
bool state_machine_check_timeout(void);

/**
 * @brief Get state name string
 * @param state State to name
 * @return State name
 */
const char* state_machine_get_state_name(wallet_state_t state);

#ifdef __cplusplus
}
#endif

#endif // STATE_MACHINE_H
