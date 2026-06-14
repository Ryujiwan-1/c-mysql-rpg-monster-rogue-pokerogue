#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "models.h"

/*
 * data_loader.h
 * data 폴더의 텍스트 파일을 읽어 GameData 구조체에 채우는 함수들을 선언한다.
 * 파일 데이터는 게임 기준 데이터이고, 계정별 저장 데이터는 database 모듈이 담당한다.
 */

/* monster.txt, item.txt, unlock.txt, stage.txt를 모두 읽어 GameData에 저장한다. */
int load_game_data(GameData *data);

/* 몬스터 이름으로 원본 몬스터 데이터를 찾는다. stage/unlock 규칙 처리에 사용한다. */
MonsterDef *find_monster_by_name(GameData *data, const char *name);

/* monster_id로 원본 몬스터 데이터를 찾는다. 도감과 DB 데이터를 화면에 표시할 때 사용한다. */
MonsterDef *find_monster_by_id(GameData *data, int id);

/* item_id로 원본 장비 데이터를 찾는다. 현재는 확장 기능을 위한 검색 함수이다. */
ItemDef *find_item_by_id(GameData *data, int id);

#endif
