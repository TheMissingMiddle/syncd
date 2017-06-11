/*
 * Created by Kremlin Master Control on 6/9/17.
 * Copyright (c) 2017 Conceptual Inertia, Inc. All rights reserved.
 *
 * License included in project root folder.
 * Legal Contact: Conceptual-Inertia@magikpns.com
 */

#include "actors.h"
#include "configobject.h"
#include <pthread.h>


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


redisContext *init_redis(const char *redis_address, int redis_port, redisContext *redis_client) {
    redis_client = redisConnect(redis_address, redis_port);
    if (connect == NULL) {
        return REDIS_CONNECT_FAILURE;
    } else if (redis_client->err) {
        printf("Redis Connect Error: %s\n", redis_client->errstr);
        return
    }
}

void cleanup_redis(redisContext *redis_client) {
    redisFree(redis_client);
}

int init_mongo(const char *mongodb_address, mongoc_client_t *mongo_client) {
    if ((mongo_client = mongoc_client_new(mongodb_address)) == NULL) {
        printf("mongo init failed\n");
        return MONGO_CONNECT_FAILURE;
    }
    return MONGO_CONNECT_SUCCESS;
}

void cleanup_mongo(mongoc_client_t *mongo_client) {
    mongoc_client_destroy(mongo_client);
    mongoc_cleanup();
}


int pull(syncd_config_t *config, mongoc_client_t *mongo_client, redisContext *redis_client) {
    struct vec_element_t recv;
    while (vect_iter_next(&(config->pull), &recv)) {
        const char *src_collection = recv.collection;
        const char *redis_key = recv.redis;
        mongoc_database_t *database;
        mongoc_collection_t *col;
        mongoc_cursor_t *query_cursor;
        bson_t *empty_query;
        bson_error_t err;
        const bson_t *doc;
        if ((database = mongoc_client_get_database(mongo_client, config->mongo_db_name)) == NULL) {
            PRINT("pull", "Database connection error.");
            return PULL_EXIT_FAILURE;
        }
        if ((col = mongoc_client_get_collection(mongo_client, config->mongo_db_name, src_collection)) == NULL) {
            PRINT("pull", "Database collection fetch error.");
        }
        empty_query = bson_new();
        query_cursor = CUSTOM_MONGO_FIND_WITH_OPTS(col, empty_query);
        while (mongoc_cursor_next(query_cursor, &doc)) {
            char *queriedObject = bson_as_json(doc, NULL);
            bson_iter_t key;
            if (!bson_iter_init_find(&key, doc, redis_key)) {
                PRINT("pull", "Cannot find designated Redis key");
                return PULL_EXIT_FAILURE;
            }
            const char *redis_key_val = bson_iter_utf8(&key, NULL);
            redisReply *ignored = redisCommand(redis_client, "SET _syncd_pulled_%s_%s_%s %s", config->mongo_db_name,
                                               src_collection, redis_key_val, queriedObject);
            freeReplyObject(ignored);
            bson_free(queriedObject);
        }
        bson_destroy(empty_query);
        mongoc_cursor_destroy(query_cursor);
        mongoc_collection_destroy(col);
        mongoc_database_destroy(database);
    }
    return PULL_EXIT_SUCCESS;
}


