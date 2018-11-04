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
    if (line_count < 0) {
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
    int scrpos[2] = {0, 0}; //Current pos of cursor
    int mempos[2] = {0, 0}; //Current pos of mem
    line_t *cur_line = doc_begin;
    int cur_lineno = 0;

    /*Declare flags*/
    int tab_removed = 0;

    /*Initialize screen mode*/
    initscr();
    noecho();
    keypad(stdscr, 1);
    raw();


    /*Draw control line*/
    init_control_line();


    /*Print first page of document to screen*/
    textw = newwin(VLINES, HLINES, 0, 0);
    next_page = scr_out(doc_begin, VLINES);
    move(0, 0);
    wrefresh(textw);

    
    char buf[16];
    /*Handle keypresses*/
    while (1) {
        int k = getch();
        update_pos(scrpos);
        cur_line = list_traverse(cur_page, 1, scrpos[0]); //Traverse forward until lineno is met.
        update_mempos(mempos, scrpos, cur_line);
        sprintf(buf, "%d", mempos[1]);
        strcat(buf, ":mempos");
        print_control_line(buf);

        /*FUNCTION KEYS*/
        if (k == KEY_F(2)) break;

        switch (k) {
            /*NAVIGATION KEYS*/
            case KEY_DOWN:
                if (scrpos[0] == VLINES - 1) { //If cursor is at last line, advance page
                    cur_page = next_page;
                    next_page = scr_out(next_page, VLINES);
                    move(0, scrpos[1]);
                }
                else if (cur_lineno < new_sz) {
                    move(scrpos[0] + 1, scrpos[1]);
                }
                cur_lineno = INCREMENT(cur_lineno, new_sz);
                break;
            case KEY_UP:
                if (scrpos[0] == 0 && cur_page != doc_begin) { //If cursor is at line 0 && not at begin of doc, rewind page
                    cur_page = list_rewind(cur_page, VLINES);
                    next_page = scr_out(cur_page, VLINES);
                    move(VLINES - 1, scrpos[1]);
                }
                else {
                    move(scrpos[0] - 1, scrpos[1]);
                }
                cur_lineno = DECREMENT(cur_lineno);
                break;
            case KEY_LEFT: //TODO: handle left keypress
                if (scrpos[1] == 0) { //Left key pressed at start of line
                    if (cur_lineno > 0) {
                        goto_endline(cur_line->prev, scrpos[0] - 1, HLINES); //Set cursor to end of previous line
                        cur_lineno = DECREMENT(cur_lineno);
                    }
                    else move(0, 0); //If start of doc then do nothing
                }
                else if (scrpos[1] > scr_len(cur_line)) { //Cursor is outside of line bound
                    goto_endline(cur_line, scrpos[0], HLINES);
                }
                else if (cur_line->str[mempos[1] - 1] == '\t' || cur_line->str[mempos[1]] == '\t') { 
                    move(scrpos[0], conv_to_scrpos(mempos[1] - 1, cur_line)); //Go to TAB begin
                }
                else {
                    move(scrpos[0], scrpos[1] - 1);
                }
                break;
            case KEY_RIGHT: 
                if (scrpos[1] == scr_len(cur_line) || scrpos[1] == HLINES - 1) { //Right key pressed at end of line
                    move(scrpos[0] + 1, 0);
                    cur_lineno = INCREMENT(cur_lineno, new_sz);
                }
                else if (cur_line->str[mempos[1]] == '\t') { //handle TABs
                    move(scrpos[0], upperbound(scrpos[1] + 1, 8));
                }
                else move(scrpos[0], scrpos[1] + 1);
                break;
            case KEY_END:
                cur_line = list_traverse(cur_page, 1, scrpos[0]); //Traverse forward until lineno is met.
                goto_endline(cur_line, scrpos[0], HLINES);
                break;
            case KEY_HOME:
                move(scrpos[0], 0);
                break;

            /*Backspace*/
            case KEY_BACKSPACE:
            case 127:
                if (scrpos[1] > 0) {
                    if (cur_line->str[mempos[1] - 1] == '\t') tab_removed = 1;
                    else tab_removed = 0;    
                    line_pop(cur_line, mempos[1] - 1, 1, HLINES);
                    scr_out(cur_page, VLINES);

                    if (tab_removed == 1) move(mempos[0], conv_to_scrpos(mempos[1] - 1, cur_line));
                    else move(scrpos[0], scrpos[1] - 1);
                }
                else { //Backspace is pressed at start of line

                }
                break;


            /*Key combo*/
            case 19: //CTRL+S - Save file
                list_write_to_file(doc_begin, file, HLINES + 1);
                break;
        }

        //TODO: handle TAB insertion
        /*INPUT KEYS (a-z, A-Z, 0-9, etc. and non-special symbols)*/
        /*Insert a char at cursor position, scrpos[0] is the line no and scrpos[1] is the char no.*/
        /*Only supports ASCII characters for now*/
        if (is_alpha(k) || is_acceptable_ascii_symbols(k)) {
            cur_line = list_traverse(cur_page, 1, scrpos[0]); //Traverse forward until lineno is met.
            /*Insert string into cur_line*/
            if (scr_len(cur_line) < HLINES) { //If current line's length hasn't reached limit, insert typed key
                char_insert(cur_line->str, mempos[1], k);
                print_line(cur_line->str, scrpos[0]); //Display modified line on screen
                move(scrpos[0], scrpos[1]+1 > HLINES ? HLINES : scrpos[1] + 1);
            }
            else { //If line has reached len limit
                /*Push list by 1 char to make space at cursor pos*/
                line_push(cur_line, mempos[1], 1, HLINES);
                memcpy(cur_line->str + mempos[1], (char*)&k, 1);

                /*Print out page and move cursor appropriately*/
                next_page = scr_out(cur_page, VLINES);
                if (scrpos[1] + 1 >= HLINES) move(scrpos[0] + 1, 0);
                else move(scrpos[0], scrpos[1]+1);
            }
        }

    }

    free_str_array(lines, line_count);
    endwin();

    return 0;
}


