#include "database.h"

#include "inventory.h"
#include "monster_book.h"
#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_MYSQL
#include <mysql.h>

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "root"
#define DB_NAME "monster_rogue"
#define DB_PORT 3307

/*
 * MySQL C API는 연결 정보를 MYSQL 구조체 포인터로 관리한다.
 * 이 프로젝트에서는 DB 연결을 프로그램 전체에서 하나만 사용하므로 static 전역 변수로 둔다.
 * static을 붙이면 이 변수는 database.c 파일 안에서만 접근할 수 있어 모듈 경계가 깔끔해진다.
 */
static MYSQL *g_conn = NULL;

static int db_query(const char *query)
{
    /*
     * mysql_query()는 성공하면 0, 실패하면 0이 아닌 값을 반환한다.
     * 여러 함수에서 같은 방식으로 쿼리를 실행하므로 공통 함수로 묶어 중복 코드를 줄였다.
     */
    if (mysql_query(g_conn, query) != 0) {
        printf("[DB] 쿼리 실패: %s\n", mysql_error(g_conn));
        printf("[DB] 실패 쿼리: %s\n", query);
        return 0;
    }
    return 1;
}

static void db_escape(char *out, size_t out_size, const char *text)
{
    /*
     * 사용자가 입력한 문자열에는 작은따옴표(')처럼 SQL 문법을 깨뜨리는 문자가 들어갈 수 있다.
     * mysql_real_escape_string()은 그런 문자를 안전한 형태로 바꿔 SQL 오류와 간단한 SQL Injection을 막는다.
     */
    if (g_conn == NULL || text == NULL) {
        if (out_size > 0) {
            out[0] = '\0';
        }
        return;
    }
    mysql_real_escape_string(g_conn, out, text, (unsigned long)strlen(text));
}

static int db_column_exists(const char *table_name, const char *column_name)
{
    char query[256];
    MYSQL_RES *result;
    int exists;

    /*
     * 기존 DB를 쓰는 중에 새 컬럼이 추가되면 테이블 생성만으로는 반영되지 않는다.
     * SHOW COLUMNS로 해당 컬럼이 이미 있는지 확인한 뒤, 없을 때만 ALTER TABLE을 실행한다.
     */
    snprintf(query, sizeof(query), "SHOW COLUMNS FROM %s LIKE '%s'", table_name, column_name);
    if (mysql_query(g_conn, query) != 0) {
        return 0;
    }

    result = mysql_store_result(g_conn);
    if (result == NULL) {
        return 0;
    }

    exists = mysql_num_rows(result) > 0;
    mysql_free_result(result);
    return exists;
}

static void db_add_column_if_missing(const char *table_name, const char *column_name, const char *definition)
{
    char query[512];

    if (db_column_exists(table_name, column_name)) {
        return;
    }

    snprintf(query, sizeof(query), "ALTER TABLE %s ADD COLUMN %s %s",
             table_name, column_name, definition);
    db_query(query);
}

static int db_use_or_create_database(void)
{
    /*
     * MySQL 서버에는 여러 데이터베이스가 있을 수 있다.
     * 게임 전용 DB가 없으면 만들고, 이후 쿼리가 monster_rogue DB에 적용되도록 USE를 실행한다.
     */
    if (!db_query("CREATE DATABASE IF NOT EXISTS monster_rogue DEFAULT CHARACTER SET utf8mb4")) {
        return 0;
    }
    if (!db_query("USE monster_rogue")) {
        return 0;
    }
    return 1;
}

