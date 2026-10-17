// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

static unsigned int g_cdd_rr_max[4];
static unsigned int g_cdd_rw_max[4];
static unsigned int g_cdd_wr_max[4];
static unsigned int g_cdd_ww_max[4];

void ddr_cfg_umctl2(struct dram_cfg_param *ddrc_cfg, int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		reg32_write(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}
}

#ifdef CONFIG_IMX8M_DRAM_INLINE_ECC
void ddrc_inline_ecc_scrub(unsigned int start_address,
			   unsigned int range_address)
{
	unsigned int tmp;

	/* Step1: Enable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000000);
	/* Step2: Set ECCCFG1.ecc_parity_region_lock to 1 */
	reg32setbit(DDRC_ECCCFG1(0), 0x4);
	/* Step3: Block the AXI ports from taking the transaction */
	reg32_write(DDRC_PCTRL_0(0), 0x0);
	/* Step4: Set scrub start address */
	reg32_write(DDRC_SBRSTART0(0), start_address);
	/* Step5: Set scrub range address */
	reg32_write(DDRC_SBRRANGE0(0), range_address);
	/* Step6: Set scrub_mode to write */
	reg32_write(DDRC_SBRCTL(0), 0x00000014);
	/* Step7: Set the desired pattern through SBRWDATA0 registers */
	reg32_write(DDRC_SBRWDATA0(0), 0x55aa55aa);
	/* Step8: Enable the SBR by programming SBRCTL.scrub_en=1 */
	reg32setbit(DDRC_SBRCTL(0), 0x0);
	/* Step9: Poll SBRSTAT.scrub_done=1 */
	tmp = reg32_read(DDRC_SBRSTAT(0));
	while (tmp != 0x00000002)
		tmp = reg32_read(DDRC_SBRSTAT(0)) & 0x2;
	/* Step10: Poll SBRSTAT.scrub_busy=0 */
	tmp = reg32_read(DDRC_SBRSTAT(0));
	while (tmp != 0x0)
		tmp = reg32_read(DDRC_SBRSTAT(0)) & 0x1;
	/* Step11: Disable SBR by programming SBRCTL.scrub_en=0 */
	clrbits_le32(DDRC_SBRCTL(0), 0x1);
	/* Step12: Prepare for normal scrub operation(Read) and set scrub_interval*/
	reg32_write(DDRC_SBRCTL(0), 0x100);
	/* Step13: Enable the SBR by programming SBRCTL.scrub_en=1 */
	reg32_write(DDRC_SBRCTL(0), 0x101);
	/* Step14: Enable AXI ports by programming */
	reg32_write(DDRC_PCTRL_0(0), 0x1);
	/* Step15: Disable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
}

void ddrc_inline_ecc_scrub_end(unsigned int start_address,
			       unsigned int range_address)
{
	/* Step1: Enable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000000);
	/* Step2: Block the AXI ports from taking the transaction */
	reg32_write(DDRC_PCTRL_0(0), 0x0);
	/* Step3: Set scrub start address */
	reg32_write(DDRC_SBRSTART0(0), start_address);
	/* Step4: Set scrub range address */
	reg32_write(DDRC_SBRRANGE0(0), range_address);
	/* Step5: Disable SBR by programming SBRCTL.scrub_en=0 */
	clrbits_le32(DDRC_SBRCTL(0), 0x1);
	/* Step6: Prepare for normal scrub operation(Read) and set scrub_interval */
	reg32_write(DDRC_SBRCTL(0), 0x100);
	/* Step7: Enable the SBR by programming SBRCTL.scrub_en=1 */
	reg32_write(DDRC_SBRCTL(0), 0x101);
	/* Step8: Enable AXI ports by programming */
	reg32_write(DDRC_PCTRL_0(0), 0x1);
	/* Step9: Disable quasi-dynamic programming */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
}
#endif

void __weak board_dram_ecc_scrub(void)
{
}

void lpddr4_mr_write(unsigned int mr_rank, unsigned int mr_addr,
		     unsigned int mr_data)
{
	unsigned int tmp;
	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0.
	 * This checks that there is no outstanding MR transaction.
	 * No writes should be performed to MRCTRL0 and MRCTRL1 if
	 * MRSTAT.mr_wr_busy = 1.
	 */
	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);
	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and
	 * (for MRWs) MRCTRL1.mr_data to define the MR transaction.
	 */
	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4));
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8) | mr_data);
	reg32setbit(DDRC_MRCTRL0(0), 31);
}

