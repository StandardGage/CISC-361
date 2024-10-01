CC = gcc
CFLAGS = -Wall -Wextra -g

shell: main.c
	$(CC) $(CFLAGS) -o shell main.c -lreadline -lhistory

.PHONY: clean
clean:
	rm -f shell

.PHONY: valgrind
valgrind: shell
	valgrind ./shell