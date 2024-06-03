// SPDX-License-Identifier: GPL-2.0+
/*
 * ti816x_emif4.c
 *
 * TI816x emif4 configuration file
 *
 * Copyright (C) 2017, Konsulko Group
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>

/*********************************************************************
 * Init DDR3 on TI816X EVM
 *********************************************************************/
static void ddr_init_settings(const struct cmd_control *ctrl, int emif)
{
	/*
	 * setup use_rank_delays to 1.  This is only necessary when
	 * multiple ranks are in use.  Though the EVM does not have
	 * multiple ranks, this is a good value to set.
	 */
	writel(1, DDRPHY_CONFIG_BASE + 0x134); // DATA0_REG_PHY_USE_RANK0_DELAYS
	writel(1, DDRPHY_CONFIG_BASE + 0x1d8); // DATA1_REG_PHY_USE_RANK0_DELAYS
	writel(1, DDRPHY_CONFIG_BASE + 0x27c); // DATA2_REG_PHY_USE_RANK0_DELAYS
	writel(1, DDRPHY_CONFIG_BASE + 0x320); // DATA3_REG_PHY_USE_RANK0_DELAYS

	config_cmd_ctrl(ctrl, emif);

	/* for ddr3 this needs to be set to 1 */
	writel(0x1, DDRPHY_CONFIG_BASE + 0x0F8); /* init mode */
	writel(0x1, DDRPHY_CONFIG_BASE + 0x104);
	writel(0x1, DDRPHY_CONFIG_BASE + 0x19C);
	writel(0x1, DDRPHY_CONFIG_BASE + 0x1A8);
	writel(0x1, DDRPHY_CONFIG_BASE + 0x240);
	writel(0x1, DDRPHY_CONFIG_BASE + 0x24C);
	writel(0x1, DDRPHY_CONFIG_BASE + 0x2E4);
	writel(0x1, DDRPHY_CONFIG_BASE + 0x2F0);

	/*
	 * This represents the initial value for the leveling process.  The
	 * value is a ratio - so 0x100 represents one cycle.  The real delay
	 * is determined through the leveling process.
	 *
	 * During the leveling process, 0x20 is subtracted from the value, so
	 * we have added that to the value we want to set.  We also set the
	 * values such that byte3 completes leveling after byte2 and byte1
	 * after byte0.
	 */
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x0F0); /*  data0 writelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x0F4);   /*   */
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x194); /*  data1 writelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x198);   /*   */
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x238); /*  data2 writelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x23c);   /*   */
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x2dc); /*  data3 writelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x2e0);   /*   */


	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x0FC); /*  data0 gatelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x100);
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x1A0); /*  data1 gatelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x1A4);
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x244); /*  data2 gatelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x248);
	writel((0x20 << 10) | 0x20, DDRPHY_CONFIG_BASE + 0x2E8); /*  data3 gatelvl init ratio */
	writel(0x0, DDRPHY_CONFIG_BASE + 0x2EC);

	writel(0x5, DDRPHY_CONFIG_BASE + 0x00C);     /* cmd0 io config - output impedance of pad */
	writel(0x5, DDRPHY_CONFIG_BASE + 0x010);     /* cmd0 io clk config - output impedance of pad */
	writel(0x5, DDRPHY_CONFIG_BASE + 0x040);     /* cmd1 io config - output impedance of pad */
	writel(0x5, DDRPHY_CONFIG_BASE + 0x044);     /* cmd1 io clk config - output impedance of pad */
	writel(0x5, DDRPHY_CONFIG_BASE + 0x074);     /* cmd2 io config - output impedance of pad */
	writel(0x5, DDRPHY_CONFIG_BASE + 0x078);     /* cmd2 io clk config - output impedance of pad */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x0A8);     /* data0 io config - output impedance of pad */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x0AC);     /* data0 io clk config - output impedance of pad */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x14C);     /* data1 io config - output impedance of pa     */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x150);     /* data1 io clk config - output impedance of pad */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x1F0);     /* data2 io config - output impedance of pa */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x1F4);     /* data2 io clk config - output impedance of pad */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x294);     /* data3 io config - output impedance of pa */
	writel(0x4, DDRPHY_CONFIG_BASE + 0x298);     /* data3 io clk config - output impedance of pad */
}

