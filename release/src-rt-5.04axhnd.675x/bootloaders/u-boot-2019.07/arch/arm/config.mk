# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_STANDALONE_LOAD_ADDR
ifneq ($(CONFIG_ARCH_OMAP2PLUS),)
CONFIG_STANDALONE_LOAD_ADDR = 0x80300000
else
CONFIG_STANDALONE_LOAD_ADDR = 0xc100000
endif
endif

CFLAGS_NON_EFI := -fno-pic -ffixed-r9 -ffunction-sections -fdata-sections
CFLAGS_EFI := -fpic -fshort-wchar

LDFLAGS_FINAL += --gc-sections
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections \
		     -fno-common -ffixed-r9
PLATFORM_RELFLAGS += $(call cc-option, -msoft-float) \
      $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,))

# LLVM support
LLVM_RELFLAGS		:= $(call cc-option,-mllvm,) \
			$(call cc-option,-mno-movt,)
PLATFORM_RELFLAGS	+= $(LLVM_RELFLAGS)

PLATFORM_CPPFLAGS += -D__ARM__

ifdef CONFIG_ARM64
PLATFORM_ELFFLAGS += -B aarch64 -O elf64-littleaarch64
else
PLATFORM_ELFFLAGS += -B arm -O elf32-littlearm
endif

# Choose between ARM/Thumb instruction sets
ifeq ($(CONFIG_$(SPL_)SYS_THUMB_BUILD),y)
AFLAGS_IMPLICIT_IT	:= $(call as-option,-Wa$(comma)-mimplicit-it=always)
PF_CPPFLAGS_ARM		:= $(AFLAGS_IMPLICIT_IT) \
			$(call cc-option, -mthumb -mthumb-interwork,\
			$(call cc-option,-marm,)\
			$(call cc-option,-mno-thumb-interwork,)\
		)
else
PF_CPPFLAGS_ARM := $(call cc-option,-marm,) \
		$(call cc-option,-mno-thumb-interwork,)
endif

# Only test once
ifeq ($(CONFIG_$(SPL_)SYS_THUMB_BUILD),y)
archprepare: checkthumb checkgcc6

checkthumb:
	@if test "$(call cc-name)" = "gcc" -a \
			"$(call cc-version)" -lt "0404"; then \
		echo -n '*** Your GCC does not produce working '; \
		echo 'binaries in THUMB mode.'; \
		echo '*** Your board is configured for THUMB mode.'; \
		false; \
	fi
else
archprepare: checkgcc6
endif

checkgcc6:
	@if test "$(call cc-name)" = "gcc" -a \
			"$(call cc-version)" -lt "0600"; then \
		echo '*** Your GCC is older than 6.0 and is not supported'; \
		false; \
	fi


# Try if EABI is supported, else fall back to old API,
# i. e. for example:
# - with ELDK 4.2 (EABI supported), use:
#	-mabi=aapcs-linux
# - with ELDK 4.1 (gcc 4.x, no EABI), use:
#	-mabi=apcs-gnu
# - with ELDK 3.1 (gcc 3.x), use:
#	-mapcs-32
PF_CPPFLAGS_ABI := $(call cc-option,\
			-mabi=aapcs-linux,\
			$(call cc-option,\
				-mapcs-32,\
				$(call cc-option,\
					-mabi=apcs-gnu,\
				)\
			)\
		)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_ARM) $(PF_CPPFLAGS_ABI)

# For EABI, make sure to provide raise()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
# This file is parsed many times, so the string may get added multiple
# times. Also, the prefix needs to be different based on whether
# CONFIG_SPL_BUILD is defined or not. 'filter-out' the existing entry
# before adding the correct one.
PLATFORM_LIBS := arch/arm/lib/eabi_compat.o \
	$(filter-out arch/arm/lib/eabi_compat.o, $(PLATFORM_LIBS))
endif

# needed for relocation
LDFLAGS_u-boot += -pie

#
# FIXME: binutils versions < 2.22 have a bug in the assembler where
# branches to weak symbols can be incorrectly optimized in thumb mode
# to a short branch (b.n instruction) that won't reach when the symbol
# gets preempted
#
# http://sourceware.org/bugzilla/show_bug.cgi?id=12532
#
ifeq ($(CONFIG_$(SPL_)SYS_THUMB_BUILD),y)
ifeq ($(GAS_BUG_12532),)
export GAS_BUG_12532:=$(shell if [ $(call binutils-version) -lt 0222 ] ; \
	then echo y; else echo n; fi)
endif
ifeq ($(GAS_BUG_12532),y)
PLATFORM_RELFLAGS += -fno-optimize-sibling-calls
endif
endif

ifneq ($(CONFIG_SPL_BUILD),y)
# Check that only R_ARM_RELATIVE relocations are generated.
ALL-y += checkarmreloc
# The movt / movw can hardcode 16 bit parts of the addresses in the
# instruction. Relocation is not supported for that case, so disable
# such usage by requiring word relocations.
PLATFORM_CPPFLAGS += $(call cc-option, -mword-relocations)
PLATFORM_CPPFLAGS += $(call cc-option, -fno-pic)
endif

# limit ourselves to the sections we want in the .bin.
ifdef CONFIG_ARM64
OBJCOPYFLAGS += -j .text -j .secure_text -j .secure_data -j .rodata -j .data \
		-j .u_boot_list -j .rela.dyn -j .got -j .got.plt \
		-j .binman_sym_table -j .text_rest
else
OBJCOPYFLAGS += -j .text -j .secure_text -j .secure_data -j .rodata -j .hash \
		-j .data -j .got -j .got.plt -j .u_boot_list -j .rel.dyn \
		-j .binman_sym_table -j .text_rest
endif

# if a dtb section exists we always have to include it
# there are only two cases where it is generated
# 1) OF_EMBEDED is turned on
# 2) unit tests include device tree blobs
OBJCOPYFLAGS += -j .dtb.init.rodata

ifdef CONFIG_EFI_LOADER
OBJCOPYFLAGS += -j .efi_runtime -j .efi_runtime_rel
endif

ifneq ($(CONFIG_IMX_CONFIG),)
ifdef CONFIG_SPL
ifndef CONFIG_SPL_BUILD
ALL-y += SPL
endif
else
ifeq ($(CONFIG_OF_SEPARATE),y)
ALL-y += u-boot-dtb.imx
else
ALL-y += u-boot.imx
endif
endif
ifneq ($(CONFIG_VF610),)
ALL-y += u-boot.vyb
endif
endif

EFI_LDS := elf_arm_efi.lds
EFI_CRT0 := crt0_arm_efi.o
EFI_RELOC := reloc_arm_efi.o
