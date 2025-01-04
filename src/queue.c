#include "queue.h"

// Exercise 2: implement a concurrent queue

// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// add a new request to the end of the queue
// NOTE: queue must be implemented as a monitor
void enqueue(queue_t *q, req_t *t) {
    /* TODO: your code here */
    pthread_mutex_lock(&queue_mutex);

    if (q->tail == NULL) {
        q->head = t;
        q->tail = t;
    }
    else {
        q->tail->next = t;
        q->tail = t;
    }
    q->count++;

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

}

// pope a request from the head of the queue.
// if the queue is empty, the thread should wait.
// NOTE: queue must be implemented as a monitor
req_t* dequeue(queue_t *q) {
    /* TODO: your code here */
    pthread_mutex_lock(&queue_mutex);

    while (q->head == NULL) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    req_t* curr_req = q->head;
    q->head = q->head->next;

    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->count--;

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    return curr_req;
}

// return the number of requests in the queue.
// NOTE: queue must be implemented as a monitor
int queue_count(queue_t *q) {
    /* TODO: your code here */
    pthread_mutex_lock(&queue_mutex);

    int count = q->count;

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    return count;
}
