#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "items.h"

#define MAX_SURVIVORS 3
#define MAX_ITEMS 16

typedef struct
{
    uint8_t alive : 1, sick : 1, tired : 1, injured : 1;
    uint8_t hunger;    // 0..3
    uint8_t thirst;    // 0..3
    uint8_t health;    // 0..3
    uint8_t morale;    // 0..100
    uint8_t trait;     // reserved
    const char *pname; // name
} Survivor;

typedef struct
{
    ItemType type;
    uint8_t qty;
} ItemStack;

#endif
