#ifndef RATIONS_H
#define RATIONS_H

#include <stdint.h>
#include "game_types.h" // for MAX_SURVIVORS

typedef struct RationPlan
{
    uint8_t food[MAX_SURVIVORS];
    uint8_t water[MAX_SURVIVORS];
    uint8_t med[MAX_SURVIVORS];
} RationPlan;

struct Game;

void rations_menu(struct Game *g, RationPlan *out_plan);
unsigned dig_count(const unsigned n);

#endif
