#include "ftp.h"


static const char *comm_list_str[] = {"USER", "PASS", "PWD", "CWD", "LIST", "PASV", "QUIT", "RETR"};

#define BUF_SIZE 8192
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
    off_t orig;
    char buf[BUF_SIZE];
    size_t toRead, numRead, numSent, totSent;

    if (offset != NULL) {
        // Save current file offset and set offset to value in '*offset'
        orig = lseek(in_fd, 0, SEEK_CUR);
        if (orig == -1)
            return -1;
        if (lseek(in_fd, *offset, SEEK_SET) == -1)
            return -1;
    }

    totSent = 0;
    while (count > 0) {
        toRead = count < BUF_SIZE ? count : BUF_SIZE;

        numRead = read(in_fd, buf, toRead);
        if (numRead == (size_t)-1)
            return -1;
        if (numRead == (size_t)0)
            break;

        numSent = write(out_fd, buf, numRead);
        if (numSent == (size_t)-1)
            return -1;
        if (numSent == (size_t)0) {
            perror("sendfile: write() transferred 0 bytes");
            exit(-1);
        }

        count -= numSent;
        totSent += numSent;
    }

    if (offset != NULL) {
        // Return updated file offset in '*offset', and reset the file offset
        // to the value it had when we were called.
        *offset = lseek(in_fd, 0, SEEK_CUR);
        if (*offset == -1)
            return -1;
        if (lseek(in_fd, orig, SEEK_SET) == -1)
            return -1;
    }
    return totSent;
}

int lookup(char *needle, const char **haystack, int count) {
    int i;
    for (i = 0; i < count; i++)
        if (strcmp(needle, haystack[i]) == 0)
            return i;

    return -1;
}

int lookup_cmd(char *cmd) {
    const int cmdlist_count = sizeof(comm_list_str) / sizeof(char *);
    return lookup(cmd, comm_list_str, cmdlist_count);
}

int create_socket(int port) {
    int sock;
    struct sockaddr_in serv_addr;
 
    inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Cannot open socket");
        exit(1);
    }

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Cannot bind socket to address");
        exit(1);
    }

    listen(sock, 5);
    return sock;
}

int accept_connection(int socket) {
    struct sockaddr_in cli_addr;

    socklen_t cli_len = sizeof(cli_addr);
    return accept(socket, (struct sockaddr *)&cli_addr, &cli_len);
}

