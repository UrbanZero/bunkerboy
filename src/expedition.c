#include <gbdk/platform.h> // MUST be first
#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>

#include "expedition.h"
#include "game.h"
#include "ui.h"
#include "inventory.h"
#include "rng.h"

#ifdef gotoxy
#undef gotoxy
#endif

#define CURSOR_CHAR '>'

// ---------- helpers ----------
static void print_padded(const char *s, uint8_t width)
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

static uint8_t have_item(struct Game *g, ItemType t)
{
    return inv_count_type(g->inv, MAX_ITEMS, t);
}

// ---------- UI ----------
static void draw_pick_survivor(struct Game *g, uint8_t row)
{
    ui_clear();
    gotoxy(0, 0);
    printf("Pick explorer");
    gotoxy(0, 1);
    printf("--------------------");
    const uint8_t pad = 7u;
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        gotoxy(0, 2 + i);
        putchar((row == i) ? CURSOR_CHAR : ' ');
        if (!g->team[i].alive)
        {
            print_padded("(dead)", pad);
            printf("  ");
        }
        else
        {
            print_padded(g->team[i].pname, pad);
            ui_put_icon(pad + 2u, i + 2u, (uint8_t)(ICON_BAR_1 + 6u - g->team[i].hunger));
            ui_put_icon(pad + 3u, i + 2u, (uint8_t)ICON_FOOD);
            ui_put_icon(pad + 5u, i + 2u, (uint8_t)(ICON_BAR_1 + 6u - g->team[i].thirst));
            ui_put_icon(pad + 6u, i + 2u, (uint8_t)ICON_WATER);
            ui_put_icon(pad + 8u, i + 2u, (uint8_t)(ICON_BAR_1 + 6u - g->team[i].health));
            ui_put_icon(pad + 9u, i + 2u, (uint8_t)ICON_MED);
            // LATER
            if (g->team[i].injured)
                printf(" Inj");
            if (g->team[i].sick)
                printf(" Sick");
        }
    }
    gotoxy(0, 2 + MAX_SURVIVORS);
    putchar((row == MAX_SURVIVORS) ? CURSOR_CHAR : ' ');
    printf(" Cancel");
}

