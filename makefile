CC = gcc
CFLAGS = -Wall -Wextra -g

shell: main.c
	$(CC) $(CFLAGS) -o shell main.c builtin.c

.PHONY: clean
clean:
	rm -f shell

.PHONY: valgrind
valgrind: shell
	valgrind ./shell