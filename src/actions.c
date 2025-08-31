#include <gbdk/platform.h> // MUST be first
#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>

#include "actions.h"
#include "ui.h"
#include "inventory.h"
#include "items.h"
#include "expedition.h"
#include "rng.h"

// How many survivors are inside right now (alive && not away)
static uint8_t count_in_bunker(Game *g)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        if (g->team[i].alive && !g->away[i])
            n++;
    }
    return n;
}

void ui_clear_soft(void)
{
    for (uint8_t y = 0; y < 18; ++y)
    {
        gotoxy(0, y);
        printf("                    "); // 20 spaces
    }
    gotoxy(0, 0);
}

static uint8_t sat_add_cap_u8(uint8_t base, uint8_t add, uint8_t cap)
{
    uint16_t s = (uint16_t)base + (uint16_t)add;
    if (s > cap)
        s = cap;
    return (uint8_t)s;
}

// Which gear a row represents
typedef enum
{
    G_MAP = 0,
    G_AXE,
    G_FILTER,
    G_BATTERY
} GearId;

static uint8_t gear_mark(const ExpeditionPlan *p, GearId id)
{
    switch (id)
    {
    case G_MAP:
        return p->use_map;
    case G_AXE:
        return p->use_axe;
    case G_FILTER:
        return p->use_filter;
    case G_BATTERY:
        return p->use_battery;
    }
    return 0;
}
static void gear_toggle(ExpeditionPlan *p, GearId id)
{
    switch (id)
    {
    case G_MAP:
        p->use_map ^= 1;
        break;
    case G_AXE:
        p->use_axe ^= 1;
        break;
    case G_FILTER:
        p->use_filter ^= 1;
        break;
    case G_BATTERY:
        p->use_battery ^= 1;
        break;
    }
}

static uint8_t have_weapon(Game *g)
{
    // Treat AXE as “weapon” for practice; extend as you add more
    return inv_count_type(g->inv, MAX_ITEMS, IT_AXE) ? 1 : 0;
}

void actions_day_reset(Game *g)
{
    g->radio_attempted_today = 0;
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
        g->acted_today[i] = 0;
}

/* ---------- Survivor picking (only alive, not away, not acted) ---------- */
static int8_t pick_available_survivor(Game *g)
{
    const char *opts[MAX_SURVIVORS + 1];
    uint8_t map[MAX_SURVIVORS];
    uint8_t n = 0;

    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        if (!g->team[i].alive)
            continue;
        if (g->away[i])
            continue;
        if (g->acted_today[i])
            continue;
        opts[n] = g->team[i].pname; // just show the name
        map[n] = i;
        n++;
    }

    if (n == 0)
    {
        ui_text_box("No one available.");
        ui_wait();
        return -1;
    }

    opts[n] = "Done"; // last entry ends the actions phase
    uint8_t pick = ui_menu("Actions for:", opts, n + 1, 0);
    if (pick == n)
        return -1; // Done
    return (int8_t)map[pick];
}

/* ---------- Actions for a specific survivor ---------- */
static void do_play(Game *g, uint8_t who)
{
    (void)g;
    if (g->team[who].morale <= 97)
        g->team[who].morale += 3;
    else
        g->team[who].morale = 100;
    ui_text_box("Was fun.\nMorale +3");
    ui_wait();
}

static void do_practice(Game *g, uint8_t who)
{
    (void)who;
    // one-time safer expedition buff
    g->combat_training[who] = 1;
    ui_text_box("Practice complete.\nNext expedition safer.");
    ui_wait();
}

static void do_try_radio(Game *g, uint8_t who)
{
    (void)who;

    // costs 1 battery attempt
    (void)inv_take(g->inv, MAX_ITEMS, IT_BATTERY, 1);

    uint8_t parts = inv_count_type(g->inv, MAX_ITEMS, IT_RADIO_PART);
    // +5% per part, capped at 20% total
    uint8_t chance = sat_add_cap_u8(10u, (uint8_t)(parts * 10u), 20u);

    if ((rng_u8() % 100u) < chance)
    {
        if (g->radio_progress < 5)
            g->radio_progress++;
        ui_text_box("Radio progress +1!");
    }
    else
    {
        ui_text_box("No luck fixing it.");
    }
    ui_wait();
}
// actions.c
#define CURSOR_CHAR '>'

