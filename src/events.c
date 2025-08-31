#include <gbdk/platform.h>
#include <gb/gb.h>
#include <gbdk/console.h> // <-- correct header for gotoxy, cls, etc.
#include <gbdk/font.h>
#include <stdio.h>
#include <string.h>

#include "events.h"
#include "rng.h"
#include "ui.h"
#include "inventory.h"
#include "items.h"

static const EventDef EVS[] = {
    {"Muffled Broadcast\n"
     "Try to decode it?",
     "Try with 1 water", "Ignore",
     /*A*/ 0, -1, "Unlucky",
     /*B*/ 0, 0, "Ignored",
     8, // W
     /*F_a*/ 0,
     /*W_a*/ 1, /*F_b*/ 0, /*W_b*/ 0},
    {"Muffled Broadcast\n"
     "Try to decode it?",
     "Try with 1 water", "Ignore",
     /*A*/ 0, 1, "Lucky! Got 2 water.",
     /*B*/ 0, 0, "Ignored.",
     8, // W
     /*F_a*/ 0,
     /*W_a*/ 1, /*F_b*/ 0, /*W_b*/ 0},
    {"Unexpected Visitor\n"
     "Trader or raider?\n",
     "1Water->2Food", "Refuse",
     /*A*/ +2, -1, "Traded succesfully!",
     /*B*/ 0, 0, "You did not open",
     8, // W
     /*F_a*/ 0,
     /*W_a*/ 1, /*F_b*/ 0, /*W_b*/ 0},
    {"Unexpected Visitor\n"
     "Trader or raider?\n",
     "1Water->2Food", "Refuse",
     /*A*/ 0, -1, "You were raided 1 Water",
     /*B*/ 0, 0, "You did not open",
     8, // W
     /*F_a*/ 0,
     /*W_a*/ 1, /*F_b*/ 0, /*W_b*/ 0},
    {"Unexpected Visitor\n"
     "Trader or raider?\n",
     "1Food->2Water", "Refuse",
     /*A*/ -1, +2, "Traded succesfully!",
     /*B*/ 0, 0, "You did not open",
     8, // W
     /*F_a*/ 1,
     /*W_a*/ 0, /*F_b*/ 0, /*W_b*/ 0},
    {"Unexpected Visitor\n"
     "Trader or raider?\n",
     "1Food->2Water", "Refuse",
     /*A*/ -1, 0, "You were raided 1 Food",
     /*B*/ 0, 0, "You did not open",
     8, // W
     /*F_a*/ 1,
     /*W_a*/ 0, /*F_b*/ 0, /*W_b*/ 0},
};
static const uint8_t EVS_COUNT = sizeof(EVS) / sizeof(EVS[0]);

void events_init(void) {}

// helpers (keep these in events.c)
static uint8_t can_afford_choice(Game *g, uint8_t nf, uint8_t nw, int8_t df, int8_t dw)
{
    uint8_t needF = nf ? nf : (df < 0 ? (uint8_t)(-df) : 0);
    uint8_t needW = nw ? nw : (dw < 0 ? (uint8_t)(-dw) : 0);
    if (needF && inv_count_type(g->inv, MAX_ITEMS, IT_FOOD) < needF)
        return 0;
    if (needW && inv_count_type(g->inv, MAX_ITEMS, IT_WATER) < needW)
        return 0;
    return 1;
}

static uint8_t event_is_eligible(Game *g, const EventDef *ev)
{
    uint8_t a_ok = can_afford_choice(g, ev->need_food_a, ev->need_water_a, ev->delta_food_a, ev->delta_water_a);
    uint8_t b_ok = can_afford_choice(g, ev->need_food_b, ev->need_water_b, ev->delta_food_b, ev->delta_water_b);
    // Show event if at least one choice is do-able AND that choice actually *does something*
    uint8_t a_does = ev->need_food_a || ev->need_water_a || ev->delta_food_a || ev->delta_water_a;
    uint8_t b_does = ev->need_food_b || ev->need_water_b || ev->delta_food_b || ev->delta_water_b;
    return (a_ok && a_does) || (b_ok && b_does);
}

const EventDef *events_pick_for_day(struct Game *g, uint16_t day)
{
    (void)day;

    // Pass 1: count eligible
    uint8_t eligible = 0;
    for (uint8_t i = 0; i < EVS_COUNT; ++i)
        if (event_is_eligible(g, &EVS[i]))
            eligible++;

    if (!eligible)
        return 0; // no event today

    // Pick kth eligible (k in [0 .. eligible-1])
    uint8_t k = rng_range((uint8_t)(eligible - 1));

    // Pass 2: return the kth eligible
    for (uint8_t i = 0; i < EVS_COUNT; ++i)
    {
        if (event_is_eligible(g, &EVS[i]))
        {
            if (k == 0)
                return &EVS[i];
            k--;
        }
    }
    return 0; // shouldn't happen
}

void events_resolve(Game *g, const EventDef *ev, uint8_t choice_idx)
{
    // Resolve costs
    uint8_t needF = choice_idx ? ev->need_food_b : ev->need_food_a;
    uint8_t needW = choice_idx ? ev->need_water_b : ev->need_water_a;
    // Re-check and consume (guard)
    if (needF && inv_count_type(g->inv, MAX_ITEMS, IT_FOOD) < needF)
    {
        ui_text_box("Not enough Food.");
        ui_wait();
        return;
    }
    if (needW && inv_count_type(g->inv, MAX_ITEMS, IT_WATER) < needW)
    {
        ui_text_box("Not enough Water.");
        ui_wait();
        return;
    }
    if (needF)
        inv_consume(g, IT_FOOD, needF);
    if (needW)
        inv_consume(g, IT_WATER, needW);

    // Apply deltas (can be +/-)
    int8_t df = choice_idx ? ev->delta_food_b : ev->delta_food_a;
    int8_t dw = choice_idx ? ev->delta_water_b : ev->delta_water_a;

    if (df < 0)
        inv_consume(g, IT_FOOD, (uint8_t)(-df));
    else
        while (df-- > 0)
            inv_add(g->inv, MAX_ITEMS, IT_FOOD, 1);

    if (dw < 0)
        inv_consume(g, IT_WATER, (uint8_t)(-dw));
    else
        while (dw-- > 0)
            inv_add(g->inv, MAX_ITEMS, IT_WATER, 1);

    ui_clear();
    gotoxy(0, 0);
    printf("%s", choice_idx ? ev->res_b : ev->res_a);
    ui_wait();
}