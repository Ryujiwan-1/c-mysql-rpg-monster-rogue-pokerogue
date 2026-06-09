#ifndef MODELS_H
#define MODELS_H

#define MAX_NAME_LEN 64
#define MAX_MONSTERS 64
#define MAX_ITEMS 64
#define MAX_INVENTORY 32
#define MAX_STAGE_RULES 32
#define MAX_STAGE_MONSTERS 8
#define MAX_UNLOCK_RULES 32

typedef enum {
    RARITY_COMMON,
    RARITY_UNCOMMON,
    RARITY_RARE,
    RARITY_HERO,
    RARITY_LEGEND,
    RARITY_MYTH
} Rarity;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int base_hp;
    int base_atk;
} MonsterDef;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int max_hp;
    int hp;
    int atk;
} Monster;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int base_atk;
} ItemDef;

typedef struct {
    char name[MAX_NAME_LEN];
    Rarity rarity;
    int atk;
    int hp;
    int equipped;
    int enhance_level;
} Item;

typedef struct {
    int monster_id;
    int discover;
    int kill_count;
} MonsterBookEntry;

typedef enum {
    UNLOCK_BY_KILL,
    UNLOCK_BY_FLOOR
} UnlockType;

typedef struct {
    char target_name[MAX_NAME_LEN];
    UnlockType type;
    char source_name[MAX_NAME_LEN];
    int required_value;
} UnlockRule;

typedef struct {
    int min_floor;
    char monster_names[MAX_STAGE_MONSTERS][MAX_NAME_LEN];
    int monster_count;
} StageRule;

typedef struct {
    MonsterDef monsters[MAX_MONSTERS];
    int monster_count;
    ItemDef items[MAX_ITEMS];
    int item_count;
    UnlockRule unlock_rules[MAX_UNLOCK_RULES];
    int unlock_count;
    StageRule stages[MAX_STAGE_RULES];
    int stage_count;
} GameData;

typedef struct {
    char nickname[MAX_NAME_LEN];
    int level;
    int exp;
    int gold;
    int best_floor;
    int material;
    int max_hp;
    int hp;
    int base_atk;
    Item inventory[MAX_INVENTORY];
    int inventory_count;
} Player;

typedef struct {
    Player player;
    GameData data;
    MonsterBookEntry book[MAX_MONSTERS];
    int account_id;
    char account_name[MAX_NAME_LEN];
    int current_floor;
    int running;
} GameState;

#endif
