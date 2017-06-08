//
// Created by alex on 6/8/17.
//

#include "pullactions.h"

#define PRINT(fn, x) printf("%s(): ERR : %s\n", fn, x)
#define IS_VALID_CHAR(c) (c>=97&&c<=122)||(c>=65&&c<=90)||(c>=48&&c<=57) /* (is lower case) || (is upper case) || (is digit) */
#define CUSTOM_MONGO_FIND_WITH_OPTS(collection, query) mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL)


int pull(syncd_config_t *config, redisContext *redis_client, mongoc_client_t *mongo_client) {
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