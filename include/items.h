#ifndef ITEMS_H
#define ITEMS_H
#include <stdint.h>

// Flags
#define ITEMF_CONSUMABLE  (1u << 0)
#define ITEMF_STACKABLE   (1u << 1)

// Add new items here
typedef enum {
    IT_NONE = 0,
    IT_FOOD,
    IT_WATER,
    IT_BATTERY,
    IT_MEDKIT,
    IT_RADIO_PART,
    IT_FILTER,     // gas filter
    IT_LOCK,       // door lock
    IT_MAP,
    IT_AXE,
    IT_MAX
} ItemType;

typedef struct {
    const char* name;
    uint8_t flags;       // ITEMF_*
    uint8_t max_stack;   // respected if STACKABLE
} ItemDef;

extern const ItemDef ITEM_DB[IT_MAX];

#endif
