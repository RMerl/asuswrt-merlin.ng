# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y += \
	fdt.o \
	fdt_ro.o \
	fdt_wip.o \
	fdt_strerror.o \
	fdt_sw.o \
	fdt_rw.o \
	fdt_empty_tree.o \
	fdt_addresses.o

obj-$(CONFIG_OF_LIBFDT_OVERLAY) += fdt_overlay.o

ccflags-y := -I$(srctree)/scripts/dtc/libfdt \
	-DFDT_ASSUME_MASK=$(CONFIG_$(SPL_TPL_)OF_LIBFDT_ASSUME_MASK)
