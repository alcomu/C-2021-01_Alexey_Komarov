#pragma once


#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define COMM_INFO_SIZE 1024

typedef struct Port_ {
    int p1;
    int p2;
} Port;

typedef struct State_ {
    // Response message to client e.g. 220 Welcome
    char *message;
    // Commander connection
    int conn;
    // Socket for passive connection (must be accepted later)
    int sock_pasv;
} State;

typedef struct Command_ {
    char command[5];
    char arg[COMM_INFO_SIZE];
} Command;

typedef enum comm_list_ { USER, PASS, PWD, CWD, LIST, PASV, QUIT, RETR } comm_list;

int create_socket(int port);
int accept_connection(int socket);

void parse_comm(char *, Command *);

// Commands handle functions
void response(Command *, State *);

void ftp_user(State *);
void ftp_pass(State *);
void ftp_pwd(State *);
void ftp_cwd(Command *, State *);
void ftp_pasv(State *);
void ftp_list(Command *, State *);
void ftp_retr(Command *, State *);
void ftp_quit(State *);
