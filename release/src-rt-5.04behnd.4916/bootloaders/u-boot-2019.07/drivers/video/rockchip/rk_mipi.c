// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Eric Gao <eric.gao@rock-chips.com>
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <fdtdec.h>
#include <panel.h>
#include <regmap.h>
#include "rk_mipi.h"
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/uclass-internal.h>
#include <linux/kernel.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3399.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/rockchip_mipi_dsi.h>

DECLARE_GLOBAL_DATA_PTR;

int rk_mipi_read_timing(struct udevice *dev,
			struct display_timing *timing)
{
	int ret;

	ret = fdtdec_decode_display_timing(gd->fdt_blob, dev_of_offset(dev),
					 0, timing);
	if (ret) {
		debug("%s: Failed to decode display timing (ret=%d)\n",
		      __func__, ret);
		return -EINVAL;
	}

	return 0;
}

/*
 * Register write function used only for mipi dsi controller.
 * Parameter:
 *  @regs: mipi controller address
 *  @reg: combination of regaddr(16bit)|bitswidth(8bit)|offset(8bit) you can
 *        use define in rk_mipi.h directly for this parameter
 *  @val: value that will be write to specified bits of register
 */
static void rk_mipi_dsi_write(uintptr_t regs, u32 reg, u32 val)
{
	u32 dat;
	u32 mask;
	u32 offset = (reg >> OFFSET_SHIFT) & 0xff;
	u32 bits = (reg >> BITS_SHIFT) & 0xff;
	uintptr_t addr = (reg >> ADDR_SHIFT) + regs;

	/* Mask for specifiled bits,the corresponding bits will be clear */
	mask = ~((0xffffffff << offset) & (0xffffffff >> (32 - offset - bits)));

	/* Make sure val in the available range */
	val &= ~(0xffffffff << bits);

	/* Get register's original val */
	dat = readl(addr);

	/* Clear specified bits */
	dat &= mask;

	/* Fill specified bits */
	dat |= val << offset;

	writel(dat, addr);
}

int rk_mipi_dsi_enable(struct udevice *dev,
		       const struct display_timing *timing)
{
	int node, timing_node;
	int val;
	struct rk_mipi_priv *priv = dev_get_priv(dev);
	uintptr_t regs = priv->regs;
	u32 txbyte_clk = priv->txbyte_clk;
	u32 txesc_clk = priv->txesc_clk;

	txesc_clk = txbyte_clk/(txbyte_clk/txesc_clk + 1);

	/* Set Display timing parameter */
	rk_mipi_dsi_write(regs, VID_HSA_TIME, timing->hsync_len.typ);
	rk_mipi_dsi_write(regs, VID_HBP_TIME, timing->hback_porch.typ);
	rk_mipi_dsi_write(regs, VID_HLINE_TIME, (timing->hsync_len.typ
			  + timing->hback_porch.typ + timing->hactive.typ
			  + timing->hfront_porch.typ));
	rk_mipi_dsi_write(regs, VID_VSA_LINES, timing->vsync_len.typ);
	rk_mipi_dsi_write(regs, VID_VBP_LINES, timing->vback_porch.typ);
	rk_mipi_dsi_write(regs, VID_VFP_LINES, timing->vfront_porch.typ);
	rk_mipi_dsi_write(regs, VID_ACTIVE_LINES, timing->vactive.typ);

	/* Set Signal Polarity */
	val = (timing->flags & DISPLAY_FLAGS_HSYNC_LOW) ? 1 : 0;
	rk_mipi_dsi_write(regs, HSYNC_ACTIVE_LOW, val);

	val = (timing->flags & DISPLAY_FLAGS_VSYNC_LOW) ? 1 : 0;
	rk_mipi_dsi_write(regs, VSYNC_ACTIVE_LOW, val);

	val = (timing->flags & DISPLAY_FLAGS_DE_LOW) ? 1 : 0;
	rk_mipi_dsi_write(regs, DATAEN_ACTIVE_LOW, val);

	val = (timing->flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE) ? 1 : 0;
	rk_mipi_dsi_write(regs, COLORM_ACTIVE_LOW, val);

	/* Set video mode */
	rk_mipi_dsi_write(regs, CMD_VIDEO_MODE, VIDEO_MODE);

	/* Set video mode transmission type as burst mode */
	rk_mipi_dsi_write(regs, VID_MODE_TYPE, BURST_MODE);

	/* Set pix num in a video package */
	rk_mipi_dsi_write(regs, VID_PKT_SIZE, 0x4b0);

	/* Set dpi color coding depth 24 bit */
	timing_node = fdt_subnode_offset(gd->fdt_blob, dev_of_offset(dev),
									 "display-timings");
	node = fdt_first_subnode(gd->fdt_blob, timing_node);
	val = fdtdec_get_int(gd->fdt_blob, node, "bits-per-pixel", -1);
	switch (val) {
	case 16:
		rk_mipi_dsi_write(regs, DPI_COLOR_CODING, DPI_16BIT_CFG_1);
		break;
	case 24:
		rk_mipi_dsi_write(regs, DPI_COLOR_CODING, DPI_24BIT);
		break;
	case 30:
		rk_mipi_dsi_write(regs, DPI_COLOR_CODING, DPI_30BIT);
		break;
	default:
		rk_mipi_dsi_write(regs, DPI_COLOR_CODING, DPI_24BIT);
	}
	/* Enable low power mode */
	rk_mipi_dsi_write(regs, LP_CMD_EN, 1);
	rk_mipi_dsi_write(regs, LP_HFP_EN, 1);
	rk_mipi_dsi_write(regs, LP_VACT_EN, 1);
	rk_mipi_dsi_write(regs, LP_VFP_EN, 1);
	rk_mipi_dsi_write(regs, LP_VBP_EN, 1);
	rk_mipi_dsi_write(regs, LP_VSA_EN, 1);

	/* Division for timeout counter clk */
	rk_mipi_dsi_write(regs, TO_CLK_DIVISION, 0x0a);

	/* Tx esc clk division from txbyte clk */
	rk_mipi_dsi_write(regs, TX_ESC_CLK_DIVISION, txbyte_clk/txesc_clk);

	/* Timeout count for hs<->lp transation between Line period */
	rk_mipi_dsi_write(regs, HSTX_TO_CNT, 0x3e8);

	/* Phy State transfer timing */
	rk_mipi_dsi_write(regs, PHY_STOP_WAIT_TIME, 32);
	rk_mipi_dsi_write(regs, PHY_TXREQUESTCLKHS, 1);
	rk_mipi_dsi_write(regs, PHY_HS2LP_TIME, 0x14);
	rk_mipi_dsi_write(regs, PHY_LP2HS_TIME, 0x10);
	rk_mipi_dsi_write(regs, MAX_RD_TIME, 0x2710);

	/* Power on */
	rk_mipi_dsi_write(regs, SHUTDOWNZ, 1);

	return 0;
}

