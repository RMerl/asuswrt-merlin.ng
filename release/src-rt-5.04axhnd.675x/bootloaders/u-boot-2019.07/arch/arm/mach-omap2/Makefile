# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(if $(filter am33xx,$(SOC)),y) += am33xx/
obj-$(CONFIG_OMAP34XX) += omap3/
obj-$(CONFIG_OMAP44XX) += omap4/
obj-$(CONFIG_OMAP54XX) += omap5/

obj-y	+= reset.o
ifeq ($(CONFIG_TIMER),)
obj-y	+= timer.o
else
ifdef CONFIG_SPL_BUILD
obj-y	+= timer.o
endif
endif
obj-y	+= utils.o

obj-y	+= sysinfo-common.o
ifneq ($(CONFIG_OMAP44XX)$(CONFIG_OMAP54XX),)
obj-y	+= hwinit-common.o
obj-y	+= clocks-common.o
obj-y	+= emif-common.o
obj-y	+= vc.o
obj-y	+= abb.o
endif

ifneq ($(CONFIG_OMAP54XX),)
ifeq ($(CONFIG_DM_SCSI),)
obj-y	+= pipe3-phy.o
obj-$(CONFIG_SCSI_AHCI_PLAT) += sata.o
endif
endif

ifeq ($(CONFIG_$(SPL_TPL_)SYS_DCACHE_OFF),)
obj-y	+= omap-cache.o
endif

obj-y	+= boot-common.o
obj-y	+= lowlevel_init.o

obj-y	+= mem-common.o

obj-y	+= fdt-common.o

obj-$(CONFIG_TI_SECURE_DEVICE) += sec-common.o
