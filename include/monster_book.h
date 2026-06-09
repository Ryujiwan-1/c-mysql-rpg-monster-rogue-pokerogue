#ifndef MONSTER_BOOK_H
#define MONSTER_BOOK_H

#include "models.h"

void init_monster_book(GameState *state);
void discover_monster(GameState *state, int monster_id);
void add_monster_kill(GameState *state, int monster_id);
void check_unlocks(GameState *state);
void show_monster_book(GameState *state);
int is_monster_discovered(GameState *state, int monster_id);

#endif
