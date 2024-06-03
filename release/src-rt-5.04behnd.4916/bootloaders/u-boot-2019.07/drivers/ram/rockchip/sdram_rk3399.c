// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * (C) Copyright 2016-2017 Rockchip Inc.
 *
 * Adapted from coreboot.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dt-structs.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/sdram_common.h>
#include <asm/arch-rockchip/sdram_rk3399.h>
#include <asm/arch-rockchip/cru_rk3399.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/err.h>
#include <time.h>

struct chan_info {
	struct rk3399_ddr_pctl_regs *pctl;
	struct rk3399_ddr_pi_regs *pi;
	struct rk3399_ddr_publ_regs *publ;
	struct rk3399_msch_regs *msch;
};

struct dram_info {
#if defined(CONFIG_TPL_BUILD) || \
	(!defined(CONFIG_TPL) && defined(CONFIG_SPL_BUILD))
	struct chan_info chan[2];
	struct clk ddr_clk;
	struct rk3399_cru *cru;
	struct rk3399_pmucru *pmucru;
	struct rk3399_pmusgrf_regs *pmusgrf;
	struct rk3399_ddr_cic_regs *cic;
#endif
	struct ram_info info;
	struct rk3399_pmugrf_regs *pmugrf;
};

#define PRESET_SGRF_HOLD(n)	((0x1 << (6 + 16)) | ((n) << 6))
#define PRESET_GPIO0_HOLD(n)	((0x1 << (7 + 16)) | ((n) << 7))
#define PRESET_GPIO1_HOLD(n)	((0x1 << (8 + 16)) | ((n) << 8))

#define PHY_DRV_ODT_Hi_Z	0x0
#define PHY_DRV_ODT_240		0x1
#define PHY_DRV_ODT_120		0x8
#define PHY_DRV_ODT_80		0x9
#define PHY_DRV_ODT_60		0xc
#define PHY_DRV_ODT_48		0xd
#define PHY_DRV_ODT_40		0xe
#define PHY_DRV_ODT_34_3	0xf

#if defined(CONFIG_TPL_BUILD) || \
	(!defined(CONFIG_TPL) && defined(CONFIG_SPL_BUILD))

struct rockchip_dmc_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3399_dmc dtplat;
#else
	struct rk3399_sdram_params sdram_params;
#endif
	struct regmap *map;
};

static void copy_to_reg(u32 *dest, const u32 *src, u32 n)
{
	int i;

	for (i = 0; i < n / sizeof(u32); i++) {
		writel(*src, dest);
		src++;
		dest++;
	}
}

static void phy_dll_bypass_set(struct rk3399_ddr_publ_regs *ddr_publ_regs,
			       u32 freq)
{
	u32 *denali_phy = ddr_publ_regs->denali_phy;

	/* From IP spec, only freq small than 125 can enter dll bypass mode */
	if (freq <= 125) {
		/* phy_sw_master_mode_X PHY_86/214/342/470 4bits offset_8 */
		setbits_le32(&denali_phy[86], (0x3 << 2) << 8);
		setbits_le32(&denali_phy[214], (0x3 << 2) << 8);
		setbits_le32(&denali_phy[342], (0x3 << 2) << 8);
		setbits_le32(&denali_phy[470], (0x3 << 2) << 8);

		/* phy_adrctl_sw_master_mode PHY_547/675/803 4bits offset_16 */
		setbits_le32(&denali_phy[547], (0x3 << 2) << 16);
		setbits_le32(&denali_phy[675], (0x3 << 2) << 16);
		setbits_le32(&denali_phy[803], (0x3 << 2) << 16);
	} else {
		/* phy_sw_master_mode_X PHY_86/214/342/470 4bits offset_8 */
		clrbits_le32(&denali_phy[86], (0x3 << 2) << 8);
		clrbits_le32(&denali_phy[214], (0x3 << 2) << 8);
		clrbits_le32(&denali_phy[342], (0x3 << 2) << 8);
		clrbits_le32(&denali_phy[470], (0x3 << 2) << 8);

		/* phy_adrctl_sw_master_mode PHY_547/675/803 4bits offset_16 */
		clrbits_le32(&denali_phy[547], (0x3 << 2) << 16);
		clrbits_le32(&denali_phy[675], (0x3 << 2) << 16);
		clrbits_le32(&denali_phy[803], (0x3 << 2) << 16);
	}
}

static void set_memory_map(const struct chan_info *chan, u32 channel,
			   const struct rk3399_sdram_params *sdram_params)
{
	const struct rk3399_sdram_channel *sdram_ch =
		&sdram_params->ch[channel];
	u32 *denali_ctl = chan->pctl->denali_ctl;
	u32 *denali_pi = chan->pi->denali_pi;
	u32 cs_map;
	u32 reduc;
	u32 row;

	/* Get row number from ddrconfig setting */
	if (sdram_ch->ddrconfig < 2 || sdram_ch->ddrconfig == 4)
		row = 16;
	else if (sdram_ch->ddrconfig == 3)
		row = 14;
	else
		row = 15;

	cs_map = (sdram_ch->rank > 1) ? 3 : 1;
	reduc = (sdram_ch->bw == 2) ? 0 : 1;

	/* Set the dram configuration to ctrl */
	clrsetbits_le32(&denali_ctl[191], 0xF, (12 - sdram_ch->col));
	clrsetbits_le32(&denali_ctl[190], (0x3 << 16) | (0x7 << 24),
			((3 - sdram_ch->bk) << 16) |
			((16 - row) << 24));

	clrsetbits_le32(&denali_ctl[196], 0x3 | (1 << 16),
			cs_map | (reduc << 16));

	/* PI_199 PI_COL_DIFF:RW:0:4 */
	clrsetbits_le32(&denali_pi[199], 0xF, (12 - sdram_ch->col));

	/* PI_155 PI_ROW_DIFF:RW:24:3 PI_BANK_DIFF:RW:16:2 */
	clrsetbits_le32(&denali_pi[155], (0x3 << 16) | (0x7 << 24),
			((3 - sdram_ch->bk) << 16) |
			((16 - row) << 24));
	/* PI_41 PI_CS_MAP:RW:24:4 */
	clrsetbits_le32(&denali_pi[41], 0xf << 24, cs_map << 24);
	if ((sdram_ch->rank == 1) && (sdram_params->base.dramtype == DDR3))
		writel(0x2EC7FFFF, &denali_pi[34]);
}

