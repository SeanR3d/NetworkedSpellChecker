#ifndef WORKQUEUE_H
#define WORKQUEUE_H
#include <stdio.h>
#include <stdlib.h>


/* Header file to implement a FIFO linked-list queue of client 
    connections waiting to be serviced by worker threads. 
    Each node of the work queue contains information about the 
    client's socket connection to the spell checker server.

    by: Sean Reddington
*/

//CLIENT_t node struct to store data of incoming client connections
typedef struct CLIENT{
    int client_num;
    int client_socket;
    struct CLIENT *next;

}CLIENT_t;

//Queue struct of CLIENT nodes
typedef struct WORKQUEUE {
    unsigned size;
    CLIENT_t *head;
    CLIENT_t *tail;

}WORKQUEUE_t;

//Creates a new CLIENT_t node for the passed client connection
CLIENT_t *newClient(int c_num, int c_socket){
    CLIENT_t *temp = (CLIENT_t*)malloc(sizeof(CLIENT_t));
    temp->client_num = c_num;
    temp->client_socket = c_socket;
    temp->next = NULL;
    return temp;
}

//Creates a linked-list queue of CLIENT nodes
WORKQUEUE_t *createWorkQueue(){
    WORKQUEUE_t *q = (WORKQUEUE_t*)malloc(sizeof(WORKQUEUE_t));
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
    return q;
}

//Adds a CLIENT_t node of the passed client connection into the queue.
void enQueueClient(int c_num, int c_socket, WORKQUEUE_t *q){
    CLIENT_t *temp = newClient(c_num, c_socket);

    //if queue is empty
    if(q->tail == NULL){
        q->head = temp;
        q->tail = temp;
        q->size++;
        return;
    }

    //adds the new node at the end of queue
    q->tail->next = temp;
    q->tail = temp;
    q->size++;
}

//Removes the CLIENT_t node at the head of the given queue q
CLIENT_t *deQueueClient(WORKQUEUE_t *q){

    //if queue is empty
    if(q->head == NULL){
        return NULL;
    }

    //stores previous head and move head one node ahead
    CLIENT_t *temp = q->head;
    q->head = q->head->next;

    //if head becomes NULL, then also change rear to NULL
    if(q->head == NULL){
        q->tail = NULL;
        q->size = 0;
    }else{
        q->size--;
    }

    return temp;
}

//Returns 1 if queue is empty, otherwise returns 0
int workQueueIsEmpty(WORKQUEUE_t *q){
    if(q->size == 0){
        return 1;
    }else
        return 0;
}
#endif
