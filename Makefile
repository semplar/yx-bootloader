# Makefile skeleton adapted from maple-bootloader

BUILDDIR = build
TARGET = $(BUILDDIR)/yx-boot
ARCH = --target=armv7m-none-eabi -mthumb -mfpu=none

OPT ?= z
UNITY ?= 0

USB_LIB_DIR = usb_lib

INCDIRS = ./$(USB_LIB_DIR) ../libopencm3/include/
INCFLAGS = $(patsubst %,-I%,$(INCDIRS))

DEBUG = -g -gembed-source
CANDCPPFLAGS  = -O$(OPT) -ffunction-sections -fdata-sections -fmerge-all-constants
CANDCPPFLAGS += -Wall -Wimplicit -Wcast-align -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CANDCPPFLAGS += -DSTM32F1

CFLAGS  = $(CANDCPPFLAGS) $(DEBUG)
CFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-exceptions
#CFLAGS += -Wa,-adhlns=$(BUILDDIR)/$(subst $(suffix $<),.lst,$<)

CXXFLAGS  = $(CANDCPPFLAGS) $(DEBUG)

ASFLAGS = -Wa,-adhlns=$(BUILDDIR)/$(<:.s=.lst)#,--g$(DEBUG)

LDFLAGS  = -nostartfiles -Wl,-Map=$(TARGET).map,--cref,--gc-sections
LDFLAGS += 
LDFLAGS +=-Tbinary.ld

CLANG_DIR = /c/Program Files/ATfE-20.1.0/bin
TCHAIN = $(CLANG_DIR)/llvm

SHELL = sh
CC = "$(CLANG_DIR)/clang"
CPP = "$(CLANG_DIR)/clang++"
AR = "$(TCHAIN)-ar"
OBJCOPY = "$(TCHAIN)-objcopy"
OBJDUMP = "$(TCHAIN)-objdump"
SIZE = "$(TCHAIN)-size"
NM = "$(TCHAIN)-nm"
REMOVE = rm -f
REMOVEDIR = rm -r
COPY = cp

#GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

#STM32_LIB_SRC = uart.c
#STM32_LIB_SRC = 

#SRCS = usb.c usb_callbacks.c usb_descriptor.c main.c hardware.c dfu.c
#SRC_FILES += $(patsubst %, $(STM32_LIB_DIR)/%, $(STM32_LIB_SRC))
SRC_FILES += main.c hardware.c debug_uart.c usb.c

ASRC = $(filter %.s, $(SRC_FILES))
CSRC = $(filter %.c, $(SRC_FILES))
CXXSRC = $(filter %.cpp, $(SRC_FILES))

COBJ = $(patsubst %, $(BUILDDIR)/%,$(CSRC:.c=.o))
CXXOBJ = $(patsubst %, $(BUILDDIR)/%,$(CXXSRC:.cpp=.o))
AOBJ = $(patsubst %, $(BUILDDIR)/%,$(ASRC:.s=.o))
LST = $(patsubst %, $(BUILDDIR)/%,$(ASRC:.s=.lst) $(CSRC:.c=.lst) $(CXXSRC:.c=.lst))

HEXSIZE = $(SIZE) --target=binary $(TARGET).hex
ELFSIZE = $(SIZE) -A -x $(TARGET).elf | grep -v -e "\s0$$" -e "^$$" | head -n-1
BINSIZE = du -bsh $(TARGET).bin

$(AOBJ) : $(BUILDDIR)/%.o : %.s
	$(CC) -c $(ARCH) -x assembler-with-cpp $(ASFLAGS) $< -o $@

$(COBJ) : $(BUILDDIR)/%.o : %.c
	$(CC) -c $(ARCH) $(CFLAGS) $(INCFLAGS) $< -o $@

$(CXXOBJ) : $(BUILDDIR)/%.o : %.cpp
	$(CPP) -c $(ARCH) $(CXXFLAGS) $(INCFLAGS) $< -o $@

ifeq ($(UNITY),1)
$(TARGET).elf:
	$(CC) $(ARCH) $(CFLAGS) $(INCFLAGS) $(CSRC) $(CXXSRC) $(ASRC) $(LDFLAGS) --output $@
else
$(TARGET).elf: $(COBJ) $(AOBJ)
	$(CC) $(ARCH) $(AOBJ) $(COBJ) $(CXXOBJ) $(LDFLAGS) --output $@
endif

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).lss: $(TARGET).elf
	$(OBJDUMP) -h $< > $@
	$(OBJDUMP) -D -S -j .text $< >> $@

$(TARGET).sym: $(TARGET).elf
	$(NM) -n $< > $@

build: elf bin lss sym

clean:
	$(REMOVE) -r build

target_dir:
	@mkdir -p $(BUILDDIR)
#	@mkdir -p $(BUILDDIR)/$(STM32_LIB_DIR:../%=%)

elf: target_dir $(TARGET).elf

elfsize: elf
	@if [ -f $(TARGET).elf ]; then $(ELFSIZE); fi

bin: target_dir $(TARGET).bin

binsize: bin
	@if [ -f $(TARGET).bin ]; then $(BINSIZE); fi

lss: target_dir $(TARGET).lss

sym: target_dir $(TARGET).sym

all: build binsize

.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJ) $(COBJ) $(CXXOBJ)
.PHONY : all target_dir unity build elf elfsize hex bin binsize lss sym clean

#dfu: $(TARGET).bin
#	sudo dfu-util -d 0110:1001 -a 0 -D $(TARGET).bin
#
#tags:
#	etags `find . -name "*.c" -o -name "*.cpp" -o -name "*.h"`
#	@echo $(MSG_ETAGS)
#
#program:
#	@echo "Flash-programming with OpenOCD"
#	cp $(TARGET).bin flash/tmpflash.bin
#	cd flash && openocd -f flash.cfg
#
#program_serial:
#	@echo "Flash-programming with stm32loader.py"
#	./flash/stm32loader.py -p /dev/ttyUSB0 -evw build/maple_boot.bin
#
#debug: $(TARGET).bin
#	@echo "Flash-programming with OpenOCD - DEBUG"
#	cp $(TARGET).bin flash/tmpflash.bin
#	cd flash && openocd -f debug.cfg
#
#install: $(TARGET).bin
#	cp $(TARGET).bin build/main.bin
#	openocd -f flash/perry_flash.cfg
#
#run: $(TARGET).bin
#	openocd -f flash/run.cfg
#
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
#
#cscope:
#	rm -rf *.cscope
#	find . -iname "*.[hcs]" | grep -v examples | xargs cscope -R -b
#
#