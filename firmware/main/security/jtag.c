/**
 * @file jtag.c
 * @brief ESP32-S3 JTAG Disable Implementation
 * @details Permanent JTAG disabling via efuse for production security
 * Prevents debugging and memory readout attacks
 */

#include <string.h>
#include "jtag.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "hal/efuse_hal.h"
#include "soc/efuse_reg.h"

static const char *TAG = "JTAG";

// Efuse bit positions
#define EFUSE_DIS_PAD_JTAG     1  // Bit 1 of BLOCK0
#define EFUSE_DIS_USB_JTAG     2  // Bit 2 of BLOCK0

static jtag_status_t s_jtag_status = JTAG_STATUS_ENABLED;
static bool s_initialized = false;

esp_err_t jtag_security_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing JTAG security check...");

    s_jtag_status = jtag_get_status();

    switch (s_jtag_status) {
        case JTAG_STATUS_FULLY_DISABLED:
            ESP_LOGI(TAG, "JTAG: FULLY DISABLED (Production mode)");
            ESP_LOGI(TAG, "PAD JTAG: Disabled");
            ESP_LOGI(TAG, "USB JTAG: Disabled");
            break;
            
        case JTAG_STATUS_DISABLED_PAD:
            ESP_LOGW(TAG, "JTAG: PAD disabled, USB still enabled");
            ESP_LOGW(TAG, "USB debugging still possible");
            break;
            
        case JTAG_STATUS_DISABLED_USB:
            ESP_LOGW(TAG, "JTAG: USB disabled, PAD still enabled");
            ESP_LOGW(TAG, "Physical JTAG still possible");
            break;
            
        case JTAG_STATUS_ENABLED:
            ESP_LOGE(TAG, "CRITICAL: JTAG FULLY ENABLED");
            ESP_LOGE(TAG, "Device is vulnerable to debug attacks!");
            
            #ifdef CONFIG_HW_WALLET_PRODUCTION
            ESP_LOGE(TAG, "PRODUCTION MODE: Halting due to enabled JTAG");
            return ESP_FAIL;
            #else
            ESP_LOGW(TAG, "Development mode - continuing with JTAG enabled");
            #endif
            break;
            
        case JTAG_STATUS_ERROR:
            ESP_LOGE(TAG, "Error checking JTAG status");
            return ESP_FAIL;
            
        default:
            break;
    }

    s_initialized = true;
    jtag_report_status();
    
    return ESP_OK;
}

jtag_status_t jtag_get_status(void)
{
    bool pad_disabled = jtag_pad_disabled();
    bool usb_disabled = jtag_usb_disabled();

    if (pad_disabled && usb_disabled) {
        return JTAG_STATUS_FULLY_DISABLED;
    } else if (pad_disabled) {
        return JTAG_STATUS_DISABLED_PAD;
    } else if (usb_disabled) {
        return JTAG_STATUS_DISABLED_USB;
    } else {
        return JTAG_STATUS_ENABLED;
    }
}

bool jtag_is_disabled(void)
{
    return (jtag_get_status() == JTAG_STATUS_FULLY_DISABLED);
}

bool jtag_pad_disabled(void)
{
    // Check DIS_PAD_JTAG efuse
    return efuse_hal_is_pad_jtag_disabled();
}

bool jtag_usb_disabled(void)
{
    // Check DIS_USB_JTAG efuse
    return efuse_hal_is_usb_jtag_disabled();
}

void jtag_report_status(void)
{
    ESP_LOGI(TAG, "=== JTAG Security Report ===");
    
    jtag_status_t status = jtag_get_status();
    
    switch (status) {
        case JTAG_STATUS_FULLY_DISABLED:
            ESP_LOGI(TAG, "Status: SECURE - JTAG fully disabled");
            break;
        case JTAG_STATUS_DISABLED_PAD:
            ESP_LOGW(TAG, "Status: WARNING - USB JTAG enabled");
            break;
        case JTAG_STATUS_DISABLED_USB:
            ESP_LOGW(TAG, "Status: WARNING - PAD JTAG enabled");
            break;
        case JTAG_STATUS_ENABLED:
            ESP_LOGE(TAG, "Status: CRITICAL - JTAG fully enabled");
            break;
        default:
            ESP_LOGW(TAG, "Status: UNKNOWN");
            break;
    }
    
    ESP_LOGI(TAG, "PAD JTAG efuse: %s", 
             jtag_pad_disabled() ? "DISABLED" : "ENABLED");
    ESP_LOGI(TAG, "USB JTAG efuse: %s", 
             jtag_usb_disabled() ? "DISABLED" : "ENABLED");
    
    ESP_LOGI(TAG, "============================");
}

bool jtag_debug_mode_active(void)
{
    // Check if any JTAG is enabled
    jtag_status_t status = jtag_get_status();
    
    #ifdef CONFIG_HW_WALLET_PRODUCTION
    // In production, JTAG should always be disabled
    if (status != JTAG_STATUS_FULLY_DISABLED) {
        ESP_LOGE(TAG, "WARNING: Debug mode active in PRODUCTION build");
        return true;
    }
    return false;
    #else
    //