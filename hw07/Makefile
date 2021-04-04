all: fsize_info

fsize_info: main.c tcp_serv.c unix_serv.c libtoml.a -lpthread
	 $(CC) $(CFLAGS) -Wall -Wextra -Wpedantic -rdynamic -D_POSIX_C_SOURCE -std=c11 $^ -o $@

libtoml.a: toml.o
	 ar rcs $@ $^

toml.o: toml.c
	 $(CC) -c $(CFLAGS) -Wall -Wextra -Wpedantic -rdynamic -std=c11 $^ -o $@

clean: 
	 rm -f fsize_info libtoml.a core *.o

.PHONY: fsize_info clean
