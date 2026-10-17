/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */
#ifndef _ASM_ARCH_GRF_RK3368_H
#define _ASM_ARCH_GRF_RK3368_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

struct rk3368_grf {
	u32 gpio1a_iomux;
	u32 gpio1b_iomux;
	u32 gpio1c_iomux;
	u32 gpio1d_iomux;
	u32 gpio2a_iomux;
	u32 gpio2b_iomux;
	u32 gpio2c_iomux;
	u32 gpio2d_iomux;
	u32 gpio3a_iomux;
	u32 gpio3b_iomux;
	u32 gpio3c_iomux;
	u32 gpio3d_iomux;
	u32 reserved[0x34];
	u32 gpio1a_pull;
	u32 gpio1b_pull;
	u32 gpio1c_pull;
	u32 gpio1d_pull;
	u32 gpio2a_pull;
	u32 gpio2b_pull;
	u32 gpio2c_pull;
	u32 gpio2d_pull;
	u32 gpio3a_pull;
	u32 gpio3b_pull;
	u32 gpio3c_pull;
	u32 gpio3d_pull;
	u32 reserved1[0x34];
	u32 gpio1a_drv;
	u32 gpio1b_drv;
	u32 gpio1c_drv;
	u32 gpio1d_drv;
	u32 gpio2a_drv;
	u32 gpio2b_drv;
	u32 gpio2c_drv;
	u32 gpio2d_drv;
	u32 gpio3a_drv;
	u32 gpio3b_drv;
	u32 gpio3c_drv;
	u32 gpio3d_drv;
	u32 reserved2[0x34];
	u32 gpio1l_sr;
	u32 gpio1h_sr;
	u32 gpio2l_sr;
	u32 gpio2h_sr;
	u32 gpio3l_sr;
	u32 gpio3h_sr;
	u32 reserved3[0x1a];
	u32 gpio_smt;
	u32 reserved4[0x1f];
	u32 soc_con0;
	u32 soc_con1;
	u32 soc_con2;
	u32 soc_con3;
	u32 soc_con4;
	u32 soc_con5;
	u32 soc_con6;
	u32 soc_con7;
	u32 soc_con8;
	u32 soc_con9;
	u32 soc_con10;
	u32 soc_con11;
	u32 soc_con12;
	u32 soc_con13;
	u32 soc_con14;
	u32 soc_con15;
	u32 soc_con16;
	u32 soc_con17;
	u32 reserved5[0x6e];
	u32 ddrc0_con0;
};
check_member(rk3368_grf, soc_con17, 0x444);
check_member(rk3368_grf, ddrc0_con0, 0x600);

struct rk3368_pmu_grf {
	u32 gpio0a_iomux;
	u32 gpio0b_iomux;
	u32 gpio0c_iomux;
	u32 gpio0d_iomux;
	u32 gpio0a_pull;
	u32 gpio0b_pull;
	u32 gpio0c_pull;
	u32 gpio0d_pull;
	u32 gpio0a_drv;
	u32 gpio0b_drv;
	u32 gpio0c_drv;
	u32 gpio0d_drv;
	u32 gpio0l_sr;
	u32 gpio0h_sr;
	u32 reserved[0x72];
	u32 os_reg[4];
};
check_member(rk3368_pmu_grf, gpio0h_sr, 0x34);
check_member(rk3368_pmu_grf, os_reg[0], 0x200);

/*GRF_SOC_CON11/12/13*/
enum {
	MCU_SRAM_BASE_BIT27_BIT12_SHIFT	= 0,
	MCU_SRAM_BASE_BIT27_BIT12_MASK	= GENMASK(15, 0),
};

/*GRF_SOC_CON12*/
enum {
	MCU_EXSRAM_BASE_BIT27_BIT12_SHIFT  = 0,
	MCU_EXSRAM_BASE_BIT27_BIT12_MASK   = GENMASK(15, 0),
};

/*GRF_SOC_CON13*/
enum {
	MCU_EXPERI_BASE_BIT27_BIT12_SHIFT  = 0,
	MCU_EXPERI_BASE_BIT27_BIT12_MASK   = GENMASK(15, 0),
};

/*GRF_SOC_CON14*/
enum {
	MCU_EXPERI_BASE_BIT31_BIT28_SHIFT	= 12,
	MCU_EXPERI_BASE_BIT31_BIT28_MASK	= GENMASK(15, 12),
	MCU_EXSRAM_BASE_BIT31_BIT28_SHIFT	= 8,
	MCU_EXSRAM_BASE_BIT31_BIT28_MASK	= GENMASK(11, 8),
	MCU_SRAM_BASE_BIT31_BIT28_SHIFT		= 4,
	MCU_SRAM_BASE_BIT31_BIT28_MASK		= GENMASK(7, 4),
	MCU_CODE_BASE_BIT31_BIT28_SHIFT		= 0,
	MCU_CODE_BASE_BIT31_BIT28_MASK		= GENMASK(3, 0),
};

#endif
