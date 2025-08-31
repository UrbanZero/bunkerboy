#include <gbdk/platform.h>
#include <gb/gb.h>
#include <gbdk/console.h> // gotoxy, cls...
#include <stdio.h>
#include <string.h>

#include "rations.h"
#include "ui.h"
#include "inventory.h"
#include "items.h"
#include "game.h"

#define CURSOR_CHAR '>'
// Columns we will use on the 20-col console
#define COL_CURSOR 0
#define COL_NAME 1 // width 5
#define COL_F 8
#define COL_W 12
#define COL_H 16

static void plan_zero(RationPlan *p)
{
    memset(p, 0, sizeof(*p));
}

static void print_padded_name(const char *s, uint8_t width)
{
    uint8_t n = 0;
    while (*s && n < width)
    {
        putchar(*s++);
        n++;
    }
    while (n++ < width)
        putchar(' ');
}

static void plan_totals(const RationPlan *p, uint8_t *f, uint8_t *w, uint8_t *m, uint8_t hf, uint8_t hw, uint8_t hm)
{
    uint8_t tf = 0, tw = 0, tm = 0;
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        tf += hf > tf ? p->food[i] : 0;
        tw += hw > tw ? p->water[i] : 0;
        tm += hm > tm ? p->med[i] : 0;
    }
    *f = tf;
    *w = tw;
    *m = tm;
}

unsigned dig_count(const unsigned n)
{
    if (n < 10)
        return 1;
    return 1 + dig_count(n / 10);
}

static void draw_screen(Game *g, const RationPlan *p, uint8_t row, uint8_t field)
{
    ui_clear();
    ui_icons_refresh();
    // Top HUD: inventory and plan totals
    uint8_t haveF = inv_count_type(g->inv, MAX_ITEMS, IT_FOOD);
    uint8_t haveW = inv_count_type(g->inv, MAX_ITEMS, IT_WATER);
    uint8_t haveM = inv_count_type(g->inv, MAX_ITEMS, IT_MEDKIT);
    uint8_t needF, needW, needM;
    plan_totals(p, &needF, &needW, &needM, haveF, haveW, haveM);

    gotoxy(0, 0);
    printf("Got    %u  %u  %u", haveF, haveW, haveM);
    ui_put_icon(6, 0, (uint8_t)ICON_FOOD);
    ui_put_icon(9, 0, (uint8_t)ICON_WATER);
    ui_put_icon(12, 0, (uint8_t)ICON_MED);

    gotoxy(0, 1);
    printf("Use    %u  %u  %u", needF, needW, needM);
    ui_put_icon(6, 1, (uint8_t)ICON_FOOD);
    ui_put_icon(9, 1, (uint8_t)ICON_WATER);
    ui_put_icon(12, 1, (uint8_t)ICON_MED);

    // Table header
    gotoxy(0, 2);
    printf("--------------------");

    // Rows: each survivor
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        uint8_t y = 3 + i;

        if (!g->team[i].alive)
        {
            gotoxy(COL_CURSOR, y);
            putchar(' ');
            gotoxy(COL_NAME, y);
            print_padded_name(g->team[i].pname, 4);
            gotoxy(COL_F, y);
            printf("DEAD");
            continue;
        }
        if (g->away[i])
        {
            gotoxy(COL_CURSOR, y);
            putchar((row == i) ? CURSOR_CHAR : ' ');
            gotoxy(COL_NAME, y);
            print_padded_name(g->team[i].pname, 4);
            // gotoxy(COL_T, y);
            // printf(" D%u", (unsigned)g->away_days[i]);
            continue;
        }
        char fmark = p->food[i] ? '*' : ' ';
        char wmark = p->water[i] ? '*' : ' ';
        char mmark = p->med[i] ? '*' : ' ';

        // Cursor + name
        gotoxy(COL_CURSOR, y);
        putchar((row == i) ? CURSOR_CHAR : ' ');
        gotoxy(COL_NAME, y);
        print_padded_name(g->team[i].pname, 4);

        ui_put_icon(COL_H - 1, y, (uint8_t)(ICON_BAR_1 + 6u - g->team[i].health));
        ui_put_icon(COL_H, y, (uint8_t)ICON_MED);
        gotoxy(COL_H + 1, y);
        putchar(mmark);

        ui_put_icon(COL_F - 1, y, (uint8_t)(ICON_BAR_1 + 6u - g->team[i].hunger));
        ui_put_icon(COL_F, y, (uint8_t)ICON_FOOD);
        gotoxy(COL_F + 1, y);
        putchar(fmark);

        ui_put_icon(COL_W - 1, y, (uint8_t)(ICON_BAR_1 + 6u - g->team[i].thirst));
        ui_put_icon(COL_W, y, (uint8_t)ICON_WATER);
        gotoxy(COL_W + 1, y);
        putchar(wmark);

        // Underline the active field under the mark column for the selected row
        if (row == i)
        {
            uint8_t ulx = (field == 0) ? COL_F + 1 : (field == 1) ? COL_W + 1
                                                                  : COL_H + 1;
            gotoxy(ulx, 3 + MAX_SURVIVORS);
            putchar('^');
        }
    }
    // Continue row
    gotoxy(0, 3 + MAX_SURVIVORS);
    putchar((row == MAX_SURVIVORS) ? CURSOR_CHAR : ' ');
    printf("OK");
    gotoxy(0, 5 + MAX_SURVIVORS);
    printf("\n^v<>:move\nA:toggle");
    gotoxy(0, 6 + MAX_SURVIVORS);
}

