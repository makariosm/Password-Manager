#include "kvstore.h"
#include <stdbool.h>

// Exercise 4: finish implementing kvstore
// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)
pthread_mutex_t kvstore_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t kvstore_cond = PTHREAD_COND_INITIALIZER;

/* read a key from the key-value store.
 *
 * if key exists, set "val" to be the value and return 0.
 * if key doesn't exist, return 1.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
int kv_read(kvstore_t *kv, char *key, int *val) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvstore_mutex);
    
    size_t key_idx = -1;
    for (size_t i = 0; i < kv->num_keys; i++) {
        if (strcmp(kv->keys[i].key,key) == 0) {
            key_idx = i;
            break;
        }
    }

    if (key_idx == -1) {
        pthread_cond_signal(&kvstore_cond);
        pthread_mutex_unlock(&kvstore_mutex);
        return 1;
    }
    else {
        *val = kv->values[key_idx];
        pthread_cond_signal(&kvstore_cond);
        pthread_mutex_unlock(&kvstore_mutex);
        return 0;
    }
}


/* write a key-value pair into the kv-store.
 *
 * - if the key exists, overwrite the old value.
 * - if key doesn't exit,
 *     -- insert one if the number of keys is smaller than TABLE_MAX
 *     -- return failure if the number of keys equals TABLE_MAX
 * - return 0 for success; return 1 for failures.
 *
 * notes:
 * - the input "key" locates somewhere out of your control, you must copy the
 *   string to kv-store's own memory. (hint: use strcpy)
 *
 * NOTE: kv-store must be implemented as a monitor.
 */

int kv_write(kvstore_t *kv, char *key, int val) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvstore_mutex);

    bool key_exists = false;
    for (size_t i = 0; i < kv->num_keys; i++) {
        if (strcmp(kv->keys[i].key, key) == 0) {
            kv->values[i] = val;
            key_exists = true;
            break;
        }
    }

    if (key_exists) {
        pthread_cond_signal(&kvstore_cond);
        pthread_mutex_unlock(&kvstore_mutex);

        return 0;
    }
    else {
        if (kv->num_keys < TABLE_MAX) {
            strcpy(kv->keys[kv->num_keys].key, key);
            kv->values[kv->num_keys] = val;
            kv->keys[kv->num_keys].stat = 1;
            kv->num_keys++;

            pthread_cond_signal(&kvstore_cond);
            pthread_mutex_unlock(&kvstore_mutex);

            return 0;
        }
        else {
            pthread_cond_signal(&kvstore_cond);
            pthread_mutex_unlock(&kvstore_mutex);

            return 1;
        }
    }

}


/* delete a key-value pair from the kv-store.
 *
 * - if the key exists, delete it.
 * - if the key doesn't exits, do nothing.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
void kv_delete(kvstore_t *kv, char *key) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvstore_mutex);

    size_t key_idx = -1;
    for (size_t i = 0; i < kv->num_keys; i++) {
        if (strcmp(kv->keys[i].key, key) == 0) {
            key_idx = i;
            break;
        }
    }

    if (key_idx != -1) {
        for (size_t i = key_idx; i < kv->num_keys - 1; i++) {
            strcpy(kv->keys[i].key, kv->keys[i + 1].key);
            kv->values[i] = kv->values[i + 1];
            kv->keys[i].stat = kv->keys[i + 1].stat;
        }

        kv->keys[kv->num_keys - 1].key[0] = '\0';
        kv->values[kv->num_keys - 1] = 0;
        kv->keys[kv->num_keys - 1].stat = 0;
        kv->num_keys--;
    }

    pthread_cond_signal(&kvstore_cond);
    pthread_mutex_unlock(&kvstore_mutex);
    
}


/* increase the value of a key
 *
 * - if the key exists, increase "val" on top of the old value.
 * - if key doesn't exit,
 *     -- insert one if the number of keys is smaller than TABLE_MAX
 *     -- return failure if the number of keys equals TABLE_MAX
 * - return 0 for success; return 1 for failures.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
int kv_increase(kvstore_t* kv, char* key, int val) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvstore_mutex);

    size_t key_idx = -1;
    for (size_t i = 0; i < kv->num_keys; i++) {
        if (strcmp(kv->keys[i].key, key) == 0) {
            key_idx = i;
            break;
        }
    }

    if (key_idx == -1) {
        if (kv->num_keys < TABLE_MAX) {
            strcpy(kv->keys[kv->num_keys].key, key);
            kv->values[kv->num_keys] = val;
            kv->keys[kv->num_keys].stat = 1;
            kv->num_keys++;

        }
        else {
            pthread_cond_signal(&kvstore_cond);
            pthread_mutex_unlock(&kvstore_mutex);

            return 1;
        }
    }
    else {
        kv->values[key_idx] += val;

    }

    pthread_cond_signal(&kvstore_cond);
    pthread_mutex_unlock(&kvstore_mutex);

    return 0;
}

// print kv-store's contents to stdout
// note: use any format that you like; this is mostly for debugging purpose
void kv_dump(kvstore_t *kv) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvstore_mutex);

    printf("Key-Value Store Contents:\n");

    for (size_t i = 0; i < kv->num_keys; i++) {
        printf("[%zu] %s: %d\n", i, kv->keys[i].key, kv->values[i]);
    }

    pthread_cond_signal(&kvstore_cond);
    pthread_mutex_unlock(&kvstore_mutex);

}
