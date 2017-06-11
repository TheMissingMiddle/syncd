//
// Created by alex on 6/8/17.
//

#include "net.h"
#include "actors.h"
#include "configobject.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <pthread.h>
#define BACKLOG 50
#define PRINT(func, x) printf("%s(): Err: %s\n", func, x)

 syncd_config_t _config;

void * handle(void * data) {
    int connfd = *(int*)data;
    int str_len, n;
    char *str_buffer;
    if(read(connfd, &str_len, sizeof(int))<0) {
        close(connfd);
        return NULL;
    }
    printf("net_handler(): connection established. the len of string is %d\n", str_len);
    str_buffer = malloc(sizeof(char) * (str_len + 1));
    n = read(connfd, str_buffer, sizeof(char) * str_len);
    if (n < 0) {
        PRINT("net_handler", "recv str error\n");
        free(data);
        free(str_buffer);
        close(connfd);
        return NULL;
    }
    str_buffer[str_len] = '\0'; // str_len+1th place, start from 0
    if(strcmp("push", str_buffer) == 0) {
        char * s= "ok:push";
        printf("OK:PUSH\n");
        int l = strlen(s);
        write(connfd, &l, sizeof(int));
        write(connfd, s, l);
        // TODO push
        do_async_push(_config); // TODO config
    } else if (strcmp("pull", str_buffer) == 0) {
        printf("OK:PULL\n");
        char * s = "ok:pull";
        int l = strlen(s);
        write(connfd, &l, sizeof(int));
        write(connfd, s, l);
        // TODO pull
        do_async_pull(_config); // TODO CONFIG
    } else {
        PRINT("net_handler", "err - the request isn't valid.");
        printf("\tdebug info: %s", str_buffer);

    }
    free(data);
    free(str_buffer);
    close(connfd);
    return NULL;
}

int net_handler(syncd_config_t *config) {
    _config.pull = config->pull;
    _config.push = config->push;
    _config.mongo_db_name = config->mongo_db_name;
    _config.address = config->address;
    _config.redis_port = config->redis_port;
    _config.redis_address = config->redis_address;
    _config.mongo_port = config->mongo_port;
    _config.mongo_address = config->mongo_address;
    const char *bind_addr = config->address;
    int bind_port = config->port;
    int sockfd, connfd;
    socklen_t cli_len;
    struct sockaddr_in server_addr, client_addr;
    ssize_t n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        PRINT("net_handler", "Socket creation Error");
        return NET_FAILURE;
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t) bind_port);
    server_addr.sin_family = AF_INET;
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        PRINT("net_handler", "bind error");
        return NET_FAILURE;
    }
    listen(sockfd, BACKLOG);
    cli_len = sizeof(client_addr);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while(1) {
        connfd = accept(sockfd, (struct sockaddr *) &client_addr, &cli_len);
        pthread_t connthrd;
        int * s = malloc(sizeof(int));
        *s = connfd;
        pthread_create(&connthrd, NULL, handle, s);
    }
    return NET_SUCCESS;
#pragma clang diagnostic pop
}

void * start_net_handler(void *d) {
    int i = net_handler(d);
    if(i == NET_FAILURE) {
        printf("start_net_handler(): unknown error occured. failred to start network handler\n");
        return NULL;
    }
}