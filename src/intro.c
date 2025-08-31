#include "intro.h"
#include "ui.h"
#include "inventory.h"
#include "items.h"
#include <stdio.h>

typedef struct
{
    ItemType t;
    uint8_t qty;
} PackItem;

typedef struct
{
    const char *name;
    const char *desc;        // short text shown after picking
    const PackItem items[6]; // up to 6 entries, terminated with {IT_NONE,0}
} SupplyPack;

static const SupplyPack PACKS[3] = {
    {"Rations",
     "  Crate: \n \n Food x4, \n Water x4, \n Medkit x1\n",
     {{IT_FOOD, 4}, {IT_WATER, 4}, {IT_MEDKIT, 1}, {IT_NONE, 0}}},
    {"Tools",
     "  Crate: \n \n Axe x1, \n Filter x1, \n Map x1, \n",
     {{IT_AXE, 1}, {IT_FILTER, 1}, {IT_MAP, 1}, {IT_NONE, 0}}},
    {"Radio Gear",
     "  Crate: \n \n Radio Part x1, \n Food x2, \n Water x2\n",
     {{IT_RADIO_PART, 1}, {IT_BATTERY, 2}, {IT_FOOD, 2}, {IT_NONE, 0}}}};

static void apply_pack(Game *g, const SupplyPack *p)
{
    for (uint8_t i = 0; i < 6 && p->items[i].t != IT_NONE; ++i)
    {
        inv_add(g->inv, MAX_ITEMS, p->items[i].t, p->items[i].qty);
    }
}

void intro_supply_choice(Game *g)
{
    const char *opts[3] = {PACKS[0].name, PACKS[1].name, PACKS[2].name};

    // Show the selection menu
    uint8_t pick = ui_menu("Choose your\nstarting crate\n", opts, 3, 0);

    // Apply and confirm
    apply_pack(g, &PACKS[pick]);
    ui_text_box(PACKS[pick].desc);
    ui_wait();

    // Quick HUD line
    ui_text_box(" Supplies loaded.\n  Good luck.\n\n");
    ui_wait();
}
