#ifndef LOGQUEUE_H
#define LOGQUEUE_H
#include <stdio.h>
#include <stdlib.h>

/* Header file to implement a FIFO linked-list log queue. 
    This queue contains a log of each result of a string that
    was sent to the server by the client to have spell checked.

    by: Sean Reddington
*/

//LOG_t node struct to store data of incoming client connections
typedef struct LOG{
    int client_id;
    int thread_id;
    char *results;
    struct LOG *next;

}LOG_t;

//Queue struct of LOG nodes
typedef struct LOGQUEUE {
    unsigned size;
    LOG_t *head;
    LOG_t *tail;

}LOGQUEUE_t;

//Creates a new LOG_t node for the passed worker thread results.
LOG_t *newLog(int c_num, int t_id, char *r){
    LOG_t *temp = (LOG_t*)malloc(sizeof(LOG_t));
    temp->client_id = c_num;
    temp->thread_id = t_id;
    temp->results = r;
    temp->next = NULL;
    return temp;
}

//Creates a linked-list queue of LOG nodes
LOGQUEUE_t *createLogQueue(){
    LOGQUEUE_t *q = (LOGQUEUE_t*)malloc(sizeof(LOGQUEUE_t));
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
    return q;
}

/*Adds a LOG_t node from the passed worker thread data and
    inserts the node into the queue. */
void enQueueLog(int c_num, int t_id, char *r, LOGQUEUE_t *q){
    LOG_t *temp = newLog(c_num, t_id, r);

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

//Removes the LOG_t node at the head of the given queue q
LOG_t *deQueueLog(LOGQUEUE_t *q){

    //if queue is empty
    if(q->head == NULL){
        return NULL;
    }

    //stores previous head and move head one node ahead
    LOG_t *temp = q->head;
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
int LogQueueIsEmpty(LOGQUEUE_t *q){
    if(q->size == 0){
        return 1;
    }else
        return 0;
}
#endif
