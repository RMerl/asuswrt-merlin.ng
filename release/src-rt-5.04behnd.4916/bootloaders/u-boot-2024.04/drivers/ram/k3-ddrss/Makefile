# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2019-2022 Texas Instruments Incorporated - https://www.ti.com/
#

obj-$(CONFIG_K3_DDRSS) += k3-ddrss.o
obj-$(CONFIG_K3_DDRSS) += lpddr4_obj_if.o
obj-$(CONFIG_K3_DDRSS) += lpddr4.o
ccflags-$(CONFIG_K3_DDRSS) += -Idrivers/ram/k3-ddrss/

obj-$(CONFIG_K3_AM62A_DDRSS) += lpddr4_am6x.o
obj-$(CONFIG_K3_AM62A_DDRSS) += lpddr4_am62a_ctl_regs_rw_masks.o
ccflags-$(CONFIG_K3_AM62A_DDRSS) += -Idrivers/ram/k3-ddrss/am62a/

obj-$(CONFIG_K3_AM64_DDRSS) += lpddr4_am6x.o
obj-$(CONFIG_K3_AM64_DDRSS) += lpddr4_am64_ctl_regs_rw_masks.o
ccflags-$(CONFIG_K3_AM64_DDRSS) += -Idrivers/ram/k3-ddrss/am64/

obj-$(CONFIG_K3_J721E_DDRSS) += lpddr4_j721e.o
obj-$(CONFIG_K3_J721E_DDRSS) += lpddr4_j721e_ctl_regs_rw_masks.o
ccflags-$(CONFIG_K3_J721E_DDRSS) += -Idrivers/ram/k3-ddrss/j721e/
