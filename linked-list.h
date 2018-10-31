#include <stdio.h>
#include <stdlib.h>
#ifndef _linked_list_h
#define _linked_list_h
/*Doubly linked list library*/

typedef struct node_t {
    char *str;
    struct node_t *prev;
    struct node_t *next;
} node_t;

node_t* list_add_prev(node_t *head, char *str);
node_t* list_add_next(node_t *head, char *str);
node_t *list_add_front(node_t *head, char *str);
node_t *list_add_back(node_t *head, char *str);
node_t *list_traverse(node_t *head, int direction, int n);
void print_list(node_t *head);

#endif