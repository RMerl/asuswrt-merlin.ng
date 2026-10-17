// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 */

#include <bouncebuf.h>
#include <clk.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <mmc.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/dma-direction.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <power/regulator.h>
#include <reset.h>
#include <asm/unaligned.h>
#include "tmio-common.h"

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)

/* SCC registers */
#define RENESAS_SDHI_SCC_DTCNTL			0x800
#define RENESAS_SDHI_SCC_DTCNTL_TAPEN		BIT(0)
#define RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT	16
#define RENESAS_SDHI_SCC_DTCNTL_TAPNUM_MASK	0xff
#define RENESAS_SDHI_SCC_TAPSET			0x804
#define RENESAS_SDHI_SCC_DT2FF			0x808
#define RENESAS_SDHI_SCC_CKSEL			0x80c
#define RENESAS_SDHI_SCC_CKSEL_DTSEL		BIT(0)
#define RENESAS_SDHI_SCC_RVSCNTL		0x810
#define RENESAS_SDHI_SCC_RVSCNTL_RVSEN		BIT(0)
#define RENESAS_SDHI_SCC_RVSREQ			0x814
#define RENESAS_SDHI_SCC_RVSREQ_RVSERR		BIT(2)
#define RENESAS_SDHI_SCC_RVSREQ_REQTAPUP	BIT(1)
#define RENESAS_SDHI_SCC_RVSREQ_REQTAPDOWN	BIT(0)
#define RENESAS_SDHI_SCC_SMPCMP			0x818
#define RENESAS_SDHI_SCC_SMPCMP_CMD_ERR		(BIT(24) | BIT(8))
#define RENESAS_SDHI_SCC_SMPCMP_CMD_REQUP	BIT(24)
#define RENESAS_SDHI_SCC_SMPCMP_CMD_REQDOWN	BIT(8)
#define RENESAS_SDHI_SCC_TMPPORT2		0x81c
#define RENESAS_SDHI_SCC_TMPPORT2_HS400EN	BIT(31)
#define RENESAS_SDHI_SCC_TMPPORT2_HS400OSEL	BIT(4)
#define RENESAS_SDHI_SCC_TMPPORT3		0x828
#define RENESAS_SDHI_SCC_TMPPORT3_OFFSET_0	3
#define RENESAS_SDHI_SCC_TMPPORT3_OFFSET_1	2
#define RENESAS_SDHI_SCC_TMPPORT3_OFFSET_2	1
#define RENESAS_SDHI_SCC_TMPPORT3_OFFSET_3	0
#define RENESAS_SDHI_SCC_TMPPORT3_OFFSET_MASK	0x3
#define RENESAS_SDHI_SCC_TMPPORT4		0x82c
#define RENESAS_SDHI_SCC_TMPPORT4_DLL_ACC_START	BIT(0)
#define RENESAS_SDHI_SCC_TMPPORT5		0x830
#define RENESAS_SDHI_SCC_TMPPORT5_DLL_RW_SEL_R	BIT(8)
#define RENESAS_SDHI_SCC_TMPPORT5_DLL_RW_SEL_W	(0 << 8)
#define RENESAS_SDHI_SCC_TMPPORT5_DLL_ADR_MASK	0x3F
#define RENESAS_SDHI_SCC_TMPPORT6		0x834
#define RENESAS_SDHI_SCC_TMPPORT7		0x838
#define RENESAS_SDHI_SCC_TMPPORT_DISABLE_WP_CODE	0xa5000000
#define RENESAS_SDHI_SCC_TMPPORT_CALIB_CODE_MASK	0x1f
#define RENESAS_SDHI_SCC_TMPPORT_MANUAL_MODE		BIT(7)

#define RENESAS_SDHI_MAX_TAP 3

#define CALIB_TABLE_MAX	(RENESAS_SDHI_SCC_TMPPORT_CALIB_CODE_MASK + 1)

static const u8 r8a7796_rev13_calib_table[2][CALIB_TABLE_MAX] = {
	{ 3,  3,  3,  3,  3,  3,  3,  4,  4,  5,  6,  7,  8,  9, 10, 15,
	 16, 16, 16, 16, 16, 16, 17, 18, 18, 19, 20, 21, 22, 23, 24, 25 },
	{ 5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  7,  8, 11,
	 12, 17, 18, 18, 18, 18, 18, 18, 18, 19, 20, 21, 22, 23, 25, 25 }
};

static const u8 r8a77965_calib_table[2][CALIB_TABLE_MAX] = {
	{ 1,  2,  6,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 15, 15, 16,
	 17, 18, 19, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 30, 31 },
	{ 2,  3,  4,  4,  5,  6,  7,  9, 10, 11, 12, 13, 14, 15, 16, 17,
	 17, 17, 20, 21, 22, 23, 24, 25, 27, 28, 29, 30, 31, 31, 31, 31 }
};

