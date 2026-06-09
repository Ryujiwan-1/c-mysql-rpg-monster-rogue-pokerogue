#include "battle.h"

#include "data_loader.h"
#include "input.h"
#include "inventory.h"
#include "monster_book.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void gain_exp(Player *player, int amount)
{
    int need;

    player->exp += amount;
    printf("EXP +%d\n", amount);

    need = player->level * 100;
    while (player->exp >= need) {
        player->exp -= need;
        player->level++;
        player->max_hp += 8;
        player->base_atk += 1;
        player->hp = get_player_total_max_hp(player);
        printf("LEVEL UP! 현재 레벨: %d\n", player->level);
        need = player->level * 100;
    }
}

static MonsterDef *pick_stage_monster(GameState *state, int floor)
{
    StageRule *selected = NULL;
    MonsterDef *candidates[MAX_STAGE_MONSTERS];
    int count = 0;
    int i;

    for (i = 0; i < state->data.stage_count; i++) {
        if (floor >= state->data.stages[i].min_floor) {
            selected = &state->data.stages[i];
        }
    }

    if (selected != NULL) {
        for (i = 0; i < selected->monster_count; i++) {
            MonsterDef *monster = find_monster_by_name(&state->data, selected->monster_names[i]);
            if (monster != NULL && is_monster_discovered(state, monster->id)) {
                candidates[count] = monster;
                count++;
            }
        }
    }

    if (count == 0) {
        for (i = 0; i < state->data.monster_count && count < MAX_STAGE_MONSTERS; i++) {
            if (is_monster_discovered(state, state->data.monsters[i].id)) {
                candidates[count] = &state->data.monsters[i];
                count++;
            }
        }
    }

    if (count == 0) {
        return &state->data.monsters[0];
    }

    return candidates[rand() % count];
}

Monster create_scaled_monster(GameState *state, int floor)
{
    MonsterDef *def = pick_stage_monster(state, floor);
    Monster monster;
    /* 스탯 보상이 5층마다 나오므로 몬스터도 5층 단위로 한 번 더 강해진다. */
    double hp_scale = 1.0 + floor * 0.12 + (floor / 5) * 0.10;
    double atk_scale = 1.0 + floor * 0.08 + (floor / 5) * 0.06;

    monster.id = def->id;
    strncpy(monster.name, def->name, MAX_NAME_LEN - 1);
    monster.name[MAX_NAME_LEN - 1] = '\0';
    monster.max_hp = (int)(def->base_hp * hp_scale);
    monster.hp = monster.max_hp;
    monster.atk = (int)(def->base_atk * atk_scale);

    return monster;
}

static void show_battle_status(Player *player, Monster *monster)
{
    printf("\n%s 등장!\n", monster->name);
    printf("몬스터 HP: %d / %d  ATK: %d\n", monster->hp, monster->max_hp, monster->atk);
    printf("플레이어 HP: %d / %d  ATK: %d\n", player->hp,
           get_player_total_max_hp(player), get_player_total_atk(player));
}

static void player_reward(GameState *state)
{
    int choice;
    Player *player = &state->player;

    printf("\n===== 5층 돌파 보상 =====\n");
    printf("1. 공격력 증가 (+4)\n");
    printf("2. 체력 증가 (+20)\n");
    printf("3. 강화 재료 획득 (+5)\n");
    printf("선택: ");

    if (read_int(&choice) != INPUT_OK) {
        choice = 1;
    }

    if (choice == 1) {
        player->base_atk += 4;
        printf("공격력이 증가했습니다.\n");
    } else if (choice == 2) {
        player->max_hp += 20;
        player->hp = get_player_total_max_hp(player);
        printf("최대 체력이 증가했습니다.\n");
    } else {
        player->material += 5;
        printf("강화 재료를 획득했습니다. 보유 재료: %d\n", player->material);
    }
}

int run_battle(GameState *state)
{
    Player *player = &state->player;
    Monster monster = create_scaled_monster(state, state->current_floor);
    int guarding = 0;

    while (player->hp > 0 && monster.hp > 0) {
        int choice;
        int damage;

        show_battle_status(player, &monster);
        printf("\n1. 공격\n");
        printf("2. 스킬(강공격)\n");
        printf("3. 아이템(응급 회복)\n");
        printf("선택: ");

        if (read_int(&choice) != INPUT_OK) {
            choice = 1;
        }

        guarding = 0;

        if (choice == 2) {
            damage = (int)(get_player_total_atk(player) * 1.6);
            monster.hp -= damage;
            printf("강공격! %d 피해\n", damage);
        } else if (choice == 3) {
            int heal = 20 + player->level * 5;
            player->hp += heal;
            if (player->hp > get_player_total_max_hp(player)) {
                player->hp = get_player_total_max_hp(player);
            }
            guarding = 1;
            printf("응급 회복으로 HP %d 회복\n", heal);
        } else {
            damage = get_player_total_atk(player);
            monster.hp -= damage;
            printf("공격! %d 피해\n", damage);
        }

        if (monster.hp <= 0) {
            break;
        }

        damage = guarding ? monster.atk / 2 : monster.atk;
        player->hp -= damage;
        printf("%s의 반격! %d 피해\n", monster.name, damage);
    }

    if (player->hp <= 0) {
        printf("\n플레이어가 쓰러졌습니다.\n");
        return 0;
    }

    printf("\n승리!\n");
    add_monster_kill(state, monster.id);
    gain_exp(player, 40 + state->current_floor * 5);
    player->gold += 20 + state->current_floor * 3;
    printf("Gold +%d\n", 20 + state->current_floor * 3);

    if (rand() % 100 < 70) {
        add_random_equipment(state, state->current_floor);
    }

    if (state->current_floor % 5 == 0) {
        player_reward(state);
    }
    check_unlocks(state);
    return 1;
}
