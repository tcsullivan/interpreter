CFLAGS = -ggdb
CFILES = $(wildcard *.c)

all:
	@echo $(CFILES)
	@gcc -m32 $(CFLAGS) $(CFILES) -o shell

arm:
	@mv shell.c shell.c.bak
	@arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(CFLAGS) -c *.c
	@arm-none-eabi-ar r libinterp.a *.o
	@mv shell.c.bak shell.c
	@rm *.o
