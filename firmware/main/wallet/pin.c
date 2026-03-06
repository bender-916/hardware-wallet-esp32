/**
 * @file pin.c
 * @brief Secure PIN Implementation
 * @details Rate-limited PIN with constant-time comparison
 * Secure zeroing after verification
 */

#include <string.h>
#include "pin.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mbedtls/sha256.h"

static const char *TAG = "PIN";

// Simulated PIN storage (in production, store in secure element or encrypted NVS)
// This would be a hash, not plaintext
static uint8_t s_stored_pin_hash[PIN_HASH_SIZE] = {0};
static bool s_pin_configured = false;

// PIN security
static uint8_t s_attempts = 0;
static const uint8_t MAX_PIN_ATTEMPTS = 10;
static uint32_t s_delay_ms = 0;
static bool s_rate_limited = false;

// Secure memory clear
static void pin_secure_zero(void *ptr, size_t len)
{
    volatile unsigned char *p = ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Generate PIN hash
static void pin_hash(const char *pin, size_t len, uint8_t hash[PIN_HASH_SIZE])
{
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts_ret(&ctx, 0);
    
    // Add salt/padding
    mbedtls_sha256_update_ret(&ctx, (const uint8_t *)"hw_wallet_pin", 13);
    mbedtls_sha256_update_ret(&ctx, (const uint8_t *)pin, len);
    mbedtls_sha256_update_ret(&ctx, (const uint8_t *)"hw_wallet_end", 11);
    
    mbedtls_sha256_finish_ret(&ctx, hash);
    mbedtls_sha256_free(&ctx);
}

esp_err_t pin_init(void)
{
    ESP_LOGI(TAG, "Initializing PIN subsystem...");
    
    // Check if PIN is configured (would load from secure storage)
    // For now, simulate new device
    s_pin_configured = false;
    s_attempts = 0;
    s_delay_ms = 0;
    s_rate_limited = false;
    
    ESP_LOGI(TAG, "PIN initialized");
    return ESP_OK;
}

bool pin_is_configured(void)
{
    return s_pin_configured;
}

esp_err_t pin_set(const char *pin, size_t len)
{
    if (!pin_validate_format(pin, len)) {
        ESP_LOGE(TAG, "Invalid PIN format");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Setting new PIN...");
    
    // Hash PIN
    pin_hash(pin, len, s_stored_pin_hash);
    s_pin_configured = true;
    
    ESP_LOGI(TAG, "PIN set successfully");
    
    return ESP_OK;
}

esp_err_t pin_verify(const char *pin, size_t len, pin_result_t *result)
{
    if (result == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *result = PIN_RESULT_ERROR;
    
    // Check rate limiting
    if (pin_is_rate_limited()) {
        ESP_LOGW(TAG, "PIN entry rate limited");
        *result = PIN_RESULT_LOCKED;
        return ESP_ERR_NOT_ALLOWED;
    }
    
    // Validate format
    if (!pin_validate_format(pin, len)) {
        *result = PIN_RESULT_INVALID_CHAR;
        return ESP_ERR_INVALID_ARG;
    }
    
    // Compute hash
    uint8_t computed_hash[PIN_HASH_SIZE];
    pin_hash(pin, len, computed_hash);
    
    // Constant-time comparison
    int cmp = pin_secure_compare(computed_hash, s_stored_pin_hash, PIN_HASH_SIZE);
    
    // Secure zero computed hash
    pin_secure_zero(computed_hash, sizeof(computed_hash));
    
    if (cmp == 0) {
        // Success
        s_attempts = 0;
        s_delay_ms = 0;
        s_rate_limited = false;
        *result = PIN_RESULT_OK;
        ESP_LOGI(TAG, "PIN verified successfully");
        return ESP_OK;
    } else {
        // Failed
        s_attempts++;
        pin_update_rate_limit();
        *result = PIN_RESULT_INCORRECT;
        ESP_LOGW(TAG, "PIN incorrect (attempt %d/%d)", s_attempts, MAX_PIN_ATTEMPTS);
        return ESP_FAIL;
    }
}

esp_err_t pin_change(const char *old_pin, size_t old_len,
                     const char *new_pin, size_t new_len)
{
    pin_result_t result;
    
    // Verify old PIN first
    esp_err_t ret = pin_verify(old_pin, old_len, &result);
    if (ret != ESP_OK || result != PIN_RESULT_OK) {
        ESP_LOGE(TAG, "Old PIN verification failed");
        return ESP_FAIL;
    }
    
    // Set new PIN
    ret = pin_set(new_pin, new_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set new PIN");
        return ret;
    }
    
    ESP_LOGI(TAG, "PIN changed successfully");
    return ESP_OK;
}

void pin_clear(void)
{
    // Secure zero any PIN buffers
    pin_secure_zero(s_stored_pin_hash, sizeof(s_stored_pin_hash));
    s_pin_configured = false;
    ESP_LOGI(TAG, "PIN cleared from memory");
}

bool pin_validate_format(const char *pin, size_t len)
{
    if (len < PIN_MIN_DIGITS) {
        return false;
    }
    if (len > PIN_MAX_DIGITS) {
        return false;
    }
    
    // Check all characters are digits
    for (size_t i = 0; i < len; i++) {
        if (pin[i] < '0' || pin[i] > '9') {
            return false;
        }
    }
    
    return true;
}

uint8_t pin_get_remaining_attempts(void)
{
    if (s_attempts >= MAX_PIN_ATTEMPTS) {
        return 0;
    }
    return MAX_PIN_ATTEMPTS - s_attempts;
}

bool pin_is_rate_limited(void)
{
    return s_rate_limited || (s_attempts >= MAX_PIN_ATTEMPTS);
}

uint32_t pin_get_delay_ms(void)
{
    return s_delay_ms;
}

int pin_secure_compare(const uint8_t *a, const uint8_t *b, size_t len)
{
    unsigned char result = 0;
    
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    
    return (int)result;  // 0 if equal
}

// Update rate limiting parameters
static void pin_update_rate_limit(void)
{
    // Exponential delay progression
    const uint32_t delays[] = {0, 100, 1000, 5000, 30000, 60000, 120000, 300000, 600000};
    size_t idx = s_attempts;
    
    if (idx >= sizeof(delays)/sizeof(delays[0])) {
        idx = sizeof(delays)/sizeof(delays[0]) - 1;
    }
    
    s_delay_ms = delays[idx];
    
    if (s_attempts >= MAX_PIN_ATTEMPTS) {
        s_rate_limited = true;
        ESP_LOGE(TAG, "Rate limit: Maximum attempts reached");
    }
}
