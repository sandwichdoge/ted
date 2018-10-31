#include <stdio.h>
#include <stdlib.h>

/*Doubly linked list library*/

typedef struct node_t {
    char *str;
    struct node_t *prev;
    struct node_t *next;
} node_t;


/*add a node before head node*/
node_t* list_add_prev(node_t *head, char *str)
{
    if (head == NULL) return NULL;

    node_t *p = head->prev;

    node_t *new_node = (node_t*)malloc(sizeof(node_t));
    new_node->str = str;
    new_node->prev = p;
    new_node->next = head;

    head->prev = new_node;
    if (p) p->next = new_node;

    return new_node;
}


/*add a node after head node*/
node_t* list_add_next(node_t *head, char *str)
{
    if (head == NULL) return NULL;

    node_t *n = head->next;

    node_t *new_node = (node_t*)malloc(sizeof(node_t));
    new_node->str = str;
    new_node->prev = head;
    new_node->next = n;

    head->next = new_node;
    if (n) n->prev = new_node;

    return new_node;
}


/*add a node to the very front of list*/
node_t *list_add_front(node_t *head, char *str)
{
    if (head == NULL) return NULL;

    node_t *new_node = (node_t*)malloc(sizeof(node_t));
    new_node->str = str;

    while (head->prev != NULL) {
        head = head->prev;
    }

    new_node->prev = NULL;
    new_node->next = head;

    head->prev = new_node;

    return new_node;
}


/*add a node to the very end of list*/
node_t *list_add_back(node_t *head, char *str)
{
    if (head == NULL) return NULL;

    node_t *new_node = (node_t*)malloc(sizeof(node_t));
    new_node->str = str;

    while (head->next != NULL) {
        head = head->next;
    }

    new_node->prev = head;
    new_node->next = NULL;

    head->next = new_node;

    return new_node;
}


void print_list(node_t *head)
{
    while (head != NULL) {
        printf(">%s[%d]\n", head->str, head->str[81]);
        head = head->next;
    }
}