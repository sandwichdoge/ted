#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ted.h"


int main(int argc, char *argv[])
{
    if (argc < 2) return -1;
    char *file = argv[1];

    int line_count = 0;
    char **lines = file_read_to_array(file, &line_count);
    if (lines == NULL) {
        printf("Cannot open file %s.\n", file);
        return -1;
    }

    line_t head; head.prev = NULL; head.next = NULL; //Declare and initialize head node

    /*Generate a list of terminal-friendly lines for viewing*/
    int new_sz;
    generate_terminal_friendly_list(lines, line_count, &head, &new_sz, HLINES);
    line_t *doc_begin = head.next; /*Advance and discard head because it's garbage*/
    doc_begin->prev = NULL;

    /*These variables must be kept track of at all times*/
    line_t *next_page; //Next unprinted line in list
    line_t *cur_page = doc_begin; //First line shown on the screen
    int pos[2] = {0, 0}; //Current pos of cursor
    line_t *cur_line = doc_begin;
    int cur_lineno = 0;

    /*Initialize screen mode*/
    initscr();
    noecho();
    keypad(stdscr, 1);
    raw();

    /*Print first page of document to screen*/
    next_page = scr_out(doc_begin, VLINES);
    move(0, 0);
    refresh();

    /*Handle keypresses*/
    while (1) {
        int k = getch();
        update_pos(pos);

        /*FUNCTION KEYS*/
        if (k == KEY_F(2)) break;

        switch (k) {
            /*NAVIGATION KEYS*/
            case KEY_DOWN:
                if (pos[0] == VLINES - 1) {
                    cur_page = next_page;
                    next_page = scr_out(next_page, VLINES);
                    move(0, pos[1]);
                }
                else if (cur_lineno < new_sz) {
                    move(pos[0] + 1, pos[1]);
                }
                cur_lineno = INCREMENT(cur_lineno, new_sz);
                break;
            case KEY_UP:
                if (pos[0] == 0 && cur_page != doc_begin) {
                    cur_page = list_rewind(cur_page, VLINES);
                    next_page = scr_out(cur_page, VLINES);
                    move(VLINES - 1, pos[1]);
                }
                else {
                    move(pos[0] - 1, pos[1]);
                }
                cur_lineno = DECREMENT(cur_lineno);
                break;
            case KEY_LEFT:
                if (pos[1] == 0) { //Left key pressed at start of line
                    move(cur_lineno > 0 ? pos[0] - 1 : pos[0], cur_lineno > 0 ? HLINES - 1 : pos[1]); //Do nothing if it's 1st line of doc
                    cur_lineno = DECREMENT(cur_lineno);
                }
                else {
                    move(pos[0], pos[1] - 1);
                }
                break;
            case KEY_RIGHT: //Right key pressed at end of line
                if (pos[1] == HLINES - 1) {
                    move(pos[0] + 1, 0);
                    cur_lineno = INCREMENT(cur_lineno, new_sz);
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

            /*Backspace*/
            case 127:
                cur_line = list_traverse(cur_page, 1, pos[0]); //Traverse forward until lineno is met.
                str_remove(cur_line->str, pos[1]-1, 1);
                print_line(cur_line->str, pos[0]);
                move(pos[0], pos[1] - 1);
                break;

            /*Key combo*/
            case 19: //CTRL+S - Save file
                list_write_to_file(doc_begin, file, HLINES + 1);
                break;
        }

        /*INPUT KEYS (a-z, A-Z, 0-9, etc. and non-special symbols)*/
        /*Insert a char at cursor position, pos[0] is the line no and pos[1] is the char no.*/
        /*Only supports ASCII characters for now*/
        if (is_alpha(k) || is_acceptable_ascii_symbols(k)) {
            cur_line = list_traverse(cur_page, 1, pos[0]); //Traverse forward until lineno is met.
            /*Insert string into cur_line*/
            if (strlen(cur_line->str) < HLINES) { //If current line's length hasnt reached limit, insert typed key
                char_insert(cur_line->str, pos[1], k);
                print_line(cur_line->str, pos[0]); //Display modified line on screen
                move(pos[0], pos[1]+1 > HLINES ? HLINES : pos[1] + 1);
            }
            else { //If line has reached len limit
                //Push list by 1 char to make space at cursor pos
                line_push(cur_line, pos[1], 1, HLINES);
                memcpy(cur_line->str + pos[1], (char*)&k, 1);
                next_page = scr_out(cur_page, VLINES);
                if (pos[1] + 1 >= HLINES) move(pos[0] + 1, 0);
                else move(pos[0], pos[1]+1);
            }
        }

    }

    free_str_array(lines, line_count);
    endwin();

    return 0;
}


/*Append last char of current line to the front of next line, remove that char from cur line, insert char at cursor pos*/
void line_push(line_t *head, int pos, int n, const int max_len)
{
    char *str = NULL;
    char *last_word = calloc(n + 1, 1);
    int len = strlen(head->str);

    //printf("\nPUSHING:[pos:%d][n:%d][len:%d]\n%s\n", pos, n, len, head->str);

    if (len + n >= max_len) {
        memcpy(last_word, head->str + max_len - n, n);
        head->str[max_len - n] = '\0'; //head->str is trimmed now
        //printf("Last word:[%s]\nCurrent len:[%d]\n", last_word, strlen(head->str));
        line_push(head->next, 0, strlen(last_word), max_len);
        memcpy(head->next->str, last_word, strlen(last_word));
    }

    char *tail = malloc(strlen(&head->str[pos]) + 1); //Next: push tail by n
    strcpy(tail, &head->str[pos]);
    memcpy((&head->str[pos + n]), tail, strlen(tail));
    //print_raw_mem(&head->str[pos], 40);

    //PROBLEM here, world should not be at pos 11, 6 instead
    //printf("TAIL:Copied [%s] into [%s] at pos [%d], got [%s]\n", tail, (&head->str[pos]), n, head->str);

    free(tail);
    tail = NULL;
    free(last_word);
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


/*print a line at line number L
 *line is the data in buffer
 *L is terminal line no.*/
int print_line(char *data, int L)
{
    move(L, 0);
    clrtoeol(); //clear current line
    addstr(data);
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


/*Return 1 if c is alphabet*/
int is_alpha(int c)
{
    return (c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || c >= 'A' && c <= 'Z');
}

int is_acceptable_ascii_symbols(int c)
{
    static int ascii_symbols[] = {' ', '_', '-', '?', '.', ',', ';', ':', '+', '*', '/', '\\', '&', '%', '$', '#', '@', '!', '=', '[', ']', '{', '}', '<', '>', '`', '~', '|', '"', '\''};
    for (int i = 0; i < sizeof(ascii_symbols); i++) {
        if (c == ascii_symbols[i]) return 1;
    }
    return 0;
}