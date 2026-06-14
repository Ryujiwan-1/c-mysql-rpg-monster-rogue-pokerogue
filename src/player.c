#include "player.h"

#include <string.h>

void init_default_player(Player *player, const char *nickname)
{
    /*
     * 새 게임 시작 또는 사망 후 초기화할 때 사용하는 기본 플레이어 생성 함수이다.
     * memset으로 이전 장비/골드/레벨 값을 모두 지운 뒤, 1레벨 기본 능력치를 다시 넣는다.
     */
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
