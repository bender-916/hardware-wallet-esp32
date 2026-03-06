/**
 * @file jtag.h
 * @brief ESP32-S3 JTAG Disable Management
 * @details Handles permanent JTAG disabling via efuse
 * Prevents debugging and memory extraction
 */

#ifndef JTAG_H
#define JTAG_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JTAG status codes
 */
typedef enum {
    JTAG_STATUS_ENABLED = 0,
    JTAG_STATUS_DISABLED_PAD,      // Physical JTAG disabled
    JTAG_STATUS_DISABLED_USB,      // USB JTAG disabled  
    JTAG_STATUS_FULLY_DISABLED,    // Both disabled
    JTAG_STATUS_ERROR
} jtag_status_t;

/**
 * @brief Initialize JTAG security check
 * @return ESP_OK on success
 */
esp_err_t jtag_security_init(void);

/**
 * @brief Get current JTAG status
 * @return JTAG status
 */
jtag_status_t jtag_get_status(void);

/**
 * @brief Check if JTAG is fully disabled
 * @return true if both PAD and USB JTAG are disabled
 */
bool jtag_is_disabled(void);

/**
 * @brief Verify PAD JTAG is disabled
 * @return true if physical JTAG pins disabled
 */
bool jtag_pad_disabled(void);

/**
 * @brief Verify USB JTAG is disabled
 * @return true if USB JTAG disabled
 */
bool jtag_usb_disabled(void);

/**
 * @brief Report JTAG security status
 * @details Logs warning if JTAG is still enabled
 */
void jtag_report_status(void);

/**
 * @brief Check if debug mode is active (development)
 * @return true if in debug mode
 */
bool jtag_debug_mode_active(void);

#ifdef __cplusplus
}
#endif

#endif // JTAG_H
