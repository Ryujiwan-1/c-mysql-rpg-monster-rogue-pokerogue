#ifndef INVENTORY_H
#define INVENTORY_H

#include "models.h"

/*
 * inventory.h
 * 장비 드랍, 인벤토리 출력, 장착, 강화, 분해와 관련된 함수들을 선언한다.
 */

/* Rarity enum 값을 화면 출력/DB 저장용 한글 문자열로 바꾼다. */
const char *rarity_to_string(Rarity rarity);

/* 인벤토리 화면을 출력하고, 장비 착용/버리기/분해/강화 입력을 처리한다. */
void show_inventory(Player *player);

/* 전투 보상으로 무작위 장비를 생성해 플레이어 인벤토리에 추가한다. */
void add_random_equipment(GameState *state, int floor);

/* 플레이어 기본 공격력과 착용 장비 공격력을 합산한다. */
int get_player_total_atk(Player *player);

/* 플레이어 기본 최대 HP와 착용 장비 HP 옵션을 합산한다. */
int get_player_total_max_hp(Player *player);

/* 장비 등급에 따른 강화 재료 가치를 반환한다. */
int get_rarity_material(Rarity rarity);

#endif