static void ddr3_sw_levelling(const struct ddr_data *data, int emif)
{
	/* Set the correct value to DDR_VTP_CTRL_0 */
	writel(0x6, (DDRPHY_CONFIG_BASE + 0x358));

	writel(data->datafwsratio0, (DDRPHY_CONFIG_BASE + 0x108));
	writel(data->datafwsratio0, (DDRPHY_CONFIG_BASE + 0x1AC));
	writel(data->datafwsratio0, (DDRPHY_CONFIG_BASE + 0x250));
	writel(data->datafwsratio0, (DDRPHY_CONFIG_BASE + 0x2F4));

	writel(data->datawdsratio0, (DDRPHY_CONFIG_BASE + 0x0DC));
	writel(data->datawdsratio0, (DDRPHY_CONFIG_BASE + 0x180));
	writel(data->datawdsratio0, (DDRPHY_CONFIG_BASE + 0x224));
	writel(data->datawdsratio0, (DDRPHY_CONFIG_BASE + 0x2C8));

	writel(data->datawrsratio0, (DDRPHY_CONFIG_BASE + 0x120));
	writel(data->datawrsratio0, (DDRPHY_CONFIG_BASE + 0x1C4));
	writel(data->datawrsratio0, (DDRPHY_CONFIG_BASE + 0x268));
	writel(data->datawrsratio0, (DDRPHY_CONFIG_BASE + 0x30C));

	writel(data->datardsratio0, (DDRPHY_CONFIG_BASE + 0x0C8));
	writel(data->datardsratio0, (DDRPHY_CONFIG_BASE + 0x16C));
	writel(data->datardsratio0, (DDRPHY_CONFIG_BASE + 0x210));
	writel(data->datardsratio0, (DDRPHY_CONFIG_BASE + 0x2B4));
}

static struct dmm_lisa_map_regs *hw_lisa_map_regs =
				(struct dmm_lisa_map_regs *)DMM_BASE;

#define DMM_PAT_BASE_ADDR		(DMM_BASE + 0x420)
void config_dmm(const struct dmm_lisa_map_regs *regs)
{
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_3);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_2);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_1);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_0);

	writel(regs->dmm_lisa_map_3, &hw_lisa_map_regs->dmm_lisa_map_3);
	writel(regs->dmm_lisa_map_2, &hw_lisa_map_regs->dmm_lisa_map_2);
	writel(regs->dmm_lisa_map_1, &hw_lisa_map_regs->dmm_lisa_map_1);
	writel(regs->dmm_lisa_map_0, &hw_lisa_map_regs->dmm_lisa_map_0);

	/* Enable Tiled Access */
	writel(0x80000000, DMM_PAT_BASE_ADDR);
}

void config_ddr(const struct ddr_data *data, const struct cmd_control *ctrl,
		const struct emif_regs *regs,
		const struct dmm_lisa_map_regs *lisa_regs, int nrs)
{
	int i;

	enable_emif_clocks();

	for (i = 0; i < nrs; i++)
		ddr_init_settings(ctrl, i);

	enable_dmm_clocks();

	/* Program the DMM to for non-interleaved configuration */
	config_dmm(lisa_regs);

	/* Program EMIF CFG Registers */
	for (i = 0; i < nrs; i++) {
		set_sdram_timings(regs, i);
		config_sdram(regs, i);
	}

	udelay(1000);
	for (i = 0; i < nrs; i++)
		ddr3_sw_levelling(data, i);

	udelay(50000);	/* Some delay needed */
}
