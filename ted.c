#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileops.h" //file_read_to_array()

#define HLINES 80
#define VLINES 24
#define LF_FLAG HLINES + 1

char** generate_terminal_friendly_array(char **arr, int sz, int *new_size, int max_len);



int main()
{
    char *file = "sample.txt";

    int line_count = 0;
    char **lines = file_read_to_array(file, &line_count);
    
    int new_sz = 0;
    char **balanced_lines = generate_terminal_friendly_array(lines, line_count, &new_sz, HLINES);
    
    printf("%d\n", new_sz);

    for (int i = 0; i < new_sz; i++) {
        printf("%d.%s[%d]\n", i, balanced_lines[i], balanced_lines[i][LF_FLAG]);
    }


    free_str_array(lines, line_count);
    free_str_array(balanced_lines, new_sz);

    return 0;
}


/*(maxlen+2)th byte [or 0-based index: maxlen+1] will represent if it's a real LF (\n) or a generated line (broken down for viewing)
 *0: fake LF, 1: real LF
 *All lines are maxlen + 2 bytes long (NULLTERM then LFflag)
 *e.g.: abcd\01 = strlen = 4bytes, mem len = 5bytes, with flag = 6bytes
 *Function returns pointer to new array and the size of it via new_sz
 *Caller is responsible for freeing returned data
 */
char** generate_terminal_friendly_array(char **arr, int sz, int *new_sz, int max_len)
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
            ret = realloc(ret, ((*new_sz)+1) * sizeof(char*));
            ret[*new_sz] = (char*)calloc(max_len + 2, 1); //+2 for NULLTERM and trailing LF flag

            /*Give it a nice linebreak formatting (avoid breaking a word in half)*/
            if ((arr[i] + cur)[max_len - 1] != '\0') { //if line is too long
                for (formatted_len = max_len; formatted_len > 0; formatted_len--) {
                    if ((arr[i] + cur)[formatted_len] == ' ') break;
                }
                if (formatted_len == 0) formatted_len = max_len;
                else formatted_len++;
            }
            
            /*Put the divided line to new array position*/
            strncpy(ret[*new_sz], arr[i] + cur, formatted_len);

            /*Advance current position*/
            cur += formatted_len;
            len -= formatted_len;
            (*new_sz) += 1;
        }
        ret[(*new_sz)-1][max_len+1] = 1; //real LF (\n)

    }

    return ret;
}


/*This is a customized version
 *It will check the flg of line to see if the linebreak is real or was generated for viewing
 *flg_pos is always the last element of line, right after NULL TERM
 */
int array_write_to_file(const char *path, char **arr, int lines, int flg_pos)
{
    FILE *fd = fopen(path, "w");
    if (fd == NULL) return -1;

    char *buf = malloc(flg_pos);

    for (int i = 0; i < lines; i++) {
        
    }

}