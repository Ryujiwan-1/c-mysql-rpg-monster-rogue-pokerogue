#include "path_utils.h"

#include <stdio.h>
#include <string.h>

static int build_source_data_path(char *out, size_t out_size, const char *filename)
{
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

    snprintf(path, sizeof(path), "data/%s", filename);
    fp = fopen(path, mode);
    if (fp != NULL) {
        return fp;
    }

    if (build_source_data_path(path, sizeof(path), filename)) {
        fp = fopen(path, mode);
        if (fp != NULL) {
            return fp;
        }
    }

    return NULL;
}
