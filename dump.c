
int alex_redis_connect(const char *ip, int port) {
    redisContext * connect = redisConnect(ip, port);
    if(connect==NULL||connect->err) {
        if(connect)
            printf("Redis Error! %s", connect->errstr);
        return REDIS_CONNECT_FAILURE;
    }
    redisCommand(connect, "AUTH foobar");
    redisCommand(connect, "SET test test123");
    redisReply * reply = redisCommand(connect, "KEYS *");
    int i;
    for(i = 0; i < reply->elements; ++i) {
        redisReply *singleReplyElement = reply->element[i];
        printf("Redis reply %d, value %s\n", i, singleReplyElement->str);
    }
    redisReply * queryReply1 = redisCommand(connect, "GET test");
    printf("Redis reply: %s\n", queryReply1->str);
}
int alex_mongo_connect(const char * mongodb_address, const char * mongodb_database_name) {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_t *command, reply, *insert;
    bson_error_t err;
    char *str;
    bool returnvalue;
    mongoc_init();
    if((client = mongoc_client_new(mongodb_address))==NULL) return MONGO_CONNECT_FAILURE;
    if((database = mongoc_client_get_database(client, mongodb_database_name))==NULL) return MONGO_GETDATABASE_FAILURE;
    collection = mongoc_client_get_collection (client, "db_name", "coll_name");
    command = BCON_NEW("ping", BCON_INT32(1));
    returnvalue = mongoc_client_command_simple(client, "admin", command, NULL, &reply, &err);
    if (!returnvalue) {
        fprintf (stderr, "%s\n", err.message);
        return MONGO_EXEC_FAILURE;
    }
    str = bson_as_json(&reply, NULL);
    printf("Mongo reply: %s\n", str);

    insert = BCON_NEW ("hello", BCON_UTF8 ("world"));

    if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, insert, NULL, &err)) {
        fprintf (stderr, "%s\n", err.message);
    }

    bson_destroy (insert);
    bson_destroy (&reply);
    bson_destroy (command);
    bson_free (str);
    mongoc_collection_destroy (collection);
    mongoc_database_destroy (database);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
    return 0;
}

//printf("Testing mongodb connectivity\n");
//alex_mongo_connect("mongodb://localhost:27017", "test");
//alex_redis_connect("127.0.0.1", 6379);