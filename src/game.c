#include "game.h"

#include "battle.h"
#include "data_loader.h"
#include "database.h"
#include "input.h"
#include "inventory.h"
#include "monster_book.h"
#include "player.h"
#include "ranking.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_player_status(GameState *state)
{
    Player *p = &state->player;

    /*
     * 플레이어의 현재 상태를 한 곳에서 출력한다.
     * 장비 능력치는 inventory.c의 계산 함수를 사용해 기본 능력치와 장비 옵션을
     * 합산한 최종 HP/ATK를 보여준다.
     */
    printf("\n===== 플레이어 =====\n");
    printf("닉네임: %s\n", p->nickname);
    printf("레벨: %d  EXP: %d/%d\n", p->level, p->exp, p->level * 100);
    printf("Gold: %d\n", p->gold);
    printf("강화 재료: %d\n", p->material);
    printf("현재 층: %d  최고 층: %d\n", state->current_floor, p->best_floor);
    printf("HP: %d/%d  ATK: %d\n", p->hp, get_player_total_max_hp(p),
           get_player_total_atk(p));
}

static void start_new_player(GameState *state)
{
    char nickname[MAX_NAME_LEN];

    /*
     * 새 게임으로 초기화해도 계정의 닉네임은 유지한다.
     * 계정 닉네임이 없는 예외 상황에서만 새로 입력받고, 입력 실패 시 Player를 기본값으로 사용한다.
     */
    if (state->player.nickname[0] != '\0') {
        strncpy(nickname, state->player.nickname, MAX_NAME_LEN - 1);
        nickname[MAX_NAME_LEN - 1] = '\0';
    } else {
        printf("닉네임 입력: ");
        if (read_word(nickname, MAX_NAME_LEN) != INPUT_OK) {
            strncpy(nickname, "Player", MAX_NAME_LEN - 1);
            nickname[MAX_NAME_LEN - 1] = '\0';
        }
    }
    init_default_player(&state->player, nickname);
    state->current_floor = 1;
    /* 도감은 새 게임 기준으로 다시 초기화한다. */
    init_monster_book(state);
}

static void game_loop(GameState *state, Database *db)
{
    state->running = 1;

    /*
     * 로그인 후 실제 던전 진행을 담당하는 반복문이다.
     * 전투, 인벤토리, 도감, 저장, 종료 메뉴를 계속 보여주며,
     * 사망하거나 사용자가 종료를 선택하면 반복문을 빠져나간다.
     */
    while (state->running) {
        int choice;

        /* 현재 층이 기존 최고 기록보다 높으면 최고 층 기록을 갱신한다. */
        if (state->current_floor > state->player.best_floor) {
            state->player.best_floor = state->current_floor;
        }

        /* 층수나 처치 수 조건을 만족한 몬스터가 있으면 도감에서 해금한다. */
        check_unlocks(state);
        print_player_status(state);
        printf("\n===== %d층 =====\n", state->current_floor);
        printf("1. 전투 시작\n");
        printf("2. 인벤토리\n");
        printf("3. 몬스터 도감\n");
        printf("4. 저장\n");
        printf("5. 게임 종료\n");
        printf("선택: ");

        int input_result = read_int(&choice);
        if (input_result == INPUT_EOF) {
            /*
             * 콘솔 입력이 닫힌 경우에도 현재 진행 상황을 저장하고 종료한다.
             * 이전 버그처럼 EOF 상태에서 무한 입력 루프를 돌지 않게 하는 부분이다.
             */
            db_save_game_state(db, state);
            state->running = 0;
            break;
        }
        if (input_result != INPUT_OK) {
            continue;
        }

        if (choice == 1) {
            int win = run_battle(state);
            if (!win) {
                int score;
                /* 사망 시점의 최고 층과 점수를 계산해 세이브/기록/랭킹에 반영한다. */
                if (state->current_floor > state->player.best_floor) {
                    state->player.best_floor = state->current_floor;
                }
                score = calculate_score(&state->player, state->current_floor);
                db_save_game_state(db, state);
                db_save_run(db, &state->player, state->current_floor, score);
                db_show_ranking(db);
                state->running = 0;
            } else {
                /* 전투에서 승리하면 다음 층으로 이동하고, 약간의 HP를 회복한다. */
                state->current_floor++;
                state->player.hp += 10;
                if (state->player.hp > get_player_total_max_hp(&state->player)) {
                    state->player.hp = get_player_total_max_hp(&state->player);
                }
            }
        } else if (choice == 2) {
            show_inventory(&state->player);
        } else if (choice == 3) {
            show_monster_book(state);
        } else if (choice == 4) {
            db_save_game_state(db, state);
            printf("저장 완료\n");
        } else if (choice == 5) {
            db_save_game_state(db, state);
            state->running = 0;
        }
    }
}

