##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY = usbdfu
LDSCRIPT = stm32f1.ld
LIBOPENCM3 = $(SDIR)/libopencm3/lib/libopencm3_stm32f1.a

PREFIX          ?= arm-none-eabi
CC              := $(PREFIX)-gcc
OBJCOPY         := $(PREFIX)-objcopy
OBJDUMP         := $(PREFIX)-objdump

CFLAGS = -Os -std=gnu99 -ggdb3 -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -fno-common -ffunction-sections -fdata-sections  -MD -Wall -Wundef -DSTM32F1 -Ilibopencm3//include


all: usbdfu.bin

usbdfu.elf: usbdfu.o spi_flash.o 128x64x1_nt7538.o hardware.h $(LIBOPENCM3)
	$(CC) --static -nostartfiles -T$(LDSCRIPT) $(CFLAGS) -Wl,-Map=usbdfu.map -Wl,--cref -Wl,--gc-sections -Llibopencm3//lib usbdfu.o spi_flash.o 128x64x1_nt7538.o -lopencm3_stm32f1 -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -o $@

%.o: %.c hardware.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.bin: %.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(*).elf $(*).bin

$(LIBOPENCM3):
	test -s libopencm3/Makefile || { echo "Fetch libopencm3 via 'git submodule update --init'"; exit 1; }
	+$(FLOCKS) $(MAKE) -C libopencm3 TARGETS=stm32/f1 lib