unsigned int lpddr4_mr_read(unsigned int mr_rank, unsigned int mr_addr)
{
	unsigned int tmp;

	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x1);
	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);

	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4) | 0x1);
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8));
	reg32setbit(DDRC_MRCTRL0(0), 31);
	do {
		tmp = reg32_read(DRC_PERF_MON_MRR0_DAT(0));
	} while ((tmp & 0x8) == 0);
	tmp = reg32_read(DRC_PERF_MON_MRR1_DAT(0));
	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x4);
	while (tmp) { //try to find a significant byte in the word
		if (tmp & 0xff) {
			tmp &= 0xff;
			break;
		}
		tmp >>= 8;
	}

	return tmp;
}

static unsigned int look_for_max(unsigned int data[], unsigned int addr_start,
				 unsigned int addr_end)
{
	unsigned int i, imax = 0;

	for (i = addr_start; i <= addr_end; i++) {
		if (((data[i] >> 7) == 0) && data[i] > imax)
			imax = data[i];
	}

	return imax;
}

void get_trained_CDD(u32 fsp)
{
	unsigned int i, ddr_type, tmp;
	unsigned int cdd_cha[12], cdd_chb[12];
	unsigned int cdd_cha_rr_max, cdd_cha_rw_max, cdd_cha_wr_max, cdd_cha_ww_max;
	unsigned int cdd_chb_rr_max, cdd_chb_rw_max, cdd_chb_wr_max, cdd_chb_ww_max;

	ddr_type = reg32_read(DDRC_MSTR(0)) & 0x3f;
	if (ddr_type == 0x20) {
		for (i = 0; i < 6; i++) {
			tmp = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + (0x54013 + i) * 4);
			cdd_cha[i * 2] = tmp & 0xff;
			cdd_cha[i * 2 + 1] = (tmp >> 8) & 0xff;
		}

		for (i = 0; i < 7; i++) {
			tmp = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + (0x5402c + i) * 4);
			if (i == 0) {
				cdd_cha[0] = (tmp >> 8) & 0xff;
			} else if (i == 6) {
				cdd_cha[11] = tmp & 0xff;
			} else {
				cdd_chb[i * 2 - 1] = tmp & 0xff;
				cdd_chb[i * 2] = (tmp >> 8) & 0xff;
			}
		}

		cdd_cha_rr_max = look_for_max(cdd_cha, 0, 1);
		cdd_cha_rw_max = look_for_max(cdd_cha, 2, 5);
		cdd_cha_wr_max = look_for_max(cdd_cha, 6, 9);
		cdd_cha_ww_max = look_for_max(cdd_cha, 10, 11);
		cdd_chb_rr_max = look_for_max(cdd_chb, 0, 1);
		cdd_chb_rw_max = look_for_max(cdd_chb, 2, 5);
		cdd_chb_wr_max = look_for_max(cdd_chb, 6, 9);
		cdd_chb_ww_max = look_for_max(cdd_chb, 10, 11);
		g_cdd_rr_max[fsp] =
			cdd_cha_rr_max > cdd_chb_rr_max ? cdd_cha_rr_max : cdd_chb_rr_max;
		g_cdd_rw_max[fsp] =
			cdd_cha_rw_max > cdd_chb_rw_max ? cdd_cha_rw_max : cdd_chb_rw_max;
		g_cdd_wr_max[fsp] =
			cdd_cha_wr_max > cdd_chb_wr_max ? cdd_cha_wr_max : cdd_chb_wr_max;
		g_cdd_ww_max[fsp] =
			cdd_cha_ww_max > cdd_chb_ww_max ? cdd_cha_ww_max : cdd_chb_ww_max;
	} else {
		unsigned int ddr4_cdd[64];

		for (i = 0; i < 29; i++) {
			tmp = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + (0x54012 + i) * 4);
			ddr4_cdd[i * 2] = tmp & 0xff;
			ddr4_cdd[i * 2 + 1] = (tmp >> 8) & 0xff;
		}

		g_cdd_rr_max[fsp] = look_for_max(ddr4_cdd, 1, 12);
		g_cdd_ww_max[fsp] = look_for_max(ddr4_cdd, 13, 24);
		g_cdd_rw_max[fsp] = look_for_max(ddr4_cdd, 25, 40);
		g_cdd_wr_max[fsp] = look_for_max(ddr4_cdd, 41, 56);
	}
}

