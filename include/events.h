#ifndef EVENTS_H
#define EVENTS_H
#include <stdint.h>
#include <stdbool.h>
#include "game.h"
struct Game;

// events.h
typedef struct
{
    const char *title;
    const char *opt_a, *opt_b;
    int8_t delta_food_a, delta_water_a;
    const char *res_a;
    int8_t delta_food_b, delta_water_b;
    const char *res_b;
    uint8_t weight;

    // NEW: explicit costs (paid up-front)
    uint8_t need_food_a, need_water_a;
    uint8_t need_food_b, need_water_b;
} EventDef;

void events_init(void);
const EventDef *events_pick_for_day(struct Game *g, uint16_t day);
void events_resolve(Game *g, const EventDef *ev, uint8_t choice_idx);

#endif
