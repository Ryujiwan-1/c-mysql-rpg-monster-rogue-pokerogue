#ifndef RANKING_H
#define RANKING_H

#include "models.h"

/*
 * ranking.h
 * 플레이 결과를 랭킹 점수로 변환하는 함수 선언 파일이다.
 */

/* 층수, 레벨, 골드를 이용해 랭킹에 저장할 점수를 계산한다. */
int calculate_score(Player *player, int floor);

#endif