static int db_create_tables(void)
{
    /*
     * 최초 실행 시 필요한 테이블을 자동 생성한다.
     * CREATE TABLE IF NOT EXISTS를 사용하면 이미 테이블이 있는 경우에는 그대로 넘어가므로
     * 매번 실행해도 기존 데이터가 삭제되지 않는다.
     */
    /*
     * account: 로그인에 필요한 계정 정보 테이블.
     * 과제 프로젝트라 비밀번호를 단순 문자열로 저장하지만, 실제 서비스라면 해시 처리가 필요하다.
     */
    if (!db_query("CREATE TABLE IF NOT EXISTS account ("
                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                  "username VARCHAR(50) NOT NULL UNIQUE,"
                  "password VARCHAR(50) NOT NULL,"
                  "nickname VARCHAR(50) NOT NULL,"
                  "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)")) {
        return 0;
    }

    /*
     * player: 계정별 플레이어 진행 상황 저장 테이블.
     * level, exp, gold, current_floor 같은 세이브 데이터가 들어간다.
     */
    if (!db_query("CREATE TABLE IF NOT EXISTS player ("
                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                  "account_id INT,"
                  "nickname VARCHAR(50) NOT NULL UNIQUE,"
                  "level INT NOT NULL DEFAULT 1,"
                  "exp INT NOT NULL DEFAULT 0,"
                  "gold INT NOT NULL DEFAULT 0,"
                  "best_floor INT NOT NULL DEFAULT 1,"
                  "material INT NOT NULL DEFAULT 0,"
                  "current_floor INT NOT NULL DEFAULT 1,"
                  "max_hp INT NOT NULL DEFAULT 100,"
                  "base_atk INT NOT NULL DEFAULT 10)")) {
        return 0;
    }

    /*
     * inventory: 플레이어가 가진 장비 목록 저장 테이블.
     * 장비 이름, 등급, 공격력/체력 옵션, 착용 여부, 강화 단계가 저장된다.
     */
    if (!db_query("CREATE TABLE IF NOT EXISTS inventory ("
                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                  "player_id INT NOT NULL,"
                  "item_name VARCHAR(100) NOT NULL,"
                  "rarity VARCHAR(20) NOT NULL,"
                  "atk INT NOT NULL DEFAULT 0,"
                  "hp INT NOT NULL DEFAULT 0,"
                  "equip TINYINT(1) NOT NULL DEFAULT 0,"
                  "enhance_level INT NOT NULL DEFAULT 0)")) {
        return 0;
    }

    /*
     * monster_book: 계정별 몬스터 도감 테이블.
     * 같은 플레이어/몬스터 조합이 중복 저장되지 않도록 UNIQUE KEY를 둔다.
     */
    if (!db_query("CREATE TABLE IF NOT EXISTS monster_book ("
                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                  "player_id INT NOT NULL,"
                  "monster_id INT NOT NULL,"
                  "discover TINYINT(1) NOT NULL DEFAULT 0,"
                  "kill_count INT NOT NULL DEFAULT 0,"
                  "UNIQUE KEY uq_book_player_monster (player_id, monster_id))")) {
        return 0;
    }

    /*
     * run_history: 사망 또는 한 판 종료 시마다 남기는 플레이 기록 테이블.
     * 랭킹과 달리 매 판의 기록을 계속 쌓는 용도이다.
     */
    if (!db_query("CREATE TABLE IF NOT EXISTS run_history ("
                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                  "player_id INT NOT NULL,"
                  "floor INT NOT NULL,"
                  "score INT NOT NULL,"
                  "play_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)")) {
        return 0;
    }

    /*
     * ranking: 점수 순으로 보여주기 위한 기록 테이블.
     * 현재 구현은 최고 점수만 갱신하는 방식이 아니라 사망 시 기록을 추가하는 방식이다.
     */
    if (!db_query("CREATE TABLE IF NOT EXISTS ranking ("
                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                  "nickname VARCHAR(50) NOT NULL,"
                  "best_floor INT NOT NULL,"
                  "score INT NOT NULL)")) {
        return 0;
    }

    /*
     * 개발 도중 스키마가 바뀌었을 때 기존 DB도 실행 가능하도록 컬럼을 보강한다.
     * 과제 진행 중 테이블 구조가 조금 바뀌어도 데이터를 지우지 않고 이어갈 수 있다.
     */
    db_add_column_if_missing("player", "account_id", "INT");
    db_add_column_if_missing("player", "exp", "INT NOT NULL DEFAULT 0");
    db_add_column_if_missing("player", "material", "INT NOT NULL DEFAULT 0");
    db_add_column_if_missing("player", "current_floor", "INT NOT NULL DEFAULT 1");
    db_add_column_if_missing("player", "max_hp", "INT NOT NULL DEFAULT 100");
    db_add_column_if_missing("player", "base_atk", "INT NOT NULL DEFAULT 10");
    db_add_column_if_missing("inventory", "enhance_level", "INT NOT NULL DEFAULT 0");

    return 1;
}

