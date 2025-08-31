#ifndef EXPEDITION_H
#define EXPEDITION_H

#include <stdint.h>
#include "game_types.h" // for MAX_SURVIVORS, ItemStack
#include "items.h"

struct Game; // forward declare to avoid cycles

typedef struct
{
    uint8_t survivor_idx; // index into g->team
    uint8_t use_map : 1;
    uint8_t use_axe : 1;
    uint8_t use_filter : 1;
    uint8_t use_battery : 1; // consumes 1 if available
} ExpeditionPlan;

typedef enum
{
    EXP_BACK = 0, // returned OK (maybe with loot)
    EXP_INJURED,  // returned but injured
    EXP_SICK,     // returned but sick
    EXP_LOST      // never came back (dead / missing)
} ExpeditionOutcome;

typedef struct
{
    ExpeditionOutcome outcome;
    uint8_t loot_food;
    uint8_t loot_water;
    uint8_t loot_battery;
    uint8_t loot_med; // radio parts
    uint8_t loot_radio;
    uint8_t loot_map;
    uint8_t loot_axe;
    uint8_t loot_filter;
    uint8_t deserted;
} ExpeditionResult;

/* UI: lets player select survivor + gear. Sets *out_send = 1 if confirmed. */
void expedition_menu(struct Game *g, ExpeditionPlan *out_plan, uint8_t *out_send);

void expedition_start(struct Game *g, const ExpeditionPlan *plan, uint8_t days_out);
/* Call at the START of each day: decrements timers, resolves any returns (shows UI). */
void expedition_day_begin(struct Game *g);

/* Simulate + apply inventory deltas (consumes battery, adds loot, sets survivor flags/state). */
void expedition_resolve(struct Game *g, const ExpeditionPlan *plan, ExpeditionResult *out_res);

/* Show a simple summary screen. */
void expedition_show_result(struct Game *g, const ExpeditionPlan *plan, const ExpeditionResult *res);

#endif
