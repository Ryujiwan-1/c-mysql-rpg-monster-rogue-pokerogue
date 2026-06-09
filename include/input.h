#ifndef INPUT_H
#define INPUT_H

/* scanf 실패 시 EOF를 구분하기 위한 값 */
#define INPUT_EOF 0
#define INPUT_OK 1
#define INPUT_INVALID -1

int read_int(int *value);
int read_word(char *text, int size);

#endif
