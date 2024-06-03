# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

extra-y	= start.o
obj-y	= cpu.o cache.o

ifdef	CONFIG_SPL_BUILD
ifdef	CONFIG_SPL_NO_CPU_SUPPORT_CODE
extra-y	:=
endif
endif

obj-$(CONFIG_ARMADA100) += armada100/
obj-$(if $(filter lpc32xx,$(SOC)),y) += lpc32xx/
obj-$(CONFIG_MX25) += mx25/
obj-$(CONFIG_MX27) += mx27/
obj-$(if $(filter mxs,$(SOC)),y) += mxs/
obj-$(if $(filter spear,$(SOC)),y) += spear/

# some files can only build in ARM or THUMB2, not THUMB1

ifdef CONFIG_$(SPL_)SYS_THUMB_BUILD
ifndef CONFIG_HAS_THUMB2

CFLAGS_cpu.o := -marm
CFLAGS_cache.o := -marm

endif
endif
