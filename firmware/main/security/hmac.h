/**
 * @file hmac.h
 * @brief ESP32-S3 HMAC efuse Key Management
 * @details Hardware HMAC operations using keys stored in efuse
 */

#ifndef HMAC_H
#define HMAC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HMAC_MAX_RESULT 32
#define HMAC_KEY_SIZE 32

/**
 * @brief HMAC key slots
 */
typedef enum {
    HMAC_KEY0 = 0,
    HMAC_KEY1,
    HMAC_KEY2,
    HMAC_KEY3,
    HMAC_KEY4,
    HMAC_KEY5
} hmac_key_slot_t;

/**
 * @brief Initialize HMAC subsystem
 * @return ESP_OK on success
 */
esp_err_t hmac_init(void);

/**
 * @brief Compute HMAC using hardware key
 * @param key_slot HMAC key slot to use
 * @param message Message to hash
 * @param msg_len Message length
 * @param hmac_out 32-byte output buffer
 * @return ESP_OK on success
 */
esp_err_t hmac_compute(hmac_key_slot_t key_slot,
                       const uint8_t *message, size_t msg_len,
                       uint8_t hmac_out[HMAC_MAX_RESULT]);

/**
 * @brief Check if HMAC key is programmed
 * @param key_slot Slot to check
 * @return true if programmed
 */
bool hmac_key_is_programmed(hmac_key_slot_t key_slot);

/**
 * @brief Get HMAC key purpose
 * @param key_slot Slot to query
 * @return Purpose value
 */
int hmac_key_get_purpose(hmac_key_slot_t key_slot);

/**
 * @brief Derive key from HMAC-based derivation
 * @param key_slot Source key slot
 * @param label Derivation label
 * @param context Additional context
 * @param derived_key Output key
 * @return ESP_OK on success
 */
esp_err_t hmac_derive_key(hmac_key_slot_t key_slot,
                            const char *label,
                            const uint8_t *context, size_t ctx_len,
                            uint8_t derived_key[HMAC_KEY_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // HMAC_H