int push(syncd_config_t *config, mongoc_client_t *mongo_client, redisContext *redis_client) {
    // arguments check
    if (config == NULL || redis_client == NULL || mongo_client == NULL) {
        printf("push(): ERR one or more arguments point to NULL\n");
        return EXIT_FAILURE;
    }
    // end arguments check
    redisReply *keys = redisCommand(redis_client, "KEYS *");
    int i;
    for (i = 0; i < keys->elements; i++) {
        redisReply *this = keys->element[i];
        struct vec_element_t recv;
        while (vect_iter_next(&(config->push), &recv)) {
            int type;
            const char *mongo_collection = recv.collection;
            const char *field_name = recv.misc;
            if (strcmp(this->str, field_name) == 0)
                if (strcmp(recv.redis, "kv")) {
                    type = REDIS_REPLY_STRING;
                    redisReply *obj = redisCommand(redis_client, "GET %s", this->str);
                    if (obj->type == REDIS_REPLY_STRING) {
                        // only proceed when is string.
                        const char *json = obj->str;
                        bson_t *b;
                        bson_error_t err;
                        b = bson_new_from_json(json, -1, &err);
                        if (!b) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            printf("push(): ERR malformed or unparsable json format.\n");
                            return PUSH_FAILURE;
                        }
                        bson_oid_t oid;
                        bson_oid_init(&oid, NULL);
                        BSON_APPEND_OID(b, "_id", &oid);
                        mongoc_collection_t *collection = mongoc_client_get_collection(mongo_client,
                                                                                       config->mongo_db_name,
                                                                                       mongo_collection);
                        if (collection == NULL) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            bson_destroy(b);
                            printf("push(): mongo collection creation error.\n");
                            return PUSH_FAILURE;
                        }
                        if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, b, NULL, &err)) {
                            bson_destroy(b);
                            mongoc_collection_destroy(collection);
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            printf("push: ERR document cannot be inserted into the destination collection.\n");
                            return PUSH_FAILURE;
                        }
                        // finally!
                        bson_destroy(b);
                        mongoc_collection_destroy(collection);
                        freeReplyObject(obj);
                        type = -1;
                        break; // we only have one pass per loop!
                    } else {
                        freeReplyObject(keys);
                        freeReplyObject(obj);
                        printf("push(): ERR the specified type is kv, but reply is NOT string.\n");
                        return PUSH_FAILURE;
                    }
                } else if (strcmp(recv.redis, "hashset")) {
                    type = REDIS_REPLY_ARRAY;
                    redisReply *obj = redisCommand(redis_client, "HGETALL %s", this->str);
                    if (obj->type == REDIS_REPLY_ARRAY) {
                        struct vec_t reply_vector;
                        init_vect(&reply_vector);
                        int c;
                        SYNCD_VEC_ELEMENT tbe; // collection=key,redis=value
                        for (c = 0; c < obj->elements; c++) {
                            if ((c % 2) == 0) {
                                // is key
                                tbe.collection = obj->element[c]->str;
                            } else {
                                // is value
                                tbe.redis = obj->element[c]->str;
                                SYNCD_VEC_ELEMENT e;
                                e.collection = tbe.collection;
                                e.redis = tbe.redis;
                                tbe.collection = NULL;
                                tbe.redis = NULL;
                                append_vect(&reply_vector, &e);
                            }
                        }
                        // iteratively create a BSON document.
                        bson_t b;
                        bson_error_t err;
                        bson_init(&b);
                        struct vec_element_t r;
                        while (vect_iter_next(&reply_vector, &r)) {
                            const char *key_ptr = r.collection;
                            const char *val_ptr = r.redis;
                            bson_append_utf8(&b, key_ptr, -1, val_ptr, -1);
                        }
                        bson_oid_t oid;
                        bson_oid_init(&oid, NULL);
                        BSON_APPEND_OID(&b, "_id", &oid);
                        // insert!!
                        mongoc_collection_t *collection = mongoc_client_get_collection(mongo_client,
                                                                                       config->mongo_db_name,
                                                                                       mongo_collection);
                        if (collection == NULL) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            bson_destroy(&b);
                            vect_incomplete_free(&reply_vector);
                            printf("push(): mongo collection creation error.\n");
                            return PUSH_FAILURE;
                        }
                        if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, &b, NULL, &err)) {
                            bson_destroy(&b);
                            mongoc_collection_destroy(collection);
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            vect_incomplete_free(&reply_vector);
                            printf("push(): ERR document cannot be inserted into the destination collection.\n");
                            return PUSH_FAILURE;
                        }
                        // finally
                        bson_destroy(&b);
                        mongoc_collection_destroy(collection);
                        freeReplyObject(obj);
                        vect_incomplete_free(&reply_vector);
                    } else {
                        freeReplyObject(keys);
                        freeReplyObject(obj);
                        printf("push(): ERR the specified type is hashset, but the reply is NOT hashset.\n");
                        return PUSH_FAILURE;
                    }
                } else if (strcmp(recv.redis, "list")) {
                    type = REDIS_REPLY_ARRAY;
                    redisReply *obj = redisCommand(redis_client, "LRANGE %s 0 -1", this->str);
                    if (obj->type == REDIS_REPLY_ARRAY) {
                        mongoc_collection_t *collection = mongoc_client_get_collection(mongo_client,
                                                                                       config->mongo_db_name,
                                                                                       mongo_collection);
                        if (collection == NULL) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            printf("push(): mongo collection creation error.\n");
                            return PUSH_FAILURE;
                        }

                        int k;
                        for (k = 0; k < obj->elements; k++) {
                            bson_error_t err;
                            bson_t *b = bson_new_from_json(obj->element[k]->str, -1, &err);
                            bson_append_int32(b, "redis_list_position", -1, k);
                            if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, b, NULL, &err)) {
                                bson_destroy(b);
                                mongoc_collection_destroy(collection);
                                freeReplyObject(keys);
                                freeReplyObject(obj);
                                printf("push(): mongo collection insertion error.\n");
                                return PUSH_FAILURE;
                            }
                            bson_destroy(b);
                        }
                        // finally
                        freeReplyObject(obj);
                        mongoc_collection_destroy(collection);
                    } else {
                        freeReplyObject(keys);
                        freeReplyObject(obj);
                        printf("push(): ERR the specified type is list, but the reply is NOT list.\n");
                        return PUSH_FAILURE;
                    }
                } else {
                    type = -1;
                    freeReplyObject(keys);
                    printf("push(): invalid push data type field. Exiting...");
                    return PUSH_FAILURE;
                }

        }
    }
    freeReplyObject(keys);
    return EXIT_SUCCESS;
}