static void draw_pick_gear(struct Game *g, const ExpeditionPlan *p, uint8_t row)
{
    ui_clear();
    gotoxy(0, 0);
    printf("Gear A toggle");
    gotoxy(0, 1);
    printf("\n");

    uint8_t has_map = have_item(g, IT_MAP);
    uint8_t has_axe = have_item(g, IT_AXE);
    uint8_t has_filter = have_item(g, IT_FILTER);
    uint8_t bat_cnt = have_item(g, IT_BATTERY);

    const char mark_map = p->use_map ? '*' : ' ';
    const char mark_axe = p->use_axe ? '*' : ' ';
    const char mark_filter = p->use_filter ? '*' : ' ';
    const char mark_bat = p->use_battery ? '*' : ' ';

    gotoxy(0, 2);
    putchar((row == 0) ? CURSOR_CHAR : ' ');
    printf(" Map    [");
    putchar(mark_map);
    printf("]  (%u)", (unsigned)has_map);
    gotoxy(0, 3);
    putchar((row == 1) ? CURSOR_CHAR : ' ');
    printf(" Axe    [");
    putchar(mark_axe);
    printf("]  (%u)", (unsigned)has_axe);
    gotoxy(0, 4);
    putchar((row == 2) ? CURSOR_CHAR : ' ');
    printf(" Filter [");
    putchar(mark_filter);
    printf("]  (%u)", (unsigned)has_filter);
    gotoxy(0, 5);
    putchar((row == 3) ? CURSOR_CHAR : ' ');
    printf(" Battery[");
    putchar(mark_bat);
    printf("]  (%u)", (unsigned)bat_cnt);

    gotoxy(0, 7);
    putchar((row == 4) ? CURSOR_CHAR : ' ');
    printf(" Send");
}
void expedition_menu(struct Game *g, ExpeditionPlan *out_plan, uint8_t *out_send)
{
    *out_send = 0;

    // ---- Step 1: pick survivor
    uint8_t row = 0, prev = 0xFF;
    while (1)
    {
        if (row != prev)
        {
            draw_pick_survivor(g, row);
            prev = row;
        }
        uint8_t j = joypad();
        if (j & J_UP)
        {
            if (row > 0)
                row--;
            waitpadup();
        }
        else if (j & J_DOWN)
        {
            if (row < MAX_SURVIVORS)
                row++;
            waitpadup();
        }
        else if (j & J_A)
        {
            waitpadup();
            if (row == MAX_SURVIVORS)
                return; // cancel
            if (!g->team[row].alive)
                continue; // can't pick dead
            break;        // proceed
        }
        wait_vbl_done();
    }

    ExpeditionPlan plan = {0};
    plan.survivor_idx = row;

    // ---- Step 2: pick gear
    uint8_t grow = 0, gprev = 0xFF;
    bool dirty = true;
    while (1)
    {
        if (dirty || grow != gprev)
        {
            draw_pick_gear(g, &plan, grow);
            gprev = grow;
            dirty = false;
        }
        uint8_t j = joypad();
        if (j & J_UP)
        {
            if (grow > 0)
                grow--;
            waitpadup();
        }
        else if (j & J_DOWN)
        {
            if (grow < 4)
                grow++;
            waitpadup();
        }
        else if (j & J_B)
        {
            waitpadup();
            return;
        }
        else if (j & J_A)
        {
            waitpadup();
            if (grow == 4)
            { // Send
                if (plan.use_map && !have_item(g, IT_MAP))
                    plan.use_map = 0;
                if (plan.use_axe && !have_item(g, IT_AXE))
                    plan.use_axe = 0;
                if (plan.use_filter && !have_item(g, IT_FILTER))
                    plan.use_filter = 0;
                if (plan.use_battery && !have_item(g, IT_BATTERY))
                    plan.use_battery = 0;
                *out_plan = plan;
                *out_send = 1;
                return;
            }
            else
            {
                // Toggle gear if available
                switch (grow)
                {
                case 0:
                    if (have_item(g, IT_MAP))
                        plan.use_map ^= 1;
                    break;
                case 1:
                    if (have_item(g, IT_AXE))
                        plan.use_axe ^= 1;
                    break;
                case 2:
                    if (have_item(g, IT_FILTER))
                        plan.use_filter ^= 1;
                    break;
                case 3:
                    if (have_item(g, IT_BATTERY))
                        plan.use_battery ^= 1;
                    break;
                }
                dirty = true; // redraw to reflect toggles
            }
        }
        wait_vbl_done();
    }
}

// ---------- Simulation ----------
static uint8_t clamp_u8(int v, int lo, int hi)
{
    if (v < lo)
        v = lo;
    if (v > hi)
        v = hi;
    return (uint8_t)v;
}

/* --- Morale-based desertion chance (before hazard “lost”) --- */
static uint8_t desertion_chance_from_morale(uint8_t morale)
{
    /* linear scale: morale < 40 → up to 80% desertion
       tweak numbers to taste */
    if (morale >= 40u)
        return 0u;
    return (uint8_t)((40u - morale) * 2u); // 1..80
}

