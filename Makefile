CFLAGS = -ggdb -fsigned-char -fno-builtin -Wall -Wextra -Werror -pedantic
CFILES = $(wildcard *.c)

all:
	@echo $(CFILES)
	@gcc -m32 $(CFLAGS) $(CFILES) -o shell

arm:
	@arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(CFLAGS) -c *.c
	@arm-none-eabi-ar r libinterp.a *.o
	@rm *.o
