//
// Created by alex on 6/6/17.
//

#ifndef MAGICSYNC_PARSECONFIG_H
#define MAGICSYNC_PARSECONFIG_H

#include "configobject.h"

#define PARSECONFIG_PARSE_FAILURE 10
int parse_config(const char * path, syncd_config_t *config);
void config_destroy(syncd_config_t * config);
#endif //MAGICSYNC_PARSECONFIG_H