static const u8 r8a77990_calib_table[2][CALIB_TABLE_MAX] = {
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
	{ 0,  0,  0,  1,  2,  3,  3,  4,  4,  4,  5,  5,  6,  8,  9, 10,
	 11, 12, 13, 15, 16, 17, 17, 18, 18, 19, 20, 22, 24, 25, 26, 26 }
};

static int rcar_is_gen3_mmc0(struct tmio_sd_priv *priv)
{
	/* On R-Car Gen3, MMC0 is at 0xee140000 */
	return (uintptr_t)(priv->regbase) == 0xee140000;
}

static u32 sd_scc_tmpport_read32(struct tmio_sd_priv *priv, u32 addr)
{
	/* read mode */
	tmio_sd_writel(priv, RENESAS_SDHI_SCC_TMPPORT5_DLL_RW_SEL_R |
		       (RENESAS_SDHI_SCC_TMPPORT5_DLL_ADR_MASK & addr),
		       RENESAS_SDHI_SCC_TMPPORT5);

	/* access start and stop */
	tmio_sd_writel(priv, RENESAS_SDHI_SCC_TMPPORT4_DLL_ACC_START,
		       RENESAS_SDHI_SCC_TMPPORT4);
	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_TMPPORT4);

	return tmio_sd_readl(priv, RENESAS_SDHI_SCC_TMPPORT7);
}

static void sd_scc_tmpport_write32(struct tmio_sd_priv *priv, u32 addr, u32 val)
{
	/* write mode */
	tmio_sd_writel(priv, RENESAS_SDHI_SCC_TMPPORT5_DLL_RW_SEL_W |
		       (RENESAS_SDHI_SCC_TMPPORT5_DLL_ADR_MASK & addr),
		       RENESAS_SDHI_SCC_TMPPORT5);
	tmio_sd_writel(priv, val, RENESAS_SDHI_SCC_TMPPORT6);

	/* access start and stop */
	tmio_sd_writel(priv, RENESAS_SDHI_SCC_TMPPORT4_DLL_ACC_START,
		       RENESAS_SDHI_SCC_TMPPORT4);
	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_TMPPORT4);
}

static bool renesas_sdhi_check_scc_error(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	unsigned long new_tap = priv->tap_set;
	unsigned long error_tap = priv->tap_set;
	u32 reg, smpcmp;

	if ((priv->caps & TMIO_SD_CAP_RCAR_UHS) &&
	    (mmc->selected_mode != UHS_SDR104) &&
	    (mmc->selected_mode != MMC_HS_200) &&
	    (mmc->selected_mode != MMC_HS_400) &&
	    (priv->nrtaps != 4))
		return false;

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	/* Handle automatic tuning correction */
	if (reg & RENESAS_SDHI_SCC_RVSCNTL_RVSEN) {
		reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSREQ);
		if (reg & RENESAS_SDHI_SCC_RVSREQ_RVSERR) {
			tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_RVSREQ);
			return true;
		}

		return false;
	}

	/* Handle manual tuning correction */
	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSREQ);
	if (!reg)	/* No error */
		return false;

	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_RVSREQ);

	if (mmc->selected_mode == MMC_HS_400) {
		/*
		 * Correction Error Status contains CMD and DAT signal status.
		 * In HS400, DAT signal based on DS signal, not CLK.
		 * Therefore, use only CMD status.
		 */
		smpcmp = tmio_sd_readl(priv, RENESAS_SDHI_SCC_SMPCMP) &
			 RENESAS_SDHI_SCC_SMPCMP_CMD_ERR;

		switch (smpcmp) {
		case 0:
			return false;	/* No error in CMD signal */
		case RENESAS_SDHI_SCC_SMPCMP_CMD_REQUP:
			new_tap = (priv->tap_set +
				   priv->tap_num + 1) % priv->tap_num;
			error_tap = (priv->tap_set +
				     priv->tap_num - 1) % priv->tap_num;
			break;
		case RENESAS_SDHI_SCC_SMPCMP_CMD_REQDOWN:
			new_tap = (priv->tap_set +
				   priv->tap_num - 1) % priv->tap_num;
			error_tap = (priv->tap_set +
				     priv->tap_num + 1) % priv->tap_num;
			break;
		default:
			return true;	/* Need re-tune */
		}

		if (priv->hs400_bad_tap & BIT(new_tap)) {
			/*
			 * New tap is bad tap (cannot change).
			 * Compare with HS200 tuning result.
			 * In HS200 tuning, when smpcmp[error_tap]
			 * is OK, retune is executed.
			 */
			if (priv->smpcmp & BIT(error_tap))
				return true;	/* Need retune */

			return false;	/* cannot change */
		}

		priv->tap_set = new_tap;
	} else {
		if (reg & RENESAS_SDHI_SCC_RVSREQ_RVSERR)
			return true;	/* Need re-tune */
		else if (reg & RENESAS_SDHI_SCC_RVSREQ_REQTAPUP)
			priv->tap_set = (priv->tap_set +
					 priv->tap_num + 1) % priv->tap_num;
		else if (reg & RENESAS_SDHI_SCC_RVSREQ_REQTAPDOWN)
			priv->tap_set = (priv->tap_set +
					 priv->tap_num - 1) % priv->tap_num;
		else
			return false;
	}

	/* Set TAP position */
	tmio_sd_writel(priv, priv->tap_set >> ((priv->nrtaps == 4) ? 1 : 0),
		       RENESAS_SDHI_SCC_TAPSET);

	return false;
}

