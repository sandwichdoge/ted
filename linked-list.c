#include "linked-list.h"

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


/*Traverse a list n times.
direction: 0=backwards, 1=forward*/
node_t *list_traverse(node_t *head, int direction, int n)
{
    if (direction == 0) { //Traverse backwards
        while (n-- && head->prev != NULL) {
            head = head->prev;
        }
    }
    else { //Traverse foward
        while (n-- && head->next != NULL) {
            head = head->next;
        }
    }
    
    return head;
}


void list_remove(node_t *head)
{
    node_t *nxt = head->next;
    if (head->prev) head->prev->next = nxt;
    if (nxt) nxt->prev = head->prev;
}


void print_list(node_t *head)
{
    while (head != NULL) {
        printf(">%s[%d]\n", head->str, head->str[81]);
        //printf(">%s\n", head->str);
        head = head->next;
    }
}