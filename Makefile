CC = gcc -m32

CFLAGS = -Wall -Wextra -I. -ggdb -fno-builtin

all:
	$(CC) $(CFLAGS) -c parser.c
	$(CC) $(CFLAGS) shell.c parser.o -o shell
