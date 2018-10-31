#ifndef _file_ops_h
#define _file_ops_h
#include <stdio.h>
#include <stdlib.h>
#include "../str-utils/str-utils.h"
#include "../linked-list.h"

typedef node_t line_t;

size_t file_get_size(char *path);
char **file_read_to_array(char *path, int *line_count);
int list_write_to_file(line_t *head, char *path, int flg_pos);


#endif