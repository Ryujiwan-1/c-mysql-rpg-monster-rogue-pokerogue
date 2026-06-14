#ifndef GAME_H
#define GAME_H

/*
 * game.h
 * 프로그램의 전체 실행 흐름을 시작하는 함수 선언 파일이다.
 * main.c는 run_game()만 호출하고, 메뉴/로그인/게임 루프는 game.c에서 처리한다.
 */

/* 게임 데이터 로딩, DB 연결, 로그인 메뉴, 플레이 메뉴를 모두 실행한다. */
void run_game(void);

#endif
