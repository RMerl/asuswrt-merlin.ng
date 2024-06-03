// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8m_ddr.h>
#include <asm/arch/sys_proto.h>

void ddr4_cfg_umctl2(struct dram_cfg_param *ddrc_cfg, int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		reg32_write(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}
}

void ddr_init(struct dram_timing_info *dram_timing)
{
	volatile unsigned int tmp_t;
	/*
	 * assert [0]ddr1_preset_n, [1]ddr1_core_reset_n,
	 * [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n,
	 * [4]src_system_rst_b!
	 */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00003F);
	/* deassert [4]src_system_rst_b! */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);

	/*
	 * change the clock source of dram_apb_clk_root
	 * to source 4 --800MHz/4
	 */
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(4) |
			     CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV4));

	dram_pll_init(DRAM_PLL_OUT_600M);

	reg32_write(0x303A00EC, 0x0000ffff); /* PGC_CPU_MAPPING */
	reg32setbit(0x303A00F8, 5); /* PU_PGC_SW_PUP_REQ */

	/* release [0]ddr1_preset_n, [3]ddr1_phy_pwrokin_n */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000006);

	reg32_write(DDRC_DBG1(0), 0x00000001);
	reg32_write(DDRC_PWRCTL(0), 0x00000001);

	while (0 != (0x7 & reg32_read(DDRC_STAT(0))))
		;

	/* config the uMCTL2's registers */
	ddr4_cfg_umctl2(dram_timing->ddrc_cfg, dram_timing->ddrc_cfg_num);

	reg32_write(DDRC_RFSHCTL3(0), 0x00000001);
	/* RESET: <ctn> DEASSERTED */
	/* RESET: <a Port 0  DEASSERTED(0) */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000004);
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000000);

	reg32_write(DDRC_DBG1(0), 0x00000000);
	reg32_write(DDRC_PWRCTL(0), 0x00000aa);
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	reg32_write(DDRC_DFIMISC(0), 0x00000000);

	/* config the DDR PHY's registers */
	ddr_cfg_phy(dram_timing);

	do {
		tmp_t = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) +
				   4 * 0x00020097);
	} while (tmp_t != 0);

	reg32_write(DDRC_DFIMISC(0), 0x00000020);

	/* wait DFISTAT.dfi_init_complete to 1 */
	while (0 == (0x1 & reg32_read(DDRC_DFISTAT(0))))
		;

	/* clear DFIMISC.dfi_init_complete_en */
	reg32_write(DDRC_DFIMISC(0), 0x00000000);
	/* set DFIMISC.dfi_init_complete_en again */
	reg32_write(DDRC_DFIMISC(0), 0x00000001);
	reg32_write(DDRC_PWRCTL(0), 0x0000088);

	/*
	 * set SWCTL.sw_done to enable quasi-dynamic register
	 * programming outside reset.
	 */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	/* wait SWSTAT.sw_done_ack to 1 */
	while (0 == (0x1 & reg32_read(DDRC_SWSTAT(0))))
		;

	/* wait STAT to normal state */
	while (0x1 != (0x7 & reg32_read(DDRC_STAT(0))))
		;

	reg32_write(DDRC_PWRCTL(0), 0x0000088);
	reg32_write(DDRC_PCTRL_0(0), 0x00000001);
	/* dis_auto-refresh is set to 0 */
	reg32_write(DDRC_RFSHCTL3(0), 0x00000000);

	/* save the dram timing config into memory */
	dram_config_save(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);
}
