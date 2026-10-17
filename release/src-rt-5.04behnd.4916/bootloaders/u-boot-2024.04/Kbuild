# SPDX-License-Identifier: GPL-2.0
#
# Kbuild for top-level directory of U-Boot

#####
# Generate generic-asm-offsets.h

generic-offsets-file := include/generated/generic-asm-offsets.h

always  := $(generic-offsets-file)
targets := lib/asm-offsets.s

CFLAGS_REMOVE_asm-offsets.o := $(LTO_CFLAGS)

$(obj)/$(generic-offsets-file): $(obj)/lib/asm-offsets.s FORCE
	$(call filechk,offsets,__GENERIC_ASM_OFFSETS_H__)

#####
# Generate asm-offsets.h

ifneq ($(wildcard $(srctree)/arch/$(ARCH)/lib/asm-offsets.c),)
offsets-file := include/generated/asm-offsets.h
endif

always  += $(offsets-file)
targets += arch/$(ARCH)/lib/asm-offsets.s

CFLAGS_asm-offsets.o := -DDO_DEPS_ONLY

$(obj)/$(offsets-file): $(obj)/arch/$(ARCH)/lib/asm-offsets.s FORCE
	$(call filechk,offsets,__ASM_OFFSETS_H__)
