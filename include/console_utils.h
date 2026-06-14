#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

/*
 * console_utils.h
 * 콘솔 화면 정리와 일시 정지를 담당하는 작은 유틸 함수 선언 파일이다.
 */

/* 이전 출력 내용을 지워 메뉴나 전투 화면을 깔끔하게 다시 그린다. */
void clear_screen(void);

/* 중요한 메시지를 읽을 수 있도록 Enter 입력을 기다린다. */
void wait_for_enter(void);

#endif
