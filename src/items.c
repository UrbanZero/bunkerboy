#include "items.h"

const ItemDef ITEM_DB[IT_MAX] = {
    [IT_NONE]       = { "—",           0,                               0  },
    [IT_FOOD]       = { "Food",        ITEMF_CONSUMABLE|ITEMF_STACKABLE,99 },
    [IT_WATER]      = { "Water",       ITEMF_CONSUMABLE|ITEMF_STACKABLE,99 },
    [IT_BATTERY]    = { "Battery",     ITEMF_STACKABLE,                 20 },
    [IT_MEDKIT]     = { "Medkit",      ITEMF_CONSUMABLE|ITEMF_STACKABLE, 9 },
    [IT_RADIO_PART] = { "Radio Part",  ITEMF_STACKABLE,                  4 },
    [IT_FILTER]     = { "Filter",      0,                                1 },
    [IT_LOCK]       = { "Lock",        0,                                1 },
    [IT_MAP]        = { "Map",         0,                                1 },
    [IT_AXE]        = { "Axe",         0,                                1 },
};