static void renesas_sdhi_adjust_hs400_mode_enable(struct tmio_sd_priv *priv)
{
	u32 calib_code;

	if (!priv->adjust_hs400_enable)
		return;

	if (!priv->needs_adjust_hs400)
		return;

	if (!priv->adjust_hs400_calib_table)
		return;

	/*
	 * Enabled Manual adjust HS400 mode
	 *
	 * 1) Disabled Write Protect
	 *    W(addr=0x00, WP_DISABLE_CODE)
	 *
	 * 2) Read Calibration code
	 *    read_value = R(addr=0x26)
	 * 3) Refer to calibration table
	 *    Calibration code = table[read_value]
	 * 4) Enabled Manual Calibration
	 *    W(addr=0x22, manual mode | Calibration code)
	 * 5) Set Offset value to TMPPORT3 Reg
	 */
	sd_scc_tmpport_write32(priv, 0x00,
			       RENESAS_SDHI_SCC_TMPPORT_DISABLE_WP_CODE);
	calib_code = sd_scc_tmpport_read32(priv, 0x26);
	calib_code &= RENESAS_SDHI_SCC_TMPPORT_CALIB_CODE_MASK;
	sd_scc_tmpport_write32(priv, 0x22,
			       RENESAS_SDHI_SCC_TMPPORT_MANUAL_MODE |
			       priv->adjust_hs400_calib_table[calib_code]);
	tmio_sd_writel(priv, priv->adjust_hs400_offset,
		       RENESAS_SDHI_SCC_TMPPORT3);

	/* Clear flag */
	priv->needs_adjust_hs400 = false;
}

static void renesas_sdhi_adjust_hs400_mode_disable(struct tmio_sd_priv *priv)
{

	/* Disabled Manual adjust HS400 mode
	 *
	 * 1) Disabled Write Protect
	 *    W(addr=0x00, WP_DISABLE_CODE)
	 * 2) Disabled Manual Calibration
	 *    W(addr=0x22, 0)
	 * 3) Clear offset value to TMPPORT3 Reg
	 */
	sd_scc_tmpport_write32(priv, 0x00,
			       RENESAS_SDHI_SCC_TMPPORT_DISABLE_WP_CODE);
	sd_scc_tmpport_write32(priv, 0x22, 0);
	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_TMPPORT3);
}

static unsigned int renesas_sdhi_init_tuning(struct tmio_sd_priv *priv)
{
	u32 reg;

	/* Initialize SCC */
	tmio_sd_writel(priv, 0, TMIO_SD_INFO1);

	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg &= ~TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	/* Set sampling clock selection range */
	tmio_sd_writel(priv, (0x8 << RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT) |
			     RENESAS_SDHI_SCC_DTCNTL_TAPEN,
			     RENESAS_SDHI_SCC_DTCNTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_CKSEL);
	reg |= RENESAS_SDHI_SCC_CKSEL_DTSEL;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_CKSEL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	tmio_sd_writel(priv, 0x300 /* scc_tappos */,
			   RENESAS_SDHI_SCC_DT2FF);

	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg |= TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	/* Read TAPNUM */
	return (tmio_sd_readl(priv, RENESAS_SDHI_SCC_DTCNTL) >>
		RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT) &
		RENESAS_SDHI_SCC_DTCNTL_TAPNUM_MASK;
}

static void renesas_sdhi_reset_tuning(struct tmio_sd_priv *priv, bool clk_disable)
{
	u32 reg;

	/* Reset SCC */
	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg &= ~TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_CKSEL);
	reg &= ~RENESAS_SDHI_SCC_CKSEL_DTSEL;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_CKSEL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_TMPPORT2);
	reg &= ~(RENESAS_SDHI_SCC_TMPPORT2_HS400EN |
		 RENESAS_SDHI_SCC_TMPPORT2_HS400OSEL);
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_TMPPORT2);

	/* Disable HS400 mode adjustment */
	renesas_sdhi_adjust_hs400_mode_disable(priv);

	reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	reg |= TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	if (clk_disable) {
		reg = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
		reg &= ~TMIO_SD_CLKCTL_SCLKEN;
		tmio_sd_writel(priv, reg, TMIO_SD_CLKCTL);
	}
}

