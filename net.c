//
// Created by alex on 6/8/17.
//

#include "net.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>

#define BACKLOG 50
#define PRINT(func, x) printf("%s(): Err: %s\n", func, x)

int net_handler(syncd_config_t *config) {
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
    while (1) {
        printf("net_handler(): waiting for connection...\n");
        connfd = accept(sockfd, (struct sockaddr *) &client_addr, &cli_len);
        while (1) {
            int str_len;
            char *str_buffer;
            if(read(connfd, &str_len, sizeof(int))<0) {
                break;
            }
            printf("net_handler(): connection established. the len of string is %d\n", str_len);
            str_buffer = malloc(sizeof(char) * (str_len + 1));
            n = read(connfd, str_buffer, sizeof(char) * str_len);
            if (n < 0) {
                PRINT("net_handler", "recv str error\n");
                free(str_buffer);
                break;
            }
            str_buffer[str_len] = '\0'; // str_len+1th place, start from 0
            if(strcmp("push", str_buffer) == 0) {
                char * s= "ok:push";
                printf("OK:PUSH\n");
                int l = strlen(s);
                write(connfd, &l, sizeof(int));
                write(connfd, s, l);
                // TODO push

                break;
            } else if (strcmp("pull", str_buffer) == 0) {
                printf("OK:PULL\n");
                char * s = "ok:pull";
                int l = strlen(s);
                write(connfd, &l, sizeof(int));
                write(connfd, s, l);
                // TODO pull

                break;
            } else {
                PRINT("net_handler", "err - the request isn't valid.");
                printf("\tdebug info: %s", str_buffer);
                free(str_buffer);
                break;
            }
        }
        close(connfd);
    }
    return NET_SUCCESS;
#pragma clang diagnostic pop
}