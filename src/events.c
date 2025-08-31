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
    {
        "Muffled Broadcast\n"
        "Try to decode it?",
        "Try with 1 water", "Ignore",
        /*A*/ 0, -1, "Unlucky",
        /*B*/ 0, 0, "Ignored",
        8 // W
    },
    {
        "Muffled Broadcast\n"
        "Try to decode it?",
        "Try with 1 water", "Ignore",
        /*A*/ 0, 1, "Lucky! Got 2 water.",
        /*B*/ 0, 0, "Ignored.",
        8 // W
    },
    {
        "Unexpected Visitor\n"
        "Trader or raider?\n",
        "1Water->2Food", "Refuse",
        /*A*/ +2, -1, "Traded succesfully!",
        /*B*/ 0, 0, "You did not open",
        8 // W
    },
    {
        "Unexpected Visitor\n"
        "Trader or raider?\n",
        "1Water->2Food", "Refuse",
        /*A*/ 0, -1, "You were raided 1 Water",
        /*B*/ 0, 0, "You did not open",
        8 // W
    },
    {
        "Unexpected Visitor\n"
        "Trader or raider?\n",
        "1Food->2Water", "Refuse",
        /*A*/ -1, +2, "Traded succesfully!",
        /*B*/ 0, 0, "You did not open",
        8 // W
    },
    {
        "Unexpected Visitor\n"
        "Trader or raider?\n",
        "1Food->2Water", "Refuse",
        /*A*/ -1, 0, "You were raided 1 Food",
        /*B*/ 0, 0, "You did not open",
        8 // W
    },
};
static const uint8_t EVS_COUNT = sizeof(EVS) / sizeof(EVS[0]);

void events_init(void) {}

// helpers (keep these in events.c)
static uint8_t have_enough(struct Game *g, int8_t df, int8_t dw)
{
    uint8_t needF = (df < 0) ? (uint8_t)(-df) : 0;
    uint8_t needW = (dw < 0) ? (uint8_t)(-dw) : 0;
    if (needF && inv_count_type(g->inv, MAX_ITEMS, IT_FOOD) < needF)
        return 0;
    if (needW && inv_count_type(g->inv, MAX_ITEMS, IT_WATER) < needW)
        return 0;
    return 1;
}

static uint8_t event_is_eligible(struct Game *g, const EventDef *ev)
{
    uint8_t a_consumes = (ev->delta_food_a < 0) || (ev->delta_water_a < 0);
    uint8_t b_consumes = (ev->delta_food_b < 0) || (ev->delta_water_b < 0);

    if (a_consumes && have_enough(g, ev->delta_food_a, ev->delta_water_a))
        return 1;
    if (b_consumes && have_enough(g, ev->delta_food_b, ev->delta_water_b))
        return 1;

    // If neither option consumes anything, allow it
    if (!a_consumes && !b_consumes)
        return 1;

    return 0;
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
    int8_t df = (choice_idx == 0) ? ev->delta_food_a : ev->delta_food_b;
    int8_t dw = (choice_idx == 0) ? ev->delta_water_a : ev->delta_water_b;

    // apply deltas
    if (df < 0)
        inv_consume(g, IT_FOOD, (uint8_t)(-df));
    else
        while (df-- > 0)
        {
            // add food
            for (uint8_t i = 0; i < MAX_ITEMS; i++)
            {
                if (g->inv[i].type == IT_NONE || (g->inv[i].type == IT_FOOD && g->inv[i].qty < 250))
                {
                    if (g->inv[i].type == IT_NONE)
                        g->inv[i].type = IT_FOOD;
                    g->inv[i].qty++;
                    break;
                }
            }
        }

    if (dw < 0)
        inv_consume(g, IT_WATER, (uint8_t)(-dw));
    else
        while (dw-- > 0)
        {
            for (uint8_t i = 0; i < MAX_ITEMS; i++)
            {
                if (g->inv[i].type == IT_NONE || (g->inv[i].type == IT_WATER && g->inv[i].qty < 250))
                {
                    if (g->inv[i].type == IT_NONE)
                        g->inv[i].type = IT_WATER;
                    g->inv[i].qty++;
                    break;
                }
            }
        }
    ui_clear();
    gotoxy(0, 0);
    printf("%s", (choice_idx == 0) ? ev->res_a : ev->res_b);
    ui_wait();
}
