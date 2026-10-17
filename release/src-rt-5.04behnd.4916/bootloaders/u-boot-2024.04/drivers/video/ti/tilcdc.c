// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <log.h>
#include <panel.h>
#include <video.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/utils.h>
#include "tilcdc.h"
#include "tilcdc-panel.h"

#define LCDC_FMAX				200000000

/* LCD Control Register */
#define LCDC_CTRL_CLK_DIVISOR_MASK		GENMASK(15, 8)
#define LCDC_CTRL_RASTER_MODE			BIT(0)
#define LCDC_CTRL_CLK_DIVISOR(x)		(((x) & GENMASK(7, 0)) << 8)
/* LCD Clock Enable Register */
#define LCDC_CLKC_ENABLE_CORECLKEN		BIT(0)
#define LCDC_CLKC_ENABLE_LIDDCLKEN		BIT(1)
#define LCDC_CLKC_ENABLE_DMACLKEN		BIT(2)
/* LCD DMA Control Register */
#define LCDC_DMA_CTRL_BURST_SIZE(x)		(((x) & GENMASK(2, 0)) << 4)
#define LCDC_DMA_CTRL_BURST_1			0x0
#define LCDC_DMA_CTRL_BURST_2			0x1
#define LCDC_DMA_CTRL_BURST_4			0x2
#define LCDC_DMA_CTRL_BURST_8			0x3
#define LCDC_DMA_CTRL_BURST_16			0x4
#define LCDC_DMA_CTRL_FIFO_TH(x)		(((x) & GENMASK(2, 0)) << 8)
/* LCD Timing_0 Register */
#define LCDC_RASTER_TIMING_0_HORMSB(x)	((((x) - 1) & BIT(10)) >> 7)
#define LCDC_RASTER_TIMING_0_HORLSB(x) (((((x) >> 4) - 1) & GENMASK(5, 0)) << 4)
#define LCDC_RASTER_TIMING_0_HSWLSB(x)	((((x) - 1) & GENMASK(5, 0)) << 10)
#define LCDC_RASTER_TIMING_0_HFPLSB(x)	((((x) - 1) & GENMASK(7, 0)) << 16)
#define LCDC_RASTER_TIMING_0_HBPLSB(x)	((((x) - 1) & GENMASK(7, 0)) << 24)
/* LCD Timing_1 Register */
#define LCDC_RASTER_TIMING_1_VERLSB(x)		(((x) - 1) & GENMASK(9, 0))
#define LCDC_RASTER_TIMING_1_VSW(x)	((((x) - 1) & GENMASK(5, 0)) << 10)
#define LCDC_RASTER_TIMING_1_VFP(x)		(((x) & GENMASK(7, 0)) << 16)
#define LCDC_RASTER_TIMING_1_VBP(x)		(((x) & GENMASK(7, 0)) << 24)
/* LCD Timing_2 Register */
#define LCDC_RASTER_TIMING_2_HFPMSB(x)	((((x) - 1) & GENMASK(9, 8)) >> 8)
#define LCDC_RASTER_TIMING_2_HBPMSB(x)	((((x) - 1) & GENMASK(9, 8)) >> 4)
#define LCDC_RASTER_TIMING_2_ACB(x)		(((x) & GENMASK(7, 0)) << 8)
#define LCDC_RASTER_TIMING_2_ACBI(x)		(((x) & GENMASK(3, 0)) << 16)
#define LCDC_RASTER_TIMING_2_VSYNC_INVERT	BIT(20)
#define LCDC_RASTER_TIMING_2_HSYNC_INVERT	BIT(21)
#define LCDC_RASTER_TIMING_2_PXCLK_INVERT	BIT(22)
#define LCDC_RASTER_TIMING_2_DE_INVERT		BIT(23)
#define LCDC_RASTER_TIMING_2_HSVS_RISEFALL	BIT(24)
#define LCDC_RASTER_TIMING_2_HSVS_CONTROL	BIT(25)
#define LCDC_RASTER_TIMING_2_VERMSB(x)		((((x) - 1) & BIT(10)) << 16)
#define LCDC_RASTER_TIMING_2_HSWMSB(x)	((((x) - 1) & GENMASK(9, 6)) << 21)
/* LCD Raster Ctrl Register */
#define LCDC_RASTER_CTRL_ENABLE			BIT(0)
#define LCDC_RASTER_CTRL_TFT_MODE		BIT(7)
#define LCDC_RASTER_CTRL_DATA_ORDER		BIT(8)
#define LCDC_RASTER_CTRL_REQDLY(x)		(((x) & GENMASK(7, 0)) << 12)
#define LCDC_RASTER_CTRL_PALMODE_RAWDATA	(0x02 << 20)
#define LCDC_RASTER_CTRL_TFT_ALT_ENABLE		BIT(23)
#define LCDC_RASTER_CTRL_TFT_24BPP_MODE		BIT(25)
#define LCDC_RASTER_CTRL_TFT_24BPP_UNPACK	BIT(26)

