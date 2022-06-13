
#ifndef LIST_H_
#define LIST_H_
#include <stdlib.h>
#include <sys/time.h>

typedef struct node_t
{
    int fd;
    suseconds_t creation_time;
    suseconds_t working_time;
    struct node_t* next;
    struct node_t* prev;
} *Node;

typedef struct list_t
{
    int size;
    int max_size;
    Node head;
    Node tail;
} *List;

List list_init(int given_max_size)
{
    List list = (List)malloc(sizeof(struct list_t));
    list->size = 0;
    list->max_size = given_max_size;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_push_back(List list, int fd, suseconds_t creation)
{
    if (list == NULL)
        return;

    struct timeval current;
    suseconds_t working = 0;
    gettimeofday(&current, NULL);
    if(creation == 0)
        creation = current.tv_usec;
    else
        working = current.tv_usec;
    
    if (list->head == NULL)
    {
        list->head = malloc(sizeof(struct node_t));
        list->head->prev = NULL;
        list->head->fd = fd;
        list->head->creation_time = creation;
        list->head->working_time = working;
        list->head->next = NULL;
        list->tail = list->head;
        list->size++;
        return;
    }
    list->tail->next = malloc(sizeof(struct node_t));
    list->tail->next->fd = fd;
    list->head->creation_time = creation;
    list->head->working_time = working;
    list->tail->next->prev = list->tail->next;
    list->tail = list->tail->next;
    list->tail->next = NULL;
    list->size++;
    return;
}

int list_remove(List list, int fd)
{
    if(list==NULL || list->head==NULL)
        return 0;
    if(list->head->fd == fd)
    {
        list_pop(list, NULL);
        return 1;
    }

    Node temp = list->head;
    while (temp != NULL)
    {
        if (temp->fd == fd)
        {
            Node prev = temp->prev;
            Node next = temp->next;
            if (temp == list->tail)
                list->tail = list->tail->prev;
            else
                next->prev = prev;
            prev->next = next;
            list->size--;
            free(temp);
            return 0;
        }
        temp = temp->next;
    }
    return 1;
}

int list_pop(List list, suseconds_t* creation)
{
    if (list == NULL)
        return -1;
    if (list->head == NULL)
        return -1;
    int ret_value = list->head->fd;
    if(creation != NULL)
        *creation = list->head->creation_time;
    
    Node temp = list->head;
    if (list->head->next == NULL)
    {
        list->head = NULL;
        list->size--;
        return ret_value;
    }
    list->head = list->head->next;
    list->head->prev = NULL;
    free(temp);
    list->size--;
    return ret_value;
}

void list_random_delete(List list)
{
    int drop_amount = list->size * 7 / 10;
    drop_amount = list->size - drop_amount; //30% of list->size, rounded up
    int step_range = list->size / drop_amount;

    time_t t;
    srand((unsigned) time(&t)); //set random seed

    int step = rand() % step_range;
    while(step == 0 && drop_amount > 0 && list->size > 0)   //randomly remove number of first nodes
    {
        list_pop(list, NULL);
        drop_amount--;
        step = rand() % step_range;
    }

    if(list->size == 0)
        return;
    Node list_iterator = list->head;
    Node prev;
    Node next;
    for(int i = 0; i < drop_amount; i++)
    {
        for(int j = 0; j < step && list_iterator != NULL; j++)  //iterate random number of steps on the queue
        {
            list_iterator = list_iterator->next;
        }

        if (list_iterator == list->tail)
        {
            break;
        }
        prev = list_iterator->prev;
        next = list_iterator->next;
        prev->next = next;
        next->prev = prev;
        list->size--;
        free(list_iterator);

        list_iterator = prev;

        step = rand() % step_range;
    }

    while(list->size > 0 && drop_amount > 0)
    {
        list->tail = list->tail->prev;
        prev = list_iterator->prev;
        prev->next = NULL;
        list->size--;
        free(list_iterator);

        list_iterator = prev;
        drop_amount--;
    }
    return;
}

#endif