/*Pop a few characters at pos*/
void line_pop(line_t *head, int pos, int n, const int max_len)
{
    int flg = 0;
    if (pos + n > max_len) {
        if (head->str[max_len + 1] != 1) {
            line_pop(head->next, 0, pos + n - max_len, max_len);
        }
        else return;
        line_pop(head, pos, max_len - pos, max_len);
    }
    else {
        char *first_word = calloc(n + 1, 1);
        if (head->next != NULL && head->str[max_len + 1] != 1 && strlen(head->next->str)) {
            memcpy(first_word, head->next->str, n);
            line_pop(head->next, 0, n, max_len);
        }
        if (pos + n > strlen(head->str)) n = strlen(head->str) - pos;
        str_remove(head->str, pos, n);

        strcat(head->str, first_word);
        if (strlen(head->str) == 0) {
            flg = head->str[max_len + 1]; //PRESERVE REAL LF
            if (head->prev != NULL) head->prev->str[max_len + 1] = flg;
            list_remove(head);
        }

        free(first_word);
    }
    
}


//TODO: take TAB into consideration
/*Make space at pos to insert string later*/
void line_push(line_t *head, int pos, int n, const int max_len)
{
    char *str = NULL, *newline = NULL;
    char *last_word = calloc(n + 1, 1);
    int len = scr_len(head);
    int flg = 0;

    if (len + n >= max_len) {
        memcpy(last_word, head->str + (strlen(head->str) - n), n);
        head->str[strlen(head->str) - n] = '\0'; //head->str is trimmed now
        if (head->str[max_len + 1] == 1) flg = 1; //PRESERVE REAL LINEBREAK
        head->str[max_len + 1] = 0; //Mark current line as fake because it's been pushed to next line

        if (flg == 1) { //If fake line, proceed to line_push() normally, otherwise make a new real line
            newline = calloc(max_len + 1, 1);
            list_add_next(head, newline); //Real linebreak, therefore make a new line
            head->next->str[max_len + 1] = 1; //PRESERVE REAL LINEBREAK
        }

        line_push(head->next, 0, strlen(last_word), max_len);
        memcpy(head->next->str, last_word, strlen(last_word));
    }

    /*Push tail by n chars*/
    char *tail = malloc(strlen(&head->str[pos]) + 1);
    strcpy(tail, &head->str[pos]);
    memcpy((&head->str[pos + n]), tail, strlen(tail));

    free(tail); tail = NULL;
    free(last_word); last_word = NULL;
}


