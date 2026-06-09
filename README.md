# Monster Rogue

C 언어와 MySQL 연동 구조를 사용하는 콘솔 기반 무한 층 로그라이크 RPG 프로젝트이다.

## 빌드

게임 진행 정보는 MySQL DB에 저장된다. 파일은 몬스터, 아이템, 해금, 스테이지 같은 실행 데이터 로드에만 사용한다.

### Windows / Visual Studio 2022

Visual Studio 2022 Community가 설치된 Windows에서는 `MonsterRogue.sln`을 열어 `x64 / Debug`로 빌드한다.

현재 프로젝트 파일은 기본 MySQL 경로를 프로젝트 내부의 MySQL 5.7 폴더로 잡아 둔다.

```text
mysql57\server
```

다른 위치에 MySQL Connector 또는 MySQL Server가 설치되어 있으면 Visual Studio의 프로젝트 속성에서 `MySqlDir` 매크로를 바꾸거나, 명령 프롬프트에서 환경 변수를 지정한다.

```bat
set "MySqlDir=C:\Users\User\Desktop\c-mysql-rpg-monster-rogue-pokerogue\mysql57\server"
build_vs2022.bat
run_vs2022.bat
```

빌드 결과는 아래에 생성된다.

```text
build\vs2022\Debug\monster_rogue.exe
```

실행할 때는 작업 디렉터리가 프로젝트 루트여야 `data` 폴더를 찾을 수 있다. Visual Studio 프로젝트에는 작업 디렉터리가 프로젝트 루트로 설정되어 있다.

MySQL 5.7 서버는 프로젝트 루트에서 아래 스크립트로 실행/종료한다.

```bat
setup_mysql57.bat
start_mysql57.bat
stop_mysql57.bat
```

접속 정보는 소스 코드와 동일하게 `127.0.0.1:3307`, `root/root`, DB 이름 `monster_rogue`이다.

### macOS / Linux

```sh
make
./build/monster_rogue
```

MySQL C Connector가 설치되어 있고 실제 DB 연동을 켜려면:

```sh
make mysql
./build/monster_rogue
```

## 데이터 파일

- `data/monster.txt`: 몬스터 기본 정보
- `data/item.txt`: 장비 기본 정보
- `data/unlock.txt`: 도감 해금 조건
- `data/stage.txt`: 층별 출현 몬스터 목록
- 세이브, 인벤토리, 도감, 랭킹은 MySQL DB에 저장

## MySQL 준비

```sh
mysql -u root -p < sql/schema.sql
```

DB 접속 정보는 실행 시 환경 변수로 설정할 수 있다.

```sh
export MR_DB_HOST=127.0.0.1
export MR_DB_USER=root
export MR_DB_PASS=password
export MR_DB_NAME=monster_rogue
```

## 구현된 기능

- 메뉴 기반 콘솔 UI
- 로그인/회원가입
- 새 게임/DB 세이브 로드
- 무한 층 진행
- 턴제 전투
- 승리 보상 선택
- 장비 드랍/인벤토리/장착
- 장비 버리기/분해/강화
- 몬스터 도감/해금
- DB 저장/불러오기
- DB 랭킹
- MySQL 연동용 모듈과 스키마
