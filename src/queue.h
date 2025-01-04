#include "lab3.h"

#ifndef __LAB3_QUEUE_H__
#define __LAB3_QUEUE_H__

typedef struct request {
    char op;         /* R/W/D/U */
    char key[31];    /* key: null-padded, max strlen = 30 */
    int  val;        /* val: an integer */
    struct request *next;   /* [queue] point to the next request */
} req_t;

typedef struct queue {
    req_t *head;    /* remove from head */
    req_t *tail;    /* append to tail */
    int count;
} queue_t;


// add a new request to the end of the queue
void enqueue(queue_t *q, req_t *t);

// pop a request from the head of the queue.
// if the queue is empty, the thread should wait.
req_t* dequeue(queue_t *q);

// get number of requests in the queue
int queue_count(queue_t *q);

#endif
