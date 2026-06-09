#include "data_loader.h"

#include "path_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *text)
{
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
    int i;
    for (i = 0; i < data->item_count; i++) {
        if (data->items[i].id == id) {
            return &data->items[i];
        }
    }
    return NULL;
}
