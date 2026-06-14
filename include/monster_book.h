#ifndef MONSTER_BOOK_H
#define MONSTER_BOOK_H

#include "models.h"

/*
 * monster_book.h
 * 몬스터 도감의 발견 여부, 처치 수, 해금 조건 처리를 담당하는 함수 선언 파일이다.
 */

/* 새 게임 기준으로 도감 배열을 초기화하고 기본 몬스터를 발견 처리한다. */
void init_monster_book(GameState *state);

/* 특정 monster_id를 도감에서 발견 상태로 바꾼다. */
void discover_monster(GameState *state, int monster_id);

/* 몬스터 처치 수를 1 증가시키고 발견 상태도 보장한다. */
void add_monster_kill(GameState *state, int monster_id);

/* unlock.txt에서 읽은 조건을 검사해 새 몬스터를 해금한다. */
void check_unlocks(GameState *state);

/* 현재 도감 상태를 콘솔에 출력한다. */
void show_monster_book(GameState *state);

/* 해당 몬스터가 도감에 발견되어 있는지 확인한다. */
int is_monster_discovered(GameState *state, int monster_id);

#endif