static void draw_gear_screen(Game *g, uint8_t who, const ExpeditionPlan *p, uint8_t row)
{
    // Use a soft clear (no DISPLAY_OFF/ON). If you don't have this helper,
    // swap for your ui_clear() and it will still work — just less pretty.
    ui_clear_soft();

    printf("Gear for %s\n", g->team[who].pname);
    printf("--------------------\n");

    uint8_t has_map = inv_count_type(g->inv, MAX_ITEMS, IT_MAP);
    uint8_t has_axe = inv_count_type(g->inv, MAX_ITEMS, IT_AXE);
    uint8_t has_filter = inv_count_type(g->inv, MAX_ITEMS, IT_FILTER);
    uint8_t has_bat = inv_count_type(g->inv, MAX_ITEMS, IT_BATTERY);

    gotoxy(0, 3);
    putchar(row == 0 ? CURSOR_CHAR : ' ');
    printf(" Map    [");
    putchar(p->use_map ? 'X' : ' ');
    printf("] (%u)\n", (unsigned)has_map);
    gotoxy(0, 4);
    putchar(row == 1 ? CURSOR_CHAR : ' ');
    printf(" Axe    [");
    putchar(p->use_axe ? 'X' : ' ');
    printf("] (%u)\n", (unsigned)has_axe);
    gotoxy(0, 5);
    putchar(row == 2 ? CURSOR_CHAR : ' ');
    printf(" Filter [");
    putchar(p->use_filter ? 'X' : ' ');
    printf("] (%u)\n", (unsigned)has_filter);
    gotoxy(0, 6);
    putchar(row == 3 ? CURSOR_CHAR : ' ');
    printf(" Battery[");
    putchar(p->use_battery ? 'X' : ' ');
    printf("] (%u)\n", (unsigned)has_bat);

    gotoxy(0, 8);
    putchar(row == 4 ? CURSOR_CHAR : ' ');
    printf(" Send\n");
    printf("\nA=select  B=back");
}

// Draw once when dirty: shows only rows for gear you have (>0)
static void draw_gear_dynamic(Game *g, uint8_t who,
                              const ExpeditionPlan *p,
                              const GearId *ids, const uint8_t *counts, uint8_t n,
                              uint8_t row)
{
    ui_clear_soft();

    printf("Gear for %s\n", g->team[who].pname);
    printf("--------------------\n");

    if (n == 0)
    {
        // No items to show
        gotoxy(0, 3);
        printf("No gear available.");
        uint8_t send_y = 5;
        gotoxy(0, send_y);
        putchar(row == 0 ? '>' : ' ');
        printf(" Send\n");
        printf("\nA=send  B=back");
        return;
    }

    // List each available gear (>0 in inventory)
    for (uint8_t i = 0; i < n; ++i)
    {
        uint8_t y = 3 + i;
        const char *label = "Item";
        switch (ids[i])
        {
        case G_MAP:
            label = "Map";
            break;
        case G_AXE:
            label = "Axe";
            break;
        case G_FILTER:
            label = "Filter";
            break;
        case G_BATTERY:
            label = "Battery";
            break;
        }
        gotoxy(0, y);
        putchar(row == i ? '>' : ' ');
        printf(" %s  [", label);
        putchar(gear_mark(p, ids[i]) ? 'X' : ' ');
        printf("] (%u)\n", (unsigned)counts[i]);
    }

    // "Send" row
    uint8_t send_y = 3 + n;
    gotoxy(0, send_y);
    putchar(row == n ? '>' : ' ');
    printf(" Send\n");

    printf("\nA=toggle/send  B=back");
}

