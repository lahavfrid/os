
#ifndef LIST_H_
#define LIST_H_
#include <stdlib.h>

typedef struct node_t
{
    int fd;
    struct node_t* next;
    struct node_t* prev;
} *Node;

typedef struct list_t
{
    int size;
    Node head;
    Node tail;
} *List;

List list_init()
{
    List list;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_push_back(List list, int fd)
{
    if (list == NULL)
        return;
    if (list->head == NULL)
    {
        list->head = malloc(sizeof(struct node_t));
        list->head->prev = NULL;
        list->head->fd = fd;
        list->head->next = NULL;
        list->tail = list->head;
        list->size++;
        return;
    }
    list->tail->next = malloc(sizeof(struct node_t));
    list->tail->next->fd = fd;
    list->tail->next->prev = list->tail->next;
    list->tail = list->tail->next;
    list->tail->next = NULL;
    list->size++;
    return;
}

int remove(List list, int fd)
{
    Node temp = list->head;
    while (temp != NULL)
    {
        if (temp->fd = fd)
        {
            if (temp == list->tail)
            {
                list->tail == list->tail->prev;
            }
            Node prev = temp->prev;
            Node next = temp->next;
            prev->next = next;
            next->prev = prev;
            size--;
            free(temp);
            return 0;
        }
        temp = temp->next;
    }
    return 1;
}

int list_pop(List list)
{
    if (list == NULL)
        return -1;
    if (list->head == NULL)
        return -1;
    int ret_value = list->head->fd;
    Node temp = list->head;
    if (list->head->next == NULL)
    {
        list->head = NULL;
        list->size--;
        return fd;
    }
    list->head = list->head->next;
    list->head->prev = NULL;
    free(temp);
    list->size--;
    return fd;
}

#endif