##
# @file Makefile
# Script for compiling the interpreter library/shell
#
# Copyright (C) 2018 Clyne Sullivan
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

CFLAGS = -ggdb -fsigned-char -fno-builtin -Wall -Wextra -Werror -pedantic
CFILES = $(wildcard *.c)

all:
	@echo $(CFILES)
	@gcc -m32 $(CFLAGS) $(CFILES) -o shell

arm:
	@arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(CFLAGS) -c *.c
	@arm-none-eabi-ar r libinterp.a *.o
	@rm *.o
