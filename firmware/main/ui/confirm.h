/**
 * @file confirm.h
 * @brief Transaction Confirmation UI
 */

#ifndef CONFIRM_H
#define CONFIRM_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *recipient_address;
    uint64_t amount_satoshis;
    uint64_t fee_satoshis;
    uint32_t input_count;
    uint32_t output_count;
} tx_display_info_t;

esp_err_t confirm_init(void);
esp_err_t confirm_transaction(const tx_display_info_t *tx_info);
esp_err_t confirm_double_button(const char *message);
void confirm_show_outputs(const tx_display_info_t *tx_info);
void confirm_show_inputs(const tx_display_info_t *tx_info);

#ifdef __cplusplus
}
#endif

#endif // CONFIRM_H
