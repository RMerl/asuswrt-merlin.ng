# SPDX-License-Identifier: GPL-2.0
#
# Copyright 2008-2014 Freescale Semiconductor, Inc.

obj-$(CONFIG_SYS_FSL_DDR1) += main.o util.o ctrl_regs.o options.o \
				lc_common_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR2) += main.o util.o ctrl_regs.o options.o \
				lc_common_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR3) += main.o util.o ctrl_regs.o options.o \
				lc_common_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR4) += main.o util.o ctrl_regs.o options.o \
				lc_common_dimm_params.o

ifdef CONFIG_DDR_SPD
SPD := y
endif
ifdef CONFIG_SPD_EEPROM
SPD := y
endif
ifdef SPD
obj-$(CONFIG_SYS_FSL_DDR1)	+= ddr1_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR2)	+= ddr2_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR3)	+= ddr3_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR4)	+= ddr4_dimm_params.o
endif

obj-$(CONFIG_FSL_DDR_INTERACTIVE)	+= interactive.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN1)	+= mpc85xx_ddr_gen1.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN2)	+= mpc85xx_ddr_gen2.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN3)	+= mpc85xx_ddr_gen3.o
obj-$(CONFIG_SYS_FSL_DDRC_ARM_GEN3)	+= arm_ddr_gen3.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN4) += fsl_ddr_gen4.o
obj-$(CONFIG_SYS_FSL_MMDC) += fsl_mmdc.o
