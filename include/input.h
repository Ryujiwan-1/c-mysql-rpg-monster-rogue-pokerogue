#ifndef INPUT_H
#define INPUT_H

/*
 * input.h
 * scanf를 직접 반복해서 쓰지 않도록 입력 처리를 모아 둔 헤더이다.
 * EOF 상황과 잘못된 입력을 구분해 무한 루프를 방지한다.
 */

/* scanf 실패 시 EOF를 구분하기 위한 반환값 */
#define INPUT_EOF 0
#define INPUT_OK 1
#define INPUT_INVALID -1

/* 정수 하나를 읽는다. 성공/EOF/잘못된 입력을 반환값으로 구분한다. */
int read_int(int *value);

/* 공백 없는 단어 하나를 읽는다. size를 이용해 버퍼 초과를 막는다. */
int read_word(char *text, int size);

#endif
