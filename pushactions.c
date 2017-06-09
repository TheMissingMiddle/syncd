//
// Created by alex on 6/8/17.
//

#include "pushactions.h"
#include "configobject.h"
#include "parseconfig.h"
#include <libbson-1.0/bson.h>
#include <libbson-1.0/bcon.h>
int push(syncd_config_t *config, redisContext *redis_client, mongoc_client_t *mongoc_client) {
    // arguments check
    if(config==NULL || redis_client==NULL||mongoc_client==NULL) {
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
                    redisReply * obj = redisCommand(redis_client, "GET %s", this->str);
                    if(obj->type == REDIS_REPLY_STRING) {
                        // only proceed when is string.
                        const char * json = obj->str;
                        bson_t * b;
                        bson_error_t err;
                        b = bson_new_from_json(json, -1, &err);
                        if(!b) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            printf("push(): ERR malformed or unparsable json format.\n");
                            return PUSH_FAILURE;
                        }
                        bson_oid_t oid;
                        bson_oid_init(&oid, NULL);
                        BSON_APPEND_OID(b, "_id", &oid);
                        mongoc_collection_t * collection = mongoc_client_get_collection(mongoc_client, config->mongo_db_name, mongo_collection);
                        if(collection == NULL) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            bson_destroy(b);
                            printf("push(): mongo collection creation error.\n");
                            return PUSH_FAILURE;
                        }
                        if(!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, b, NULL, &err)) {
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
                    redisReply * obj = redisCommand(redis_client, "HGETALL %s", this->str);
                    if(obj->type == REDIS_REPLY_ARRAY) {
                        struct vec_t reply_vector;
                        init_vect(&reply_vector);
                        int c;
                        SYNCD_VEC_ELEMENT tbe; // collection=key,redis=value
                        for( c = 0; c < obj->elements; c++ ) {
                            if((c % 2) == 0) {
                                // is key
                                tbe.collection =obj->element[c]->str;
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
                        bson_t  b;
                        bson_error_t err;
                        bson_init(&b);
                        struct vec_element_t r;
                        while(vect_iter_next(&reply_vector, &r)) {
                            const char * key_ptr = r.collection;
                            const char * val_ptr = r.redis;
                            bson_append_utf8(&b, key_ptr, -1, val_ptr, -1);
                        }
                        bson_oid_t oid;
                        bson_oid_init(&oid, NULL);
                        BSON_APPEND_OID(&b, "_id", &oid);
                        // insert!!
                        mongoc_collection_t * collection = mongoc_client_get_collection(mongoc_client, config->mongo_db_name, mongo_collection);
                        if(collection == NULL) {
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            bson_destroy(&b);
                            vect_incomplete_free(&reply_vector);
                            printf("push(): mongo collection creation error.\n");
                            return PUSH_FAILURE;
                        }
                        if(!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, &b, NULL, &err)) {
                            bson_destroy(&b);
                            mongoc_collection_destroy(collection);
                            freeReplyObject(keys);
                            freeReplyObject(obj);
                            vect_incomplete_free(&reply_vector);
                            printf("push: ERR document cannot be inserted into the destination collection.\n");
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
                        printf("push(); ERR the specified type is hashset, but the reply is NOT hashset.\n");
                        return PUSH_FAILURE;
                    }
                } else if (strcmp(recv.redis, "list")) {
                    type = REDIS_REPLY_ARRAY;
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