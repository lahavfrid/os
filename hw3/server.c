#include "segel.h"
#include "request.h"
#include "list.h"

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
pthread_cond_t available_job;
pthread_cond_t available_queue;

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


void* worker_thread(void* given_queue)
{
    int current_request = -1;

    pthread_mutex_lock(&mutex);
    List input_queue = (List)given_queue;
    while(1)    //let the worker threads continue working
    {
        while(current_request == -1)
        {
            pthread_cond_wait(&available_job, &mutex);
            current_request = list_pop(input_queue);
        }
        pthread_mutex_unlock(&mutex);

        requestHandle(current_request);
        Close(current_request);

        current_request = -1;
        pthread_mutex_lock(&mutex);
        if(input_queue->max_size > input_queue->size)
            pthread_cond_signal(&available_queue);
    }
    pthread_mutex_unlock(&mutex);   //oi vey, shouldn't reach here
}

int main(int argc, char *argv[])
{
    printf("sanity\n");
    pthread_mutex_init(&mutex, NULL);   //is this needed?
    if(pthread_cond_init(&available_job, NULL) != 0)
    {
        perror("Condition Variable initializtion failed");
        return -1;  //may have better error signal
    }
    if(pthread_cond_init(&available_queue, NULL) != 0)
    {
        perror("Condition Variable initializtion failed");
        return -1;  //may have better error signal
    }

    int listenfd, connfd, port, clientlen, thread_num, queue_max_size;
    struct sockaddr_in clientaddr;
    char* policy;

    getargs(&port, argc, argv, &thread_num, &queue_max_size, policy);
    printf("port: %d\n", port);

    // 
    // HW3: Create some threads...
    //

    List input_queue = list_init(queue_max_size);

    pthread_t* thread_array[thread_num];
    for(int i=0; i<thread_num; i++)
    {
        pthread_create(thread_array[i], NULL, &worker_thread, input_queue);
    }


    
    listenfd = Open_listenfd(port);
    printf("listenfd: %d\n", listenfd);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    printf("connfd: %d\n", connfd);

    pthread_mutex_lock(&mutex);
    if(input_queue->size < queue_max_size){
      list_push_back(input_queue, connfd);
    }
    else
    {
        while(queue_max_size <= input_queue->size)
        {
            pthread_cond_wait(&available_queue, &mutex);
        }
      //HANDLE ACCORDING TO policy
    }

    pthread_cond_signal(&available_job);
    pthread_mutex_unlock(&mutex);
    }
}


    


 