static int db_get_player_id(GameState *state)
{
    char query[256];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int player_id = 0;

    /*
     * inventory, monster_book, run_history는 player 테이블의 id를 기준으로 연결된다.
     * 그래서 저장/불러오기 전에 현재 GameState가 DB의 어느 player 행인지 먼저 찾는다.
     */
    if (state->account_id > 0) {
        snprintf(query, sizeof(query), "SELECT id FROM player WHERE account_id=%d", state->account_id);
    } else {
        char safe_name[MAX_NAME_LEN * 2];
        db_escape(safe_name, sizeof(safe_name), state->player.nickname);
        snprintf(query, sizeof(query), "SELECT id FROM player WHERE nickname='%s'", safe_name);
    }

    if (mysql_query(g_conn, query) != 0) {
        printf("[DB] player id 조회 실패: %s\n", mysql_error(g_conn));
        return 0;
    }

    result = mysql_store_result(g_conn);
    if (result == NULL) {
        return 0;
    }

    row = mysql_fetch_row(result);
    if (row != NULL) {
        player_id = atoi(row[0]);
    }

    mysql_free_result(result);
    return player_id;
}

static void db_insert_default_book(GameState *state, int player_id)
{
    int i;
    char query[512];

    /*
     * 새 플레이어 생성 시 모든 몬스터의 도감 행을 미리 만들어 둔다.
     * INSERT IGNORE는 UNIQUE KEY 중복이 있으면 오류 대신 무시한다.
     */
    for (i = 0; i < state->data.monster_count; i++) {
        snprintf(query, sizeof(query),
                 "INSERT IGNORE INTO monster_book(player_id, monster_id, discover, kill_count) "
                 "VALUES(%d, %d, %d, %d)",
                 player_id, state->book[i].monster_id, state->book[i].discover,
                 state->book[i].kill_count);
        db_query(query);
    }
}

static void db_create_default_player(GameState *state)
{
    int player_id;
    char nickname[MAX_NAME_LEN];

    /*
     * 로그인은 성공했지만 player 세이브가 아직 없는 경우 호출된다.
     * 계정 닉네임을 유지한 상태로 기본 플레이어를 만들고 DB에 저장한다.
     */
    strncpy(nickname, state->player.nickname, MAX_NAME_LEN - 1);
    nickname[MAX_NAME_LEN - 1] = '\0';
    init_default_player(&state->player, nickname);
    state->current_floor = 1;
    init_monster_book(state);
    db_save_game_state(NULL, state);
    player_id = db_get_player_id(state);
    if (player_id > 0) {
        db_insert_default_book(state, player_id);
    }
}
#endif

