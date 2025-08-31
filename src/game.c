#include <gbdk/platform.h>
#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "rations.h"
#include "inventory.h"
#include "items.h"
#include "ui.h"

#define MIN_HEALTH 6
#define MIN_HUNGER 6
#define MIN_THIRST 6

RationPlan plan;
Game G;
static const char *DEFAULT_NAMES[MAX_SURVIVORS] = {"Mike", "Rose", "Jeff"};

static void _init_team(Game *g)
{
    memset(g, 0, sizeof(*g));
    g->day = 0;
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        g->team[i].alive = 1;
        g->team[i].health = 1;
        g->team[i].hunger = 1;
        g->team[i].thirst = 1;
        g->team[i].morale = 75;
        g->team[i].pname = DEFAULT_NAMES[i];
    }
    inv_clear(g->inv, MAX_ITEMS); // <- start empty, add using the crate choice

    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        g->away[i] = 0;
        g->away_days[i] = 0;
        g->away_map[i] = g->away_axe[i] = g->away_filter[i] = g->away_battery[i] = 0;
        g->acted_today[i] = 0;
        g->combat_training[i] = 0;
    }
    g->radio_attempted_today = 0;
}

void game_init(void)
{
    _init_team(&G);
}

void game_new_run(void)
{
    _init_team(&G);
}

uint8_t inv_count(Game *g, ItemType t)
{
    return inv_count_type(g->inv, MAX_ITEMS, t);
}
bool inv_consume(Game *g, ItemType t, uint8_t n)
{
    return inv_take(g->inv, MAX_ITEMS, t, n);
}
void game_apply_day_end(Game *g, const RationPlan *plan)
{
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        Survivor *s = &g->team[i];
        if (!s->alive)
            continue;

        if (g->away[i])
        {
            /* Outside today — no home rations or penalties here.
               Their risks/flags are applied on return resolution. */
            continue;
        }

        // --- FOOD (unchanged) ---
        if (plan->food[i])
        {
            if (inv_take(g->inv, MAX_ITEMS, IT_FOOD, 1))
            {
                if (s->hunger > 0)
                    s->hunger--;
            }
            else
            {
                if (s->hunger < MIN_HUNGER)
                    s->hunger++;
                else if (s->health < MIN_HEALTH)
                    s->health++;
            }
        }
        else
        {
            if (s->hunger < MIN_HUNGER)
                s->hunger++;
            else if (s->health < MIN_HEALTH)
                s->health++;
        }

        // --- WATER (unchanged) ---
        if (plan->water[i])
        {
            if (inv_take(g->inv, MAX_ITEMS, IT_WATER, 1))
            {
                if (s->thirst > 0)
                    s->thirst--;
            }
            else
            {
                if (s->thirst < MIN_THIRST)
                    s->thirst++;
                else if (s->health < MIN_HEALTH)
                    s->health++;
            }
        }
        else
        {
            if (s->thirst < MIN_THIRST)
                s->thirst++;
            else if (s->health < MIN_HEALTH)
                s->health++;
        }

        // --- MED (only if toggled) ---
        if (plan->med[i] && (s->sick || s->injured))
        {
            if (inv_take(g->inv, MAX_ITEMS, IT_MEDKIT, 1))
            {
                s->sick = 0;
                s->injured = 0;
                for (int i = 0; i < MIN_HEALTH; i++)
                {
                    if (s->health > 0)
                        s->health--;
                }
            }
        }
        /* If they are still sick/injured after med attempt, HP -1 */
        if ((s->sick || s->injured) && !plan->med[i])
        {
            if (s->health < MIN_HEALTH)
                s->health++;
        }
    }

    // Death check (unchanged)
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        Survivor *s = &g->team[i];
        if (s->alive && s->health == MIN_HEALTH)
        {
            ui_clear();
            printf("%s died at night.", s->pname);
            ui_wait();
            s->alive = 0;
        }
    }
}
bool game_is_over(const Game *g)
{
    uint8_t alive = 0;
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
        if (g->team[i].alive)
            alive++;
    return (alive == 0) || (g->radio_progress >= 5); // e.g., rescued at 5 signals
}
