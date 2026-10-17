// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for AT91/AT32 LCD Controller
 *
 * Copyright (C) 2007 Atmel Corporation
 */

#include <common.h>
#include <atmel_lcd.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <part.h>
#include <video.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <bmp_layout.h>
#include <atmel_lcdc.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};

struct atmel_fb_priv {
	struct display_timing timing;
};

/* configurable parameters */
#define ATMEL_LCDC_CVAL_DEFAULT		0xc8
#define ATMEL_LCDC_DMA_BURST_LEN	8
#ifndef ATMEL_LCDC_GUARD_TIME
#define ATMEL_LCDC_GUARD_TIME		1
#endif

#if defined(CONFIG_AT91SAM9263)
#define ATMEL_LCDC_FIFO_SIZE		2048
#else
#define ATMEL_LCDC_FIFO_SIZE		512
#endif

#define lcdc_readl(mmio, reg)		__raw_readl((mmio)+(reg))
#define lcdc_writel(mmio, reg, val)	__raw_writel((val), (mmio)+(reg))

static void atmel_fb_init(ulong addr, struct display_timing *timing, int bpix,
			  bool tft, bool cont_pol_low, ulong lcdbase)
{
	unsigned long value;
	void *reg = (void *)addr;

	/* Turn off the LCD controller and the DMA controller */
	lcdc_writel(reg, ATMEL_LCDC_PWRCON,
		    ATMEL_LCDC_GUARD_TIME << ATMEL_LCDC_GUARDT_OFFSET);

	/* Wait for the LCDC core to become idle */
	while (lcdc_readl(reg, ATMEL_LCDC_PWRCON) & ATMEL_LCDC_BUSY)
		udelay(10);

	lcdc_writel(reg, ATMEL_LCDC_DMACON, 0);

	/* Reset LCDC DMA */
	lcdc_writel(reg, ATMEL_LCDC_DMACON, ATMEL_LCDC_DMARST);

	/* ...set frame size and burst length = 8 words (?) */
	value = (timing->hactive.typ * timing->vactive.typ *
		 (1 << bpix)) / 32;
	value |= ((ATMEL_LCDC_DMA_BURST_LEN - 1) << ATMEL_LCDC_BLENGTH_OFFSET);
	lcdc_writel(reg, ATMEL_LCDC_DMAFRMCFG, value);

	/* Set pixel clock */
	value = get_lcdc_clk_rate(0) / timing->pixelclock.typ;
	if (get_lcdc_clk_rate(0) % timing->pixelclock.typ)
		value++;
	value = (value / 2) - 1;

	if (!value) {
		lcdc_writel(reg, ATMEL_LCDC_LCDCON1, ATMEL_LCDC_BYPASS);
	} else
		lcdc_writel(reg, ATMEL_LCDC_LCDCON1,
			    value << ATMEL_LCDC_CLKVAL_OFFSET);

	/* Initialize control register 2 */
	value = ATMEL_LCDC_MEMOR_LITTLE | ATMEL_LCDC_CLKMOD_ALWAYSACTIVE;
	if (tft)
		value |= ATMEL_LCDC_DISTYPE_TFT;

	if (!(timing->flags & DISPLAY_FLAGS_HSYNC_HIGH))
		value |= ATMEL_LCDC_INVLINE_INVERTED;
	if (!(timing->flags & DISPLAY_FLAGS_VSYNC_HIGH))
		value |= ATMEL_LCDC_INVFRAME_INVERTED;
	value |= bpix << 5;
	lcdc_writel(reg, ATMEL_LCDC_LCDCON2, value);

	/* Vertical timing */
	value = (timing->vsync_len.typ - 1) << ATMEL_LCDC_VPW_OFFSET;
	value |= timing->vback_porch.typ << ATMEL_LCDC_VBP_OFFSET;
	value |= timing->vfront_porch.typ;
	/* Magic! (Datasheet says "Bit 31 must be written to 1") */
	value |= 1U << 31;
	lcdc_writel(reg, ATMEL_LCDC_TIM1, value);

	/* Horizontal timing */
	value = (timing->hfront_porch.typ - 1) << ATMEL_LCDC_HFP_OFFSET;
	value |= (timing->hsync_len.typ - 1) << ATMEL_LCDC_HPW_OFFSET;
	value |= (timing->hback_porch.typ - 1);
	lcdc_writel(reg, ATMEL_LCDC_TIM2, value);

	/* Display size */
	value = (timing->hactive.typ - 1) << ATMEL_LCDC_HOZVAL_OFFSET;
	value |= timing->vactive.typ - 1;
	lcdc_writel(reg, ATMEL_LCDC_LCDFRMCFG, value);

	/* FIFO Threshold: Use formula from data sheet */
	value = ATMEL_LCDC_FIFO_SIZE - (2 * ATMEL_LCDC_DMA_BURST_LEN + 3);
	lcdc_writel(reg, ATMEL_LCDC_FIFO, value);

	/* Toggle LCD_MODE every frame */
	lcdc_writel(reg, ATMEL_LCDC_MVAL, 0);

	/* Disable all interrupts */
	lcdc_writel(reg, ATMEL_LCDC_IDR, ~0UL);

	/* Set contrast */
	value = ATMEL_LCDC_PS_DIV8 |
		ATMEL_LCDC_ENA_PWMENABLE;
	if (!cont_pol_low)
		value |= ATMEL_LCDC_POL_POSITIVE;
	lcdc_writel(reg, ATMEL_LCDC_CONTRAST_CTR, value);
	lcdc_writel(reg, ATMEL_LCDC_CONTRAST_VAL, ATMEL_LCDC_CVAL_DEFAULT);

	/* Set framebuffer DMA base address and pixel offset */
	lcdc_writel(reg, ATMEL_LCDC_DMABADDR1, lcdbase);

	lcdc_writel(reg, ATMEL_LCDC_DMACON, ATMEL_LCDC_DMAEN);
	lcdc_writel(reg, ATMEL_LCDC_PWRCON,
		    (ATMEL_LCDC_GUARD_TIME << ATMEL_LCDC_GUARDT_OFFSET) | ATMEL_LCDC_PWR);
}

