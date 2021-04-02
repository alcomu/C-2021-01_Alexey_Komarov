#include "mylog.h"

void ml_init(MyLog *lo, const char *logfile) {
    lo->f = fopen(logfile, "a");
    if (!lo->f) {
        fprintf(stderr, "Error in %s() open logfile %s!!!\n", __func__, logfile);
        exit(1);
    }
}

void ml_final(MyLog *lo) {
    if (!lo->f) {
        fprintf(stderr, "Error in %s() not init file pointer!!!\n", __func__);
        exit(1);
    }

    fclose(lo->f);
}

const char *get_level_name(int id) {
    switch (id) {
    case LL_FATAL:
        return "FATAL";
        break;
    case LL_ERROR:
        return "ERROR";
        break;
    case LL_WARNING:
        return "WARNING";
        break;
    case LL_INFO:
        return "INFO";
        break;
    case LL_DEBUG:
        return "DEBUG";
        break;
    default:
        return "NONE";
        break;
    }
}
