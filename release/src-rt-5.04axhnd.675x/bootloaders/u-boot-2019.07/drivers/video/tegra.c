// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <panel.h>
#include <pwm.h>
#include <video.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/pwm.h>
#include <asm/arch/display.h>
#include <asm/arch-tegra/timer.h>

DECLARE_GLOBAL_DATA_PTR;

/* Information about the display controller */
struct tegra_lcd_priv {
	int width;			/* width in pixels */
	int height;			/* height in pixels */
	enum video_log2_bpp log2_bpp;	/* colour depth */
	struct display_timing timing;
	struct udevice *panel;
	struct disp_ctlr *disp;		/* Display controller to use */
	fdt_addr_t frame_buffer;	/* Address of frame buffer */
	unsigned pixel_clock;		/* Pixel clock in Hz */
};

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};

static void update_window(struct dc_ctlr *dc, struct disp_ctl_win *win)
{
	unsigned h_dda, v_dda;
	unsigned long val;

	val = readl(&dc->cmd.disp_win_header);
	val |= WINDOW_A_SELECT;
	writel(val, &dc->cmd.disp_win_header);

	writel(win->fmt, &dc->win.color_depth);

	clrsetbits_le32(&dc->win.byte_swap, BYTE_SWAP_MASK,
			BYTE_SWAP_NOSWAP << BYTE_SWAP_SHIFT);

	val = win->out_x << H_POSITION_SHIFT;
	val |= win->out_y << V_POSITION_SHIFT;
	writel(val, &dc->win.pos);

	val = win->out_w << H_SIZE_SHIFT;
	val |= win->out_h << V_SIZE_SHIFT;
	writel(val, &dc->win.size);

	val = (win->w * win->bpp / 8) << H_PRESCALED_SIZE_SHIFT;
	val |= win->h << V_PRESCALED_SIZE_SHIFT;
	writel(val, &dc->win.prescaled_size);

	writel(0, &dc->win.h_initial_dda);
	writel(0, &dc->win.v_initial_dda);

	h_dda = (win->w * 0x1000) / max(win->out_w - 1, 1U);
	v_dda = (win->h * 0x1000) / max(win->out_h - 1, 1U);

	val = h_dda << H_DDA_INC_SHIFT;
	val |= v_dda << V_DDA_INC_SHIFT;
	writel(val, &dc->win.dda_increment);

	writel(win->stride, &dc->win.line_stride);
	writel(0, &dc->win.buf_stride);

	val = WIN_ENABLE;
	if (win->bpp < 24)
		val |= COLOR_EXPAND;
	writel(val, &dc->win.win_opt);

	writel((unsigned long)win->phys_addr, &dc->winbuf.start_addr);
	writel(win->x, &dc->winbuf.addr_h_offset);
	writel(win->y, &dc->winbuf.addr_v_offset);

	writel(0xff00, &dc->win.blend_nokey);
	writel(0xff00, &dc->win.blend_1win);

	val = GENERAL_ACT_REQ | WIN_A_ACT_REQ;
	val |= GENERAL_UPDATE | WIN_A_UPDATE;
	writel(val, &dc->cmd.state_ctrl);
}

static int update_display_mode(struct dc_disp_reg *disp,
			       struct tegra_lcd_priv *priv)
{
	struct display_timing *dt = &priv->timing;
	unsigned long val;
	unsigned long rate;
	unsigned long div;

	writel(0x0, &disp->disp_timing_opt);

	writel(1 | 1 << 16, &disp->ref_to_sync);
	writel(dt->hsync_len.typ | dt->vsync_len.typ << 16, &disp->sync_width);
	writel(dt->hback_porch.typ | dt->vback_porch.typ << 16,
	       &disp->back_porch);
	writel((dt->hfront_porch.typ - 1) | (dt->vfront_porch.typ - 1) << 16,
	       &disp->front_porch);
	writel(dt->hactive.typ | (dt->vactive.typ << 16), &disp->disp_active);

	val = DE_SELECT_ACTIVE << DE_SELECT_SHIFT;
	val |= DE_CONTROL_NORMAL << DE_CONTROL_SHIFT;
	writel(val, &disp->data_enable_opt);

	val = DATA_FORMAT_DF1P1C << DATA_FORMAT_SHIFT;
	val |= DATA_ALIGNMENT_MSB << DATA_ALIGNMENT_SHIFT;
	val |= DATA_ORDER_RED_BLUE << DATA_ORDER_SHIFT;
	writel(val, &disp->disp_interface_ctrl);

