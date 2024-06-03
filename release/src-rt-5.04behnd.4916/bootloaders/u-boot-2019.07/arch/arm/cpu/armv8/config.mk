# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
PLATFORM_RELFLAGS += -fno-common -ffixed-x18

PF_NO_UNALIGNED := $(call cc-option, -mstrict-align)
PLATFORM_CPPFLAGS += $(PF_NO_UNALIGNED)

EFI_LDS := elf_aarch64_efi.lds
EFI_CRT0 := crt0_aarch64_efi.o
EFI_RELOC := reloc_aarch64_efi.o
