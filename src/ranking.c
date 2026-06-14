#include "ranking.h"

int calculate_score(Player *player, int floor)
{
    /*
     * 랭킹 점수 계산식이다.
     * 높은 층까지 갈수록 점수가 크게 오르고, 레벨과 골드는 보조 점수로 반영한다.
     */
    return floor * 100 + player->level * 50 + player->gold;
}
