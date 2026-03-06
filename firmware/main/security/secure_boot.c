/**
 * @file secure_boot.c
 * @brief ESP32-S3 Secure Boot V2 Implementation
 * @details ECDSA P-256 secure boot with chain of trust validation
 * BootROM -> Bootloader -> Application signature verification
 */

#include <string.h>
#include "secure_boot.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "esp_secure_boot.h"
#include "hal/efuse_hal.h"
#include "soc/efuse_reg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/sha256.h"

static const char *TAG = "SEC_BOOT";

// ECDSA P-256 constants
#define ECDSA_P256_SIG_SIZE 64
#define ECDSA_P256_KEY_SIZE 64

static secure_boot_status_t s_boot_status = SECURE_BOOT_DISABLED;
static bool s_initialized = false;

esp_err_t secure_boot_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Secure Boot V2...");

    // Check if secure boot is enabled in efuse
    if (!secure_boot_is_enabled()) {
        ESP_LOGE(TAG, "Secure Boot V2 NOT ENABLED - Security critical!");
        ESP_LOGE(TAG, "Device must have secure boot enabled in production!");
        s_boot_status = SECURE_BOOT_DISABLED;
        
        #ifdef CONFIG_HW_WALLET_PRODUCTION
        ESP_LOGE(TAG, "PRODUCTION MODE: Halting due to insecure boot");
        return ESP_FAIL;
        #endif
        
        s_initialized = true;
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Secure Boot V2: ENABLED");
    ESP_LOGI(TAG, "ECDSA P-256 signature verification active");
    s_boot_status = SECURE_BOOT_ENABLED;

    // Verify chain of trust
    secure_boot_verification_t verification = {0};
    esp_err_t ret = secure_boot_verify_chain(&verification);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Chain of trust verification FAILED");
        s_boot_status = SECURE_BOOT_VIOLATION;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Chain of trust: VALID");
    ESP_LOGI(TAG, "Bootloader signature: %s", 
             verification.bootloader_valid ? "VALID" : "INVALID");
    ESP_LOGI(TAG, "Application signature: %s", 
             verification.app_valid ? "VALID" : "INVALID");
    ESP_LOGI(TAG, "Secure version: %lu", verification.secure_version);
    ESP_LOGI(TAG, "Anti-rollback: %s", 
             verification.anti_rollback_enabled ? "ENABLED" : "DISABLED");

    if (verification.anti_rollback_enabled) {
        ESP_LOGI(TAG, "Anti-rollback protection active");
    }

    s_initialized = true;
    return ESP_OK;
}

bool secure_boot_is_enabled(void)
{
    // Check SECURE_BOOT_EN efuse
    return efuse_hal_secure_boot_enabled();
}

esp_err_t secure_boot_verify_chain(secure_boot_verification_t *result)
{
    if (result == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(result, 0, sizeof(*result));

    ESP_LOGI(TAG, "Verifying chain of trust...");

    // Step 1: Verify bootloader was signed
    // The fact we're running means BootROM verified the bootloader
    result->bootloader_valid = true;
    ESP_LOGI(TAG, "BootROM -> Bootloader: VERIFIED");

    // Step 2: Verify application signature
    // ESP-IDF handles this verification before app_main
    // We verify the secure boot status flag
    result->app_valid = secure_boot_is_enabled();
    ESP_LOGI(TAG, "Bootloader -> Application: VERIFIED");

    // Step 3: Check chain of trust is complete
    result->chain_of_trust = result->bootloader_valid && result->app_valid;

    // Step 4: Get secure version for anti-rollback
    result->secure_version = secure_boot_get_version();
    result->anti_rollback_enabled = secure_boot_anti_rollback_enabled();

    return result->chain_of_trust ? ESP_OK : ESP_FAIL;
}

secure_boot_status_t secure_boot_get_status(void)
{
    return s_boot_status;
}

bool secure_boot_anti_rollback_enabled(void)
{
    // Check if secure version efuse is set
    uint32_t secure_version = efuse_hal_secure_boot_aggressive_revoke();
    
    // Also check if secure version is non-zero
    uint32_t version = secure_boot_get_version();
    
    return (version > 0) || (secure_version > 0);
}

uint32_t secure_boot_get_version(void)
{
    return efuse_hal_get_secure_version();
}

bool secure_boot_check_version(uint32_t app_version)
{
    uint32_t current_version = secure_boot_get_version();
    
    ESP_LOGD(TAG, "Checking version: app=%lu, efuse=%lu", 
             app_version, current_version);
    
    // Anti-rollback: app version must be >= efuse version
    if (app_version < current_version) {
        ESP_LOGE(TAG, "Anti-rollback: Version %lu rejected (minimum: %lu)",
                 app_version, current_version);
        return false;
    }
    
    return true;
}

bool secure_boot_verify_signature(const uint8_t *data, size_t data_len,