/*
 * Created by Kremlin Master Control on 6/9/17.
 * Copyright (c) 2017 Conceptual Inertia, Inc. All rights reserved.
 *
 * License included in project root folder.
 * Legal Contact: Conceptual-Inertia@magikpns.com
 */

#ifndef MAGICSYNC_ACTORS_H
#define MAGICSYNC_ACTORS_H

#define PUSH_SUCCESS 1
#define PUSH_FAILURE 0

#define PULL_EXIT_FAILURE 0
#define PULL_EXIT_SUCCESS 1
/*
 * Created by Kremlin Master Control on 6/9/17.
 * Copyright (c) 2017 Conceptual Inertia, Inc. All rights reserved.
 *
 * License included in project root folder.
 * Legal Contact: Conceptual-Inertia@magikpns.com
 */

#include "actors.h"
#include "configobject.h"
#include <stdio.h>
#include <stdlib.h>
#include <libbson-1.0/bson.h>
#include <libmongoc-1.0/mongoc.h>
#include <hiredis/hiredis.h>
#include <unistd.h>

#define MONGO_CONNECT_FAILURE 10
#define MONGO_CONNECT_SUCCESS 110
#define MONGO_GETDATABASE_FAILURE 11
#define MONGO_EXEC_FAILURE 12
#define REDIS_CONNECT_FAILURE 13
#define REDIS_CONNECT_SUCCESS 113
#define GENERAL_FAILURE 201
#define GENERAL_SUCCESS 200
#define IS_VALID_CHAR(c) (c>=97&&c<=122)||(c>=65&&c<=90)||(c>=48&&c<=57) /* (is lower case) || (is upper case) || (is digit) */
#define CUSTOM_MONGO_FIND_WITH_OPTS(collection, query) mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL)
#define PRINT(fn, x) printf("%s(): ERR : %s\n", fn, x)
#define TASK_QUEUE_PULL 1
#define TASK_QUEUE_PUSH 2
#define URGENCY_REAL_TIME 3
#define URGENCY_IMPORTANT 4
#define URGENCY_MEDIOCRE 5
#define URGENCY_LOW 6
#define THREAD_QUEUE_SUCCESS 1
#define THREAD_QUEUE_FAILURE 0
#define THREAD_QUEUE_INIT_SIZE 50
#define THREAD_QUEUE_DOUBLE_SIZE 30
struct thread_queue_element {
    int action;
    int urgency;
};
struct thread_queue_t {
    long long len;
    long long bottom;
    long long top;
    struct thread_queue_element * queue;
};
#define TASK_QUEUE static struct thread_queue_t
#define TASK_QUEUE_ELEM static struct thread_queue_element

mongoc_client_t *mongo_client = NULL;
redisContext *redis_client = NULL;

int init_redis(const char *redis_address, int redis_port);

void cleanup_redis();

int init_mongo(const char *mongodb_address);

void cleanup_mongo();


int pull(syncd_config_t *config);


int push(syncd_config_t *config);

#endif //MAGICSYNC_ACTORS_H
