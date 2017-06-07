//
// Created by alex on 6/6/17.
//

#include <stdio.h>
#include <stdlib.h>
#include "configobject.h"

int
syncd_queue_create_queue_element(syncd_queue_element_t *element, const char *col, const char *red, const char *misc) {
    element = malloc(sizeof(element));
    element->collection = col;
    element->misc = misc;
    element->redis = red;
}

int syncd_queue_init(syncd_queue_t *queue, size_t optional_length) {
    if (optional_length != 0) {
        // create using user-specified length
        queue->len = optional_length;
        queue->ptr = -1;
        queue->q = malloc(sizeof(syncd_queue_element_t) * optional_length);
        return (queue->q != NULL) ? SYNCD_QUEUE_OP_SUCCESS : SYNCD_QUEUE_OP_FAILURE;
    } else {
        // create using default length
        queue->len = SYNCD_QUEUE_INIT_LEN;
        queue->ptr = 0;
        queue->q = malloc(sizeof(syncd_queue_element_t) * SYNCD_QUEUE_INIT_LEN);
        return (queue->q != NULL) ? SYNCD_QUEUE_OP_SUCCESS : SYNCD_QUEUE_OP_FAILURE;
    }
}

int syncd_queue_insert(syncd_queue_t *queue, syncd_queue_element_t *element) {
    if (queue->ptr > queue->len) {
        // a little big!!
        // reallocate
        syncd_queue_element_t *n = realloc(queue->q,
                                           sizeof(syncd_queue_element_t) * (queue->len + SYNCD_QUEUE_ADD_SLOTS));
        if (n == NULL) {
            printf("syncd_queue_insert(): realloc() failure, panicking...");
            free(queue->q);
            return SYNCD_QUEUE_OP_FAILURE;
        }
        queue->q = n;
        queue->len += SYNCD_QUEUE_ADD_SLOTS;
    }
    queue->q[++queue->ptr] = *element;
    return SYNCD_QUEUE_OP_SUCCESS;

}

int syncd_queue_peek(syncd_queue_t *queue, syncd_queue_element_t *element) {
    if (queue->ptr < 0) return SYNCD_QUEUE_OP_FAILURE; // no elements to peek!
    syncd_queue_element_t e = queue->q[0];
    element->redis = e.redis;
    element->misc = e.misc;
    element->collection = e.collection;
    return SYNCD_QUEUE_OP_SUCCESS;
}

int syncd_queue_pop(syncd_queue_t *queue, syncd_queue_element_t *element) {
    if (queue->ptr < 0) return SYNCD_QUEUE_OP_FAILURE; // no elements to pop!
    syncd_queue_element_t e = queue->q[0];
    element->collection = e.collection;
    element->misc = e.misc;
    element->redis = e.redis;
    // but now...
    free((char *)queue->q[0].collection);
    free((char *)queue->q[0].redis);
    free((char *)queue->q[0].misc);
    --queue->ptr;
    if (queue->ptr == 0) {
        // unique case when only one is left
        return SYNCD_QUEUE_OP_SUCCESS;
    }
    void *ptr = &queue->q[1];
    if (ptr == NULL) {
        printf("syncd_queue_pop(): Err: len doesn't match! panicking...");
        return SYNCD_QUEUE_OP_FAILURE;
    }
    queue->q = ptr;
    return SYNCD_QUEUE_OP_SUCCESS;
}

void syncd_queue_destroy(syncd_queue_t *queue) {
    int i;
    for (i = 0; i < queue->ptr; ++i) {
        free((char *)queue->q[i].collection);
        free((char *)queue->q[i].redis);
        free((char *)queue->q[i].misc);
    }
    free(queue->q);
    queue->len = 0;
    queue->ptr = -1;
}

