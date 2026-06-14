#include "console_utils.h"

#include <stdio.h>
#include <stdlib.h>

void clear_screen(void)
{
    /*
     * 전투나 메뉴가 바뀔 때 이전 출력이 계속 남아 있으면 화면이 복잡해진다.
     * Windows에서는 cls, macOS/Linux에서는 clear 명령으로 콘솔 화면을 지운다.
     */
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void wait_for_enter(void)
{
    /*
     * 화면을 바로 지우면 승리/패배/보상 메시지를 읽기 전에 사라질 수 있다.
     * 그래서 중요한 안내를 출력한 뒤 Enter를 누를 때까지 잠깐 멈춘다.
     */
    int ch;

    printf("\n계속하려면 Enter를 누르세요...");
    fflush(stdout);

    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}
