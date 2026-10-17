# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

extra-y	:= start.o

obj-y	+= cpu.o
ifndef CONFIG_$(SPL_TPL_)TIMER
obj-$(CONFIG_SYS_ARCH_TIMER) += generic_timer.o
endif
ifndef CONFIG_$(SPL_)SYS_DCACHE_OFF
obj-y	+= cache_v8.o
obj-y	+= cache.o
endif
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMV8_SPL_EXCEPTION_VECTORS) += exceptions.o
else
obj-y	+= exceptions.o
obj-y	+= exception_level.o
endif
obj-y	+= tlb.o
obj-y	+= transition.o
ifndef CONFIG_ARMV8_PSCI
obj-y	+= fwcall.o
endif
obj-y	+= cpu-dt.o
obj-$(CONFIG_ARM_SMCCC)		+= smccc-call.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMV8_SPIN_TABLE) += spin_table.o spin_table_v8.o
else
obj-$(CONFIG_ARCH_SUNXI) += fel_utils.o
endif
obj-$(CONFIG_$(SPL_)ARMV8_SEC_FIRMWARE_SUPPORT) += sec_firmware.o sec_firmware_asm.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_RECOVER_DATA_SECTION) += spl_data.o
endif

obj-$(CONFIG_FSL_LAYERSCAPE) += fsl-layerscape/
obj-$(CONFIG_TARGET_HIKEY) += hisilicon/
obj-$(CONFIG_ARMV8_PSCI) += psci.o
obj-$(CONFIG_TARGET_BCMNS3) += bcmns3/
obj-$(CONFIG_XEN) += xen/
obj-$(CONFIG_ARMV8_CE_SHA1) += sha1_ce_glue.o sha1_ce_core.o
obj-$(CONFIG_ARMV8_CE_SHA256) += sha256_ce_glue.o sha256_ce_core.o
