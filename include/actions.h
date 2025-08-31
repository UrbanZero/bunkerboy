#ifndef ACTIONS_H
#define ACTIONS_H
#include "game.h"

// Call at the start of each day
void actions_day_reset(Game *g);

// Run the Actions loop: choose survivor first, then their action.
// Returns when the player picks "Done" or nobody is available.
void actions_run_menu(Game *g);

#endif