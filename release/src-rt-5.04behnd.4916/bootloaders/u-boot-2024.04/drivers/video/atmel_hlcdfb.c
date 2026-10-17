// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for AT91/AT32 MULTI LAYER LCD Controller
 *
 * Copyright (C) 2012 Atmel Corporation
 */

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <video.h>
#include <wait_bit.h>
#include <atmel_hlcdc.h>
#include <linux/bug.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	LCD_MAX_WIDTH		= 1024,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};

struct atmel_hlcdc_priv {
	struct atmel_hlcd_regs *regs;
	struct display_timing timing;
	unsigned int vl_bpix;
	unsigned int output_mode;
	unsigned int guard_time;
	ulong clk_rate;
};

static int at91_hlcdc_enable_clk(struct udevice *dev)
{
	struct atmel_hlcdc_priv *priv = dev_get_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate) {
		clk_disable(&clk);
		return -ENODEV;
	}

	priv->clk_rate = clk_rate;

	return 0;
}

static void atmel_hlcdc_init(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct atmel_hlcdc_priv *priv = dev_get_priv(dev);
	struct atmel_hlcd_regs *regs = priv->regs;
	struct display_timing *timing = &priv->timing;
	struct lcd_dma_desc *desc;
	unsigned long value, vl_clk_pol;
	int ret;

	/* Disable DISP signal */
	writel(LCDC_LCDDIS_DISPDIS, &regs->lcdc_lcddis);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_DISPSTS,
				false, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
	/* Disable synchronization */
	writel(LCDC_LCDDIS_SYNCDIS, &regs->lcdc_lcddis);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_LCDSTS,
				false, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
	/* Disable pixel clock */
	writel(LCDC_LCDDIS_CLKDIS, &regs->lcdc_lcddis);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_CLKSTS,
				false, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
	/* Disable PWM */
	writel(LCDC_LCDDIS_PWMDIS, &regs->lcdc_lcddis);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_PWMSTS,
				false, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);

	/* Set pixel clock */
	value = priv->clk_rate / timing->pixelclock.typ;
	if (priv->clk_rate % timing->pixelclock.typ)
		value++;

	vl_clk_pol = 0;
	if (timing->flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE)
		vl_clk_pol = LCDC_LCDCFG0_CLKPOL;

	if (value < 1) {
		/* Using system clock as pixel clock */
		writel(LCDC_LCDCFG0_CLKDIV(0)
			| LCDC_LCDCFG0_CGDISHCR
			| LCDC_LCDCFG0_CGDISHEO
			| LCDC_LCDCFG0_CGDISOVR1
			| LCDC_LCDCFG0_CGDISBASE
			| vl_clk_pol
			| LCDC_LCDCFG0_CLKSEL,
			&regs->lcdc_lcdcfg0);

	} else {
		writel(LCDC_LCDCFG0_CLKDIV(value - 2)
			| LCDC_LCDCFG0_CGDISHCR
			| LCDC_LCDCFG0_CGDISHEO
			| LCDC_LCDCFG0_CGDISOVR1
			| LCDC_LCDCFG0_CGDISBASE
			| vl_clk_pol,
			&regs->lcdc_lcdcfg0);
	}

	/* Initialize control register 5 */
	value = 0;

	if (!(timing->flags & DISPLAY_FLAGS_HSYNC_HIGH))
		value |= LCDC_LCDCFG5_HSPOL;
	if (!(timing->flags & DISPLAY_FLAGS_VSYNC_HIGH))
		value |= LCDC_LCDCFG5_VSPOL;

	switch (priv->output_mode) {
	case 12:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_12BPP;
		break;
	case 16:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_16BPP;
		break;
	case 18:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_18BPP;
		break;
	case 24:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_24BPP;
		break;
	default:
		BUG();
		break;
	}

	value |= LCDC_LCDCFG5_GUARDTIME(priv->guard_time);
	value |= (LCDC_LCDCFG5_DISPDLY | LCDC_LCDCFG5_VSPDLYS);
	writel(value, &regs->lcdc_lcdcfg5);

	/* Vertical & Horizontal Timing */
	value = LCDC_LCDCFG1_VSPW(timing->vsync_len.typ - 1);
	value |= LCDC_LCDCFG1_HSPW(timing->hsync_len.typ - 1);
	writel(value, &regs->lcdc_lcdcfg1);

	value = LCDC_LCDCFG2_VBPW(timing->vback_porch.typ);
	value |= LCDC_LCDCFG2_VFPW(timing->vfront_porch.typ - 1);
	writel(value, &regs->lcdc_lcdcfg2);

	value = LCDC_LCDCFG3_HBPW(timing->hback_porch.typ - 1);
	value |= LCDC_LCDCFG3_HFPW(timing->hfront_porch.typ - 1);
	writel(value, &regs->lcdc_lcdcfg3);

	/* Display size */
	value = LCDC_LCDCFG4_RPF(timing->vactive.typ - 1);
	value |= LCDC_LCDCFG4_PPL(timing->hactive.typ - 1);
	writel(value, &regs->lcdc_lcdcfg4);

	writel(LCDC_BASECFG0_BLEN_AHB_INCR4 | LCDC_BASECFG0_DLBO,
	       &regs->lcdc_basecfg0);

	switch (VNBITS(priv->vl_bpix)) {
	case 16:
		writel(LCDC_BASECFG1_RGBMODE_16BPP_RGB_565,
		       &regs->lcdc_basecfg1);
		break;
	case 32:
		writel(LCDC_BASECFG1_RGBMODE_24BPP_RGB_888,
		       &regs->lcdc_basecfg1);
		break;
	default:
		BUG();
		break;
	}

	writel(LCDC_BASECFG2_XSTRIDE(0), &regs->lcdc_basecfg2);
	writel(0, &regs->lcdc_basecfg3);
	writel(LCDC_BASECFG4_DMA, &regs->lcdc_basecfg4);

	/* Disable all interrupts */
	writel(~0UL, &regs->lcdc_lcdidr);
	writel(~0UL, &regs->lcdc_baseidr);

	/* Setup the DMA descriptor, this descriptor will loop to itself */
	desc = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*desc));
	if (!desc)
		return;

	desc->address = (u32)uc_plat->base;

	/* Disable DMA transfer interrupt & descriptor loaded interrupt. */
	desc->control = LCDC_BASECTRL_ADDIEN | LCDC_BASECTRL_DSCRIEN
			| LCDC_BASECTRL_DMAIEN | LCDC_BASECTRL_DFETCH;
	desc->next = (u32)desc;

	/* Flush the DMA descriptor if we enabled dcache */
	flush_dcache_range((u32)desc,
			   ALIGN(((u32)desc + sizeof(*desc)),
			   CONFIG_SYS_CACHELINE_SIZE));

	writel(desc->address, &regs->lcdc_baseaddr);
	writel(desc->control, &regs->lcdc_basectrl);
	writel(desc->next, &regs->lcdc_basenext);
	writel(LCDC_BASECHER_CHEN | LCDC_BASECHER_UPDATEEN,
	       &regs->lcdc_basecher);

	/* Enable LCD */
	value = readl(&regs->lcdc_lcden);
	writel(value | LCDC_LCDEN_CLKEN, &regs->lcdc_lcden);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_CLKSTS,
				true, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
	value = readl(&regs->lcdc_lcden);
	writel(value | LCDC_LCDEN_SYNCEN, &regs->lcdc_lcden);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_LCDSTS,
				true, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
	value = readl(&regs->lcdc_lcden);
	writel(value | LCDC_LCDEN_DISPEN, &regs->lcdc_lcden);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_DISPSTS,
				true, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
	value = readl(&regs->lcdc_lcden);
	writel(value | LCDC_LCDEN_PWMEN, &regs->lcdc_lcden);
	ret = wait_for_bit_le32(&regs->lcdc_lcdsr, LCDC_LCDSR_PWMSTS,
				true, 1000, false);
	if (ret)
		printf("%s: %d: Timeout!\n", __func__, __LINE__);
}

