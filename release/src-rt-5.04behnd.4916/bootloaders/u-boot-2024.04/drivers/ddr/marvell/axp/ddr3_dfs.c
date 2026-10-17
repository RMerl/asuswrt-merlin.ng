// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>

#include "ddr3_hw_training.h"

/*
 * Debug
 */
#define DEBUG_DFS_C(s, d, l) \
	DEBUG_DFS_S(s); DEBUG_DFS_D(d, l); DEBUG_DFS_S("\n")
#define DEBUG_DFS_FULL_C(s, d, l) \
	DEBUG_DFS_FULL_S(s); DEBUG_DFS_FULL_D(d, l); DEBUG_DFS_FULL_S("\n")

#ifdef MV_DEBUG_DFS
#define DEBUG_DFS_S(s)			puts(s)
#define DEBUG_DFS_D(d, l)		printf("%x", d)
#else
#define DEBUG_DFS_S(s)
#define DEBUG_DFS_D(d, l)
#endif

#ifdef MV_DEBUG_DFS_FULL
#define DEBUG_DFS_FULL_S(s)		puts(s)
#define DEBUG_DFS_FULL_D(d, l)		printf("%x", d)
#else
#define DEBUG_DFS_FULL_S(s)
#define DEBUG_DFS_FULL_D(d, l)
#endif

#if defined(MV88F672X)
extern u8 div_ratio[CLK_VCO][CLK_DDR];
extern void get_target_freq(u32 freq_mode, u32 *ddr_freq, u32 *hclk_ps);
#else
extern u16 odt_dynamic[ODT_OPT][MAX_CS];
extern u8 div_ratio1to1[CLK_VCO][CLK_DDR];
extern u8 div_ratio2to1[CLK_VCO][CLK_DDR];
#endif
extern u16 odt_static[ODT_OPT][MAX_CS];

extern u32 cpu_fab_clk_to_hclk[FAB_OPT][CLK_CPU];

extern u32 ddr3_get_vco_freq(void);

u32 ddr3_get_freq_parameter(u32 target_freq, int ratio_2to1);

#ifdef MV_DEBUG_DFS
static inline void dfs_reg_write(u32 addr, u32 val)
{
	printf("\n write reg 0x%08x = 0x%08x", addr, val);
	writel(val, INTER_REGS_BASE + addr);
}
#else
static inline void dfs_reg_write(u32 addr, u32 val)
{
	writel(val, INTER_REGS_BASE + addr);
}
#endif

static void wait_refresh_op_complete(void)
{
	u32 reg;

	/* Poll - Wait for Refresh operation completion */
	do {
		reg = reg_read(REG_SDRAM_OPERATION_ADDR) &
			REG_SDRAM_OPERATION_CMD_RFRS_DONE;
	} while (reg);		/* Wait for '0' */
}

/*
 * Name:     ddr3_get_freq_parameter
 * Desc:     Finds CPU/DDR frequency ratio according to Sample@reset and table.
 * Args:     target_freq - target frequency
 * Notes:
 * Returns:  freq_par - the ratio parameter
 */
u32 ddr3_get_freq_parameter(u32 target_freq, int ratio_2to1)
{
	u32 ui_vco_freq, freq_par;

	ui_vco_freq = ddr3_get_vco_freq();

#if defined(MV88F672X)
	freq_par = div_ratio[ui_vco_freq][target_freq];
#else
	/* Find the ratio between PLL frequency and ddr-clk */
	if (ratio_2to1)
		freq_par = div_ratio2to1[ui_vco_freq][target_freq];
	else
		freq_par = div_ratio1to1[ui_vco_freq][target_freq];
#endif

	return freq_par;
}

/*
 * Name:     ddr3_dfs_high_2_low
 * Desc:
 * Args:     freq - target frequency
 * Notes:
 * Returns:  MV_OK - success, MV_FAIL - fail
 */
int ddr3_dfs_high_2_low(u32 freq, MV_DRAM_INFO *dram_info)
{
#if defined(MV88F78X60) || defined(MV88F672X)
	/* This Flow is relevant for ArmadaXP A0 */
	u32 reg, freq_par, tmp;
	u32 cs = 0;

	DEBUG_DFS_C("DDR3 - DFS - High To Low - Starting DFS procedure to Frequency - ",
		    freq, 1);

	/* target frequency - 100MHz */
	freq_par = ddr3_get_freq_parameter(freq, 0);

#if defined(MV88F672X)
	u32 hclk;
	u32 cpu_freq = ddr3_get_cpu_freq();
	get_target_freq(cpu_freq, &tmp, &hclk);
#endif

	/* Configure - DRAM DLL final state after DFS is complete - Enable */
	reg = reg_read(REG_DFS_ADDR);
	/* [0] - DfsDllNextState - Disable */
	reg |= (1 << REG_DFS_DLLNEXTSTATE_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Configure - XBAR Retry response during Block to enable internal
	 * access - Disable
	 */
	reg = reg_read(REG_METAL_MASK_ADDR);
	/* [0] - RetryMask - Disable */
	reg &= ~(1 << REG_METAL_MASK_RETRY_OFFS);
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	/* Configure - Block new external transactions - Enable */
	reg = reg_read(REG_DFS_ADDR);
	reg |= (1 << REG_DFS_BLOCK_OFFS);	/* [1] - DfsBlock - Enable  */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Registered DIMM support */
	if (dram_info->reg_dimm) {
		/*
		 * Configure - Disable Register DIMM CKE Power
		 * Down mode - CWA_RC
		 */
		reg = (0x9 & REG_SDRAM_OPERATION_CWA_RC_MASK) <<
			REG_SDRAM_OPERATION_CWA_RC_OFFS;
		/*
		 * Configure - Disable Register DIMM CKE Power
		 * Down mode - CWA_DATA
		 */
		reg |= ((0 & REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
			REG_SDRAM_OPERATION_CWA_DATA_OFFS);

		/*
		 * Configure - Disable Register DIMM CKE Power
		 * Down mode - Set Delay - tMRD
		 */
		reg |= (0 << REG_SDRAM_OPERATION_CWA_DELAY_SEL_OFFS);

		/* Configure - Issue CWA command with the above parameters */
		reg |= (REG_SDRAM_OPERATION_CMD_CWA &
			~(0xF << REG_SDRAM_OPERATION_CS_OFFS));

		/* 0x1418 - SDRAM Operation Register */
		dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

		/* Poll - Wait for CWA operation completion */
		do {
			reg = reg_read(REG_SDRAM_OPERATION_ADDR) &
			       (REG_SDRAM_OPERATION_CMD_MASK);
		} while (reg);

		/* Configure - Disable outputs floating during Self Refresh */
		reg = reg_read(REG_REGISTERED_DRAM_CTRL_ADDR);
		/* [15] - SRFloatEn - Disable */
		reg &= ~(1 << REG_REGISTERED_DRAM_CTRL_SR_FLOAT_OFFS);
		/* 0x16D0 - DDR3 Registered DRAM Control */
		dfs_reg_write(REG_REGISTERED_DRAM_CTRL_ADDR, reg);
	}

	/* Optional - Configure - DDR3_Rtt_nom_CS# */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = reg_read(REG_DDR3_MR1_CS_ADDR +
				       (cs << MR_CS_ADDR_OFFS));
			reg &= REG_DDR3_MR1_RTT_MASK;
			dfs_reg_write(REG_DDR3_MR1_CS_ADDR +
				      (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	/* Configure - Move DRAM into Self Refresh */
	reg = reg_read(REG_DFS_ADDR);
	reg |= (1 << REG_DFS_SR_OFFS);	/* [2] - DfsSR - Enable */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Poll - Wait for Self Refresh indication */
	do {
		reg = ((reg_read(REG_DFS_ADDR)) & (1 << REG_DFS_ATSR_OFFS));
	} while (reg == 0x0);	/* 0x1528 [3] - DfsAtSR - Wait for '1' */

	/* Start of clock change procedure (PLL) */
#if defined(MV88F672X)
	/* avantaLP */
	/* Configure    cpupll_clkdiv_reset_mask */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL0);
	reg &= CPU_PLL_CLOCK_DIVIDER_CNTRL0_MASK;
	/* 0xE8264[7:0]   0xff CPU Clock Dividers Reset mask */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL0, (reg + 0xFF));

	/* Configure    cpu_clkdiv_reload_smooth    */
	reg = reg_read(CPU_PLL_CNTRL0);
	reg &= CPU_PLL_CNTRL0_RELOAD_SMOOTH_MASK;
	/* 0xE8260   [15:8]  0x2 CPU Clock Dividers Reload Smooth enable */
	dfs_reg_write(CPU_PLL_CNTRL0,
		      (reg + (2 << CPU_PLL_CNTRL0_RELOAD_SMOOTH_OFFS)));

	/* Configure    cpupll_clkdiv_relax_en */
	reg = reg_read(CPU_PLL_CNTRL0);
	reg &= CPU_PLL_CNTRL0_RELAX_EN_MASK;
	/* 0xE8260 [31:24] 0x2 Relax Enable */
	dfs_reg_write(CPU_PLL_CNTRL0,
		      (reg + (2 << CPU_PLL_CNTRL0_RELAX_EN_OFFS)));

	/* Configure    cpupll_clkdiv_ddr_clk_ratio */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL1);
	/*
	 * 0xE8268  [13:8]  N   Set Training clock:
	 * APLL Out Clock (VCO freq) / N = 100 MHz
	 */
	reg &= CPU_PLL_CLOCK_DIVIDER_CNTRL1_MASK;
	reg |= (freq_par << 8);	/* full Integer ratio from PLL-out to ddr-clk */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL1, reg);

	/* Configure    cpupll_clkdiv_reload_ratio  */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL0);
	reg &= CPU_PLL_CLOCK_RELOAD_RATIO_MASK;
	/* 0xE8264 [8]=0x1 CPU Clock Dividers Reload Ratio trigger set */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL0,
		      (reg + (1 << CPU_PLL_CLOCK_RELOAD_RATIO_OFFS)));

	udelay(1);

	/* Configure    cpupll_clkdiv_reload_ratio  */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL0);
	reg &= CPU_PLL_CLOCK_RELOAD_RATIO_MASK;
	/* 0xE8264 [8]=0x0 CPU Clock Dividers Reload Ratio trigger clear */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL0, reg);

	udelay(5);

