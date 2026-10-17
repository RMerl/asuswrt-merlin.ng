// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX23/i.MX28 LCDIF driver
 *
 * Copyright (C) 2011-2013 Marek Vasut <marex@denx.de>
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <env.h>
#include <log.h>
#include <asm/cache.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <malloc.h>
#include <video.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/dma.h>
#include <asm/io.h>

#include "videomodes.h"

#define	PS2KHZ(ps)	(1000000000UL / (ps))
#define HZ2PS(hz)	(1000000000UL / ((hz) / 1000))

#define BITS_PP		18
#define BYTES_PP	4

struct mxs_dma_desc desc;

/**
 * mxsfb_system_setup() - Fine-tune LCDIF configuration
 *
 * This function is used to adjust the LCDIF configuration. This is usually
 * needed when driving the controller in System-Mode to operate an 8080 or
 * 6800 connected SmartLCD.
 */
__weak void mxsfb_system_setup(void)
{
}

/*
 * ARIES M28EVK:
 * setenv videomode
 * video=ctfb:x:800,y:480,depth:18,mode:0,pclk:30066,
 *       le:0,ri:256,up:0,lo:45,hs:1,vs:1,sync:100663296,vmode:0
 *
 * Freescale mx23evk/mx28evk with a Seiko 4.3'' WVGA panel:
 * setenv videomode
 * video=ctfb:x:800,y:480,depth:24,mode:0,pclk:29851,
 *	 le:89,ri:164,up:23,lo:10,hs:10,vs:10,sync:0,vmode:0
 */

static void mxs_lcd_init(struct udevice *dev, u32 fb_addr,
			 struct display_timing *timings, int bpp)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	const enum display_flags flags = timings->flags;
	uint32_t word_len = 0, bus_width = 0;
	uint8_t valid_data = 0;
	uint32_t vdctrl0;

#if CONFIG_IS_ENABLED(CLK)
	struct clk clk;
	int ret;

	ret = clk_get_by_name(dev, "pix", &clk);
	if (ret) {
		dev_err(dev, "Failed to get mxs pix clk: %d\n", ret);
		return;
	}

	ret = clk_set_rate(&clk, timings->pixelclock.typ);
	if (ret < 0) {
		dev_err(dev, "Failed to set mxs pix clk: %d\n", ret);
		return;
	}

	ret = clk_enable(&clk);
	if (ret < 0) {
		dev_err(dev, "Failed to enable mxs pix clk: %d\n", ret);
		return;
	}

	ret = clk_get_by_name(dev, "axi", &clk);
	if (ret < 0) {
		debug("%s: Failed to get mxs axi clk: %d\n", __func__, ret);
	} else {
		ret = clk_enable(&clk);
		if (ret < 0) {
			dev_err(dev, "Failed to enable mxs axi clk: %d\n", ret);
			return;
		}
	}

	ret = clk_get_by_name(dev, "disp_axi", &clk);
	if (ret < 0) {
		debug("%s: Failed to get mxs disp_axi clk: %d\n", __func__, ret);
	} else {
		ret = clk_enable(&clk);
		if (ret < 0) {
			dev_err(dev, "Failed to enable mxs disp_axi clk: %d\n", ret);
			return;
		}
	}
#else
	/* Kick in the LCDIF clock */
	mxs_set_lcdclk(MXS_LCDIF_BASE, timings->pixelclock.typ / 1000);
