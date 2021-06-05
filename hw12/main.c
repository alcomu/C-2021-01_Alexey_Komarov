#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <libpq-fe.h>
#include <math.h>


#define DEF_HOST "127.0.0.1"
#define DEF_PGSQL_PORT "5432"
#define DEF_MYSQL_PORT "3306"

#define REQ_LEN 128

typedef struct DbConnData_ {
    char *host;
    char *port;
    char *login;
    char *passwd;
    char *dbname;
    char *tblname;
    char *colname;
} DbConnData;




int stat_min(int a, int b) {
    if (a < b) return a;
    else return b;
}

int stat_max(int a, int b) {
    if (a > b) return a;
    else return b;
}

static void __attribute__ ((noreturn)) finish_err_pgsql(PGconn* conn) {
    puts(PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

int pgsql_handle(DbConnData *cdata) {
    char req[REQ_LEN];
    int i, max_val, min_val, sum_val;
    double sd_val, mean_val, tmp_val;
    PGconn *conn = PQsetdbLogin(cdata->host, cdata->port, 
                                NULL, NULL, cdata->dbname, cdata->login, cdata->passwd);
    
    if (PQstatus(conn) != CONNECTION_OK)
        finish_err_pgsql(conn);

    snprintf(req, REQ_LEN, "SELECT %s FROM %s;", cdata->colname, cdata->tblname);
    PGresult *res = PQexec(conn, req);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        finish_err_pgsql(conn);

    int nrows = PQntuples(res);
    if (nrows > 0) {
        mean_val = 0.0;
        sd_val = 0.0;
        tmp_val = 0.0;
        max_val = min_val = sum_val = atoi(PQgetvalue(res, 0, 0));
        
        for (i = 1; i < nrows; i++) {
            int val = atoi(PQgetvalue(res, i, 0));
            max_val = stat_max(max_val, val);
            min_val = stat_min(min_val, val);
            sum_val += val;
        }

        mean_val = (double)sum_val/nrows;

        for (i = 0; i < nrows; i++) {
            int val = atoi(PQgetvalue(res, i, 0));
            tmp_val += (val - mean_val) * (val - mean_val);
        }
        tmp_val /= nrows;
        sd_val = sqrt(tmp_val);

        printf("Total %d rows\n", nrows);
        printf("max_val: %d\n", max_val);
        printf("min_val: %d\n", min_val);
        printf("sum_val: %d\n", sum_val);
        printf("mean_val: %.4f\n", mean_val);
        printf("sd_val: %.4f\n", sd_val);
    }

    PQclear(res);
    PQfinish(conn);

    return 0;
}

static void __attribute__ ((noreturn)) finish_err_mysql(MYSQL* conn) {
    puts(mysql_error(conn));
    mysql_close(conn);
    exit(1);
}

int mysql_handle(DbConnData *cdata) {
    char req[REQ_LEN];
    int max_val, min_val, sum_val;
    double sd_val, mean_val, tmp_val;
    MYSQL* conn = mysql_init(NULL);

    if (!conn) {
        puts(mysql_error(conn));
        return 1;
    }

    if (!mysql_real_connect(conn, cdata->host, cdata->login, cdata->passwd,
                            cdata->dbname, atoi(cdata->port), NULL, 0))
        finish_err_mysql(conn);
    
    snprintf(req, REQ_LEN, "SELECT %s FROM %s;", cdata->colname, cdata->tblname);
    if (mysql_query(conn, req))
        finish_err_mysql(conn);

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result)
        finish_err_mysql(conn);

    int nrows = mysql_num_rows(result);
    if (nrows > 0) {
        mean_val = 0.0;
        sd_val = 0.0;
        tmp_val = 0.0;
        MYSQL_ROW row;

        row = mysql_fetch_row(result);
        max_val = min_val = sum_val = atoi(row[0]);
        
        while ((row = mysql_fetch_row(result))) {
            int val = atoi(row[0]);
            max_val = stat_max(max_val, val);
            min_val = stat_min(min_val, val);
            sum_val += val;
        }

        mean_val = (double)sum_val/nrows;
        mysql_data_seek(result, 0);

        while ((row = mysql_fetch_row(result))) {
            int val = atoi(row[0]);
            tmp_val += (val - mean_val) * (val - mean_val);
        }
        tmp_val /= nrows;
        sd_val = sqrt(tmp_val);

        printf("Total %d rows\n", nrows);
        printf("max_val: %d\n", max_val);
        printf("min_val: %d\n", min_val);
        printf("sum_val: %d\n", sum_val);
        printf("mean_val: %.4f\n", mean_val);
        printf("sd_val: %.4f\n", sd_val);
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}

int main(int argc, char **argv) {
    char *dbtype;
    DbConnData cdata;
    
    if (argc == 1) {
        printf("Usage: ./statdb dbtype dbname table column login passwd [host:port]\n");
        exit(0);
    } else if (argc < 7 || argc > 8) {
        fprintf(stderr, "Error arguments count!!!\n");
        exit(1);
    }

    dbtype = argv[1]; cdata.dbname = argv[2]; cdata.tblname = argv[3]; cdata.colname = argv[4];
    cdata.login = argv[5]; cdata.passwd = argv[6];
    if (argc == 8) {
        cdata.host = strtok(argv[7], ":");
        cdata.port = argv[7]+strlen(cdata.host)+1;
    } else {
        cdata.host = DEF_HOST;
        if (strncmp(dbtype, "pgsql", strlen(dbtype)) == 0)
            cdata.port = DEF_PGSQL_PORT;
        else if (strncmp(dbtype, "mysql", strlen(dbtype)) == 0)
            cdata.port = DEF_MYSQL_PORT;
        else
            cdata.port = 0;
    }

    // Run calc stat
    if (strncmp(dbtype, "pgsql", strlen(dbtype)) == 0)
        pgsql_handle(&cdata);
    else if (strncmp(dbtype, "mysql", strlen(dbtype)) == 0)
        mysql_handle(&cdata);
    else
        fprintf(stderr, "dbtype %s not supported!!!\n", dbtype);
    
    return 0;
}