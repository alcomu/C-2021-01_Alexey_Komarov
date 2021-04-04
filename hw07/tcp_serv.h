#pragma once

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <pthread.h>

#define t_error(...)                                                                               \
    {                                                                                              \
        fprintf(stderr, __VA_ARGS__);                                                              \
        fflush(stderr);                                                                            \
        exit(1);                                                                                   \
    }

typedef struct TcpServArg_ {
    const char *bind_addr;
    int bind_port;
    pthread_mutex_t *dlock;
    uint64_t *data;
} TcpServArg;

void * tcp_serv(void *arg);