void get_ip(int sock, int *ip) {
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    getsockname(sock, (struct sockaddr *)&addr, &addr_size);

    char *host = inet_ntoa(addr.sin_addr);
    sscanf(host, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

void write_state(State *state) { write(state->conn, state->message, strlen(state->message)); }

void gen_port(Port *port) {
    srand(time(NULL));
    port->p1 = 128 + (rand() % 64);
    port->p2 = rand() % 0xff;
}

void parse_comm(char *comm_str, Command *comm) {
    sscanf(comm_str, "%s %s", comm->command, comm->arg);
}

void response(Command *cmd, State *state) {
    switch (lookup_cmd(cmd->command)) {
    case USER:
        ftp_user(state);
        break;
    case PASS:
        ftp_pass(state);
        break;
    case PASV:
        ftp_pasv(state);
        break;
    case LIST:
        ftp_list(cmd, state);
        break;
    case CWD:
        ftp_cwd(cmd, state);
        break;
    case PWD:
        ftp_pwd(state);
        break;
    case RETR:
        ftp_retr(cmd, state);
        break;
    case QUIT:
        ftp_quit(state);
        break;
    default:
        state->message = "500 command not supported\n";
        write_state(state);
        break;
    }
}

void ftp_pass(State *state) {
    state->message = "230 Login successful\n";
    write_state(state);
}

void ftp_user(State *state) {
    state->message = "331 User name okay, need password\n";
    write_state(state);
}

void ftp_pwd(State *state) {
    char cwd[COMM_INFO_SIZE];
    char result[COMM_INFO_SIZE];
    memset(result, 0, COMM_INFO_SIZE);
    if (getcwd(cwd, COMM_INFO_SIZE - 8) !=
        NULL) { // Making sure the length of cwd and additional text don't exceed BSIZE
        strcat(result, "257 \""); // 5 characters
        strcat(result, cwd);      // strlen(cwd)
        strcat(result, "\"\n");   // 2 characters + 0 byte
        state->message = result;
    } else {
        state->message = "550 Failed to get pwd.\n";
    }
    write_state(state);
}

void ftp_pasv(State *state) {
    int ip[4];
    char buff[255];
    char *response = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n";
    Port *port = malloc(sizeof(Port));
    gen_port(port);
    get_ip(state->conn, ip);

    if (state->sock_pasv != NULL)
        close(state->sock_pasv);

    // Start listening here, but don't accept the connection
    state->sock_pasv = create_socket((256 * port->p1) + port->p2);
    if (state->sock_pasv < 0) {
        fprintf(stderr, "create_socket error\n");
        exit(0);
    }

    sprintf(buff, response, ip[0], ip[1], ip[2], ip[3], port->p1, port->p2);
    state->message = buff;
    // puts(state->message);
    write_state(state);
}

void str_perm(int perm, char *str_perm) {
    int curperm = 0;
    int read, write, exec;

    // Flags buffer
    char fbuff[4];

    read = write = exec = 0;

    int i;
    // Explode permissions of user, group, others; starting with users
    for (i = 6; i >= 0; i -= 3) {
        curperm = ((perm & (S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)) >> i) & 0x7;

        memset(fbuff, 0, 3);
        /* Check rwx flags for each*/
        read = (curperm >> 2) & 0x1;
        write = (curperm >> 1) & 0x1;
        exec = (curperm >> 0) & 0x1;

        sprintf(fbuff, "%c%c%c", read ? 'r' : '-', write ? 'w' : '-', exec ? 'x' : '-');
        strcat(str_perm, fbuff);
    }
}

void ftp_list(Command *cmd, State *state) {
    struct dirent *entry;
    struct stat statbuf;
    struct tm *time;
    char timebuff[80]; // current_dir[COMM_INFO_SIZE];
    int connection;
    time_t rawtime;

    /* TODO: dynamic buffering maybe? */
    char cwd[COMM_INFO_SIZE], cwd_orig[COMM_INFO_SIZE];
    memset(cwd, 0, COMM_INFO_SIZE);
    memset(cwd_orig, 0, COMM_INFO_SIZE);

    /* Later we want to go to the original path */
    getcwd(cwd_orig, COMM_INFO_SIZE);

    /* Just chdir to specified path */
    if (strlen(cmd->arg) > 0 && cmd->arg[0] != '-') {
        chdir(cmd->arg);
    }

    getcwd(cwd, COMM_INFO_SIZE);
    DIR *dp = opendir(cwd);

    if (!dp) {
        state->message = "550 Failed to open directory.\n";
    } else {
        connection = accept_connection(state->sock_pasv);
        if (connection < 0) {
            fprintf(stderr, "accept_conn error\n");
            exit(0);
        }

        state->message = "150 Here comes the directory listing.\n";
        // puts(state->message);

        while ((entry = readdir(dp)) != NULL) {
            if (stat(entry->d_name, &statbuf) < 0) {
                fprintf(stderr, "FTP: Error reading file stats...\n");
            } else {
                // printf("Herna kakayato\n");
                char *perms = malloc(9);
                memset(perms, 0, 9);

                // Convert time_t to tm struct
                rawtime = statbuf.st_mtime;
                time = localtime(&rawtime);
                strftime(timebuff, 80, "%b %d %H:%M", time);
                str_perm((statbuf.st_mode & (S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)),
                         perms);
                dprintf(connection, "%c %s %5ld %4d %4d %8ld %s %s\r\n",
                        (entry->d_type == 4) ? 'd' : '-', perms, statbuf.st_nlink, statbuf.st_uid,
                        statbuf.st_gid, statbuf.st_size, timebuff, entry->d_name);

                free(perms);
            }
        }
        write_state(state);
        state->message = "226 Directory send OK.\n";
        close(connection);
        close(state->sock_pasv);
    }
    closedir(dp);
    chdir(cwd_orig);

    write_state(state);
}

void ftp_quit(State *state) {
    state->message = "221 Goodbye!\n";
    write_state(state);
    close(state->conn);
}

void ftp_cwd(Command *cmd, State *state) {
    if (chdir(cmd->arg) == 0) {
        state->message = "250 Directory successfully changed.\n";
    } else {
        state->message = "550 Failed to change directory.\n";
    }
    write_state(state);
}

void ftp_retr(Command *cmd, State *state) {
    int connection;
    int fd;
    struct stat stat_buf;
    off_t offset = 0;
    int sent_total = 0;

    if (access(cmd->arg, R_OK) == 0 && (fd = open(cmd->arg, O_RDONLY))) {
        fstat(fd, &stat_buf);

        state->message = "150 Opening BINARY mode data connection.\n";

        write_state(state);

        connection = accept_connection(state->sock_pasv);
        if (connection < 0) {
            fprintf(stderr, "accept_conn error\n");
            exit(0);
        }

        close(state->sock_pasv);
        if ((sent_total = sendfile(connection, fd, &offset, stat_buf.st_size)) != 0) {
            if (sent_total != stat_buf.st_size) {
                fprintf(stderr, "ftp_retr:sendfile\n");
                exit(0);
            }

            state->message = "226 File send OK.\n";
        } else {
            state->message = "550 Failed to read file.\n";
        }
    } else {
        state->message = "550 Failed to get file\n";
    }

    close(fd);
    close(connection);
    write_state(state);
}