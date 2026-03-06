/**
 * @file wizard.h
 * @brief Setup Wizard for Hardware Wallet
 */

#ifndef WIZARD_H
#define WIZARD_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WIZARD_STEP_NONE = 0,
    WIZARD_STEP_WELCOME,
    WIZARD_STEP_GENERATE_SEED,
    WIZARD_STEP_BACKUP_SEED,
    WIZARD_STEP_VERIFY_SEED,
    WIZARD_STEP_SET_PIN,
    WIZARD_STEP_CONFIRM_PIN,
    WIZARD_STEP_COMPLETE,
    WIZARD_STEP_ERROR
} wizard_step_t;

typedef struct {
    wizard_step_t current_step;
    char mnemonic[256];
    char pin[16];
    bool seed_verified;
    bool pin_set;
} wizard_ctx_t;

esp_err_t wizard_init(void);
esp_err_t wizard_start(void);
wizard_step_t wizard_get_step(void);
esp_err_t wizard_next_step(void);
esp_err_t wizard_prev_step(void);
esp_err_t wizard_process(void);
bool wizard_is_complete(void);
void wizard_reset(void);

#ifdef __cplusplus
}
#endif

#endif // WIZARD_H
