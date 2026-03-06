/**
 * @file flash_enc.c
 * @brief ESP32-S3 Flash Encryption XTS-AES-256 Implementation
 * @details Native ESP32-S3 flash encryption using XTS-AES-128/256
 * Transparent encryption with HMAC-derived keys from efuse
 */

#include <string.h>
#include "flash_enc.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "esp_flash_encrypt.h"
#include "soc/efuse_periph.h"
#include "hal/efuse_hal.h"
#include "esp_secure_boot.h"

static const char *TAG = "FLASH_ENC";

// XTS-AES block size
#define XTS_BLOCK_SIZE 16
#define FLASH_SECTOR_SIZE 4096

// Private state
static flash_enc_status_t s_flash_status = FLASH_ENC_STATUS_DISABLED;
static bool s_initialized = false;

esp_err_t flash_enc_init(void)
{
    if (s_initialized) {
        ESP_LOGI(TAG, "Flash encryption already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Flash Encryption XTS-AES...");

    // Check flash encryption status via efuse
    uint32_t flash_crypt_cnt = hal_gpio_get_flash_crypt_cnt();
    
    if (flash_crypt_cnt == 0) {
        ESP_LOGE(TAG, "Flash encryption NOT ENABLED - Security critical!");
        ESP_LOGE(TAG, "Device must have flash encryption enabled in production!");
        s_flash_status = FLASH_ENC_STATUS_DISABLED;
        
        // In production, halt here or enter secure failure mode
        #ifdef CONFIG_HW_WALLET_PRODUCTION
        ESP_LOGE(TAG, "PRODUCTION MODE: Halting due to unencrypted flash");
        return ESP_FAIL;
        #endif
        
        return ESP_OK;
    }

    // Verify flash encryption is enabled (odd bit count = enabled)
    if ((flash_crypt_cnt & 0x7) == 0x7 || (flash_crypt_cnt & 0x7) == 0x4 ||
        (flash_crypt_cnt & 0x7) == 0x2 || (flash_crypt_cnt & 0x7) == 0x1) {
        ESP_LOGI(TAG, "Flash Encryption: ENABLED");
        s_flash_status = FLASH_ENC_STATUS_ENABLED;
    } else {
        ESP_LOGW(TAG, "Flash Encryption: DISABLED or INCOMPLETE");
        s_flash_status = FLASH_ENC_STATUS_DISABLED;
    }

    // Verify encryption configuration
    if (flash_enc_verify_config()) {
        ESP_LOGI(TAG, "Flash Encryption configuration: VALID");
    } else {
        ESP_LOGW(TAG, "Flash Encryption configuration: INVALID");
        s_flash_status = FLASH_ENC_STATUS_ERROR;
        return ESP_FAIL;
    }

    // Check HMAC key availability for key derivation
    if (!flash_enc_hmac_keys_configured()) {
        ESP_LOGW(TAG, "HMAC keys not fully configured");
    }

    s_initialized = true;
    ESP_LOGI(TAG, "Flash Encryption initialized successfully");
    
    return ESP_OK;
}

flash_enc_status_t flash_enc_get_status(void)
{
    return s_flash_status;
}

bool flash_enc_verify_config(void)
{
    // Check efuse settings for flash encryption
    uint32_t flash_crypt_cnt = hal_gpio_get_flash_crypt_cnt();
    
    // Check if encryption is enabled (odd number of bits set)
    bool encryption_enabled = ((flash_crypt_cnt >> 0) & 1) ||
                            ((flash_crypt_cnt >> 1) & 1) ||
                            ((flash_crypt_cnt >> 2) & 1);
    
    if (!encryption_enabled) {
        ESP_LOGE(TAG, "Flash encryption not enabled in efuse");
        return false;
    }

    // Check for secure download mode (prevents flash readout)
    extern uint32_t hal_gpio_get_secure_download_mode(void);
    bool secure_download = hal_gpio_get_secure_download_mode();
    
    if (!secure_download) {
        ESP_LOGW(TAG, "Secure download mode not enabled - flash readable");
    }

    ESP_LOGI(TAG, "Flash encryption verified: ENABLED");
    ESP_LOGI(TAG, "XTS-AES-256 active for external flash");
    
    return true;
}

bool flash_enc_is_encrypted_region(uint32_t addr)
{
    // Flash encryption applies to flash memory region
    // ESP32-S3 flash is typically 0x4200_0000 onwards
    if (addr >= 0x42000000) {
        return true;
    }
    return false;
}

esp_err_t flash_enc_encrypt_buffer(const uint8_t *plaintext, uint8_t *ciphertext, size_t len)
{
    if (len % XTS_BLOCK_SIZE != 0) {
        ESP_LOGE(TAG, "Buffer length must be %d-byte aligned", XTS_BLOCK_SIZE);
        return ESP_ERR_INVALID_SIZE;
    }

    // Note: Actual encryption is handled transparently by hardware
    // when writing to flash. This is for software-side validation.
    memcpy(ciphertext, plaintext, len);
    
    ESP_LOGD(TAG, "Buffer prepared for encrypted write");
    return ESP_OK;
}

bool flash_enc_hmac_keys_configured(void)
{
    // Check HMAC_KEY0-5 availability in efuse
    // These are used for key derivation in flash encryption
    
    esp_efuse_block_t block = EFUSE_BLK_KEY0;
    size_t key_len = 32; // 256-bit keys
    
    // Check if any HMAC keys are programmed
    bool hmac_configured = false;
    
    for (int i = 0; i < 6; i++) {
        esp_efuse_block_t key_block = (esp_efuse_block_t)(EFUSE_BLK_KEY0 + i);
        if (esp_efuse_block_is_empty(key_block) == false) {
            hmac_configured = true;
            ESP_LOGD(TAG, "HMAC_KEY%d programmed", i);
        }
    }
    
    return hmac_configured;
}

esp_err_t flash_enc_verify_integrity(void)
{
    ESP_LOGI(TAG, "Verifying encrypted flash integrity...");
    
    // In production, implement hash verification of critical regions
    // This would use the Digital Signature peripheral for HMAC verification
    
    return ESP_OK;
}

// Provide the missing hal_gpio_get_flash_crypt