int db_connect(Database *db)
{
#ifdef USE_MYSQL
    /*
     * 1. mysql_init으로 연결 객체를 만든다.
     * 2. mysql_real_connect로 실제 MySQL 5.7 서버에 접속한다.
     * 3. DB와 테이블을 준비한다.
     *
     * 이 함수가 성공해야 로그인, 세이브, 랭킹 기능을 사용할 수 있다.
     */
    db->connected = 0;
    g_conn = mysql_init(NULL);
    if (g_conn == NULL) {
        printf("[DB] mysql_init 실패\n");
        return 0;
    }

    /*
     * 마지막 DB 이름 인자를 NULL로 둔 이유:
     * 최초 실행 때 monster_rogue DB가 아직 없을 수 있으므로 서버에 먼저 접속한 뒤
     * db_use_or_create_database()에서 CREATE DATABASE와 USE를 실행한다.
     */
    if (mysql_real_connect(g_conn, DB_HOST, DB_USER, DB_PASS, NULL, DB_PORT, NULL, 0) == NULL) {
        printf("[DB] MySQL 5.7 서버 연결 실패: %s\n", mysql_error(g_conn));
        printf("[DB] 설정: host=%s, user=%s, password=%s, port=%d\n", DB_HOST, DB_USER, DB_PASS, DB_PORT);
        printf("[DB] 서버가 꺼져 있으면 MySQL 5.7 서버를 먼저 실행하세요.\n");
        mysql_close(g_conn);
        g_conn = NULL;
        return 0;
    }

    printf("[DB] MySQL 연결 성공 (%s:%d)\n", DB_HOST, DB_PORT);
    /* 한글 닉네임/몬스터 이름을 안전하게 저장하기 위해 연결 문자셋도 utf8mb4로 맞춘다. */
    mysql_set_character_set(g_conn, "utf8mb4");

    if (!db_use_or_create_database() || !db_create_tables()) {
        mysql_close(g_conn);
        g_conn = NULL;
        return 0;
    }

    db->connected = 1;
    printf("[DB] monster_rogue 데이터베이스 준비 완료\n");
    return 1;
#else
    db->connected = 0;
    printf("[DB] MySQL 비활성 빌드입니다. 계정 세이브 기능을 사용할 수 없습니다.\n");
    return 0;
#endif
}

void db_close(Database *db)
{
#ifdef USE_MYSQL
    /*
     * mysql_close()를 호출하면 MySQL 연결에 사용하던 자원이 정리된다.
     * g_conn을 NULL로 바꿔 이후 잘못된 포인터를 다시 쓰지 않게 한다.
     */
    if (g_conn != NULL) {
        mysql_close(g_conn);
        g_conn = NULL;
    }
#endif
    db->connected = 0;
}

int db_register_account(Database *db, const char *username, const char *password, const char *nickname)
{
#ifdef USE_MYSQL
    char safe_user[MAX_NAME_LEN * 2];
    char safe_pass[MAX_NAME_LEN * 2];
    char safe_nick[MAX_NAME_LEN * 2];
    char query[512];

    if (!db->connected || g_conn == NULL) {
        printf("[DB] DB 연결이 없어 회원가입을 할 수 없습니다.\n");
        return 0;
    }

    /*
     * 사용자가 입력한 아이디/비밀번호/닉네임을 바로 SQL에 넣지 않고 escape 처리한다.
     * 이 과제에서는 prepared statement까지 쓰지 않고, C 초중급 수준에서 이해하기 쉬운 방식으로 처리했다.
     */
    db_escape(safe_user, sizeof(safe_user), username);
    db_escape(safe_pass, sizeof(safe_pass), password);
    db_escape(safe_nick, sizeof(safe_nick), nickname);

    /* account 테이블에 새 계정을 추가한다. username은 UNIQUE라 중복 아이디를 막을 수 있다. */
    snprintf(query, sizeof(query),
             "INSERT INTO account(username, password, nickname) VALUES('%s', '%s', '%s')",
             safe_user, safe_pass, safe_nick);

    if (!db_query(query)) {
        printf("[DB] 이미 존재하는 아이디일 수 있습니다.\n");
        return 0;
    }

    printf("회원가입 완료! 로그인 후 게임을 시작하세요.\n");
    return 1;
#else
    (void)db;
    (void)username;
    (void)password;
    (void)nickname;
    printf("MySQL 빌드가 아니라 회원가입을 사용할 수 없습니다.\n");
    return 0;
#endif
}