#else
	/*
	 * Initial Setup - assure that the "load new ratio" is clear (bit 24)
	 * and in the same chance, block reassertions of reset [15:8] and
	 * force reserved bits[7:0].
	 */
	reg = 0x0000FDFF;
	/* 0x18700 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	/*
	 * RelaX whenever reset is asserted to that channel
	 * (good for any case)
	 */
	reg = 0x0000FF00;
	/* 0x18704 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_1_ADDR, reg);

	reg = reg_read(REG_CPU_DIV_CLK_CTRL_2_ADDR) &
		REG_CPU_DIV_CLK_CTRL_3_FREQ_MASK;

	/* full Integer ratio from PLL-out to ddr-clk */
	reg |= (freq_par << REG_CPU_DIV_CLK_CTRL_3_FREQ_OFFS);
	/* 0x1870C - CPU Div CLK control 3 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_2_ADDR, reg);

	/*
	 * Shut off clock enable to the DDRPHY clock channel (this is the "D").
	 * All the rest are kept as is (forced, but could be read-modify-write).
	 * This is done now by RMW above.
	 */

	/* Clock is not shut off gracefully - keep it running */
	reg = 0x000FFF02;
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_4_ADDR, reg);

	/* Wait before replacing the clock on the DDR Phy Channel. */
	udelay(1);

	/*
	 * This for triggering the frequency update. Bit[24] is the
	 * central control
	 * bits [23:16] == which channels to change ==2 ==>
	 *                 only DDR Phy (smooth transition)
	 * bits [15:8] == mask reset reassertion due to clock modification
	 *                to these channels.
	 * bits [7:0] == not in use
	 */
	reg = 0x0102FDFF;
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(1);		/* Wait 1usec */

	/*
	 * Poll Div CLK status 0 register - indication that the clocks
	 * are active - 0x18718 [8]
	 */
	do {
		reg = (reg_read(REG_CPU_DIV_CLK_STATUS_0_ADDR)) &
			(1 << REG_CPU_DIV_CLK_ALL_STABLE_OFFS);
	} while (reg == 0);

	/*
	 * Clean the CTRL0, to be ready for next resets and next requests
	 * of ratio modifications.
	 */
	reg = 0x000000FF;
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(5);
#endif
	/* End of clock change procedure (PLL) */

	/* Configure - Select normal clock for the DDR PHY - Enable */
	reg = reg_read(REG_DRAM_INIT_CTRL_STATUS_ADDR);
	/* [16] - ddr_phy_trn_clk_sel - Enable  */
	reg |= (1 << REG_DRAM_INIT_CTRL_TRN_CLK_OFFS);
	/* 0x18488 - DRAM Init control status register */
	dfs_reg_write(REG_DRAM_INIT_CTRL_STATUS_ADDR, reg);

	/* Configure - Set Correct Ratio - 1:1 */
	/* [15] - Phy2UnitClkRatio = 0 - Set 1:1 Ratio between Dunit and Phy */

	reg = reg_read(REG_DDR_IO_ADDR) & ~(1 << REG_DDR_IO_CLK_RATIO_OFFS);
	dfs_reg_write(REG_DDR_IO_ADDR, reg);	/* 0x1524 - DDR IO Register */

	/* Configure - 2T Mode - Restore original configuration */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	/* [3:4] 2T - 1T Mode - low freq */
	reg &= ~(REG_DUNIT_CTRL_LOW_2T_MASK << REG_DUNIT_CTRL_LOW_2T_OFFS);
	/* 0x1404 - DDR Controller Control Low Register */
	dfs_reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);

	/* Configure - Restore CL and CWL - MRS Commands */
	reg = reg_read(REG_DFS_ADDR);
	reg &= ~(REG_DFS_CL_NEXT_STATE_MASK << REG_DFS_CL_NEXT_STATE_OFFS);
	reg &= ~(REG_DFS_CWL_NEXT_STATE_MASK << REG_DFS_CWL_NEXT_STATE_OFFS);
	/* [8] - DfsCLNextState - MRS CL=6 after DFS (due to DLL-off mode) */
	reg |= (0x4 << REG_DFS_CL_NEXT_STATE_OFFS);
	/* [12] - DfsCWLNextState - MRS CWL=6 after DFS (due to DLL-off mode) */
	reg |= (0x1 << REG_DFS_CWL_NEXT_STATE_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Poll - Wait for APLL + ADLLs lock on new frequency */
	do {
		reg = (reg_read(REG_PHY_LOCK_STATUS_ADDR)) &
			REG_PHY_LOCK_APLL_ADLL_STATUS_MASK;
		/* 0x1674 [10:0] - Phy lock status Register */
	} while (reg != REG_PHY_LOCK_APLL_ADLL_STATUS_MASK);

	/* Configure - Reset the PHY Read FIFO and Write channels - Set Reset */
	reg = (reg_read(REG_SDRAM_CONFIG_ADDR) & REG_SDRAM_CONFIG_MASK);
	/* [30:29] = 0 - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/*
	 * Configure - DRAM Data PHY Read [30], Write [29] path
	 * reset - Release Reset
	 */
	reg = (reg_read(REG_SDRAM_CONFIG_ADDR) | ~REG_SDRAM_CONFIG_MASK);
	/* [30:29] = '11' - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/* Registered DIMM support */
	if (dram_info->reg_dimm) {
		/*
		 * Configure - Change register DRAM operating speed
		 * (below 400MHz) - CWA_RC
		 */
		reg = (0xA & REG_SDRAM_OPERATION_CWA_RC_MASK) <<
			REG_SDRAM_OPERATION_CWA_RC_OFFS;

		/*
		 * Configure - Change register DRAM operating speed
		 * (below 400MHz) - CWA_DATA
		 */
		reg |= ((0x0 & REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
			REG_SDRAM_OPERATION_CWA_DATA_OFFS);

		/* Configure - Set Delay - tSTAB */
		reg |= (0x1 << REG_SDRAM_OPERATION_CWA_DELAY_SEL_OFFS);

		/* Configure - Issue CWA command with the above parameters */
		reg |= (REG_SDRAM_OPERATION_CMD_CWA &
			~(0xF << REG_SDRAM_OPERATION_CS_OFFS));

		/* 0x1418 - SDRAM Operation Register */
		dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

		/* Poll - Wait for CWA operation completion */
		do {
			reg = reg_read(REG_SDRAM_OPERATION_ADDR) &
				(REG_SDRAM_OPERATION_CMD_MASK);
		} while (reg);
	}

	/* Configure - Exit Self Refresh */
	/* [2] - DfsSR  */
	reg = (reg_read(REG_DFS_ADDR) & ~(1 << REG_DFS_SR_OFFS));
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Poll - DFS Register - 0x1528 [3] - DfsAtSR - All DRAM devices
	 * on all ranks are NOT in self refresh mode
	 */
	do {
		reg = ((reg_read(REG_DFS_ADDR)) & (1 << REG_DFS_ATSR_OFFS));
	} while (reg);		/* Wait for '0' */

	/* Configure - Issue Refresh command */
	/* [3-0] = 0x2 - Refresh Command, [11-8] - enabled Cs */
	reg = REG_SDRAM_OPERATION_CMD_RFRS;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs))
			reg &= ~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
	}

	/* 0x1418 - SDRAM Operation Register */
	dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

	/* Poll - Wait for Refresh operation completion */
	wait_refresh_op_complete();

	/* Configure - Block new external transactions - Disable */
	reg = reg_read(REG_DFS_ADDR);
	reg &= ~(1 << REG_DFS_BLOCK_OFFS);	/* [1] - DfsBlock - Disable  */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Configure -  XBAR Retry response during Block to enable
	 * internal access - Disable
	 */
	reg = reg_read(REG_METAL_MASK_ADDR);
	/* [0] - RetryMask - Enable */
	reg |= (1 << REG_METAL_MASK_RETRY_OFFS);
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			/* Configure - Set CL */
			reg = reg_read(REG_DDR3_MR0_CS_ADDR +
				       (cs << MR_CS_ADDR_OFFS)) &
				~REG_DDR3_MR0_CL_MASK;
			tmp = 0x4;	/* CL=6 - 0x4 */
			reg |= ((tmp & 0x1) << REG_DDR3_MR0_CL_OFFS);
			reg |= ((tmp & 0xE) << REG_DDR3_MR0_CL_HIGH_OFFS);
			dfs_reg_write(REG_DDR3_MR0_CS_ADDR +
				      (cs << MR_CS_ADDR_OFFS), reg);

			/* Configure - Set CWL */
			reg = reg_read(REG_DDR3_MR2_CS_ADDR +
				       (cs << MR_CS_ADDR_OFFS))
				& ~(REG_DDR3_MR2_CWL_MASK << REG_DDR3_MR2_CWL_OFFS);
			/* CWL=6 - 0x1 */
			reg |= ((0x1) << REG_DDR3_MR2_CWL_OFFS);
			dfs_reg_write(REG_DDR3_MR2_CS_ADDR +
				      (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	DEBUG_DFS_C("DDR3 - DFS - High To Low - Ended successfuly - new Frequency - ",
		    freq, 1);

	return MV_OK;
#else
	/* This Flow is relevant for Armada370 A0 and ArmadaXP Z1 */

	u32 reg, freq_par;
	u32 cs = 0;

	DEBUG_DFS_C("DDR3 - DFS - High To Low - Starting DFS procedure to Frequency - ",
		    freq, 1);

	/* target frequency - 100MHz */
	freq_par = ddr3_get_freq_parameter(freq, 0);

	reg = 0x0000FF00;
	/* 0x18700 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_1_ADDR, reg);

	/* 0x1600 - ODPG_CNTRL_Control */
	reg = reg_read(REG_ODPG_CNTRL_ADDR);
	/* [21] = 1 - auto refresh disable */
	reg |= (1 << REG_ODPG_CNTRL_OFFS);
	dfs_reg_write(REG_ODPG_CNTRL_ADDR, reg);

	/* 0x1670 - PHY lock mask register */
	reg = reg_read(REG_PHY_LOCK_MASK_ADDR);
	reg &= REG_PHY_LOCK_MASK_MASK;	/* [11:0] = 0 */
	dfs_reg_write(REG_PHY_LOCK_MASK_ADDR, reg);

	reg = reg_read(REG_DFS_ADDR);	/* 0x1528 - DFS register */

	/* Disable reconfig */
	reg &= ~0x10;	/* [4] - Enable reconfig MR registers after DFS_ERG */
	reg |= 0x1;	/* [0] - DRAM DLL disabled after DFS */

	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	reg = reg_read(REG_METAL_MASK_ADDR) & ~(1 << 0); /* [0] - disable */
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	/* [1] - DFS Block enable  */
	reg = reg_read(REG_DFS_ADDR) | (1 << REG_DFS_BLOCK_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* [2] - DFS Self refresh enable  */
	reg = reg_read(REG_DFS_ADDR) | (1 << REG_DFS_SR_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Poll DFS Register - 0x1528 [3] - DfsAtSR -
	 * All DRAM devices on all ranks are in self refresh mode -
	 * DFS can be executed afterwards
	 */
	do {
		reg = reg_read(REG_DFS_ADDR) & (1 << REG_DFS_ATSR_OFFS);
	} while (reg == 0x0);	/* Wait for '1' */

	/* Disable ODT on DLL-off mode */
	dfs_reg_write(REG_SDRAM_ODT_CTRL_HIGH_ADDR,
		      REG_SDRAM_ODT_CTRL_HIGH_OVRD_MASK);

	/* [11:0] = 0 */
	reg = (reg_read(REG_PHY_LOCK_MASK_ADDR) & REG_PHY_LOCK_MASK_MASK);
	/* 0x1670 - PHY lock mask register */
	dfs_reg_write(REG_PHY_LOCK_MASK_ADDR, reg);

	/* Add delay between entering SR and start ratio modification */
	udelay(1);

	/*
	 * Initial Setup - assure that the "load new ratio" is clear (bit 24)
	 * and in the same chance, block reassertions of reset [15:8] and
	 * force reserved bits[7:0].
	 */
	reg = 0x0000FDFF;
	/* 0x18700 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	/*
	 * RelaX whenever reset is asserted to that channel (good for any case)
	 */
	reg = 0x0000FF00;
	/* 0x18700 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_1_ADDR, reg);

	reg = reg_read(REG_CPU_DIV_CLK_CTRL_3_ADDR) &
		REG_CPU_DIV_CLK_CTRL_3_FREQ_MASK;
	/* Full Integer ratio from PLL-out to ddr-clk */
	reg |= (freq_par << REG_CPU_DIV_CLK_CTRL_3_FREQ_OFFS);
	/* 0x1870C - CPU Div CLK control 3 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_3_ADDR, reg);

	/*
	 * Shut off clock enable to the DDRPHY clock channel (this is the "D").
	 * All the rest are kept as is (forced, but could be read-modify-write).
	 * This is done now by RMW above.
	 */

	/* Clock is not shut off gracefully - keep it running */
	reg = 0x000FFF02;
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_4_ADDR, reg);

	/* Wait before replacing the clock on the DDR Phy Channel. */
	udelay(1);

	/*
	 * This for triggering the frequency update. Bit[24] is the
	 * central control
	 * bits [23:16] == which channels to change ==2 ==> only DDR Phy
	 *                 (smooth transition)
	 * bits [15:8] == mask reset reassertion due to clock modification
	 *                to these channels.
	 * bits [7:0] == not in use
	 */
	reg = 0x0102FDFF;
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(1);		/* Wait 1usec */

	/*
	 * Poll Div CLK status 0 register - indication that the clocks
	 * are active - 0x18718 [8]
	 */
	do {
		reg = (reg_read(REG_CPU_DIV_CLK_STATUS_0_ADDR)) &
			(1 << REG_CPU_DIV_CLK_ALL_STABLE_OFFS);
	} while (reg == 0);

	/*
	 * Clean the CTRL0, to be ready for next resets and next requests of
	 * ratio modifications.
	 */
	reg = 0x000000FF;
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(5);

	/* Switch HCLK Mux to training clk (100Mhz), keep DFS request bit */
	reg = 0x20050000;
	/* 0x18488 - DRAM Init control status register */
	dfs_reg_write(REG_DRAM_INIT_CTRL_STATUS_ADDR, reg);

	reg = reg_read(REG_DDR_IO_ADDR) & ~(1 << REG_DDR_IO_CLK_RATIO_OFFS);
	/* [15] = 0 - Set 1:1 Ratio between Dunit and Phy */
	dfs_reg_write(REG_DDR_IO_ADDR, reg);	/* 0x1524 - DDR IO Regist */

	reg = reg_read(REG_DRAM_PHY_CONFIG_ADDR) & REG_DRAM_PHY_CONFIG_MASK;
	/* [31:30]] - reset pup data ctrl ADLL */
	/* 0x15EC - DRAM PHY Config register */
	dfs_reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

	reg = (reg_read(REG_DRAM_PHY_CONFIG_ADDR) | ~REG_DRAM_PHY_CONFIG_MASK);
	/* [31:30] - normal pup data ctrl ADLL */
	/* 0x15EC - DRAM PHY Config register */
	dfs_reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

	udelay(1);		/* Wait 1usec */

	/* 0x1404 */
	reg = (reg_read(REG_DUNIT_CTRL_LOW_ADDR) & 0xFFFFFFE7);
	dfs_reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);

	/* Poll Phy lock status register - APLL lock indication - 0x1674 */
	do {
		reg = (reg_read(REG_PHY_LOCK_STATUS_ADDR)) &
			REG_PHY_LOCK_STATUS_LOCK_MASK;
	} while (reg != REG_PHY_LOCK_STATUS_LOCK_MASK);	/* Wait for '0xFFF' */

	reg = (reg_read(REG_SDRAM_CONFIG_ADDR) & REG_SDRAM_CONFIG_MASK);
	/* [30:29] = 0 - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	reg = reg_read(REG_SDRAM_CONFIG_ADDR) | ~REG_SDRAM_CONFIG_MASK;
	/* [30:29] = '11' - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	udelay(1000);		/* Wait 1msec */

	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			/* Config CL and CWL with MR0 and MR2 registers */
			reg = reg_read(REG_DDR3_MR0_ADDR);
			reg &= ~0x74;	/* CL [3:0]; [6:4],[2] */
			reg |= (1 << 5);	/* CL = 4, CAS is 6 */
			dfs_reg_write(REG_DDR3_MR0_ADDR, reg);
			reg = REG_SDRAM_OPERATION_CMD_MR0 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/* 0x1418 - SDRAM Operation Register */
			dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			reg = reg_read(REG_DDR3_MR2_ADDR);
			reg &= ~0x38;	/* CWL [5:3] */
			reg |= (1 << 3);	/* CWL = 1, CWL is 6 */
			dfs_reg_write(REG_DDR3_MR2_ADDR, reg);

			reg = REG_SDRAM_OPERATION_CMD_MR2 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/* 0x1418 - SDRAM Operation Register */
			dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			/* Set current rd_sample_delay  */
			reg = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
			reg &= ~(REG_READ_DATA_SAMPLE_DELAYS_MASK <<
				 (REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			reg |= (5 << (REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			dfs_reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR, reg);

			/* Set current rd_ready_delay  */
			reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
			reg &= ~(REG_READ_DATA_READY_DELAYS_MASK <<
				 (REG_READ_DATA_READY_DELAYS_OFFS * cs));
			reg |= ((6) << (REG_READ_DATA_READY_DELAYS_OFFS * cs));
			dfs_reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
		}
	}

	/* [2] - DFS Self refresh disable  */
	reg = reg_read(REG_DFS_ADDR) & ~(1 << REG_DFS_SR_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* [1] - DFS Block enable  */
	reg = reg_read(REG_DFS_ADDR) & ~(1 << REG_DFS_BLOCK_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Poll DFS Register - 0x1528 [3] - DfsAtSR -
	 * All DRAM devices on all ranks are in self refresh mode - DFS can
	 * be executed afterwards
	 */
	do {
		reg = reg_read(REG_DFS_ADDR) & (1 << REG_DFS_ATSR_OFFS);
	} while (reg);		/* Wait for '1' */

	reg = (reg_read(REG_METAL_MASK_ADDR) | (1 << 0));
	/* [0] - Enable Dunit to crossbar retry */
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	/* 0x1600 - PHY lock mask register */
	reg = reg_read(REG_ODPG_CNTRL_ADDR);
	reg &= ~(1 << REG_ODPG_CNTRL_OFFS);	/* [21] = 0 */
	dfs_reg_write(REG_ODPG_CNTRL_ADDR, reg);

	/* 0x1670 - PHY lock mask register */
	reg = reg_read(REG_PHY_LOCK_MASK_ADDR);
	reg |= ~REG_PHY_LOCK_MASK_MASK;	/* [11:0] = FFF */
	dfs_reg_write(REG_PHY_LOCK_MASK_ADDR, reg);

	DEBUG_DFS_C("DDR3 - DFS - High To Low - Ended successfuly - new Frequency - ",
		    freq, 1);

	return MV_OK;
#endif
}

/*
 * Name:     ddr3_dfs_low_2_high
 * Desc:
 * Args:     freq - target frequency
 * Notes:
 * Returns:  MV_OK - success, MV_FAIL - fail
 */
int ddr3_dfs_low_2_high(u32 freq, int ratio_2to1, MV_DRAM_INFO *dram_info)
{
#if defined(MV88F78X60) || defined(MV88F672X)
	/* This Flow is relevant for ArmadaXP A0 */
	u32 reg, freq_par, tmp;
	u32 cs = 0;

	DEBUG_DFS_C("DDR3 - DFS - Low To High - Starting DFS procedure to Frequency - ",
		    freq, 1);

	/* target frequency - freq */
	freq_par = ddr3_get_freq_parameter(freq, ratio_2to1);

#if defined(MV88F672X)
	u32 hclk;
	u32 cpu_freq = ddr3_get_cpu_freq();
	get_target_freq(cpu_freq, &tmp, &hclk);
#endif

	/* Configure - DRAM DLL final state after DFS is complete - Enable */
	reg = reg_read(REG_DFS_ADDR);
	/* [0] - DfsDllNextState - Enable */
	reg &= ~(1 << REG_DFS_DLLNEXTSTATE_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Configure -  XBAR Retry response during Block to enable
	 * internal access - Disable
	 */
	reg = reg_read(REG_METAL_MASK_ADDR);
	/* [0] - RetryMask - Disable */
	reg &= ~(1 << REG_METAL_MASK_RETRY_OFFS);
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	/* Configure - Block new external transactions - Enable */
	reg = reg_read(REG_DFS_ADDR);
	reg |= (1 << REG_DFS_BLOCK_OFFS);	/* [1] - DfsBlock - Enable  */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Configure - Move DRAM into Self Refresh */
	reg = reg_read(REG_DFS_ADDR);
	reg |= (1 << REG_DFS_SR_OFFS);	/* [2] - DfsSR - Enable */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Poll - Wait for Self Refresh indication */
	do {
		reg = ((reg_read(REG_DFS_ADDR)) & (1 << REG_DFS_ATSR_OFFS));
	} while (reg == 0x0);	/* 0x1528 [3] - DfsAtSR - Wait for '1' */

	/* Start of clock change procedure (PLL) */
#if defined(MV88F672X)
	/* avantaLP */
	/* Configure    cpupll_clkdiv_reset_mask */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL0);
	reg &= CPU_PLL_CLOCK_DIVIDER_CNTRL0_MASK;
	/* 0xE8264[7:0]   0xff CPU Clock Dividers Reset mask */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL0, (reg + 0xFF));

	/* Configure    cpu_clkdiv_reload_smooth    */
	reg = reg_read(CPU_PLL_CNTRL0);
	reg &= CPU_PLL_CNTRL0_RELOAD_SMOOTH_MASK;
	/* 0xE8260   [15:8]  0x2 CPU Clock Dividers Reload Smooth enable */
	dfs_reg_write(CPU_PLL_CNTRL0,
		      reg + (2 << CPU_PLL_CNTRL0_RELOAD_SMOOTH_OFFS));

	/* Configure    cpupll_clkdiv_relax_en */
	reg = reg_read(CPU_PLL_CNTRL0);
	reg &= CPU_PLL_CNTRL0_RELAX_EN_MASK;
	/* 0xE8260 [31:24] 0x2 Relax Enable */
	dfs_reg_write(CPU_PLL_CNTRL0,
		      reg + (2 << CPU_PLL_CNTRL0_RELAX_EN_OFFS));

	/* Configure    cpupll_clkdiv_ddr_clk_ratio */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL1);
	/*
	 * 0xE8268  [13:8]  N   Set Training clock:
	 * APLL Out Clock (VCO freq) / N = 100 MHz
	 */
	reg &= CPU_PLL_CLOCK_DIVIDER_CNTRL1_MASK;
	reg |= (freq_par << 8);	/* full Integer ratio from PLL-out to ddr-clk */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL1, reg);
	/* Configure    cpupll_clkdiv_reload_ratio  */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL0);
	reg &= CPU_PLL_CLOCK_RELOAD_RATIO_MASK;
	/* 0xE8264 [8]=0x1 CPU Clock Dividers Reload Ratio trigger set */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL0,
		      reg + (1 << CPU_PLL_CLOCK_RELOAD_RATIO_OFFS));

	udelay(1);

	/* Configure    cpupll_clkdiv_reload_ratio  */
	reg = reg_read(CPU_PLL_CLOCK_DIVIDER_CNTRL0);
	reg &= CPU_PLL_CLOCK_RELOAD_RATIO_MASK;
	/* 0xE8264 [8]=0x0 CPU Clock Dividers Reload Ratio trigger clear */
	dfs_reg_write(CPU_PLL_CLOCK_DIVIDER_CNTRL0, reg);

	udelay(5);

