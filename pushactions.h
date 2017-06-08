//
// Created by alex on 6/8/17.
//

#ifndef MAGICSYNC_PUSHACTIONS_H
#define MAGICSYNC_PUSHACTIONS_H

#include "configobject.h"
#include <memory.h>
#include <stdio.h>
#include <libbson-1.0/bson.h>
#include <libmongoc-1.0/mongoc.h>
#include <hiredis/hiredis.h>

#define PUSH_SUCCESS 1
#define PUSH_FAILURE 0

int push(syncd_config_t *config, redisContext *redis_client, mongoc_client_t *mongoc_client);

#endif //MAGICSYNC_PUSHACTIONS_H
