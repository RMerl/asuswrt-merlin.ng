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
#include <video.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <bmp_layout.h>
#include <atmel_lcdc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DM_VIDEO
enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};
#endif

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

#ifndef CONFIG_DM_VIDEO
ushort *configuration_get_cmap(void)
{
	return (ushort *)(panel_info.mmio + ATMEL_LCDC_LUT(0));
}

#if defined(CONFIG_BMP_16BPP) && defined(CONFIG_ATMEL_LCD_BGR555)
void fb_put_word(uchar **fb, uchar **from)
{
	*(*fb)++ = (((*from)[0] & 0x1f) << 2) | ((*from)[1] & 0x03);
	*(*fb)++ = ((*from)[0] & 0xe0) | (((*from)[1] & 0x7c) >> 2);
	*from += 2;
}
#endif

#ifdef CONFIG_LCD_LOGO
#include <bmp_logo.h>
void lcd_logo_set_cmap(void)
{
	int i;
	uint lut_entry;
	ushort colreg;
	uint *cmap = (uint *)configuration_get_cmap();

	for (i = 0; i < BMP_LOGO_COLORS; ++i) {
		colreg = bmp_logo_palette[i];
#ifdef CONFIG_ATMEL_LCD_BGR555
		lut_entry = ((colreg & 0x000F) << 11) |
				((colreg & 0x00F0) <<  2) |
				((colreg & 0x0F00) >>  7);
#else
		lut_entry = ((colreg & 0x000F) << 1) |
				((colreg & 0x00F0) << 3) |
				((colreg & 0x0F00) << 4);
#endif
		*(cmap + BMP_LOGO_OFFSET) = lut_entry;
		cmap++;
	}
}
#endif

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
#if defined(CONFIG_ATMEL_LCD_BGR555)
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LUT(regno),
		    (red >> 3) | ((green & 0xf8) << 2) | ((blue & 0xf8) << 7));
#else
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LUT(regno),
		    (blue >> 3) | ((green & 0xfc) << 3) | ((red & 0xf8) << 8));
#endif
}

void lcd_set_cmap(struct bmp_image *bmp, unsigned colors)
{
	int i;

	for (i = 0; i < colors; ++i) {
		struct bmp_color_table_entry cte = bmp->color_table[i];
		lcd_setcolreg(i, cte.red, cte.green, cte.blue);
	}
}
#endif

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

#ifndef CONFIG_DM_VIDEO
void lcd_ctrl_init(void *lcdbase)
{
	struct display_timing timing;

	timing.flags = 0;
	if (!(panel_info.vl_sync & ATMEL_LCDC_INVLINE_INVERTED))
		timing.flags |= DISPLAY_FLAGS_HSYNC_HIGH;
	if (!(panel_info.vl_sync & ATMEL_LCDC_INVFRAME_INVERTED))
		timing.flags |= DISPLAY_FLAGS_VSYNC_LOW;
	timing.pixelclock.typ = panel_info.vl_clk;

	timing.hactive.typ = panel_info.vl_col;
	timing.hfront_porch.typ = panel_info.vl_right_margin;
	timing.hback_porch.typ = panel_info.vl_left_margin;
	timing.hsync_len.typ = panel_info.vl_hsync_len;

	timing.vactive.typ = panel_info.vl_row;
	timing.vfront_porch.typ = panel_info.vl_clk;
	timing.vback_porch.typ = panel_info.vl_clk;
	timing.vsync_len.typ = panel_info.vl_clk;

	atmel_fb_init(panel_info.mmio, &timing, panel_info.vl_bpix,
		      panel_info.vl_tft, panel_info.vl_cont_pol_low,
		      (ulong)lcdbase);
}

ulong calc_fbsize(void)
{
	return ((panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8) + PAGE_SIZE;
}
#endif

#ifdef CONFIG_DM_VIDEO
static int atmel_fb_lcd_probe(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
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

static int atmel_fb_ofdata_to_platdata(struct udevice *dev)
{
	struct atmel_lcd_platdata *plat = dev_get_platdata(dev);
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
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);

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
	.ofdata_to_platdata	= atmel_fb_ofdata_to_platdata,
	.probe	= atmel_fb_lcd_probe,
	.platdata_auto_alloc_size = sizeof(struct atmel_lcd_platdata),
	.priv_auto_alloc_size	= sizeof(struct atmel_fb_priv),
};
#endif
