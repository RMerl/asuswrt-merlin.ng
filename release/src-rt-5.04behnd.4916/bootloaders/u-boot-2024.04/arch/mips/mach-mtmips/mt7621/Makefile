# SPDX-License-Identifier: GPL-2.0

obj-y += init.o
obj-y += serial.o

ifeq ($(CONFIG_SPL_BUILD),y)
ifeq ($(CONFIG_TPL_BUILD),y)
obj-y += tpl/
else
obj-y += spl/
endif

obj-y += sram_init.o
endif
