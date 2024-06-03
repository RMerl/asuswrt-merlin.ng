// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <fdtdec.h>
#include <mmc.h>
#include <dm.h>
#include <linux/compat.h>
#include <linux/dma-direction.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <power/regulator.h>
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
#define RENESAS_SDHI_SCC_SMPCMP			0x818
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

static void renesas_sdhi_adjust_hs400_mode_enable(struct tmio_sd_priv *priv)
{
	u32 calib_code;

	if (!priv->adjust_hs400_enable)
		return;

	if (!priv->needs_adjust_hs400)
		return;

	/*
	 * Enabled Manual adjust HS400 mode
	 *
	 * 1) Disabled Write Protect
	 *    W(addr=0x00, WP_DISABLE_CODE)
	 * 2) Read Calibration code and adjust
	 *    R(addr=0x26) - adjust value
	 * 3) Enabled Manual Calibration
	 *    W(addr=0x22, manual mode | Calibration code)
	 * 4) Set Offset value to TMPPORT3 Reg
	 */
	sd_scc_tmpport_write32(priv, 0x00,
			       RENESAS_SDHI_SCC_TMPPORT_DISABLE_WP_CODE);
	calib_code = sd_scc_tmpport_read32(priv, 0x26);
	calib_code &= RENESAS_SDHI_SCC_TMPPORT_CALIB_CODE_MASK;
	if (calib_code > priv->adjust_hs400_calibrate)
		calib_code -= priv->adjust_hs400_calibrate;
	else
		calib_code = 0;
	sd_scc_tmpport_write32(priv, 0x22,
			       RENESAS_SDHI_SCC_TMPPORT_MANUAL_MODE |
			       calib_code);
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

static void renesas_sdhi_reset_tuning(struct tmio_sd_priv *priv)
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
}

