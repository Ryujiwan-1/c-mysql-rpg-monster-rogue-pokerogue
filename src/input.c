#include "input.h"

#include <stdio.h>

/*
 * scanf는 숫자 입력에 실패했을 때 잘못 입력된 문자를 버퍼에 남긴다.
 * 예를 들어 정수를 입력해야 하는데 "abc"를 입력하면 다음 scanf도 같은
 * "abc"를 다시 읽으면서 메뉴가 반복해서 실패할 수 있다.
 *
 * 그래서 입력을 한 번 처리한 뒤에는 줄 끝('\n')까지 버퍼를 비운다.
 * 단, 입력 스트림이 닫힌 경우 EOF가 나오므로 EOF도 반드시 조건에 넣어야
 * 무한 루프가 생기지 않는다.
 */
static void clear_input_line(void)
{
    int ch;

    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}

int read_int(int *value)
{
    int result = scanf("%d", value);

    /*
     * scanf 반환값:
     *  - 1      : 정수 하나를 정상적으로 읽음
     *  - EOF    : 입력이 닫힘
     *  - 그 외  : 정수가 아닌 값이 들어오는 등 입력 실패
     */
    if (result == 1) {
        clear_input_line();
        return INPUT_OK;
    }

    if (result == EOF) {
        return INPUT_EOF;
    }

    clear_input_line();
    return INPUT_INVALID;
}

int read_word(char *text, int size)
{
    int result;
    char format[16];

    /*
     * 호출한 쪽에서 잘못된 배열을 넘겼을 때를 방어한다.
     * size가 1 이하이면 문자열 끝을 의미하는 '\0'도 저장하기 어렵다.
     */
    if (text == NULL || size <= 1) {
        return INPUT_INVALID;
    }

    /*
     * "%63s"처럼 입력 가능한 최대 길이를 제한한 format 문자열을 만든다.
     * 이렇게 해야 사용자가 너무 긴 문자열을 입력해도 배열 범위를 넘지 않는다.
     */
    snprintf(format, sizeof(format), "%%%ds", size - 1);
    result = scanf(format, text);

    if (result == 1) {
        clear_input_line();
        return INPUT_OK;
    }

    if (result == EOF) {
        return INPUT_EOF;
    }

    clear_input_line();
    return INPUT_INVALID;
}
