#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <stdio.h>

/*
 * path_utils.h
 * 실행 위치가 달라져도 data 폴더의 파일을 찾을 수 있도록 보조하는 파일 경로 유틸이다.
 */

/* data 폴더에 있는 파일을 열고, 실패하면 소스 위치 기준 경로로 한 번 더 시도한다. */
FILE *open_data_file(const char *filename, const char *mode);

#endif
