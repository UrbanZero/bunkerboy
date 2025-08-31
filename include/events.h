#ifndef EVENTS_H
#define EVENTS_H
#include <stdint.h>
#include <stdbool.h>
#include "game.h"
struct Game;

typedef struct {
    const char* title;
    // simple 2-choice event for starter
    const char* opt_a;
    const char* opt_b;
    // outcome indices
    int8_t  delta_food_a, delta_water_a; // +/- qty
    const char* res_a;
    int8_t  delta_food_b, delta_water_b;
    const char* res_b;
    uint8_t weight; // for weighted pick later
} EventDef;


void events_init(void);
const EventDef *events_pick_for_day(struct Game *g, uint16_t day);
void events_resolve(Game* g, const EventDef* ev, uint8_t choice_idx);

#endif
