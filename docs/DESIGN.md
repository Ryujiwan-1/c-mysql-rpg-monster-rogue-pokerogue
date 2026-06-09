# Monster Rogue 프로젝트 설계

Monster Rogue는 C 언어와 MySQL을 사용하는 콘솔 기반 무한 층 로그라이크 RPG이다.
참고 흐름은 PokeRogue의 층 진행 방식과 Dungeon Slasher의 장비 파밍 방식이며, 포획 시스템 대신 장비 드랍과 몬스터 도감을 사용한다.

## 폴더 구조

```text
MonsterRogue/
├── Makefile
├── README.md
├── data/
│   ├── item.txt
│   ├── monster.txt
│   ├── stage.txt
│   └── unlock.txt
├── docs/
│   └── DESIGN.md
├── include/
│   ├── battle.h
│   ├── data_loader.h
│   ├── database.h
│   ├── game.h
│   ├── inventory.h
│   ├── models.h
│   ├── monster_book.h
│   ├── player.h
│   ├── ranking.h
│   └── path_utils.h
├── sql/
│   └── schema.sql
└── src/
    ├── battle.c
    ├── data_loader.c
    ├── database.c
    ├── game.c
    ├── inventory.c
    ├── main.c
    ├── monster_book.c
    ├── player.c
    ├── path_utils.c
    ├── ranking.c
```

## 구조체 설계

핵심 구조체는 `include/models.h`에 모았다.

- `Player`: 닉네임, 레벨, 경험치, 골드, 강화 재료, 현재 HP, 최대 HP, 기본 공격력, 최고 층, 현재 장비, 인벤토리.
- `MonsterDef`: 파일에서 읽는 몬스터 원본 정보.
- `Monster`: 실제 전투에 사용하는 층 보정 완료 몬스터.
- `ItemDef`: 파일에서 읽는 아이템 원본 정보.
- `Item`: 실제 획득 장비. 등급, 공격력, 체력 옵션, 장착 여부, 강화 레벨 포함.
- `MonsterBookEntry`: 도감 등록 여부와 처치 수.
- `GameData`: 몬스터, 아이템, 스테이지, 해금 조건 전체 데이터.
- `GameState`: 플레이어, 도감, 현재 층, 전체 게임 데이터를 묶은 런타임 상태.

## DB 스키마

요구사항의 테이블을 `sql/schema.sql`에 정의한다.

- `player`: 플레이어 기본 정보.
- `inventory`: 플레이어 장비 목록.
- `monster_book`: 몬스터 발견 여부와 처치 수.
- `run_history`: 사망 또는 종료 시 런 기록.
- `ranking`: 최고 기록 조회용 랭킹.

DB 연동 코드는 `database.c`에 분리했다. 계정, 세이브, 인벤토리, 도감, 랭킹은 MySQL에 저장하고, 파일은 몬스터/아이템/스테이지/해금 데이터 로드에만 사용한다.

## 모듈 분리 계획

- `main.c`: 프로그램 시작점.
- `game.c`: 메뉴, 게임 시작, 층 진행 루프.
- `battle.c`: 턴제 전투, 몬스터 스케일링, 경험치/골드 보상.
- `inventory.c`: 장비 생성, 등급 결정, 인벤토리, 장착.
- `monster_book.c`: 도감 표시, 처치 수 누적, 조건 해금.
- `data_loader.c`: `monster.txt`, `item.txt`, `unlock.txt`, `stage.txt` 로딩.
- `player.c`: 새 플레이어 기본값 초기화.
- `ranking.c`: 점수 계산.
- `database.c`: MySQL 연결, 테이블 저장 확장 지점.

## 주요 게임 흐름

```text
게임 시작
-> 세이브 로드 또는 새 게임
-> 현재 층 몬스터 랜덤 등장
-> 턴제 전투
-> 승리 시 보상 선택
-> 장비 드랍
-> 도감/해금 갱신
-> 다음 층
-> 사망 시 결과 저장
-> 랭킹 출력
```

## 확장 방향

- 스킬 종류를 늘릴 때는 `battle.c`의 `player_turn`을 확장한다.
- 장비 옵션을 늘릴 때는 `Item` 구조체와 `inventory.c`의 드랍 생성 로직을 확장한다.
- 몬스터 해금 조건은 `unlock.txt` 데이터만 추가해도 기본적인 처치/층 조건을 처리할 수 있다.
- MySQL 저장을 강화하려면 `database.c`의 stub 함수 내부에 prepared statement를 추가한다.
