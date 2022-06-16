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
// Repeatedly handles HTTP requests sent to this port number .
// Most of the work is done within routines written in request.c
//

pthread_mutex_t mutex;
pthread_cond_t available_cond;
pthread_cond_t full_cond;

int working_threads;
int global_id;

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
    //printf("enter worker\n");
    List input_queue = (List)arg;

    //int id = list_pop(input_queue, NULL);   //getting the thread id
    
    struct timeval creation;    //stats declaration
    struct timeval working;
    struct timeval interval;
    struct Satistics stats;
    pthread_mutex_lock(&mutex);
    stats.id = global_id;
    global_id++;
    pthread_mutex_unlock(&mutex);
    stats.total_requests = 0;
    stats.static_requests = 0;
    stats.dynamic_requests = 0;

    int current_request = -1;
    pthread_mutex_lock(&mutex);
    while(1)    //let the worker threads continue working
    {
        //printf("enter while %d\n", current_request);
        while(input_queue->size <= 0)
        {
            //printf("enter current request\n");
            pthread_cond_wait(&available_cond, &mutex);
            //printf("Got to cond_wait\n");
            assert(input_queue != NULL);
        }
        
        current_request = list_pop(input_queue, &creation);
        assert(current_request != -1);
        working_threads++;
        
        stats.total_requests++;
        pthread_mutex_unlock(&mutex);

        gettimeofday(&working, NULL);

        timersub(&working, &creation, &interval);   //calculate the interval between creation of the request
                                             //to the time you started working on the request.


        requestHandle(current_request, &creation, &interval, &stats); //added creation time, working time, stats
        Close(current_request);

        //printf("Stats: total: %d, static: %d, dynamic:%d\n", stats.total_requests, stats.static_requests, stats.dynamic_requests);

        pthread_mutex_lock(&mutex);
        //printf("Got to loop's end\n");
        working_threads--;
        //printf("list_remove result: %d\n", remove);
        //current_request = -1;

        //if(input_queue->max_size > input_queue->size + working_threads)
            pthread_cond_signal(&full_cond);
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
    if(pthread_cond_init(&full_cond, NULL) != 0)
    {
        perror("Condition Variable initializtion failed");
        return -1;  //may have better error signal
    }

    working_threads = 0;
    int listenfd, connfd, port, clientlen, thread_num, queue_max_size;
    struct sockaddr_in clientaddr;
    char* policy = NULL;

    getargs(&port, argc, argv, &thread_num, &queue_max_size, policy);
    //printf("port: %d\n", port);

    //printf("before list creation\n");
    List input_queue = list_init(queue_max_size);

    //printf("created list\n");
    pthread_t* thread_array = malloc(sizeof(*thread_array)*thread_num);
    global_id = 0;
    //printf("created thread array\n");
    for(int i=0; i<thread_num; i++)
    {
        //list_push_back(input_queue, i);   //giving id to each thread
        //printf("for %d iteration\n", i);
        //thread_array[i] = (pthread_t*)malloc(sizeof(pthread_t));
        //pthread_create(thread_array[i], NULL, &worker_thread, list_array);
        pthread_create(&thread_array[i], NULL, &worker_thread, (void*)input_queue);
        //assert(thread_array[i] != NULL);
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
        if(input_queue->size + working_threads < queue_max_size){
            list_push_back(input_queue, connfd);
        }
        else
        {
            if(strcmp(policy, "dt") == 0)
            {
                Close(connfd);
            }
            else if(strcmp(policy, "random") == 0)
            {
                if(input_queue->size > 0)
                {
                    Close(connfd);
                }
                else
                {
                    list_random_delete(input_queue);
                    list_push_back(input_queue, connfd);
                }
            }
            else if(strcmp(policy, "dh") == 0)
            {
                if(input_queue->size > 0)
                {
                    Close(list_pop(input_queue, NULL));
                    list_push_back(input_queue, connfd);
                }
                else
                    Close(connfd);
            }
            else    //policy == "block"
            {
                while(queue_max_size <= input_queue->size + working_threads)
                {
                    pthread_cond_wait(&full_cond, &mutex);
                }
                list_push_back(input_queue, connfd);
            }
        }

        if(input_queue->size > 0)
            pthread_cond_signal(&available_cond);
        pthread_mutex_unlock(&mutex);
    }

    printf("HOW DID WE GET HERE");//-------------------------------------------------
    /*for(int i=0; i<thread_num; i++)
    {
        free(thread_array[i]);
    }*/
    free(thread_array);
    free(policy);

    list_destroy(input_queue);
}



    


 
