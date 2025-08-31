#ifndef UI_H
#define UI_H
#include <stdint.h>

#define ICON_BASE 240u

/* Icon indices (matching the order in ICONS_1BPP) */
typedef enum
{
    ICON_FOOD = 0,
    ICON_WATER,
    ICON_MED,
    ICON_BATT,
    // bars (one tile each)
    ICON_BAR_1,
    ICON_BAR_2,
    ICON_BAR_3,
    ICON_BAR_4,
    ICON_BAR_5,
    ICON_BAR_6,
} UIIcon;

void ui_load_icons(void);
void ui_icons_refresh(void);
void ui_put_icon(uint8_t x, uint8_t y, uint8_t icon_tile);
void ui_init_console(void);
void ui_clear(void);
void ui_wait(void);
void ui_wait_msgless(void);
void ui_day_banner(uint16_t day);
void ui_text_box(const char *s);
uint8_t ui_menu(const char *title, const char *options[], uint8_t count, uint8_t selected);

#endif
