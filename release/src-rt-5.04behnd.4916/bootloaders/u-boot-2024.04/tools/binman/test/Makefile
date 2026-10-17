#
# Builds test programs. This is launched from elf_test.BuildElfTestFiles()
#
# Copyright (C) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

HOSTARCH := $(shell uname -m | sed -e s/i.86/x86/ )
ifeq ($(findstring $(HOSTARCH),"x86" "x86_64"),)
ifeq ($(findstring $(MAKECMDGOALS),"help" "clean"),)
ifndef CROSS_COMPILE
$(error Binman tests need to compile to x86, but the CPU arch of your \
	machine is $(HOSTARCH). Set CROSS_COMPILE to a suitable cross compiler)
endif
endif
endif

CC		= $(CROSS_COMPILE)gcc
OBJCOPY		= $(CROSS_COMPILE)objcopy

VPATH := $(SRC)
CFLAGS := -march=i386 -m32 -nostdlib -I $(SRC)../../../include -I $(SRC) \
	-Wl,--no-dynamic-linker

LDS_UCODE := -T $(SRC)u_boot_ucode_ptr.lds
LDS_BINMAN := -T $(SRC)u_boot_binman_syms.lds
LDS_BINMAN_BAD := -T $(SRC)u_boot_binman_syms_bad.lds
LDS_BINMAN_X86 := -T $(SRC)u_boot_binman_syms_x86.lds
LDS_BINMAN_EMBED := -T $(SRC)u_boot_binman_embed.lds
LDS_EFL_SECTIONS := -T $(SRC)elf_sections.lds
LDS_BLOB := -T $(SRC)blob_syms.lds

TARGETS = u_boot_ucode_ptr u_boot_no_ucode_ptr bss_data bss_data_zero \
	u_boot_binman_syms u_boot_binman_syms.bin u_boot_binman_syms_bad \
	u_boot_binman_syms_size u_boot_binman_syms_x86 embed_data \
	u_boot_binman_embed u_boot_binman_embed_sm elf_sections blob_syms.bin

all: $(TARGETS)

u_boot_no_ucode_ptr: CFLAGS += $(LDS_UCODE)
u_boot_no_ucode_ptr: u_boot_no_ucode_ptr.c

u_boot_ucode_ptr: CFLAGS += $(LDS_UCODE)
u_boot_ucode_ptr: u_boot_ucode_ptr.c

bss_data: CFLAGS += $(SRC)bss_data.lds
bss_data: bss_data.c

bss_data_zero: CFLAGS += $(SRC)bss_data_zero.lds
bss_data_zero: bss_data_zero.c

embed_data: CFLAGS += $(SRC)embed_data.lds
embed_data: embed_data.c

u_boot_binman_syms.bin: u_boot_binman_syms
	$(OBJCOPY) -O binary $< -R .note.gnu.build-id $@

u_boot_binman_syms: CFLAGS += $(LDS_BINMAN)
u_boot_binman_syms: u_boot_binman_syms.c

u_boot_binman_syms_x86: CFLAGS += $(LDS_BINMAN_X86)
u_boot_binman_syms_x86: u_boot_binman_syms_x86.c

u_boot_binman_syms_bad: CFLAGS += $(LDS_BINMAN_BAD)
u_boot_binman_syms_bad: u_boot_binman_syms_bad.c

u_boot_binman_syms_size: CFLAGS += $(LDS_BINMAN)
u_boot_binman_syms_size: u_boot_binman_syms_size.c

u_boot_binman_embed: CFLAGS += $(LDS_BINMAN_EMBED)
u_boot_binman_embed: u_boot_binman_embed.c

u_boot_binman_embed_sm: CFLAGS += $(LDS_BINMAN_EMBED)
u_boot_binman_embed_sm: u_boot_binman_embed_sm.c

blob_syms.bin: blob_syms
	$(OBJCOPY) -O binary $< -R .note.gnu.build-id $@

blob_syms: CFLAGS += $(LDS_BLOB)
blob_syms: blob_syms.c

elf_sections: CFLAGS += $(LDS_EFL_SECTIONS)
elf_sections: elf_sections.c

clean:
	rm -f $(TARGETS)

help:
	@echo "Makefile for binman test programs"
	@echo
	@echo "Intended for use on x86 hosts"
	@echo
	@echo "Targets:"
	@echo
	@echo -e "\thelp	- Print help (this is it!)"
	@echo -e "\tall	- Builds test programs (default targget)"
	@echo -e "\tclean	- Delete output files"
