#ifndef DATABASE_H
#define DATABASE_H

#include "models.h"

typedef struct {
    int connected;
} Database;

int db_connect(Database *db);
void db_close(Database *db);
int db_register_account(Database *db, const char *username, const char *password, const char *nickname);
int db_login_account(Database *db, const char *username, const char *password, GameState *state);
int db_load_game_state(Database *db, GameState *state);
void db_save_monster_book(Database *db, GameState *state);
void db_save_game_state(Database *db, GameState *state);
void db_save_run(Database *db, Player *player, int floor, int score);
void db_show_ranking(Database *db);

#endif
