#
# (C) Copyright 2013-2014
# NVIDIA Corporation <www.nvidia.com>
#
# SPDX-License-Identifier:     GPL-2.0+
#

obj-$(CONFIG_SPL_BUILD) += cpu.o

obj-y	+= clock.o
obj-y	+= funcmux.o
obj-y	+= pinmux.o
obj-y	+= pmc.o
obj-y	+= xusb-padctl.o
obj-y	+= ../xusb-padctl-common.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMV7_NONSEC) += psci.o
endif