void run_game(void)
{
    GameState state;
    Database db;
    int choice;
    int logged_in = 0;
    int program_running = 1;

    /* 장비 드랍, 몬스터 선택 등에 rand()를 사용하므로 프로그램 시작 시 난수 시드를 설정한다. */
    srand((unsigned int)time(NULL));

    /*
     * 구조체를 0으로 초기화해 쓰레기 값을 방지한다.
     * C에서는 지역 구조체 변수가 자동으로 0이 되지 않으므로 명시적으로 초기화해야 안전하다.
     */
    memset(&state, 0, sizeof(GameState));
    memset(&db, 0, sizeof(Database));

    /*
     * 몬스터, 아이템, 해금 조건, 스테이지 규칙은 고정 데이터이므로 파일에서 읽는다.
     * 반대로 계정별 진행 정보는 파일 저장을 사용하지 않고 DB에 저장한다.
     */
    if (!load_game_data(&state.data)) {
        return;
    }

    /* MySQL 서버에 연결하고, 필요한 DB/테이블이 없으면 자동으로 준비한다. */
    db_connect(&db);

    while (program_running) {
        if (!logged_in) {
            char username[MAX_NAME_LEN];
            char password[MAX_NAME_LEN];
            char nickname[MAX_NAME_LEN];

            printf("\n===== Monster Rogue 계정 =====\n");
            printf("1. 로그인\n");
            printf("2. 회원가입\n");
            printf("3. 종료\n");
            printf("선택: ");

            int input_result = read_int(&choice);
            if (input_result == INPUT_EOF) {
                /* 입력이 닫힌 경우 프로그램을 안전하게 종료한다. */
                program_running = 0;
                break;
            }
            if (input_result != INPUT_OK) {
                continue;
            }

            if (choice == 1) {
                printf("아이디: ");
                if (read_word(username, MAX_NAME_LEN) != INPUT_OK) {
                    program_running = 0;
                    continue;
                }
                printf("비밀번호: ");
                if (read_word(password, MAX_NAME_LEN) != INPUT_OK) {
                    program_running = 0;
                    continue;
                }

                /*
                 * 다른 계정의 정보가 남지 않도록 로그인 시 플레이어/도감/계정 정보를 초기화한다.
                 * 이후 db_login_account()가 DB에서 해당 계정의 세이브를 다시 채운다.
                 */
                memset(&state.player, 0, sizeof(Player));
                memset(state.book, 0, sizeof(state.book));
                state.account_id = 0;
                state.account_name[0] = '\0';
                state.current_floor = 1;

                if (db_login_account(&db, username, password, &state)) {
                    logged_in = 1;
                }
            } else if (choice == 2) {
                printf("사용할 아이디: ");
                if (read_word(username, MAX_NAME_LEN) != INPUT_OK) {
                    program_running = 0;
                    continue;
                }
                printf("사용할 비밀번호: ");
                if (read_word(password, MAX_NAME_LEN) != INPUT_OK) {
                    program_running = 0;
                    continue;
                }
                printf("게임 닉네임: ");
                if (read_word(nickname, MAX_NAME_LEN) != INPUT_OK) {
                    program_running = 0;
                    continue;
                }
                db_register_account(&db, username, password, nickname);
            } else if (choice == 3) {
                program_running = 0;
            }
            continue;
        }

        printf("\n===== Monster Rogue =====\n");
        printf("계정: %s / 닉네임: %s\n", state.account_name, state.player.nickname);
        printf("1. 게임 시작/이어하기\n");
        printf("2. 새 게임으로 초기화\n");
        printf("3. 랭킹 보기\n");
        printf("4. 로그아웃\n");
        printf("5. 종료\n");
        printf("선택: ");

        int input_result = read_int(&choice);
        if (input_result == INPUT_EOF) {
            /* 로그인 상태에서 입력이 닫히면 현재 계정 세이브를 저장하고 종료한다. */
            db_save_game_state(&db, &state);
            program_running = 0;
            break;
        }
        if (input_result != INPUT_OK) {
            continue;
        }

        if (choice == 1) {
            game_loop(&state, &db);
        } else if (choice == 2) {
            /* 계정은 유지하되 플레이어 진행도만 처음 상태로 되돌린다. */
            start_new_player(&state);
            db_save_game_state(&db, &state);
            printf("계정 세이브를 새 게임으로 초기화했습니다.\n");
        } else if (choice == 3) {
            db_show_ranking(&db);
        } else if (choice == 4) {
            db_save_game_state(&db, &state);
            logged_in = 0;
            printf("로그아웃했습니다.\n");
        } else if (choice == 5) {
            db_save_game_state(&db, &state);
            program_running = 0;
        }
    }

    /* 프로그램 종료 전에 MySQL 연결을 닫아 사용 중인 자원을 정리한다. */
    db_close(&db);
}
