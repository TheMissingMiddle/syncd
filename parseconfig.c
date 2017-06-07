//
// Created by alex on 6/6/17.
//

#include "parseconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <bson.h>

#define IS_BSON_SUBDOCUMENT(itr_ref, itr_child_ref, doc, name) \
    (bson_iter_init_find(itr_ref, doc, name) && \
    bson_iter_recurse(itr_ref, itr_child_ref))

static bson_iter_t getf(const bson_t *b, char *key) {
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

int parse_config(const char *config_file_path) {
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

    printf("parse_config(): reading in the document...");
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
    printf("parse_config(): read in document raw format\n");
    printf("%s\n", buf);
    printf("parse_config(): read in document BSON format\n");
    printf("%s\n\n", bson_as_json(bs, NULL));
    printf("parse_config(): parsing each document");
    port = getf(bs, "port");
    address = getf(bs, "address");
    mongo_db_name = getf(bs, "mongo-db-name");
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
    printf("parse_config(): string parsing results: %d, %s, %s\n", int_port, str_address, str_mongo_db_name);
    printf("parse_config(): string parsing finished, parsing arrays push/pull.\n");
    // parsing arrays, push & pull
    // do sub-document parsing
    if (IS_BSON_SUBDOCUMENT(&push_actions, &push_child, bs, "push")) {
        printf("push: VOILA!!\n");
        unsigned int counter = 0;
        while (bson_iter_next(&push_child)) {
            // do parsing of each array elements (3rd-level sub-document)
            bson_iter_t array_element;
            char str_counter[8];
            if (sprintf(str_counter, "%d", counter) > 0) {
                if (IS_BSON_SUBDOCUMENT(&push_child, &array_element, bs, str_counter)) {

                } else {
                    free(buf);
                    bson_destroy(bs);
                    printf("parse_config(): push definition array sub-document is blank. Exiting...\n");
                    return PARSECONFIG_PARSE_FAILURE;
                }
            } else {
                free(buf);
                bson_destroy(bs);
                printf("parse_config(): wierd sprintf() error... Idk why... panicking...\n");
                return PARSECONFIG_PARSE_FAILURE;
            }
        }
    } else {
        printf("parse_config(): Parsing failure. \"push\" field is not a valid sub-document.");
        free(buf);
        bson_destroy(bs);
        return PARSECONFIG_PARSE_FAILURE;
    }
    free(buf);
    bson_destroy(bs);
    return 0;
}
