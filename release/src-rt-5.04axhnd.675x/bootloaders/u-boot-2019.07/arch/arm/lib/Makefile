# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

lib-$(CONFIG_USE_PRIVATE_LIBGCC) += ashldi3.o ashrdi3.o lshrdi3.o \
				    lib1funcs.o uldivmod.o div0.o \
				    div64.o muldi3.o

ifdef CONFIG_CPU_V7M
obj-y	+= vectors_m.o crt0.o
else ifdef CONFIG_ARM64
obj-y	+= crt0_64.o
else
obj-y	+= vectors.o crt0.o
endif

ifdef CONFIG_ARM64
obj-y   += setjmp_aarch64.o
else
obj-y   += setjmp.o
endif

ifndef CONFIG_SPL_BUILD
ifdef CONFIG_ARM64
obj-y	+= relocate_64.o
else
obj-y	+= relocate.o
endif

obj-$(CONFIG_CPU_V7M) += cmd_boot.o
obj-$(CONFIG_OF_LIBFDT) += bootm-fdt.o
obj-$(CONFIG_CMD_BOOTI) += bootm.o image.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o
obj-$(CONFIG_CMD_BOOTZ) += bootm.o zimage.o
obj-$(CONFIG_SYS_L2_PL310) += cache-pl310.o
else
obj-$(CONFIG_SPL_FRAMEWORK) += spl.o
obj-$(CONFIG_SPL_FRAMEWORK) += zimage.o
obj-$(CONFIG_OF_LIBFDT) += bootm-fdt.o
endif
obj-$(CONFIG_$(SPL_TPL_)USE_ARCH_MEMSET) += memset.o
obj-$(CONFIG_$(SPL_TPL_)USE_ARCH_MEMCPY) += memcpy.o
obj-$(CONFIG_SEMIHOSTING) += semihosting.o

obj-y	+= sections.o
obj-y	+= stack.o
ifdef CONFIG_CPU_V7M
obj-y	+= interrupts_m.o
else ifdef CONFIG_ARM64
obj-$(CONFIG_FSL_LAYERSCAPE) += ccn504.o
ifneq ($(CONFIG_GICV2)$(CONFIG_GICV3),)
obj-y	+= gic_64.o
endif
obj-y	+= interrupts_64.o
else
obj-y	+= interrupts.o
endif
ifndef CONFIG_SYSRESET
obj-y	+= reset.o
endif

obj-y	+= cache.o
obj-$(CONFIG_SYS_ARM_CACHE_CP15)	+= cache-cp15.o

obj-y	+= psci-dt.o

obj-$(CONFIG_DEBUG_LL)	+= debug.o

# For EABI conformant tool chains, provide eabi_compat()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
extra-y	+= eabi_compat.o
endif

# some files can only build in ARM or THUMB2, not THUMB1

ifdef CONFIG_$(SPL_)SYS_THUMB_BUILD
asflags-$(CONFIG_HAS_THUMB2) += -DCONFIG_THUMB2_KERNEL
ifndef CONFIG_HAS_THUMB2

# for C files, just apend -marm, which will override previous -mthumb*

ifndef CONFIG_ARM64
CFLAGS_cache.o := -marm
CFLAGS_cache-cp15.o := -marm
endif

# For .S, drop -mthumb* and other thumb-related options.
# CFLAGS_REMOVE_* would not have an effet, so AFLAGS_REMOVE_*
# was implemented and is used here.
# Also, define ${target}_NO_THUMB_BUILD for these two targets
# so that the code knows it should not use Thumb.

AFLAGS_REMOVE_memset.o := -mthumb -mthumb-interwork
AFLAGS_REMOVE_memcpy.o := -mthumb -mthumb-interwork
AFLAGS_memset.o := -DMEMSET_NO_THUMB_BUILD
AFLAGS_memcpy.o := -DMEMCPY_NO_THUMB_BUILD
endif
endif

# For building EFI apps
CFLAGS_$(EFI_CRT0) := $(CFLAGS_EFI)
CFLAGS_REMOVE_$(EFI_CRT0) := $(CFLAGS_NON_EFI)

CFLAGS_$(EFI_RELOC) := $(CFLAGS_EFI)
CFLAGS_REMOVE_$(EFI_RELOC) := $(CFLAGS_NON_EFI)

extra-$(CONFIG_CMD_BOOTEFI_HELLO_COMPILE) += $(EFI_CRT0) $(EFI_RELOC)
# TODO: As of v2019.01 the relocation code for the EFI application cannot
# be built on ARMv7-M.
ifndef CONFIG_CPU_V7M
#extra-$(CONFIG_CMD_BOOTEFI_SELFTEST) += $(EFI_CRT0) $(EFI_RELOC)
endif
extra-$(CONFIG_EFI) += $(EFI_CRT0) $(EFI_RELOC)