static int renesas_sdhi_hs400(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	bool hs400 = (mmc->selected_mode == MMC_HS_400);
	int ret, taps = hs400 ? priv->nrtaps : 8;
	const u32 sdn_rate = 200000000;
	u32 sdnh_rate = 800000000;
	unsigned long new_tap;
	u32 reg;

	if (clk_valid(&priv->clkh) && !priv->needs_clkh_fallback) {
		/* HS400 on 4tap SoC => SDnH=400 MHz, SDn=200 MHz */
		if (taps == 4)
			sdnh_rate /= 2;
		ret = clk_set_rate(&priv->clkh, sdnh_rate);
		if (ret < 0)
			return ret;
	}

	ret = clk_set_rate(&priv->clk, sdn_rate);
	if (ret < 0)
		return ret;

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg &= ~RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_TMPPORT2);
	if (hs400) {
		reg |= RENESAS_SDHI_SCC_TMPPORT2_HS400EN |
		       RENESAS_SDHI_SCC_TMPPORT2_HS400OSEL;
	} else {
		reg &= ~(RENESAS_SDHI_SCC_TMPPORT2_HS400EN |
		       RENESAS_SDHI_SCC_TMPPORT2_HS400OSEL);
	}

	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_TMPPORT2);

	/* Disable HS400 mode adjustment */
	if (!hs400)
		renesas_sdhi_adjust_hs400_mode_disable(priv);

	tmio_sd_writel(priv, (0x8 << RENESAS_SDHI_SCC_DTCNTL_TAPNUM_SHIFT) |
			     RENESAS_SDHI_SCC_DTCNTL_TAPEN,
			     RENESAS_SDHI_SCC_DTCNTL);

	/* Avoid bad TAP */
	if (priv->hs400_bad_tap & BIT(priv->tap_set)) {
		new_tap = (priv->tap_set +
			   priv->tap_num + 1) % priv->tap_num;

		if (priv->hs400_bad_tap & BIT(new_tap))
			new_tap = (priv->tap_set +
				   priv->tap_num - 1) % priv->tap_num;

		if (priv->hs400_bad_tap & BIT(new_tap)) {
			new_tap = priv->tap_set;
			debug("Three consecutive bad tap is prohibited\n");
		}

		priv->tap_set = new_tap;
		tmio_sd_writel(priv, priv->tap_set, RENESAS_SDHI_SCC_TAPSET);
	}

	if (taps == 4) {
		tmio_sd_writel(priv, priv->tap_set >> 1,
			       RENESAS_SDHI_SCC_TAPSET);
		tmio_sd_writel(priv, hs400 ? 0x100 : 0x300,
			       RENESAS_SDHI_SCC_DT2FF);
	} else {
		tmio_sd_writel(priv, priv->tap_set, RENESAS_SDHI_SCC_TAPSET);
		tmio_sd_writel(priv, 0x300, RENESAS_SDHI_SCC_DT2FF);
	}

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_CKSEL);
	reg |= RENESAS_SDHI_SCC_CKSEL_DTSEL;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_CKSEL);

	/* Execute adjust hs400 offset after setting to HS400 mode */
	if (hs400)
		priv->needs_adjust_hs400 = true;

	return 0;
}

static void renesas_sdhi_prepare_tuning(struct tmio_sd_priv *priv,
				       unsigned long tap)
{
	/* Set sampling clock position */
	tmio_sd_writel(priv, tap, RENESAS_SDHI_SCC_TAPSET);
}

static unsigned int renesas_sdhi_compare_scc_data(struct tmio_sd_priv *priv)
{
	/* Get comparison of sampling data */
	return tmio_sd_readl(priv, RENESAS_SDHI_SCC_SMPCMP);
}