int db_login_account(Database *db, const char *username, const char *password, GameState *state)
{
#ifdef USE_MYSQL
    char safe_user[MAX_NAME_LEN * 2];
    char safe_pass[MAX_NAME_LEN * 2];
    char query[512];
    MYSQL_RES *result;
    MYSQL_ROW row;

    if (!db->connected || g_conn == NULL) {
        printf("[DB] DB 연결이 없어 로그인할 수 없습니다.\n");
        return 0;
    }

    db_escape(safe_user, sizeof(safe_user), username);
    db_escape(safe_pass, sizeof(safe_pass), password);

    /*
     * account 테이블에서 아이디와 비밀번호가 일치하는 행을 찾는다.
     * 찾으면 account_id와 닉네임을 GameState에 저장하고, 이어서 player 세이브를 불러온다.
     */
    snprintf(query, sizeof(query),
             "SELECT id, username, nickname FROM account "
             "WHERE username='%s' AND password='%s'",
             safe_user, safe_pass);

    if (mysql_query(g_conn, query) != 0) {
        printf("[DB] 로그인 조회 실패: %s\n", mysql_error(g_conn));
        return 0;
    }

    result = mysql_store_result(g_conn);
    if (result == NULL) {
        return 0;
    }

    row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        printf("아이디 또는 비밀번호가 틀렸습니다.\n");
        return 0;
    }

    /*
     * mysql_fetch_row()로 얻은 값은 문자열 배열이다.
     * 숫자로 써야 하는 id는 atoi()로 int로 변환하고, 문자열은 strncpy()로 구조체에 복사한다.
     */
    state->account_id = atoi(row[0]);
    strncpy(state->account_name, row[1], MAX_NAME_LEN - 1);
    state->account_name[MAX_NAME_LEN - 1] = '\0';
    strncpy(state->player.nickname, row[2], MAX_NAME_LEN - 1);
    state->player.nickname[MAX_NAME_LEN - 1] = '\0';
    mysql_free_result(result);

    /*
     * 회원가입 직후에는 account는 있지만 player 세이브가 없을 수 있다.
     * 이 경우 기본 플레이어를 만들어 첫 세이브를 생성한다.
     */
    if (!db_load_game_state(db, state)) {
        db_create_default_player(state);
    }

    printf("%s 계정으로 로그인했습니다.\n", state->account_name);
    return 1;
#else
    (void)db;
    (void)username;
    (void)password;
    (void)state;
    return 0;
#endif
}

