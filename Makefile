###################################################################################
##	
##						Copyright 2009-2013 xxxxxxx, xxxxxxx
##	File:	Makefile
##	Author:	Bala B. (bhat.balasubramanya@gmail.com)
##	Description: Makefile for OS files
##
###################################################################################

ASM=arm-elf-as
CC:=arm-elf-gcc
LINK:=arm-elf-ld
LIBPATH:=/usr/local/dev-arm/i386-Darwin-arm-gcc-4.6.1/lib/gcc/arm-elf/4.6.1

## Initialize default arguments
TARGET		?=	tq2440
DST			?=	build
CONFIG		?=	debug

## Initialize dependent parameters
ifeq ($(TARGET), tq2440)
	SOC := s3c2440
endif

ifeq ($(SOC), s3c2440)
	CORE := arm920t
endif

BUILD_DIR		:=	$(DST)/$(CONFIG)-$(TARGET)
MAP_FILE		:=	$(BUILD_DIR)/$(TARGET).map
LINKERS_CRIPT	:=	scripts/$(TARGET)/memmap.ld
DEP_DIR			:=	$(BUILD_DIR)/dep
OBJ_DIR			:=	$(BUILD_DIR)/obj
BUILD_TARGET	:=	$(BUILD_DIR)/$(TARGET).elf
BOOT_TARGET		:=	$(BUILD_DIR)/boot.elf

## Create INCLUDES 
include $(wildcard includes/*.mk)
INCLUDES		:=	$(addprefix -I ,includes $(INCLUDES))

## Build list of source and object files
SUBDIRS			:=	sources main
SOURCES			:=	$(wildcard *.c)

## Include Boot source files
include $(wildcard boot/$(TARGET)/*.mk)

## Include sources from other directories
include $(foreach sdir, $(SUBDIRS), $(wildcard $(sdir)/*.mk))

## Build a list of corresponding object files
OBJS			:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(SOURCES))))
BOOT_OBJS		:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(BOOT_SOURCES))))

## Build flags
AFLAGS		:=	-mcpu=arm920t -EL -g --defsym NOR_BOOT=1
CFLAGS		:=	-Wall -nostdinc -mcpu=$(CORE) -mlittle-endian
LDFLAGS		:=	-nostartfiles -nostdlib -T$(LINKERS_CRIPT) -Map $(MAP_FILE) -L $(LIBPATH) -lgcc
ifeq ($(CONFIG),debug)
	CFLAGS	:=	-g -O0 -D DEBUG $(CFLAGS)
else ifeq ($(CONFIG),release)
	CFLAGS	:=	-O2 -D RELEASE $(CFLAGS)
endif


## Rule specifications
.PHONY:	all boot dep clean

all:
	@echo --------------------------------------------------------------------------------
	@echo Starting build with following parameters:
	@echo --------------------------------------------------------------------------------
	@echo TARGET=$(TARGET) 
	@echo SOC=$(SOC)
	@echo CONFIG=$(CONFIG)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OBJ_DIR=$(OBJ_DIR)
	@echo MAP_FILE=$(MAP_FILE)
	@echo SOURCES=$(SOURCES)
	@echo OBJS=$(OBJS)
	@echo INCLUDES=$(INCLUDES)
	@echo
	make $(BUILD_TARGET)

boot:
	@echo --------------------------------------------------------------------------------
	@echo Starting build with following parameters:
	@echo --------------------------------------------------------------------------------
	@echo TARGET=$(TARGET) 
	@echo SOC=$(SOC)
	@echo CONFIG=$(CONFIG)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OBJ_DIR=$(OBJ_DIR)
	@echo BOOT_SOURCES=$(BOOT_SOURCES)
	@echo BOOT_OBJS=$(BOOT_OBJS)
	@echo
	make $(BOOT_TARGET)

$(OBJ_DIR)/%.o: %.s
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(ASM) $(AFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(BUILD_TARGET): $(OBJS) $(OS_TARGET)
	$(LINK) $^ $(LDFLAGS) -o $@

$(BOOT_TARGET): $(BOOT_OBJS)
	$(LINK) -nostartfiles -nostdlib -Ttext 0x00000000 $(BOOT_OBJS) -o $@

clean:
	rm -rf $(DST)

## Validate the arguments for build
ifneq ($(CONFIG),debug)
	ifneq ($(CONFIG),release)
		$(error CONFIG should be either debug or release)
	endif
endif

ifeq ($(TARGET),)
	$(error Missing TARGET specification)
endif
ifeq ($(SOC),)
	$(error Missing SOC specification)
endif
ifeq ($(CORE),)
	$(error Missing CORE specification)
endif