static void set_ds_odt(const struct chan_info *chan,
		       const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_phy = chan->publ->denali_phy;

	u32 tsel_idle_en, tsel_wr_en, tsel_rd_en;
	u32 tsel_idle_select_p, tsel_wr_select_p, tsel_rd_select_p;
	u32 ca_tsel_wr_select_p, ca_tsel_wr_select_n;
	u32 tsel_idle_select_n, tsel_wr_select_n, tsel_rd_select_n;
	u32 reg_value;

	if (sdram_params->base.dramtype == LPDDR4) {
		tsel_rd_select_p = PHY_DRV_ODT_Hi_Z;
		tsel_wr_select_p = PHY_DRV_ODT_40;
		ca_tsel_wr_select_p = PHY_DRV_ODT_40;
		tsel_idle_select_p = PHY_DRV_ODT_Hi_Z;

		tsel_rd_select_n = PHY_DRV_ODT_240;
		tsel_wr_select_n = PHY_DRV_ODT_40;
		ca_tsel_wr_select_n = PHY_DRV_ODT_40;
		tsel_idle_select_n = PHY_DRV_ODT_240;
	} else if (sdram_params->base.dramtype == LPDDR3) {
		tsel_rd_select_p = PHY_DRV_ODT_240;
		tsel_wr_select_p = PHY_DRV_ODT_34_3;
		ca_tsel_wr_select_p = PHY_DRV_ODT_48;
		tsel_idle_select_p = PHY_DRV_ODT_240;

		tsel_rd_select_n = PHY_DRV_ODT_Hi_Z;
		tsel_wr_select_n = PHY_DRV_ODT_34_3;
		ca_tsel_wr_select_n = PHY_DRV_ODT_48;
		tsel_idle_select_n = PHY_DRV_ODT_Hi_Z;
	} else {
		tsel_rd_select_p = PHY_DRV_ODT_240;
		tsel_wr_select_p = PHY_DRV_ODT_34_3;
		ca_tsel_wr_select_p = PHY_DRV_ODT_34_3;
		tsel_idle_select_p = PHY_DRV_ODT_240;

		tsel_rd_select_n = PHY_DRV_ODT_240;
		tsel_wr_select_n = PHY_DRV_ODT_34_3;
		ca_tsel_wr_select_n = PHY_DRV_ODT_34_3;
		tsel_idle_select_n = PHY_DRV_ODT_240;
	}

	if (sdram_params->base.odt == 1)
		tsel_rd_en = 1;
	else
		tsel_rd_en = 0;

	tsel_wr_en = 0;
	tsel_idle_en = 0;

	/*
	 * phy_dq_tsel_select_X 24bits DENALI_PHY_6/134/262/390 offset_0
	 * sets termination values for read/idle cycles and drive strength
	 * for write cycles for DQ/DM
	 */
	reg_value = tsel_rd_select_n | (tsel_rd_select_p << 0x4) |
		    (tsel_wr_select_n << 8) | (tsel_wr_select_p << 12) |
		    (tsel_idle_select_n << 16) | (tsel_idle_select_p << 20);
	clrsetbits_le32(&denali_phy[6], 0xffffff, reg_value);
	clrsetbits_le32(&denali_phy[134], 0xffffff, reg_value);
	clrsetbits_le32(&denali_phy[262], 0xffffff, reg_value);
	clrsetbits_le32(&denali_phy[390], 0xffffff, reg_value);

	/*
	 * phy_dqs_tsel_select_X 24bits DENALI_PHY_7/135/263/391 offset_0
	 * sets termination values for read/idle cycles and drive strength
	 * for write cycles for DQS
	 */
	clrsetbits_le32(&denali_phy[7], 0xffffff, reg_value);
	clrsetbits_le32(&denali_phy[135], 0xffffff, reg_value);
	clrsetbits_le32(&denali_phy[263], 0xffffff, reg_value);
	clrsetbits_le32(&denali_phy[391], 0xffffff, reg_value);

	/* phy_adr_tsel_select_ 8bits DENALI_PHY_544/672/800 offset_0 */
	reg_value = ca_tsel_wr_select_n | (ca_tsel_wr_select_p << 0x4);
	clrsetbits_le32(&denali_phy[544], 0xff, reg_value);
	clrsetbits_le32(&denali_phy[672], 0xff, reg_value);
	clrsetbits_le32(&denali_phy[800], 0xff, reg_value);

	/* phy_pad_addr_drive 8bits DENALI_PHY_928 offset_0 */
	clrsetbits_le32(&denali_phy[928], 0xff, reg_value);

	/* phy_pad_rst_drive 8bits DENALI_PHY_937 offset_0 */
	clrsetbits_le32(&denali_phy[937], 0xff, reg_value);

	/* phy_pad_cke_drive 8bits DENALI_PHY_935 offset_0 */
	clrsetbits_le32(&denali_phy[935], 0xff, reg_value);

	/* phy_pad_cs_drive 8bits DENALI_PHY_939 offset_0 */
	clrsetbits_le32(&denali_phy[939], 0xff, reg_value);

	/* phy_pad_clk_drive 8bits DENALI_PHY_929 offset_0 */
	clrsetbits_le32(&denali_phy[929], 0xff, reg_value);

	/* phy_pad_fdbk_drive 23bit DENALI_PHY_924/925 */
	clrsetbits_le32(&denali_phy[924], 0xff,
			tsel_wr_select_n | (tsel_wr_select_p << 4));
	clrsetbits_le32(&denali_phy[925], 0xff,
			tsel_rd_select_n | (tsel_rd_select_p << 4));

	/* phy_dq_tsel_enable_X 3bits DENALI_PHY_5/133/261/389 offset_16 */
	reg_value = (tsel_rd_en | (tsel_wr_en << 1) | (tsel_idle_en << 2))
		<< 16;
	clrsetbits_le32(&denali_phy[5], 0x7 << 16, reg_value);
	clrsetbits_le32(&denali_phy[133], 0x7 << 16, reg_value);
	clrsetbits_le32(&denali_phy[261], 0x7 << 16, reg_value);
	clrsetbits_le32(&denali_phy[389], 0x7 << 16, reg_value);

	/* phy_dqs_tsel_enable_X 3bits DENALI_PHY_6/134/262/390 offset_24 */
	reg_value = (tsel_rd_en | (tsel_wr_en << 1) | (tsel_idle_en << 2))
		<< 24;
	clrsetbits_le32(&denali_phy[6], 0x7 << 24, reg_value);
	clrsetbits_le32(&denali_phy[134], 0x7 << 24, reg_value);
	clrsetbits_le32(&denali_phy[262], 0x7 << 24, reg_value);
	clrsetbits_le32(&denali_phy[390], 0x7 << 24, reg_value);

	/* phy_adr_tsel_enable_ 1bit DENALI_PHY_518/646/774 offset_8 */
	reg_value = tsel_wr_en << 8;
	clrsetbits_le32(&denali_phy[518], 0x1 << 8, reg_value);
	clrsetbits_le32(&denali_phy[646], 0x1 << 8, reg_value);
	clrsetbits_le32(&denali_phy[774], 0x1 << 8, reg_value);

	/* phy_pad_addr_term tsel 1bit DENALI_PHY_933 offset_17 */
	reg_value = tsel_wr_en << 17;
	clrsetbits_le32(&denali_phy[933], 0x1 << 17, reg_value);
	/*
	 * pad_rst/cke/cs/clk_term tsel 1bits
	 * DENALI_PHY_938/936/940/934 offset_17
	 */
	clrsetbits_le32(&denali_phy[938], 0x1 << 17, reg_value);
	clrsetbits_le32(&denali_phy[936], 0x1 << 17, reg_value);
	clrsetbits_le32(&denali_phy[940], 0x1 << 17, reg_value);
	clrsetbits_le32(&denali_phy[934], 0x1 << 17, reg_value);

	/* phy_pad_fdbk_term 1bit DENALI_PHY_930 offset_17 */
	clrsetbits_le32(&denali_phy[930], 0x1 << 17, reg_value);
}

