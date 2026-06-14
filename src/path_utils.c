#include "path_utils.h"

#include <stdio.h>
#include <string.h>

static int build_source_data_path(char *out, size_t out_size, const char *filename)
{
    /*
     * Visual Studio에서 실행 위치가 프로젝트 루트가 아닐 때 data 파일을 못 찾을 수 있다.
     * 이 함수는 현재 소스 파일 경로(__FILE__)를 이용해 프로젝트 루트/data 경로를 추정한다.
     */
    char source_path[512];
    char *src_pos;

    strncpy(source_path, __FILE__, sizeof(source_path) - 1);
    source_path[sizeof(source_path) - 1] = '\0';

    src_pos = strstr(source_path, "/src/path_utils.c");
    if (src_pos == NULL) {
        return 0;
    }

    *src_pos = '\0';
    snprintf(out, out_size, "%s/data/%s", source_path, filename);
    return 1;
}

FILE *open_data_file(const char *filename, const char *mode)
{
    char path[1024];
    FILE *fp;

    /*
     * 1차 시도: 현재 작업 디렉터리 기준 data 폴더에서 파일을 연다.
     * 정상 실행에서는 Visual Studio 작업 디렉터리를 프로젝트 루트로 맞췄기 때문에 이 경로가 성공한다.
     */
    snprintf(path, sizeof(path), "data/%s", filename);
    fp = fopen(path, mode);
    if (fp != NULL) {
        return fp;
    }

    /*
     * 2차 시도: 작업 디렉터리가 달라도 소스 코드 위치를 기준으로 data 파일을 찾아본다.
     * 제출 환경에서 실행 위치가 달라졌을 때를 대비한 보조 처리이다.
     */
    if (build_source_data_path(path, sizeof(path), filename)) {
        fp = fopen(path, mode);
        if (fp != NULL) {
            return fp;
        }
    }

    return NULL;
}
