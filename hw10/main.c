#include <dirent.h>
#include <glib.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_LEN 16384
#define STAT_UNIT_SIZE 10
#define MAX_LOG_FILES 64

typedef struct AccStatUnit_ {
    char *url;
    char *ref;
    size_t tx_bytes;
    size_t count;
} AccStatUnit;

typedef struct LogStat_ {
    char fname[STR_LEN];
    GPtrArray *top_urls;
    GPtrArray *top_refs;
    uint64_t tx_bytes;
    int ptid;
} LogStat;

typedef struct LogArg_ {
    LogStat *lstats;
    size_t lstats_size;
    int ptid;
} LogArg;




gint comp_tx_bytes(gconstpointer a, gconstpointer b) {
    const AccStatUnit *asu_a = *((AccStatUnit **)a);
    const AccStatUnit *asu_b = *((AccStatUnit **)b);

    if (asu_a->tx_bytes < asu_b->tx_bytes) return -1;
    else if (asu_a->tx_bytes > asu_b->tx_bytes) return 1;
    else return 0;
}

gint comp_count(gconstpointer a, gconstpointer b) {
    const AccStatUnit *asu_a = *((AccStatUnit **)a);
    const AccStatUnit *asu_b = *((AccStatUnit **)b);

    if (asu_a->count < asu_b->count) return -1;
    else if (asu_a->count > asu_b->count) return 1;
    else return 0;
}

gint comp_ref(gconstpointer a, gconstpointer b) {
    const AccStatUnit *asu_a = *((AccStatUnit **)a);
    const AccStatUnit *asu_b = *((AccStatUnit **)b);

    return g_strcmp0(asu_a->ref, asu_b->ref);
}

AccStatUnit *acc_stat_unit_new(const char *u, const char *r, size_t b, size_t c) {
    AccStatUnit *asu = g_malloc0(sizeof(AccStatUnit));
    asu->url = malloc(strlen(u)+1);
    asu->ref = malloc(strlen(r)+1);

    memset(asu->url, 0, strlen(u)+1);
    memset(asu->ref, 0, strlen(r)+1);
    strncpy(asu->url, u, strlen(u));
    strncpy(asu->ref, r, strlen(r));
    asu->tx_bytes = b;
    asu->count = c;

    return asu;
}

void acc_stat_unit_free(AccStatUnit *asu) {
    free(asu->url);
    free(asu->ref);
    g_free(asu);
}

