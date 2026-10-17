# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

#ccflags-y += -DET_DEBUG -DDEBUG

obj-$(CONFIG_$(SPL_TPL_)PARTITIONS)  += part.o
ifdef CONFIG_$(SPL_TPL_)BLK
obj-$(CONFIG_$(SPL_TPL_)PARTITIONS)  += disk-uclass.o
endif

# Must have BLK or SPL_LEGACY_BLOCK to support partitions
ifneq ($(CONFIG_$(SPL_TPL_)BLK),$(CONFIG_SPL_LEGACY_BLOCK),)
obj-$(CONFIG_$(SPL_TPL_)MAC_PARTITION)   += part_mac.o
obj-$(CONFIG_$(SPL_TPL_)DOS_PARTITION)   += part_dos.o
obj-$(CONFIG_$(SPL_TPL_)ISO_PARTITION)   += part_iso.o
obj-$(CONFIG_$(SPL_TPL_)AMIGA_PARTITION) += part_amiga.o
obj-$(CONFIG_$(SPL_TPL_)EFI_PARTITION)   += part_efi.o
endif