void update_umctl2_rank_space_setting(unsigned int pstat_num)
{
	unsigned int i, ddr_type;
	unsigned int addr_slot, rdata, tmp, tmp_t;
	unsigned int ddrc_w2r, ddrc_r2w, ddrc_wr_gap, ddrc_rd_gap;

	ddr_type = reg32_read(DDRC_MSTR(0)) & 0x3f;
	for (i = 0; i < pstat_num; i++) {
		addr_slot = i ? (i + 1) * 0x1000 : 0;
		if (ddr_type == 0x20) {
			/* update r2w:[13:8], w2r:[5:0] */
			rdata = reg32_read(DDRC_DRAMTMG2(0) + addr_slot);
			ddrc_w2r = rdata & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1);
			else
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1) + 1;
			ddrc_w2r = (tmp > 0x3f) ? 0x3f : tmp;

			ddrc_r2w = (rdata >> 8) & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1);
			else
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1) + 1;
			ddrc_r2w = (tmp > 0x3f) ? 0x3f : tmp;

			tmp_t = (rdata & 0xffffc0c0) | (ddrc_r2w << 8) | ddrc_w2r;
			reg32_write((DDRC_DRAMTMG2(0) + addr_slot), tmp_t);
		} else {
			/* update w2r:[5:0] */
			rdata = reg32_read(DDRC_DRAMTMG9(0) + addr_slot);
			ddrc_w2r = rdata & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1);
			else
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1) + 1;
			ddrc_w2r = (tmp > 0x3f) ? 0x3f : tmp;
			tmp_t = (rdata & 0xffffffc0) | ddrc_w2r;
			reg32_write((DDRC_DRAMTMG9(0) + addr_slot), tmp_t);

			/* update r2w:[13:8] */
			rdata = reg32_read(DDRC_DRAMTMG2(0) + addr_slot);
			ddrc_r2w = (rdata >> 8) & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1);
			else
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1) + 1;
			ddrc_r2w = (tmp > 0x3f) ? 0x3f : tmp;

			tmp_t = (rdata & 0xffffc0ff) | (ddrc_r2w << 8);
			reg32_write((DDRC_DRAMTMG2(0) + addr_slot), tmp_t);
		}

		if (!is_imx8mq()) {
			/*
			 * update rankctl: wr_gap:11:8; rd:gap:7:4; quasi-dymic, doc wrong(static)
			 */
			rdata = reg32_read(DDRC_RANKCTL(0) + addr_slot);
			ddrc_wr_gap = (rdata >> 8) & 0xf;
			if (is_imx8mp())
				tmp = ddrc_wr_gap + (g_cdd_ww_max[i] >> 1);
			else
				tmp = ddrc_wr_gap + (g_cdd_ww_max[i] >> 1) + 1;
			ddrc_wr_gap = (tmp > 0xf) ? 0xf : tmp;

			ddrc_rd_gap = (rdata >> 4) & 0xf;
			if (is_imx8mp())
				tmp = ddrc_rd_gap + (g_cdd_rr_max[i] >> 1);
			else
				tmp = ddrc_rd_gap + (g_cdd_rr_max[i] >> 1) + 1;
			ddrc_rd_gap = (tmp > 0xf) ? 0xf : tmp;

			tmp_t = (rdata & 0xfffff00f) | (ddrc_wr_gap << 8) | (ddrc_rd_gap << 4);
			reg32_write((DDRC_RANKCTL(0) + addr_slot), tmp_t);
		}
	}

	if (is_imx8mq()) {
		/* update rankctl: wr_gap:11:8; rd:gap:7:4; quasi-dymic, doc wrong(static) */
		rdata = reg32_read(DDRC_RANKCTL(0));
		ddrc_wr_gap = (rdata >> 8) & 0xf;
		tmp = ddrc_wr_gap + (g_cdd_ww_max[0] >> 1) + 1;
		ddrc_wr_gap = (tmp > 0xf) ? 0xf : tmp;

		ddrc_rd_gap = (rdata >> 4) & 0xf;
		tmp = ddrc_rd_gap + (g_cdd_rr_max[0] >> 1) + 1;
		ddrc_rd_gap = (tmp > 0xf) ? 0xf : tmp;

		tmp_t = (rdata & 0xfffff00f) | (ddrc_wr_gap << 8) | (ddrc_rd_gap << 4);
		reg32_write(DDRC_RANKCTL(0), tmp_t);
	}
}

