# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the bus drivers.
#

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)
obj-$(CONFIG_TI_PWMSS)	+= ti-pwmss.o
obj-$(CONFIG_UNIPHIER_SYSTEM_BUS) += uniphier-system-bus.o
endif

obj-$(CONFIG_$(SPL_)TI_SYSC)	+= ti-sysc.o
