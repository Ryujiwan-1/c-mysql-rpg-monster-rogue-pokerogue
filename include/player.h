#ifndef PLAYER_H
#define PLAYER_H

#include "models.h"

/*
 * player.h
 * 새 게임 또는 사망 후 초기화에 사용할 플레이어 기본값 설정 함수를 선언한다.
 */

/* 닉네임은 유지하고 레벨/HP/공격력/골드/장비 등을 기본 상태로 초기화한다. */
void init_default_player(Player *player, const char *nickname);

#endif
