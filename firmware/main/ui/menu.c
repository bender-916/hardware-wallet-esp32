/**
 * @file menu.c
 * @brief Main Menu UI Implementation
 */

#include <string.h>
#include "menu.h"
#include "drivers/oled.h"
#include "drivers/buttons.h"
#include "esp_log.h"
#include "wallet/state_machine.h"

static const char *TAG = "MENU";

static const char* MENU_ITEMS[MENU_ITEM_COUNT] = {
    "Show Address",
    "Sign Transaction",
    "Settings",
    "Wipe Device",
    "Lock Wallet"
};

static menu_item_t s_selection = MENU_ITEM_SHOW_ADDRESS;
static bool s_initialized = false;

esp_err_t menu_init(void)
{
    if (s_initialized) return ESP_OK;
    s_initialized = true;
    s_selection = MENU_ITEM_SHOW_ADDRESS;
    
    ESP_LOGI(TAG, "Menu initialized");
    return ESP_OK;
}

menu_item_t menu_get_selection(void)
{
    return s_selection;
}

void menu_display(void)
{
    oled_clear();
    oled_draw_string(0, 0, "=== MAIN MENU ===");
    
    for (int i = 0; i < 4 && (s_selection + i) < MENU_ITEM_COUNT; i++) {
        if (i == 0) {
            oled_draw_string(0, 2 + i, "> %s", MENU_ITEMS[s_selection + i]);
        } else {
            oled_draw_string(0, 2 + i, "  %s", MENU_ITEMS[s_selection + i]);
        }
    }
    
    oled_refresh();
}

void menu_up(void)
{
    if (s_selection > 0) {
        s_selection--;
    }
    menu_display();
}

void menu_down(void)
{
    if (s_selection < MENU_ITEM_COUNT - 1) {
        s_selection++;
    }
    menu_display();
}

esp_err_t menu_process_selection(menu_item_t item)
{
    switch (item) {
        case MENU_ITEM_SHOW_ADDRESS:
            oled_clear();
            oled_draw_string(0, 0, "Address:");
            // TODO: Display address
            oled_refresh();
            break;
            
        case MENU_ITEM_SIGN_TX:
            oled_clear();
            oled_draw_string(0, 0, "Insert SD card");
            oled_refresh();
            break;
            
        case MENU_ITEM_SETTINGS:
            oled_clear();
            oled_draw_string(0, 0, "Settings");
            oled_refresh();
            break;
            
        case MENU_ITEM_WIPE:
            oled_clear();
            oled_draw_string(0, 0, "WIPE DEVICE?");
            oled_draw_string(0, 2, "ALL DATA LOST!");
            oled_refresh();
            break;
            
        case MENU_ITEM_LOCK:
            state_machine_process_event(EVENT_LOCK_REQUEST);
            break;
            
        default:
            break;
    }
    
    return ESP_OK;
}
