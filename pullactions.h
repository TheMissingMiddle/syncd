//
// Created by alex on 6/8/17.
//

#ifndef MAGICSYNC_PULLACTIONS_H
#define MAGICSYNC_PULLACTIONS_H
#include "configobject.h"
#include <memory.h>
#include <stdio.h>
#include <libbson-1.0/bson.h>
#include <libmongoc-1.0/mongoc.h>
#include <hiredis/hiredis.h>
#define PULL_EXIT_FAILURE 0
#define PULL_EXIT_SUCCESS 1
int pull(syncd_config_t *config, redisContext *redis_client, mongoc_client_t *mongo_client);

#endif //MAGICSYNC_PULLACTIONS_H