void rations_menu(Game *g, RationPlan *out_plan)
{
    RationPlan plan;
    plan_zero(&plan);

    uint8_t row = 0;   // 0..MAX_SURVIVORS, where last is "Continue"
    uint8_t field = 0; // 0=Food, 1=Water, 2=Med
    draw_screen(g, &plan, row, field);

    while (1)
    {
        // Yeah twice
        uint8_t haveF = inv_count_type(g->inv, MAX_ITEMS, IT_FOOD);
        uint8_t haveW = inv_count_type(g->inv, MAX_ITEMS, IT_WATER);
        uint8_t haveM = inv_count_type(g->inv, MAX_ITEMS, IT_MEDKIT);
        uint8_t needF, needW, needM;
        plan_totals(&plan, &needF, &needW, &needM, haveF, haveW, haveM);

        uint8_t j = joypad();
        if (j & J_UP)
        {
            if (row > 0)
                row--;
            waitpadup();
            draw_screen(g, &plan, row, field);
        }
        else if (j & J_DOWN)
        {
            if (row < MAX_SURVIVORS)
                row++;
            waitpadup();
            draw_screen(g, &plan, row, field);
        }
        else if (j & J_LEFT)
        {
            if (row < MAX_SURVIVORS && field > 0)
            {
                field--;
                waitpadup();
                draw_screen(g, &plan, row, field);
            }
        }
        else if (j & J_RIGHT)
        {
            if (row < MAX_SURVIVORS && field < 2)
            {
                field++;
                waitpadup();
                draw_screen(g, &plan, row, field);
            }
        }
        else if (j & J_A)
        {
            if (row == MAX_SURVIVORS)
            {
                // Confirm
                *out_plan = plan;
                waitpadup();
                return;
            }
            else
            {
                // Toggle selected field for this survivor
                if (!g->team[row].alive || g->away[row])
                {
                    waitpadup();
                    continue;
                }

                switch (field)
                {
                case 0:
                    plan.food[row] ^= needF < haveF || plan.food[row] == 1 ? 1 : 0;
                    break;
                case 1:
                    plan.water[row] ^= needW < haveW || plan.water[row] == 1 ? 1 : 0;
                    break;
                case 2:
                    plan.med[row] ^= needM < haveM || plan.med[row] == 1 ? 1 : 0;
                    break;
                }
                waitpadup();
                draw_screen(g, &plan, row, field);
            }
        }
        wait_vbl_done();
    }
}
