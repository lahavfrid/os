#include "segel.h"
#include "request.h"
#include "list.h"
#include <assert.h>

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

pthread_mutex_t mutex;
pthread_cond_t available_cond;

// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[], int* thread_num, int* queue_size, char* policy)
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    policy = malloc(sizeof(argv[4]));
    strcpy(policy, argv[4]);
}


void* worker_thread(void* arg)
{
    int current_request = -1;

    //printf("enter worker\n");
    List* list_array = (List*)arg;
    List input_queue = (List)list_array[0];
    List threads_queue = (List)list_array[1];

    int id = list_pop(input_queue, NULL);   //getting the thread id
    
    struct timeval creation;    //stats declaration
    struct timeval working;
    struct Satistics stats;
    stats.id = id;
    stats.total_requests = 0;
    stats.static_requests = 0;
    stats.dynamic_requests = 0;

    pthread_mutex_lock(&mutex);
    while(1)    //let the worker threads continue working
    {
        //printf("enter while %d\n", current_request);
        while(current_request == -1)
        {
            //printf("enter current request\n");
            pthread_cond_wait(&available_cond, &mutex);
            //printf("Got to cond_wait\n");
            assert(input_queue != NULL);

            current_request = list_pop(input_queue, &creation);

            gettimeofday(&working, NULL);
            list_push_back(threads_queue, current_request);
        }
        pthread_mutex_unlock(&mutex);

        stats.total_requests++;

        requestHandle(current_request, &creation, &working, &stats); //added creation time, working time, stats
        Close(current_request);

        pthread_mutex_lock(&mutex);
        //printf("Got to loop's end\n");
        list_remove(threads_queue, current_request);    //maybe need to remove by getting the node earlier, and not searching in the queue
        current_request = -1;

        if(input_queue->max_size > input_queue->size)
            pthread_cond_signal(&available_cond);
    }
    pthread_mutex_unlock(&mutex);   //oi vey, shouldn't reach here
}

int main(int argc, char *argv[])
{
    //printf("sanity\n");
    pthread_mutex_init(&mutex, NULL);   //is this needed?
    if(pthread_cond_init(&available_cond, NULL) != 0)
    {
        perror("Condition Variable initializtion failed");
        return -1;  //may have better error signal
    }

    int listenfd, connfd, port, clientlen, thread_num, queue_max_size;
    struct sockaddr_in clientaddr;
    char* policy;

    getargs(&port, argc, argv, &thread_num, &queue_max_size, policy);
    //printf("port: %d\n", port);

    //printf("before list creation\n");
    List input_queue = list_init(queue_max_size);
    List threads_queue = list_init(queue_max_size);
    List list_array[2];
    list_array[0] = input_queue;
    list_array[1] = threads_queue;

    //printf("created list\n");
    pthread_t* thread_array[thread_num];
    //printf("created thread array\n");
    for(int i=0; i<thread_num; i++)
    {
        list_push_back(input_queue, i);   //giving id to each thread
        //printf("for %d iteration\n", i);
        thread_array[i] = (pthread_t)malloc(sizeof(pthread_t));
        pthread_create(thread_array[i], NULL, &worker_thread, list_array);
        assert(thread_array[i] != NULL);
        //printf("created %d thread\n", i);
    }
    //printf("after threads creation\n");

    
    listenfd = Open_listenfd(port);
    //printf("listenfd: %d\n", listenfd);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        //printf("connfd: %d\n", connfd);

        pthread_mutex_lock(&mutex);
        if(input_queue->size < queue_max_size){
            list_push_back(input_queue, connfd);
        }
        else
        {
            if(strcmp(policy, "drop_tail") == 0)
            {
                Close(connfd);
            }
            else if(strcmp(policy, "drop_random") == 0)
            {
                list_random_delete(input_queue);
                list_push_back(input_queue, connfd);
            }
            else if(strcmp(policy, "drop_head") == 0)
            {
                Close(list_pop(input_queue, NULL));
                list_push_back(input_queue, connfd);
            }
            else    //policy == "block"
            {
                while(queue_max_size <= input_queue->size)
                {
                    pthread_cond_wait(&available_cond, &mutex);
                }
                list_push_back(input_queue, connfd);
            }
        }

        if(input_queue->size > 0)
            pthread_cond_signal(&available_cond);
        pthread_mutex_unlock(&mutex);
    }
}


    


 
