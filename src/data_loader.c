#include "data_loader.h"

#include "path_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 데이터 로더는 data 폴더의 txt 파일을 읽어 GameData 구조체에 저장한다.
 * 파일은 몬스터/아이템/스테이지 같은 고정 데이터만 담당하고,
 * 계정별 세이브 정보는 database.c를 통해 MySQL에 저장한다.
 */

static void trim_newline(char *text)
{
    /*
     * fgets()로 한 줄을 읽으면 줄 끝에 '\n' 또는 Windows 파일의 '\r\n'이 남을 수 있다.
     * 이름 비교를 정확히 하기 위해 줄바꿈 문자를 문자열 끝('\0')으로 바꾼다.
     */
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

static int parse_monsters(GameData *data)
{
    FILE *fp = open_data_file("monster.txt", "r");
    char line[256];

    if (fp == NULL) {
        return 0;
    }

    /*
     * monster.txt 형식:
     * id,이름,기본HP,기본공격력
     * 예: 1,슬라임,30,5
     */
    while (fgets(line, sizeof(line), fp) != NULL && data->monster_count < MAX_MONSTERS) {
        MonsterDef *monster = &data->monsters[data->monster_count];
        trim_newline(line);
        if (sscanf(line, "%d,%63[^,],%d,%d", &monster->id, monster->name,
                   &monster->base_hp, &monster->base_atk) == 4) {
            data->monster_count++;
        }
    }

    fclose(fp);
    return 1;
}

static int parse_items(GameData *data)
{
    FILE *fp = open_data_file("item.txt", "r");
    char line[256];

    if (fp == NULL) {
        return 0;
    }

    /*
     * item.txt 형식:
     * id,장비이름,기본공격력
     */
    while (fgets(line, sizeof(line), fp) != NULL && data->item_count < MAX_ITEMS) {
        ItemDef *item = &data->items[data->item_count];
        trim_newline(line);
        if (sscanf(line, "%d,%63[^,],%d", &item->id, item->name, &item->base_atk) == 3) {
            data->item_count++;
        }
    }

    fclose(fp);
    return 1;
}

static int parse_unlocks(GameData *data)
{
    FILE *fp = open_data_file("unlock.txt", "r");
    char line[256];

    if (fp == NULL) {
        return 0;
    }

    /*
     * unlock.txt는 두 가지 형식을 처리한다.
     * 1) 몬스터이름,5층        -> 특정 층 도달 시 해금
     * 2) 몬스터이름,재료몬스터,3 -> 재료몬스터를 3번 처치하면 해금
     */
    while (fgets(line, sizeof(line), fp) != NULL && data->unlock_count < MAX_UNLOCK_RULES) {
        UnlockRule *rule = &data->unlock_rules[data->unlock_count];
        char second[MAX_NAME_LEN] = "";
        int value = 0;

        trim_newline(line);

        if (sscanf(line, "%63[^,],%d층", rule->target_name, &value) == 2) {
            rule->type = UNLOCK_BY_FLOOR;
            rule->required_value = value;
            rule->source_name[0] = '\0';
            data->unlock_count++;
        } else if (sscanf(line, "%63[^,],%63[^,],%d", rule->target_name, second, &value) == 3) {
            rule->type = UNLOCK_BY_KILL;
            strncpy(rule->source_name, second, MAX_NAME_LEN - 1);
            rule->source_name[MAX_NAME_LEN - 1] = '\0';
            rule->required_value = value;
            data->unlock_count++;
        }
    }

    fclose(fp);
    return 1;
}

static int parse_stages(GameData *data)
{
    FILE *fp = open_data_file("stage.txt", "r");
    char line[256];

    if (fp == NULL) {
        return 0;
    }

    /*
     * stage.txt 형식:
     * 시작층,몬스터1,몬스터2,...
     * 현재 층보다 시작층이 낮거나 같은 규칙 중 가장 마지막 규칙을 사용한다.
     */
    while (fgets(line, sizeof(line), fp) != NULL && data->stage_count < MAX_STAGE_RULES) {
        StageRule *stage = &data->stages[data->stage_count];
        char *token;

        trim_newline(line);
        token = strtok(line, ",");
        if (token == NULL) {
            continue;
        }

        stage->min_floor = atoi(token);
        stage->monster_count = 0;

        while ((token = strtok(NULL, ",")) != NULL && stage->monster_count < MAX_STAGE_MONSTERS) {
            strncpy(stage->monster_names[stage->monster_count], token, MAX_NAME_LEN - 1);
            stage->monster_names[stage->monster_count][MAX_NAME_LEN - 1] = '\0';
            stage->monster_count++;
        }

        if (stage->monster_count > 0) {
            data->stage_count++;
        }
    }

    fclose(fp);
    return 1;
}

int load_game_data(GameData *data)
{
    /*
     * GameData를 먼저 0으로 초기화해야 count 값들이 0에서 시작한다.
     * 초기화하지 않으면 쓰레기 값 때문에 배열 범위를 넘어갈 수 있다.
     */
    memset(data, 0, sizeof(GameData));

    if (!parse_monsters(data)) {
        printf("monster.txt 파일을 열 수 없습니다.\n");
        return 0;
    }
    if (!parse_items(data)) {
        printf("item.txt 파일을 열 수 없습니다.\n");
        return 0;
    }
    if (!parse_unlocks(data)) {
        printf("unlock.txt 파일을 열 수 없습니다.\n");
        return 0;
    }
    if (!parse_stages(data)) {
        printf("stage.txt 파일을 열 수 없습니다.\n");
        return 0;
    }

    return 1;
}

MonsterDef *find_monster_by_name(GameData *data, const char *name)
{
    /* 몬스터 이름으로 MonsterDef를 찾는다. 스테이지/해금 규칙 처리에서 사용한다. */
    int i;
    for (i = 0; i < data->monster_count; i++) {
        if (strcmp(data->monsters[i].name, name) == 0) {
            return &data->monsters[i];
        }
    }
    return NULL;
}

MonsterDef *find_monster_by_id(GameData *data, int id)
{
    /* DB나 도감에는 monster_id가 저장되므로, id로 원본 몬스터 정보를 다시 찾을 때 사용한다. */
    int i;
    for (i = 0; i < data->monster_count; i++) {
        if (data->monsters[i].id == id) {
            return &data->monsters[i];
        }
    }
    return NULL;
}

ItemDef *find_item_by_id(GameData *data, int id)
{
    /* 현재 코드에서는 확장용 함수에 가깝다. 아이템 id로 원본 장비 정보를 찾을 수 있다. */
    int i;
    for (i = 0; i < data->item_count; i++) {
        if (data->items[i].id == id) {
            return &data->items[i];
        }
    }
    return NULL;
}
