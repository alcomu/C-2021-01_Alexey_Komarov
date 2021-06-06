#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#include "http.h"


#define MAX_EVENTS 64
#define BUF_SIZE   4096
static int th_count = 1;


typedef struct ServArg_ {
    int serv_fd;
} ServArg;


static void epoll_ctl_add(int epfd, int fd, uint32_t events) {
	struct epoll_event ev = {0};
	ev.events = events;
    ev.data.fd = fd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl()\n");
		exit(1);
	}
}

static int setnonblocking(int sockfd) {
	if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1)
		return -1;
	
	return 0;
}

int accept_connection(int socket) {
    struct sockaddr_in cli_addr;

    socklen_t cli_len = sizeof(cli_addr);
    return accept(socket, (struct sockaddr *)&cli_addr, &cli_len);
}

void *serv_func(void *arg) {
    ServArg *sarg = (ServArg *)arg;
    int epfd, nfds, conn_sock, i;
    struct epoll_event events[MAX_EVENTS];
    

    epfd = epoll_create(1);
	epoll_ctl_add(epfd, sarg->serv_fd, EPOLLIN | EPOLLOUT | EPOLLET);

    while (1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (i=0; i<nfds; i++) {
            if (events[i].data.fd == sarg->serv_fd) {
                conn_sock = accept_connection(sarg->serv_fd);

                setnonblocking(conn_sock);

                epoll_ctl_add(epfd, conn_sock,
				    EPOLLIN  | EPOLLET | EPOLLRDHUP | EPOLLHUP);
            } else {
                if (events[i].events & EPOLLIN)
                    http_req(events[i].data.fd);
                                
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
            }
        }
    }
}

int main(int argc, char **argv) {
    int serv_fd, i;
    char *host;
    int port;
    struct sockaddr_in serv_addr;

    if (argc == 1) {
        printf("Usage: ./http_serv root_dir ipaddr:port [threads]\n");
        exit(0);
    } else if (argc < 3) {
        fprintf(stderr, "Error arguments count!!!\n");
        exit(1);
    }

    // Init config params
    if (chdir(argv[1]) < 0)
        fprintf(stderr, "No set root_dir");

    host = strtok(argv[2], ":");
    port = atoi(argv[2]+strlen(host)+1);
    if (argc == 4)
        th_count = atoi(argv[3]);

    pthread_t threads[th_count];
    ServArg sargs[th_count];

    
    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, host, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    
    if (bind(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        fprintf(stderr, "Error bind serv socket!!!\n");
        exit(1);
    }
    setnonblocking(serv_fd);
    if (listen(serv_fd, 256) != 0) {
        fprintf(stderr, "Error listen serv socket!!!\n");
        exit(1);
    }

    for (i=0; i<th_count; ++i) {
        sargs[i].serv_fd = serv_fd;
        pthread_create(&threads[i], NULL, serv_func, (void *)&(sargs[i]));
    }

    for (i=0; i<th_count; ++i)
        pthread_join(threads[i], NULL);
}