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

#======================= START OF SETTINGS ==========================
#The following settings need to be updated for each transmitter

# TVXER: Unique string used by Deviation to identify transmitter
#        Known Values: T8SGV1, T8SGV2, T8SGV2+, T8SGV3, T8SGV3+,
#                      DEVO7E, DEVO7E-256
TXVER = "T8SGV2"

# ROMSIZE: Size of FLASH rom.  Valid values 128, 256
ROMSIZE = 256

# TWO_STAGE: Whether to load the bootloader after the Walkera bootloader
#            This should only be used for testing as it requires
#            modifications to deviation to support.
#            The default value is 0, and should only be changed to 1
#            if you really know what you are doing
TWO_STAGE = 0

# DISPLAY_TYPE: Type of Display.  Valid values 'OLED' or 'LCD'
# This value can be auto-detected based on the value of TXVER
# DISPLAY_TYPE = "LCD"

# HAS_LCD_FLIPPED:  Whether LCD is flipped.  Vlaid values are 1 or 0
# This value can be auto-detected based on the value of TXVER
# HAS_LCD_FLIPPED = 1
#======================= END OF SETTINGS ==========================


ifeq ($(origin DISPLAY_TYPE), undefined)
    ifeq ($(findstring +, $(TXVER)), +)
        DISPLAY_TYPE = "OLED"
    else
        DISPLAY_TYPE = "LCD"
    endif
endif

ifeq ($(DISPLAY_TYPE),"OLED")
    DISPLAY = 128x64x1_oled_ssd1306
else
    DISPLAY = 128x64x1_nt7538
endif

ifeq ($(findstring DEVO7E, $(TXVER)), DEVO7E)
    HAS_LCD_FLIPPED=0
    MANUFACTURER="Walkera"
else
    HAS_LCD_FLIPPED=1
    MANUFACTURER="Jumper"
endif

PREFIX          ?= arm-none-eabi
CC              := $(PREFIX)-gcc
OBJCOPY         := $(PREFIX)-objcopy
OBJDUMP         := $(PREFIX)-objdump

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

TXVERlc = $(patsubst "%",%,$(call lc,$(TXVER)))
TARGET = bootloader-$(TXVERlc)
LIBOPENCM3 = libopencm3/lib/libopencm3_stm32f1.a

DFU_ENCRYPT_VAL  := 7

ifeq ($(TWO_STAGE),1)
    LOAD_ADDRESS := 0x08003000
    LDSCRIPT     := two_stage.ld
    TARGETS      := $(TARGET).dfu
    EXT_BUTTON   := 0
else
    INST_ADDRESS := 0x08003000
    LOAD_ADDRESS := 0x08000000
    LDSCRIPT     := default.ld
    LDSCRIPT_L   := two_stage.ld
    TARGETS      := $(TARGET).bin installer-enc-$(TARGET).dfu installer-$(TARGET).dfu
    EXT_BUTTON   := 1
endif

ODIR = objs/$(TXVERlc)
CFLAGS = -D'TXVER=$(TXVER)' -DROMSIZE=$(ROMSIZE) -DLOAD_ADDRESS=$(LOAD_ADDRESS) \
         -DHAS_LCD_FLIPPED=$(HAS_LCD_FLIPPED) -DUSE_EXT_BUTTON=$(EXT_BUTTON) -D'MANUFACTURER=$(MANUFACTURER)' \
         -Os -std=gnu99 -ggdb3 -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd \
         -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls \
         -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef \
         -DSTM32F1 -Ilibopencm3//include -flto

SRC    = usbdfu.c init.c periph_init.c spi_flash.c $(DISPLAY).c
SRC_L  = loader.c bootloader_data.S
OBJS   = $(addprefix $(ODIR)/, $(SRC:.c=.o))
OBJS_L = $(ODIR)/loader.o $(ODIR)/bootloader_data.o 

all: $(TARGETS)

clean:
	rm $(ODIR)/* 2> /dev/null || /bin/true

distclean:
	rm objs/* 2> /dev/null || /bin/true
	rm *.dfu *.bin 2> /dev/null || /bin/true

$(OBJS): | $(ODIR)

$(ODIR):
	@mkdir -p $@

$(TARGET).dfu: $(ODIR)/$(TARGET).bin
	./utils/dfu.py --name "$(TXVERlc) Bootloader Firmware" -c $(DFU_ENCRYPT_VAL) -b $(LOAD_ADDRESS):$< $@
	./utils/get_mem_usage.pl $(ODIR)/$(TARGET).map

$(ODIR)/$(TARGET).elf: $(OBJS) src/hardware.h $(LIBOPENCM3)
	$(CC) --static -nostartfiles -Tsrc/$(LDSCRIPT) $(CFLAGS) -Wl,-Map=$(ODIR)/$(TARGET).map -Wl,--cref -Wl,--gc-sections -Llibopencm3/lib $(OBJS) -lopencm3_stm32f1 -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -o $@

$(OBJS): $(ODIR)/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<


%.bin: %.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $< $@

#################
## options for compiling bootloader-installer

$(ODIR)/loader.elf: $(OBJS_L) src/hardware.h $(LIBOPENCM3)
	$(CC) --static -nostartfiles -Tsrc/loader.ld $(CFLAGS) -Wl,-Map=$(ODIR)/loader.map -Wl,--cref -Wl,--gc-sections -Llibopencm3/lib $(OBJS_L) -lopencm3_stm32f1 -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -o $@

$(ODIR)/bootloader_data.o: src/bootloader_data.S Makefile $(ODIR)/$(TARGET).bin
	/bin/sed -e 's,@file@,$(ODIR)/$(TARGET).bin,' < $< | $(CC) $(CFLAGS) -x assembler-with-cpp - -o $@ -c

$(ODIR)/loader.o: src/loader.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

installer-$(TARGET).dfu: $(ODIR)/loader.bin
	./utils/dfu.py --name "$(TXVERlc) Bootloader Installer" -c 0 -b $(INST_ADDRESS):$< $@

installer-enc-$(TARGET).dfu: $(ODIR)/loader.bin
	./utils/dfu.py --name "$(TXVERlc) Bootloader Installer" -c $(DFU_ENCRYPT_VAL) -b $(INST_ADDRESS):$< $@

$(TARGET).bin: $(ODIR)/$(TARGET).bin
	cp -pf $< $@

#################

$(LIBOPENCM3):
	test -s libopencm3/Makefile || { echo "Fetch libopencm3 via 'git submodule update --init'"; exit 1; }
	+$(FLOCKS) $(MAKE) -C libopencm3 TARGETS=stm32/f1 FP_FLAGS="-flto" lib

