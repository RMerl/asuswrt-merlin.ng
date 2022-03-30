#
# Builds test programs
#
# Copyright (C) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

CFLAGS := -march=i386 -m32 -nostdlib -I ../../../include

LDS_UCODE := -T u_boot_ucode_ptr.lds
LDS_BINMAN := -T u_boot_binman_syms.lds
LDS_BINMAN_BAD := -T u_boot_binman_syms_bad.lds

TARGETS = u_boot_ucode_ptr u_boot_no_ucode_ptr bss_data \
	u_boot_binman_syms u_boot_binman_syms.bin u_boot_binman_syms_bad \
	u_boot_binman_syms_size

all: $(TARGETS)

u_boot_no_ucode_ptr: CFLAGS += $(LDS_UCODE)
u_boot_no_ucode_ptr: u_boot_no_ucode_ptr.c

u_boot_ucode_ptr: CFLAGS += $(LDS_UCODE)
u_boot_ucode_ptr: u_boot_ucode_ptr.c

bss_data: CFLAGS += bss_data.lds
bss_data: bss_data.c

u_boot_binman_syms.bin: u_boot_binman_syms
	objcopy -O binary $< -R .note.gnu.build-id $@

u_boot_binman_syms: CFLAGS += $(LDS_BINMAN)
u_boot_binman_syms: u_boot_binman_syms.c

u_boot_binman_syms_bad: CFLAGS += $(LDS_BINMAN_BAD)
u_boot_binman_syms_bad: u_boot_binman_syms_bad.c

u_boot_binman_syms_size: CFLAGS += $(LDS_BINMAN)
u_boot_binman_syms_size: u_boot_binman_syms_size.c

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
