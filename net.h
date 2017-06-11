//
// Created by alex on 6/8/17.
//

#include "configobject.h"

#ifndef MAGICSYNC_NET_H
#define MAGICSYNC_NET_H

#endif //MAGICSYNC_NET_H
#define NET_SUCCESS 1

int net_handler(syncd_config_t *config);

void *start_net_handler(void *d);

#define NET_FAILURE 0