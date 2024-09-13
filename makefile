CC = gcc
CFLAGS = -Wall -Wextra -g

slist: main.c student.h
	$(CC) $(CFLAGS) -o slist main.c

.PHONY: clean
clean:
	rm -f slist

.PHONY: valgrind
valgrind: slist
	valgrind ./slist