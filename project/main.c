#include "common.h"


#define T_PREP_COMM "ssh -p%d %s@%s \"%s\""
#define T_RUN_COMM "ssh -p%d %s@%s \"cd %s; %s\""
#define T_SEND_COMM "scp -P%d ./temp.tar %s@%s:%s"

extern char cfg_t_user[STR_LEN];
extern char cfg_t_host[STR_LEN];
extern int cfg_t_port;
extern char cfg_b_work_dir[STR_LEN];
extern char cfg_b_dependences[STR_LEN];
extern char cfg_b_build_comm[STR_LEN];
extern char cfg_b_install_comm[STR_LEN];
extern char cfg_b_clean_comm[STR_LEN];


extern void read_conf();
void prep_remote_comm(const char *c);
void run_remote_comm(const char *c);
void prepare_local_files();
void prepare_remote_files();
void clear_tmp_files();
void build_handler();
void install_handler();
void clean_handler();

void prep_remote_comm(const char *c) {
    char comm[COMM_SIZE];

    snprintf(comm, COMM_SIZE, T_PREP_COMM, cfg_t_port, cfg_t_user, cfg_t_host, c);
    system(comm);
}

void run_remote_comm(const char *c) {
    char comm[COMM_SIZE];

    snprintf(comm, COMM_SIZE, T_RUN_COMM, cfg_t_port, cfg_t_user, cfg_t_host, cfg_b_work_dir, c);
    system(comm);
}

void prepare_local_files() { 
    system("tar -cf temp.tar ./*"); 
}

void prepare_remote_files() {
    char comm[COMM_SIZE];
    snprintf(comm, COMM_SIZE, "mkdir -p %s; cd %s; rm -r ./*", cfg_b_work_dir, cfg_b_work_dir);
    prep_remote_comm(comm);

    // Send archive to remote host
    memset(comm, 0, COMM_SIZE);
    snprintf(comm, COMM_SIZE, T_SEND_COMM, cfg_t_port, cfg_t_user, cfg_t_host, cfg_b_work_dir);
    system(comm);

    // Prepare remote files
    run_remote_comm("tar -xf temp.tar");
}

void clear_tmp_files() {
    printf("<<< Clear tmp files >>>\n");

    // Local
    system("rm -r ./temp*");
    // Remote
    run_remote_comm("rm ./temp.tar; rm rbuilder.conf");
}

void build_handler() {
    printf("<<< Start building role >>>\n");
    printf("<<< Prepare local files >>>\n");
    prepare_local_files();
    printf("<<< Prepare remote files >>>\n");
    prepare_remote_files();

    if (strlen(cfg_b_dependences)) {
        printf("<<< Run install dependences >>>\n");
        run_remote_comm(cfg_b_dependences);
    }

    if (strlen(cfg_b_build_comm)) {
        printf("<<< Run build commands >>>\n");
        run_remote_comm(cfg_b_build_comm);
    }

    clear_tmp_files();
    printf("<<< Done >>>\n");
}

void install_handler() {
    printf("<<< Start install role >>>\n");

    if (strlen(cfg_b_install_comm)) {
        printf("<<< Run install commands >>>\n");
        run_remote_comm(cfg_b_install_comm);
    }

    printf("<<< Done >>>\n");
}

void clean_handler() {
    char comm[COMM_SIZE];
    printf("<<< Start clean role >>>\n");

    if (strlen(cfg_b_clean_comm)) {
        printf("<<< Run clean commands >>>\n");
        run_remote_comm(cfg_b_clean_comm);
    }

    snprintf(comm, COMM_SIZE, "rm -r %s", cfg_b_work_dir);
    prep_remote_comm(comm);

    printf("<<< Done >>>\n");
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Usage: ./rbuilder role\n");
        printf("       role - build | install | clean \n");
        exit(0);
    } else if (argc != 2) {
        fprintf(stderr, "Not found role patam!!!\n");
        exit(1);
    }

    read_conf();

    if (!strncmp(argv[1], "build", strlen("build")))
        build_handler();
    else if (!strncmp(argv[1], "install", strlen("install")))
        install_handler();
    else if (!strncmp(argv[1], "clean", strlen("clean")))
        clean_handler();
    else
        printf("Not found %s role !!!\n", argv[1]);

    return 0;
}