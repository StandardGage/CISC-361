CC = gcc
CFLAGS = -Wall -Wextra -g
LFLAGS = -lreadline -lhistory

desh: main.c builtin.c
	$(CC) $(CFLAGS) -o desh main.c builtin.c $(LFLAGS)

.PHONY: clean
clean:
	rm -f desh

.PHONY: valgrind
valgrind: desh
	valgrind ./desh