#pragma save
#pragma disable_warning 110 // Goofy ahh
/* internal: simulate a returning survivor using the gear flags */
static void simulate_return(struct Game *g, uint8_t idx,
                            uint8_t use_map, uint8_t use_axe, uint8_t use_filter,
                            ExpeditionResult *out_res)
{
    memset(out_res, 0, sizeof(*out_res));
    out_res->outcome = EXP_BACK;

    int lost_ch = 12, inj_ch = 20, sick_ch = 12;
    if (use_map)
        lost_ch -= 6;
    if (use_filter)
    {
        inj_ch = inj_ch / 2;
        sick_ch = 5;
    }

    if (lost_ch < 2)
        lost_ch = 2;
    if (lost_ch > 40)
        lost_ch = 40;
    if (inj_ch < 0)
        inj_ch = 0;
    if (inj_ch > 60)
        inj_ch = 60;
    if (sick_ch < 0)
        sick_ch = 0;
    if (sick_ch > 40)
        sick_ch = 40;

    uint8_t m = g->team[idx].morale;
    uint8_t dch = desertion_chance_from_morale(m);
    if (dch && (rng_u8() % 100u) < dch)
    {
        g->team[idx].alive = 0;
        out_res->outcome = EXP_LOST;
        out_res->deserted = 1; // mark as desertion
        return;                // gear they carried is lost
    }

    /* --- Hazard “lost?” roll --- */
    if ((rng_u8() % 100u) < (uint8_t)lost_ch)
    {
        g->team[idx].alive = 0;
        out_res->outcome = EXP_LOST;
        return;
    }

    /* ------- LOOT (consumables) ------- */
    uint8_t food = rng_range(2);  // 0..2
    uint8_t water = rng_range(2); // 0..2
    uint8_t bat = 0;
    uint8_t radio = 0;

    if (use_axe && (rng_u8() & 1) == 0)
        food++;
    if ((rng_u8() % 100) < 20)
        bat = 1;
    if ((rng_u8() % 100) < (use_map ? 20 : 10))
        radio = 1;

    /* ------- LOOT (gear) ------- */
    /* Small chances; Map is treated as unique (don’t add if you already have one). */
    uint8_t map_ch = 6;     // %
    uint8_t axe_ch = 10;    // %
    uint8_t filter_ch = 10; // %
    if (use_map)
    {
        axe_ch += 3;
        filter_ch += 3;
    } // better navigation → better odds

    uint8_t got_map = 0;
    uint8_t got_axe = 0;
    uint8_t got_filter = 0;
    uint8_t got_radio = 0;

    if ((rng_u8() % 100) < map_ch)
    {
        inv_add(g->inv, MAX_ITEMS, IT_MAP, 1);
        got_map = 1;
    }
    if ((rng_u8() % 100) < axe_ch)
    {
        inv_add(g->inv, MAX_ITEMS, IT_AXE, 1);
        got_axe = 1;
    }
    if ((rng_u8() % 100) < filter_ch)
    {
        inv_add(g->inv, MAX_ITEMS, IT_FILTER, 1);
        got_filter = 1;
    }
    /* Injury / sickness (only if returned) */
    if ((rng_u8() % 100) < (uint8_t)inj_ch)
    {
        g->team[idx].injured = 1;
        out_res->outcome = EXP_INJURED;
    }
    else if ((rng_u8() % 100) < (uint8_t)sick_ch)
    {
        g->team[idx].sick = 1;
        out_res->outcome = EXP_SICK;
    }

    /* Apply consumables */
    if (food)
        inv_add(g->inv, MAX_ITEMS, IT_FOOD, food);
    if (water)
        inv_add(g->inv, MAX_ITEMS, IT_WATER, water);
    if (bat)
        inv_add(g->inv, MAX_ITEMS, IT_BATTERY, bat);
    if (radio)
        inv_add(g->inv, MAX_ITEMS, IT_RADIO_PART, radio);

    /* Report */
    out_res->loot_food = food;
    out_res->loot_water = water;
    out_res->loot_battery = bat;
    out_res->loot_radio = radio;
    out_res->loot_map = got_map;
    out_res->loot_axe = got_axe;
    out_res->loot_filter = got_filter;
}
#pragma restore
void expedition_start(struct Game *g, const ExpeditionPlan *plan, uint8_t days_out)
{

    uint8_t i = plan->survivor_idx;
    if (!g->team[i].alive || g->away[i])
        return;

    // tune these two numbers to taste
    const uint8_t BASE = 2u;    // fixed cost
    const uint8_t PER_DAY = 2u; // extra per day out
    uint8_t pen = (uint8_t)(BASE + (uint8_t)(PER_DAY * days_out));
    Survivor *s = &g->team[i];
    if (s->morale > pen)
        s->morale -= pen;
    else
        s->morale = 0;

    // TAKE gear from inventory now; remember what they carry
    g->away_map[i] = 0;
    g->away_axe[i] = 0;
    g->away_filter[i] = 0;

    if (plan->use_map && inv_take(g->inv, MAX_ITEMS, IT_MAP, 1))
        g->away_map[i] = 1;
    if (plan->use_axe && inv_take(g->inv, MAX_ITEMS, IT_AXE, 1))
        g->away_axe[i] = 1;
    if (plan->use_filter && inv_take(g->inv, MAX_ITEMS, IT_FILTER, 1))
        g->away_filter[i] = 1;

    g->away[i] = 1;
    g->away_days[i] = days_out;

    ui_text_box("They head outside...\nBack in a few days.");
    ui_wait();
}