void *parse_func(void *arg) {
    LogArg *larg = (LogArg *)arg;
    LogStat *lstat;
    size_t it;
    FILE *f;
    char str[STR_LEN];
    size_t i, j, k;
    AccStatUnit *asu, *asu1;

    for (it=0; it<larg->lstats_size; ++it) {
        k = 10;

        lstat = &larg->lstats[it];
        if (lstat->ptid != larg->ptid) continue;  // Check ptid

        GPtrArray *stat_arr = g_ptr_array_new_full(100000, (GDestroyNotify)acc_stat_unit_free);
        
        // Read stat from file
        f = fopen(lstat->fname, "r");
        if (!f) {
            fprintf(stderr, "Error read log file %s!!!\n", lstat->fname);
        } else {
            while (fgets(str, STR_LEN, f) && k--) {
                if (!*str)
                    continue;

                char strtmp[STR_LEN] = {0};
                char url[STR_LEN] = {0};
                char referer[STR_LEN] = {0};
                unsigned long tx_bytes = 0;
                size_t len = 0;
                char *fc , *fc1;

                // printf("str: %s", str);

                fc = strchr(str, '"')+1;
                len = strchr(fc+1, '"')-fc;
                strncpy(strtmp, fc, len);
                fc += len;
                // printf("req: %s\n", strtmp);

                fc1 = strchr(strtmp, ' ');
                if (fc1 != NULL) {
                    fc1 = strchr(fc1+1, ' ');
                    if (fc1 != NULL) {
                        fc1 = strchr(strtmp, ' ')+1;
                        len = strchr(fc1+1, ' ')-fc1;
                        strncpy(url, fc1, len);
                    }
                }
                // printf("url: %s\n", url);

                fc = strchr(fc, ' ')+1;
                fc = strchr(fc, ' ')+1;
                len = strchr(fc+1, ' ')-fc;
                memset(strtmp, 0, STR_LEN);
                strncpy(strtmp, fc, len);
                fc += len;
                tx_bytes = atol(strtmp);
                // printf("tx_bytes: %ld\n", tx_bytes);
                
                lstat->tx_bytes += tx_bytes;

                fc = strchr(fc, '"')+1;
                len = strchr(fc+1, '"')-fc;
                strncpy(referer, fc, len);
                // printf("referer: [%s]\n\n", referer);

                g_ptr_array_add(stat_arr, acc_stat_unit_new(url, referer, tx_bytes, 1));
            }

            // Top 10 urls 
            g_ptr_array_sort(stat_arr, (GCompareFunc)comp_tx_bytes);

            i = STAT_UNIT_SIZE - 1;
            asu1 = (AccStatUnit *)stat_arr->pdata[stat_arr->len-1];
            for (j=stat_arr->len-2; j>0; --j) {
                asu = (AccStatUnit *)stat_arr->pdata[j];

                // First or new url 
                if (j == stat_arr->len-2 || strncmp(asu->url, asu1->url, strlen(asu->url)) != 0) {
                    g_ptr_array_add(lstat->top_urls, acc_stat_unit_new(asu->url, asu->ref, asu->tx_bytes, asu->count));
                    asu1 = asu;
                    i--;
                }

                if (!i) break;
            }

            // Top 10 referers
            g_ptr_array_sort(stat_arr, (GCompareFunc)comp_ref);

            asu1 = (AccStatUnit *)stat_arr->pdata[stat_arr->len-1];
            for (j=stat_arr->len-1; j>0; --j) {
                asu = (AccStatUnit *)stat_arr->pdata[j];

                // First or new referer
                if (j == stat_arr->len-1 || strncmp(asu->ref, asu1->ref, strlen(asu->ref)) == 0) {
                    asu1->count++;
                } else {
                    asu1 = asu;
                }
            }

            g_ptr_array_sort(stat_arr, (GCompareFunc)comp_count);

            i = STAT_UNIT_SIZE;
            for (j=stat_arr->len-1; j>0; --j) {
                asu = (AccStatUnit *)stat_arr->pdata[j];

                g_ptr_array_add(lstat->top_refs, acc_stat_unit_new(asu->url, asu->ref, asu->tx_bytes, asu->count));
                
                if (!--i) break;
            }

            // Free resource
            g_ptr_array_unref(stat_arr);
            fclose(f);
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    struct dirent *entry;
    size_t i, j, thcount;
    AccStatUnit *asu, *asu1;
    size_t tx_bytes_sum = 0;

    if (argc == 1) {
        printf("Usage: ./logstat logdir threads\n");
        exit(0);
    } else if (argc != 3) {
        fprintf(stderr, "Error arguments count!!!\n");
        exit(1);
    }

    thcount = atoi(argv[2]);
    pthread_t threads[thcount];
    LogStat lstats[MAX_LOG_FILES];
    LogArg larg[MAX_LOG_FILES];
    GPtrArray *urls_stat_arr = g_ptr_array_new_full(1000, (GDestroyNotify)acc_stat_unit_free);
    GPtrArray *refs_stat_arr = g_ptr_array_new_full(1000, (GDestroyNotify)acc_stat_unit_free);
    
    // larg init
    for (i=0; i<MAX_LOG_FILES; ++i) {
        larg[i].lstats = (LogStat *)&lstats;
        larg[i].lstats_size = 0;
        larg[i].ptid = -1;
    }

    DIR *dp = opendir(argv[1]);

    if (!dp) {
        fprintf(stderr, "Not open log dir!!!\n");
        exit(1);
    } else {
        i = 0, j = 0;
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_type != 4) { // If not directory
                memset(lstats[i].fname, 0, STR_LEN);
                strncpy(lstats[i].fname, argv[1], strlen(argv[1]));
                strncat(lstats[i].fname, "/", 2);
                strncat(lstats[i].fname, entry->d_name, strlen(entry->d_name));
                lstats[i].top_urls = g_ptr_array_new_full(1000, (GDestroyNotify)acc_stat_unit_free);
                lstats[i].top_refs = g_ptr_array_new_full(1000, (GDestroyNotify)acc_stat_unit_free);
                lstats[i].tx_bytes = 0;
                lstats[i].ptid = j;
                
                if (++j >= thcount) j = 0;  // files per thread
                if (++i >= MAX_LOG_FILES) break;  // threads need <= files
            }
        }
        if (thcount > i) thcount = i;  // threads need <= files

        for (j=0; j<i; ++j)
            larg[j].lstats_size = i;
    }
    closedir(dp);

    // Create threads
    for (i=0; i<thcount; ++i) {
        larg[i].ptid = i;
        pthread_create(&threads[i], NULL, parse_func, (void *)&(larg[i]));
    }

    // Wait end work threads
    for (i=0; i<thcount; ++i)
        pthread_join(threads[i], NULL);

    // Show stat
    for (i=0; i<larg[0].lstats_size; ++i) {
        printf("\n---------------------------------------------------------------------");
        printf("\n          Top 10 urls %s:\n  tx_bytes | url\n", lstats[i].fname);
        printf("---------------------------------------------------------------------\n");
        for (j=0; j<lstats[i].top_urls->len; j++) {
            asu = (AccStatUnit *)lstats[i].top_urls->pdata[j];

            g_ptr_array_add(urls_stat_arr, acc_stat_unit_new(asu->url, asu->ref, asu->tx_bytes, asu->count));
            printf("  %ld %s\n", asu->tx_bytes, asu->url);
        }

        printf("\n---------------------------------------------------------------------");
        printf("\n          Top 10 refs %s:\n  count | referer\n", lstats[i].fname);
        printf("---------------------------------------------------------------------\n");
        for (j=0; j<lstats[i].top_refs->len; j++) {
            asu = (AccStatUnit *)lstats[i].top_refs->pdata[j];

            g_ptr_array_add(refs_stat_arr, acc_stat_unit_new(asu->url, asu->ref, asu->tx_bytes, asu->count));
            printf("  %ld %s\n", asu->count, asu->ref);
        }

        printf("\n---------------------------------------------------------------------");
        printf("\n          Transmit bytes %s\n", lstats[i].fname);
        printf("---------------------------------------------------------------------\n");
        printf("  %ld\n", lstats[i].tx_bytes); 

        // Get summaru transmit bytes
        tx_bytes_sum += lstats[i].tx_bytes;
    }

    // Show summary stat
    g_ptr_array_sort(urls_stat_arr, (GCompareFunc)comp_tx_bytes);
    printf("\n---------------------------------------------------------------------");
    printf("\n                      Top 10 urls summary:\n  tx_bytes | url\n");
    printf("---------------------------------------------------------------------\n");
    i = STAT_UNIT_SIZE - 1;
    asu1 = (AccStatUnit *)urls_stat_arr->pdata[urls_stat_arr->len-1];
    for (j=urls_stat_arr->len-2; j>0; --j) {
        asu = (AccStatUnit *)urls_stat_arr->pdata[j];

        // First or new url 
        if (j == urls_stat_arr->len-2 || strncmp(asu->url, asu1->url, strlen(asu->url)) != 0) {
            printf("  %ld %s\n", asu->tx_bytes, asu->url);
            asu1 = asu;
            i--;
        }

        if (!i) break;
    }

    g_ptr_array_sort(refs_stat_arr, (GCompareFunc)comp_ref);
    printf("\n---------------------------------------------------------------------");
    printf("\n                      Top 10 refs summary:\n  count | referer\n");
    printf("---------------------------------------------------------------------\n");
    asu1 = (AccStatUnit *)refs_stat_arr->pdata[refs_stat_arr->len-1];
    for (j=refs_stat_arr->len-1; j>0; --j) {
        asu = (AccStatUnit *)refs_stat_arr->pdata[j];

        // First or new referer
        if (strncmp(asu->ref, asu1->ref, strlen(asu->ref)) == 0) {
            asu1->count += asu->count;
        } else {
            asu1 = asu;
        }
    }

    g_ptr_array_sort(refs_stat_arr, (GCompareFunc)comp_count);

    i = STAT_UNIT_SIZE;
    asu1 = (AccStatUnit *)refs_stat_arr->pdata[refs_stat_arr->len-1];
    printf("  %ld %s\n", asu1->count, asu1->ref);
    for (j=refs_stat_arr->len-2; j>0; --j) {
        asu = (AccStatUnit *)refs_stat_arr->pdata[j];

        if (strncmp(asu->ref, asu1->ref, strlen(asu->ref)) != 0) {
            printf("  %ld %s\n", asu->count, asu->ref);
            asu1 = asu;
            if (!--i) break;
        }
    }

    printf("\n---------------------------------------------------------------------");
    printf("\n          Transmit bytes summary %s\n", lstats[i].fname);
    printf("---------------------------------------------------------------------\n");
    printf("  %ld\n", tx_bytes_sum); 


    // Free resource
    for (i=0; i<larg[0].lstats_size; ++i) {
        g_ptr_array_unref(lstats[i].top_urls);
        g_ptr_array_unref(lstats[i].top_refs);
    }
    g_ptr_array_unref(urls_stat_arr);
    g_ptr_array_unref(refs_stat_arr);
    
    return 0;
}