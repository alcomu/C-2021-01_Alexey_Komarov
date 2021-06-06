#include "http.h"


MimeType const m_types[] = {
	{ ".html",  "text/html"			},
	{ ".htm" ,  "text/html"			},
	{ ".css" ,  "text/css"			},
	{ ".js"  ,  "text/javascript"   },
	{ ".json",  "application/json"	},
	{ ".pdf" ,  "application/pdf"	},
	{ ".png" ,  "image/png"			},
	{ ".jpg" ,  "image/jpeg"		},
	{ ".svg" ,  "image/svg+xml"		},
	{ ".gif" ,  "image/gif"			}
};

// HTTP messages
const char *HTTP_ERR_501 = "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/html\r\nContent-length: 120\r\n\r\n"
	"<html><head><title>Error</title></head><body><hr><h1>HTTP method not implemented.</h1><hr><p>http_serv</p></body></html>";

const char *HTTP_ERR_404 = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\nContent-length: 107\r\n\r\n"
	"<html><head><title>Error</title></head><body><hr><h1>File not found.</h1><hr><p>http_serv</p></body></html>";

const char *HTTP_ERR_403 = "HTTP/1.0 403 Forbidden\r\nContent-Type: text/html\r\nContent-length: 106\r\n\r\n"
	"<html><head><title>Error</title></head><body><hr><h1>No permission.</h1><hr><p>http_serv</p></body></html>";

const char *HTTP_OK  = "HTTP/1.0 200 OK\r\n";

enum file_errs {
    NOT_EXISTING = -1,
    FILE_IS_DIR = -2,
    FILE_NO_PERM = -3
};


#define HTTP_BUF_SIZE 4096
#define DEFAULT_FILE "index.html"


size_t file_attr(char *fname) {
	struct stat info;

	if (strstr(fname, "..") != NULL || fname[0]=='/') return FILE_NO_PERM;
	if (stat(fname, &info) == -1) return NOT_EXISTING;
	if (S_ISDIR(info.st_mode)) return FILE_IS_DIR;
	if (info.st_uid == getuid()) {
		if (!(info.st_mode & S_IRUSR)) return FILE_NO_PERM;
	} else if (info.st_gid == getgid()) {
		if (!(info.st_mode & S_IRGRP)) return FILE_NO_PERM;
	} else if (!(info.st_mode & S_IROTH)) {
		return FILE_NO_PERM;
	}

	return info.st_size;
}

char *get_content_type(char *fname) {
	size_t i, len_name, len_key;
	len_name = strlen(fname);
	char *p;

	for (i=0; i<sizeof(m_types)/sizeof(MimeType); ++i) {
		p = m_types[i].val;
		len_key = strlen(m_types[i].key);

		if (len_key > len_name){
			continue;
		} else {
			if (strcmp(m_types[i].key, &(fname[len_name-len_key])) == 0)
				return p;
        }
	}

	// Default mime type
	return "application/octet-stream";
}

size_t recv_line(int fd, char *buf, size_t len) {
	size_t i = 0, err=1;

	while ((i<len-1) && err==1) {
		err = read(fd, &(buf[i]), 1);

		if (buf[i] == '\n')
            break;
		else
            i++;
	}

	if (i && (buf[i-1] == '\r'))
        i--;

	buf[i] = '\0';

	return i;
}

int parse_head_line(const char *src, char *method, char *filepath){
	if (strlen(src)<5) return 1;

	while ((*method++ = *src++) && *src!=' ');
	src++;
	while ((*filepath++ = *src++) && *src!=' ');

	return 0;
}

int http_req(int fd) {
	char buf[HTTP_BUF_SIZE] = "\0", request[8] = "\0", url[256] = "\0";
	char *fname, *content_type;
	size_t file_size;
	int f;
	off_t off = 0;


	if (recv_line(fd, buf, (sizeof(buf)-1)) <= 3) {
		fprintf(stderr, "can not receive request\n");
		return 1;
	}
	if (parse_head_line(buf, request, url)) {
		fprintf(stderr, "parsing error\n >request: `%s`\n", buf);
		return 1;
	}

	while (recv_line(fd, buf, (sizeof(buf)-1)) > 0);

	if ((strcmp(request, "GET")!=0) && (strcmp(request, "HEAD")!=0)) {
		write(fd, HTTP_ERR_501, strlen(HTTP_ERR_501));
		fprintf(stderr, "request method not supported\n >method: `%s`\n", request);
		return 0;
	}

	if (url[strlen(url)-1] == '/')
		url[strlen(url)-1] = '\0';

	fname = &(url[1]);
	if (!strlen(fname)) 
        fname = DEFAULT_FILE;

	file_size = file_attr(fname);

	if (file_size == (size_t)FILE_IS_DIR) {
		strcat(fname, "/index.html");
		file_size = file_attr(fname);
	}
	if (file_size == (size_t)NOT_EXISTING) {
        write(fd, HTTP_ERR_404, strlen(HTTP_ERR_404));
		return 0;
	}
	if (file_size == (size_t)FILE_NO_PERM) {
		write(fd, HTTP_ERR_403, strlen(HTTP_ERR_403));
		return 0;
	}

	if ((f = open(fname, O_RDONLY)) < 0) {
		write(fd, HTTP_ERR_404, strlen(HTTP_ERR_404));
		return 0;
	}

	write(fd, HTTP_OK, strlen(HTTP_OK));

	// Add content information
	content_type = get_content_type(fname);
	sprintf(buf, "Content-type: %s\r\n", content_type);
	write(fd, buf, strlen(buf));
	
	sprintf(buf, "Content-length: %ld\r\nServer: http_serv\r\n\r\n", file_size);
	write(fd, buf, strlen(buf));

	sendfile(fd, f, &off, file_size);

	close(f);

	return 0;
}