#else
	/*
	 * Initial Setup - assure that the "load new ratio" is clear (bit 24)
	 * and in the same chance, block reassertions of reset [15:8]
	 * and force reserved bits[7:0].
	 */
	reg = 0x0000FFFF;

	/* 0x18700 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	/*
	 * RelaX whenever reset is asserted to that channel (good for any case)
	 */
	reg = 0x0000FF00;
	/* 0x18704 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_1_ADDR, reg);

	reg = reg_read(REG_CPU_DIV_CLK_CTRL_2_ADDR) &
		REG_CPU_DIV_CLK_CTRL_3_FREQ_MASK;
	reg |= (freq_par << REG_CPU_DIV_CLK_CTRL_3_FREQ_OFFS);
	/* full Integer ratio from PLL-out to ddr-clk */
	/* 0x1870C - CPU Div CLK control 3 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_2_ADDR, reg);

	/*
	 * Shut off clock enable to the DDRPHY clock channel (this is the "D").
	 * All the rest are kept as is (forced, but could be read-modify-write).
	 * This is done now by RMW above.
	 */
	reg = 0x000FFF02;
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_4_ADDR, reg);

	/* Wait before replacing the clock on the DDR Phy Channel. */
	udelay(1);

	reg = 0x0102FDFF;
	/*
	 * This for triggering the frequency update. Bit[24] is the
	 * central control
	 * bits [23:16] == which channels to change ==2 ==> only DDR Phy
	 *                 (smooth transition)
	 * bits [15:8] == mask reset reassertion due to clock modification
	 *                to these channels.
	 * bits [7:0] == not in use
	 */
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(1);

	/*
	 * Poll Div CLK status 0 register - indication that the clocks
	 * are active - 0x18718 [8]
	 */
	do {
		reg = reg_read(REG_CPU_DIV_CLK_STATUS_0_ADDR) &
			(1 << REG_CPU_DIV_CLK_ALL_STABLE_OFFS);
	} while (reg == 0);

	reg = 0x000000FF;
	/*
	 * Clean the CTRL0, to be ready for next resets and next requests
	 * of ratio modifications.
	 */
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);
#endif
	/* End of clock change procedure (PLL) */

	if (ratio_2to1) {
		/* Configure - Select normal clock for the DDR PHY - Disable */
		reg = reg_read(REG_DRAM_INIT_CTRL_STATUS_ADDR);
		/* [16] - ddr_phy_trn_clk_sel - Disable  */
		reg &= ~(1 << REG_DRAM_INIT_CTRL_TRN_CLK_OFFS);
		/* 0x18488 - DRAM Init control status register */
		dfs_reg_write(REG_DRAM_INIT_CTRL_STATUS_ADDR, reg);
	}

	/*
	 * Configure - Set Correct Ratio - according to target ratio
	 * parameter - 2:1/1:1
	 */
	if (ratio_2to1) {
		/*
		 * [15] - Phy2UnitClkRatio = 1 - Set 2:1 Ratio between
		 * Dunit and Phy
		 */
		reg = reg_read(REG_DDR_IO_ADDR) |
			(1 << REG_DDR_IO_CLK_RATIO_OFFS);
	} else {
		/*
		 * [15] - Phy2UnitClkRatio = 0 - Set 1:1 Ratio between
		 * Dunit and Phy
		 */
		reg = reg_read(REG_DDR_IO_ADDR) &
			~(1 << REG_DDR_IO_CLK_RATIO_OFFS);
	}
	dfs_reg_write(REG_DDR_IO_ADDR, reg);	/* 0x1524 - DDR IO Register */

	/* Configure - 2T Mode - Restore original configuration */
	reg = reg_read(REG_DUNIT_CTRL_LOW_ADDR);
	/* [3:4] 2T - Restore value */
	reg &= ~(REG_DUNIT_CTRL_LOW_2T_MASK << REG_DUNIT_CTRL_LOW_2T_OFFS);
	reg |= ((dram_info->mode_2t & REG_DUNIT_CTRL_LOW_2T_MASK) <<
		REG_DUNIT_CTRL_LOW_2T_OFFS);
	/* 0x1404 - DDR Controller Control Low Register */
	dfs_reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);

	/* Configure - Restore CL and CWL - MRS Commands */
	reg = reg_read(REG_DFS_ADDR);
	reg &= ~(REG_DFS_CL_NEXT_STATE_MASK << REG_DFS_CL_NEXT_STATE_OFFS);
	reg &= ~(REG_DFS_CWL_NEXT_STATE_MASK << REG_DFS_CWL_NEXT_STATE_OFFS);

	if (freq == DDR_400) {
		if (dram_info->target_frequency == 0x8)
			tmp = ddr3_cl_to_valid_cl(5);
		else
			tmp = ddr3_cl_to_valid_cl(6);
	} else {
		tmp = ddr3_cl_to_valid_cl(dram_info->cl);
	}

	/* [8] - DfsCLNextState */
	reg |= ((tmp & REG_DFS_CL_NEXT_STATE_MASK) << REG_DFS_CL_NEXT_STATE_OFFS);
	if (freq == DDR_400) {
		/* [12] - DfsCWLNextState */
		reg |= (((0) & REG_DFS_CWL_NEXT_STATE_MASK) <<
			REG_DFS_CWL_NEXT_STATE_OFFS);
	} else {
		/* [12] - DfsCWLNextState */
		reg |= (((dram_info->cwl) & REG_DFS_CWL_NEXT_STATE_MASK) <<
			REG_DFS_CWL_NEXT_STATE_OFFS);
	}
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Optional - Configure - DDR3_Rtt_nom_CS# */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = reg_read(REG_DDR3_MR1_CS_ADDR +
				       (cs << MR_CS_ADDR_OFFS));
			reg &= REG_DDR3_MR1_RTT_MASK;
			reg |= odt_static[dram_info->cs_ena][cs];
			dfs_reg_write(REG_DDR3_MR1_CS_ADDR +
				      (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	/* Configure - Reset ADLLs - Set Reset */
	reg = reg_read(REG_DRAM_PHY_CONFIG_ADDR) & REG_DRAM_PHY_CONFIG_MASK;
	/* [31:30]] - reset pup data ctrl ADLL */
	/* 0x15EC - DRAM PHY Config Register */
	dfs_reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

	/* Configure - Reset ADLLs - Release Reset */
	reg = reg_read(REG_DRAM_PHY_CONFIG_ADDR) | ~REG_DRAM_PHY_CONFIG_MASK;
	/* [31:30] - normal pup data ctrl ADLL */
	/* 0x15EC - DRAM PHY Config register */
	dfs_reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

	/* Poll - Wait for APLL + ADLLs lock on new frequency */
	do {
		reg = reg_read(REG_PHY_LOCK_STATUS_ADDR) &
			REG_PHY_LOCK_APLL_ADLL_STATUS_MASK;
		/* 0x1674 [10:0] - Phy lock status Register */
	} while (reg != REG_PHY_LOCK_APLL_ADLL_STATUS_MASK);

	/* Configure - Reset the PHY SDR clock divider */
	if (ratio_2to1) {
		/* Pup Reset Divider B - Set Reset */
		/* [28] - DataPupRdRST = 0 */
		reg = reg_read(REG_SDRAM_CONFIG_ADDR) &
			~(1 << REG_SDRAM_CONFIG_PUPRSTDIV_OFFS);
		/* [28] - DataPupRdRST = 1 */
		tmp = reg_read(REG_SDRAM_CONFIG_ADDR) |
			(1 << REG_SDRAM_CONFIG_PUPRSTDIV_OFFS);
		/* 0x1400 - SDRAM Configuration register */
		dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

		/* Pup Reset Divider B - Release Reset */
		/* 0x1400 - SDRAM Configuration register */
		dfs_reg_write(REG_SDRAM_CONFIG_ADDR, tmp);
	}

	/* Configure - Reset the PHY Read FIFO and Write channels - Set Reset */
	reg = reg_read(REG_SDRAM_CONFIG_ADDR) & REG_SDRAM_CONFIG_MASK;
	/* [30:29] = 0 - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/*
	 * Configure - DRAM Data PHY Read [30], Write [29] path reset -
	 * Release Reset
	 */
	reg = reg_read(REG_SDRAM_CONFIG_ADDR) | ~REG_SDRAM_CONFIG_MASK;
	/* [30:29] = '11' - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/* Registered DIMM support */
	if (dram_info->reg_dimm) {
		/*
		 * Configure - Change register DRAM operating speed
		 * (DDR3-1333 / DDR3-1600) - CWA_RC
		 */
		reg = (0xA & REG_SDRAM_OPERATION_CWA_RC_MASK) <<
			REG_SDRAM_OPERATION_CWA_RC_OFFS;
		if (freq <= DDR_400) {
			/*
			 * Configure - Change register DRAM operating speed
			 * (DDR3-800) - CWA_DATA
			 */
			reg |= ((0x0 & REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
				REG_SDRAM_OPERATION_CWA_DATA_OFFS);
		} else if ((freq > DDR_400) && (freq <= DDR_533)) {
			/*
			 * Configure - Change register DRAM operating speed
			 * (DDR3-1066) - CWA_DATA
			 */
			reg |= ((0x1 & REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
				REG_SDRAM_OPERATION_CWA_DATA_OFFS);
		} else if ((freq > DDR_533) && (freq <= DDR_666)) {
			/*
			 * Configure - Change register DRAM operating speed
			 * (DDR3-1333) - CWA_DATA
			 */
			reg |= ((0x2 & REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
				REG_SDRAM_OPERATION_CWA_DATA_OFFS);
		} else {
			/*
			 * Configure - Change register DRAM operating speed
			 * (DDR3-1600) - CWA_DATA
			 */
			reg |= ((0x3 & REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
				REG_SDRAM_OPERATION_CWA_DATA_OFFS);
		}

		/* Configure - Set Delay - tSTAB */
		reg |= (0x1 << REG_SDRAM_OPERATION_CWA_DELAY_SEL_OFFS);
		/* Configure - Issue CWA command with the above parameters */
		reg |= (REG_SDRAM_OPERATION_CMD_CWA &
			~(0xF << REG_SDRAM_OPERATION_CS_OFFS));

		/* 0x1418 - SDRAM Operation Register */
		dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

		/* Poll - Wait for CWA operation completion */
		do {
			reg = reg_read(REG_SDRAM_OPERATION_ADDR) &
				REG_SDRAM_OPERATION_CMD_MASK;
		} while (reg);
	}

	/* Configure - Exit Self Refresh */
	/* [2] - DfsSR  */
	reg = reg_read(REG_DFS_ADDR) & ~(1 << REG_DFS_SR_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Poll - DFS Register - 0x1528 [3] - DfsAtSR - All DRAM
	 * devices on all ranks are NOT in self refresh mode
	 */
	do {
		reg = reg_read(REG_DFS_ADDR) & (1 << REG_DFS_ATSR_OFFS);
	} while (reg);		/* Wait for '0' */

	/* Configure - Issue Refresh command */
	/* [3-0] = 0x2 - Refresh Command, [11-8] - enabled Cs */
	reg = REG_SDRAM_OPERATION_CMD_RFRS;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs))
			reg &= ~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
	}

	/* 0x1418 - SDRAM Operation Register */
	dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

	/* Poll - Wait for Refresh operation completion */
	wait_refresh_op_complete();

	/* Configure - Block new external transactions - Disable */
	reg = reg_read(REG_DFS_ADDR);
	reg &= ~(1 << REG_DFS_BLOCK_OFFS);	/* [1] - DfsBlock - Disable  */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Configure -  XBAR Retry response during Block to enable
	 * internal access - Disable
	 */
	reg = reg_read(REG_METAL_MASK_ADDR);
	/* [0] - RetryMask - Enable */
	reg |= (1 << REG_METAL_MASK_RETRY_OFFS);
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			/* Configure - Set CL */
			reg = reg_read(REG_DDR3_MR0_CS_ADDR +
				       (cs << MR_CS_ADDR_OFFS)) &
				~REG_DDR3_MR0_CL_MASK;
			if (freq == DDR_400)
				tmp = ddr3_cl_to_valid_cl(6);
			else
				tmp = ddr3_cl_to_valid_cl(dram_info->cl);
			reg |= ((tmp & 0x1) << REG_DDR3_MR0_CL_OFFS);
			reg |= ((tmp & 0xE) << REG_DDR3_MR0_CL_HIGH_OFFS);
			dfs_reg_write(REG_DDR3_MR0_CS_ADDR +
				      (cs << MR_CS_ADDR_OFFS), reg);

			/* Configure - Set CWL */
			reg = reg_read(REG_DDR3_MR2_CS_ADDR +
				       (cs << MR_CS_ADDR_OFFS)) &
				~(REG_DDR3_MR2_CWL_MASK << REG_DDR3_MR2_CWL_OFFS);
			if (freq == DDR_400)
				reg |= ((0) << REG_DDR3_MR2_CWL_OFFS);
			else
				reg |= ((dram_info->cwl) << REG_DDR3_MR2_CWL_OFFS);
			dfs_reg_write(REG_DDR3_MR2_CS_ADDR +
				      (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	DEBUG_DFS_C("DDR3 - DFS - Low To High - Ended successfuly - new Frequency - ",
		    freq, 1);

	return MV_OK;

#else

	/* This Flow is relevant for Armada370 A0 and ArmadaXP Z1 */

	u32 reg, freq_par, tmp;
	u32 cs = 0;

	DEBUG_DFS_C("DDR3 - DFS - Low To High - Starting DFS procedure to Frequency - ",
		    freq, 1);

	/* target frequency - freq */
	freq_par = ddr3_get_freq_parameter(freq, ratio_2to1);

	reg = 0x0000FF00;
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_1_ADDR, reg);

	/* 0x1600 - PHY lock mask register */
	reg = reg_read(REG_ODPG_CNTRL_ADDR);
	reg |= (1 << REG_ODPG_CNTRL_OFFS);	/* [21] = 1 */
	dfs_reg_write(REG_ODPG_CNTRL_ADDR, reg);

	/* 0x1670 - PHY lock mask register */
	reg = reg_read(REG_PHY_LOCK_MASK_ADDR);
	reg &= REG_PHY_LOCK_MASK_MASK;	/* [11:0] = 0 */
	dfs_reg_write(REG_PHY_LOCK_MASK_ADDR, reg);

	/* Enable reconfig MR Registers after DFS */
	reg = reg_read(REG_DFS_ADDR);	/* 0x1528 - DFS register */
	/* [4] - Disable - reconfig MR registers after DFS_ERG */
	reg &= ~0x11;
	/* [0] - Enable - DRAM DLL after DFS */
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Disable DRAM Controller to crossbar retry */
	/* [0] - disable */
	reg = reg_read(REG_METAL_MASK_ADDR) & ~(1 << 0);
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	/* Enable DRAM Blocking */
	/* [1] - DFS Block enable  */
	reg = reg_read(REG_DFS_ADDR) | (1 << REG_DFS_BLOCK_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Enable Self refresh */
	/* [2] - DFS Self refresh enable  */
	reg = reg_read(REG_DFS_ADDR) | (1 << REG_DFS_SR_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Poll DFS Register - All DRAM devices on all ranks are in
	 * self refresh mode - DFS can be executed afterwards
	 */
	/* 0x1528 [3] - DfsAtSR  */
	do {
		reg = reg_read(REG_DFS_ADDR) & (1 << REG_DFS_ATSR_OFFS);
	} while (reg == 0x0);	/* Wait for '1' */

	/*
	 * Set Correct Ratio - if freq>MARGIN_FREQ use 2:1 ratio
	 * else use 1:1 ratio
	 */
	if (ratio_2to1) {
		/* [15] = 1 - Set 2:1 Ratio between Dunit and Phy */
		reg = reg_read(REG_DDR_IO_ADDR) |
			(1 << REG_DDR_IO_CLK_RATIO_OFFS);
	} else {
		/* [15] = 0 - Set 1:1 Ratio between Dunit and Phy */
		reg = reg_read(REG_DDR_IO_ADDR) &
			~(1 << REG_DDR_IO_CLK_RATIO_OFFS);
	}
	dfs_reg_write(REG_DDR_IO_ADDR, reg);	/* 0x1524 - DDR IO Register */

	/* Switch HCLK Mux from (100Mhz) [16]=0, keep DFS request bit */
	reg = 0x20040000;
	/*
	 * [29] - training logic request DFS, [28:27] -
	 * preload patterns frequency [18]
	 */

	/* 0x18488 - DRAM Init control status register */
	dfs_reg_write(REG_DRAM_INIT_CTRL_STATUS_ADDR, reg);

	/* Add delay between entering SR and start ratio modification */
	udelay(1);

	/*
	 * Initial Setup - assure that the "load new ratio" is clear (bit 24)
	 * and in the same chance, block reassertions of reset [15:8] and
	 * force reserved bits[7:0].
	 */
	reg = 0x0000FFFF;
	/* 0x18700 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	/*
	 * RelaX whenever reset is asserted to that channel (good for any case)
	 */
	reg = 0x0000FF00;
	/* 0x18704 - CPU Div CLK control 0 */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_1_ADDR, reg);

	reg = reg_read(REG_CPU_DIV_CLK_CTRL_3_ADDR) &
		REG_CPU_DIV_CLK_CTRL_3_FREQ_MASK;
	reg |= (freq_par << REG_CPU_DIV_CLK_CTRL_3_FREQ_OFFS);
	/* Full Integer ratio from PLL-out to ddr-clk */
	/* 0x1870C - CPU Div CLK control 3 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_3_ADDR, reg);

	/*
	 * Shut off clock enable to the DDRPHY clock channel (this is the "D").
	 * All the rest are kept as is (forced, but could be read-modify-write).
	 * This is done now by RMW above.
	 */

	reg = 0x000FFF02;

	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_4_ADDR, reg);

	/* Wait before replacing the clock on the DDR Phy Channel. */
	udelay(1);

	reg = 0x0102FDFF;
	/*
	 * This for triggering the frequency update. Bit[24] is the
	 * central control
	 * bits [23:16] == which channels to change ==2 ==> only DDR Phy
	 *                 (smooth transition)
	 * bits [15:8] == mask reset reassertion due to clock modification
	 *                to these channels.
	 * bits [7:0] == not in use
	 */
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(1);

	/*
	 * Poll Div CLK status 0 register - indication that the clocks are
	 * active - 0x18718 [8]
	 */
	do {
		reg = reg_read(REG_CPU_DIV_CLK_STATUS_0_ADDR) &
			(1 << REG_CPU_DIV_CLK_ALL_STABLE_OFFS);
	} while (reg == 0);

	reg = 0x000000FF;
	/*
	 * Clean the CTRL0, to be ready for next resets and next requests of
	 * ratio modifications.
	 */
	/* 0x18700 - CPU Div CLK control 0 register */
	dfs_reg_write(REG_CPU_DIV_CLK_CTRL_0_ADDR, reg);

	udelay(5);

	if (ratio_2to1) {
		/* Pup Reset Divider B - Set Reset */
		/* [28] = 0 - Pup Reset Divider B */
		reg = reg_read(REG_SDRAM_CONFIG_ADDR) & ~(1 << 28);
		/* [28] = 1 - Pup Reset Divider B */
		tmp = reg_read(REG_SDRAM_CONFIG_ADDR) | (1 << 28);
		/* 0x1400 - SDRAM Configuration register */
		dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

		/* Pup Reset Divider B - Release Reset */
		/* 0x1400 - SDRAM Configuration register */
		dfs_reg_write(REG_SDRAM_CONFIG_ADDR, tmp);
	}

	/* DRAM Data PHYs ADLL Reset - Set Reset */
	reg = (reg_read(REG_DRAM_PHY_CONFIG_ADDR) & REG_DRAM_PHY_CONFIG_MASK);
	/* [31:30]] - reset pup data ctrl ADLL */
	/* 0x15EC - DRAM PHY Config Register */
	dfs_reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

	udelay(25);

	/* APLL lock indication - Poll Phy lock status Register - 0x1674 [9] */
	do {
		reg = reg_read(REG_PHY_LOCK_STATUS_ADDR) &
			(1 << REG_PHY_LOCK_STATUS_LOCK_OFFS);
	} while (reg == 0);

	/* DRAM Data PHYs ADLL Reset - Release Reset */
	reg = reg_read(REG_DRAM_PHY_CONFIG_ADDR) | ~REG_DRAM_PHY_CONFIG_MASK;
	/* [31:30] - normal pup data ctrl ADLL */
	/* 0x15EC - DRAM PHY Config register */
	dfs_reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

	udelay(10000);		/* Wait 10msec */

	/*
	 * APLL lock indication - Poll Phy lock status Register - 0x1674 [11:0]
	 */
	do {
		reg = reg_read(REG_PHY_LOCK_STATUS_ADDR) &
			REG_PHY_LOCK_STATUS_LOCK_MASK;
	} while (reg != REG_PHY_LOCK_STATUS_LOCK_MASK);

	/* DRAM Data PHY Read [30], Write [29] path reset - Set Reset */
	reg = reg_read(REG_SDRAM_CONFIG_ADDR) & REG_SDRAM_CONFIG_MASK;
	/* [30:29] = 0 - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/* DRAM Data PHY Read [30], Write [29] path reset - Release Reset */
	reg = reg_read(REG_SDRAM_CONFIG_ADDR) | ~REG_SDRAM_CONFIG_MASK;
	/* [30:29] = '11' - Data Pup R/W path reset */
	/* 0x1400 - SDRAM Configuration register */
	dfs_reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/* Disable DFS Reconfig */
	reg = reg_read(REG_DFS_ADDR) & ~(1 << 4);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* [2] - DFS Self refresh disable  */
	reg = reg_read(REG_DFS_ADDR) & ~(1 << REG_DFS_SR_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/*
	 * Poll DFS Register - 0x1528 [3] - DfsAtSR - All DRAM devices on
	 * all ranks are NOT in self refresh mode
	 */
	do {
		reg = reg_read(REG_DFS_ADDR) & (1 << REG_DFS_ATSR_OFFS);
	} while (reg);		/* Wait for '0' */

	/* 0x1404 */
	reg = (reg_read(REG_DUNIT_CTRL_LOW_ADDR) & 0xFFFFFFE7) | 0x2;

	/* Configure - 2T Mode - Restore original configuration */
	/* [3:4] 2T - Restore value */
	reg &= ~(REG_DUNIT_CTRL_LOW_2T_MASK << REG_DUNIT_CTRL_LOW_2T_OFFS);
	reg |= ((dram_info->mode_2t & REG_DUNIT_CTRL_LOW_2T_MASK) <<
		REG_DUNIT_CTRL_LOW_2T_OFFS);
	dfs_reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);

	udelay(1);		/* Wait 1us */

	for (cs = 0; cs < MAX_CS; cs++) {
		if (dram_info->cs_ena & (1 << cs)) {
			reg = (reg_read(REG_DDR3_MR1_ADDR));
			/* DLL Enable */
			reg &= ~(1 << REG_DDR3_MR1_DLL_ENA_OFFS);
			dfs_reg_write(REG_DDR3_MR1_ADDR, reg);

			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR1 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			/* DLL Reset - MR0 */
			reg = reg_read(REG_DDR3_MR0_ADDR);
			dfs_reg_write(REG_DDR3_MR0_ADDR, reg);

			/* Issue MRS Command to current cs */
			reg = REG_SDRAM_OPERATION_CMD_MR0 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/*
			 * [3-0] = 0x4 - MR1 Command, [11-8] -
			 * enable current cs
			 */
			/* 0x1418 - SDRAM Operation Register */
			dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			reg = reg_read(REG_DDR3_MR0_ADDR);
			reg &= ~0x74;	/* CL [3:0]; [6:4],[2] */

			if (freq == DDR_400)
				tmp = ddr3_cl_to_valid_cl(6) & 0xF;
			else
				tmp = ddr3_cl_to_valid_cl(dram_info->cl) & 0xF;

			reg |= ((tmp & 0x1) << 2);
			reg |= ((tmp >> 1) << 4);	/* to bit 4 */
			dfs_reg_write(REG_DDR3_MR0_ADDR, reg);

			reg = REG_SDRAM_OPERATION_CMD_MR0 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/* 0x1418 - SDRAM Operation Register */
			dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			reg = reg_read(REG_DDR3_MR2_ADDR);
			reg &= ~0x38;	/* CWL [5:3] */
			/* CWL = 0 ,for 400 MHg is 5 */
			if (freq != DDR_400)
				reg |= dram_info->cwl << REG_DDR3_MR2_CWL_OFFS;
			dfs_reg_write(REG_DDR3_MR2_ADDR, reg);
			reg = REG_SDRAM_OPERATION_CMD_MR2 &
				~(1 << (REG_SDRAM_OPERATION_CS_OFFS + cs));
			/* 0x1418 - SDRAM Operation Register */
			dfs_reg_write(REG_SDRAM_OPERATION_ADDR, reg);

			/* Poll - Wait for Refresh operation completion */
			wait_refresh_op_complete();

			/* Set current rd_sample_delay  */
			reg = reg_read(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
			reg &= ~(REG_READ_DATA_SAMPLE_DELAYS_MASK <<
				 (REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			reg |= (dram_info->cl <<
				(REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			dfs_reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR, reg);

			/* Set current rd_ready_delay  */
			reg = reg_read(REG_READ_DATA_READY_DELAYS_ADDR);
			reg &= ~(REG_READ_DATA_READY_DELAYS_MASK <<
				 (REG_READ_DATA_READY_DELAYS_OFFS * cs));
			reg |= ((dram_info->cl + 1) <<
				(REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
			dfs_reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
		}
	}

	/* Enable ODT on DLL-on mode */
	dfs_reg_write(REG_SDRAM_ODT_CTRL_HIGH_ADDR, 0);

	/* [1] - DFS Block disable  */
	reg = reg_read(REG_DFS_ADDR) & ~(1 << REG_DFS_BLOCK_OFFS);
	dfs_reg_write(REG_DFS_ADDR, reg);	/* 0x1528 - DFS register */

	/* Change DDR frequency to 100MHz procedure: */
	/* 0x1600 - PHY lock mask register */
	reg = reg_read(REG_ODPG_CNTRL_ADDR);
	reg &= ~(1 << REG_ODPG_CNTRL_OFFS);	/* [21] = 0 */
	dfs_reg_write(REG_ODPG_CNTRL_ADDR, reg);

	/* Change DDR frequency to 100MHz procedure: */
	/* 0x1670 - PHY lock mask register */
	reg = reg_read(REG_PHY_LOCK_MASK_ADDR);
	reg |= ~REG_PHY_LOCK_MASK_MASK;	/* [11:0] = FFF */
	dfs_reg_write(REG_PHY_LOCK_MASK_ADDR, reg);

	reg = reg_read(REG_METAL_MASK_ADDR) | (1 << 0);	/* [0] - disable */
	/* 0x14B0 - Dunit MMask Register */
	dfs_reg_write(REG_METAL_MASK_ADDR, reg);

	DEBUG_DFS_C("DDR3 - DFS - Low To High - Ended successfuly - new Frequency - ",
		    freq, 1);
	return MV_OK;
#endif
}
