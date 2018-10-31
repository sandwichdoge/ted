#include "linked-list.h"
#include "fileops/fileops.h"
//#include "str-utils/str-utils.h"

#define HLINES 80
#define VLINES 24
#define LF_FLAG HLINES + 1
#define DECREMENT(x) (x - 1 < 0 ? 0 : x - 1)
#define INCREMENT(x, max) (x > max ? max : x + 1) //increment value with max threshold

typedef node_t line_t;

char** generate_terminal_friendly_array(char **arr, int sz, int *new_size, int max_len);
int generate_terminal_friendly_list(char **arr, int sz, line_t *head, int *new_size, int max_len);
int list_write_to_file(line_t *head, char *path, int flg_pos);
void scr_clear();
line_t* scr_out(line_t *head, int how_many);
int print_line(char *data, int L);
void update_pos(int pos[]);
line_t* list_rewind(line_t *head, int how_many);
int is_alpha(int c);