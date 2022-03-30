# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_DM_SCSI) += scsi-uclass.o
obj-$(CONFIG_SCSI) += scsi.o
endif

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_SATA_SUPPORT
obj-$(CONFIG_DM_SCSI) += scsi-uclass.o
obj-$(CONFIG_SCSI) += scsi.o
endif
endif

obj-$(CONFIG_SANDBOX) += sandbox_scsi.o
