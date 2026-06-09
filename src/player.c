#include "player.h"

#include <string.h>

void init_default_player(Player *player, const char *nickname)
{
    memset(player, 0, sizeof(Player));
    strncpy(player->nickname, nickname, MAX_NAME_LEN - 1);
    player->nickname[MAX_NAME_LEN - 1] = '\0';
    player->level = 1;
    player->exp = 0;
    player->gold = 0;
    player->best_floor = 1;
    player->material = 0;
    player->max_hp = 100;
    player->hp = player->max_hp;
    player->base_atk = 10;
}
