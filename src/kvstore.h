#include "lab3.h"

#ifndef __LAB3_KVSTORE_H__
#define __LAB3_KVSTORE_H__

// a shared key-value store: mapping keys to entries
// NOTE: this is a horribly inefficient implementation...
// Want better performance?
// re-implement below with better data structures


typedef struct key_entry {
    int stat; // the status of one key (or entry)
              // FREE = 0, USED = 1
    char key[32];  // key is a string within 31 characters
} key_entry_t;


// an inefficient mapping from keys to values:
//    keys[i] => values[i]  (i < TABLE_MAX)
typedef struct kvstore {
    key_entry_t keys[TABLE_MAX];
    int values[TABLE_MAX];
    size_t num_keys;
} kvstore_t;


//=== key-value store APIs ===
// DO NOT change the interfaces below

// read a key from the key-value store.
// - if key exists, put key's value into "val" and return 0.
// - if key doesn't exist, return 1 for failure.
int kv_read(kvstore_t *kv, char *key, int *val);

// write the value of a key
// - if the key exists, overwrite the old value.
// - if key doesn't exit,
//     -- insert one if the number of keys is smaller than TABLE_MAX
//     -- return failure if the number of keys equals TABLE_MAX
// - return 0 for success; return 1 for failures.
int kv_write(kvstore_t *kv, char *key, int val);


// delete a key-value pair from the kv-store.
// - if the key exists, delete it.
// - if the key doesn't exits, do nothing.
void kv_delete(kvstore_t *kv, char *key);


// increase the value of a key
// - if the key exists,
//     -- increase the value of the key by val
//     -- note that val might be a negative number (then means decrease)
// - if key doesn't exit,
//     -- insert one if the number of keys is smaller than TABLE_MAX
//     -- set the value to be the given "val"
//     -- return failure if the number of keys equals TABLE_MAX
// - return 0 for success; return 1 for failures.
int kv_increase(kvstore_t *kv, char *key, int val);

// print all the kv contents to stdout
void kv_dump(kvstore_t *kv);

#endif
