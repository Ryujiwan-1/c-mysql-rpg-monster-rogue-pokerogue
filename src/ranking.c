#include "ranking.h"

int calculate_score(Player *player, int floor)
{
    return floor * 100 + player->level * 50 + player->gold;
}
