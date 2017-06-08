//
// Created by alex on 6/6/17.
//

#include "parseconfig.h"
#include "configobject.h"
#include <stdio.h>
#include <bson.h>

#define IS_BSON_SUBDOCUMENT(itr_ref, itr_child_ref, doc, name) \
    (bson_iter_init_find(itr_ref, doc, name) && \
    bson_iter_recurse(itr_ref, itr_child_ref))

static bson_iter_t bgetf(const bson_t *b, char *key) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, b) && bson_iter_find(&iter, key)) {
        return iter;
    } else {
        printf("get_field_not_array: input invalid!!\n");
        iter.len = 0;
        iter.key = 0;
        return iter;

    }
}

int parse_config(const char *config_file_path, syncd_config_t * config) {
    printf("parse_config(): initializing vairables...");
    bson_t *bs;
    bson_error_t err;
    bson_iter_t port,
            address,
            mongo_db_name,
            push_actions,
            push_child,
            pull_actions,
            pull_child;
    const char *str_address,
            *str_mongo_db_name;
    int int_port = -1;
    FILE *fp;
    char c, *buf;
    int t = 1;
    if ((fp = fopen(config_file_path, "r")) == NULL) return PARSECONFIG_PARSE_FAILURE;
    while ((c = fgetc(fp)) != EOF)++t;
    rewind(fp);
    buf = malloc(sizeof(char) * t);
    c = '\0';
    t = 0;
    while ((c = fgetc(fp)) != EOF) buf[t++] = c;
    buf[t] = '\0';
    bs = bson_new_from_json(buf, -1, &err);
    if (bs == NULL) {
        printf("parse_config(): Config file parse failure: %s.", err.message);
        free(buf);
        bson_destroy(bs);
        return PARSECONFIG_PARSE_FAILURE;
    }
    port = bgetf(bs, "port");
    address = bgetf(bs, "address");
    mongo_db_name = bgetf(bs, "mongo-db-name");
    if ((address.len && mongo_db_name.len) == 0) {
        printf("parse_config(): Parsing failure. Some fields are not specified.");
        free(buf);
        bson_destroy(bs);
        return PARSECONFIG_PARSE_FAILURE;

    }
    int_port = bson_iter_int32(&port);
    str_address = bson_iter_utf8(&address, NULL);
    str_mongo_db_name = bson_iter_utf8(&mongo_db_name, NULL);
    if (int_port == -1 || str_address == NULL || str_mongo_db_name == NULL) {
        printf("parse_config(): Parsing failure. Some fields are NULL!!");
        free(buf);
        bson_destroy(bs);
        return PARSECONFIG_PARSE_FAILURE;
    }
    // parsing arrays, push & pull
    // do sub-document parsing
    SYNCD_GLOBAL_VECTOR push_vector;
    SYNCD_GLOBAL_VECTOR pull_vector;
    init_vect(&push_vector);
    init_vect(&pull_vector);
    if (IS_BSON_SUBDOCUMENT(&push_actions, &push_child, bs, "push")) {
        unsigned int counter = 0;
        while (bson_iter_next(&push_child)) {
            // do parsing of each array elements (3rd-level sub-document)
            bson_iter_t array_element;
            if (bson_iter_recurse(&push_child, &array_element)) {
                const char *collection = NULL;
                const char *data_type = NULL;
                const char *misc = NULL;
                while (bson_iter_next(&array_element)) {
                    if (strcmp(bson_iter_key(&array_element), "collection") == 0) {
                        collection = bson_iter_utf8(&array_element, NULL);
                    } else if (strcmp(bson_iter_key(&array_element), "redis-data-type") == 0) {
                        data_type = bson_iter_utf8(&array_element, NULL);
                    } else if (strcmp(bson_iter_key(&array_element), "misc") == 0) {
                        misc = bson_iter_utf8(&array_element, NULL);
                    } else {
                        free(buf);
                        bson_destroy(bs);
                        printf("parse_config(): invalid fields in array sub-document. Exiting...");
                        return PARSECONFIG_PARSE_FAILURE;
                    }
                }
                if (collection == NULL || data_type == NULL || misc == NULL) {
                    free(buf);
                    bson_destroy(bs);
                    printf("parse_config(): array sub-document contains blank or incorrect fields. Exiting...");
                    return PARSECONFIG_PARSE_FAILURE;
                }
                SYNCD_VEC_ELEMENT this_element;
                init_element(&this_element,collection,data_type,misc);
                append_vect(&push_vector, &this_element);
            } else {
                free(buf);
                bson_destroy(bs);
                printf("parse_config(): push definition array sub-document is blank. Exiting...\n");
                return PARSECONFIG_PARSE_FAILURE;
            }
        }
    } else {
        printf("parse_config(): Parsing failure. \"push\" field is not collection valid sub-document.");
        free(buf);
        bson_destroy(bs);
        return PARSECONFIG_PARSE_FAILURE;
    }
    if (IS_BSON_SUBDOCUMENT(&pull_actions, &pull_child, bs, "pull")) {
        while (bson_iter_next(&pull_child)) {
            // do parsing of each array elements (3rd-level sub-document)
            bson_iter_t array_element;
            if (bson_iter_recurse(&pull_child, &array_element)) {
                const char *collection = NULL;
                const char *redis_key_field = NULL;
                const char *misc = NULL;
                while (bson_iter_next(&array_element)) {
                    if (strcmp(bson_iter_key(&array_element), "collection") == 0) {
                        collection = bson_iter_utf8(&array_element, NULL);
                    } else if (strcmp(bson_iter_key(&array_element), "redis-key-field") == 0) {
                        redis_key_field = bson_iter_utf8(&array_element, NULL);
                    } else if (strcmp(bson_iter_key(&array_element), "misc") == 0) {
                        misc = bson_iter_utf8(&array_element, NULL);
                    } else {
                        free(buf);
                        bson_destroy(bs);
                        printf("parse_config(): invalid fields in array sub-document. Exiting...");
                        return PARSECONFIG_PARSE_FAILURE;
                    }
                }
                if (collection == NULL || redis_key_field == NULL || misc == NULL) {
                    free(buf);
                    bson_destroy(bs);
                    printf("parse_config(): array sub-document contains blank or incorrect fields. Exiting...");
                    return PARSECONFIG_PARSE_FAILURE;
                }
                SYNCD_VEC_ELEMENT this_element;
                init_element(&this_element, collection, redis_key_field, misc);
                append_vect(&pull_vector, &this_element);
            } else {
                free(buf);
                bson_destroy(bs);
                printf("parse_config(): pull definition array sub-document is blank. Exiting...\n");
                return PARSECONFIG_PARSE_FAILURE;
            }
        }
    } else {
        printf("parse_config(): Parsing failure. \"pull\" field is not collection valid sub-document.");
        free(buf);
        bson_destroy(bs);
        return PARSECONFIG_PARSE_FAILURE;
    }
    config->address = str_address;
    config->mongo_db_name = str_mongo_db_name;
    config->port = int_port;
    config->push = push_vector;
    config->pull = pull_vector;
    free(buf);
    bson_destroy(bs);

    return 0;
}

void config_destroy(syncd_config_t * config) {
    config->port = -1;
    config->push.len = 0;
    config->pull.len = 0;
    bson_free((char *)config->address);
    bson_free((char *)config->mongo_db_name);
    int i;
    for(i = 0; i <= config->push.ptr; i++) {
        bson_free((char *)config->push.elements[i].redis);
        bson_free((char *)config->push.elements[i].collection);
        bson_free((char *)config->push.elements[i].misc);
    }
    for(i = 0; i <= config->pull.ptr; i++) {
        bson_free((char *)config->pull.elements[i].redis);
        bson_free((char *)config->pull.elements[i].collection);
        bson_free((char *)config->pull.elements[i].misc);
    }
}