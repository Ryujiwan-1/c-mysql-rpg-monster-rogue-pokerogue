#include "inventory.h"
#include "console_utils.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Rarity roll_rarity(void)
{
    int roll = rand() % 100;

    if (roll < 45) return RARITY_COMMON;
    if (roll < 70) return RARITY_UNCOMMON;
    if (roll < 87) return RARITY_RARE;
    if (roll < 96) return RARITY_HERO;
    if (roll < 99) return RARITY_LEGEND;
    return RARITY_MYTH;
}

static int rarity_bonus(Rarity rarity)
{
    switch (rarity) {
    case RARITY_COMMON: return 0;
    case RARITY_UNCOMMON: return 3;
    case RARITY_RARE: return 7;
    case RARITY_HERO: return 14;
    case RARITY_LEGEND: return 25;
    case RARITY_MYTH: return 40;
    default: return 0;
    }
}

int get_rarity_material(Rarity rarity)
{
    switch (rarity) {
    case RARITY_COMMON: return 1;
    case RARITY_UNCOMMON: return 2;
    case RARITY_RARE: return 4;
    case RARITY_HERO: return 7;
    case RARITY_LEGEND: return 11;
    case RARITY_MYTH: return 16;
    default: return 1;
    }
}

const char *rarity_to_string(Rarity rarity)
{
    switch (rarity) {
    case RARITY_COMMON: return "일반";
    case RARITY_UNCOMMON: return "고급";
    case RARITY_RARE: return "희귀";
    case RARITY_HERO: return "영웅";
    case RARITY_LEGEND: return "전설";
    case RARITY_MYTH: return "신화";
    default: return "알수없음";
    }
}

static void remove_item(Player *player, int index)
{
    int i;

    for (i = index; i < player->inventory_count - 1; i++) {
        player->inventory[i] = player->inventory[i + 1];
    }
    player->inventory_count--;
}

static void equip_item(Player *player, int index)
{
    int i;

    for (i = 0; i < player->inventory_count; i++) {
        player->inventory[i].equipped = 0;
    }
    player->inventory[index].equipped = 1;
    printf("%s 장착 완료!\n", player->inventory[index].name);
}

static void disassemble_item(Player *player, int index)
{
    Item *item = &player->inventory[index];
    int gain = get_rarity_material(item->rarity) + item->enhance_level;

    player->material += gain;
    printf("%s 분해 완료! 강화 재료 +%d (보유 재료: %d)\n",
           item->name, gain, player->material);
    remove_item(player, index);
}

static void enhance_item(Player *player, int index)
{
    Item *item = &player->inventory[index];
    int cost = (item->enhance_level + 1) * 2 + get_rarity_material(item->rarity);
    int atk_up = 2 + get_rarity_material(item->rarity);
    int hp_up = 4 + get_rarity_material(item->rarity) * 2;

    if (player->material < cost) {
        printf("재료가 부족합니다. 필요: %d / 보유: %d\n", cost, player->material);
        return;
    }

    player->material -= cost;
    item->enhance_level++;
    item->atk += atk_up;
    item->hp += hp_up;
    printf("%s 강화 성공! +%d (ATK +%d, HP +%d, 남은 재료: %d)\n",
           item->name, item->enhance_level, atk_up, hp_up, player->material);
}

int get_player_total_atk(Player *player)
{
    int i;
    int total = player->base_atk;

    for (i = 0; i < player->inventory_count; i++) {
        if (player->inventory[i].equipped) {
            total += player->inventory[i].atk;
        }
    }

    return total;
}

int get_player_total_max_hp(Player *player)
{
    int i;
    int total = player->max_hp;

    for (i = 0; i < player->inventory_count; i++) {
        if (player->inventory[i].equipped) {
            total += player->inventory[i].hp;
        }
    }

    return total;
}

void show_inventory(Player *player)
{
    int i;
    int choice;
    int action;

    clear_screen();
    printf("\n===== 인벤토리 =====\n");
    printf("강화 재료: %d\n", player->material);
    if (player->inventory_count == 0) {
        printf("보유 장비가 없습니다.\n");
        wait_for_enter();
        return;
    }

    for (i = 0; i < player->inventory_count; i++) {
        Item *item = &player->inventory[i];
        printf("%2d. [%s] %s +%d ATK +%d HP +%d %s\n", i + 1,
               rarity_to_string(item->rarity), item->name, item->enhance_level,
               item->atk, item->hp,
               item->equipped ? "(장착중)" : "");
    }

    printf("선택할 장비 번호 입력(0 취소): ");
    if (read_int(&choice) != INPUT_OK) {
        clear_screen();
        return;
    }

    if (choice <= 0 || choice > player->inventory_count) {
        clear_screen();
        return;
    }

    choice--;
    printf("\n1. 착용\n");
    printf("2. 버리기\n");
    printf("3. 분해\n");
    printf("4. 강화\n");
    printf("0. 취소\n");
    printf("선택: ");

    if (read_int(&action) != INPUT_OK) {
        clear_screen();
        return;
    }

    if (action == 1) {
        equip_item(player, choice);
    } else if (action == 2) {
        printf("%s 버림\n", player->inventory[choice].name);
        remove_item(player, choice);
    } else if (action == 3) {
        disassemble_item(player, choice);
    } else if (action == 4) {
        enhance_item(player, choice);
    }

    wait_for_enter();
    clear_screen();
}

void add_random_equipment(GameState *state, int floor)
{
    Player *player = &state->player;
    ItemDef *def;
    Item item;
    int index;

    if (state->data.item_count == 0) {
        return;
    }

    index = rand() % state->data.item_count;
    def = &state->data.items[index];

    /* 장비 공격력은 기본 공격력 + 층 보정 + 등급 보너스로 계산한다. */
    memset(&item, 0, sizeof(Item));
    strncpy(item.name, def->name, MAX_NAME_LEN - 1);
    item.name[MAX_NAME_LEN - 1] = '\0';
    item.rarity = roll_rarity();
    item.atk = def->base_atk + floor * 2 + rarity_bonus(item.rarity);
    item.hp = rarity_bonus(item.rarity) * 2;
    item.equipped = 0;
    item.enhance_level = 0;

    if (player->inventory_count >= MAX_INVENTORY) {
        printf("인벤토리가 가득 차 장비를 획득하지 못했습니다.\n");
        return;
    }

    player->inventory[player->inventory_count] = item;
    player->inventory_count++;

    printf("\n장비 획득: [%s] %s (ATK +%d, HP +%d)\n",
           rarity_to_string(item.rarity), item.name, item.atk, item.hp);
}
