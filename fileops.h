#ifndef _file_ops_h
#define _file_ops_h
#include <stdio.h>
#include <stdlib.h>
#include "str-utils/str-utils.h"


size_t file_get_size(char *path);
char **file_read_to_array(char *path, int *line_count);


size_t file_get_size(char *path)
{
    FILE *fd = fopen(path, "r");
    if (fd == NULL) return -1;
    fseek(fd, 0L, SEEK_END);
    size_t ret = ftell(fd);
    fclose(fd);
    return ret;
}


char **file_read_to_array(char *path, int *line_count)
{
    size_t sz = file_get_size(path);
    FILE *fd = fopen(path, "r");
    if (fd == NULL) return NULL;
    
    char* buf = (char*)malloc(sz);
    fread(buf, sz, 1, fd);

    char **lines = str_split(buf, "\n", line_count);

    free(buf);

    return lines;
}


#endif