static int atmel_fb_lcd_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct atmel_fb_priv *priv = dev_get_priv(dev);
	struct display_timing *timing = &priv->timing;

	/*
	 * For now some values are hard-coded. We could use the device tree
	 * bindings in simple-framebuffer.txt to specify the format/bpp and
	 * some Atmel-specific binding for tft and cont_pol_low.
	 */
	atmel_fb_init(ATMEL_BASE_LCDC, timing, VIDEO_BPP16, true, false,
		      uc_plat->base);
	uc_priv->xsize = timing->hactive.typ;
	uc_priv->ysize = timing->vactive.typ;
	uc_priv->bpix = VIDEO_BPP16;
	video_set_flush_dcache(dev, true);
	debug("LCD frame buffer at %lx, size %x, %dx%d pixels\n", uc_plat->base,
	      uc_plat->size, uc_priv->xsize, uc_priv->ysize);

	return 0;
}

static int atmel_fb_of_to_plat(struct udevice *dev)
{
	struct atmel_lcd_plat *plat = dev_get_plat(dev);
	struct atmel_fb_priv *priv = dev_get_priv(dev);
	struct display_timing *timing = &priv->timing;
	const void *blob = gd->fdt_blob;

	if (fdtdec_decode_display_timing(blob, dev_of_offset(dev),
					 plat->timing_index, timing)) {
		debug("%s: Failed to decode display timing\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int atmel_fb_lcd_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
			(1 << VIDEO_BPP16) / 8;
	debug("%s: Frame buffer size %x\n", __func__, uc_plat->size);

	return 0;
}

static const struct udevice_id atmel_fb_lcd_ids[] = {
	{ .compatible = "atmel,at91sam9g45-lcdc" },
	{ }
};

U_BOOT_DRIVER(atmel_fb) = {
	.name	= "atmel_fb",
	.id	= UCLASS_VIDEO,
	.of_match = atmel_fb_lcd_ids,
	.bind	= atmel_fb_lcd_bind,
	.of_to_plat	= atmel_fb_of_to_plat,
	.probe	= atmel_fb_lcd_probe,
	.plat_auto	= sizeof(struct atmel_lcd_plat),
	.priv_auto	= sizeof(struct atmel_fb_priv),
};
