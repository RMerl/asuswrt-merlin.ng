// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 STMicroelectronics - All Rights Reserved
 * Author(s): Philippe Cornu <philippe.cornu@st.com> for STMicroelectronics.
 *	      Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 *
 * This MIPI DSI controller driver is based on the Linux Kernel driver from
 * drivers/gpu/drm/stm/dw_mipi_dsi-stm.c.
 */

#define LOG_CATEGORY UCLASS_VIDEO_BRIDGE

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dsi_host.h>
#include <log.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <reset.h>
#include <video.h>
#include <video_bridge.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/printk.h>
#include <power/regulator.h>

#define HWVER_130			0x31333000	/* IP version 1.30 */
#define HWVER_131			0x31333100	/* IP version 1.31 */

/* DSI digital registers & bit definitions */
#define DSI_VERSION			0x00
#define VERSION				GENMASK(31, 8)

/*
 * DSI wrapper registers & bit definitions
 * Note: registers are named as in the Reference Manual
 */
#define DSI_WCFGR	0x0400		/* Wrapper ConFiGuration Reg */
#define WCFGR_DSIM	BIT(0)		/* DSI Mode */
#define WCFGR_COLMUX	GENMASK(3, 1)	/* COLor MUltipleXing */

#define DSI_WCR		0x0404		/* Wrapper Control Reg */
#define WCR_DSIEN	BIT(3)		/* DSI ENable */

#define DSI_WISR	0x040C		/* Wrapper Interrupt and Status Reg */
#define WISR_PLLLS	BIT(8)		/* PLL Lock Status */
#define WISR_RRS	BIT(12)		/* Regulator Ready Status */

#define DSI_WPCR0	0x0418		/* Wrapper Phy Conf Reg 0 */
#define WPCR0_UIX4	GENMASK(5, 0)	/* Unit Interval X 4 */
#define WPCR0_TDDL	BIT(16)		/* Turn Disable Data Lanes */

#define DSI_WRPCR	0x0430		/* Wrapper Regulator & Pll Ctrl Reg */
#define WRPCR_PLLEN	BIT(0)		/* PLL ENable */
#define WRPCR_NDIV	GENMASK(8, 2)	/* pll loop DIVision Factor */
#define WRPCR_IDF	GENMASK(14, 11)	/* pll Input Division Factor */
#define WRPCR_ODF	GENMASK(17, 16)	/* pll Output Division Factor */
#define WRPCR_REGEN	BIT(24)		/* REGulator ENable */
#define WRPCR_BGREN	BIT(28)		/* BandGap Reference ENable */
#define IDF_MIN		1
#define IDF_MAX		7
#define NDIV_MIN	10
#define NDIV_MAX	125
#define ODF_MIN		1
#define ODF_MAX		8

/* dsi color format coding according to the datasheet */
enum dsi_color {
	DSI_RGB565_CONF1,
	DSI_RGB565_CONF2,
	DSI_RGB565_CONF3,
	DSI_RGB666_CONF1,
	DSI_RGB666_CONF2,
	DSI_RGB888,
};

#define LANE_MIN_KBPS	31250
#define LANE_MAX_KBPS	500000

/* Timeout for regulator on/off, pll lock/unlock & fifo empty */
#define TIMEOUT_US	200000

struct stm32_dsi_priv {
	struct mipi_dsi_device device;
	void __iomem *base;
	struct udevice *panel;
	u32 pllref_clk;
	u32 hw_version;
	int lane_min_kbps;
	int lane_max_kbps;
	struct udevice *vdd_reg;
	struct udevice *dsi_host;
};

static inline void dsi_write(struct stm32_dsi_priv *dsi, u32 reg, u32 val)
{
	writel(val, dsi->base + reg);
}

static inline u32 dsi_read(struct stm32_dsi_priv *dsi, u32 reg)
{
	return readl(dsi->base + reg);
}

static inline void dsi_set(struct stm32_dsi_priv *dsi, u32 reg, u32 mask)
{
	dsi_write(dsi, reg, dsi_read(dsi, reg) | mask);
}

static inline void dsi_clear(struct stm32_dsi_priv *dsi, u32 reg, u32 mask)
{
	dsi_write(dsi, reg, dsi_read(dsi, reg) & ~mask);
}

static inline void dsi_update_bits(struct stm32_dsi_priv *dsi, u32 reg,
				   u32 mask, u32 val)
{
	dsi_write(dsi, reg, (dsi_read(dsi, reg) & ~mask) | val);
}

