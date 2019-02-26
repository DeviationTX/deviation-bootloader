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

DISPLAY = 128x64x1_oled_ssd1306
TARGET = t8sgdfu
LDSCRIPT = devo.ld
LIBOPENCM3 = $(SDIR)/libopencm3/lib/libopencm3_stm32f1.a
DFU_ARGS    := -c 7 -b 0x08003000
PREFIX          ?= arm-none-eabi
CC              := $(PREFIX)-gcc
OBJCOPY         := $(PREFIX)-objcopy
OBJDUMP         := $(PREFIX)-objdump

CFLAGS = -Os -std=gnu99 -ggdb3 -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -fno-common -ffunction-sections -fdata-sections  -MD -Wall -Wundef -DSTM32F1 -Ilibopencm3//include

SRC    = usbdfu.c $(DISPLAY).c
OBJS   = $(addprefix objs/, $(SRC:.c=.o))

all: $(TARGET).dfu

$(TARGET).dfu: objs/$(TARGET).bin
	./utils/dfu.py --name "t8sg Bootloader Firmware" $(DFU_ARGS):$< $@
	./utils/get_mem_usage.pl objs/$(TARGET).map


objs/$(TARGET).elf: $(OBJS) src/hardware.h $(LIBOPENCM3)
	$(CC) --static -nostartfiles -Tsrc/$(LDSCRIPT) $(CFLAGS) -Wl,-Map=objs/$(TARGET).map -Wl,--cref -Wl,--gc-sections -Llibopencm3/lib $(OBJS) -lopencm3_stm32f1 -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -o $@

$(OBJS): objs/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.bin: %.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $< $@

$(LIBOPENCM3):
	test -s libopencm3/Makefile || { echo "Fetch libopencm3 via 'git submodule update --init'"; exit 1; }
	+$(FLOCKS) $(MAKE) -C libopencm3 TARGETS=stm32/f1 lib

