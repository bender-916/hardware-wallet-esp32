#ifndef OLED_H
#define OLED_H

int oled_init(void);
void oled_clear(void);
void oled_refresh(void);
void oled_draw_string(int x, int y, const char* str);

#endif
