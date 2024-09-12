CC = gcc
CFLAGS = -Wall -Wextra -g

main: main.c student.h
	$(CC) $(CFLAGS) -o main main.c

.PHONY: clean
clean:
	rm -f main

.PHONY: valgrind
valgrind: main
	valgrind ./main