//TODO: take TAB into consideration
/*(maxlen+2)th byte [or 0-based index: maxlen+1] will represent if it's a real LF (\n) or a generated line (broken down for viewing)
 *0: fake LF, 1: real LF
 *All lines are maxlen + 2 bytes long (NULLTERM then LFflag)
 *e.g.: abcd\01 = strlen = 4bytes, mem len = 5bytes, with flag = 6bytes
 *Function returns pointer to new array and the size of it via new_sz
 *Caller is responsible for freeing returned data
 */
int generate_terminal_friendly_list(char **arr, int sz, line_t *head, int *new_sz, int max_len)
{
    if (arr == NULL) {
        char *str = calloc(max_len + 1, 1);
        head = list_add_next(head, str);
    }
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
            if (strlen(arr[i] + cur) > max_len) { //if line is too long
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
void scr_clear(WINDOW *win)
{
    werase(win);
    wrefresh(win);
}


/*Print a specific number of lines to the screen, starting from head*/
line_t* scr_out(line_t *head, int how_many)
{
    scr_clear(textw);
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


void update_pos(int scrpos[])
{
    scrpos[0] = getcury(stdscr);
    scrpos[1] = getcurx(stdscr);
}


line_t *list_rewind(line_t *head, int how_many)
{
    while (how_many-- && head->prev != NULL) {
        head = head->prev;
    }

    return head;
}


void goto_endline(line_t *line, int y, int max_len)
{
    int len = scr_len(line);
    move(y, len >= max_len ? max_len - 1 : len);
}


void print_control_line(char *str)
{
    wmove(menuw, 0, 1);
    wclrtoeol(menuw);
    mvwprintw(menuw, 0, 1, str);
    wrefresh(menuw);

}


void reset_control_line()
{
    wattron(menuw, A_STANDOUT);
    mvwprintw(menuw, 0, 1, "CTRL+S:Save");
    wattroff(menuw, A_STANDOUT);
    mvwprintw(menuw, 0, 1 + strlen("CTRL+S:Save"), "    ");
    wattron(menuw, A_STANDOUT);
    mvwprintw(menuw, 0, 1 + strlen("CTRL+S:Save    "), "F2:Quit");
    wattroff(menuw, A_STANDOUT);
    wrefresh(menuw);
}


void init_control_line()
{
    menuw = newwin(1, HLINES, VLINES, 0);
    box(menuw, 0, 0);
    reset_control_line();
}


/*Ceiling n to the nearest upper multiplication of div*/
int upperbound(int n, int divi)
{
    if (n % divi == 0) return n;
    int res = n / divi;
    return divi * (res+1);
}


/*Convert scrpos to mempos. Handle special characters that take up more than 1 space*/
/*a____bc scr-input:5, mem-output:1*/
void update_mempos(int mempos[], int scrpos[], line_t *cur_line)
{
    mempos[0] = scrpos[0]; //y coord
    mempos[1] = 0;
    int scr = 0;
    for (int i = 0; cur_line->str[i]; i++) { //this part could look prettier but this expression is more logically sound
        if (cur_line->str[i] == '\t') {
            scr = upperbound(scr + 1, 8);
        }
        else {
            scr++;
        }
        if (scr > scrpos[1]) break;
        mempos[1]++;
    }

}


/*Get number of chars needed to print the line to screen, to handle TABs*/
int scr_len(line_t *line)
{
    int ret = 0;
   for (int i = 0; line->str[i]; i++) {
        if (line->str[i] == '\t') ret = upperbound(ret + 1, 8);
        else ret++;
    }

    return ret;
}


int conv_to_scrpos(int memposx, line_t *line)
{
    int ret = 0;
    for (int i = 0; i < memposx; i++) {
        if (line->str[i] == '\t') ret = upperbound(ret + 1, 8);
        else ret++;
    }
    /*
    char buf[16];
    sprintf(buf, "%d", ret);
    print_control_line(buf);*/
    return ret;
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