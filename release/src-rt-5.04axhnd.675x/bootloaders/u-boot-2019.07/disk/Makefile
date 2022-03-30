# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

#ccflags-y += -DET_DEBUG -DDEBUG

obj-$(CONFIG_PARTITIONS) 	+= part.o
obj-$(CONFIG_$(SPL_TPL_)MAC_PARTITION)   += part_mac.o
obj-$(CONFIG_$(SPL_TPL_)DOS_PARTITION)   += part_dos.o
obj-$(CONFIG_$(SPL_TPL_)ISO_PARTITION)   += part_iso.o
obj-$(CONFIG_$(SPL_TPL_)AMIGA_PARTITION) += part_amiga.o
obj-$(CONFIG_$(SPL_TPL_)EFI_PARTITION)   += part_efi.o