int db_load_game_state(Database *db, GameState *state)
{
#ifdef USE_MYSQL
    char query[512];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int player_id;
    int i;

    if (!db->connected || g_conn == NULL || state->account_id <= 0) {
        return 0;
    }

    /*
     * 세이브 불러오기 순서:
     * 1. player 테이블에서 기본 능력치와 현재 층을 읽는다.
     * 2. inventory 테이블에서 장비 목록을 읽는다.
     * 3. monster_book 테이블에서 도감 상태를 읽는다.
     */
    snprintf(query, sizeof(query),
             "SELECT id, nickname, level, exp, gold, best_floor, material, current_floor, max_hp, base_atk "
             "FROM player WHERE account_id=%d",
             state->account_id);

    if (mysql_query(g_conn, query) != 0) {
        printf("[DB] 세이브 조회 실패: %s\n", mysql_error(g_conn));
        return 0;
    }

    result = mysql_store_result(g_conn);
    if (result == NULL) {
        return 0;
    }

    row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        return 0;
    }

    /*
     * DB에서 읽은 player 행을 C 구조체로 옮긴다.
     * DB 결과는 모두 문자열이므로 level, exp 같은 숫자 필드는 atoi()로 변환한다.
     */
    player_id = atoi(row[0]);
    memset(&state->player, 0, sizeof(Player));
    strncpy(state->player.nickname, row[1], MAX_NAME_LEN - 1);
    state->player.nickname[MAX_NAME_LEN - 1] = '\0';
    state->player.level = atoi(row[2]);
    state->player.exp = atoi(row[3]);
    state->player.gold = atoi(row[4]);
    state->player.best_floor = atoi(row[5]);
    state->player.material = atoi(row[6]);
    state->current_floor = atoi(row[7]);
    state->player.max_hp = atoi(row[8]);
    state->player.base_atk = atoi(row[9]);
    state->player.hp = state->player.max_hp;
    state->player.inventory_count = 0;
    mysql_free_result(result);

    /* 저장된 장비를 id 순서대로 읽어 메모리의 inventory 배열에 다시 채운다. */
    snprintf(query, sizeof(query),
             "SELECT item_name, rarity, atk, hp, equip, enhance_level FROM inventory WHERE player_id=%d ORDER BY id",
             player_id);
    if (mysql_query(g_conn, query) == 0) {
        result = mysql_store_result(g_conn);
        if (result != NULL) {
            while ((row = mysql_fetch_row(result)) != NULL &&
                   state->player.inventory_count < MAX_INVENTORY) {
                Item *item = &state->player.inventory[state->player.inventory_count];
                strncpy(item->name, row[0], MAX_NAME_LEN - 1);
                item->name[MAX_NAME_LEN - 1] = '\0';
                /*
                 * DB에는 등급을 문자열로 저장한다.
                 * 게임 내부에서는 enum 값으로 사용하므로 문자열을 다시 enum으로 변환한다.
                 */
                item->rarity = RARITY_COMMON;
                for (i = RARITY_COMMON; i <= RARITY_MYTH; i++) {
                    if (strcmp(row[1], rarity_to_string((Rarity)i)) == 0) {
                        item->rarity = (Rarity)i;
                    }
                }
                item->atk = atoi(row[2]);
                item->hp = atoi(row[3]);
                item->equipped = atoi(row[4]);
                item->enhance_level = atoi(row[5]);
                state->player.inventory_count++;
            }
            mysql_free_result(result);
        }
    }

    /*
     * 먼저 도감 배열을 기본 상태로 초기화한 뒤, DB에 저장된 발견 여부와 처치 수만 덮어쓴다.
     * 이렇게 하면 새 몬스터 데이터가 추가되어도 배열 구조가 깨지지 않는다.
     */
    init_monster_book(state);
    snprintf(query, sizeof(query),
             "SELECT monster_id, discover, kill_count FROM monster_book WHERE player_id=%d",
             player_id);
    if (mysql_query(g_conn, query) == 0) {
        result = mysql_store_result(g_conn);
        if (result != NULL) {
            while ((row = mysql_fetch_row(result)) != NULL) {
                int monster_id = atoi(row[0]);
                for (i = 0; i < state->data.monster_count; i++) {
                    if (state->book[i].monster_id == monster_id) {
                        state->book[i].discover = atoi(row[1]);
                        state->book[i].kill_count = atoi(row[2]);
                    }
                }
            }
            mysql_free_result(result);
        }
    }

    if (state->current_floor < 1) {
        state->current_floor = 1;
    }
    return 1;
#else
    (void)db;
    (void)state;
    return 0;
#endif
}

