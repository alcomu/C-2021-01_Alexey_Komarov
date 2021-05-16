#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "ftp.h"


#define MAX_POOL_SIZE 16

int pids[MAX_POOL_SIZE];
static int pool_size = 1;

static char *welcome_msg = "Welcome to FTP server!!!";


void close_serv() {
    int i;

    for (i = 0; i < pool_size; i++)
        kill(pids[i], SIGKILL);

    exit(0);
}

int main(int argc, char **argv) {
    int i, res_fork;
    int serv_fd, cli_fd, bytes;
    char *host;
    int port;
    struct sockaddr_in serv_addr;
    struct sigaction act;

    if (argc == 1) {
        printf("Usage: ./ftp_serv root_dir ipaddr:port proc_count\n");
        exit(0);
    } else if (argc != 4) {
        fprintf(stderr, "Error arguments count!!!\n");
        exit(1);
    }

    // Init config params
    if (chdir(argv[1]) < 0)
        fprintf(stderr, "No set root_dir");

    host = strtok(argv[2], ":");
    port = atoi(argv[2]+strlen(host)+1);
    pool_size = atoi(argv[3]);

    act.sa_handler = close_serv;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, host, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    printf("%s:%d\n", host, port);

    bind(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serv_fd, MAX_POOL_SIZE);

    for (i = 0; i < pool_size; i++) {
        if ((res_fork = fork()) == 0) {
            while (1) {
                cli_fd = accept_connection(serv_fd);
               
                char buffer[COMM_INFO_SIZE];
                Command *cmd = malloc(sizeof(Command));
                State *state = malloc(sizeof(State));

                memset(buffer, 0, COMM_INFO_SIZE);

                char welcome[COMM_INFO_SIZE] = "220 ";
                if(strlen(welcome_msg) < COMM_INFO_SIZE-4)
                    strcat(welcome, welcome_msg);
               
                // Write welcome message
                strcat(welcome, "\n");
                write(cli_fd, welcome, strlen(welcome));

                // Read commands from client
                while ((bytes = read(cli_fd, buffer, COMM_INFO_SIZE)) > 0) {
                    if(bytes < COMM_INFO_SIZE) {
                        buffer[bytes] = '\0';
                        
                        parse_comm(buffer, cmd);
                        state->conn = cli_fd;
                                                                        
                        response(cmd, state);
                        
                        memset(buffer, 0, COMM_INFO_SIZE);
                        memset(cmd, 0, sizeof(Command));
                    } else {
                        fprintf(stderr, "Error req client read!!!\n");
                    }
                }
            }
        } else {
            pids[i] = res_fork;
        }
    }

    sigaction(SIGINT, &act, 0);
    wait(NULL);
}
