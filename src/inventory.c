#include "inventory.h"

static int8_t find_stackable_slot(ItemStack inv[], uint8_t cap, ItemType t)
{
    const ItemDef *def = &ITEM_DB[t];
    if (!(def->flags & ITEMF_STACKABLE))
        return -1;
    for (uint8_t i = 0; i < cap; i++)
    {
        if (inv[i].type == t && inv[i].qty < def->max_stack)
            return (int8_t)i;
    }
    return -1;
}
static int8_t find_empty_slot(ItemStack inv[], uint8_t cap)
{
    for (uint8_t i = 0; i < cap; i++)
        if (inv[i].type == IT_NONE || inv[i].qty == 0)
            return (int8_t)i;
    return -1;
}

void inv_clear(ItemStack inv[], uint8_t cap)
{
    for (uint8_t i = 0; i < cap; i++)
    {
        inv[i].type = IT_NONE;
        inv[i].qty = 0;
    }
}

uint8_t inv_add(ItemStack inv[], uint8_t cap, ItemType t, uint8_t qty)
{
    const ItemDef *def = &ITEM_DB[t];
    uint8_t left = qty;

    // 1) Fill existing stacks
    if (def->flags & ITEMF_STACKABLE)
    {
        for (uint8_t i = 0; i < cap && left; i++)
        {
            if (inv[i].type == t && inv[i].qty < def->max_stack)
            {
                uint8_t space = def->max_stack - inv[i].qty;
                uint8_t put = (left < space) ? left : space;
                inv[i].qty += put;
                left -= put;
            }
        }
    }

    // 2) Use empty slots
    while (left)
    {
        int8_t s = find_empty_slot(inv, cap);
        if (s < 0)
            break; // no space
        inv[s].type = t;
        if (def->flags & ITEMF_STACKABLE)
        {
            uint8_t put = (left < def->max_stack) ? left : def->max_stack;
            inv[s].qty = put;
            left -= put;
        }
        else
        {
            inv[s].qty = 1;
            left -= 1;
        }
    }
    return left; // 0 if all added
}

bool inv_take(ItemStack inv[], uint8_t cap, ItemType t, uint8_t qty)
{
    uint8_t need = qty;
    for (uint8_t i = 0; i < cap && need; i++)
    {
        if (inv[i].type == t && inv[i].qty)
        {
            uint8_t take = (inv[i].qty < need) ? inv[i].qty : need;
            inv[i].qty -= take;
            need -= take;
            if (inv[i].qty == 0)
                inv[i].type = IT_NONE;
        }
    }
    return (need == 0);
}

uint8_t inv_count_type(const ItemStack inv[], uint8_t cap, ItemType t)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < cap; i++)
        if (inv[i].type == t)
            sum += inv[i].qty;
    return sum;
}

int8_t inv_first_slot_with(const ItemStack inv[], uint8_t cap, ItemType t)
{
    for (uint8_t i = 0; i < cap; i++)
        if (inv[i].type == t)
            return (int8_t)i;
    return -1;
}