void expedition_day_begin(struct Game *g)
{
    // Process returns BEFORE rations
    for (uint8_t i = 0; i < MAX_SURVIVORS; i++)
    {
        if (!g->away[i])
            continue;
        if (g->away_days[i] > 0)
        {
            g->away_days[i]--;
            if (g->away_days[i] > 0)
                continue; // still away
        }

        // day hits 0 -> resolve today
        ExpeditionResult res;
        simulate_return(g, i, g->away_map[i], g->away_axe[i], g->away_filter[i], &res);

        /* If they returned (any outcome except EXP_LOST), give back carried gear */
        if (res.outcome != EXP_LOST)
        {
            if (g->away_map[i])
                inv_add(g->inv, MAX_ITEMS, IT_MAP, 1);
            if (g->away_axe[i])
                inv_add(g->inv, MAX_ITEMS, IT_AXE, 1);
            if (g->away_filter[i])
                inv_add(g->inv, MAX_ITEMS, IT_FILTER, 1);
        }

        /* Clear away state afterwards */
        g->away[i] = 0;
        g->away_days[i] = 0;
        g->away_map[i] = g->away_axe[i] = g->away_filter[i] = 0;
        g->away_battery[i] = 0;

        // morale based on outcome
        if (res.outcome == EXP_INJURED || res.outcome == EXP_SICK)
        {
            if (g->team[i].morale >= 3u)
                g->team[i].morale -= 3u;
            else
                g->team[i].morale = 0;
        }
        else if (res.outcome == EXP_BACK)
        {
            // tiny morale rebound for a safe return
            if (g->team[i].morale <= 98u)
                g->team[i].morale += 2u;
        }

        // Show result
        switch (res.outcome)
        {
        case EXP_LOST:
            ui_clear();
            const char *name = g->team[i].pname;
            if (res.deserted)
                printf("%s left to never return.\nMorale was too low...", name);
            else
                printf("%s never came back...\nMorale was too low...", name);
            ui_wait();
            break;
        case EXP_INJURED:
            ui_clear();
            printf("%s returned injured.\n", g->team[i].pname);
            ui_wait();
            break;
        case EXP_SICK:
            ui_clear();
            printf("%s returned sick.\n", g->team[i].pname);
            gotoxy(2, 2);
            printf("%u", (unsigned)res.loot_food);
            ui_put_icon(3u, 2u, (uint8_t)ICON_FOOD);
            gotoxy(5, 2);
            printf("%u", (unsigned)res.loot_water);
            ui_put_icon(6u, 2u, (uint8_t)ICON_WATER);
            gotoxy(8, 2);
            printf("%u", (unsigned)res.loot_med);
            ui_put_icon(9u, 2u, (uint8_t)ICON_MED);
            ui_wait();
            break;
        case EXP_BACK:
        default:
            ui_clear();
            printf("%s returned safe.\n", g->team[i].pname);
            gotoxy(2, 2);
            printf("%u", (unsigned)res.loot_food);
            ui_put_icon(3u, 2u, (uint8_t)ICON_FOOD);
            gotoxy(5, 2);
            printf("%u", (unsigned)res.loot_water);
            ui_put_icon(6u, 2u, (uint8_t)ICON_WATER);
            gotoxy(8, 2);
            printf("%u", (unsigned)res.loot_med);
            ui_put_icon(9u, 2u, (uint8_t)ICON_MED);
            /* NEW: gear lines (only if found) */
            if (res.loot_map || res.loot_axe || res.loot_filter)
            {
                printf("\nGear:");
                if (res.loot_map)
                    printf(" Map");
                if (res.loot_axe)
                    printf(" Axe");
                if (res.loot_filter)
                    printf(" Filter");
                printf("\n");
            }
            ui_wait();
            break;
        }
        ui_clear();
        ui_day_banner(G.day);
    }
}