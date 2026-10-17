# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
#	Lokesh Vutla <lokeshvutla@ti.com>

obj-$(CONFIG_CPU_V7R) += r5/
obj-$(CONFIG_ARM64) += arm64-mmu.o
obj-$(CONFIG_ARM64) += cache.o
obj-$(CONFIG_OF_LIBFDT) += common_fdt.o
ifeq ($(CONFIG_OF_LIBFDT)$(CONFIG_OF_SYSTEM_SETUP),yy)
obj-$(CONFIG_SOC_K3_AM654) += am654_fdt.o
obj-$(CONFIG_SOC_K3_J721E) += j721e_fdt.o
obj-$(CONFIG_SOC_K3_J721S2) += j721s2_fdt.o
obj-$(CONFIG_SOC_K3_AM625) += am625_fdt.o
endif
ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_SOC_K3_AM654) += am654_init.o
obj-$(CONFIG_SOC_K3_J721E) += j721e_init.o
obj-$(CONFIG_SOC_K3_J721S2) += j721s2_init.o
obj-$(CONFIG_SOC_K3_AM642) += am642_init.o
obj-$(CONFIG_SOC_K3_AM625) += am625_init.o
obj-$(CONFIG_SOC_K3_AM62A7) += am62a7_init.o
endif
obj-y += common.o security.o
