/**
 * @file pin_entry.h
 * @brief Secure PIN Entry UI with rate limiting
 */

#ifndef PIN_ENTRY_H
#define PIN_ENTRY_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_ENTRY_MAX_LEN 12

esp_err_t pin_entry_init(void);
esp_err_t pin_entry_get(char *pin_out, size_t max_len);
esp_err_t pin_entry_confirm(const char *message);
void pin_entry_clear(void);
void pin_entry_mask_display(void);
uint8_t pin_entry_get_length(void);
bool pin_entry_is_rate_limited(void);
uint32_t pin_entry_get_delay_ms(void);

#ifdef __cplusplus
}
#endif

#endif // PIN_ENTRY_H
