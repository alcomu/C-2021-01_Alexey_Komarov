#include <curl/curl.h>
#include <inttypes.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define URL_LEN 256
#define BUFFER_SIZE (1024 * 1024)

#define URL_ID "https://www.metaweather.com/api/location/search/?query=%s"
#define URL_WEATHER "https://www.metaweather.com/api/location/%d"

enum Steps { GET_LOC_ID, GET_WEATHER };

typedef struct wr_res_ {
    uint32_t len;
    char *data;
    uint32_t pos;
} wr_res;

static size_t wr_func(void *ptr, size_t size, size_t nmemb, void *data) {
    wr_res *result = (wr_res *)data;

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

int get_locid_from_json(char *data) {
    json_error_t jerror;
    json_t *root, *obj, *value;
    const char *key;
    int res = 0;

    root = json_loads(data, 0, &jerror);

    if (json_is_array(root) && json_array_size(root) == 1) {
        obj = json_array_get(root, 0);
        json_object_foreach(obj, key, value) {
            if (strncmp(key, "woeid", strlen(key)) == 0)
                res = json_integer_value(value);
        }
    } else if (!json_is_array(root)) {
        fprintf(stderr, "This response is not a proper json array.\n");
        fprintf(stderr, "Error: %u:%u: %s\n", jerror.line, jerror.column, jerror.text);
    }

    if (root)
        json_decref(root);

    return res;
}

void print_weather_from_json(char *data) {
    json_error_t jerror;
    json_t *root, *val, *val1, *val2;
    const char *key, *key1;
    int i = 0;

    root = json_loads(data, 0, &jerror);

    if (json_is_object(root)) {
        json_object_foreach(root, key, val) {
            if (strncmp(key, "consolidated_weather", strlen("consolidated_weather")) == 0) {
                json_array_foreach(val, i, val1) {
                    json_object_foreach(val1, key1, val2) {
                        if (strncmp(key1, "weather_state_name", strlen("weather_state_name")) == 0)
                            printf("\n%s: %s\n", key1, json_string_value(val2));

                        if (strncmp(key1, "wind_direction_compass",
                                    strlen("wind_direction_compass")) == 0)
                            printf("%s: %s\n", key1, json_string_value(val2));

                        if (strncmp(key1, "applicable_date", strlen("applicable_date")) == 0)
                            printf("%s: %s\n", key1, json_string_value(val2));

                        if (strncmp(key1, "min_temp", strlen("min_temp")) == 0)
                            printf("%s: %d\n", key1, (int)json_real_value(val2));

                        if (strncmp(key1, "max_temp", strlen("max_temp")) == 0)
                            printf("%s: %d\n", key1, (int)json_real_value(val2));

                        if (strncmp(key1, "the_temp", strlen("the_temp")) == 0)
                            printf("%s: %d\n", key1, (int)json_real_value(val2));

                        if (strncmp(key1, "wind_speed", strlen("wind_speed")) == 0)
                            printf("%s: %.1f\n", key1, json_real_value(val2));

                        if (strncmp(key1, "air_pressure", strlen("air_pressure")) == 0)
                            printf("%s: %d\n", key1, (int)json_real_value(val2));

                        if (strncmp(key1, "humidity", strlen("humidity")) == 0)
                            printf("%s: %lld\n", key1, json_integer_value(val2));

                        if (strncmp(key1, "visibility", strlen("visibility")) == 0)
                            printf("%s: %.1f\n", key1, json_real_value(val2));

                        if (strncmp(key1, "predictability", strlen("predictability")) == 0)
                            printf("%s: %lld\n", key1, json_integer_value(val2));
                    }
                }
            }

            if (strncmp(key, "title", strlen("title")) == 0)
                printf("\n\n%s: %s, ", key, json_string_value(val));

            if (strncmp(key, "location_type", strlen("location_type")) == 0)
                printf("%s: %s\n", key, json_string_value(val));
        }
    } else {
        fprintf(stderr, "This response is not a proper json object.\n");
        fprintf(stderr, "Error: %u:%u: %s\n", jerror.line, jerror.column, jerror.text);
    }

    if (root)
        json_decref(root);
}

int curl_req(char *url, int step) {
    int res = 0;
    CURL *curl;
    CURLcode cres;
    wr_res wr_buff;

    // Initialisation write buffer
    wr_buff.data = (char *)malloc(BUFFER_SIZE * sizeof(char));
    wr_buff.len = BUFFER_SIZE;
    wr_buff.pos = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // Need for redirection
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wr_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&wr_buff);

        // Send request
        cres = curl_easy_perform(curl);

        // Check error
        if (cres != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(cres));

        if (step == GET_LOC_ID)
            res = get_locid_from_json(wr_buff.data);
        else if (step == GET_WEATHER)
            print_weather_from_json(wr_buff.data);

        curl_easy_cleanup(curl);
    }

    free(wr_buff.data);

    return res;
}

int main(int argc, char **argv) {
    char url_str[URL_LEN];
    int loc_id = 0;
    int state;

    if (argc == 1) {
        printf("Usage: ./get_weather location\n");
        exit(0);
    } else if (argc != 2) {
        fprintf(stderr, "Not found location patam!!!\n");
        exit(1);
    }

    state = GET_LOC_ID;
    sprintf(url_str, URL_ID, argv[1]);
    loc_id = curl_req(url_str, state);

    if (loc_id) {
        state = GET_WEATHER;
        sprintf(url_str, URL_WEATHER, loc_id);
        curl_req(url_str, state);
    } else {
        fprintf(stderr, "Location %s not found!!!\n", argv[1]);
    }

    return 0;
}