enum {
	LCDC_MAX_WIDTH = 2048,
	LCDC_MAX_HEIGHT = 2048,
	LCDC_MAX_LOG2_BPP = VIDEO_BPP32,
};

struct tilcdc_regs {
	u32 pid;
	u32 ctrl;
	u32 gap0;
	u32 lidd_ctrl;
	u32 lidd_cs0_conf;
	u32 lidd_cs0_addr;
	u32 lidd_cs0_data;
	u32 lidd_cs1_conf;
	u32 lidd_cs1_addr;
	u32 lidd_cs1_data;
	u32 raster_ctrl;
	u32 raster_timing0;
	u32 raster_timing1;
	u32 raster_timing2;
	u32 raster_subpanel;
	u32 raster_subpanel2;
	u32 lcddma_ctrl;
	u32 lcddma_fb0_base;
	u32 lcddma_fb0_ceiling;
	u32 lcddma_fb1_base;
	u32 lcddma_fb1_ceiling;
	u32 sysconfig;
	u32 irqstatus_raw;
	u32 irqstatus;
	u32 irqenable_set;
	u32 irqenable_clear;
	u32 gap1;
	u32 clkc_enable;
	u32 clkc_reset;
};

struct tilcdc_priv {
	struct tilcdc_regs *regs;
	struct clk gclk;
	struct clk dpll_m2_clk;
};

DECLARE_GLOBAL_DATA_PTR;

static ulong tilcdc_set_pixel_clk_rate(struct udevice *dev, ulong rate)
{
	struct tilcdc_priv *priv = dev_get_priv(dev);
	struct tilcdc_regs *regs = priv->regs;
	ulong mult_rate, mult_round_rate, best_err, err;
	u32 v;
	int div, i;

	best_err = rate;
	div = 0;
	for (i = 2; i <= 255; i++) {
		mult_rate = rate * i;
		mult_round_rate = clk_round_rate(&priv->gclk, mult_rate);
		if (IS_ERR_VALUE(mult_round_rate))
			return mult_round_rate;

		err = mult_rate - mult_round_rate;
		if (err < best_err) {
			best_err = err;
			div = i;
			if (err == 0)
				break;
		}
	}

	if (div == 0) {
		dev_err(dev, "failed to find a divisor\n");
		return -EFAULT;
	}

	mult_rate = clk_set_rate(&priv->gclk, rate * div);
	v = readl(&regs->ctrl) & ~LCDC_CTRL_CLK_DIVISOR_MASK;
	v |= LCDC_CTRL_CLK_DIVISOR(div);
	writel(v, &regs->ctrl);
	rate = mult_rate / div;
	dev_dbg(dev, "rate=%ld, div=%d, err=%ld\n", rate, div, err);
	return rate;
}

static int tilcdc_remove(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct tilcdc_priv *priv = dev_get_priv(dev);

	uc_plat->base -= 0x20;
	uc_plat->size += 0x20;
	clk_release_all(&priv->gclk, 1);
	clk_release_all(&priv->dpll_m2_clk, 1);
	return 0;
}

