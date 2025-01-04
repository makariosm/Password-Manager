#include "worker.h"
#include "stats.h"
#include "assert.h"

extern int if_sleep;          // defined in kvserver.c
extern void cleanup(int sig); // defined in kvserver.c
extern void trace(char *msg); // defined in kvserver.c


// handle writes
void handle_write(char *key, int val, kvstore_t *kv)
{
    inc_write(); // see stats.c

    char msg[128]; // trace for debug

    int ret = kv_write(kv, key, val);
    if (ret == 0) {
        sprintf(msg, "W[%s<-%d]:OK", key, val);
    } else {
        sprintf(msg, "W[%s<-%d]:Error(%d)", key, val, ret);
    }

    trace(msg);
}

void handle_read(char *key, kvstore_t *kv)
{
    inc_read(); // see stats.c

    char msg[128]; // trace for debug

    int val = -1;
    int ret = kv_read(kv, key, &val);

    if (ret == 0) {
        sprintf(msg, "R[%s->%d]:OK", key, val);
    } else {
        sprintf(msg, "R[%s->%d]:Error(%d)", key, val, ret);
    }

    trace(msg);
}

void handle_delete(char *key, kvstore_t *kv)
{
    inc_delete(); // see stats.c

    kv_delete(kv, key);

    char msg[128]; // trace for debug
    sprintf(msg, "D[%s]:OK", key);
    trace(msg);
}


void handle_increase(char *key, int val, kvstore_t *kv)
{
    inc_increase();

    char msg[128]; // trace for debug

    int ret = kv_increase(kv, key, val);
    if (ret == 0) {
        sprintf(msg, "I[%s+=%d]:OK", key, val);
    } else {
        sprintf(msg, "I[%s+=%d]:Error(%d)", key, val, ret);
    }

    trace(msg);
}


void execute(req_t *rq, kvstore_t *kv)
{
    // random sleep to trigger concurrency bugs
    usleep( (random() * if_sleep)  % 10000);
    switch(rq->op) {
        case 'W':
            handle_write(rq->key, rq->val, kv);
            break;
        case 'R':
            handle_read(rq->key, kv);
            break;
        case 'D':
            handle_delete(rq->key, kv);
            break;
        case 'I':
            handle_increase(rq->key, rq->val, kv);
            break;
        default:
            fprintf(stderr,
                    "worker.c:execute(): UNKOWN command '%c' with [key='%s', and val='%d']",
                    rq->op, rq->key, rq->val);
            assert(0);
    }
}

// worker thread function
void *worker(void *args)
{
    queue_t *wq = ((args_t*)args)->q;
    kvstore_t *kv = ((args_t*)args)->kv;
    int first_null = 0;

    for (;;) {
        req_t *req = dequeue(wq);  // get one request
        if (req != NULL) {         // if request is not empty,
            execute(req, kv);      // then execute the request on kvstore
            free(req);
        } else {
            if (first_null == 0) {
                printf("[INFO] worker thread dequeue NULL...\n");
                first_null = 1;
            }
            sleep(1);
        }
    }
}
