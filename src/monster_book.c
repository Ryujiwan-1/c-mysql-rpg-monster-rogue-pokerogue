#include "monster_book.h"

#include "console_utils.h"
#include "data_loader.h"

#include <stdio.h>
#include <string.h>

static MonsterBookEntry *find_book_entry(GameState *state, int monster_id)
{
    /* monster_id에 해당하는 도감 항목을 찾는다. 없으면 NULL을 반환한다. */
    int i;
    for (i = 0; i < state->data.monster_count; i++) {
        if (state->book[i].monster_id == monster_id) {
            return &state->book[i];
        }
    }
    return NULL;
}

void init_monster_book(GameState *state)
{
    /*
     * 새 게임 시작 시 도감을 초기 상태로 만든다.
     * 모든 몬스터는 미발견/처치 0회로 시작하고, 초반 진행을 위해 슬라임과 박쥐만 기본 발견 처리한다.
     */
    int i;
    MonsterDef *monster;

    for (i = 0; i < state->data.monster_count; i++) {
        state->book[i].monster_id = state->data.monsters[i].id;
        state->book[i].discover = 0;
        state->book[i].kill_count = 0;
    }

    monster = find_monster_by_name(&state->data, "슬라임");
    if (monster != NULL) {
        discover_monster(state, monster->id);
    }

    monster = find_monster_by_name(&state->data, "박쥐");
    if (monster != NULL) {
        discover_monster(state, monster->id);
    }
}

void discover_monster(GameState *state, int monster_id)
{
    /*
     * 몬스터를 도감에 등록한다.
     * 이미 발견된 몬스터는 다시 메시지를 출력하지 않도록 discover 값을 확인한다.
     */
    MonsterBookEntry *entry = find_book_entry(state, monster_id);
    MonsterDef *monster = find_monster_by_id(&state->data, monster_id);

    if (entry != NULL && !entry->discover) {
        entry->discover = 1;
        if (monster != NULL) {
            printf("[도감] %s 발견!\n", monster->name);
        }
    }
}

void add_monster_kill(GameState *state, int monster_id)
{
    /* 전투에서 몬스터를 처치했을 때 처치 수를 증가시키고 자동으로 발견 처리한다. */
    MonsterBookEntry *entry = find_book_entry(state, monster_id);

    if (entry != NULL) {
        entry->kill_count++;
        entry->discover = 1;
    }
}

int is_monster_discovered(GameState *state, int monster_id)
{
    /* 해당 몬스터가 현재 플레이어 도감에 등록되어 있는지 확인한다. */
    MonsterBookEntry *entry = find_book_entry(state, monster_id);
    return entry != NULL && entry->discover;
}

void check_unlocks(GameState *state)
{
    /*
     * unlock.txt에서 읽은 해금 조건을 검사한다.
     * 층 도달 조건이나 특정 몬스터 처치 수 조건을 만족하면 새 몬스터를 발견 처리한다.
     */
    int i;

    for (i = 0; i < state->data.unlock_count; i++) {
        UnlockRule *rule = &state->data.unlock_rules[i];
        MonsterDef *target = find_monster_by_name(&state->data, rule->target_name);

        if (target == NULL || is_monster_discovered(state, target->id)) {
            continue;
        }

        if (rule->type == UNLOCK_BY_FLOOR) {
            if (state->current_floor >= rule->required_value) {
                discover_monster(state, target->id);
            }
        } else {
            MonsterDef *source = find_monster_by_name(&state->data, rule->source_name);
            MonsterBookEntry *entry;

            if (source == NULL) {
                continue;
            }

            entry = find_book_entry(state, source->id);
            if (entry != NULL && entry->kill_count >= rule->required_value) {
                discover_monster(state, target->id);
            }
        }
    }
}

void show_monster_book(GameState *state)
{
    /* 현재 발견한 몬스터와 아직 미등록인 몬스터를 나누어 출력한다. */
    int i;

    clear_screen();
    printf("\n===== 몬스터 도감 =====\n");
    printf("\n[등록]\n");
    for (i = 0; i < state->data.monster_count; i++) {
        if (state->book[i].discover) {
            MonsterDef *monster = find_monster_by_id(&state->data, state->book[i].monster_id);
            if (monster != NULL) {
                printf("%s (처치 %d)\n", monster->name, state->book[i].kill_count);
            }
        }
    }

    printf("\n[미등록]\n");
    for (i = 0; i < state->data.monster_count; i++) {
        if (!state->book[i].discover) {
            printf("???\n");
        }
    }

    wait_for_enter();
    clear_screen();
}
