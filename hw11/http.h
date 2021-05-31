#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


typedef struct MimeType_ {
	char *key;
	char *val;
} MimeType;


size_t file_attr(char *fname);
char *get_content_type(char *fname);
size_t recv_line(int fd, char *buf, size_t len);
int parse_head_line(const char *src, char *method, char *filepath);
int http_req(int fd);