/* rk mipi dphy write function. It is used to write test data to dphy */
static void rk_mipi_phy_write(uintptr_t regs, unsigned char test_code,
			      unsigned char *test_data, unsigned char size)
{
	int i = 0;

	/* Write Test code */
	rk_mipi_dsi_write(regs, PHY_TESTCLK, 1);
	rk_mipi_dsi_write(regs, PHY_TESTDIN, test_code);
	rk_mipi_dsi_write(regs, PHY_TESTEN, 1);
	rk_mipi_dsi_write(regs, PHY_TESTCLK, 0);
	rk_mipi_dsi_write(regs, PHY_TESTEN, 0);

	/* Write Test data */
	for (i = 0; i < size; i++) {
		rk_mipi_dsi_write(regs, PHY_TESTCLK, 0);
		rk_mipi_dsi_write(regs, PHY_TESTDIN, test_data[i]);
		rk_mipi_dsi_write(regs, PHY_TESTCLK, 1);
	}
}

/*
 * Mipi dphy config function. Calculate the suitable prediv, feedback div,
 * fsfreqrang value ,cap ,lpf and so on according to the given pix clk rate,
 * and then enable phy.
 */
int rk_mipi_phy_enable(struct udevice *dev)
{
	int i;
	struct rk_mipi_priv *priv = dev_get_priv(dev);
	uintptr_t regs = priv->regs;
	u64 fbdiv;
	u64 prediv = 1;
	u32 max_fbdiv = 512;
	u32 max_prediv, min_prediv;
	u64 ddr_clk = priv->phy_clk;
	u32 refclk = priv->ref_clk;
	u32 remain = refclk;
	unsigned char test_data[2] = {0};

	int freq_rang[][2] = {
		{90, 0x01},   {100, 0x10},  {110, 0x20},  {130, 0x01},
		{140, 0x11},  {150, 0x21},  {170, 0x02},  {180, 0x12},
		{200, 0x22},  {220, 0x03},  {240, 0x13},  {250, 0x23},
		{270, 0x04},  {300, 0x14},  {330, 0x05},  {360, 0x15},
		{400, 0x25},  {450, 0x06},  {500, 0x16},  {550, 0x07},
		{600, 0x17},  {650, 0x08},  {700, 0x18},  {750, 0x09},
		{800, 0x19},  {850, 0x29},  {900, 0x39},  {950, 0x0a},
		{1000, 0x1a}, {1050, 0x2a}, {1100, 0x3a}, {1150, 0x0b},
		{1200, 0x1b}, {1250, 0x2b}, {1300, 0x3b}, {1350, 0x0c},
		{1400, 0x1c}, {1450, 0x2c}, {1500, 0x3c}
	};

	/* Shutdown mode */
	rk_mipi_dsi_write(regs, PHY_SHUTDOWNZ, 0);
	rk_mipi_dsi_write(regs, PHY_RSTZ, 0);
	rk_mipi_dsi_write(regs, PHY_TESTCLR, 1);

	/* Pll locking */
	rk_mipi_dsi_write(regs, PHY_TESTCLR, 0);

	/* config cp and lfp */
	test_data[0] = 0x80 | (ddr_clk / (200 * MHz)) << 3 | 0x3;
	rk_mipi_phy_write(regs, CODE_PLL_VCORANGE_VCOCAP, test_data, 1);

	test_data[0] = 0x8;
	rk_mipi_phy_write(regs, CODE_PLL_CPCTRL, test_data, 1);

	test_data[0] = 0x80 | 0x40;
	rk_mipi_phy_write(regs, CODE_PLL_LPF_CP, test_data, 1);

	/* select the suitable value for fsfreqrang reg */
	for (i = 0; i < ARRAY_SIZE(freq_rang); i++) {
		if (ddr_clk / (MHz) <= freq_rang[i][0])
			break;
	}
	if (i == ARRAY_SIZE(freq_rang)) {
		debug("%s: Dphy freq out of range!\n", __func__);
		return -EINVAL;
	}
	test_data[0] = freq_rang[i][1] << 1;
	rk_mipi_phy_write(regs, CODE_HS_RX_LANE0, test_data, 1);

	/*
	 * Calculate the best ddrclk and it's corresponding div value. If the
	 * given pixelclock is great than 250M, ddrclk will be fix 1500M.
	 * Otherwise,
	 * it's equal to ddr_clk= pixclk * 6. 40MHz >= refclk / prediv >= 5MHz
	 * according to spec.
	 */
	max_prediv = (refclk / (5 * MHz));
	min_prediv = ((refclk / (40 * MHz)) ? (refclk / (40 * MHz) + 1) : 1);

	debug("%s: DEBUG: max_prediv=%u, min_prediv=%u\n", __func__, max_prediv,
	      min_prediv);

	if (max_prediv < min_prediv) {
		debug("%s: Invalid refclk value\n", __func__);
		return -EINVAL;
	}

	/* Calculate the best refclk and feedback division value for dphy pll */
	for (i = min_prediv; i < max_prediv; i++) {
		if ((ddr_clk * i % refclk < remain) &&
		    (ddr_clk * i / refclk) < max_fbdiv) {
			prediv = i;
			remain = ddr_clk * i % refclk;
		}
	}
	fbdiv = ddr_clk * prediv / refclk;
	ddr_clk = refclk * fbdiv / prediv;
	priv->phy_clk = ddr_clk;

	debug("%s: DEBUG: refclk=%u, refclk=%llu, fbdiv=%llu, phyclk=%llu\n",
	      __func__, refclk, prediv, fbdiv, ddr_clk);

	/* config prediv and feedback reg */
	test_data[0] = prediv - 1;
	rk_mipi_phy_write(regs, CODE_PLL_INPUT_DIV_RAT, test_data, 1);
	test_data[0] = (fbdiv - 1) & 0x1f;
	rk_mipi_phy_write(regs, CODE_PLL_LOOP_DIV_RAT, test_data, 1);
	test_data[0] = (fbdiv - 1) >> 5 | 0x80;
	rk_mipi_phy_write(regs, CODE_PLL_LOOP_DIV_RAT, test_data, 1);
	test_data[0] = 0x30;
	rk_mipi_phy_write(regs, CODE_PLL_INPUT_LOOP_DIV_RAT, test_data, 1);

	/* rest config */
	test_data[0] = 0x4d;
	rk_mipi_phy_write(regs, CODE_BANDGAP_BIAS_CTRL, test_data, 1);

	test_data[0] = 0x3d;
	rk_mipi_phy_write(regs, CODE_TERMINATION_CTRL, test_data, 1);

	test_data[0] = 0xdf;
	rk_mipi_phy_write(regs, CODE_TERMINATION_CTRL, test_data, 1);

	test_data[0] =  0x7;
	rk_mipi_phy_write(regs, CODE_AFE_BIAS_BANDGAP_ANOLOG, test_data, 1);

	test_data[0] = 0x80 | 0x7;
	rk_mipi_phy_write(regs, CODE_AFE_BIAS_BANDGAP_ANOLOG, test_data, 1);

	test_data[0] = 0x80 | 15;
	rk_mipi_phy_write(regs, CODE_HSTXDATALANEREQUSETSTATETIME,
			  test_data, 1);
	test_data[0] = 0x80 | 85;
	rk_mipi_phy_write(regs, CODE_HSTXDATALANEPREPARESTATETIME,
			  test_data, 1);
	test_data[0] = 0x40 | 10;
	rk_mipi_phy_write(regs, CODE_HSTXDATALANEHSZEROSTATETIME,
			  test_data, 1);

	/* enter into stop mode */
	rk_mipi_dsi_write(regs, N_LANES, 0x03);
	rk_mipi_dsi_write(regs, PHY_ENABLECLK, 1);
	rk_mipi_dsi_write(regs, PHY_FORCEPLL, 1);
	rk_mipi_dsi_write(regs, PHY_SHUTDOWNZ, 1);
	rk_mipi_dsi_write(regs, PHY_RSTZ, 1);

	return 0;
}

