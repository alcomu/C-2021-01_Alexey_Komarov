#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>

#include "toml.h"
#include "tcp_serv.h"
#include "unix_serv.h"

#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>


#define CONFIG_FILE "fsize_info.conf"
#define STR_LEN 256

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

#define f_error(...)                                                                               \
    {                                                                                              \
        fprintf(stderr, __VA_ARGS__);                                                              \
        fflush(stderr);                                                                            \
        exit(1);                                                                                   \
    }

// Default config
static char cfg_gen_file[STR_LEN] = "file.txt";
static int  cfg_gen_demon = 0;
static int  cfg_tcp_en = 0;
static char cfg_tcp_bind_addr[STR_LEN] = "127.0.0.1";
static int  cfg_tcp_bind_port = 1111;
static int  cfg_unix_en = 0;
static char cfg_unix_name[STR_LEN] = "unix_sock";

pthread_mutex_t lock;
static uint64_t cur_file_size = 0;

void daemonize()
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    uint32_t i;


    umask(0);

    // Get max file descriptor number
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        perror("max file descriptor number");

    if ((pid = fork()) < 0)
        perror("run fork error");
    else if (pid != 0) /* parrent process */
    exit(0);

    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        perror("not ignore SIGHUP");
    if ((pid = fork()) < 0)
        perror("run fork error");
    else if (pid != 0) /* parent process */
        exit(0);

    // if (chdir("/") < 0)
    //     perror("no set root dir /");

    // Close descriptors
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    // Cennetct descriptors 0,1,2 to /dev/null
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    // Init syslog
    openlog("fsize_info", LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "error file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}

void read_conf() {
    FILE *fp;
    char errbuf[STR_LEN];

    fp = fopen(CONFIG_FILE, "r");
    if (!fp) {
        fprintf(stderr, "Error cannot open sample.toml - %s\n", CONFIG_FILE);
    }

    toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) {
        fprintf(stderr, "cannot parse - %s\n", errbuf);
    }

    // Read config parametrs from file
    toml_table_t *gen = toml_table_in(conf, "general");
    if (gen) {
        toml_datum_t file = toml_string_in(gen, "file");
        if (file.ok) {
            memset(cfg_gen_file, 0, STR_LEN);
            strncpy(cfg_gen_file, file.u.s, strlen(file.u.s));
        }

        toml_datum_t demon = toml_int_in(gen, "demon");
        if (demon.ok)
            cfg_gen_demon = demon.u.i;

        free(file.u.s);
    }

    toml_table_t *tcp_s = toml_table_in(conf, "tcp_sock");
    if (tcp_s) {
        toml_datum_t en = toml_int_in(tcp_s, "enable");
        if (en.ok)
            cfg_tcp_en = en.u.i;

        toml_datum_t b_addr = toml_string_in(tcp_s, "bind_addr");
        if (b_addr.ok) {
            memset(cfg_tcp_bind_addr, 0, STR_LEN);
            strncpy(cfg_tcp_bind_addr, b_addr.u.s, strlen(b_addr.u.s));
        }

        toml_datum_t b_port = toml_int_in(tcp_s, "bind_port");
        if (b_port.ok)
            cfg_tcp_bind_port = b_port.u.i;

        free(b_addr.u.s);
    }

    toml_table_t *unix_s = toml_table_in(conf, "unix_sock");
    if (unix_s) {
        toml_datum_t en = toml_int_in(unix_s, "enable");
        if (en.ok)
            cfg_unix_en = en.u.i;

        toml_datum_t name = toml_string_in(unix_s, "name");
        if (name.ok) {
            memset(cfg_unix_name, 0, STR_LEN);
            strncpy(cfg_unix_name, name.u.s, strlen(name.u.s));
        }

        free(name.u.s);
    }

    toml_free(conf);
}

void update_file_size() {
    FILE *fp;
    uint64_t fpos;

    fp = fopen(cfg_gen_file, "r");
    if (!fp)
        f_error("Error cannot open %s\n", cfg_gen_file);
        
    fpos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    pthread_mutex_lock(&lock);
    cur_file_size = ftell(fp);
    pthread_mutex_unlock(&lock);
    fseek(fp, fpos, SEEK_SET);

    fclose(fp);
}

int main() {
    int len;
    int fd;
    int wd;
    char buff[BUF_LEN];
    char *p;
    struct inotify_event *event;

    pthread_t thr_tcp;
    pthread_t thr_unix;
    int pstat;
    
    TcpServArg tscfg;
    UnixServArg uscfg;

    if (pthread_mutex_init(&lock, NULL) != 0)
        f_error("mutex init failed\n");
        
    read_conf();

    if (cfg_gen_demon)
        daemonize();

    // Get current file size
    update_file_size();

    // Starting tcp server
    if (cfg_tcp_en) {
        tscfg.bind_addr = cfg_tcp_bind_addr;
        tscfg.bind_port = cfg_tcp_bind_port;
        tscfg.dlock = &lock;
        tscfg.data = &cur_file_size;

        pstat = pthread_create(&thr_tcp, NULL, tcp_serv, (void *)&tscfg);
        if (pstat != 0)
            f_error("create thread: %s\n", strerror(pstat));
    }

    // Open unix domain socket
    if (cfg_unix_en) {
        uscfg.name_sock = cfg_unix_name;
        uscfg.dlock = &lock;
        uscfg.data = &cur_file_size;

        pstat = pthread_create(&thr_unix, NULL, unix_serv, (void *)&uscfg);
        if (pstat != 0)
            f_error("create thread: %s\n", strerror(pstat));
    }

    // Update file size
    fd = inotify_init();

    if (fd < 0)
        perror("inotify_init");
    
    wd = inotify_add_watch(fd, cfg_gen_file, IN_MODIFY);

    while(1) {
        len = read(fd, buff, BUF_LEN);

        if (len == -1)
            perror("inotify_read");
        
        for (p = buff; p < buff + len; ) {
            event = (struct inotify_event *)p;

            // displayInotifyEvent(event);
            if (event->mask & IN_MODIFY)
                update_file_size();

            p += EVENT_SIZE + event->len;
        }
    }

    if (cfg_tcp_en)
        pthread_join(thr_tcp, NULL);
    if (cfg_unix_en)
        pthread_join(thr_unix, NULL);

    inotify_rm_watch(fd, wd);
    close(fd);
    pthread_mutex_destroy(&lock);

    return 0;
}