struct syncd_thread_args {
    syncd_config_t config;
    redisContext *redis_client;
    mongoc_client_t *mongo_client;
};

void *async_push_worker(void *data) {
    struct syncd_thread_args *args = data;
    syncd_config_t config = args->config;
    redisContext *redis_client = args->redis_client;
    mongoc_client_t *mongo_client = args->mongo_client;
    push(&config, mongo_client, redis_client);
    return NULL;
}

void *async_pull_worker(void *data) {
    struct syncd_thread_args *args = data;
    syncd_config_t config = args->config;
    redisContext *redis_client = args->redis_client;
    mongoc_client_t *mongo_client = args->mongo_client;
    pull(&config, mongo_client, redis_client);
    return NULL;
}

int do_async_push(syncd_config_t config) {
    redisContext *redis_client;
    redis_client = redisConnect(config.mongo_address, config.redis_port);
    if (redis_client == NULL) {
        PRINT("do_async_push", "redis client init failure\n");
        return GENERAL_FAILURE;
    } else if (redis_client->err) {
        PRINT("do_async_push", "redis client init failure\n");
        return GENERAL_FAILURE;
    }
    mongoc_client_t *mongo_client = mongoc_client_new(config.mongo_address);
    if (mongo_client == NULL) {
        PRINT("do_async_push", "mongo client init failure\n");
        return GENERAL_FAILURE;
    }
    pthread_t apt;
    // in case we're not passing along a static struct
    static syncd_config_t passed_config;
    passed_config.push = config.push;
    passed_config.pull = config.pull;
    passed_config.mongo_address = config.mongo_address;
    passed_config.redis_port = config.redis_port;
    passed_config.redis_address = config.redis_address;
    passed_config.mongo_port = config.mongo_port;
    passed_config.address = config.address;
    passed_config.port = config.port;
    passed_config.mongo_db_name = config.mongo_db_name;
    struct syncd_thread_args aargs = {.redis_client = redis_client,
            .mongo_client = mongo_client,
            .config = config};
    pthread_create(&apt, NULL, async_push_worker, &aargs);
    // finally
    redisFree(redis_client);
    mongoc_client_destroy(mongo_client);
    mongoc_cleanup();
    return GENERAL_SUCCESS;
}

int do_async_pull(syncd_config_t config) {
    redisContext *redis_client;
    redis_client = redisConnect(config.mongo_address, config.redis_port);
    if (redis_client == NULL) {
        PRINT("do_async_push", "redis client init failure\n");
        return GENERAL_FAILURE;
    } else if (redis_client->err) {
        PRINT("do_async_push", "redis client init failure\n");
        return GENERAL_FAILURE;
    }
    mongoc_client_t *mongo_client = mongoc_client_new(config.mongo_address);
    if (mongo_client == NULL) {
        PRINT("do_async_push", "mongo client init failure\n");
        return GENERAL_FAILURE;
    }
    pthread_t apt;
    // in case we're not passing along a static struct
    static syncd_config_t passed_config;
    passed_config.push = config.push;
    passed_config.pull = config.pull;
    passed_config.mongo_address = config.mongo_address;
    passed_config.redis_port = config.redis_port;
    passed_config.redis_address = config.redis_address;
    passed_config.mongo_port = config.mongo_port;
    passed_config.address = config.address;
    passed_config.port = config.port;
    passed_config.mongo_db_name = config.mongo_db_name;
    struct syncd_thread_args aargs = {.redis_client = redis_client,
            .mongo_client = mongo_client,
            .config = config};
    pthread_create(&apt, NULL, async_pull_worker, &aargs);
    // finally
    redisFree(redis_client);
    mongoc_client_destroy(mongo_client);
    mongoc_cleanup();
    return GENERAL_SUCCESS;
}