/**
 * @file app_state.h
 * @brief Application state machine definitions
 */

#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>

typedef enum {
    STATE_BOOT,
    STATE_SETUP_WIZARD,
    STATE_PIN_ENTRY,
    STATE_MAIN_MENU,
    STATE_SIGN_TRANSACTION,
    STATE_VIEW_ADDRESS,
    STATE_BACKUP,
    STATE_SETTINGS,
    STATE_LOCKED,
    STATE_ERROR
} app_state_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_BUTTON_UP,
    EVENT_BUTTON_DOWN,
    EVENT_BUTTON_OK,
    EVENT_BUTTON_CANCEL,
    EVENT_BUTTON_LONG_OK,
    EVENT_BUTTON_LONG_CANCEL,
    EVENT_SD_INSERTED,
    EVENT_SD_REMOVED,
    EVENT_TIMEOUT,
    EVENT_ERROR
} app_event_t;

typedef struct {
    app_state_t current_state;
    app_state_t previous_state;
    bool is_initialized;
    bool is_unlocked;
    uint32_t selected_menu_item;
    uint32_t last_activity_time;
    char current_xpub[112];
} app_context_t;

// State transition function
typedef app_state_t (*state_handler_t)(app_context_t *ctx, app_event_t event);

#endif // APP_STATE_H
