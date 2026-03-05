#ifndef BUTTONS_H
#define BUTTONS_H

typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_CONFIRM,
    BTN_CANCEL,
    BTN_NONE
} button_event_t;

int buttons_init(void);
int buttons_confirm_pressed(void);
button_event_t buttons_wait_for_event(int timeout_ms);

#endif