static int phy_io_config(const struct chan_info *chan,
			  const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_phy = chan->publ->denali_phy;
	u32 vref_mode_dq, vref_value_dq, vref_mode_ac, vref_value_ac;
	u32 mode_sel;
	u32 reg_value;
	u32 drv_value, odt_value;
	u32 speed;

	/* vref setting */
	if (sdram_params->base.dramtype == LPDDR4) {
		/* LPDDR4 */
		vref_mode_dq = 0x6;
		vref_value_dq = 0x1f;
		vref_mode_ac = 0x6;
		vref_value_ac = 0x1f;
	} else if (sdram_params->base.dramtype == LPDDR3) {
		if (sdram_params->base.odt == 1) {
			vref_mode_dq = 0x5;  /* LPDDR3 ODT */
			drv_value = (readl(&denali_phy[6]) >> 12) & 0xf;
			odt_value = (readl(&denali_phy[6]) >> 4) & 0xf;
			if (drv_value == PHY_DRV_ODT_48) {
				switch (odt_value) {
				case PHY_DRV_ODT_240:
					vref_value_dq = 0x16;
					break;
				case PHY_DRV_ODT_120:
					vref_value_dq = 0x26;
					break;
				case PHY_DRV_ODT_60:
					vref_value_dq = 0x36;
					break;
				default:
					debug("Invalid ODT value.\n");
					return -EINVAL;
				}
			} else if (drv_value == PHY_DRV_ODT_40) {
				switch (odt_value) {
				case PHY_DRV_ODT_240:
					vref_value_dq = 0x19;
					break;
				case PHY_DRV_ODT_120:
					vref_value_dq = 0x23;
					break;
				case PHY_DRV_ODT_60:
					vref_value_dq = 0x31;
					break;
				default:
					debug("Invalid ODT value.\n");
					return -EINVAL;
				}
			} else if (drv_value == PHY_DRV_ODT_34_3) {
				switch (odt_value) {
				case PHY_DRV_ODT_240:
					vref_value_dq = 0x17;
					break;
				case PHY_DRV_ODT_120:
					vref_value_dq = 0x20;
					break;
				case PHY_DRV_ODT_60:
					vref_value_dq = 0x2e;
					break;
				default:
					debug("Invalid ODT value.\n");
					return -EINVAL;
				}
			} else {
				debug("Invalid DRV value.\n");
				return -EINVAL;
			}
		} else {
			vref_mode_dq = 0x2;  /* LPDDR3 */
			vref_value_dq = 0x1f;
		}
		vref_mode_ac = 0x2;
		vref_value_ac = 0x1f;
	} else if (sdram_params->base.dramtype == DDR3) {
		/* DDR3L */
		vref_mode_dq = 0x1;
		vref_value_dq = 0x1f;
		vref_mode_ac = 0x1;
		vref_value_ac = 0x1f;
	} else {
		debug("Unknown DRAM type.\n");
		return -EINVAL;
	}

	reg_value = (vref_mode_dq << 9) | (0x1 << 8) | vref_value_dq;

	/* PHY_913 PHY_PAD_VREF_CTRL_DQ_0 12bits offset_8 */
	clrsetbits_le32(&denali_phy[913], 0xfff << 8, reg_value << 8);
	/* PHY_914 PHY_PAD_VREF_CTRL_DQ_1 12bits offset_0 */
	clrsetbits_le32(&denali_phy[914], 0xfff, reg_value);
	/* PHY_914 PHY_PAD_VREF_CTRL_DQ_2 12bits offset_16 */
	clrsetbits_le32(&denali_phy[914], 0xfff << 16, reg_value << 16);
	/* PHY_915 PHY_PAD_VREF_CTRL_DQ_3 12bits offset_0 */
	clrsetbits_le32(&denali_phy[915], 0xfff, reg_value);

	reg_value = (vref_mode_ac << 9) | (0x1 << 8) | vref_value_ac;

	/* PHY_915 PHY_PAD_VREF_CTRL_AC 12bits offset_16 */
	clrsetbits_le32(&denali_phy[915], 0xfff << 16, reg_value << 16);

	if (sdram_params->base.dramtype == LPDDR4)
		mode_sel = 0x6;
	else if (sdram_params->base.dramtype == LPDDR3)
		mode_sel = 0x0;
	else if (sdram_params->base.dramtype == DDR3)
		mode_sel = 0x1;
	else
		return -EINVAL;

	/* PHY_924 PHY_PAD_FDBK_DRIVE */
	clrsetbits_le32(&denali_phy[924], 0x7 << 15, mode_sel << 15);
	/* PHY_926 PHY_PAD_DATA_DRIVE */
	clrsetbits_le32(&denali_phy[926], 0x7 << 6, mode_sel << 6);
	/* PHY_927 PHY_PAD_DQS_DRIVE */
	clrsetbits_le32(&denali_phy[927], 0x7 << 6, mode_sel << 6);
	/* PHY_928 PHY_PAD_ADDR_DRIVE */
	clrsetbits_le32(&denali_phy[928], 0x7 << 14, mode_sel << 14);
	/* PHY_929 PHY_PAD_CLK_DRIVE */
	clrsetbits_le32(&denali_phy[929], 0x7 << 14, mode_sel << 14);
	/* PHY_935 PHY_PAD_CKE_DRIVE */
	clrsetbits_le32(&denali_phy[935], 0x7 << 14, mode_sel << 14);
	/* PHY_937 PHY_PAD_RST_DRIVE */
	clrsetbits_le32(&denali_phy[937], 0x7 << 14, mode_sel << 14);
	/* PHY_939 PHY_PAD_CS_DRIVE */
	clrsetbits_le32(&denali_phy[939], 0x7 << 14, mode_sel << 14);


	/* speed setting */
	if (sdram_params->base.ddr_freq < 400)
		speed = 0x0;
	else if (sdram_params->base.ddr_freq < 800)
		speed = 0x1;
	else if (sdram_params->base.ddr_freq < 1200)
		speed = 0x2;
	else
		speed = 0x3;

	/* PHY_924 PHY_PAD_FDBK_DRIVE */
	clrsetbits_le32(&denali_phy[924], 0x3 << 21, speed << 21);
	/* PHY_926 PHY_PAD_DATA_DRIVE */
	clrsetbits_le32(&denali_phy[926], 0x3 << 9, speed << 9);
	/* PHY_927 PHY_PAD_DQS_DRIVE */
	clrsetbits_le32(&denali_phy[927], 0x3 << 9, speed << 9);
	/* PHY_928 PHY_PAD_ADDR_DRIVE */
	clrsetbits_le32(&denali_phy[928], 0x3 << 17, speed << 17);
	/* PHY_929 PHY_PAD_CLK_DRIVE */
	clrsetbits_le32(&denali_phy[929], 0x3 << 17, speed << 17);
	/* PHY_935 PHY_PAD_CKE_DRIVE */
	clrsetbits_le32(&denali_phy[935], 0x3 << 17, speed << 17);
	/* PHY_937 PHY_PAD_RST_DRIVE */
	clrsetbits_le32(&denali_phy[937], 0x3 << 17, speed << 17);
	/* PHY_939 PHY_PAD_CS_DRIVE */
	clrsetbits_le32(&denali_phy[939], 0x3 << 17, speed << 17);

	return 0;
}