static int renesas_sdhi_select_tuning(struct tmio_sd_priv *priv,
				     unsigned int taps)
{
	unsigned long tap_cnt;  /* counter of tuning success */
	unsigned long tap_start;/* start position of tuning success */
	unsigned long tap_end;  /* end position of tuning success */
	unsigned long ntap;     /* temporary counter of tuning success */
	unsigned long match_cnt;/* counter of matching data */
	unsigned long i;
	bool select = false;
	u32 reg;

	priv->needs_adjust_hs400 = false;

	/* Clear SCC_RVSREQ */
	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_RVSREQ);

	/* Merge the results */
	for (i = 0; i < priv->tap_num * 2; i++) {
		if (!(taps & BIT(i))) {
			taps &= ~BIT(i % priv->tap_num);
			taps &= ~BIT((i % priv->tap_num) + priv->tap_num);
		}
		if (!(priv->smpcmp & BIT(i))) {
			priv->smpcmp &= ~BIT(i % priv->tap_num);
			priv->smpcmp &= ~BIT((i % priv->tap_num) + priv->tap_num);
		}
	}

	/*
	 * Find the longest consecutive run of successful probes.  If that
	 * is more than RENESAS_SDHI_MAX_TAP probes long then use the
	 * center index as the tap.
	 */
	tap_cnt = 0;
	ntap = 0;
	tap_start = 0;
	tap_end = 0;
	for (i = 0; i < priv->tap_num * 2; i++) {
		if (taps & BIT(i))
			ntap++;
		else {
			if (ntap > tap_cnt) {
				tap_start = i - ntap;
				tap_end = i - 1;
				tap_cnt = ntap;
			}
			ntap = 0;
		}
	}

	if (ntap > tap_cnt) {
		tap_start = i - ntap;
		tap_end = i - 1;
		tap_cnt = ntap;
	}

	/*
	 * If all of the TAP is OK, the sampling clock position is selected by
	 * identifying the change point of data.
	 */
	if (tap_cnt == priv->tap_num * 2) {
		match_cnt = 0;
		ntap = 0;
		tap_start = 0;
		tap_end = 0;
		for (i = 0; i < priv->tap_num * 2; i++) {
			if (priv->smpcmp & BIT(i))
				ntap++;
			else {
				if (ntap > match_cnt) {
					tap_start = i - ntap;
					tap_end = i - 1;
					match_cnt = ntap;
				}
				ntap = 0;
			}
		}
		if (ntap > match_cnt) {
			tap_start = i - ntap;
			tap_end = i - 1;
			match_cnt = ntap;
		}
		if (match_cnt)
			select = true;
	} else if (tap_cnt >= RENESAS_SDHI_MAX_TAP)
		select = true;

	if (select)
		priv->tap_set = ((tap_start + tap_end) / 2) % priv->tap_num;
	else
		return -EIO;

	/* Set SCC */
	tmio_sd_writel(priv, priv->tap_set, RENESAS_SDHI_SCC_TAPSET);

	/* Enable auto re-tuning */
	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg |= RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

	return 0;
}

int renesas_sdhi_execute_tuning(struct udevice *dev, uint opcode)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;
	unsigned int tap_num;
	unsigned int taps = 0;
	int i, ret = 0;
	u32 caps;

	/* Only supported on Renesas RCar */
	if (!(priv->caps & TMIO_SD_CAP_RCAR_UHS))
		return -EINVAL;

	/* clock tuning is not needed for upto 52MHz */
	if (!((mmc->selected_mode == MMC_HS_200) ||
	      (mmc->selected_mode == MMC_HS_400) ||
	      (mmc->selected_mode == UHS_SDR104) ||
	      (mmc->selected_mode == UHS_SDR50)))
		return 0;

	tap_num = renesas_sdhi_init_tuning(priv);
	if (!tap_num)
		/* Tuning is not supported */
		goto out;

	priv->tap_num = tap_num;

	if (priv->tap_num * 2 >= sizeof(taps) * 8) {
		dev_err(dev,
			"Too many taps, skipping tuning. Please consider updating size of taps field of tmio_mmc_host\n");
		goto out;
	}

	priv->smpcmp = 0;

	/* Issue CMD19 twice for each tap */
	for (i = 0; i < 2 * priv->tap_num; i++) {
		renesas_sdhi_prepare_tuning(priv, i % priv->tap_num);

		/* Force PIO for the tuning */
		caps = priv->caps;
		priv->caps &= ~TMIO_SD_CAP_DMA_INTERNAL;

		ret = mmc_send_tuning(mmc, opcode, NULL);

		priv->caps = caps;

		if (ret == 0)
			taps |= BIT(i);

		ret = renesas_sdhi_compare_scc_data(priv);
		if (ret == 0)
			priv->smpcmp |= BIT(i);

		mdelay(1);

		/*
		 * eMMC specification specifies that CMD12 can be used to stop a tuning
		 * command, but SD specification does not, so do nothing unless it is
		 * eMMC.
		 */
		if (ret && (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200)) {
			ret = mmc_send_stop_transmission(mmc, false);
			if (ret < 0)
				dev_dbg(dev, "Tuning abort fail (%d)\n", ret);
		}
	}

	ret = renesas_sdhi_select_tuning(priv, taps);

out:
	if (ret < 0) {
		dev_warn(dev, "Tuning procedure failed\n");
		renesas_sdhi_reset_tuning(priv, true);
	}

	return ret;
}
#else
static int renesas_sdhi_hs400(struct udevice *dev)
{
	return 0;
}
#endif

