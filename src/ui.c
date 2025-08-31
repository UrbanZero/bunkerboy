#include <gbdk/platform.h> // must be first
#include <gb/gb.h>
#include <gbdk/console.h> // <-- correct header for gotoxy, cls, etc.
#include <gbdk/font.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"

#define CURSOR_CHAR '>'
static const uint8_t MENU_ARROW = 0x10; // just a char index; console font uses ASCII

void ui_init_console(void)
{
    DISPLAY_OFF;
    HIDE_SPRITES;
    HIDE_WIN;
    SHOW_BKG;

    font_init();
    font_set(font_load(font_ibm));
    ui_load_icons();

    // (optional) clear once
    printf("\n");

    DISPLAY_ON;
}

void ui_clear(void)
{
    DISPLAY_OFF; // avoid VRAM writes while LCD is on
    for (uint8_t y = 0; y < 18; ++y)
    {
        gotoxy(0, y);
        printf("                    "); // 20 spaces
    }
    gotoxy(0, 0);
    DISPLAY_ON;
}

void ui_wait(void)
{
    printf("\n  Press A...");
    ui_wait_msgless();
}
void ui_wait_msgless(void)
{
    waitpad(J_A);
    waitpadup();
}

void ui_day_banner(uint16_t day)
{
    ui_clear();
    gotoxy(0, 0);
    printf(" Day %u", day);
    gotoxy(0, 1);
    printf("--------------------");
}

void ui_text_box(const char *s)
{
    ui_clear();
    printf("%s", s);
}

uint8_t ui_menu(const char *title, const char *options[], uint8_t count, uint8_t selected)
{
    if (selected >= count)
        selected = 0;

    // Draw static parts once
    ui_clear();
    gotoxy(0, 0);
    printf("%s", title);
    for (uint8_t i = 0; i < count; ++i)
    {
        gotoxy(2, 2 + i); // leave col 0 for the selector
        printf("%s", options[i]);
    }

    // Draw initial cursor
    uint8_t prev = 0xFF;
    while (1)
    {
        if (selected != prev)
        {
            // erase old cursor
            if (prev != 0xFF)
            {
                gotoxy(0, 2 + prev);
                printf(" ");
            }
            // draw new cursor
            gotoxy(0, 2 + selected);
            putchar(CURSOR_CHAR);
            prev = selected;
        }

        uint8_t j = joypad();
        if (j & J_UP)
        {
            if (selected > 0)
                selected--;
            waitpadup(); // debounce
        }
        else if (j & J_DOWN)
        {
            if (selected + 1 < count)
                selected++;
            waitpadup();
        }
        else if (j & J_A)
        {
            waitpadup();
            return selected;
        }

        wait_vbl_done(); // 1 frame
    }
}
