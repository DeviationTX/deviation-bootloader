MAKEFLAGS += --no-builtin-rules

TARGET  = dbl

LIBOPENCM3 = libopencm3/lib/libopencm3_stm32f1.a

SRC_C    = $(wildcard src/*.c)

CFLAGS_usb_isr = -fno-lto

###############################################
#This section defines binaries needed to build#
###############################################
    CROSS = arm-none-eabi-
    CC   = $(CROSS)gcc
    CXX  = $(CROSS)g++
    LD   = $(CROSS)ld
    AR   = $(CROSS)ar
    AS   = $(CROSS)as
    CP   = $(CROSS)objcopy
    DUMP = $(CROSS)objdump
    NM   = $(CROSS)nm
###############################################
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

############################################
#This section intermediate build files     #
############################################
ODIR	= objs/$(TARGET)
OBJS	= $(addprefix $(ODIR)/, $(notdir $(SRC_C:.c=.o) $(SRC_S:.s=.o)))


CFLAGS   = -DSTM32F1 \
           -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd\
           -fdata-sections -ffunction-sections -fno-builtin-printf \
           -flto -ffat-lto-objects -fwhole-program \
           -Wall -Wextra -std=gnu99 --specs=nano.specs \
           -Ilibopencm3/include

LFLAGS = --static -nostartfiles -Tld/$(TARGET).ld -Wl,-Map=$(TARGET).map \
         -Wl,--gc-sections -L../libopencm3/lib/ -L../libopencm3/lib/stm32/f1/ \
         -lopencm3_stm32f1 -Wl,--start-group -lc -lnosys -Wl,--end-group

all: $(LIBOPENCM3) $(TARGET).dfu

####################################
# recompile if the Makefile changes#
####################################
$(OBJS): Makefile


##################################################################################
# The following enables quiet output unless you use VERBOSE=1                    #
# Note that this must be after the 1st rule so that it doesn't execute by default#
##################################################################################
$(VERBOSE).SILENT:

.PHONY: clean

##########################################
#Ensure necessray directories are created#
##########################################
$(OBJS): | $(ODIR)

$(ODIR):
	@mkdir -p $@

######################
#The main executable#
######################
$(TARGET).elf: $(LINKFILE) $(OBJS) $(LIBOPENCM3) ld/$(TARGET).ld
	@echo " + Building '$@'"
	$(CC) -o $@ $(OBJS) $(LIBOPENCM3) $(LFLAGS) $(CFLAGS)

$(TARGET).dfu: $(TARGET).elf
	$(CP) -O binary $(TARGET).elf $(TARGET).bin
	$(DUMP) -d $(TARGET).elf > $(TARGET).list
	utils/dfu.py -b 0x08000000:$(TARGET).bin $(TARGET).dfu
	utils/get_mem_usage.pl $(TARGET).map

##############################
#Build rules for all .o files#
##############################
## The autodependency magic below was adapeted from:
## http://mad-scientist.net/make/autodep.html
-include $(OBJS:.o=.P)
-include $(PROTO_OBJS:.o=.P)
-include $(PROTO_EXTRA_OBJS:.o=.P)

dollar = $$
define define_compile_rules
$(ODIR)/%.o: $(1)%.c $(LIBOPENCM3)
	@echo " + Compiling '$$<'"
	$(CC) $$(CFLAGS) $$(CFLAGS_$$(basename $$(notdir $$<))) -MD -c -o $$@ $$<
	@cp $(ODIR)/$$*.d $(ODIR)/$$*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$(dollar)//' \
                -e '/^$$(dollar)/ d' -e 's/$$(dollar)/ :/' < $(ODIR)/$$*.d >> $(ODIR)/$$*.P; \
            rm -f $(ODIR)/$$*.d
endef
$(foreach directory,$(sort $(dir $(SRC_C) $(PROTO_EXTRA_C))),$(eval $(call define_compile_rules,$(directory))))

$(LIBOPENCM3):
	test -s libopencm3/Makefile || { echo "Fetch libopencm3 via 'git submodule update --init'"; exit 1; }
	make -C libopencm3 TARGETS=stm32/f1 lib

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).dfu $(TARGET).list \
		$(TARGET).map $(ODIR)/*.o $(ODIR)/*.o_ $(ODIR)/*.P  $(ODIR)/*.bin \
		2> /dev/null || true
