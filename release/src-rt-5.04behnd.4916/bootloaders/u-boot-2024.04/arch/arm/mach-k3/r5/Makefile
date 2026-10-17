# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
#	Andrew Davis <afd@ti.com>

obj-$(CONFIG_SOC_K3_J721E) += j721e/
obj-$(CONFIG_SOC_K3_J721E) += j7200/
obj-$(CONFIG_SOC_K3_J721S2) += j721s2/
obj-$(CONFIG_SOC_K3_AM625) += am62x/
obj-$(CONFIG_SOC_K3_AM62A7) += am62ax/

obj-y += lowlevel_init.o
obj-y += r5_mpu.o

ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_K3_LOAD_SYSFW) += sysfw-loader.o
endif