static uint8_t gear_menu_for(Game *g, uint8_t who, ExpeditionPlan *out_plan)
{
    ExpeditionPlan plan;
    memset(&plan, 0, sizeof(plan));
    plan.survivor_idx = who;

    // Build the list of gear we actually have (>0). Rebuilt on each redraw.
    GearId ids[4];
    uint8_t counts[4];
    uint8_t n = 0;

    // current counts
    uint8_t c_map = inv_count_type(g->inv, MAX_ITEMS, IT_MAP);
    uint8_t c_axe = inv_count_type(g->inv, MAX_ITEMS, IT_AXE);
    uint8_t c_filter = inv_count_type(g->inv, MAX_ITEMS, IT_FILTER);
    uint8_t c_bat = inv_count_type(g->inv, MAX_ITEMS, IT_BATTERY);

    if (c_map)
    {
        ids[n] = G_MAP;
        counts[n] = c_map;
        n++;
    }
    if (c_axe)
    {
        ids[n] = G_AXE;
        counts[n] = c_axe;
        n++;
    }
    if (c_filter)
    {
        ids[n] = G_FILTER;
        counts[n] = c_filter;
        n++;
    }
    if (c_bat)
    {
        ids[n] = G_BATTERY;
        counts[n] = c_bat;
        n++;
    }

    uint8_t row = 0, prev_row = 0xFF;
    uint8_t dirty = 1;

    for (;;)
    {
        // Recompute counts/ids if needed (e.g., if you want them to reflect changes while open)
        if (dirty)
        {
            // (Optional) Rebuild the arrays here if your inventory can change while in this menu.
            // For now we keep the initial snapshot; it’s cheaper and stable.
        }

        if (dirty || row != prev_row)
        {
            draw_gear_dynamic(g, who, &plan, ids, counts, n, row);
            prev_row = row;
            dirty = 0;
        }

        uint8_t j = joypad();
        uint8_t max_row = n; // last row index = Send
        if (j & J_UP)
        {
            if (row > 0)
                row--;
            waitpadup();
        }
        else if (j & J_DOWN)
        {
            if (row < max_row)
                row++;
            waitpadup();
        }
        else if (j & J_B)
        {
            waitpadup();
            return 0;
        }
        else if (j & J_A)
        {
            uint8_t max_row = n; // last row index = Send (so 0 when n==0)
            if (j & J_UP)
            {
                if (row > 0)
                    row--;
                waitpadup();
            }
            else if (j & J_DOWN)
            {
                if (row < max_row)
                    row++;
                waitpadup();
            }
            else if (j & J_B)
            {
                waitpadup();
                return 0;
            }
            else if (j & J_A)
            {
                waitpadup();
                if (row == max_row)
                { // Send
                    // sanitize toggles vs current inventory snapshot
                    if (plan.use_map && !c_map)
                        plan.use_map = 0;
                    if (plan.use_axe && !c_axe)
                        plan.use_axe = 0;
                    if (plan.use_filter && !c_filter)
                        plan.use_filter = 0;
                    if (plan.use_battery && !c_bat)
                        plan.use_battery = 0;
                    *out_plan = plan;
                    return 1;
                }
                else
                {
                    gear_toggle(&plan, ids[row]); // will never run when n==0
                    dirty = 1;
                }
            }
        }
        wait_vbl_done();
    }
}

/* Let the picked survivor choose ONE action; returns 1 if they used an action */
static uint8_t run_action_menu_for_survivor(Game *g, uint8_t who)
{
    const char *opts[5];
    uint8_t act[5];
    uint8_t n = 0;

    enum
    {
        ACT_PLAY = 0,
        ACT_PRACTICE,
        ACT_TRYRADIO,
        ACT_SEND,
        ACT_BACK
    };

    uint8_t allow_send = (count_in_bunker(g) > 1u); // at least 2 inside now?

    opts[n] = "Play";
    act[n++] = ACT_PLAY;
    if (have_weapon(g))
    {
        opts[n] = "Practice";
        act[n++] = ACT_PRACTICE;
    }
    if (g->radio_attempted_today == 0)
    {
        opts[n] = "Try radio";
        act[n++] = ACT_TRYRADIO;
    }
    if (allow_send)
    {
        opts[n] = "Send out";
        act[n++] = ACT_SEND;
    }
    opts[n] = "Back";
    act[n++] = ACT_BACK;

    uint8_t pick = ui_menu(g->team[who].pname, opts, n, 0);

    switch (act[pick])
    {
    case ACT_PLAY:
        do_play(g, who);
        g->acted_today[who] = 1;
        return 1;
    case ACT_PRACTICE:
        do_practice(g, who);
        g->acted_today[who] = 1;
        return 1;
    case ACT_TRYRADIO:
        do_try_radio(g, who);
        g->radio_attempted_today = 1;
        g->acted_today[who] = 1;
        return 1;

    case ACT_SEND:
    {
        // Extra safety: re-check right before sending (state may have changed)
        if (count_in_bunker(g) <= 1u)
        {
            ui_text_box("1 must stay in.");
            ui_wait();
            return 0; // not marked as acted
        }
        ExpeditionPlan plan;
        uint8_t send = gear_menu_for(g, who, &plan);
        if (send)
        {
            uint8_t days_out = 2 + rng_range(2); // 2..4 days
            expedition_start(g, &plan, days_out);
            g->acted_today[who] = 1;
            return 1;
        }
        return 0;
    }

    case ACT_BACK:
    default:
        return 0;
    }
}

/* Public entry: choose survivor first, then action; one action per survivor */
void actions_run_menu(Game *g)
{
    while (1)
    {
        int8_t who = pick_available_survivor(g);
        if (who < 0)
            return; // Done / none available
        (void)run_action_menu_for_survivor(g, (uint8_t)who);
        // Loop continues so the player can assign actions to other survivors.
        // The chosen one is now locked for today via acted_today[].
    }
}
