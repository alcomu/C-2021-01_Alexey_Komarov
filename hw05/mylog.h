#pragma once

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#define BT_BUF_SIZE 100

enum LogLevel { LL_FATAL, LL_ERROR, LL_WARNING, LL_INFO, LL_DEBUG };

typedef struct MyLog_ {
    FILE *f;
} MyLog;

void ml_init(MyLog *lo, const char *logfile);
void ml_final(MyLog *lo);
const char *get_level_name(int id);

#define ml_write(lo, level, fs, ...)                                                               \
    {                                                                                              \
        if (!lo.f) {                                                                               \
            fprintf(stderr, "ml_write(): Error not init file pointer!!!\n");                       \
            exit(1);                                                                               \
        }                                                                                          \
                                                                                                   \
        fprintf(lo.f, "%s:%d %s(): [%s]: " fs "\n", __FILE__, __LINE__, __func__,                  \
                get_level_name(level), ##__VA_ARGS__);                                             \
                                                                                                   \
        if (level == LL_ERROR) {                                                                   \
            int nptrs;                                                                             \
            void *buffer[BT_BUF_SIZE];                                                             \
            char **strings;                                                                        \
                                                                                                   \
            nptrs = backtrace(buffer, BT_BUF_SIZE);                                                \
            strings = backtrace_symbols(buffer, nptrs);                                            \
                                                                                                   \
            if (strings == NULL) {                                                                 \
                fprintf(stderr, "ml_write(): Error backtrace_symbols!!!\n");                       \
                exit(1);                                                                           \
            }                                                                                      \
                                                                                                   \
            for (int j = 0; j < nptrs; j++)                                                        \
                fprintf(lo.f, "  %s\n", strings[j]);                                               \
                                                                                                   \
            free(strings);                                                                         \
        }                                                                                          \
    }

#define ml_fatal_write(lo, expr, fs, ...)                                                          \
    ((void)sizeof((expr) ? 1 : 0), __extension__({                                                 \
         if (expr)                                                                                 \
             ; /* empty */                                                                         \
         else {                                                                                    \
                                                                                                   \
             if (!lo.f) {                                                                          \
                 fprintf(stderr, "ml_expr_write(): Error not init file pointer!!!\n");             \
                 exit(1);                                                                          \
             }                                                                                     \
                                                                                                   \
             fprintf(lo.f, "%s:%d %s(): %s [%s]: " fs "\n", __FILE__, __LINE__, __func__, #expr,   \
                     get_level_name(LL_FATAL), ##__VA_ARGS__);                                     \
                                                                                                   \
             int nptrs;                                                                            \
             void *buffer[BT_BUF_SIZE];                                                            \
             char **strings;                                                                       \
                                                                                                   \
             nptrs = backtrace(buffer, BT_BUF_SIZE);                                               \
             strings = backtrace_symbols(buffer, nptrs);                                           \
                                                                                                   \
             if (strings == NULL) {                                                                \
                 fprintf(stderr, "ml_expr_write(): Error backtrace_symbols!!!\n");                 \
                 exit(1);                                                                          \
             }                                                                                     \
                                                                                                   \
             for (int j = 0; j < nptrs; j++)                                                       \
                 fprintf(lo.f, "  %s\n", strings[j]);                                              \
                                                                                                   \
             free(strings);                                                                        \
                                                                                                   \
             ml_final(&lo);                                                                         \
             exit(1);                                                                              \
         }                                                                                         \
     }))