static enum dsi_color dsi_color_from_mipi(u32 fmt)
{
	switch (fmt) {
	case MIPI_DSI_FMT_RGB888:
		return DSI_RGB888;
	case MIPI_DSI_FMT_RGB666:
		return DSI_RGB666_CONF2;
	case MIPI_DSI_FMT_RGB666_PACKED:
		return DSI_RGB666_CONF1;
	case MIPI_DSI_FMT_RGB565:
		return DSI_RGB565_CONF1;
	default:
		log_err("MIPI color invalid, so we use rgb888\n");
	}
	return DSI_RGB888;
}

static int dsi_pll_get_clkout_khz(int clkin_khz, int idf, int ndiv, int odf)
{
	int divisor = idf * odf;

	/* prevent from division by 0 */
	if (!divisor)
		return 0;

	return DIV_ROUND_CLOSEST(clkin_khz * ndiv, divisor);
}

static int dsi_pll_get_params(struct stm32_dsi_priv *dsi,
			      int clkin_khz, int clkout_khz,
			      int *idf, int *ndiv, int *odf)
{
	int i, o, n, n_min, n_max;
	int fvco_min, fvco_max, delta, best_delta; /* all in khz */

	/* Early checks preventing division by 0 & odd results */
	if (clkin_khz <= 0 || clkout_khz <= 0)
		return -EINVAL;

	fvco_min = dsi->lane_min_kbps * 2 * ODF_MAX;
	fvco_max = dsi->lane_max_kbps * 2 * ODF_MIN;

	best_delta = 1000000; /* big started value (1000000khz) */

	for (i = IDF_MIN; i <= IDF_MAX; i++) {
		/* Compute ndiv range according to Fvco */
		n_min = ((fvco_min * i) / (2 * clkin_khz)) + 1;
		n_max = (fvco_max * i) / (2 * clkin_khz);

		/* No need to continue idf loop if we reach ndiv max */
		if (n_min >= NDIV_MAX)
			break;

		/* Clamp ndiv to valid values */
		if (n_min < NDIV_MIN)
			n_min = NDIV_MIN;
		if (n_max > NDIV_MAX)
			n_max = NDIV_MAX;

		for (o = ODF_MIN; o <= ODF_MAX; o *= 2) {
			n = DIV_ROUND_CLOSEST(i * o * clkout_khz, clkin_khz);
			/* Check ndiv according to vco range */
			if (n < n_min || n > n_max)
				continue;
			/* Check if new delta is better & saves parameters */
			delta = dsi_pll_get_clkout_khz(clkin_khz, i, n, o) -
				clkout_khz;
			if (delta < 0)
				delta = -delta;
			if (delta < best_delta) {
				*idf = i;
				*ndiv = n;
				*odf = o;
				best_delta = delta;
			}
			/* fast return in case of "perfect result" */
			if (!delta)
				return 0;
		}
	}

	return 0;
}

static int dsi_phy_init(void *priv_data)
{
	struct mipi_dsi_device *device = priv_data;
	struct udevice *dev = device->dev;
	struct stm32_dsi_priv *dsi = dev_get_priv(dev);
	u32 val;
	int ret;

	dev_dbg(dev, "Initialize DSI physical layer\n");

	/* Enable the regulator */
	dsi_set(dsi, DSI_WRPCR, WRPCR_REGEN | WRPCR_BGREN);
	ret = readl_poll_timeout(dsi->base + DSI_WISR, val, val & WISR_RRS,
				 TIMEOUT_US);
	if (ret) {
		dev_dbg(dev, "!TIMEOUT! waiting REGU\n");
		return ret;
	}

	/* Enable the DSI PLL & wait for its lock */
	dsi_set(dsi, DSI_WRPCR, WRPCR_PLLEN);
	ret = readl_poll_timeout(dsi->base + DSI_WISR, val, val & WISR_PLLLS,
				 TIMEOUT_US);
	if (ret) {
		dev_dbg(dev, "!TIMEOUT! waiting PLL\n");
		return ret;
	}

	return 0;
}