static int atmel_hlcdc_probe(struct udevice *dev)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct atmel_hlcdc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = at91_hlcdc_enable_clk(dev);
	if (ret)
		return ret;

	atmel_hlcdc_init(dev);

	uc_priv->xsize = priv->timing.hactive.typ;
	uc_priv->ysize = priv->timing.vactive.typ;
	uc_priv->bpix = priv->vl_bpix;

	/* Enable flushing if we enabled dcache */
	video_set_flush_dcache(dev, true);

	return 0;
}

static int atmel_hlcdc_of_to_plat(struct udevice *dev)
{
	struct atmel_hlcdc_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs) {
		debug("%s: No display controller address\n", __func__);
		return -EINVAL;
	}

	if (fdtdec_decode_display_timing(blob, dev_of_offset(dev),
					 0, &priv->timing)) {
		debug("%s: Failed to decode display timing\n", __func__);
		return -EINVAL;
	}

	if (priv->timing.hactive.typ > LCD_MAX_WIDTH)
		priv->timing.hactive.typ = LCD_MAX_WIDTH;

	if (priv->timing.vactive.typ > LCD_MAX_HEIGHT)
		priv->timing.vactive.typ = LCD_MAX_HEIGHT;

	priv->vl_bpix = fdtdec_get_int(blob, node, "atmel,vl-bpix", 0);
	if (!priv->vl_bpix) {
		debug("%s: Failed to get bits per pixel\n", __func__);
		return -EINVAL;
	}

	priv->output_mode = fdtdec_get_int(blob, node, "atmel,output-mode", 24);
	priv->guard_time = fdtdec_get_int(blob, node, "atmel,guard-time", 1);

	return 0;
}

static int atmel_hlcdc_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
				(1 << LCD_MAX_LOG2_BPP) / 8;

	debug("%s: Frame buffer size %x\n", __func__, uc_plat->size);

	return 0;
}

static const struct udevice_id atmel_hlcdc_ids[] = {
	{ .compatible = "atmel,sama5d2-hlcdc" },
	{ .compatible = "atmel,at91sam9x5-hlcdc" },
	{ }
};

U_BOOT_DRIVER(atmel_hlcdfb) = {
	.name	= "atmel_hlcdfb",
	.id	= UCLASS_VIDEO,
	.of_match = atmel_hlcdc_ids,
	.bind	= atmel_hlcdc_bind,
	.probe	= atmel_hlcdc_probe,
	.of_to_plat = atmel_hlcdc_of_to_plat,
	.priv_auto	= sizeof(struct atmel_hlcdc_priv),
};
