//
// Created by alex on 6/6/17.
//

#include <stdio.h>
#include <stdlib.h>
#include "configobject.h"

int syncd_queue_create_queue_element(syncd_queue_element_t * element, void * data, size_t length) {
    element = malloc(sizeof(element));
    element->len = length;
    element->raw = data;
}

int syncd_queue_init(syncd_queue_t * queue, size_t optional_length) {
    if(optional_length == -1) {
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

int syncd_queue_insert(syncd_queue_t * queue, syncd_queue_element_t * element) {
    if(queue->ptr > queue->len) {
        // a little big!!
        // reallocate
        syncd_queue_t * newqueue = realloc(queue, queue->len + SYNCD_QUEUE_ADD_SLOTS);
        if(newqueue == NULL) {
            printf("syncd_queue_insert(): realloc() failure, panicking...");
            free(queue);
            return SYNCD_QUEUE_OP_FAILURE;
        }
        queue = newqueue;
        queue->ptr += SYNCD_QUEUE_ADD_SLOTS;
    }
    queue->q[++queue->len] = * element;
    return SYNCD_QUEUE_OP_SUCCESS;

}

int syncd_queue_peek(syncd_queue_t * queue, syncd_queue_element_t * element) {
    if(queue->ptr < 0) return SYNCD_QUEUE_OP_FAILURE; // no elements to peek!
    syncd_queue_element_t e = queue->q[0];
    element->len = e.len;
    element->raw = e.raw;
    return SYNCD_QUEUE_OP_SUCCESS;
}

int syncd_queue_pop(syncd_queue_t * queue, syncd_queue_element_t * element) {
    if(queue->ptr < 0) return SYNCD_QUEUE_OP_FAILURE; // no elements to pop!
    syncd_queue_element_t e = queue->q[0];
    element->len = e.len;
    element->raw = e.raw;
    // but now...
    queue->q[0].len = 0;
    free(queue->q[0].raw);
    queue->q[0].raw = NULL;
    --queue->ptr;
    if(queue->ptr == 0) {
        // unique case when only one is left
        return SYNCD_QUEUE_OP_SUCCESS;
    }
    void * ptr = &queue->q[1];
    if(ptr==NULL) {
        printf("syncd_queue_pop(): Err: len doesn't match! panicking...");
        return SYNCD_QUEUE_OP_FAILURE;
    }
    queue->q = ptr;
    return SYNCD_QUEUE_OP_SUCCESS;
}