// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Toradex, Inc.
 *
 * Based on vf610twr:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/arch/ddrmc-vf610.h>
#include <linux/delay.h>
#include "ddrmc-vf610-calibration.h"

void ddrmc_setup_iomux(const iomux_v3_cfg_t *pads, int pads_count)
{
	static const iomux_v3_cfg_t default_pads[] = {
		VF610_PAD_DDR_A15__DDR_A_15,
		VF610_PAD_DDR_A14__DDR_A_14,
		VF610_PAD_DDR_A13__DDR_A_13,
		VF610_PAD_DDR_A12__DDR_A_12,
		VF610_PAD_DDR_A11__DDR_A_11,
		VF610_PAD_DDR_A10__DDR_A_10,
		VF610_PAD_DDR_A9__DDR_A_9,
		VF610_PAD_DDR_A8__DDR_A_8,
		VF610_PAD_DDR_A7__DDR_A_7,
		VF610_PAD_DDR_A6__DDR_A_6,
		VF610_PAD_DDR_A5__DDR_A_5,
		VF610_PAD_DDR_A4__DDR_A_4,
		VF610_PAD_DDR_A3__DDR_A_3,
		VF610_PAD_DDR_A2__DDR_A_2,
		VF610_PAD_DDR_A1__DDR_A_1,
		VF610_PAD_DDR_A0__DDR_A_0,
		VF610_PAD_DDR_BA2__DDR_BA_2,
		VF610_PAD_DDR_BA1__DDR_BA_1,
		VF610_PAD_DDR_BA0__DDR_BA_0,
		VF610_PAD_DDR_CAS__DDR_CAS_B,
		VF610_PAD_DDR_CKE__DDR_CKE_0,
		VF610_PAD_DDR_CLK__DDR_CLK_0,
		VF610_PAD_DDR_CS__DDR_CS_B_0,
		VF610_PAD_DDR_D15__DDR_D_15,
		VF610_PAD_DDR_D14__DDR_D_14,
		VF610_PAD_DDR_D13__DDR_D_13,
		VF610_PAD_DDR_D12__DDR_D_12,
		VF610_PAD_DDR_D11__DDR_D_11,
		VF610_PAD_DDR_D10__DDR_D_10,
		VF610_PAD_DDR_D9__DDR_D_9,
		VF610_PAD_DDR_D8__DDR_D_8,
		VF610_PAD_DDR_D7__DDR_D_7,
		VF610_PAD_DDR_D6__DDR_D_6,
		VF610_PAD_DDR_D5__DDR_D_5,
		VF610_PAD_DDR_D4__DDR_D_4,
		VF610_PAD_DDR_D3__DDR_D_3,
		VF610_PAD_DDR_D2__DDR_D_2,
		VF610_PAD_DDR_D1__DDR_D_1,
		VF610_PAD_DDR_D0__DDR_D_0,
		VF610_PAD_DDR_DQM1__DDR_DQM_1,
		VF610_PAD_DDR_DQM0__DDR_DQM_0,
		VF610_PAD_DDR_DQS1__DDR_DQS_1,
		VF610_PAD_DDR_DQS0__DDR_DQS_0,
		VF610_PAD_DDR_RAS__DDR_RAS_B,
		VF610_PAD_DDR_WE__DDR_WE_B,
		VF610_PAD_DDR_ODT1__DDR_ODT_0,
		VF610_PAD_DDR_ODT0__DDR_ODT_1,
		VF610_PAD_DDR_DDRBYTE1__DDR_DDRBYTE1,
		VF610_PAD_DDR_DDRBYTE2__DDR_DDRBYTE2,
		VF610_PAD_DDR_RESETB,
	};

	if ((pads == NULL) || (pads_count == 0)) {
		pads = default_pads;
		pads_count = ARRAY_SIZE(default_pads);
	}

	imx_iomux_v3_setup_multiple_pads(pads, pads_count);
}

static struct ddrmc_phy_setting default_phy_settings[] = {
	{ DDRMC_PHY_DQ_TIMING,  0 },
	{ DDRMC_PHY_DQ_TIMING, 16 },
	{ DDRMC_PHY_DQ_TIMING, 32 },

	{ DDRMC_PHY_DQS_TIMING,  1 },
	{ DDRMC_PHY_DQS_TIMING, 17 },

	{ DDRMC_PHY_CTRL,  2 },
	{ DDRMC_PHY_CTRL, 18 },
	{ DDRMC_PHY_CTRL, 34 },

	{ DDRMC_PHY_MASTER_CTRL,  3 },
	{ DDRMC_PHY_MASTER_CTRL, 19 },
	{ DDRMC_PHY_MASTER_CTRL, 35 },