static void db_save_player_with_state(Database *db, GameState *state)
{
#ifdef USE_MYSQL
    char safe_name[MAX_NAME_LEN * 2];
    char query[512];

    if ((db != NULL && !db->connected) || g_conn == NULL) {
        return;
    }

    /*
     * player 테이블은 세이브의 중심이다.
     * 이미 같은 계정의 player 행이 있으면 UPDATE하고, 없으면 INSERT한다.
     */
    db_escape(safe_name, sizeof(safe_name), state->player.nickname);
    if (state->account_id > 0) {
        /*
         * ON DUPLICATE KEY UPDATE:
         * UNIQUE 제약 조건에 걸리는 행이 이미 있으면 INSERT 대신 UPDATE를 수행한다.
         * 즉, 새 세이브 생성과 기존 세이브 갱신을 한 쿼리로 처리할 수 있다.
         */
        snprintf(query, sizeof(query),
                 "INSERT INTO player(account_id, nickname, level, exp, gold, best_floor, material, current_floor, max_hp, base_atk) "
                 "VALUES(%d, '%s', %d, %d, %d, %d, %d, %d, %d, %d) "
                 "ON DUPLICATE KEY UPDATE nickname='%s', level=%d, exp=%d, gold=%d, "
                 "best_floor=%d, material=%d, current_floor=%d, max_hp=%d, base_atk=%d",
                 state->account_id, safe_name, state->player.level, state->player.exp,
                 state->player.gold, state->player.best_floor, state->player.material,
                 state->current_floor, state->player.max_hp, state->player.base_atk, safe_name,
                 state->player.level, state->player.exp, state->player.gold,
                 state->player.best_floor, state->player.material, state->current_floor,
                 state->player.max_hp, state->player.base_atk);
    } else {
        snprintf(query, sizeof(query),
                 "INSERT INTO player(nickname, level, exp, gold, best_floor, material, current_floor, max_hp, base_atk) "
                 "VALUES('%s', %d, %d, %d, %d, %d, %d, %d, %d) "
                 "ON DUPLICATE KEY UPDATE level=%d, exp=%d, gold=%d, best_floor=%d, "
                 "material=%d, current_floor=%d, max_hp=%d, base_atk=%d",
                 safe_name, state->player.level, state->player.exp, state->player.gold,
                 state->player.best_floor, state->player.material, state->current_floor, state->player.max_hp,
                 state->player.base_atk, state->player.level, state->player.exp,
                 state->player.gold, state->player.best_floor, state->player.material, state->current_floor,
                 state->player.max_hp, state->player.base_atk);
    }
    db_query(query);
#else
    (void)db;
    (void)state;
#endif
}

void db_save_monster_book(Database *db, GameState *state)
{
#ifdef USE_MYSQL
    int player_id;
    int i;
    char query[512];

    if ((db != NULL && !db->connected) || g_conn == NULL) {
        return;
    }

    player_id = db_get_player_id(state);
    if (player_id == 0) {
        return;
    }

    /*
     * 도감은 몬스터마다 discover/kill_count가 따로 있으므로 반복문으로 전부 저장한다.
     * UNIQUE KEY 덕분에 같은 플레이어의 같은 몬스터는 한 행만 유지된다.
     */
    for (i = 0; i < state->data.monster_count; i++) {
        snprintf(query, sizeof(query),
                 "INSERT INTO monster_book(player_id, monster_id, discover, kill_count) "
                 "VALUES(%d, %d, %d, %d) "
                 "ON DUPLICATE KEY UPDATE discover=%d, kill_count=%d",
                 player_id, state->book[i].monster_id, state->book[i].discover,
                 state->book[i].kill_count, state->book[i].discover,
                 state->book[i].kill_count);
        db_query(query);
    }
#else
    (void)db;
    (void)state;
#endif
}

