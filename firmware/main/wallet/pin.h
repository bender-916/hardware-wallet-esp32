/**
 * @file pin.h
 * @brief Secure PIN Entry and Verification
 * @details Rate-limited PIN with constant-time comparison and secure memory
 */

#ifndef PIN_H
#define PIN_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_MAX_DIGITS 12
#define PIN_MIN_DIGITS 4
#define PIN_HASH_SIZE 32

/**
 * @brief PIN entry result
 */
typedef enum {
    PIN_RESULT_OK,
    PIN_RESULT_TOO_SHORT,
    PIN_RESULT_TOO_LONG,
    PIN_RESULT_INVALID_CHAR,
    PIN_RESULT_INCORRECT,
    PIN_RESULT_LOCKED,
    PIN_RESULT_ERROR
} pin_result_t;

/**
 * @brief Secure PIN handle
 */
typedef struct {
    uint8_t buffer[PIN_MAX_DIGITS];
    uint8_t length;
    bool masked;
} pin_entry_t;

/**
 * @brief Initialize PIN subsystem
 * @return ESP_OK on success
 */
esp_err_t pin_init(void);

/**
 * @brief Check if PIN is set/configured
 * @return true if PIN configured
 */
bool pin_is_configured(void);

/**
 * @brief Set PIN (during setup)
 * @param pin PIN digits
 * @param len PIN length
 * @return ESP_OK on success
 */
esp_err_t pin_set(const char *pin, size_t len);

/**
 * @brief Verify PIN
 * @param pin PIN to verify
 * @param len PIN length
 * @param result Output result
 * @return ESP_OK if verified
 */
esp_err_t pin_verify(const char *pin, size_t len, pin_result_t *result);

/**
 * @brief Change PIN (requires old PIN)
 * @param old_pin Current PIN
 * @param old_len Length
 * @param new_pin New PIN
 * @param new_len Length
 * @return ESP_OK on success
 */
esp_err_t pin_change(const char *old_pin, size_t old_len,
                     const char *new_pin, size_t new_len);

/**
 * @brief Clear PIN from memory
 * @details Securely zeros PIN buffer
 */
void pin_clear(void);

/**
 * @brief Validate PIN format
 * @param pin PIN string
 * @param len Length
 * @return true if valid format
 */
bool pin_validate_format(const char *pin, size_t len);

/**
 * @brief Get remaining attempts
 * @return Attempts remaining
 */
uint8_t pin_get_remaining_attempts(void);

/**
 * @brief Check if PIN entry is rate limited
 * @return true if rate limited
 */
bool pin_is_rate_limited(void);

/**
 * @brief Get PIN entry delay (rate limiting)
 * @return Delay in milliseconds
 */
uint32_t pin_get_delay_ms(void);

/**
 * @brief Secure comparison (constant-time)
 * @param a First buffer
 * @param b Second buffer
 * @param len Length
 * @return 0 if equal
 */
int pin_secure_compare(const uint8_t *a, const uint8_t *b, size_t len);

#ifdef __cplusplus
}
#endif

#endif // PIN_H