static void dsi_phy_post_set_mode(void *priv_data, unsigned long mode_flags)
{
	struct mipi_dsi_device *device = priv_data;
	struct udevice *dev = device->dev;
	struct stm32_dsi_priv *dsi = dev_get_priv(dev);

	dev_dbg(dev, "Set mode %p enable %ld\n", dsi,
		mode_flags & MIPI_DSI_MODE_VIDEO);

	if (!dsi)
		return;

	/*
	 * DSI wrapper must be enabled in video mode & disabled in command mode.
	 * If wrapper is enabled in command mode, the display controller
	 * register access will hang.
	 */

	if (mode_flags & MIPI_DSI_MODE_VIDEO)
		dsi_set(dsi, DSI_WCR, WCR_DSIEN);
	else
		dsi_clear(dsi, DSI_WCR, WCR_DSIEN);
}

static int dsi_get_lane_mbps(void *priv_data, struct display_timing *timings,
			     u32 lanes, u32 format, unsigned int *lane_mbps)
{
	struct mipi_dsi_device *device = priv_data;
	struct udevice *dev = device->dev;
	struct stm32_dsi_priv *dsi = dev_get_priv(dev);
	int idf, ndiv, odf, pll_in_khz, pll_out_khz;
	int ret, bpp;
	u32 val;

	/* Update lane capabilities according to hw version */
	dsi->lane_min_kbps = LANE_MIN_KBPS;
	dsi->lane_max_kbps = LANE_MAX_KBPS;
	if (dsi->hw_version == HWVER_131) {
		dsi->lane_min_kbps *= 2;
		dsi->lane_max_kbps *= 2;
	}

	pll_in_khz = dsi->pllref_clk / 1000;

	/* Compute requested pll out */
	bpp = mipi_dsi_pixel_format_to_bpp(format);
	pll_out_khz = (timings->pixelclock.typ / 1000) * bpp / lanes;
	/* Add 20% to pll out to be higher than pixel bw (burst mode only) */
	pll_out_khz = (pll_out_khz * 12) / 10;
	if (pll_out_khz > dsi->lane_max_kbps) {
		pll_out_khz = dsi->lane_max_kbps;
		dev_warn(dev, "Warning max phy mbps is used\n");
	}
	if (pll_out_khz < dsi->lane_min_kbps) {
		pll_out_khz = dsi->lane_min_kbps;
		dev_warn(dev, "Warning min phy mbps is used\n");
	}

	/* Compute best pll parameters */
	idf = 0;
	ndiv = 0;
	odf = 0;
	ret = dsi_pll_get_params(dsi, pll_in_khz, pll_out_khz,
				 &idf, &ndiv, &odf);
	if (ret) {
		dev_err(dev, "Warning dsi_pll_get_params(): bad params\n");
		return ret;
	}

	/* Get the adjusted pll out value */
	pll_out_khz = dsi_pll_get_clkout_khz(pll_in_khz, idf, ndiv, odf);

	/* Set the PLL division factors */
	dsi_update_bits(dsi, DSI_WRPCR,	WRPCR_NDIV | WRPCR_IDF | WRPCR_ODF,
			(ndiv << 2) | (idf << 11) | ((ffs(odf) - 1) << 16));

	/* Compute uix4 & set the bit period in high-speed mode */
	val = 4000000 / pll_out_khz;
	dsi_update_bits(dsi, DSI_WPCR0, WPCR0_UIX4, val);

	/* Select video mode by resetting DSIM bit */
	dsi_clear(dsi, DSI_WCFGR, WCFGR_DSIM);

	/* Select the color coding */
	dsi_update_bits(dsi, DSI_WCFGR, WCFGR_COLMUX,
			dsi_color_from_mipi(format) << 1);

	*lane_mbps = pll_out_khz / 1000;

	dev_dbg(dev, "pll_in %ukHz pll_out %ukHz lane_mbps %uMHz\n",
		pll_in_khz, pll_out_khz, *lane_mbps);

	return 0;
}

static const struct mipi_dsi_phy_ops dsi_stm_phy_ops = {
	.init = dsi_phy_init,
	.get_lane_mbps = dsi_get_lane_mbps,
	.post_set_mode = dsi_phy_post_set_mode,
};

