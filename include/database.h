#ifndef DATABASE_H
#define DATABASE_H

#include "models.h"

/*
 * database.h
 * MySQL 5.7과 연동하는 DB 함수들의 인터페이스이다.
 * 계정, 플레이어 세이브, 인벤토리, 도감, 플레이 기록, 랭킹을 DB에 저장한다.
 */

typedef struct {
    /* connected가 1이면 MySQL 연결 성공, 0이면 연결 실패 또는 미연결 상태이다. */
    int connected;
} Database;

/* MySQL 서버에 접속하고, monster_rogue DB와 필요한 테이블을 준비한다. */
int db_connect(Database *db);

/* 사용이 끝난 MySQL 연결을 닫는다. */
void db_close(Database *db);

/* 새 계정을 account 테이블에 등록한다. */
int db_register_account(Database *db, const char *username, const char *password, const char *nickname);

/* 아이디/비밀번호를 확인하고, 로그인 성공 시 GameState에 계정과 세이브를 불러온다. */
int db_login_account(Database *db, const char *username, const char *password, GameState *state);

/* 현재 로그인 계정의 플레이어, 인벤토리, 도감 정보를 DB에서 읽는다. */
int db_load_game_state(Database *db, GameState *state);

/* 현재 도감 상태만 DB에 저장한다. */
void db_save_monster_book(Database *db, GameState *state);

/* 플레이어 기본 정보, 인벤토리, 도감을 한 번에 DB에 저장한다. */
void db_save_game_state(Database *db, GameState *state);

/* 사망 또는 한 판 종료 결과를 run_history와 ranking에 저장한다. */
void db_save_run(Database *db, Player *player, int floor, int score);

/* DB에 저장된 랭킹 상위 기록을 콘솔에 출력한다. */
void db_show_ranking(Database *db);

#endif