static int tilcdc_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tilcdc_priv *priv = dev_get_priv(dev);
	struct tilcdc_regs *regs = priv->regs;
	struct udevice *panel, *clk_dev;
	struct tilcdc_panel_info info;
	struct display_timing timing;
	ulong rate;
	u32 reg;
	int err;

	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	err = uclass_get_device(UCLASS_PANEL, 0, &panel);
	if (err) {
		dev_err(dev, "failed to get panel\n");
		return err;
	}

	err = panel_get_display_timing(panel, &timing);
	if (err) {
		dev_err(dev, "failed to get display timing\n");
		return err;
	}

	if (timing.pixelclock.typ > (LCDC_FMAX / 2)) {
		dev_err(dev, "invalid display clock-frequency: %d Hz\n",
			timing.pixelclock.typ);
		return -EINVAL;
	}

	if (timing.hactive.typ > LCDC_MAX_WIDTH)
		timing.hactive.typ = LCDC_MAX_WIDTH;

	if (timing.vactive.typ > LCDC_MAX_HEIGHT)
		timing.vactive.typ = LCDC_MAX_HEIGHT;

	err = tilcdc_panel_get_display_info(panel, &info);
	if (err) {
		dev_err(dev, "failed to get panel info\n");
		return err;
	}

	switch (info.bpp) {
	case 16:
	case 24:
	case 32:
		break;
	default:
		dev_err(dev, "invalid seting, bpp: %d\n", info.bpp);
		return -EINVAL;
	}

	switch (info.dma_burst_sz) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
		break;
	default:
		dev_err(dev, "invalid setting, dma-burst-sz: %d\n",
			info.dma_burst_sz);
		return -EINVAL;
	}

	err = uclass_get_device_by_name(UCLASS_CLK, "lcd_gclk@534", &clk_dev);
	if (err) {
		dev_err(dev, "failed to get lcd_gclk device\n");
		return err;
	}

	err = clk_request(clk_dev, &priv->gclk);
	if (err) {
		dev_err(dev, "failed to get %s clock\n", clk_dev->name);
		return err;
	}

	rate = tilcdc_set_pixel_clk_rate(dev, timing.pixelclock.typ);
	if (IS_ERR_VALUE(rate)) {
		dev_err(dev, "failed to set pixel clock rate\n");
		return rate;
	}

	err = uclass_get_device_by_name(UCLASS_CLK, "dpll_disp_m2_ck@4a4",
					&clk_dev);
	if (err) {
		dev_err(dev, "failed to get dpll_disp_m2 clock device\n");
		return err;
	}

	err = clk_request(clk_dev, &priv->dpll_m2_clk);
	if (err) {
		dev_err(dev, "failed to get %s clock\n", clk_dev->name);
		return err;
	}

	err = clk_set_parent(&priv->gclk, &priv->dpll_m2_clk);
	if (err) {
		dev_err(dev, "failed to set %s clock as %s's parent\n",
			priv->dpll_m2_clk.dev->name, priv->gclk.dev->name);
		return err;
	}

	/* palette default entry */
	memset((void *)uc_plat->base, 0, 0x20);
	*(unsigned int *)uc_plat->base = 0x4000;
	/* point fb behind palette */
	uc_plat->base += 0x20;
	uc_plat->size -= 0x20;

	writel(LCDC_CLKC_ENABLE_CORECLKEN | LCDC_CLKC_ENABLE_LIDDCLKEN |
	       LCDC_CLKC_ENABLE_DMACLKEN, &regs->clkc_enable);
	writel(0, &regs->raster_ctrl);

	reg = readl(&regs->ctrl) & LCDC_CTRL_CLK_DIVISOR_MASK;
	reg |= LCDC_CTRL_RASTER_MODE;
	writel(reg, &regs->ctrl);

	reg = (timing.hactive.typ * timing.vactive.typ * info.bpp) >> 3;
	reg += uc_plat->base;
	writel(uc_plat->base, &regs->lcddma_fb0_base);
	writel(reg, &regs->lcddma_fb0_ceiling);
	writel(uc_plat->base, &regs->lcddma_fb1_base);
	writel(reg, &regs->lcddma_fb1_ceiling);

	reg = LCDC_DMA_CTRL_FIFO_TH(info.fifo_th);
	switch (info.dma_burst_sz) {
	case 1:
		reg |= LCDC_DMA_CTRL_BURST_SIZE(LCDC_DMA_CTRL_BURST_1);
		break;
	case 2:
		reg |= LCDC_DMA_CTRL_BURST_SIZE(LCDC_DMA_CTRL_BURST_2);
		break;
	case 4:
		reg |= LCDC_DMA_CTRL_BURST_SIZE(LCDC_DMA_CTRL_BURST_4);
		break;
	case 8:
		reg |= LCDC_DMA_CTRL_BURST_SIZE(LCDC_DMA_CTRL_BURST_8);
		break;
	case 16:
		reg |= LCDC_DMA_CTRL_BURST_SIZE(LCDC_DMA_CTRL_BURST_16);
		break;
	}

	writel(reg, &regs->lcddma_ctrl);

	writel(LCDC_RASTER_TIMING_0_HORLSB(timing.hactive.typ) |
	       LCDC_RASTER_TIMING_0_HORMSB(timing.hactive.typ) |
	       LCDC_RASTER_TIMING_0_HFPLSB(timing.hfront_porch.typ) |
	       LCDC_RASTER_TIMING_0_HBPLSB(timing.hback_porch.typ) |
	       LCDC_RASTER_TIMING_0_HSWLSB(timing.hsync_len.typ),
	       &regs->raster_timing0);

	writel(LCDC_RASTER_TIMING_1_VBP(timing.vback_porch.typ) |
	       LCDC_RASTER_TIMING_1_VFP(timing.vfront_porch.typ) |
	       LCDC_RASTER_TIMING_1_VSW(timing.vsync_len.typ) |
	       LCDC_RASTER_TIMING_1_VERLSB(timing.vactive.typ),
	       &regs->raster_timing1);

	reg = LCDC_RASTER_TIMING_2_ACB(info.ac_bias) |
		LCDC_RASTER_TIMING_2_ACBI(info.ac_bias_intrpt) |
		LCDC_RASTER_TIMING_2_HSWMSB(timing.hsync_len.typ) |
		LCDC_RASTER_TIMING_2_VERMSB(timing.vactive.typ) |
		LCDC_RASTER_TIMING_2_HBPMSB(timing.hback_porch.typ) |
		LCDC_RASTER_TIMING_2_HFPMSB(timing.hfront_porch.typ);

	if (timing.flags & DISPLAY_FLAGS_VSYNC_LOW)
		reg |= LCDC_RASTER_TIMING_2_VSYNC_INVERT;

	if (timing.flags & DISPLAY_FLAGS_HSYNC_LOW)
		reg |= LCDC_RASTER_TIMING_2_HSYNC_INVERT;

	if (info.invert_pxl_clk)
		reg |= LCDC_RASTER_TIMING_2_PXCLK_INVERT;

	if (info.sync_edge)
		reg |= LCDC_RASTER_TIMING_2_HSVS_RISEFALL;

	if (info.sync_ctrl)
		reg |= LCDC_RASTER_TIMING_2_HSVS_CONTROL;

	writel(reg, &regs->raster_timing2);

	reg = LCDC_RASTER_CTRL_PALMODE_RAWDATA | LCDC_RASTER_CTRL_TFT_MODE |
		LCDC_RASTER_CTRL_ENABLE | LCDC_RASTER_CTRL_REQDLY(info.fdd);

	if (info.tft_alt_mode)
		reg |= LCDC_RASTER_CTRL_TFT_ALT_ENABLE;

	if (info.bpp == 24)
		reg |= LCDC_RASTER_CTRL_TFT_24BPP_MODE;
	else if (info.bpp == 32)
		reg |= LCDC_RASTER_CTRL_TFT_24BPP_MODE |
			LCDC_RASTER_CTRL_TFT_24BPP_UNPACK;

	if (info.raster_order)
		reg |= LCDC_RASTER_CTRL_DATA_ORDER;

	writel(reg, &regs->raster_ctrl);

	uc_priv->xsize = timing.hactive.typ;
	uc_priv->ysize = timing.vactive.typ;
	uc_priv->bpix = log_2_n_round_up(info.bpp);

	err = panel_enable_backlight(panel);
	if (err) {
		dev_err(dev, "failed to enable panel backlight\n");
		return err;
	}

	return 0;
}

static int tilcdc_of_to_plat(struct udevice *dev)
{
	struct tilcdc_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs) {
		dev_err(dev, "failed to get base address\n");
		return -EINVAL;
	}

	dev_dbg(dev, "LCD: base address=0x%x\n", (unsigned int)priv->regs);
	return 0;
}

static int tilcdc_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->size = ((LCDC_MAX_WIDTH * LCDC_MAX_HEIGHT *
			  (1 << LCDC_MAX_LOG2_BPP)) >> 3) + 0x20;

	dev_dbg(dev, "frame buffer size 0x%x\n", uc_plat->size);
	return 0;
}

static const struct udevice_id tilcdc_ids[] = {
	{.compatible = "ti,am33xx-tilcdc"},
	{}
};

U_BOOT_DRIVER(tilcdc) = {
	.name = "tilcdc",
	.id = UCLASS_VIDEO,
	.of_match = tilcdc_ids,
	.bind = tilcdc_bind,
	.of_to_plat = tilcdc_of_to_plat,
	.probe = tilcdc_probe,
	.remove = tilcdc_remove,
	.priv_auto = sizeof(struct tilcdc_priv)
};