static int pctl_cfg(const struct chan_info *chan, u32 channel,
		    const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_ctl = chan->pctl->denali_ctl;
	u32 *denali_pi = chan->pi->denali_pi;
	u32 *denali_phy = chan->publ->denali_phy;
	const u32 *params_ctl = sdram_params->pctl_regs.denali_ctl;
	const u32 *params_phy = sdram_params->phy_regs.denali_phy;
	u32 tmp, tmp1, tmp2;
	u32 pwrup_srefresh_exit;
	int ret;
	const ulong timeout_ms = 200;

	/*
	 * work around controller bug:
	 * Do not program DRAM_CLASS until NO_PHY_IND_TRAIN_INT is programmed
	 */
	copy_to_reg(&denali_ctl[1], &params_ctl[1],
		    sizeof(struct rk3399_ddr_pctl_regs) - 4);
	writel(params_ctl[0], &denali_ctl[0]);
	copy_to_reg(denali_pi, &sdram_params->pi_regs.denali_pi[0],
		    sizeof(struct rk3399_ddr_pi_regs));
	/* rank count need to set for init */
	set_memory_map(chan, channel, sdram_params);

	writel(sdram_params->phy_regs.denali_phy[910], &denali_phy[910]);
	writel(sdram_params->phy_regs.denali_phy[911], &denali_phy[911]);
	writel(sdram_params->phy_regs.denali_phy[912], &denali_phy[912]);

	pwrup_srefresh_exit = readl(&denali_ctl[68]) & PWRUP_SREFRESH_EXIT;
	clrbits_le32(&denali_ctl[68], PWRUP_SREFRESH_EXIT);

	/* PHY_DLL_RST_EN */
	clrsetbits_le32(&denali_phy[957], 0x3 << 24, 1 << 24);

	setbits_le32(&denali_pi[0], START);
	setbits_le32(&denali_ctl[0], START);

	/* Wating for phy DLL lock */
	while (1) {
		tmp = readl(&denali_phy[920]);
		tmp1 = readl(&denali_phy[921]);
		tmp2 = readl(&denali_phy[922]);
		if ((((tmp >> 16) & 0x1) == 0x1) &&
		    (((tmp1 >> 16) & 0x1) == 0x1) &&
		    (((tmp1 >> 0) & 0x1) == 0x1) &&
		    (((tmp2 >> 0) & 0x1) == 0x1))
			break;
	}

	copy_to_reg(&denali_phy[896], &params_phy[896], (958 - 895) * 4);
	copy_to_reg(&denali_phy[0], &params_phy[0], (90 - 0 + 1) * 4);
	copy_to_reg(&denali_phy[128], &params_phy[128], (218 - 128 + 1) * 4);
	copy_to_reg(&denali_phy[256], &params_phy[256], (346 - 256 + 1) * 4);
	copy_to_reg(&denali_phy[384], &params_phy[384], (474 - 384 + 1) * 4);
	copy_to_reg(&denali_phy[512], &params_phy[512], (549 - 512 + 1) * 4);
	copy_to_reg(&denali_phy[640], &params_phy[640], (677 - 640 + 1) * 4);
	copy_to_reg(&denali_phy[768], &params_phy[768], (805 - 768 + 1) * 4);
	set_ds_odt(chan, sdram_params);

	/*
	 * phy_dqs_tsel_wr_timing_X 8bits DENALI_PHY_84/212/340/468 offset_8
	 * dqs_tsel_wr_end[7:4] add Half cycle
	 */
	tmp = (readl(&denali_phy[84]) >> 8) & 0xff;
	clrsetbits_le32(&denali_phy[84], 0xff << 8, (tmp + 0x10) << 8);
	tmp = (readl(&denali_phy[212]) >> 8) & 0xff;
	clrsetbits_le32(&denali_phy[212], 0xff << 8, (tmp + 0x10) << 8);
	tmp = (readl(&denali_phy[340]) >> 8) & 0xff;
	clrsetbits_le32(&denali_phy[340], 0xff << 8, (tmp + 0x10) << 8);
	tmp = (readl(&denali_phy[468]) >> 8) & 0xff;
	clrsetbits_le32(&denali_phy[468], 0xff << 8, (tmp + 0x10) << 8);

	/*
	 * phy_dqs_tsel_wr_timing_X 8bits DENALI_PHY_83/211/339/467 offset_8
	 * dq_tsel_wr_end[7:4] add Half cycle
	 */
	tmp = (readl(&denali_phy[83]) >> 16) & 0xff;
	clrsetbits_le32(&denali_phy[83], 0xff << 16, (tmp + 0x10) << 16);
	tmp = (readl(&denali_phy[211]) >> 16) & 0xff;
	clrsetbits_le32(&denali_phy[211], 0xff << 16, (tmp + 0x10) << 16);
	tmp = (readl(&denali_phy[339]) >> 16) & 0xff;
	clrsetbits_le32(&denali_phy[339], 0xff << 16, (tmp + 0x10) << 16);
	tmp = (readl(&denali_phy[467]) >> 16) & 0xff;
	clrsetbits_le32(&denali_phy[467], 0xff << 16, (tmp + 0x10) << 16);

	ret = phy_io_config(chan, sdram_params);
	if (ret)
		return ret;

	/* PHY_DLL_RST_EN */
	clrsetbits_le32(&denali_phy[957], 0x3 << 24, 0x2 << 24);

	/* Wating for PHY and DRAM init complete */
	tmp = get_timer(0);
	do {
		if (get_timer(tmp) > timeout_ms) {
			pr_err("DRAM (%s): phy failed to lock within  %ld ms\n",
			      __func__, timeout_ms);
			return -ETIME;
		}
	} while (!(readl(&denali_ctl[203]) & (1 << 3)));
	debug("DRAM (%s): phy locked after %ld ms\n", __func__, get_timer(tmp));

	clrsetbits_le32(&denali_ctl[68], PWRUP_SREFRESH_EXIT,
			pwrup_srefresh_exit);
	return 0;
}