	/*
	 * The pixel clock divider is in 7.1 format (where the bottom bit
	 * represents 0.5). Here we calculate the divider needed to get from
	 * the display clock (typically 600MHz) to the pixel clock. We round
	 * up or down as requried.
	 */
	rate = clock_get_periph_rate(PERIPH_ID_DISP1, CLOCK_ID_CGENERAL);
	div = ((rate * 2 + priv->pixel_clock / 2) / priv->pixel_clock) - 2;
	debug("Display clock %lu, divider %lu\n", rate, div);

	writel(0x00010001, &disp->shift_clk_opt);

	val = PIXEL_CLK_DIVIDER_PCD1 << PIXEL_CLK_DIVIDER_SHIFT;
	val |= div << SHIFT_CLK_DIVIDER_SHIFT;
	writel(val, &disp->disp_clk_ctrl);

	return 0;
}

/* Start up the display and turn on power to PWMs */
static void basic_init(struct dc_cmd_reg *cmd)
{
	u32 val;

	writel(0x00000100, &cmd->gen_incr_syncpt_ctrl);
	writel(0x0000011a, &cmd->cont_syncpt_vsync);
	writel(0x00000000, &cmd->int_type);
	writel(0x00000000, &cmd->int_polarity);
	writel(0x00000000, &cmd->int_mask);
	writel(0x00000000, &cmd->int_enb);

	val = PW0_ENABLE | PW1_ENABLE | PW2_ENABLE;
	val |= PW3_ENABLE | PW4_ENABLE | PM0_ENABLE;
	val |= PM1_ENABLE;
	writel(val, &cmd->disp_pow_ctrl);

	val = readl(&cmd->disp_cmd);
	val |= CTRL_MODE_C_DISPLAY << CTRL_MODE_SHIFT;
	writel(val, &cmd->disp_cmd);
}

static void basic_init_timer(struct dc_disp_reg *disp)
{
	writel(0x00000020, &disp->mem_high_pri);
	writel(0x00000001, &disp->mem_high_pri_timer);
}

