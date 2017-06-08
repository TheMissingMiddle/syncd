//
// Created by alex on 6/6/17.
//

#include <stdio.h>
#include <stdlib.h>
#include "configobject.h"

#define PRINT(functname, x) printf("syncd vector %s(): ERR %s", functname, x);

int init_element(struct vec_element_t *element, const char *a, const char *b, const char *c) {
    element->collection = a;
    element->redis = b;
    element->misc = c;
    if(element->collection==NULL||element->redis==NULL||element->misc==NULL) {
        PRINT("init_element", "one or more elements passed in are null");
        return VEC_FAILURE;
    }
    return VEC_SUCCESS;
}

int init_vect(struct vec_t *vec) {
    vec->ptr = -1;
    vec->len = NUM_OF_ELEMENTS;
    vec->elements = malloc(sizeof(struct vec_element_t) * NUM_OF_ELEMENTS);
    if(vec->elements==NULL) {
        PRINT("init_vect", "malloc() vailure while initializing vector.");
        return VEC_FAILURE;
    }
    return VEC_SUCCESS;
}

int add_size_vect(struct vec_t *vec, unsigned int addSize) {
    struct vec_element_t *newSize = realloc(vec->elements,
                                            sizeof(vec->elements) + addSize * sizeof(struct vec_element_t));
    if(newSize==NULL) {
        PRINT("add_size_vect", "realloc() failure. CANNOT re-allocate vector to new size.");
        return VEC_FAILURE;
    }
    vec->elements = newSize;
    return VEC_SUCCESS;
}

int append_vect(struct vec_t *vec, struct vec_element_t *e) {
    if (vec->ptr >= vec->len) {
        if(add_size_vect(vec, DOUBLE_SIZE)==VEC_FAILURE) {
            PRINT("append","Throwing realloc failure down the link.");
            return VEC_FAILURE;
        }
    }
    vec->elements[++vec->ptr] = *e;
    return VEC_SUCCESS;
}

int peek_vect(struct vec_t *vec, struct vec_element_t *e) {
    if (vec->ptr == -1) {
        PRINT("peek_vect", "vector is empty. Nothing to peek_vect!");
        return VEC_FAILURE;
    }
    struct vec_element_t e1 = vec->elements[0];
    e->collection = e1.collection;
    e->redis = e1.redis;
    e->misc = e1.misc;
    return VEC_SUCCESS;
}

int pop_back_vect(struct vec_t *vec, struct vec_element_t *recv) {
    if(vec->ptr==-1) {
        PRINT("pop_back_vect", "vector is empty. Nothing to pop!");
        return VEC_FAILURE;
    }
    const struct vec_element_t d1 = vec->elements[--vec->ptr];
    recv->redis = d1.redis;
    recv->collection = d1.collection;
    recv->misc = d1.misc;
    return VEC_SUCCESS;
}

int pop_front_vect(struct vec_t *vec, struct vec_element_t *recv) {
    if(vec->ptr==-1) {
        PRINT("pop_front_vect", "vector is empty. Nothing to pop!");
        return VEC_FAILURE;
    }
    const struct vec_element_t t1 = vec->elements[0];
    recv->redis = t1.redis;
    recv->collection = t1.collection;
    recv->misc = t1.misc;
    if(vec->ptr==0) {
        --vec->ptr;
    } else {
        void * p = &vec->elements[1];
        vec->elements = p;
    }
    return VEC_SUCCESS;
}

int vect_iter_next(struct vec_t * vec, struct vec_element_t * recv) {
    static int iter_counter = 0;
    if(iter_counter <= vec->ptr) {
        *recv = vec->elements[iter_counter];
        ++iter_counter;
        return 1;
    } else {
        iter_counter = 0;
        recv=NULL;
        return 0;
    }
}