static void select_per_cs_training_index(const struct chan_info *chan,
					 u32 rank)
{
	u32 *denali_phy = chan->publ->denali_phy;

	/* PHY_84 PHY_PER_CS_TRAINING_EN_0 1bit offset_16 */
	if ((readl(&denali_phy[84])>>16) & 1) {
		/*
		 * PHY_8/136/264/392
		 * phy_per_cs_training_index_X 1bit offset_24
		 */
		clrsetbits_le32(&denali_phy[8], 0x1 << 24, rank << 24);
		clrsetbits_le32(&denali_phy[136], 0x1 << 24, rank << 24);
		clrsetbits_le32(&denali_phy[264], 0x1 << 24, rank << 24);
		clrsetbits_le32(&denali_phy[392], 0x1 << 24, rank << 24);
	}
}

static void override_write_leveling_value(const struct chan_info *chan)
{
	u32 *denali_ctl = chan->pctl->denali_ctl;
	u32 *denali_phy = chan->publ->denali_phy;
	u32 byte;

	/* PHY_896 PHY_FREQ_SEL_MULTICAST_EN 1bit offset_0 */
	setbits_le32(&denali_phy[896], 1);

	/*
	 * PHY_8/136/264/392
	 * phy_per_cs_training_multicast_en_X 1bit offset_16
	 */
	clrsetbits_le32(&denali_phy[8], 0x1 << 16, 1 << 16);
	clrsetbits_le32(&denali_phy[136], 0x1 << 16, 1 << 16);
	clrsetbits_le32(&denali_phy[264], 0x1 << 16, 1 << 16);
	clrsetbits_le32(&denali_phy[392], 0x1 << 16, 1 << 16);

	for (byte = 0; byte < 4; byte++)
		clrsetbits_le32(&denali_phy[63 + (128 * byte)], 0xffff << 16,
				0x200 << 16);

	/* PHY_896 PHY_FREQ_SEL_MULTICAST_EN 1bit offset_0 */
	clrbits_le32(&denali_phy[896], 1);

	/* CTL_200 ctrlupd_req 1bit offset_8 */
	clrsetbits_le32(&denali_ctl[200], 0x1 << 8, 0x1 << 8);
}

static int data_training_ca(const struct chan_info *chan, u32 channel,
			    const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_pi = chan->pi->denali_pi;
	u32 *denali_phy = chan->publ->denali_phy;
	u32 i, tmp;
	u32 obs_0, obs_1, obs_2, obs_err = 0;
	u32 rank = sdram_params->ch[channel].rank;

	for (i = 0; i < rank; i++) {
		select_per_cs_training_index(chan, i);
		/* PI_100 PI_CALVL_EN:RW:8:2 */
		clrsetbits_le32(&denali_pi[100], 0x3 << 8, 0x2 << 8);
		/* PI_92 PI_CALVL_REQ:WR:16:1,PI_CALVL_CS:RW:24:2 */
		clrsetbits_le32(&denali_pi[92],
				(0x1 << 16) | (0x3 << 24),
				(0x1 << 16) | (i << 24));

		/* Waiting for training complete */
		while (1) {
			/* PI_174 PI_INT_STATUS:RD:8:18 */
			tmp = readl(&denali_pi[174]) >> 8;
			/*
			 * check status obs
			 * PHY_532/660/789 phy_adr_calvl_obs1_:0:32
			 */
			obs_0 = readl(&denali_phy[532]);
			obs_1 = readl(&denali_phy[660]);
			obs_2 = readl(&denali_phy[788]);
			if (((obs_0 >> 30) & 0x3) ||
			    ((obs_1 >> 30) & 0x3) ||
			    ((obs_2 >> 30) & 0x3))
				obs_err = 1;
			if ((((tmp >> 11) & 0x1) == 0x1) &&
			    (((tmp >> 13) & 0x1) == 0x1) &&
			    (((tmp >> 5) & 0x1) == 0x0) &&
			    (obs_err == 0))
				break;
			else if ((((tmp >> 5) & 0x1) == 0x1) ||
				 (obs_err == 1))
				return -EIO;
		}
		/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
		writel(0x00003f7c, (&denali_pi[175]));
	}
	clrbits_le32(&denali_pi[100], 0x3 << 8);

	return 0;
}

static int data_training_wl(const struct chan_info *chan, u32 channel,
			    const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_pi = chan->pi->denali_pi;
	u32 *denali_phy = chan->publ->denali_phy;
	u32 i, tmp;
	u32 obs_0, obs_1, obs_2, obs_3, obs_err = 0;
	u32 rank = sdram_params->ch[channel].rank;

	for (i = 0; i < rank; i++) {
		select_per_cs_training_index(chan, i);
		/* PI_60 PI_WRLVL_EN:RW:8:2 */
		clrsetbits_le32(&denali_pi[60], 0x3 << 8, 0x2 << 8);
		/* PI_59 PI_WRLVL_REQ:WR:8:1,PI_WRLVL_CS:RW:16:2 */
		clrsetbits_le32(&denali_pi[59],
				(0x1 << 8) | (0x3 << 16),
				(0x1 << 8) | (i << 16));

		/* Waiting for training complete */
		while (1) {
			/* PI_174 PI_INT_STATUS:RD:8:18 */
			tmp = readl(&denali_pi[174]) >> 8;

			/*
			 * check status obs, if error maybe can not
			 * get leveling done PHY_40/168/296/424
			 * phy_wrlvl_status_obs_X:0:13
			 */
			obs_0 = readl(&denali_phy[40]);
			obs_1 = readl(&denali_phy[168]);
			obs_2 = readl(&denali_phy[296]);
			obs_3 = readl(&denali_phy[424]);
			if (((obs_0 >> 12) & 0x1) ||
			    ((obs_1 >> 12) & 0x1) ||
			    ((obs_2 >> 12) & 0x1) ||
			    ((obs_3 >> 12) & 0x1))
				obs_err = 1;
			if ((((tmp >> 10) & 0x1) == 0x1) &&
			    (((tmp >> 13) & 0x1) == 0x1) &&
			    (((tmp >> 4) & 0x1) == 0x0) &&
			    (obs_err == 0))
				break;
			else if ((((tmp >> 4) & 0x1) == 0x1) ||
				 (obs_err == 1))
				return -EIO;
		}
		/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
		writel(0x00003f7c, (&denali_pi[175]));
	}

	override_write_leveling_value(chan);
	clrbits_le32(&denali_pi[60], 0x3 << 8);

	return 0;
}

