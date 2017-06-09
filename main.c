#include <stdio.h>
#include <stdlib.h>
#include "parseconfig.h"
#include "configobject.h"
#include "actors.h"
#include "net.h"

void testConfigReader() {
    static syncd_config_t config;
    printf("test(): return result: %d\n", parse_config("/home/alex/Projects/magicsync/sync-config.json", &config));
    printf("config: addr %s\tport %d\tDB Name %s\nPull config:\n", config.address, config.port, config.mongo_db_name);
    struct vec_element_t recv;
    while(vect_iter_next(&(config.push), &recv)) {
        printf("\tCollection: %s\n\tRedis: %s\n\tMisc: %s\n", recv.collection, recv.redis, recv.misc);
    }
    printf("Pull config:\n");
    while(vect_iter_next(&(config.pull), &recv)) {
        printf("\tCollection:%s\n\tRedis:%s\n\tMisc:%s\n", recv.collection, recv.redis, recv.misc);
    }
    config_destroy(&config);
}

void testNet() {
    static syncd_config_t config;
    parse_config("/home/alex/Projects/magicsync/sync-config.json", &config);
    net_handler(&config);
    config_destroy(&config);
}

void testRedisErr() {
    init_redis("127.0.0.1", 6379);
    if(redis_client==NULL) return ;
    redisReply * reply = redisCommand(redis_client,"HGETALL sample_hash_set");
    printf("result: %s", reply->str);
    if(reply->type == REDIS_REPLY_ARRAY) printf("yes!!\n");
    freeReplyObject(reply);
    cleanup_redis();
    return;
}

int main() {
   // testNet();
    testRedisErr();
    return 0;
}