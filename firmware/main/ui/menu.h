/**
 * @file menu.h
 * @brief Main Menu UI
 */

#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MENU_ITEM_SHOW_ADDRESS = 0,
    MENU_ITEM_SIGN_TX,
    MENU_ITEM_SETTINGS,
    MENU_ITEM_WIPE,
    MENU_ITEM_LOCK,
    MENU_ITEM_COUNT
} menu_item_t;

esp_err_t menu_init(void);
menu_item_t menu_get_selection(void);
void menu_display(void);
void menu_up(void);
void menu_down(void);
esp_err_t menu_process_selection(menu_item_t item);

#ifdef __cplusplus
}
#endif

#endif // MENU_H