static int data_training_rg(const struct chan_info *chan, u32 channel,
			    const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_pi = chan->pi->denali_pi;
	u32 *denali_phy = chan->publ->denali_phy;
	u32 i, tmp;
	u32 obs_0, obs_1, obs_2, obs_3, obs_err = 0;
	u32 rank = sdram_params->ch[channel].rank;

	for (i = 0; i < rank; i++) {
		select_per_cs_training_index(chan, i);
		/* PI_80 PI_RDLVL_GATE_EN:RW:24:2 */
		clrsetbits_le32(&denali_pi[80], 0x3 << 24, 0x2 << 24);
		/*
		 * PI_74 PI_RDLVL_GATE_REQ:WR:16:1
		 * PI_RDLVL_CS:RW:24:2
		 */
		clrsetbits_le32(&denali_pi[74],
				(0x1 << 16) | (0x3 << 24),
				(0x1 << 16) | (i << 24));

		/* Waiting for training complete */
		while (1) {
			/* PI_174 PI_INT_STATUS:RD:8:18 */
			tmp = readl(&denali_pi[174]) >> 8;

			/*
			 * check status obs
			 * PHY_43/171/299/427
			 *     PHY_GTLVL_STATUS_OBS_x:16:8
			 */
			obs_0 = readl(&denali_phy[43]);
			obs_1 = readl(&denali_phy[171]);
			obs_2 = readl(&denali_phy[299]);
			obs_3 = readl(&denali_phy[427]);
			if (((obs_0 >> (16 + 6)) & 0x3) ||
			    ((obs_1 >> (16 + 6)) & 0x3) ||
			    ((obs_2 >> (16 + 6)) & 0x3) ||
			    ((obs_3 >> (16 + 6)) & 0x3))
				obs_err = 1;
			if ((((tmp >> 9) & 0x1) == 0x1) &&
			    (((tmp >> 13) & 0x1) == 0x1) &&
			    (((tmp >> 3) & 0x1) == 0x0) &&
			    (obs_err == 0))
				break;
			else if ((((tmp >> 3) & 0x1) == 0x1) ||
				 (obs_err == 1))
				return -EIO;
		}
		/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
		writel(0x00003f7c, (&denali_pi[175]));
	}
	clrbits_le32(&denali_pi[80], 0x3 << 24);

	return 0;
}

static int data_training_rl(const struct chan_info *chan, u32 channel,
			    const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_pi = chan->pi->denali_pi;
	u32 i, tmp;
	u32 rank = sdram_params->ch[channel].rank;

	for (i = 0; i < rank; i++) {
		select_per_cs_training_index(chan, i);
		/* PI_80 PI_RDLVL_EN:RW:16:2 */
		clrsetbits_le32(&denali_pi[80], 0x3 << 16, 0x2 << 16);
		/* PI_74 PI_RDLVL_REQ:WR:8:1,PI_RDLVL_CS:RW:24:2 */
		clrsetbits_le32(&denali_pi[74],
				(0x1 << 8) | (0x3 << 24),
				(0x1 << 8) | (i << 24));

		/* Waiting for training complete */
		while (1) {
			/* PI_174 PI_INT_STATUS:RD:8:18 */
			tmp = readl(&denali_pi[174]) >> 8;

			/*
			 * make sure status obs not report error bit
			 * PHY_46/174/302/430
			 *     phy_rdlvl_status_obs_X:16:8
			 */
			if ((((tmp >> 8) & 0x1) == 0x1) &&
			    (((tmp >> 13) & 0x1) == 0x1) &&
			    (((tmp >> 2) & 0x1) == 0x0))
				break;
			else if (((tmp >> 2) & 0x1) == 0x1)
				return -EIO;
		}
		/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
		writel(0x00003f7c, (&denali_pi[175]));
	}
	clrbits_le32(&denali_pi[80], 0x3 << 16);

	return 0;
}

static int data_training_wdql(const struct chan_info *chan, u32 channel,
			      const struct rk3399_sdram_params *sdram_params)
{
	u32 *denali_pi = chan->pi->denali_pi;
	u32 i, tmp;
	u32 rank = sdram_params->ch[channel].rank;

	for (i = 0; i < rank; i++) {
		select_per_cs_training_index(chan, i);
		/*
		 * disable PI_WDQLVL_VREF_EN before wdq leveling?
		 * PI_181 PI_WDQLVL_VREF_EN:RW:8:1
		 */
		clrbits_le32(&denali_pi[181], 0x1 << 8);
		/* PI_124 PI_WDQLVL_EN:RW:16:2 */
		clrsetbits_le32(&denali_pi[124], 0x3 << 16, 0x2 << 16);
		/* PI_121 PI_WDQLVL_REQ:WR:8:1,PI_WDQLVL_CS:RW:16:2 */
		clrsetbits_le32(&denali_pi[121],
				(0x1 << 8) | (0x3 << 16),
				(0x1 << 8) | (i << 16));

		/* Waiting for training complete */
		while (1) {
			/* PI_174 PI_INT_STATUS:RD:8:18 */
			tmp = readl(&denali_pi[174]) >> 8;
			if ((((tmp >> 12) & 0x1) == 0x1) &&
			    (((tmp >> 13) & 0x1) == 0x1) &&
			    (((tmp >> 6) & 0x1) == 0x0))
				break;
			else if (((tmp >> 6) & 0x1) == 0x1)
				return -EIO;
		}
		/* clear interrupt,PI_175 PI_INT_ACK:WR:0:17 */
		writel(0x00003f7c, (&denali_pi[175]));
	}
	clrbits_le32(&denali_pi[124], 0x3 << 16);

	return 0;
}

static int data_training(const struct chan_info *chan, u32 channel,
			 const struct rk3399_sdram_params *sdram_params,
			 u32 training_flag)
{
	u32 *denali_phy = chan->publ->denali_phy;

	/* PHY_927 PHY_PAD_DQS_DRIVE  RPULL offset_22 */
	setbits_le32(&denali_phy[927], (1 << 22));

	if (training_flag == PI_FULL_TRAINING) {
		if (sdram_params->base.dramtype == LPDDR4) {
			training_flag = PI_CA_TRAINING | PI_WRITE_LEVELING |
					PI_READ_GATE_TRAINING |
					PI_READ_LEVELING | PI_WDQ_LEVELING;
		} else if (sdram_params->base.dramtype == LPDDR3) {
			training_flag = PI_CA_TRAINING | PI_WRITE_LEVELING |
					PI_READ_GATE_TRAINING;
		} else if (sdram_params->base.dramtype == DDR3) {
			training_flag = PI_WRITE_LEVELING |
					PI_READ_GATE_TRAINING |
					PI_READ_LEVELING;
		}
	}

	/* ca training(LPDDR4,LPDDR3 support) */
	if ((training_flag & PI_CA_TRAINING) == PI_CA_TRAINING)
		data_training_ca(chan, channel, sdram_params);

	/* write leveling(LPDDR4,LPDDR3,DDR3 support) */
	if ((training_flag & PI_WRITE_LEVELING) == PI_WRITE_LEVELING)
		data_training_wl(chan, channel, sdram_params);

	/* read gate training(LPDDR4,LPDDR3,DDR3 support) */
	if ((training_flag & PI_READ_GATE_TRAINING) == PI_READ_GATE_TRAINING)
		data_training_rg(chan, channel, sdram_params);

	/* read leveling(LPDDR4,LPDDR3,DDR3 support) */
	if ((training_flag & PI_READ_LEVELING) == PI_READ_LEVELING)
		data_training_rl(chan, channel, sdram_params);

	/* wdq leveling(LPDDR4 support) */
	if ((training_flag & PI_WDQ_LEVELING) == PI_WDQ_LEVELING)
		data_training_wdql(chan, channel, sdram_params);

	/* PHY_927 PHY_PAD_DQS_DRIVE  RPULL offset_22 */
	clrbits_le32(&denali_phy[927], (1 << 22));

	return 0;
}

