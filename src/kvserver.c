/*
 * Peter Desnoyers, 2020
 *
 * 2021 fall: modified by CS5600 staff
 * 2024 spring: modified by CS3650 staff
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "lab3.h"
#include "worker.h"
#include "stats.h"
#include "queue.h"
#include "kvstore.h"

// === global variables ===
queue_t wq;  // the working queue
kvstore_t kv;  // the kv-store

// a trace log for debugging
int debug = 1;  // 0: stop debug tracing; 1: open debug
FILE *trace_fp;
pthread_mutex_t trm = PTHREAD_MUTEX_INITIALIZER;

// if workers will sleep
int if_sleep = 1;

// === helper functions ===

// close the resources when quitting
void cleanup(int sig)
{
    if (trace_fp != NULL) {
        fclose(trace_fp);
    }
    exit(0);
}

void init(void) {
    if (signal(SIGINT, cleanup) == SIG_ERR) {
        perror("catch SIGINT:");
    }
    for (int i = 0; i < TABLE_MAX; i++) {
        kv.keys[i].stat = 0; // set to FREE
    }
    kv.num_keys = 0;
    if (debug) {
        trace_fp = fopen("./kvserver.trace", "w");
    }
}


void _trace(char *msg) {
    pthread_mutex_lock(&trm);
    fprintf(trace_fp, "%s\n", msg);
    pthread_mutex_unlock(&trm);
}

void trace(char *msg) {
    if (!debug) {return;}
    _trace(msg);
}

// === dispatcher function ===

req_t *parsereq(char *line) {
    req_t *ret = malloc(sizeof(req_t));
    assert(line[1] == ',');
    ret->op = line[0];

    switch(ret->op) {
        case 'R':
        case 'D':
            assert(strchr(&line[2], ',') == NULL); // assert no ',' in key
            strncpy(ret->key, &line[2], 31);
            ret->key[30] = '\0'; // terminate the string anyway
            ret->val = 0;
            ret->next = NULL; // used in queue
            break;
        case 'W':
        case 'I':
            {
                char *ptr;
                char *key = strtok_r(&line[2], ",", &ptr);
                strncpy(ret->key, key, 31);
                ret->key[30] = '\0'; // terminate the string anyway
                char *val = strtok_r(NULL, ",", &ptr);
                ret->val = atoi(val);
                ret->next = NULL; // used in queue
            }
            break;
        default:
            fprintf(stderr, "[Error] Unknown op=%c\n", line[0]);
            exit(1);
    }
    return ret;
}


void *dispatcher(void *file) {
    // load the input file
    int fd = open((char*)file, O_RDONLY);
    if (fd < 0) {
        perror("[Error] opening file");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("[Error] getting file size");
        return NULL;
    }

    char *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("[Error] mmapping file");
        return NULL;
    }
    char *buf = malloc(sb.st_size);
    memcpy(buf, mapped, sb.st_size);

    // parse and enqueue the inputs
    int op_counter = 0;
    char *ptr;
    char *line = strtok_r(buf, "\n", &ptr);
    while (line != NULL) {
        req_t *req = parsereq(line);  // parse string => req_t
        // printf("req[op=%c],key=%s,val=%d\n", req->op, req->key, req->val);
        enqueue(&wq, req);            // enqueue req
        line = strtok_r(NULL, "\n", &ptr);    // get the next req
        op_counter++;
    }

    printf("[INFO] #ops=[%d] are loaded\n", op_counter);
    free(buf);
    return NULL;
}


// === control (main) thread ===

int main(int argc, char **argv)
{
    init();

    // number of threads
    int num_thread = 4;
    char *str_nthr = getenv("LAB3_NUM_THREAD");
    if (str_nthr != NULL) {
        num_thread = atoi(str_nthr);
        printf("[INFO] set num_thread = %d\n", num_thread);
    }

    // if workers sleep
    char *str_ifsleep = getenv("LAB3_IF_SLEEP");
    if (str_ifsleep != NULL) {
        if_sleep = atoi(str_ifsleep);
        printf("[INFO] set if_sleep = %d\n", if_sleep);
    }

    // set up the arguments for worker threads
    args_t args;
    args.q = &wq;
    args.kv = &kv;
    (void) args; // cancel compiler warnings

    // Exercise 1.a: create multiple worker threads
    //   - the worker thread should run function "worker".
    //     (defined in worker.h; implemented in worker.c)
    //   - each worker thread needs the argument "args" (defined above)
    //     (how to pass arguments to a thread? read pthread tutorials)
    //   - you should create multiple worker threads
    //     (create "num_thread" of them)
    //
    //  (hint: you only need to write a few lines of code here)

    /* TODO: your code here */
    pthread_t worker_threads[num_thread]; // Array to hold worker threads

    for (int i = 0; i < num_thread; i++) {
        pthread_create(&worker_threads[i], NULL, worker, (void*)&args);
    }


    // main thread: getting cmd from admin
    for (;;) {
        char line[80], cmd[8];
        printf("cs3650> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);

        int n = sscanf(line, "%s", cmd);
        if (n < 1)
            continue;

        if (strcmp(cmd, "quit") == 0) {
            printf("quitting...\n");
            cleanup(0);
        }
        else if (strcmp(cmd, "load") == 0) {
            char file[80];
            sscanf(line, "%*s %s", file);

            // Exercise 1.b: create a dispatcher threads
            //   - the dispatcher thread should run function "dispatcher".
            //     (defined above in this file)
            //   - the dispatcher thread needs the argument "file" (defined above)
            //   - you should create one thread
            /* TODO: your code here */
            pthread_t dispatcher_thread;

            pthread_create(&dispatcher_thread, NULL, dispatcher, (void*)file);


        }
        else if (strcmp(cmd, "stats") == 0) {
            printf("queued:    %d\n", queue_count(&wq));
            printf("writes:    %d\n", get_writes());
            printf("reads:     %d\n", get_reads());
            printf("deletes:   %d\n", get_deletes());
            printf("increases: %d\n", get_increases());
        }
        else if (strcmp(cmd, "list") == 0) {
            kv_dump(&kv);
        }
        else if (strcmp(cmd, "help") == 0) {
            printf("load <f>: load a workload from file <f>\n");
            printf("stats:    display the status of kvserver\n");
            printf("list:     list all existing kv-pairs in kvserver\n");
            printf("quit:     terminiate kvserver\n");
        }
        else {
            printf("UNKNOWN cmd: <%s>\n", cmd);
        }
    }

    cleanup(0);
}

