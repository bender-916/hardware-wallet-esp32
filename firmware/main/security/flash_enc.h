/**
 * @file flash_enc.h
 * @brief ESP32-S3 Flash Encryption XTS-AES-256 Management
 * @details Handles Flash Encryption initialization and verification using XTS-AES-256
 * This provides transparent encryption of external flash contents
 */

#ifndef FLASH_ENC_H
#define FLASH_ENC_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flash encryption status codes
 */
typedef enum {
    FLASH_ENC_STATUS_DISABLED = 0,
    FLASH_ENC_STATUS_ENABLED,
    FLASH_ENC_STATUS_ERROR
} flash_enc_status_t;

/**
 * @brief Initialize and verify Flash Encryption
 * @details Checks if flash encryption is enabled and configures properly
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t flash_enc_init(void);

/**
 * @brief Get current flash encryption status
 * @return Status of flash encryption
 */
flash_enc_status_t flash_enc_get_status(void);

/**
 * @brief Verify flash encryption is properly configured
 * @details Checks:
 * - Flash encryption enabled in efuse
 * - XTS-AES-256 mode active
 * - Keys properly derived from HMAC
 * @return true if properly configured
 */
bool flash_enc_verify_config(void);

/**
 * @brief Check if writing to encrypted flash is needed
 * @param addr Flash address to check
 * @return true if address is in encrypted region
 */
bool flash_enc_is_encrypted_region(uint32_t addr);

/**
 * @brief Securely encrypt data buffer for flash write
 * @param plaintext Input plaintext data
 * @param ciphertext Output encrypted data
 * @param len Length of data (must be 16-byte aligned)
 * @return ESP_OK on success
 */
esp_err_t flash_enc_encrypt_buffer(const uint8_t *plaintext, uint8_t *ciphertext, size_t len);

/**
 * @brief Get flash encryption key status (HMAC derived)
 * @return true if HMAC keys are properly configured
 */
bool flash_enc_hmac_keys_configured(void);

/**
 * @brief Secure boot check - validate encrypted flash integrity
 * @return ESP_OK if flash integrity verified
 */
esp_err_t flash_enc_verify_integrity(void);

#ifdef __cplusplus
}
#endif

#endif // FLASH_ENC_H
