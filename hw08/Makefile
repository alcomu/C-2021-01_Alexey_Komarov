all: ftp_serv

ftp_serv: main.c ftp.c
	 $(CC) $(CFLAGS) -Wall -Wextra -Wpedantic -rdynamic -D_POSIX_C_SOURCE -D_GNU_SOURCE -std=c11 $^ -o $@

clean: 
	 rm -f ftp_serv core *.o

.PHONY: ftp_serv clean
