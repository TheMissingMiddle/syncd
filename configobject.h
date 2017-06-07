//
// Created by alex on 6/6/17.
//

#ifndef MAGICSYNC_CONFIGOBJECT_H
#define MAGICSYNC_CONFIGOBJECT_H

#include <stdlib.h>

#define SYNCD_QUEUE_INIT_LEN 20
#define SYNCD_QUEUE_MAX_LEN 1000
#define SYNCD_QUEUE_ADD_SLOTS 10
#define SYNCD_QUEUE_OP_SUCCESS 11
#define SYNCD_QUEUE_OP_FAILURE 12
typedef struct {
    const char *collection;
    const char *redis;
    const char *misc;
} syncd_queue_element_t;

typedef struct {
    size_t len;
    long ptr;
    syncd_queue_element_t *q;
} syncd_queue_t;

typedef struct {
    int port;
    char *address;
    char *mongo_db_name;
    syncd_queue_t push;
    syncd_queue_t pull;
} syncd_config_t;

int
syncd_queue_create_queue_element(syncd_queue_element_t *element, const char *col, const char *red, const char *misc);

int syncd_queue_init(syncd_queue_t *queue, size_t optional_length);

int syncd_queue_insert(syncd_queue_t *queue, syncd_queue_element_t *element);

int syncd_queue_peek(syncd_queue_t *queue, syncd_queue_element_t *element);

int syncd_queue_pop(syncd_queue_t *queue, syncd_queue_element_t *element);

void syncd_queue_destroy(syncd_queue_t *queue);


#endif //MAGICSYNC_CONFIGOBJECT_H
