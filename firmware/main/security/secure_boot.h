/**
 * @file secure_boot.h
 * @brief ESP32-S3 Secure Boot V2 Management
 * @details Handles Secure Boot V2 ECDSA P-256 validation and chain of trust
 */

#ifndef SECURE_BOOT_H
#define SECURE_BOOT_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Secure boot status
 */
typedef enum {
    SECURE_BOOT_DISABLED = 0,
    SECURE_BOOT_ENABLED,
    SECURE_BOOT_VIOLATION,
    SECURE_BOOT_ERROR
} secure_boot_status_t;

/**
 * @brief Secure boot verification result
 */
typedef struct {
    bool bootloader_valid;
    bool app_valid;
    bool chain_of_trust;
    uint32_t secure_version;
    bool anti_rollback_enabled;
} secure_boot_verification_t;

/**
 * @brief Initialize secure boot verification
 * @return ESP_OK on success
 */
esp_err_t secure_boot_init(void);

/**
 * @brief Verify secure boot is enabled and configured
 * @return true if secure boot is properly enabled
 */
bool secure_boot_is_enabled(void);

/**
 * @brief Perform chain of trust validation
 * @details Verifies:
 * - BootROM -> Bootloader signature
 * - Bootloader -> Application signature
 * - ECDSA P-256 signatures valid
 * @param result Output verification result
 * @return ESP_OK if valid
 */
esp_err_t secure_boot_verify_chain(secure_boot_verification_t *result);

/**
 * @brief Get secure boot status
 * @return Current secure boot status
 */
secure_boot_status_t secure_boot_get_status(void);

/**
 * @brief Verify anti-rollback configuration
 * @details Checks secure_version in efuse
 * @return true if anti-rollback enabled
 */
bool secure_boot_anti_rollback_enabled(void);

/**
 * @brief Get current secure version from efuse
 * @return Secure version number
 */
uint32_t secure_boot_get_version(void);

/**
 * @brief Check if application version is allowed (anti-rollback)
 * @param app_version Version to check
 * @return true if version can boot
 */
bool secure_boot_check_version(uint32_t app_version);

/**
 * @brief Validate ECDSA signature
 * @param data Data that was signed
 * @param data_len Length of data
 * @param signature ECDSA signature (64 bytes)
 * @param public_key Public key for verification
 * @return true if signature valid
 */
bool secure_boot_verify_signature(const uint8_t *data, size_t data_len,
                                  const uint8_t *signature,
                                  const uint8_t *public_key);

#ifdef __cplusplus
}
#endif

#endif // SECURE_BOOT_H
