//
// Created by alex on 6/8/17.
//

#include "pushactions.h"
#include "configobject.h"

int push(syncd_config_t *config, redisContext * redis_client, mongoc_client_t * mongoc_client) {
    char * msg_directive = "_syncd_push_msg_";
    redisReply * keys = redisCommand(redis_client, "KEYS *");
    int i;
    for(i = 0; i < keys->elements; i++) {
        redisReply * this = keys->element[i];
        struct vec_element_t recv;
        while(vect_iter_next(&(config->push), &recv)) {
            int type;
            if(strcmp(recv.redis, "kv")) {
                type = REDIS_REPLY_STRING;
            } else if (strcmp(recv.redis, "hashset")) {
                type = REDIS_REPLY_ARRAY;
            } else if (strcmp(recv.redis, "list")) {
                type = REDIS_REPLY_ARRAY;
            } else {
                printf("push(): invalid push data type field. Exiting...");
                return PUSH_FAILURE;
            }

        }
    }
}