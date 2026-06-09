#ifndef INVENTORY_H
#define INVENTORY_H

#include "models.h"

const char *rarity_to_string(Rarity rarity);
void show_inventory(Player *player);
void add_random_equipment(GameState *state, int floor);
int get_player_total_atk(Player *player);
int get_player_total_max_hp(Player *player);
int get_rarity_material(Rarity rarity);

#endif
