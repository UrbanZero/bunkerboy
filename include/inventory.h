#ifndef INVENTORY_H
#define INVENTORY_H
#include <stdint.h>
#include <stdbool.h>
#include "game_types.h"   // ItemStack, ItemType
#include "items.h"

void    inv_clear(ItemStack inv[], uint8_t capacity);
uint8_t inv_add(ItemStack inv[], uint8_t capacity, ItemType t, uint8_t qty);      // returns leftover
bool    inv_take(ItemStack inv[], uint8_t capacity, ItemType t, uint8_t qty);     // true if took all
uint8_t inv_count_type(const ItemStack inv[], uint8_t capacity, ItemType t);
int8_t  inv_first_slot_with(const ItemStack inv[], uint8_t capacity, ItemType t);

#endif