#endif

	/* Restart the LCDIF block */
	mxs_reset_block(&regs->hw_lcdif_ctrl_reg);

	switch (bpp) {
	case 24:
		word_len = LCDIF_CTRL_WORD_LENGTH_24BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_24BIT;
		valid_data = 0x7;
		break;
	case 18:
		word_len = LCDIF_CTRL_WORD_LENGTH_24BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_18BIT;
		valid_data = 0x7;
		break;
	case 16:
		word_len = LCDIF_CTRL_WORD_LENGTH_16BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_16BIT;
		valid_data = 0xf;
		break;
	case 8:
		word_len = LCDIF_CTRL_WORD_LENGTH_8BIT;
		bus_width = LCDIF_CTRL_LCD_DATABUS_WIDTH_8BIT;
		valid_data = 0xf;
		break;
	}

	writel(bus_width | word_len | LCDIF_CTRL_DOTCLK_MODE |
		LCDIF_CTRL_BYPASS_COUNT | LCDIF_CTRL_LCDIF_MASTER,
		&regs->hw_lcdif_ctrl);

	writel(valid_data << LCDIF_CTRL1_BYTE_PACKING_FORMAT_OFFSET,
		&regs->hw_lcdif_ctrl1);

	mxsfb_system_setup();

	writel((timings->vactive.typ << LCDIF_TRANSFER_COUNT_V_COUNT_OFFSET) |
		timings->hactive.typ, &regs->hw_lcdif_transfer_count);

	vdctrl0 = LCDIF_VDCTRL0_ENABLE_PRESENT | LCDIF_VDCTRL0_ENABLE_POL |
		  LCDIF_VDCTRL0_VSYNC_PERIOD_UNIT |
		  LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_UNIT |
		  timings->vsync_len.typ;

	if(flags & DISPLAY_FLAGS_HSYNC_HIGH)
		vdctrl0 |= LCDIF_VDCTRL0_HSYNC_POL;
	if(flags & DISPLAY_FLAGS_VSYNC_HIGH)
		vdctrl0 |= LCDIF_VDCTRL0_VSYNC_POL;
	if(flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE)
		vdctrl0 |= LCDIF_VDCTRL0_DOTCLK_POL;
	if(flags & DISPLAY_FLAGS_DE_HIGH)
		vdctrl0 |= LCDIF_VDCTRL0_ENABLE_POL;

	writel(vdctrl0, &regs->hw_lcdif_vdctrl0);
	writel(timings->vback_porch.typ + timings->vfront_porch.typ +
		timings->vsync_len.typ + timings->vactive.typ,
		&regs->hw_lcdif_vdctrl1);
	writel((timings->hsync_len.typ << LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH_OFFSET) |
		(timings->hback_porch.typ + timings->hfront_porch.typ +
		timings->hsync_len.typ + timings->hactive.typ),
		&regs->hw_lcdif_vdctrl2);
	writel(((timings->hback_porch.typ + timings->hsync_len.typ) <<
		LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT_OFFSET) |
		(timings->vback_porch.typ + timings->vsync_len.typ),
		&regs->hw_lcdif_vdctrl3);
	writel((0 << LCDIF_VDCTRL4_DOTCLK_DLY_SEL_OFFSET) | timings->hactive.typ,
		&regs->hw_lcdif_vdctrl4);

	writel(fb_addr, &regs->hw_lcdif_cur_buf);
	writel(fb_addr, &regs->hw_lcdif_next_buf);

	/* Flush FIFO first */
	writel(LCDIF_CTRL1_FIFO_CLEAR, &regs->hw_lcdif_ctrl1_set);

#ifndef CONFIG_VIDEO_MXS_MODE_SYSTEM
	/* Sync signals ON */
	setbits_le32(&regs->hw_lcdif_vdctrl4, LCDIF_VDCTRL4_SYNC_SIGNALS_ON);
#endif

	/* FIFO cleared */
	writel(LCDIF_CTRL1_FIFO_CLEAR, &regs->hw_lcdif_ctrl1_clr);

	/* RUN! */
	writel(LCDIF_CTRL_RUN, &regs->hw_lcdif_ctrl_set);
}

static int mxs_probe_common(struct udevice *dev, struct display_timing *timings,
			    int bpp, u32 fb)
{
	/* Start framebuffer */
	mxs_lcd_init(dev, fb, timings, bpp);

#ifdef CONFIG_VIDEO_MXS_MODE_SYSTEM
	/*
	 * If the LCD runs in system mode, the LCD refresh has to be triggered
	 * manually by setting the RUN bit in HW_LCDIF_CTRL register. To avoid
	 * having to set this bit manually after every single change in the
	 * framebuffer memory, we set up specially crafted circular DMA, which
	 * sets the RUN bit, then waits until it gets cleared and repeats this
	 * infinitelly. This way, we get smooth continuous updates of the LCD.
	 */
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;

	memset(&desc, 0, sizeof(struct mxs_dma_desc));
	desc.address = (dma_addr_t)&desc;
	desc.cmd.data = MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_CHAIN |
			MXS_DMA_DESC_WAIT4END |
			(1 << MXS_DMA_DESC_PIO_WORDS_OFFSET);
	desc.cmd.pio_words[0] = readl(&regs->hw_lcdif_ctrl) | LCDIF_CTRL_RUN;
	desc.cmd.next = (uint32_t)&desc.cmd;

	/* Execute the DMA chain. */
	mxs_dma_circ_start(MXS_DMA_CHANNEL_AHB_APBH_LCDIF, &desc);
#endif

	return 0;
}

