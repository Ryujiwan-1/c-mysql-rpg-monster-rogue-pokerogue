#include "game.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(void)
{
#ifdef _WIN32
    /*
     * Windows 콘솔은 기본 코드 페이지가 UTF-8이 아닐 수 있다.
     * 이 프로젝트는 메뉴와 데이터에 한글을 사용하므로, 실행 시작 시 콘솔의
     * 입력/출력 코드 페이지를 UTF-8로 맞춰 한글 깨짐을 줄인다.
     */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    /* 실제 게임 메뉴, 데이터 로딩, DB 연결은 game.c의 run_game()에서 시작한다. */
    run_game();
    return 0;
}