static int renesas_sdhi_hs400(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	bool hs400 = (mmc->selected_mode == MMC_HS_400);
	int ret, taps = hs400 ? priv->nrtaps : 8;
	u32 reg;

	if (taps == 4)	/* HS400 on 4tap SoC needs different clock */
		ret = clk_set_rate(&priv->clk, 400000000);
	else
		ret = clk_set_rate(&priv->clk, 200000000);
	if (ret < 0)
		return ret;

	tmio_sd_writel(priv, 0, RENESAS_SDHI_SCC_RVSREQ);

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

	if (taps == 4) {
		tmio_sd_writel(priv, priv->tap_set >> 1,
			       RENESAS_SDHI_SCC_TAPSET);
	} else {
		tmio_sd_writel(priv, priv->tap_set, RENESAS_SDHI_SCC_TAPSET);
	}

	tmio_sd_writel(priv, hs400 ? 0x704 : 0x300,
		       RENESAS_SDHI_SCC_DT2FF);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_CKSEL);
	reg |= RENESAS_SDHI_SCC_CKSEL_DTSEL;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_CKSEL);

	reg = tmio_sd_readl(priv, RENESAS_SDHI_SCC_RVSCNTL);
	reg |= RENESAS_SDHI_SCC_RVSCNTL_RVSEN;
	tmio_sd_writel(priv, reg, RENESAS_SDHI_SCC_RVSCNTL);

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
				     unsigned int tap_num, unsigned int taps,
				     unsigned int smpcmp)
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
	for (i = 0; i < tap_num * 2; i++) {
		if (!(taps & BIT(i))) {
			taps &= ~BIT(i % tap_num);
			taps &= ~BIT((i % tap_num) + tap_num);
		}
		if (!(smpcmp & BIT(i))) {
			smpcmp &= ~BIT(i % tap_num);
			smpcmp &= ~BIT((i % tap_num) + tap_num);
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
	for (i = 0; i < tap_num * 2; i++) {
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
	if (tap_cnt == tap_num * 2) {
		match_cnt = 0;
		ntap = 0;
		tap_start = 0;
		tap_end = 0;
		for (i = 0; i < tap_num * 2; i++) {
			if (smpcmp & BIT(i))
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
		priv->tap_set = ((tap_start + tap_end) / 2) % tap_num;
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
	unsigned int taps = 0, smpcmp = 0;
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

	if (tap_num * 2 >= sizeof(taps) * 8) {
		dev_err(dev,
			"Too many taps, skipping tuning. Please consider updating size of taps field of tmio_mmc_host\n");
		goto out;
	}

	/* Issue CMD19 twice for each tap */
	for (i = 0; i < 2 * tap_num; i++) {
		renesas_sdhi_prepare_tuning(priv, i % tap_num);

		/* Force PIO for the tuning */
		caps = priv->caps;
		priv->caps &= ~TMIO_SD_CAP_DMA_INTERNAL;

		ret = mmc_send_tuning(mmc, opcode, NULL);

		priv->caps = caps;

		if (ret == 0)
			taps |= BIT(i);

		ret = renesas_sdhi_compare_scc_data(priv);
		if (ret == 0)
			smpcmp |= BIT(i);

		mdelay(1);
	}

	ret = renesas_sdhi_select_tuning(priv, tap_num, taps, smpcmp);

out:
	if (ret < 0) {
		dev_warn(dev, "Tuning procedure failed\n");
		renesas_sdhi_reset_tuning(priv);
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
		renesas_sdhi_reset_tuning(priv);
	}
#endif

	return ret;
}

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
static int renesas_sdhi_wait_dat0(struct udevice *dev, int state, int timeout)
{
	int ret = -ETIMEDOUT;
	bool dat0_high;
	bool target_dat0_high = !!state;
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	timeout = DIV_ROUND_UP(timeout, 10); /* check every 10 us. */
	while (timeout--) {
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

static int renesas_sdhi_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
				 struct mmc_data *data)
{
	int ret;

	ret = tmio_sd_send_cmd(dev, cmd, data);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	if (cmd->cmdidx == MMC_CMD_SEND_STATUS)
		renesas_sdhi_adjust_hs400_mode_enable(priv);
#endif

	return 0;
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
	{ .compatible = "renesas,sdhi-r8a77965", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77970", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77990", .data = RENESAS_GEN3_QUIRKS },
	{ .compatible = "renesas,sdhi-r8a77995", .data = RENESAS_GEN3_QUIRKS },
	{ /* sentinel */ }
};

static ulong renesas_sdhi_clk_get_rate(struct tmio_sd_priv *priv)
{
	return clk_get_rate(&priv->clk);
}

static void renesas_sdhi_filter_caps(struct udevice *dev)
{
	struct tmio_sd_plat *plat = dev_get_platdata(dev);
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	if (!(priv->caps & TMIO_SD_CAP_RCAR_GEN3))
		return;

	/* HS400 is not supported on H3 ES1.x and M3W ES1.0,ES1.1,ES1.2 */
	if (((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A7795) &&
	    (rmobile_get_cpu_rev_integer() <= 1)) ||
	    ((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A7796) &&
	    (rmobile_get_cpu_rev_integer() == 1) &&
	    (rmobile_get_cpu_rev_fraction() <= 2)))
		plat->cfg.host_caps &= ~MMC_MODE_HS400;

	/* M3W ES1.x for x>2 can use HS400 with manual adjustment */
	if ((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A7796) &&
	    (rmobile_get_cpu_rev_integer() == 1) &&
	    (rmobile_get_cpu_rev_fraction() > 2)) {
		priv->adjust_hs400_enable = true;
		priv->adjust_hs400_offset = 0;
		priv->adjust_hs400_calibrate = 0x9;
	}

	/* M3N can use HS400 with manual adjustment */
	if (rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A77965) {
		priv->adjust_hs400_enable = true;
		priv->adjust_hs400_offset = 0;
		priv->adjust_hs400_calibrate = 0x0;
	}

	/* E3 can use HS400 with manual adjustment */
	if (rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A77990) {
		priv->adjust_hs400_enable = true;
		priv->adjust_hs400_offset = 0;
		priv->adjust_hs400_calibrate = 0x2;
	}

	/* H3 ES2.0 uses 4 tuning taps */
	if ((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A7795) &&
	    (rmobile_get_cpu_rev_integer() == 2))
		priv->nrtaps = 4;
	else
		priv->nrtaps = 8;

	/* H3 ES1.x and M3W ES1.0 uses bit 17 for DTRAEND */
	if (((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A7795) &&
	    (rmobile_get_cpu_rev_integer() <= 1)) ||
	    ((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A7796) &&
	    (rmobile_get_cpu_rev_integer() == 1) &&
	    (rmobile_get_cpu_rev_fraction() == 0)))
		priv->read_poll_flag = TMIO_SD_DMA_INFO1_END_RD;
	else
		priv->read_poll_flag = TMIO_SD_DMA_INFO1_END_RD2;
}

static int renesas_sdhi_probe(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	u32 quirks = dev_get_driver_data(dev);
	struct fdt_resource reg_res;
	DECLARE_GLOBAL_DATA_PTR;
	int ret;

	priv->clk_get_rate = renesas_sdhi_clk_get_rate;

	if (quirks == RENESAS_GEN2_QUIRKS) {
		ret = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev),
				       "reg", 0, &reg_res);
		if (ret < 0) {
			dev_err(dev, "\"reg\" resource not found, ret=%i\n",
				ret);
			return ret;
		}

		if (fdt_resource_size(&reg_res) == 0x100)
			quirks |= TMIO_SD_CAP_16BIT;
	}

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0) {
		dev_err(dev, "failed to get host clock\n");
		return ret;
	}

	/* set to max rate */
	ret = clk_set_rate(&priv->clk, 200000000);
	if (ret < 0) {
		dev_err(dev, "failed to set rate for host clock\n");
		clk_free(&priv->clk);
		return ret;
	}

	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "failed to enable host clock\n");
		return ret;
	}

	ret = tmio_sd_probe(dev, quirks);

	renesas_sdhi_filter_caps(dev);

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	if (!ret && (priv->caps & TMIO_SD_CAP_RCAR_UHS))
		renesas_sdhi_reset_tuning(priv);
#endif
	return ret;
}

U_BOOT_DRIVER(renesas_sdhi) = {
	.name = "renesas-sdhi",
	.id = UCLASS_MMC,
	.of_match = renesas_sdhi_match,
	.bind = tmio_sd_bind,
	.probe = renesas_sdhi_probe,
	.priv_auto_alloc_size = sizeof(struct tmio_sd_priv),
	.platdata_auto_alloc_size = sizeof(struct tmio_sd_plat),
	.ops = &renesas_sdhi_ops,
};
