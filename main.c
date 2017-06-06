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
mongoc_client_t *mongo_client = NULL;
redisContext *redis_client = NULL;

int init_redis(const char *redis_address, int redis_port) {
    redis_client = redisConnect(redis_address, redis_port);
    if (connect == NULL) {
        return REDIS_CONNECT_FAILURE;
    } else if (redis_client->err) {
        printf("Redis Connect Error: %s\n", redis_client->errstr);
        return REDIS_CONNECT_FAILURE;
    }
    redisCommand(redis_client, "AUTH foobar");
    return REDIS_CONNECT_SUCCESS;
}

void cleanup_redis() {
    redisFree(redis_client);
}

int init_mongo(const char *mongodb_address) {
    if ((mongo_client = mongoc_client_new(mongodb_address)) == NULL) {
        printf("mongo init failed\n");
        return MONGO_CONNECT_FAILURE;
    }
    return MONGO_CONNECT_SUCCESS;
}

void cleanup_mongo() {
    mongoc_client_destroy(mongo_client);
    mongoc_cleanup();
}

bson_iter_t get_field_not_array(const bson_t *b, char *key) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, b) && bson_iter_find(&iter, key)) {
        return iter;
    } else {
        printf("get_field_not_array: input invalid!!\n");
        return iter;
    }
}

int cache_all_users() {
    mongoc_database_t *tmm;
    mongoc_collection_t *user;
    mongoc_cursor_t *query_cursor;
    bson_t *empty_query;
    bson_error_t err;
    const bson_t *doc;
    printf(" - get database\n");
    if ((tmm = mongoc_client_get_database(mongo_client, "tmmbackend")) == NULL) return MONGO_GETDATABASE_FAILURE;
    printf(" - get collection\n");
    if ((user = mongoc_client_get_collection(mongo_client, "tmmbackend", "users")) == NULL)
        return MONGO_GETDATABASE_FAILURE;
    // specify empty doc
    empty_query = bson_new();
    printf(" - query\n");
    query_cursor = CUSTOM_MONGO_FIND_WITH_OPTS(user, empty_query);
    while (mongoc_cursor_next(query_cursor, &doc)) {
        printf("printing next...\n");
        char *queriedObject = bson_as_json(doc, NULL);
        printf("%s\n", queriedObject);
        bson_iter_t email_iter = get_field_not_array(doc, "email");
        const char *emailKey = bson_iter_key(&email_iter);
        const char *emailVal = bson_iter_utf8(&email_iter, NULL);
        printf("\tK/v result: %s / %s \n", emailKey, emailVal);
        redisReply *reply = redisCommand(redis_client, "SET _syncd_pulled_tmmbackend_users_%s %s", emailVal,
                                         queriedObject);
        freeReplyObject(reply);
        bson_free(queriedObject);
    }
    bson_destroy(empty_query);
    mongoc_cursor_destroy(query_cursor);
    mongoc_collection_destroy(user);
    mongoc_database_destroy(tmm);
}

int fetch_messages(char * collection_name, char * database_name) {
    char * msg_directive = "_syncd_push_msg";
    redisReply * keys = redisCommand(redis_client, "KEYS *"); // query all
    int i;
    for(i = 0; i < keys->elements; ++i) {
        redisReply *singleReplyElement = keys->element[i];
        if(strstr(singleReplyElement->str, msg_directive)!=NULL) { // we have what we want!!
            char * msg_key = singleReplyElement->str;
            redisReply * list = redisCommand(redis_client, "LRANGE %s 0 -1", msg_key);
            if(list->type == REDIS_REPLY_ARRAY) {
                int j;
                for (j = 0; j < list->elements; ++j) {
                    const char * msg_json = list->element[j]->str;
                    bson_t * b;
                    bson_error_t err;
                    b = bson_new_from_json(msg_json, -1, &err);
                    if(!b) {
                        printf("bson: json parse error! Message is %s\n", err);
                        return GENERAL_FAILURE;
                    }
                    printf("calling mongo...");
                    bson_oid_t oid;
                    bson_oid_init(&oid, NULL);
                    BSON_APPEND_OID(b, "_id", &oid);
                    mongoc_collection_t * collection = mongoc_client_get_collection(mongo_client, database_name, collection_name);
                    if(collection==NULL) {
                        printf("fetch_messages(): err: collection is NULL");
                        bson_destroy(b);
                        return GENERAL_FAILURE;
                    }
                    if(!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, b, NULL, &err)) {
                        printf("fetch_messages(): err: %s", err);
                        return GENERAL_FAILURE;
                    }
                    bson_destroy(b);
                    mongoc_collection_destroy(collection);
                    return GENERAL_SUCCESS;
                }
            }
        }
    }

    freeReplyObject(keys); // we only need to free once (keys * command)
}

int main() {
    printf("mongo init\n");
    if (init_mongo("mongodb://localhost:27017") == MONGO_CONNECT_SUCCESS) {
        printf("catching documents\n");
        if (init_redis("127.0.0.1", 6379) == REDIS_CONNECT_SUCCESS) {
            while (1) {
                sleep(300); // sleep for 5 mins
                cache_all_users();
            }
        } else {
            printf("redis init failure!");
        }
    } else {
        printf("mongo init failure!");
    }


    return 0;
}