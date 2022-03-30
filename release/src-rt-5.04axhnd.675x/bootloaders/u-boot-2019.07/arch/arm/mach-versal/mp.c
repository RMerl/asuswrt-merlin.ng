// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019 Xilinx, Inc.
 * Siva Durga Prasad <siva.durga.paladugu@xilinx.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#define HALT		0
#define RELEASE		1

#define VERSAL_RPU_CFG_CPU_HALT_MASK		0x01
#define VERSAL_RPU_GLBL_CTRL_SPLIT_LOCK_MASK	0x08
#define VERSAL_RPU_GLBL_CTRL_TCM_COMB_MASK	0x40
#define VERSAL_RPU_GLBL_CTRL_SLCLAMP_MASK	0x10

#define VERSAL_CRLAPB_RST_LPD_AMBA_RST_MASK	0x04
#define VERSAL_CRLAPB_RST_LPD_R50_RST_MASK	0x01
#define VERSAL_CRLAPB_RST_LPD_R51_RST_MASK	0x02
#define VERSAL_CRL_RST_CPU_R5_RESET_PGE_MASK	0x10
#define VERSAL_CRLAPB_CPU_R5_CTRL_CLKACT_MASK	0x1000000

void set_r5_halt_mode(u8 halt, u8 mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu0_cfg);
	if (halt == HALT)
		tmp &= ~VERSAL_RPU_CFG_CPU_HALT_MASK;
	else
		tmp |= VERSAL_RPU_CFG_CPU_HALT_MASK;
	writel(tmp, &rpu_base->rpu0_cfg);

	if (mode == TCM_LOCK) {
		tmp = readl(&rpu_base->rpu1_cfg);
		if (halt == HALT)
			tmp &= ~VERSAL_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= VERSAL_RPU_CFG_CPU_HALT_MASK;
		writel(tmp, &rpu_base->rpu1_cfg);
	}
}

void set_r5_tcm_mode(u8 mode)
{
	u32 tmp;

	tmp = readl(&rpu_base->rpu_glbl_ctrl);
	if (mode == TCM_LOCK) {
		tmp &= ~VERSAL_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
		tmp |= VERSAL_RPU_GLBL_CTRL_TCM_COMB_MASK |
		       VERSAL_RPU_GLBL_CTRL_SLCLAMP_MASK;
	} else {
		tmp |= VERSAL_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
		tmp &= ~(VERSAL_RPU_GLBL_CTRL_TCM_COMB_MASK |
		       VERSAL_RPU_GLBL_CTRL_SLCLAMP_MASK);
	}

	writel(tmp, &rpu_base->rpu_glbl_ctrl);
}

void release_r5_reset(u8 mode)
{
	u32 tmp;

	tmp = readl(&crlapb_base->rst_cpu_r5);
	tmp &= ~(VERSAL_CRLAPB_RST_LPD_AMBA_RST_MASK |
	       VERSAL_CRLAPB_RST_LPD_R50_RST_MASK |
	       VERSAL_CRL_RST_CPU_R5_RESET_PGE_MASK);

	if (mode == TCM_LOCK)
		tmp &= ~VERSAL_CRLAPB_RST_LPD_R51_RST_MASK;

	writel(tmp, &crlapb_base->rst_cpu_r5);
}

void enable_clock_r5(void)
{
	u32 tmp;

	tmp = readl(&crlapb_base->cpu_r5_ctrl);
	tmp |= VERSAL_CRLAPB_CPU_R5_CTRL_CLKACT_MASK;
	writel(tmp, &crlapb_base->cpu_r5_ctrl);
}

void initialize_tcm(bool mode)
{
	if (!mode) {
		set_r5_tcm_mode(TCM_LOCK);
		set_r5_halt_mode(HALT, TCM_LOCK);
		enable_clock_r5();
		release_r5_reset(TCM_LOCK);
	} else {
		set_r5_tcm_mode(TCM_SPLIT);
		set_r5_halt_mode(HALT, TCM_SPLIT);
		enable_clock_r5();
		release_r5_reset(TCM_SPLIT);
	}
}

void tcm_init(u8 mode)
{
	puts("WARNING: Initializing TCM overwrites TCM content\n");
	initialize_tcm(mode);
	memset((void *)VERSAL_TCM_BASE_ADDR, 0, VERSAL_TCM_SIZE);
}