static void set_ddrconfig(const struct chan_info *chan,
			  const struct rk3399_sdram_params *sdram_params,
			  unsigned char channel, u32 ddrconfig)
{
	/* only need to set ddrconfig */
	struct rk3399_msch_regs *ddr_msch_regs = chan->msch;
	unsigned int cs0_cap = 0;
	unsigned int cs1_cap = 0;

	cs0_cap = (1 << (sdram_params->ch[channel].cs0_row
			+ sdram_params->ch[channel].col
			+ sdram_params->ch[channel].bk
			+ sdram_params->ch[channel].bw - 20));
	if (sdram_params->ch[channel].rank > 1)
		cs1_cap = cs0_cap >> (sdram_params->ch[channel].cs0_row
				- sdram_params->ch[channel].cs1_row);
	if (sdram_params->ch[channel].row_3_4) {
		cs0_cap = cs0_cap * 3 / 4;
		cs1_cap = cs1_cap * 3 / 4;
	}

	writel(ddrconfig | (ddrconfig << 8), &ddr_msch_regs->ddrconf);
	writel(((cs0_cap / 32) & 0xff) | (((cs1_cap / 32) & 0xff) << 8),
	       &ddr_msch_regs->ddrsize);
}

static void dram_all_config(struct dram_info *dram,
			    const struct rk3399_sdram_params *sdram_params)
{
	u32 sys_reg = 0;
	unsigned int channel, idx;

	sys_reg |= sdram_params->base.dramtype << SYS_REG_DDRTYPE_SHIFT;
	sys_reg |= (sdram_params->base.num_channels - 1)
		    << SYS_REG_NUM_CH_SHIFT;
	for (channel = 0, idx = 0;
	     (idx < sdram_params->base.num_channels) && (channel < 2);
	     channel++) {
		const struct rk3399_sdram_channel *info =
			&sdram_params->ch[channel];
		struct rk3399_msch_regs *ddr_msch_regs;
		const struct rk3399_msch_timings *noc_timing;

		if (sdram_params->ch[channel].col == 0)
			continue;
		idx++;
		sys_reg |= info->row_3_4 << SYS_REG_ROW_3_4_SHIFT(channel);
		sys_reg |= 1 << SYS_REG_CHINFO_SHIFT(channel);
		sys_reg |= (info->rank - 1) << SYS_REG_RANK_SHIFT(channel);
		sys_reg |= (info->col - 9) << SYS_REG_COL_SHIFT(channel);
		sys_reg |= info->bk == 3 ? 0 : 1 << SYS_REG_BK_SHIFT(channel);
		sys_reg |= (info->cs0_row - 13) << SYS_REG_CS0_ROW_SHIFT(channel);
		sys_reg |= (info->cs1_row - 13) << SYS_REG_CS1_ROW_SHIFT(channel);
		sys_reg |= (2 >> info->bw) << SYS_REG_BW_SHIFT(channel);
		sys_reg |= (2 >> info->dbw) << SYS_REG_DBW_SHIFT(channel);

		ddr_msch_regs = dram->chan[channel].msch;
		noc_timing = &sdram_params->ch[channel].noc_timings;
		writel(noc_timing->ddrtiminga0,
		       &ddr_msch_regs->ddrtiminga0);
		writel(noc_timing->ddrtimingb0,
		       &ddr_msch_regs->ddrtimingb0);
		writel(noc_timing->ddrtimingc0,
		       &ddr_msch_regs->ddrtimingc0);
		writel(noc_timing->devtodev0,
		       &ddr_msch_regs->devtodev0);
		writel(noc_timing->ddrmode,
		       &ddr_msch_regs->ddrmode);

		/* rank 1 memory clock disable (dfi_dram_clk_disable = 1) */
		if (sdram_params->ch[channel].rank == 1)
			setbits_le32(&dram->chan[channel].pctl->denali_ctl[276],
				     1 << 17);
	}

	writel(sys_reg, &dram->pmugrf->os_reg2);
	rk_clrsetreg(&dram->pmusgrf->soc_con4, 0x1f << 10,
		     sdram_params->base.stride << 10);

	/* reboot hold register set */
	writel(PRESET_SGRF_HOLD(0) | PRESET_GPIO0_HOLD(1) |
		PRESET_GPIO1_HOLD(1),
		&dram->pmucru->pmucru_rstnhold_con[1]);
	clrsetbits_le32(&dram->cru->glb_rst_con, 0x3, 0x3);
}

static int switch_to_phy_index1(struct dram_info *dram,
				 const struct rk3399_sdram_params *sdram_params)
{
	u32 channel;
	u32 *denali_phy;
	u32 ch_count = sdram_params->base.num_channels;
	int ret;
	int i = 0;

	writel(RK_CLRSETBITS(0x03 << 4 | 1 << 2 | 1,
			     1 << 4 | 1 << 2 | 1),
			&dram->cic->cic_ctrl0);
	while (!(readl(&dram->cic->cic_status0) & (1 << 2))) {
		mdelay(10);
		i++;
		if (i > 10) {
			debug("index1 frequency change overtime\n");
			return -ETIME;
		}
	}

	i = 0;
	writel(RK_CLRSETBITS(1 << 1, 1 << 1), &dram->cic->cic_ctrl0);
	while (!(readl(&dram->cic->cic_status0) & (1 << 0))) {
		mdelay(10);
		i++;
		if (i > 10) {
			debug("index1 frequency done overtime\n");
			return -ETIME;
		}
	}

	for (channel = 0; channel < ch_count; channel++) {
		denali_phy = dram->chan[channel].publ->denali_phy;
		clrsetbits_le32(&denali_phy[896], (0x3 << 8) | 1, 1 << 8);
		ret = data_training(&dram->chan[channel], channel,
				  sdram_params, PI_FULL_TRAINING);
		if (ret) {
			debug("index1 training failed\n");
			return ret;
		}
	}

	return 0;
}

