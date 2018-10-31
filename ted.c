#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileops.h" //file_read_to_array()
#include "linked-list.h"

#define HLINES 80
#define VLINES 24
#define LF_FLAG HLINES + 1

typedef node_t line_t;


char** generate_terminal_friendly_array(char **arr, int sz, int *new_size, int max_len);
int generate_terminal_friendly_list(char **arr, int sz, line_t *head, int *new_size, int max_len);
int list_write_to_file(line_t *head, char *path, int flg_pos);

int main()
{
    char *file = "sample.txt";

    int line_count = 0;
    char **lines = file_read_to_array(file, &line_count);

    line_t head; head.prev = NULL; head.next = NULL; //Initialize head node

    int new_sz;
    generate_terminal_friendly_list(lines, line_count, &head, &new_sz, HLINES);
    line_t *first_line = head.next; /*advance and discard head since it's garbage*/
    //print_list(first_line);
    list_write_to_file(first_line, "sample2.txt", HLINES + 1);

    free_str_array(lines, line_count);

    return 0;
}


/*(maxlen+2)th byte [or 0-based index: maxlen+1] will represent if it's a real LF (\n) or a generated line (broken down for viewing)
 *0: fake LF, 1: real LF
 *All lines are maxlen + 2 bytes long (NULLTERM then LFflag)
 *e.g.: abcd\01 = strlen = 4bytes, mem len = 5bytes, with flag = 6bytes
 *Function returns pointer to new array and the size of it via new_sz
 *Caller is responsible for freeing returned data
 */
int generate_terminal_friendly_list(char **arr, int sz, line_t *head, int *new_sz, int max_len)
{
    char **ret = malloc(1);
    int len;
    int cur;
    int formatted_len;
    *new_sz = 0;

    for (int i = 0; i < sz; i++) {
        len = strlen(arr[i]);
        cur = 0;
        while (len >= 0) {
            /*Expand the new array*/
            char *str = calloc(max_len + 2, 1);
            head = list_add_next(head, str);

            /*Give it a nice linebreak formatting (avoid breaking a word in half)*/
            if ((arr[i] + cur)[max_len - 1] != '\0') { //if line is too long
                for (formatted_len = max_len; formatted_len > 0; formatted_len--) {
                    if ((arr[i] + cur)[formatted_len] == ' ') break;
                }
                if (formatted_len == 0) formatted_len = max_len;
                else formatted_len++;
            }
            
            /*Put the divided line to new array position*/
            strncpy(str, arr[i] + cur, formatted_len);

            /*Advance current position*/
            cur += formatted_len;
            len -= formatted_len;
            (*new_sz) += 1;
        }

        head->str[max_len+1] = 1;

    }

    return 0;
}



/*This is a customized version
 *It will check the flg of line to see if the linebreak is real or was generated for viewing
 *flg_pos is always the last element of line, right after NULL TERM
 */
int list_write_to_file(line_t *head, char *path, int flg_pos)
{
    FILE *fd = fopen(path, "w");
    if (fd == NULL) return -1;

    char *buf = calloc(flg_pos, 1);
    int lineno = 0;
    int len;

    /*print a line, then print LF*/
    while (head) {
        lineno++;
        strncpy(buf, head->str, flg_pos-1);
        len = strlen(buf);

        /*only write LF for lines with real LF*/
        if (head->str[flg_pos] == 1) {
            buf[len] = '\n';
            len++;
        }

        fwrite(buf, len, 1, fd);
        head = head->next;
    }

    free(buf);
    fclose(fd);
    
    return 0;
}