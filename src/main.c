#include <gbdk/platform.h>
#include <gb/gb.h>
#include <gbdk/font.h>
#include <stdio.h>

#include "game.h"
#include "ui.h"
#include "rng.h"
#include "events.h"
#include "intro.h"
#include "rations.h"
#include "expedition.h"
#include "bg_helpers.h"
#include "actions.h"

#include "../assets/door.h"
#include "../assets/bombs.h"

#include "hUGEDriver.h"
#include "../assets/music/on_fire.c"

#define FONT_TILES 96
#define BG_BASE FONT_TILES

static GameState gs = GS_TITLE;
extern const hUGESong_t on_fire;

static void state_title(void)
{
    DISPLAY_OFF;
    font_init();
    font_set(font_load(font_ibm));
    // Draw your background image (e.g., 'door' asset) after the font tiles
    BG_SHOW_ASSET(door, FONT_TILES, 1); // hides WINDOW while drawing

    ui_wait_msgless();
    game_new_run();
    gs = GS_SUMMARY;
}

static void state_summary(void)
{
    ui_clear();
    BG_SHOW_ASSET(bombs, FONT_TILES, 1);
    printf("   OH NOT AGAIN!\n BETTER TAKE COVER!");
    ui_wait();
    intro_supply_choice(&G);
    gs = GS_DAY_LOOP;
}
static void hud_print_other_items(Game *g)
{
    // label list for your defined items
    static const struct
    {
        ItemType t;
        const char *label;
    } extra[] = {
        {IT_MEDKIT, "Medkits"},
        {IT_RADIO_PART, "RadioParts"},
        {IT_FILTER, "Filter"},
        {IT_LOCK, "Lock"},
        {IT_MAP, "Map"},
        {IT_AXE, "Axe"},
    };

    for (uint8_t i = 0; i < (uint8_t)(sizeof(extra) / sizeof(extra[0])); ++i)
    {
        uint8_t c = inv_count(g, extra[i].t); // uses your game.c wrapper
        if (c)
            printf(" %s:%u\n", extra[i].label, (unsigned)c);
    }
}

static void state_day_loop(void)
{
    while (!game_is_over(&G))
    {
        G.day++;
        ui_day_banner(G.day);

        // Reset actions
        actions_day_reset(&G);

        // Handle any expedition returns first
        expedition_day_begin(&G);

        // Show quick HUD
        printf(" Food:%u\n Water:%u\n",
               (unsigned)inv_count(&G, IT_FOOD),
               (unsigned)inv_count(&G, IT_WATER));
        // print only the other items we actually have
        hud_print_other_items(&G);
        ui_wait();

        // Rations screen (per-survivor toggles)
        RationPlan plan;
        rations_menu(&G, &plan);

        // Rations menu
        actions_run_menu(&G);

        // Do choices/events for the day
        const EventDef *ev = events_pick_for_day(&G, G.day);
        if (ev)
        {
            const char *opts[2] = {ev->opt_a, ev->opt_b};
            uint8_t pick = ui_menu(ev->title, opts, 2, 0);
            events_resolve(&G, ev, pick);
        }
        else
        {
            ui_text_box("It was a quiet day.");
            ui_wait();
        }
        // Pause between days
        game_apply_day_end(&G, &plan);
        ui_text_box("Night falls...");
        ui_wait();
    }
    gs = GS_ENDING;
}

static void state_ending(void)
{
    ui_clear();
    if (G.radio_progress >= 5)
    {
        printf(" A rescue team found you!\n");
        printf(" You survived %u days.\n", G.day);
    }
    else
    {
        printf(" Everyone perished...\n");
        printf(" You lasted %u days.\n", G.day);
    }
    ui_wait();
    gs = GS_TITLE;
}

void main(void)
{
    NR52_REG = 0x80; // master on
    NR50_REG = 0xFF; // L/R volume
    NR51_REG = 0xFF; // enable all channels
    __critical
    {
        hUGE_init(&on_fire);
        add_VBL(hUGE_dosound); // call driver every VBlank
    }
    disable_interrupts();
    DISPLAY_OFF;

    // Seed RNG with DIV register (simple)
    rng_seed(DIV_REG);

    ui_init_console();
    events_init();
    game_init();

    DISPLAY_ON;
    enable_interrupts();

    gs = GS_TITLE;

    while (1)
    {
        switch (gs)
        {
        case GS_TITLE:
            state_title();
            break;
        case GS_SUMMARY:
            state_summary();
            break;
        case GS_DAY_LOOP:
            state_day_loop();
            break;
        case GS_ENDING:
            state_ending();
            break;
        default:
            gs = GS_TITLE;
            break;
        }
        wait_vbl_done();
    }
}
