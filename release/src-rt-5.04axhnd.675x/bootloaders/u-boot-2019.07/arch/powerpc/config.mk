# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := ppc_8xx-
endif

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000
LDFLAGS_FINAL += --gc-sections
LDFLAGS_FINAL += --bss-plt
PLATFORM_RELFLAGS += -fpic -mrelocatable -ffunction-sections \
-fdata-sections -mcall-linux

PF_CPPFLAGS_POWERPC	:= $(call cc-option,-fno-ira-hoist-pressure,)
PLATFORM_CPPFLAGS += -D__powerpc__ -ffixed-r2 -m32 $(PF_CPPFLAGS_POWERPC)
PLATFORM_LDFLAGS  += -m32 -melf32ppclinux

#
# When cross-compiling on NetBSD, we have to define __PPC__ or else we
# will pick up a va_list declaration that is incompatible with the
# actual argument lists emitted by the compiler.
#
# [Tested on NetBSD/i386 1.5 + cross-powerpc-netbsd-1.3]

ifeq ($(CROSS_COMPILE),powerpc-netbsd-)
PLATFORM_CPPFLAGS+= -D__PPC__
endif
ifeq ($(CROSS_COMPILE),powerpc-openbsd-)
PLATFORM_CPPFLAGS+= -D__PPC__
endif

# Only test once
ifneq ($(CONFIG_SPL_BUILD),y)
archprepare: checkgcc4

# GCC 3.x is reported to have problems generating the type of relocation
# that U-Boot wants.
# See http://lists.denx.de/pipermail/u-boot/2012-September/135156.html
checkgcc4:
	@if test "$(call cc-name)" = "gcc" -a \
			$(call cc-version) -lt 0400; then \
		echo -n '*** Your GCC is too old, please upgrade to GCC 4.x or newer'; \
		false; \
	fi
endif