static int renesas_sdhi_set_ios(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	u32 tmp;
	int ret;

	/* Stop the clock before changing its rate to avoid a glitch signal */
	tmp = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	tmp &= ~TMIO_SD_CLKCTL_SCLKEN;
	tmio_sd_writel(priv, tmp, TMIO_SD_CLKCTL);

	ret = renesas_sdhi_hs400(dev);
	if (ret)
		return ret;

	ret = tmio_sd_set_ios(dev);

	mdelay(10);

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	if ((priv->caps & TMIO_SD_CAP_RCAR_UHS) &&
	    (mmc->selected_mode != UHS_SDR104) &&
	    (mmc->selected_mode != MMC_HS_200) &&
	    (mmc->selected_mode != MMC_HS_400)) {
		renesas_sdhi_reset_tuning(priv, mmc->clk_disable);
	}
#endif

	return ret;
}

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
static int renesas_sdhi_wait_dat0(struct udevice *dev, int state,
				  int timeout_us)
{
	int ret = -ETIMEDOUT;
	bool dat0_high;
	bool target_dat0_high = !!state;
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	timeout_us = DIV_ROUND_UP(timeout_us, 10); /* check every 10 us. */
	while (timeout_us--) {
		dat0_high = !!(tmio_sd_readl(priv, TMIO_SD_INFO2) & TMIO_SD_INFO2_DAT0);
		if (dat0_high == target_dat0_high) {
			ret = 0;
			break;
		}
		udelay(10);
	}

	return ret;
}
#endif

#define RENESAS_SDHI_DMA_ALIGNMENT	128

static int renesas_sdhi_addr_aligned_gen(uintptr_t ubuf,
					 size_t len, size_t len_aligned)
{
	/* Check if start is aligned */
	if (!IS_ALIGNED(ubuf, RENESAS_SDHI_DMA_ALIGNMENT)) {
		debug("Unaligned buffer address %lx\n", ubuf);
		return 0;
	}

	/* Check if length is aligned */
	if (len != len_aligned) {
		debug("Unaligned buffer length %zu\n", len);
		return 0;
	}

#ifdef CONFIG_PHYS_64BIT
	/* Check if below 32bit boundary */
	if ((ubuf >> 32) || (ubuf + len_aligned) >> 32) {
		debug("Buffer above 32bit boundary %lx-%lx\n",
			ubuf, ubuf + len_aligned);
		return 0;
	}
#endif

	/* Aligned */
	return 1;
}

static int renesas_sdhi_addr_aligned(struct bounce_buffer *state)
{
	uintptr_t ubuf = (uintptr_t)state->user_buffer;

	return renesas_sdhi_addr_aligned_gen(ubuf, state->len,
					     state->len_aligned);
}

static int renesas_sdhi_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
				 struct mmc_data *data)
{
	struct bounce_buffer bbstate;
	unsigned int bbflags;
	bool bbok = false;
	size_t len;
	void *buf;
	int ret;

	if (data) {
		if (data->flags & MMC_DATA_READ) {
			buf = data->dest;
			bbflags = GEN_BB_WRITE;
		} else {
			buf = (void *)data->src;
			bbflags = GEN_BB_READ;
		}
		len = data->blocks * data->blocksize;

		ret = bounce_buffer_start_extalign(&bbstate, buf, len, bbflags,
						   RENESAS_SDHI_DMA_ALIGNMENT,
						   renesas_sdhi_addr_aligned);
		/*
		 * If the amount of data to transfer is too large, we can get
		 * -ENOMEM when starting the bounce buffer. If that happens,
		 *  fall back to PIO as it was before, otherwise use the BB.
		 */
		if (!ret) {
			bbok = true;
			if (data->flags & MMC_DATA_READ)
				data->dest = bbstate.bounce_buffer;
			else
				data->src = bbstate.bounce_buffer;
		}
	}

	ret = tmio_sd_send_cmd(dev, cmd, data);

	if (data && bbok) {
		buf = bbstate.user_buffer;

		bounce_buffer_stop(&bbstate);

		if (data->flags & MMC_DATA_READ)
			data->dest = buf;
		else
			data->src = buf;
	}

	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	renesas_sdhi_check_scc_error(dev);

	if (cmd->cmdidx == MMC_CMD_SEND_STATUS)
		renesas_sdhi_adjust_hs400_mode_enable(priv);
#endif

	return 0;
}

int renesas_sdhi_get_b_max(struct udevice *dev, void *dst, lbaint_t blkcnt)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;
	size_t len = blkcnt * mmc->read_bl_len;
	size_t len_align = roundup(len, RENESAS_SDHI_DMA_ALIGNMENT);

	if (renesas_sdhi_addr_aligned_gen((uintptr_t)dst, len, len_align)) {
		if (priv->quirks & TMIO_SD_CAP_16BIT)
			return U16_MAX;
		else
			return U32_MAX;
	} else {
		return (CONFIG_SYS_MALLOC_LEN / 4) / mmc->read_bl_len;
	}
}

