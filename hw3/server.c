#include "segel.h"
#include "request.h"

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
pthread_cond_t cond;

// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[], int* thread_num, int* queue_size, char* policy)
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    policy = malloc(sizeof(argv[4]));
    strcpy(policy, argv[4]);
}


void* worker_thread(void* args)
{
    int current_request;
    pthread_mutex_lock(&mutex);
    while(1)    //let the worker threads continue working
    {
        pthread_cond_wait(&cond, &mutex);
        //current_request = queue.pop();
        pthread_mutex_unlock(&mutex);

        //requestHandle(current_request);
        //Close(current_request);

        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&mutex, NULL);   //is this needed?
    if(pthread_cond_init(&cond, NULL) != 0)
    {
        perror("Condition Variable initializtion failed");
        return;
    }

    int listenfd, connfd, port, clientlen, thread_num, queue_max_size;
    struct sockaddr_in clientaddr;
    char* policy;

    getargs(&port, argc, argv, &thread_num, &queue_max_size, policy);

    // 
    // HW3: Create some threads...
    //

    //intilaize queue for requests (fds)    -----------------------------

    pthread_t* thread_array[thread_num];
    for(int i=0; i<thread_num; i++)
    {
        pthread_create(thread_array[i], NULL, &worker_thread, NULL);
    }



    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    pthread_mutex_lock(&mutex);
    //if(queue.size < queue_max_size){
    //  queue.pushback(connfd);
    //}
    //else{
    //  HANDLE ACCORDING TO policy
    //}
    pthread_mutex_unlock(&mutex);
	
    }

}


    


 