	{ DDRMC_PHY_SLAVE_CTRL,  4 },
	{ DDRMC_PHY_SLAVE_CTRL, 20 },
	{ DDRMC_PHY_SLAVE_CTRL, 36 },

	/* LPDDR2 only parameter */
	{ DDRMC_PHY_OFF, 49 },

	{ DDRMC_PHY50_DDR3_MODE | DDRMC_PHY50_EN_SW_HALF_CYCLE, 50 },

	/* Processor Pad ODT settings */
	{ DDRMC_PHY_PROC_PAD_ODT, 52 },

	/* end marker */
	{ 0, -1 }
};

void ddrmc_ctrl_init_ddr3(struct ddr3_jedec_timings const *timings,
			  struct ddrmc_cr_setting *board_cr_settings,
			  struct ddrmc_phy_setting *board_phy_settings,
			  int col_diff, int row_diff)
{
	struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;
	struct ddrmc_cr_setting *cr_setting;
	struct ddrmc_phy_setting *phy_setting;

	writel(DDRMC_CR00_DRAM_CLASS_DDR3, &ddrmr->cr[0]);
	writel(DDRMC_CR02_DRAM_TINIT(timings->tinit), &ddrmr->cr[2]);
	writel(DDRMC_CR10_TRST_PWRON(timings->trst_pwron), &ddrmr->cr[10]);

	writel(DDRMC_CR11_CKE_INACTIVE(timings->cke_inactive), &ddrmr->cr[11]);
	writel(DDRMC_CR12_WRLAT(timings->wrlat) |
		   DDRMC_CR12_CASLAT_LIN(timings->caslat_lin), &ddrmr->cr[12]);
	writel(DDRMC_CR13_TRC(timings->trc) | DDRMC_CR13_TRRD(timings->trrd) |
		   DDRMC_CR13_TCCD(timings->tccd) |
		   DDRMC_CR13_TBST_INT_INTERVAL(timings->tbst_int_interval),
		   &ddrmr->cr[13]);
	writel(DDRMC_CR14_TFAW(timings->tfaw) | DDRMC_CR14_TRP(timings->trp) |
		   DDRMC_CR14_TWTR(timings->twtr) |
		   DDRMC_CR14_TRAS_MIN(timings->tras_min), &ddrmr->cr[14]);
	writel(DDRMC_CR16_TMRD(timings->tmrd) |
		   DDRMC_CR16_TRTP(timings->trtp), &ddrmr->cr[16]);
	writel(DDRMC_CR17_TRAS_MAX(timings->tras_max) |
		   DDRMC_CR17_TMOD(timings->tmod), &ddrmr->cr[17]);
	writel(DDRMC_CR18_TCKESR(timings->tckesr) |
		   DDRMC_CR18_TCKE(timings->tcke), &ddrmr->cr[18]);

	writel(DDRMC_CR20_AP_EN, &ddrmr->cr[20]);
	writel(DDRMC_CR21_TRCD_INT(timings->trcd_int) | DDRMC_CR21_CCMAP_EN |
		   DDRMC_CR21_TRAS_LOCKOUT(timings->tras_lockout),
		   &ddrmr->cr[21]);

	writel(DDRMC_CR22_TDAL(timings->tdal), &ddrmr->cr[22]);
	writel(DDRMC_CR23_BSTLEN(timings->bstlen) |
		   DDRMC_CR23_TDLL(timings->tdll), &ddrmr->cr[23]);
	writel(DDRMC_CR24_TRP_AB(timings->trp_ab), &ddrmr->cr[24]);

	writel(DDRMC_CR25_TREF_EN, &ddrmr->cr[25]);
	writel(DDRMC_CR26_TREF(timings->tref) |
		   DDRMC_CR26_TRFC(timings->trfc), &ddrmr->cr[26]);
	writel(DDRMC_CR28_TREF_INT(timings->tref_int), &ddrmr->cr[28]);
	writel(DDRMC_CR29_TPDEX(timings->tpdex), &ddrmr->cr[29]);

	writel(DDRMC_CR30_TXPDLL(timings->txpdll), &ddrmr->cr[30]);
	writel(DDRMC_CR31_TXSNR(timings->txsnr) |
		   DDRMC_CR31_TXSR(timings->txsr), &ddrmr->cr[31]);
	writel(DDRMC_CR33_EN_QK_SREF, &ddrmr->cr[33]);
	writel(DDRMC_CR34_CKSRX(timings->cksrx) |
		   DDRMC_CR34_CKSRE(timings->cksre), &ddrmr->cr[34]);

	writel(DDRMC_CR38_FREQ_CHG_EN(timings->freq_chg_en), &ddrmr->cr[38]);
	writel(DDRMC_CR39_PHY_INI_COM(1024) | DDRMC_CR39_PHY_INI_STA(16) |
		   DDRMC_CR39_FRQ_CH_DLLOFF(2), &ddrmr->cr[39]);

	writel(DDRMC_CR41_PHY_INI_STRT_INI_DIS, &ddrmr->cr[41]);
	writel(DDRMC_CR48_MR1_DA_0(70) |
		   DDRMC_CR48_MR0_DA_0(1056), &ddrmr->cr[48]);

	writel(DDRMC_CR66_ZQCL(timings->zqcl) |
		   DDRMC_CR66_ZQINIT(timings->zqinit), &ddrmr->cr[66]);
	writel(DDRMC_CR67_ZQCS(timings->zqcs), &ddrmr->cr[67]);
	writel(DDRMC_CR69_ZQ_ON_SREF_EX(2), &ddrmr->cr[69]);

	writel(DDRMC_CR70_REF_PER_ZQ(timings->ref_per_zq), &ddrmr->cr[70]);
	writel(DDRMC_CR72_ZQCS_ROTATE(timings->zqcs_rotate), &ddrmr->cr[72]);

	writel(DDRMC_CR73_APREBIT(timings->aprebit) |
		   DDRMC_CR73_COL_DIFF(col_diff) |
		   DDRMC_CR73_ROW_DIFF(row_diff), &ddrmr->cr[73]);
	writel(DDRMC_CR74_BANKSPLT_EN | DDRMC_CR74_ADDR_CMP_EN |
		   DDRMC_CR74_CMD_AGE_CNT(timings->cmd_age_cnt) |
		   DDRMC_CR74_AGE_CNT(timings->age_cnt),
		   &ddrmr->cr[74]);
	writel(DDRMC_CR75_RW_PG_EN | DDRMC_CR75_RW_EN | DDRMC_CR75_PRI_EN |
		   DDRMC_CR75_PLEN, &ddrmr->cr[75]);
	writel(DDRMC_CR76_NQENT_ACTDIS(3) | DDRMC_CR76_D_RW_G_BKCN(3) |
		   DDRMC_CR76_W2R_SPLT_EN, &ddrmr->cr[76]);
	writel(DDRMC_CR77_CS_MAP | DDRMC_CR77_DI_RD_INTLEAVE |
		   DDRMC_CR77_SWAP_EN, &ddrmr->cr[77]);
	writel(DDRMC_CR78_Q_FULLNESS(timings->q_fullness) |
		   DDRMC_CR78_BUR_ON_FLY_BIT(12), &ddrmr->cr[78]);

	writel(DDRMC_CR82_INT_MASK, &ddrmr->cr[82]);

	writel(DDRMC_CR87_ODT_RD_MAPCS0(timings->odt_rd_mapcs0) |
		   DDRMC_CR87_ODT_WR_MAPCS0(timings->odt_wr_mapcs0),
		   &ddrmr->cr[87]);
	writel(DDRMC_CR88_TODTL_CMD(4), &ddrmr->cr[88]);
	writel(DDRMC_CR89_AODT_RWSMCS(2), &ddrmr->cr[89]);

	writel(DDRMC_CR91_R2W_SMCSDL(2), &ddrmr->cr[91]);
	writel(DDRMC_CR96_WLMRD(timings->wlmrd) |
		   DDRMC_CR96_WLDQSEN(timings->wldqsen), &ddrmr->cr[96]);

	/* execute custom CR setting sequence (may be NULL) */
	cr_setting = board_cr_settings;
	if (cr_setting != NULL)
		while (cr_setting->cr_rnum >= 0) {
			writel(cr_setting->setting,
			       &ddrmr->cr[cr_setting->cr_rnum]);
			cr_setting++;
		}

	/* perform default PHY settings (may be overridden by custom settings */
	phy_setting = default_phy_settings;
	while (phy_setting->phy_rnum >= 0) {
		writel(phy_setting->setting,
		       &ddrmr->phy[phy_setting->phy_rnum]);
		phy_setting++;
	}

	/* execute custom PHY setting sequence (may be NULL) */
	phy_setting = board_phy_settings;
	if (phy_setting != NULL)
		while (phy_setting->phy_rnum >= 0) {
			writel(phy_setting->setting,
			       &ddrmr->phy[phy_setting->phy_rnum]);
			phy_setting++;
		}

	/* all inits done, start the DDR controller */
	writel(DDRMC_CR00_DRAM_CLASS_DDR3 | DDRMC_CR00_START, &ddrmr->cr[0]);

	while (!(readl(&ddrmr->cr[80]) & DDRMC_CR80_MC_INIT_COMPLETE))
		udelay(10);
	writel(DDRMC_CR80_MC_INIT_COMPLETE, &ddrmr->cr[81]);

#ifdef CONFIG_DDRMC_VF610_CALIBRATION
	ddrmc_calibration(ddrmr);
#endif
}