static const struct dm_mmc_ops renesas_sdhi_ops = {
	.send_cmd = renesas_sdhi_send_cmd,
	.set_ios = renesas_sdhi_set_ios,
	.get_cd = tmio_sd_get_cd,
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	.execute_tuning = renesas_sdhi_execute_tuning,
#endif
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	.wait_dat0 = renesas_sdhi_wait_dat0,
#endif
	.get_b_max = renesas_sdhi_get_b_max,
};

#define RENESAS_GEN2_QUIRKS	TMIO_SD_CAP_RCAR_GEN2
#define RENESAS_GEN3_QUIRKS				\
	TMIO_SD_CAP_64BIT | TMIO_SD_CAP_RCAR_GEN3 | TMIO_SD_CAP_RCAR_UHS

static const struct udevice_id renesas_sdhi_match[] = {
	{ .compatible = "renesas,sdhi-r8a7790", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7791", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7792", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7793", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7794", .data = RENESAS_GEN2_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7795", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a7796", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77961", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,rcar-gen3-sdhi", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77965", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77970", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77990", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77995", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,rcar-gen4-sdhi", .data = RENESAS_GEN3_QUIRKS },
	{ /* sentinel */ }
};

static ulong renesas_sdhi_clk_get_rate(struct tmio_sd_priv *priv)
{
	return clk_get_rate(&priv->clk);
}

static void renesas_sdhi_filter_caps(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	if (!(priv->caps & TMIO_SD_CAP_RCAR_GEN3))
		return;

	if (priv->caps & TMIO_SD_CAP_DMA_INTERNAL)
		priv->idma_bus_width = TMIO_SD_DMA_MODE_BUS_WIDTH;

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	struct tmio_sd_plat *plat = dev_get_plat(dev);

	/* HS400 is not supported on H3 ES1.x, M3W ES1.[012], V3M, V3H ES1.x, D3 */
	if (((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7795) &&
	    (renesas_get_cpu_rev_integer() <= 1)) ||
	    ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7796) &&
	    (renesas_get_cpu_rev_integer() == 1) &&
	    (renesas_get_cpu_rev_fraction() <= 2)) ||
	    (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77970) ||
	    ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77980) &&
	    (renesas_get_cpu_rev_integer() <= 1)) ||
	    (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77995))
		plat->cfg.host_caps &= ~MMC_MODE_HS400;

	/* H3 ES2.0, ES3.0 and M3W ES1.2 and M3N bad taps */
	if (((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7795) &&
	    (renesas_get_cpu_rev_integer() >= 2)) ||
	    ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7796) &&
	    (renesas_get_cpu_rev_integer() == 1) &&
	    (renesas_get_cpu_rev_fraction() == 2)) ||
	    (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77965))
		priv->hs400_bad_tap = BIT(2) | BIT(3) | BIT(6) | BIT(7);

	/* M3W ES1.x for x>2 can use HS400 with manual adjustment and taps */
	if ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7796) &&
	    (renesas_get_cpu_rev_integer() == 1) &&
	    (renesas_get_cpu_rev_fraction() > 2)) {
		priv->adjust_hs400_enable = true;
		priv->adjust_hs400_offset = 3;
		priv->hs400_bad_tap = BIT(1) | BIT(3) | BIT(5) | BIT(7);
		priv->adjust_hs400_calib_table =
			r8a7796_rev13_calib_table[!rcar_is_gen3_mmc0(priv)];
	}

	/* M3W+ bad taps */
	if ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7796) &&
	    (renesas_get_cpu_rev_integer() == 3))
		priv->hs400_bad_tap = BIT(1) | BIT(3) | BIT(5) | BIT(7);

	/* M3N can use HS400 with manual adjustment */
	if (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77965) {
		priv->adjust_hs400_enable = true;
		priv->adjust_hs400_offset = 3;
		priv->adjust_hs400_calib_table =
			r8a77965_calib_table[!rcar_is_gen3_mmc0(priv)];
	}

	/* E3 can use HS400 with manual adjustment */
	if (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77990) {
		priv->adjust_hs400_enable = true;
		priv->adjust_hs400_offset = 3;
		priv->adjust_hs400_calib_table =
			r8a77990_calib_table[!rcar_is_gen3_mmc0(priv)];
	}

	/* H3 ES1.x, ES2.0 and M3W ES1.[0123] uses 4 tuning taps */
	if (((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7795) &&
	    (renesas_get_cpu_rev_integer() <= 2)) ||
	    ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7796) &&
	    (renesas_get_cpu_rev_integer() == 1) &&
	    (renesas_get_cpu_rev_fraction() <= 3)))
		priv->nrtaps = 4;
	else
		priv->nrtaps = 8;
