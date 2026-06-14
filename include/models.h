#ifndef MODELS_H
#define MODELS_H

#define MAX_NAME_LEN 64
#define MAX_MONSTERS 64
#define MAX_ITEMS 64
#define MAX_INVENTORY 32
#define MAX_STAGE_RULES 32
#define MAX_STAGE_MONSTERS 8
#define MAX_UNLOCK_RULES 32

/*
 * 장비 등급을 숫자로 관리하기 위한 enum이다.
 * 문자열로 직접 비교하는 것보다 enum을 쓰면 switch문으로 등급별 보너스를 처리하기 쉽다.
 */
typedef enum {
    RARITY_COMMON,
    RARITY_UNCOMMON,
    RARITY_RARE,
    RARITY_HERO,
    RARITY_LEGEND,
    RARITY_MYTH
} Rarity;

/*
 * MonsterDef는 data/monster.txt에서 읽어 온 "원본 몬스터 정보"이다.
 * 실제 전투에서는 층수에 따라 HP/공격력이 바뀌므로 Monster 구조체를 따로 사용한다.
 */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int base_hp;
    int base_atk;
} MonsterDef;

/*
 * Monster는 전투에 실제로 등장한 몬스터 정보이다.
 * MonsterDef의 기본 능력치에 현재 층 보정을 적용한 결과가 들어간다.
 */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int max_hp;
    int hp;
    int atk;
} Monster;

/* ItemDef는 data/item.txt에서 읽어 온 장비의 기본 정보이다. */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    int base_atk;
} ItemDef;

/*
 * Item은 플레이어가 실제로 획득한 장비이다.
 * 같은 ItemDef에서 나온 장비라도 등급, 옵션, 강화 단계가 달라질 수 있다.
 */
typedef struct {
    char name[MAX_NAME_LEN];
    Rarity rarity;
    int atk;
    int hp;
    int equipped;
    int enhance_level;
} Item;

/*
 * MonsterBookEntry는 플레이어의 몬스터 도감 상태를 저장한다.
 * discover는 발견 여부, kill_count는 처치 횟수이다.
 */
typedef struct {
    int monster_id;
    int discover;
    int kill_count;
} MonsterBookEntry;

/* 도감 해금 조건의 종류이다. 처치 수 조건과 층 도달 조건을 구분한다. */
typedef enum {
    UNLOCK_BY_KILL,
    UNLOCK_BY_FLOOR
} UnlockType;

/*
 * UnlockRule은 data/unlock.txt의 한 줄을 구조체로 옮긴 것이다.
 * 예: 특정 몬스터를 몇 번 잡으면 target_name 몬스터가 해금된다.
 */
typedef struct {
    char target_name[MAX_NAME_LEN];
    UnlockType type;
    char source_name[MAX_NAME_LEN];
    int required_value;
} UnlockRule;

/*
 * StageRule은 data/stage.txt의 한 줄을 구조체로 옮긴 것이다.
 * min_floor 이상부터 어떤 몬스터들이 등장할 수 있는지 저장한다.
 */
typedef struct {
    int min_floor;
    char monster_names[MAX_STAGE_MONSTERS][MAX_NAME_LEN];
    int monster_count;
} StageRule;

/*
 * GameData는 파일에서 읽은 고정 게임 데이터를 모두 모아 둔 구조체이다.
 * 실행 중 바뀌는 계정 정보가 아니라, 몬스터/아이템/스테이지 같은 기준 데이터이다.
 */
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

/*
 * Player는 계정별로 저장되는 플레이어 진행 정보이다.
 * 레벨, 경험치, 골드, 장비, 강화 재료처럼 게임을 진행하며 바뀌는 값이 들어간다.
 */
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

/*
 * GameState는 실행 중 필요한 전체 상태를 묶은 구조체이다.
 * Player, GameData, 도감, 로그인 계정, 현재 층을 한 번에 전달하기 위해 사용한다.
 */
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
