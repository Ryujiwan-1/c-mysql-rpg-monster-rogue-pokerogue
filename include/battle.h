#ifndef BATTLE_H
#define BATTLE_H

#include "models.h"

/*
 * battle.h
 * 전투 시스템에서 외부 모듈이 사용할 함수들을 선언한다.
 * 실제 구현은 src/battle.c에 있으며, game.c는 이 함수를 호출해 전투 결과를 받는다.
 */

/* 현재 GameState를 기준으로 한 번의 전투를 진행한다. 승리하면 1, 패배하면 0을 반환한다. */
int run_battle(GameState *state);

/* 몬스터 원본 데이터에 현재 층 보정을 적용해 실제 전투용 몬스터를 만든다. */
Monster create_scaled_monster(GameState *state, int floor);

#endif