#endif
	/* H3 ES1.x and M3W ES1.0 uses bit 17 for DTRAEND */
	if (((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7795) &&
	    (renesas_get_cpu_rev_integer() <= 1)) ||
	    ((renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A7796) &&
	    (renesas_get_cpu_rev_integer() == 1) &&
	    (renesas_get_cpu_rev_fraction() == 0)))
		priv->read_poll_flag = TMIO_SD_DMA_INFO1_END_RD;
	else
		priv->read_poll_flag = TMIO_SD_DMA_INFO1_END_RD2;

	/* V3M handles SD0H differently than other Gen3 SoCs */
	if (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A77970)
		priv->needs_clkh_fallback = true;
	else
		priv->needs_clkh_fallback = false;
}

static int rzg2l_sdhi_setup(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct clk imclk2, aclk;
	struct reset_ctl rst;
	int ret;

	/*
	 * On members of the RZ/G2L SoC family, we need to enable
	 * additional chip detect and bus clocks, then release the SDHI
	 * module from reset.
	 */
	ret = clk_get_by_name(dev, "cd", &imclk2);
	if (ret < 0) {
		dev_err(dev, "failed to get imclk2 (chip detect clk)\n");
		return ret;
	}

	ret = clk_get_by_name(dev, "aclk", &aclk);
	if (ret < 0) {
		dev_err(dev, "failed to get aclk\n");
		return ret;
	}

	ret = clk_enable(&imclk2);
	if (ret < 0) {
		dev_err(dev, "failed to enable imclk2 (chip detect clk)\n");
		return ret;
	}

	ret = clk_enable(&aclk);
	if (ret < 0) {
		dev_err(dev, "failed to enable aclk\n");
		goto err_aclk;
	}

	ret = reset_get_by_index(dev, 0, &rst);
	if (ret < 0) {
		dev_err(dev, "failed to get reset line\n");
		goto err_get_reset;
	}

	ret = reset_deassert(&rst);
	if (ret < 0) {
		dev_err(dev, "failed to de-assert reset line\n");
		goto err_reset;
	}

	ret = tmio_sd_probe(dev, priv->quirks);
	if (ret)
		goto err_tmio_probe;

	return 0;

err_tmio_probe:
	reset_assert(&rst);
err_reset:
	reset_free(&rst);
err_get_reset:
	clk_disable(&aclk);
err_aclk:
	clk_disable(&imclk2);
	return ret;
}

static int renesas_sdhi_probe(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct fdt_resource reg_res;
	DECLARE_GLOBAL_DATA_PTR;
	int ret;

	priv->clk_get_rate = renesas_sdhi_clk_get_rate;

	priv->quirks = dev_get_driver_data(dev);
	if (priv->quirks == RENESAS_GEN2_QUIRKS) {
		ret = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev),
				       "reg", 0, &reg_res);
		if (ret < 0) {
			dev_err(dev, "\"reg\" resource not found, ret=%i\n",
				ret);
			return ret;
		}

		if (fdt_resource_size(&reg_res) == 0x100)
			priv->quirks |= TMIO_SD_CAP_16BIT;
	}

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0) {
		dev_err(dev, "failed to get host clock\n");
		return ret;
	}

	/* optional SDnH clock */
	ret = clk_get_by_name(dev, "clkh", &priv->clkh);
	if (ret < 0) {
		dev_dbg(dev, "failed to get clkh\n");
	} else {
		ret = clk_set_rate(&priv->clkh, 800000000);
		if (ret < 0) {
			dev_err(dev, "failed to set rate for SDnH clock (%d)\n", ret);
			return ret;
		}
	}

	/* set to max rate */
	ret = clk_set_rate(&priv->clk, 200000000);
	if (ret < 0) {
		dev_err(dev, "failed to set rate for SDn clock (%d)\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "failed to enable SDn clock (%d)\n", ret);
		return ret;
	}

	if (device_is_compatible(dev, "renesas,sdhi-r9a07g044"))
		ret = rzg2l_sdhi_setup(dev);
	else
		ret = tmio_sd_probe(dev, priv->quirks);
	if (ret)
		goto err_tmio_probe;

	renesas_sdhi_filter_caps(dev);

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	if (priv->caps & TMIO_SD_CAP_RCAR_UHS)
		renesas_sdhi_reset_tuning(priv, true);
#endif
	return 0;

err_tmio_probe:
	clk_disable(&priv->clk);
	return ret;
}

U_BOOT_DRIVER(renesas_sdhi) = {
	.name = "renesas-sdhi",
	.id = UCLASS_MMC,
	.of_match = renesas_sdhi_match,
	.bind = tmio_sd_bind,
	.probe = renesas_sdhi_probe,
	.priv_auto	= sizeof(struct tmio_sd_priv),
	.plat_auto	= sizeof(struct tmio_sd_plat),
	.ops = &renesas_sdhi_ops,
};
