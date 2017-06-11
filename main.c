#include <stdio.h>
#include <stdlib.h>
#include "parseconfig.h"
#include "configobject.h"
#include "actors.h"
#include "net.h"
#include <pthread.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
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
/*
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
*/
int main(int argc, char ** argv) {
    if(argc!=2) return EXIT_FAILURE;
    // parse
    static syncd_config_t config;
    int r = parse_config(argv[1], &config);
    if(r == PARSECONFIG_PARSE_FAILURE) {
        printf("main(): config file parse failure. Exiting...");
        return EXIT_FAILURE;
    }
    pthread_t net_thrd;
    pthread_create(&net_thrd, NULL, &start_net_handler, &config);
    while (1) {
        // begin looping
        sleep(300); // sleep for 5 mins
        do_async_push(config);
        do_async_pull(config);
    }
    return EXIT_SUCCESS;
}
#pragma clang diagnostic pop