// SPDX-License-Identifier: GPL-2.0+
/*
* Copyright 2018 NXP
*
*/

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/lpddr4_define.h>
#include <asm/arch/sys_proto.h>

void lpddr4_cfg_umctl2(struct dram_cfg_param *ddrc_cfg, int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		reg32_write(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}
}

void ddr_init(struct dram_timing_info *dram_timing)
{
	unsigned int tmp;

	debug("DDRINFO: start lpddr4 ddr init\n");
	/* step 1: reset */
	if (is_imx8mq()) {
		reg32_write(SRC_DDRC_RCR_ADDR + 0x04, 0x8F00000F);
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);
		reg32_write(SRC_DDRC_RCR_ADDR + 0x04, 0x8F000000);
	} else {
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00001F);
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);
	}

	mdelay(100);

	debug("DDRINFO: reset done\n");
	/*
	 * change the clock source of dram_apb_clk_root:
	 * source 4 800MHz /4 = 200MHz
	 */
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(4) |
			     CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV4));

	/* disable iso */
	reg32_write(0x303A00EC, 0x0000ffff); /* PGC_CPU_MAPPING */
	reg32setbit(0x303A00F8, 5); /* PU_PGC_SW_PUP_REQ */

	debug("DDRINFO: cfg clk\n");
	dram_pll_init(MHZ(750));

	/*
	 * release [0]ddr1_preset_n, [1]ddr1_core_reset_n,
	 * [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n
	 */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000006);

	/*step2 Configure uMCTL2's registers */
	debug("DDRINFO: ddrc config start\n");
	lpddr4_cfg_umctl2(dram_timing->ddrc_cfg, dram_timing->ddrc_cfg_num);
	debug("DDRINFO: ddrc config done\n");

	/*
	 * step3 de-assert all reset
	 * RESET: <core_ddrc_rstn> DEASSERTED
	 * RESET: <aresetn> for Port 0  DEASSERT(0)ED
	 */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000004);
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000000);

	reg32_write(DDRC_DBG1(0), 0x00000000);
	/* step4 */
	/* [0]dis_auto_refresh=1 */
	reg32_write(DDRC_RFSHCTL3(0), 0x00000011);

	/* [8]--1: lpddr4_sr allowed; [5]--1: software entry to SR */
	reg32_write(DDRC_PWRCTL(0), 0x000000a8);

	do {
		tmp = reg32_read(DDRC_STAT(0));
	} while ((tmp & 0x33f) != 0x223);

	reg32_write(DDRC_DDR_SS_GPR0, 0x01); /* LPDDR4 mode */

	/* step5 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* step6 */
	tmp = reg32_read(DDRC_MSTR2(0));
	if (tmp == 0x2)
		reg32_write(DDRC_DFIMISC(0), 0x00000210);
	else if (tmp == 0x1)
		reg32_write(DDRC_DFIMISC(0), 0x00000110);
	else
		reg32_write(DDRC_DFIMISC(0), 0x00000010);

	/* step7 [0]--1: disable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000001);

	/* step8 Configure LPDDR4 PHY's registers */
	debug("DDRINFO:ddrphy config start\n");
	ddr_cfg_phy(dram_timing);
	debug("DDRINFO: ddrphy config done\n");

	/*
	 * step14 CalBusy.0 =1, indicates the calibrator is actively
	 * calibrating. Wait Calibrating done.
	 */
	do {
		tmp = reg32_read(DDRPHY_CalBusy(0));
	} while ((tmp & 0x1));

	debug("DDRINFO:ddrphy calibration done\n");

	/* step15 [0]--0: to enable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* step16 */
	tmp = reg32_read(DDRC_MSTR2(0));
	if (tmp == 0x2)
		reg32_write(DDRC_DFIMISC(0), 0x00000230);
	else if (tmp == 0x1)
		reg32_write(DDRC_DFIMISC(0), 0x00000130);
	else
		reg32_write(DDRC_DFIMISC(0), 0x00000030);

	/* step17 [0]--1: disable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	/* step18 wait DFISTAT.dfi_init_complete to 1 */
	do {
		tmp = reg32_read(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* step19 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* step20~22 */
	tmp = reg32_read(DDRC_MSTR2(0));
	if (tmp == 0x2) {
		reg32_write(DDRC_DFIMISC(0), 0x00000210);
		/* set DFIMISC.dfi_init_complete_en again */
		reg32_write(DDRC_DFIMISC(0), 0x00000211);
	} else if (tmp == 0x1) {
		reg32_write(DDRC_DFIMISC(0), 0x00000110);
		/* set DFIMISC.dfi_init_complete_en again */
		reg32_write(DDRC_DFIMISC(0), 0x00000111);
	} else {
		/* clear DFIMISC.dfi_init_complete_en */
		reg32_write(DDRC_DFIMISC(0), 0x00000010);
		/* set DFIMISC.dfi_init_complete_en again */
		reg32_write(DDRC_DFIMISC(0), 0x00000011);
	}

	/* step23 [5]selfref_sw=0; */
	reg32_write(DDRC_PWRCTL(0), 0x00000008);
	/* step24 sw_done=1 */
	reg32_write(DDRC_SWCTL(0), 0x00000001);

	/* step25 wait SWSTAT.sw_done_ack to 1 */
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

#ifdef DFI_BUG_WR
	reg32_write(DDRC_DFIPHYMSTR(0), 0x00000001);
#endif
	/* wait STAT.operating_mode([1:0] for ddr3) to normal state */
	do {
		tmp = reg32_read(DDRC_STAT(0));
	} while ((tmp & 0x3) != 0x1);

	/* step26 */
	reg32_write(DDRC_RFSHCTL3(0), 0x00000010);

	/* enable port 0 */
	reg32_write(DDRC_PCTRL_0(0), 0x00000001);
	debug("DDRINFO: ddrmix config done\n");

	/* save the dram timing config into memory */
	dram_config_save(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);
}