static int mxs_remove_common(u32 fb)
{
	struct mxs_lcdif_regs *regs = (struct mxs_lcdif_regs *)MXS_LCDIF_BASE;
	int timeout = 1000000;

	if (!fb)
		return -EINVAL;

	writel(fb, &regs->hw_lcdif_cur_buf_reg);
	writel(fb, &regs->hw_lcdif_next_buf_reg);
	writel(LCDIF_CTRL1_VSYNC_EDGE_IRQ, &regs->hw_lcdif_ctrl1_clr);
	while (--timeout) {
		if (readl(&regs->hw_lcdif_ctrl1_reg) &
		    LCDIF_CTRL1_VSYNC_EDGE_IRQ)
			break;
		udelay(1);
	}
	mxs_reset_block((struct mxs_register_32 *)&regs->hw_lcdif_ctrl_reg);

	return 0;
}

static int mxs_of_get_timings(struct udevice *dev,
			      struct display_timing *timings,
			      u32 *bpp)
{
	int ret = 0;
	u32 display_phandle;
	ofnode display_node;

	ret = ofnode_read_u32(dev_ofnode(dev), "display", &display_phandle);
	if (ret) {
		dev_err(dev, "required display property isn't provided\n");
		return -EINVAL;
	}

	display_node = ofnode_get_by_phandle(display_phandle);
	if (!ofnode_valid(display_node)) {
		dev_err(dev, "failed to find display subnode\n");
		return -EINVAL;
	}

	ret = ofnode_read_u32(display_node, "bits-per-pixel", bpp);
	if (ret) {
		dev_err(dev,
			"required bits-per-pixel property isn't provided\n");
		return -EINVAL;
	}

	ret = ofnode_decode_display_timing(display_node, 0, timings);
	if (ret) {
		dev_err(dev, "failed to get any display timings\n");
		return -EINVAL;
	}

	return ret;
}

static int mxs_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);

	struct display_timing timings;
	u32 bpp = 0;
	u32 fb_start, fb_end;
	int ret;

	debug("%s() plat: base 0x%lx, size 0x%x\n",
	       __func__, plat->base, plat->size);

	ret = mxs_of_get_timings(dev, &timings, &bpp);
	if (ret)
		return ret;

	ret = mxs_probe_common(dev, &timings, bpp, plat->base);
	if (ret)
		return ret;

	switch (bpp) {
	case 32:
	case 24:
	case 18:
		uc_priv->bpix = VIDEO_BPP32;
		break;
	case 16:
		uc_priv->bpix = VIDEO_BPP16;
		break;
	case 8:
		uc_priv->bpix = VIDEO_BPP8;
		break;
	default:
		dev_err(dev, "invalid bpp specified (bpp = %i)\n", bpp);
		return -EINVAL;
	}

	uc_priv->xsize = timings.hactive.typ;
	uc_priv->ysize = timings.vactive.typ;

	/* Enable dcache for the frame buffer */
	fb_start = plat->base & ~(MMU_SECTION_SIZE - 1);
	fb_end = plat->base + plat->size;
	fb_end = ALIGN(fb_end, 1 << MMU_SECTION_SHIFT);
	mmu_set_region_dcache_behaviour(fb_start, fb_end - fb_start,
					DCACHE_WRITEBACK);
	video_set_flush_dcache(dev, true);
	gd->fb_base = plat->base;

	return ret;
}

static int mxs_video_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct display_timing timings;
	u32 bpp = 0;
	u32 bytes_pp = 0;
	int ret;

	ret = mxs_of_get_timings(dev, &timings, &bpp);
	if (ret)
		return ret;

	switch (bpp) {
	case 32:
	case 24:
	case 18:
		bytes_pp = 4;
		break;
	case 16:
		bytes_pp = 2;
		break;
	case 8:
		bytes_pp = 1;
		break;
	default:
		dev_err(dev, "invalid bpp specified (bpp = %i)\n", bpp);
		return -EINVAL;
	}

	plat->size = timings.hactive.typ * timings.vactive.typ * bytes_pp;

	return 0;
}

static int mxs_video_remove(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	mxs_remove_common(plat->base);

	return 0;
}

static const struct udevice_id mxs_video_ids[] = {
	{ .compatible = "fsl,imx23-lcdif" },
	{ .compatible = "fsl,imx28-lcdif" },
	{ .compatible = "fsl,imx6sx-lcdif" },
	{ .compatible = "fsl,imx7ulp-lcdif" },
	{ .compatible = "fsl,imxrt-lcdif" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mxs_video) = {
	.name	= "mxs_video",
	.id	= UCLASS_VIDEO,
	.of_match = mxs_video_ids,
	.bind	= mxs_video_bind,
	.probe	= mxs_video_probe,
	.remove = mxs_video_remove,
	.flags	= DM_FLAG_PRE_RELOC | DM_FLAG_OS_PREPARE,
};
