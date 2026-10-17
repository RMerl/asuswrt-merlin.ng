# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao, X.Xiao@motorola.com

PLATFORM_CPPFLAGS += -msoft-float -mno-string
PLATFORM_RELFLAGS += -msingle-pic-base -fno-jump-tables

# No SPE instruction when building u-boot
# (We use all available options to help semi-broken compilers)
# see "[PATCH,rs6000] make -mno-spe work as expected" on
# http://gcc.gnu.org/ml/gcc-patches/2008-04/msg00311.html
PLATFORM_CPPFLAGS += $(call cc-option,-mno-spe) \
		     $(call cc-option,-mspe=no)

# No AltiVec or VSX instructions when building u-boot
PLATFORM_CPPFLAGS += $(call cc-option,-mno-altivec)
PLATFORM_CPPFLAGS += $(call cc-option,-mno-vsx)

ifdef CONFIG_E6500
PLATFORM_CPPFLAGS += -mcpu=e6500
else ifdef CONFIG_E5500
PLATFORM_CPPFLAGS += -mcpu=e5500
else ifdef CONFIG_E500MC
PLATFORM_CPPFLAGS += -mcpu=e500mc
else
PLATFORM_CPPFLAGS += -mcpu=8540
endif
