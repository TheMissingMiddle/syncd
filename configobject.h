//
// Created by alex on 6/6/17.
//

#ifndef MAGICSYNC_CONFIGOBJECT_H
#define MAGICSYNC_CONFIGOBJECT_H

#include <stdlib.h>

#define NUM_OF_ELEMENTS 20
#define DOUBLE_SIZE 20
#define VEC_SUCCESS 1
#define VEC_FAILURE 0
struct vec_element_t {
    const char *collection;
    const char *redis;
    const char *misc;
};

struct vec_t {
    int ptr;
    int len;
    struct vec_element_t *elements;
};


#define SYNCD_VEC_ELEMENT static struct vec_element_t
#define SYNCD_GLOBAL_VECTOR static struct vec_t
typedef struct {
    int port;
    int redis_port;
    int mongo_port;
    const char *redis_address;
    const char *mongo_address;
    const char *address;
    const char *mongo_db_name;
    struct vec_t push;
    struct vec_t pull;
} syncd_config_t;


int init_element(struct vec_element_t *element, const char *a, const char *b, const char *c);

int init_vect(struct vec_t *vec);

int add_size_vect(struct vec_t *vec, unsigned int addSize);

int append_vect(struct vec_t *vec, struct vec_element_t *e);

int peek_vect(struct vec_t *vec, struct vec_element_t *e);

int pop_back_vect(struct vec_t *vec, struct vec_element_t *recv);

int vect_iter_next(struct vec_t *vec, struct vec_element_t *recv);

void vect_incomplete_free(struct vec_t *vec);

#endif //MAGICSYNC_CONFIGOBJECT_H
