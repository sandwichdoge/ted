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
void scr_clear();
line_t* scr_out(line_t *head, int how_many);
int print_line(char *data, int L);
void update_pos(int pos[]);
line_t* list_rewind(line_t *head, int how_many);


int main(int argc, char *argv[])
{

    if (argc < 2) return -1;
    char *file = argv[1];


    int line_count = 0;
    char **lines = file_read_to_array(file, &line_count);
    if (lines == NULL) {
        printf("Cannot open file.\n");
        return -1;
    }

    line_t head; head.prev = NULL; head.next = NULL; //Declare and initialize head node

    /*Generate a list of terminal-friendly lines for viewing*/
    int new_sz;
    generate_terminal_friendly_list(lines, line_count, &head, &new_sz, HLINES);
    line_t *first_line = head.next; /*Advance and discard head because it's garbage*/
    first_line->prev = NULL;

    /*These variables must be kept track of all times*/
    line_t *next_page; //Next unprinted line in list
    line_t *cur_page = first_line;
    int pos[2] = {0, 0}; //Current pos of cursor
    
    /*Initialize screen mode*/
    initscr();
    noecho();
    keypad(stdscr, 1);
    raw();

    /*Print first page of document to screen*/
    next_page = scr_out(first_line, VLINES);
    move(0, 0);
    refresh();

    /*Handle keypresses*/
    while (1) {
        int k = getch();
        update_pos(pos);

        if (k == KEY_F(2)) break;
        switch (k) {
            case KEY_DOWN:
                if (pos[0] == VLINES - 1) {
                    cur_page = next_page;
                    next_page = scr_out(next_page, VLINES);
                    move(0, pos[1]);
                }
                else {
                    move(pos[0] + 1, pos[1]);
                }
                break;
            case KEY_UP:
                if (pos[0] == 0 && cur_page != first_line) {
                    cur_page = list_rewind(cur_page, VLINES);
                    next_page = scr_out(cur_page, VLINES);
                    move(VLINES - 1, pos[1]);
                }
                else {
                    move(pos[0] - 1, pos[1]);
                }
                break;
            case KEY_LEFT:
                if (pos[1] == 0) {
                    move(pos[0] - 1, HLINES > 0 ? HLINES - 1 : HLINES);
                }
                else {
                    move(pos[0], pos[1] - 1);
                }
                break;
            case KEY_RIGHT:
                if (pos[1] == HLINES - 1) {
                    move(pos[0] + 1, 0);
                }
                else {
                    move(pos[0], pos[1] + 1);
                }
                break;
            case KEY_END:
                move(pos[0], HLINES - 1);
                break;
            case KEY_HOME:
                move(pos[0], 0);
                break;
        }
    }

    //list_write_to_file(first_line, "sample2.txt", HLINES + 1);

    free_str_array(lines, line_count);
    endwin();

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
    int len = 0;
    int cur = 0;
    int formatted_len = 0;
    *new_sz = 0;

    for (int i = 0; i < sz; i++) {
        len = strlen(arr[i]);
        cur = 0;
        while (len >= 0) {
            /*Expand the new array*/
            char *str = calloc(max_len + 2, 1);
            head = list_add_next(head, str);
            
            /*Give it a nice linebreak formatting (avoid breaking a word in half)*/
            formatted_len = strlen(arr[i] + cur);
            if ((arr[i] + cur)[max_len - 1] != '\0') { //if line is too long
                for (formatted_len = max_len; formatted_len > 0; formatted_len--) {
                    if ((arr[i] + cur)[formatted_len] == ' ') break;
                }

                if (formatted_len == 0) formatted_len = max_len; //If SPACE doesn't exist in line
                else formatted_len++; //Print space at end of line
            }
            
            /*Put the divided line to new array position*/
            strncpy(str, arr[i] + cur, formatted_len);

            /*Advance current position*/
            cur += formatted_len;
            len -= formatted_len;
            (*new_sz) += 1;
            if (len == 0) break;
        }

        head->str[max_len+1] = 1; //Natural end of line
    }

    head->str[max_len+1] = 0; //Do not add LF to last line

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


/*Clear the screen*/
void scr_clear()
{
    erase();
    refresh();
}


/*Print a specific number of lines to the screen, starting from head*/
line_t* scr_out(line_t *head, int how_many)
{
    scr_clear();
    int l = 0; //line no
    while (l < how_many) {
        print_line(head->str, l);
        if (head->next == NULL) break;
        head = head->next;
        l++;
    }

    return head;
}


//print a line at line number L
//line is the data in buffer
//L is terminal line no.
int print_line(char *data, int L)
{
    mvaddstr(L, 0, data);
    return 0;
}


void update_pos(int pos[])
{
    pos[0] = getcury(stdscr);
    pos[1] = getcurx(stdscr);
}


line_t *list_rewind(line_t *head, int how_many)
{
    while (how_many-- && head->prev != NULL) {
        head = head->prev;
    }

    return head;
}