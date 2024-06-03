# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

extra-y	:= start.o

obj-y	+= cpu.o
ifndef CONFIG_$(SPL_TPL_)TIMER
obj-$(CONFIG_SYS_ARCH_TIMER) += generic_timer.o
endif
obj-y	+= cache_v8.o
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMV8_SPL_EXCEPTION_VECTORS) += exceptions.o
else
obj-y	+= exceptions.o
obj-y	+= exception_level.o
endif
obj-y	+= cache.o
obj-y	+= tlb.o
obj-y	+= transition.o
ifndef CONFIG_ARMV8_PSCI
obj-y	+= fwcall.o
endif
obj-y	+= cpu-dt.o
obj-$(CONFIG_ARM_SMCCC)		+= smccc-call.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMV8_SPIN_TABLE) += spin_table.o spin_table_v8.o
endif
obj-$(CONFIG_$(SPL_)ARMV8_SEC_FIRMWARE_SUPPORT) += sec_firmware.o sec_firmware_asm.o

obj-$(CONFIG_FSL_LAYERSCAPE) += fsl-layerscape/
obj-$(CONFIG_S32V234) += s32v234/
obj-$(CONFIG_TARGET_HIKEY) += hisilicon/
obj-$(CONFIG_ARMV8_PSCI) += psci.o
obj-$(CONFIG_ARCH_SUNXI) += lowlevel_init.o
