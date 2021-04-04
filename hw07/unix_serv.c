#include "unix_serv.h"


void * unix_serv(void *arg) {
    int serv_fd, cli_fd, err;
    struct sockaddr_un serv, cli;
    char str[64];
    
    UnixServArg *cfg = (UnixServArg *)arg;
    
    serv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serv_fd < 0)
        u_error("Could not create socket\n");

    serv.sun_family = AF_UNIX;
    strcpy(serv.sun_path, cfg->name_sock);

    unlink(cfg->name_sock);
    err = bind(serv_fd, (struct sockaddr *)&serv, sizeof(serv));
    if (err < 0)
        u_error("Could not bind socket\n");

    err = listen(serv_fd, 128);
    if (err < 0)
        u_error("Could not listen on socket\n");

    printf("Unix socket is listening on %s\n", cfg->name_sock);

    while (1) {
        socklen_t cli_len = sizeof(cli);
        cli_fd = accept(serv_fd, (struct sockaddr *)&cli, &cli_len);

        if (cli_fd < 0)
            u_error("Could not establish new connection\n");

        pthread_mutex_lock(cfg->dlock);
        sprintf(str, "%lu\n", *(cfg->data));
        pthread_mutex_unlock(cfg->dlock);

        err = send(cli_fd, str, strlen(str), 0);
        if (err < 0)
            u_error("Client write failed\n");
        close(cli_fd);
    }

    return 0;
}