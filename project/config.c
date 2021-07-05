#include "toml.h"
#include "common.h"


#define CONFIG_FILE "rbuilder.conf"

char cfg_t_user[STR_LEN] = "root";
char cfg_t_host[STR_LEN] = "";
int cfg_t_port = 22;
char cfg_b_work_dir[STR_LEN] = "/usr/src/project";
char cfg_b_dependences[STR_LEN] = "";
char cfg_b_build_comm[STR_LEN] = "";
char cfg_b_install_comm[STR_LEN] = "";
char cfg_b_clean_comm[STR_LEN] = "";




void read_conf() {
    FILE *fp;
    char errbuf[STR_LEN];
    printf("<<< Read config file >>>\n");

    fp = fopen(CONFIG_FILE, "r");
    if (!fp) {
        fprintf(stderr, "Error cannot open %s\n", CONFIG_FILE);
    }

    toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) {
        fprintf(stderr, "cannot parse - %s\n", errbuf);
    }

    // Read config parametrs from file
    toml_table_t *transport = toml_table_in(conf, "Transport");
    if (transport) {
        toml_datum_t user = toml_string_in(transport, "user");
        if (user.ok) {
            memset(cfg_t_user, 0, STR_LEN);
            strncpy(cfg_t_user, user.u.s, strlen(user.u.s));
        }

        toml_datum_t host = toml_string_in(transport, "host");
        if (host.ok) {
            memset(cfg_t_host, 0, STR_LEN);
            strncpy(cfg_t_host, host.u.s, strlen(host.u.s));
        }

        toml_datum_t port = toml_int_in(transport, "port");
        if (port.ok)
            cfg_t_port = port.u.i;

        free(user.u.s);
        free(host.u.s);
    }

    toml_table_t *builder = toml_table_in(conf, "Builder");
    if (builder) {
        toml_datum_t work_dir = toml_string_in(builder, "work_dir");
        if (work_dir.ok) {
            memset(cfg_b_work_dir, 0, STR_LEN);
            strncpy(cfg_b_work_dir, work_dir.u.s, strlen(work_dir.u.s));
        }

        toml_array_t *dep_array = toml_array_in(builder, "dependences");
        if (dep_array) {
            for (int i = 0;; i++) {
                toml_datum_t dep = toml_string_at(dep_array, i);
                if (!dep.ok) break;
                char str[STR_LEN];
                // Prepare install dependences command
                snprintf(str, STR_LEN, 
                    "if [ \\$(dpkg -s %s | egrep 'Status:.*installed' | wc -l) -eq 0 ]; then apt-get install -y %s; fi;", 
                    dep.u.s, dep.u.s);
                strncat(cfg_b_dependences, str, strlen(str));
                free(dep.u.s);
            }
        }

        toml_array_t *build_array = toml_array_in(builder, "build_comm");
        if (build_array) {
            for (int i = 0;; i++) {
                toml_datum_t comm = toml_string_at(build_array, i);
                if (!comm.ok) break;
                strncat(cfg_b_build_comm, comm.u.s, strlen(comm.u.s));
                strncat(cfg_b_build_comm, "; ", (int)sizeof("; "));
                free(comm.u.s);
            }
        }

        toml_array_t *install_array = toml_array_in(builder, "install_comm");
        if (install_array) {
            for (int i = 0;; i++) {
                toml_datum_t comm = toml_string_at(install_array, i);
                if (!comm.ok)
                    break;
                strncat(cfg_b_install_comm, comm.u.s, strlen(comm.u.s));
                strncat(cfg_b_install_comm, "; ", (int)sizeof("; "));
                free(comm.u.s);
            }
        }

        toml_array_t *clean_array = toml_array_in(builder, "clean_comm");
        if (clean_array) {
            for (int i = 0;; i++) {
                toml_datum_t comm = toml_string_at(clean_array, i);
                if (!comm.ok)
                    break;
                strncat(cfg_b_clean_comm, comm.u.s, strlen(comm.u.s));
                strncat(cfg_b_clean_comm, "; ", (int)sizeof("; "));
                free(comm.u.s);
            }
        }
    }

    toml_free(conf);
}