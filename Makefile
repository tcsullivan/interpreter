CC = gcc -m32

CFLAGS = -Wall -Wextra -Wno-strict-aliasing -I. -ggdb -fno-builtin

all:
	$(CC) $(CFLAGS) -c shelpers.c
	$(CC) $(CFLAGS) -c parser.c
	$(CC) $(CFLAGS) -c builtins.c
	$(CC) $(CFLAGS) -c stack.c
	$(CC) $(CFLAGS) -c ops.c
	$(CC) $(CFLAGS) -c variable.c
	$(CC) $(CFLAGS) shell.c *.o -o shell
