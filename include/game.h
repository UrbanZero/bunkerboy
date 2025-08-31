#ifndef GAME_H
#define GAME_H

#include "game_types.h"
#include "rations.h"

struct RationPlan;

typedef struct Game
{ // <-- give it a tag: struct Game
    Survivor team[MAX_SURVIVORS];
    ItemStack inv[MAX_ITEMS];
    uint16_t day;
    uint8_t radio_attempted_today;
    uint8_t radio_progress;
    uint8_t noise;

    uint8_t acted_today[MAX_SURVIVORS];   
    uint8_t combat_training[MAX_SURVIVORS];
    /* Expeditions: per-survivor state */
    uint8_t away[MAX_SURVIVORS];      // 0/1 = currently outside
    uint8_t away_days[MAX_SURVIVORS]; // days left to return (if 0 -> due today)
    uint8_t away_map[MAX_SURVIVORS];  // gear flags remembered for resolution
    uint8_t away_axe[MAX_SURVIVORS];
    uint8_t away_filter[MAX_SURVIVORS];
    uint8_t away_battery[MAX_SURVIVORS]; // 1 if a battery was consumed on departure
} Game;

typedef enum
{
    GS_TITLE = 0,
    GS_GRAB, // (later)
    GS_SUMMARY,
    GS_DAY_LOOP,
    GS_ENDING
} GameState;

extern Game G;

void game_init(void);
void game_new_run(void);
void game_apply_day_end(Game *g, const RationPlan *plan);
bool game_is_over(const Game *g);
uint8_t inv_count(Game *g, ItemType t);
bool inv_consume(Game *g, ItemType t, uint8_t n);

#endif