int ddr_init(struct dram_timing_info *dram_timing)
{
	unsigned int tmp, initial_drate, target_freq;
	int ret;

	debug("DDRINFO: start DRAM init\n");

	/* Step1: Follow the power up procedure */
	if (is_imx8mq()) {
		reg32_write(SRC_DDRC_RCR_ADDR + 0x04, 0x8F00000F);
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);
		reg32_write(SRC_DDRC_RCR_ADDR + 0x04, 0x8F000000);
	} else {
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00001F);
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);
	}

	debug("DDRINFO: cfg clk\n");
	/* change the clock source of dram_apb_clk_root: source 4 800MHz /4 = 200MHz */
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(4) |
			     CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV4));

	/* disable iso */
	reg32_write(0x303A00EC, 0x0000ffff); /* PGC_CPU_MAPPING */
	reg32setbit(0x303A00F8, 5); /* PU_PGC_SW_PUP_REQ */

	initial_drate = dram_timing->fsp_msg[0].drate;
	/* default to the frequency point 0 clock */
	ddrphy_init_set_dfi_clk(initial_drate);

	/* D-aasert the presetn */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000006);

	/* Step2: Program the dwc_ddr_umctl2 registers */
	debug("DDRINFO: ddrc config start\n");
	ddr_cfg_umctl2(dram_timing->ddrc_cfg, dram_timing->ddrc_cfg_num);
	debug("DDRINFO: ddrc config done\n");

	/* Step3: De-assert reset signal(core_ddrc_rstn & aresetn_n) */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000004);
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000000);

	/*
	 * Step4: Disable auto-refreshes, self-refresh, powerdown, and
	 * assertion of dfi_dram_clk_disable by setting RFSHCTL3.dis_auto_refresh = 1,
	 * PWRCTL.powerdown_en = 0, and PWRCTL.selfref_en = 0, PWRCTL.en_dfi_dram_clk_disable = 0
	 */
	reg32_write(DDRC_DBG1(0), 0x00000000);
	reg32_write(DDRC_RFSHCTL3(0), 0x0000001);
	reg32_write(DDRC_PWRCTL(0), 0xa0);

	/* if ddr type is LPDDR4, do it */
	tmp = reg32_read(DDRC_MSTR(0));
	if (tmp & (0x1 << 5) && !is_imx8mn())
		reg32_write(DDRC_DDR_SS_GPR0, 0x01); /* LPDDR4 mode */

	/* determine the initial boot frequency */
	target_freq = reg32_read(DDRC_MSTR2(0)) & 0x3;
	target_freq = (tmp & (0x1 << 29)) ? target_freq : 0x0;

	/* Step5: Set SWCT.sw_done to 0 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* Set the default boot frequency point */
	clrsetbits_le32(DDRC_DFIMISC(0), (0x1f << 8), target_freq << 8);
	/* Step6: Set DFIMISC.dfi_init_complete_en to 0 */
	clrbits_le32(DDRC_DFIMISC(0), 0x1);

	/* Step7: Set SWCTL.sw_done to 1; need to polling SWSTAT.sw_done_ack */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/*
	 * Step8 ~ Step13: Start PHY initialization and training by
	 * accessing relevant PUB registers
	 */
	debug("DDRINFO:ddrphy config start\n");

	ret = ddr_cfg_phy(dram_timing);
	if (ret)
		return ret;

	debug("DDRINFO: ddrphy config done\n");

	/*
	 * step14 CalBusy.0 =1, indicates the calibrator is actively
	 * calibrating. Wait Calibrating done.
	 */
	do {
		tmp = reg32_read(DDRPHY_CalBusy(0));
	} while ((tmp & 0x1));

	debug("DDRINFO:ddrphy calibration done\n");

	/* Step15: Set SWCTL.sw_done to 0 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* Apply rank-to-rank workaround */
	update_umctl2_rank_space_setting(dram_timing->fsp_msg_num - 1);

	/* Step16: Set DFIMISC.dfi_init_start to 1 */
	setbits_le32(DDRC_DFIMISC(0), (0x1 << 5));

	/* Step17: Set SWCTL.sw_done to 1; need to polling SWSTAT.sw_done_ack */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* Step18: Polling DFISTAT.dfi_init_complete = 1 */
	do {
		tmp = reg32_read(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* Step19: Set SWCTL.sw_done to 0 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* Step20: Set DFIMISC.dfi_init_start to 0 */
	clrbits_le32(DDRC_DFIMISC(0), (0x1 << 5));

	/* Step21: optional */

	/* Step22: Set DFIMISC.dfi_init_complete_en to 1 */
	setbits_le32(DDRC_DFIMISC(0), 0x1);

	/* Step23: Set PWRCTL.selfref_sw to 0 */
	clrbits_le32(DDRC_PWRCTL(0), (0x1 << 5));

	/* Step24: Set SWCTL.sw_done to 1; need polling SWSTAT.sw_done_ack */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* Step25: Wait for dwc_ddr_umctl2 to move to normal operating mode by monitoring
	 * STAT.operating_mode signal */
	do {
		tmp = reg32_read(DDRC_STAT(0));
	} while ((tmp & 0x3) != 0x1);

	/* Step26: Set back register in Step4 to the original values if desired */
	reg32_write(DDRC_RFSHCTL3(0), 0x0000000);

	/* enable port 0 */
	reg32_write(DDRC_PCTRL_0(0), 0x00000001);
	debug("DDRINFO: ddrmix config done\n");

	board_dram_ecc_scrub();

	/* enable selfref_en by default */
	setbits_le32(DDRC_PWRCTL(0), 0x1);

	/* save the dram timing config into memory */
	dram_config_save(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);

	return 0;
}

ulong ddrphy_addr_remap(uint32_t paddr_apb_from_ctlr)
{
	return 4 * paddr_apb_from_ctlr;
}
