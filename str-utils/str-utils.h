#ifndef _str_utils_h
#define _str_utils_h
#include <stdlib.h>
#include <string.h>

char **str_split(char *stringf, const char *delim, int *refOcc);
int free_str_array(char **arr, int sz);
int str_count(char* string, const char* substr);
void char_insert(char *str, int pos, char c);
void str_insert(char *str, int pos, char *substr);
int str_remove(char *str, int pos, int len);

#endif