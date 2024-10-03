CC = gcc
CFLAGS = -Wall -Wextra -g
LFLAGS = -lreadline -lhistory

shell: main.c builtin.c
	$(CC) $(CFLAGS) -o shell main.c builtin.c $(LFLAGS)

.PHONY: clean
clean:
	rm -f shell

.PHONY: valgrind
valgrind: shell
	valgrind ./shell