void db_save_game_state(Database *db, GameState *state)
{
#ifdef USE_MYSQL
    int player_id;
    int i;
    char query[512];

    if ((db != NULL && !db->connected) || g_conn == NULL) {
        return;
    }

    /*
     * 전체 세이브 저장 순서:
     * 1. player 기본 정보 저장
     * 2. inventory를 다시 저장
     * 3. monster_book 저장
     */
    db_save_player_with_state(db, state);
    player_id = db_get_player_id(state);
    if (player_id == 0) {
        return;
    }

    /*
     * 인벤토리는 추가/삭제/장착 변경이 많다.
     * 간단한 구현을 위해 기존 장비 행을 모두 지운 뒤 현재 메모리 배열을 다시 INSERT한다.
     */
    snprintf(query, sizeof(query), "DELETE FROM inventory WHERE player_id=%d", player_id);
    db_query(query);

    /* 현재 메모리에 있는 장비 배열을 한 개씩 inventory 테이블에 저장한다. */
    for (i = 0; i < state->player.inventory_count; i++) {
        char safe_item[MAX_NAME_LEN * 2];
        Item *item = &state->player.inventory[i];

        db_escape(safe_item, sizeof(safe_item), item->name);
        snprintf(query, sizeof(query),
                 "INSERT INTO inventory(player_id, item_name, rarity, atk, hp, equip, enhance_level) "
                 "VALUES(%d, '%s', '%s', %d, %d, %d, %d)",
                 player_id, safe_item, rarity_to_string(item->rarity),
                 item->atk, item->hp, item->equipped, item->enhance_level);
        db_query(query);
    }

    db_save_monster_book(db, state);
#else
    (void)db;
    (void)state;
#endif
}

void db_save_run(Database *db, Player *player, int floor, int score)
{
#ifdef USE_MYSQL
    GameState temp;
    int player_id;
    char safe_name[MAX_NAME_LEN * 2];
    char query[512];

    if (!db->connected || g_conn == NULL) {
        return;
    }

    /*
     * 사망 기록 저장용 함수는 Player 포인터만 받는다.
     * db_save_player_with_state()는 GameState를 받으므로, 필요한 값만 담은 임시 GameState를 만든다.
     */
    memset(&temp, 0, sizeof(temp));
    temp.player = *player;
    temp.current_floor = floor;
    db_save_player_with_state(db, &temp);
    player_id = db_get_player_id(&temp);
    if (player_id == 0) {
        return;
    }

    /* run_history에는 매 판의 결과를 누적 저장한다. */
    snprintf(query, sizeof(query),
             "INSERT INTO run_history(player_id, floor, score) VALUES(%d, %d, %d)",
             player_id, floor, score);
    db_query(query);

    /* ranking에는 점수 순 정렬에 사용할 기록을 저장한다. */
    db_escape(safe_name, sizeof(safe_name), player->nickname);
    snprintf(query, sizeof(query),
             "INSERT INTO ranking(nickname, best_floor, score) VALUES('%s', %d, %d)",
             safe_name, floor, score);
    db_query(query);
#else
    (void)db;
    (void)player;
    (void)floor;
    (void)score;
#endif
}

void db_show_ranking(Database *db)
{
#ifdef USE_MYSQL
    MYSQL_RES *result;
    MYSQL_ROW row;
    int rank = 1;

    printf("\n===== 랭킹 =====\n");

    if (!db->connected || g_conn == NULL) {
        printf("DB 연결이 없어 랭킹을 볼 수 없습니다.\n");
        return;
    }

    /*
     * 점수가 높은 순서대로 최대 10명만 조회한다.
     * ORDER BY score DESC는 점수를 내림차순으로 정렬한다는 뜻이다.
     */
    if (mysql_query(g_conn,
                    "SELECT nickname, best_floor, score "
                    "FROM ranking ORDER BY score DESC LIMIT 10") != 0) {
        printf("[DB] 랭킹 조회 실패: %s\n", mysql_error(g_conn));
        return;
    }

    result = mysql_store_result(g_conn);
    if (result == NULL) {
        printf("아직 랭킹 기록이 없습니다.\n");
        return;
    }

    while ((row = mysql_fetch_row(result)) != NULL) {
        printf("%2d. %-12s 층:%4s 점수:%6s\n", rank, row[0], row[1], row[2]);
        rank++;
    }

    if (rank == 1) {
        printf("아직 랭킹 기록이 없습니다.\n");
    }

    mysql_free_result(result);
#else
    (void)db;
    printf("MySQL 빌드가 아니라 랭킹을 볼 수 없습니다.\n");
#endif
}
