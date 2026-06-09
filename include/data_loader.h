#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "models.h"

int load_game_data(GameData *data);
MonsterDef *find_monster_by_name(GameData *data, const char *name);
MonsterDef *find_monster_by_id(GameData *data, int id);
ItemDef *find_item_by_id(GameData *data, int id);

#endif
