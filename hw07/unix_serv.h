#pragma once

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <pthread.h>

#define u_error(...)                                                                               \
    {                                                                                              \
        fprintf(stderr, __VA_ARGS__);                                                              \
        fflush(stderr);                                                                            \
        exit(1);                                                                                   \
    }

typedef struct UnixServArg_ {
    const char *name_sock;
    pthread_mutex_t *dlock;
    uint64_t *data;
} UnixServArg;

void * unix_serv(void *arg);