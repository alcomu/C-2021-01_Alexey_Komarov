#include "tcp_serv.h"


void * tcp_serv(void *arg) {
    int serv_fd, cli_fd, err;
    struct sockaddr_in serv, cli;
    char str[64];
    
    TcpServArg *cfg = (TcpServArg *)arg;
    
    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd < 0)
        t_error("Could not create socket\n");

    serv.sin_family = AF_INET;
    serv.sin_port = htons(cfg->bind_port);
    // serv.sin_addr.s_addr = htonl(inet_aton(bind_addr));
    inet_pton(AF_INET, cfg->bind_addr, &serv.sin_addr);

    int opt_val = 1;
    setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(serv_fd, (struct sockaddr *)&serv, sizeof(serv));
    if (err < 0)
        t_error("Could not bind socket\n");

    err = listen(serv_fd, 128);
    if (err < 0)
        t_error("Could not listen on socket\n");

    printf("Server is listening on %d\n", cfg->bind_port);

    while (1) {
        socklen_t cli_len = sizeof(cli);
        cli_fd = accept(serv_fd, (struct sockaddr *)&cli, &cli_len);

        if (cli_fd < 0)
            t_error("Could not establish new connection\n");

        pthread_mutex_lock(cfg->dlock);
        sprintf(str, "%lu\n", *(cfg->data));
        pthread_mutex_unlock(cfg->dlock);

        err = send(cli_fd, str, strlen(str), 0);
        if (err < 0)
            t_error("Client write failed\n");
        close(cli_fd);
    }

    return 0;
}