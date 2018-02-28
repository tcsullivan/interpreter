#CC = gcc -m32
#AR = ar
CC = arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
AR = arm-none-eabi-ar

CFLAGS = -Wall -Wextra -Werror -pedantic \
	-Wno-discarded-qualifiers \
	-I. -fsigned-char -fno-builtin -ggdb

all:
	$(CC) $(CFLAGS) -c shelpers.c
	$(CC) $(CFLAGS) -c parser.c
	$(CC) $(CFLAGS) -c builtins.c
	$(CC) $(CFLAGS) -c stack.c
	$(CC) $(CFLAGS) -c ops.c
	$(CC) $(CFLAGS) -c variable.c
	$(AR) r libinterp.a *.o
	@rm -f *.o
	#$(CC) $(CFLAGS) shell.c *.o -o shell #-L. -l interp