static int sdram_init(struct dram_info *dram,
		      const struct rk3399_sdram_params *sdram_params)
{
	unsigned char dramtype = sdram_params->base.dramtype;
	unsigned int ddr_freq = sdram_params->base.ddr_freq;
	int channel;

	debug("Starting SDRAM initialization...\n");

	if ((dramtype == DDR3 && ddr_freq > 933) ||
	    (dramtype == LPDDR3 && ddr_freq > 933) ||
	    (dramtype == LPDDR4 && ddr_freq > 800)) {
		debug("SDRAM frequency is to high!");
		return -E2BIG;
	}

	for (channel = 0; channel < 2; channel++) {
		const struct chan_info *chan = &dram->chan[channel];
		struct rk3399_ddr_publ_regs *publ = chan->publ;

		phy_dll_bypass_set(publ, ddr_freq);

		if (channel >= sdram_params->base.num_channels)
			continue;

		if (pctl_cfg(chan, channel, sdram_params) != 0) {
			printf("pctl_cfg fail, reset\n");
			return -EIO;
		}

		/* LPDDR2/LPDDR3 need to wait DAI complete, max 10us */
		if (dramtype == LPDDR3)
			udelay(10);

		if (data_training(chan, channel,
				  sdram_params, PI_FULL_TRAINING)) {
			printf("SDRAM initialization failed, reset\n");
			return -EIO;
		}

		set_ddrconfig(chan, sdram_params, channel,
			      sdram_params->ch[channel].ddrconfig);
	}
	dram_all_config(dram, sdram_params);
	switch_to_phy_index1(dram, sdram_params);

	debug("Finish SDRAM initialization...\n");
	return 0;
}

static int rk3399_dmc_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_dmc_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = dev_read_u32_array(dev, "rockchip,sdram-params",
				 (u32 *)&plat->sdram_params,
				 sizeof(plat->sdram_params) / sizeof(u32));
	if (ret) {
		printf("%s: Cannot read rockchip,sdram-params %d\n",
		       __func__, ret);
		return ret;
	}
	ret = regmap_init_mem(dev_ofnode(dev), &plat->map);
	if (ret)
		printf("%s: regmap failed %d\n", __func__, ret);

#endif
	return 0;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int conv_of_platdata(struct udevice *dev)
{
	struct rockchip_dmc_plat *plat = dev_get_platdata(dev);
	struct dtd_rockchip_rk3399_dmc *dtplat = &plat->dtplat;
	int ret;

	ret = regmap_init_mem_platdata(dev, dtplat->reg,
			ARRAY_SIZE(dtplat->reg) / 2,
			&plat->map);
	if (ret)
		return ret;

	return 0;
}
#endif

static int rk3399_dmc_init(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct rockchip_dmc_plat *plat = dev_get_platdata(dev);
	int ret;
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3399_sdram_params *params = &plat->sdram_params;
#else
	struct dtd_rockchip_rk3399_dmc *dtplat = &plat->dtplat;
	struct rk3399_sdram_params *params =
					(void *)dtplat->rockchip_sdram_params;

	ret = conv_of_platdata(dev);
	if (ret)
		return ret;
#endif

	priv->cic = syscon_get_first_range(ROCKCHIP_SYSCON_CIC);
	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);
	priv->pmusgrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUSGRF);
	priv->pmucru = rockchip_get_pmucru();
	priv->cru = rockchip_get_cru();
	priv->chan[0].pctl = regmap_get_range(plat->map, 0);
	priv->chan[0].pi = regmap_get_range(plat->map, 1);
	priv->chan[0].publ = regmap_get_range(plat->map, 2);
	priv->chan[0].msch = regmap_get_range(plat->map, 3);
	priv->chan[1].pctl = regmap_get_range(plat->map, 4);
	priv->chan[1].pi = regmap_get_range(plat->map, 5);
	priv->chan[1].publ = regmap_get_range(plat->map, 6);
	priv->chan[1].msch = regmap_get_range(plat->map, 7);

	debug("con reg %p %p %p %p %p %p %p %p\n",
	      priv->chan[0].pctl, priv->chan[0].pi,
	      priv->chan[0].publ, priv->chan[0].msch,
	      priv->chan[1].pctl, priv->chan[1].pi,
	      priv->chan[1].publ, priv->chan[1].msch);
	debug("cru %p, cic %p, grf %p, sgrf %p, pmucru %p\n", priv->cru,
	      priv->cic, priv->pmugrf, priv->pmusgrf, priv->pmucru);
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = clk_get_by_index_platdata(dev, 0, dtplat->clocks, &priv->ddr_clk);
#else
	ret = clk_get_by_index(dev, 0, &priv->ddr_clk);
#endif
	if (ret) {
		printf("%s clk get failed %d\n", __func__, ret);
		return ret;
	}
	ret = clk_set_rate(&priv->ddr_clk, params->base.ddr_freq * MHz);
	if (ret < 0) {
		printf("%s clk set failed %d\n", __func__, ret);
		return ret;
	}
	ret = sdram_init(priv, params);
	if (ret < 0) {
		printf("%s DRAM init failed%d\n", __func__, ret);
		return ret;
	}

	return 0;
}
#endif

static int rk3399_dmc_probe(struct udevice *dev)
{
#if defined(CONFIG_TPL_BUILD) || \
	(!defined(CONFIG_TPL) && defined(CONFIG_SPL_BUILD))
	if (rk3399_dmc_init(dev))
		return 0;
#else
	struct dram_info *priv = dev_get_priv(dev);

	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);
	debug("%s: pmugrf=%p\n", __func__, priv->pmugrf);
	priv->info.base = CONFIG_SYS_SDRAM_BASE;
	priv->info.size = rockchip_sdram_size(
			(phys_addr_t)&priv->pmugrf->os_reg2);
#endif
	return 0;
}

static int rk3399_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3399_dmc_ops = {
	.get_info = rk3399_dmc_get_info,
};


static const struct udevice_id rk3399_dmc_ids[] = {
	{ .compatible = "rockchip,rk3399-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3399) = {
	.name = "rockchip_rk3399_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3399_dmc_ids,
	.ops = &rk3399_dmc_ops,
#if defined(CONFIG_TPL_BUILD) || \
	(!defined(CONFIG_TPL) && defined(CONFIG_SPL_BUILD))
	.ofdata_to_platdata = rk3399_dmc_ofdata_to_platdata,
#endif
	.probe = rk3399_dmc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
#if defined(CONFIG_TPL_BUILD) || \
	(!defined(CONFIG_TPL) && defined(CONFIG_SPL_BUILD))
	.platdata_auto_alloc_size = sizeof(struct rockchip_dmc_plat),
#endif
};