static int stm32_dsi_attach(struct udevice *dev)
{
	struct stm32_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct mipi_dsi_panel_plat *mplat;
	struct display_timing timings;
	int ret;

	ret = uclass_first_device_err(UCLASS_PANEL, &priv->panel);
	if (ret) {
		dev_err(dev, "panel device error %d\n", ret);
		return ret;
	}

	mplat = dev_get_plat(priv->panel);
	mplat->device = &priv->device;
	device->lanes = mplat->lanes;
	device->format = mplat->format;
	device->mode_flags = mplat->mode_flags;

	ret = panel_get_display_timing(priv->panel, &timings);
	if (ret) {
		ret = ofnode_decode_display_timing(dev_ofnode(priv->panel),
						   0, &timings);
		if (ret) {
			dev_err(dev, "decode display timing error %d\n", ret);
			return ret;
		}
	}

	ret = uclass_get_device(UCLASS_DSI_HOST, 0, &priv->dsi_host);
	if (ret) {
		dev_err(dev, "No video dsi host detected %d\n", ret);
		return ret;
	}

	ret = dsi_host_init(priv->dsi_host, device, &timings, 2,
			    &dsi_stm_phy_ops);
	if (ret) {
		dev_err(dev, "failed to initialize mipi dsi host\n");
		return ret;
	}

	return 0;
}

static int stm32_dsi_set_backlight(struct udevice *dev, int percent)
{
	struct stm32_dsi_priv *priv = dev_get_priv(dev);
	int ret;

	ret = panel_enable_backlight(priv->panel);
	if (ret) {
		dev_err(dev, "panel %s enable backlight error %d\n",
			priv->panel->name, ret);
		return ret;
	}

	ret = dsi_host_enable(priv->dsi_host);
	if (ret) {
		dev_err(dev, "failed to enable mipi dsi host\n");
		return ret;
	}

	return 0;
}

static int stm32_dsi_bind(struct udevice *dev)
{
	int ret;

	ret = device_bind_driver_to_node(dev, "dw_mipi_dsi", "dsihost",
					 dev_ofnode(dev), NULL);
	if (ret)
		return ret;

	return dm_scan_fdt_dev(dev);
}

static int stm32_dsi_probe(struct udevice *dev)
{
	struct stm32_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct reset_ctl rst;
	struct clk clk;
	int ret;

	device->dev = dev;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base) {
		dev_err(dev, "dsi dt register address error\n");
		return -EINVAL;
	}

	ret =  device_get_supply_regulator(dev, "phy-dsi-supply",
					   &priv->vdd_reg);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "Warning: cannot get phy dsi supply\n");
		return -ENODEV;
	}

	if (ret != -ENOENT) {
		ret = regulator_set_enable(priv->vdd_reg, true);
		if (ret)
			return ret;
	}

	ret = clk_get_by_name(device->dev, "pclk", &clk);
	if (ret) {
		dev_err(dev, "peripheral clock get error %d\n", ret);
		goto err_reg;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "peripheral clock enable error %d\n", ret);
		goto err_reg;
	}

	ret = clk_get_by_name(dev, "ref", &clk);
	if (ret) {
		dev_err(dev, "pll reference clock get error %d\n", ret);
		goto err_clk;
	}

	priv->pllref_clk = (unsigned int)clk_get_rate(&clk);

	ret = reset_get_by_index(device->dev, 0, &rst);
	if (ret) {
		dev_err(dev, "missing dsi hardware reset\n");
		goto err_clk;
	}

	/* Reset */
	reset_deassert(&rst);

	/* check hardware version */
	priv->hw_version = dsi_read(priv, DSI_VERSION) & VERSION;
	if (priv->hw_version != HWVER_130 &&
	    priv->hw_version != HWVER_131) {
		dev_err(dev, "DSI version 0x%x not supported\n", priv->hw_version);
		dev_dbg(dev, "remove and unbind all DSI child\n");
		device_chld_remove(dev, NULL, DM_REMOVE_NORMAL);
		device_chld_unbind(dev, NULL);
		ret = -ENODEV;
		goto err_clk;
	}

	return 0;
err_clk:
	clk_disable(&clk);
err_reg:
	regulator_set_enable(priv->vdd_reg, false);

	return ret;
}

struct video_bridge_ops stm32_dsi_ops = {
	.attach = stm32_dsi_attach,
	.set_backlight = stm32_dsi_set_backlight,
};

static const struct udevice_id stm32_dsi_ids[] = {
	{ .compatible = "st,stm32-dsi"},
	{ }
};

U_BOOT_DRIVER(stm32_dsi) = {
	.name				= "stm32-display-dsi",
	.id				= UCLASS_VIDEO_BRIDGE,
	.of_match			= stm32_dsi_ids,
	.bind				= stm32_dsi_bind,
	.probe				= stm32_dsi_probe,
	.ops				= &stm32_dsi_ops,
	.priv_auto		= sizeof(struct stm32_dsi_priv),
};
