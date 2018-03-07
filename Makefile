#CC = gcc -m32
#AR = ar
CC = arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
AR = arm-none-eabi-ar

CFLAGS = -Wall -Wextra -Werror -pedantic \
	-Wno-discarded-qualifiers \
	-I. -fsigned-char -fno-builtin -ggdb

FILES = $(wildcard *.c)
OUTFILES = $(patsubst %.c, %.o, $(FILES))

all: $(OUTFILES)
	@#$(CC) $(CFLAGS) *.o -o shell
	@$(AR) r libinterp.a *.o

clean:
	@echo "  CLEAN"
	@rm -f *.o shell libinterp.a

%.o: %.c
	@echo "  CC     " $<
	@$(CC) $(CFLAGS) -c $< -o $@
