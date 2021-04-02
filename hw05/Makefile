all: hw05

hw05: main.c libmylog.a
	 $(CC) $(CFLAGS) -Wall -Wextra -Wpedantic -rdynamic -std=c11 $^ -o $@

libmylog.a: mylog.o
	 ar rcs $@ $^

mylog.o: mylog.c
	 $(CC) -c $(CFLAGS) -Wall -Wextra -Wpedantic -rdynamic -std=c11 $^ -o $@

clean: 
	 rm -f hw05 libmylog.a core *.o

.PHONY: hw05 clean