static const u32 rgb_enb_tab[PIN_REG_COUNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

static const u32 rgb_polarity_tab[PIN_REG_COUNT] = {
	0x00000000,
	0x01000000,
	0x00000000,
	0x00000000,
};

static const u32 rgb_data_tab[PIN_REG_COUNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

static const u32 rgb_sel_tab[PIN_OUTPUT_SEL_COUNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00210222,
	0x00002200,
	0x00020000,
};

static void rgb_enable(struct dc_com_reg *com)
{
	int i;

	for (i = 0; i < PIN_REG_COUNT; i++) {
		writel(rgb_enb_tab[i], &com->pin_output_enb[i]);
		writel(rgb_polarity_tab[i], &com->pin_output_polarity[i]);
		writel(rgb_data_tab[i], &com->pin_output_data[i]);
	}

	for (i = 0; i < PIN_OUTPUT_SEL_COUNT; i++)
		writel(rgb_sel_tab[i], &com->pin_output_sel[i]);
}

static int setup_window(struct disp_ctl_win *win,
			struct tegra_lcd_priv *priv)
{
	win->x = 0;
	win->y = 0;
	win->w = priv->width;
	win->h = priv->height;
	win->out_x = 0;
	win->out_y = 0;
	win->out_w = priv->width;
	win->out_h = priv->height;
	win->phys_addr = priv->frame_buffer;
	win->stride = priv->width * (1 << priv->log2_bpp) / 8;
	debug("%s: depth = %d\n", __func__, priv->log2_bpp);
	switch (priv->log2_bpp) {
	case VIDEO_BPP32:
		win->fmt = COLOR_DEPTH_R8G8B8A8;
		win->bpp = 32;
		break;
	case VIDEO_BPP16:
		win->fmt = COLOR_DEPTH_B5G6R5;
		win->bpp = 16;
		break;

	default:
		debug("Unsupported LCD bit depth");
		return -1;
	}

	return 0;
}

/**
 * Register a new display based on device tree configuration.
 *
 * The frame buffer can be positioned by U-Boot or overridden by the fdt.
 * You should pass in the U-Boot address here, and check the contents of
 * struct tegra_lcd_priv to see what was actually chosen.
 *
 * @param blob			Device tree blob
 * @param priv			Driver's private data
 * @param default_lcd_base	Default address of LCD frame buffer
 * @return 0 if ok, -1 on error (unsupported bits per pixel)
 */
static int tegra_display_probe(const void *blob, struct tegra_lcd_priv *priv,
			       void *default_lcd_base)
{
	struct disp_ctl_win window;
	struct dc_ctlr *dc;

	priv->frame_buffer = (u32)default_lcd_base;

	dc = (struct dc_ctlr *)priv->disp;

	/*
	 * A header file for clock constants was NAKed upstream.
	 * TODO: Put this into the FDT and fdt_lcd struct when we have clock
	 * support there
	 */
	clock_start_periph_pll(PERIPH_ID_HOST1X, CLOCK_ID_PERIPH,
			       144 * 1000000);
	clock_start_periph_pll(PERIPH_ID_DISP1, CLOCK_ID_CGENERAL,
			       600 * 1000000);
	basic_init(&dc->cmd);
	basic_init_timer(&dc->disp);
	rgb_enable(&dc->com);

	if (priv->pixel_clock)
		update_display_mode(&dc->disp, priv);

	if (setup_window(&window, priv))
		return -1;

	update_window(dc, &window);

	return 0;
}

static int tegra_lcd_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int ret;

	/* Initialize the Tegra display controller */
	funcmux_select(PERIPH_ID_DISP1, FUNCMUX_DEFAULT);
	if (tegra_display_probe(blob, priv, (void *)plat->base)) {
		printf("%s: Failed to probe display driver\n", __func__);
		return -1;
	}

	pinmux_set_func(PMUX_PINGRP_GPU, PMUX_FUNC_PWM);
	pinmux_tristate_disable(PMUX_PINGRP_GPU);

	ret = panel_enable_backlight(priv->panel);
	if (ret) {
		debug("%s: Cannot enable backlight, ret=%d\n", __func__, ret);
		return ret;
	}

	mmu_set_region_dcache_behaviour(priv->frame_buffer, plat->size,
					DCACHE_WRITETHROUGH);

	/* Enable flushing after LCD writes if requested */
	video_set_flush_dcache(dev, true);

	uc_priv->xsize = priv->width;
	uc_priv->ysize = priv->height;
	uc_priv->bpix = priv->log2_bpp;
	debug("LCD frame buffer at %pa, size %x\n", &priv->frame_buffer,
	      plat->size);

	return 0;
}

static int tegra_lcd_ofdata_to_platdata(struct udevice *dev)
{
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	struct display_timing *timing;
	int node = dev_of_offset(dev);
	int panel_node;
	int rgb;
	int ret;

	priv->disp = (struct disp_ctlr *)devfdt_get_addr(dev);
	if (!priv->disp) {
		debug("%s: No display controller address\n", __func__);
		return -EINVAL;
	}

	rgb = fdt_subnode_offset(blob, node, "rgb");
	if (rgb < 0) {
		debug("%s: Cannot find rgb subnode for '%s' (ret=%d)\n",
		      __func__, dev->name, rgb);
		return -EINVAL;
	}

	ret = fdtdec_decode_display_timing(blob, rgb, 0, &priv->timing);
	if (ret) {
		debug("%s: Cannot read display timing for '%s' (ret=%d)\n",
		      __func__, dev->name, ret);
		return -EINVAL;
	}
	timing = &priv->timing;
	priv->width = timing->hactive.typ;
	priv->height = timing->vactive.typ;
	priv->pixel_clock = timing->pixelclock.typ;
	priv->log2_bpp = VIDEO_BPP16;

	/*
	 * Sadly the panel phandle is in an rgb subnode so we cannot use
	 * uclass_get_device_by_phandle().
	 */
	panel_node = fdtdec_lookup_phandle(blob, rgb, "nvidia,panel");
	if (panel_node < 0) {
		debug("%s: Cannot find panel information\n", __func__);
		return -EINVAL;
	}
	ret = uclass_get_device_by_of_offset(UCLASS_PANEL, panel_node,
					     &priv->panel);
	if (ret) {
		debug("%s: Cannot find panel for '%s' (ret=%d)\n", __func__,
		      dev->name, ret);
		return ret;
	}

	return 0;
}

static int tegra_lcd_bind(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int rgb;

	rgb = fdt_subnode_offset(blob, node, "rgb");
	if ((rgb < 0) || !fdtdec_get_is_enabled(blob, rgb))
		return -ENODEV;

	plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
		(1 << LCD_MAX_LOG2_BPP) / 8;

	return 0;
}

static const struct video_ops tegra_lcd_ops = {
};

static const struct udevice_id tegra_lcd_ids[] = {
	{ .compatible = "nvidia,tegra20-dc" },
	{ }
};

U_BOOT_DRIVER(tegra_lcd) = {
	.name	= "tegra_lcd",
	.id	= UCLASS_VIDEO,
	.of_match = tegra_lcd_ids,
	.ops	= &tegra_lcd_ops,
	.bind	= tegra_lcd_bind,
	.probe	= tegra_lcd_probe,
	.ofdata_to_platdata	= tegra_lcd_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct tegra_lcd_priv),
};
