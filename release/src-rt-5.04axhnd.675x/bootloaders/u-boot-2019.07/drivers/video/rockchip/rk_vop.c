// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <edid.h>
#include <regmap.h>
#include <syscon.h>
#include <video.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/edp_rk3288.h>
#include <asm/arch-rockchip/vop_rk3288.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <power/regulator.h>
#include "rk_vop.h"

DECLARE_GLOBAL_DATA_PTR;

enum vop_pol {
	HSYNC_POSITIVE = 0,
	VSYNC_POSITIVE = 1,
	DEN_NEGATIVE   = 2,
	DCLK_INVERT    = 3
};

static void rkvop_enable(struct rk3288_vop *regs, ulong fbbase,
			 int fb_bits_per_pixel,
			 const struct display_timing *edid)
{
	u32 lb_mode;
	u32 rgb_mode;
	u32 hactive = edid->hactive.typ;
	u32 vactive = edid->vactive.typ;

	writel(V_ACT_WIDTH(hactive - 1) | V_ACT_HEIGHT(vactive - 1),
	       &regs->win0_act_info);

	writel(V_DSP_XST(edid->hsync_len.typ + edid->hback_porch.typ) |
	       V_DSP_YST(edid->vsync_len.typ + edid->vback_porch.typ),
	       &regs->win0_dsp_st);

	writel(V_DSP_WIDTH(hactive - 1) |
		V_DSP_HEIGHT(vactive - 1),
		&regs->win0_dsp_info);

	clrsetbits_le32(&regs->win0_color_key, M_WIN0_KEY_EN | M_WIN0_KEY_COLOR,
			V_WIN0_KEY_EN(0) | V_WIN0_KEY_COLOR(0));

	switch (fb_bits_per_pixel) {
	case 16:
		rgb_mode = RGB565;
		writel(V_RGB565_VIRWIDTH(hactive), &regs->win0_vir);
		break;
	case 24:
		rgb_mode = RGB888;
		writel(V_RGB888_VIRWIDTH(hactive), &regs->win0_vir);
		break;
	case 32:
	default:
		rgb_mode = ARGB8888;
		writel(V_ARGB888_VIRWIDTH(hactive), &regs->win0_vir);
		break;
	}

	if (hactive > 2560)
		lb_mode = LB_RGB_3840X2;
	else if (hactive > 1920)
		lb_mode = LB_RGB_2560X4;
	else if (hactive > 1280)
		lb_mode = LB_RGB_1920X5;
	else
		lb_mode = LB_RGB_1280X8;

	clrsetbits_le32(&regs->win0_ctrl0,
			M_WIN0_LB_MODE | M_WIN0_DATA_FMT | M_WIN0_EN,
			V_WIN0_LB_MODE(lb_mode) | V_WIN0_DATA_FMT(rgb_mode) |
			V_WIN0_EN(1));

	writel(fbbase, &regs->win0_yrgb_mst);
	writel(0x01, &regs->reg_cfg_done); /* enable reg config */
}

static void rkvop_set_pin_polarity(struct udevice *dev,
				   enum vop_modes mode, u32 polarity)
{
	struct rkvop_driverdata *ops =
		(struct rkvop_driverdata *)dev_get_driver_data(dev);

	if (ops->set_pin_polarity)
		ops->set_pin_polarity(dev, mode, polarity);
}

static void rkvop_enable_output(struct udevice *dev, enum vop_modes mode)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct rk3288_vop *regs = priv->regs;

	/* remove from standby */
	clrbits_le32(&regs->sys_ctrl, V_STANDBY_EN(1));

	switch (mode) {
	case VOP_MODE_HDMI:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_HDMI_OUT_EN(1));
		break;

	case VOP_MODE_EDP:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_EDP_OUT_EN(1));
		break;

	case VOP_MODE_LVDS:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_RGB_OUT_EN(1));
		break;

	case VOP_MODE_MIPI:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_MIPI_OUT_EN(1));
		break;

	default:
		debug("%s: unsupported output mode %x\n", __func__, mode);
	}
}

static void rkvop_mode_set(struct udevice *dev,
			   const struct display_timing *edid,
			   enum vop_modes mode)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct rk3288_vop *regs = priv->regs;
	struct rkvop_driverdata *data =
		(struct rkvop_driverdata *)dev_get_driver_data(dev);

	u32 hactive = edid->hactive.typ;
	u32 vactive = edid->vactive.typ;
	u32 hsync_len = edid->hsync_len.typ;
	u32 hback_porch = edid->hback_porch.typ;
	u32 vsync_len = edid->vsync_len.typ;
	u32 vback_porch = edid->vback_porch.typ;
	u32 hfront_porch = edid->hfront_porch.typ;
	u32 vfront_porch = edid->vfront_porch.typ;
	int mode_flags;
	u32 pin_polarity;

	pin_polarity = BIT(DCLK_INVERT);
	if (edid->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		pin_polarity |= BIT(HSYNC_POSITIVE);
	if (edid->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		pin_polarity |= BIT(VSYNC_POSITIVE);

	rkvop_set_pin_polarity(dev, mode, pin_polarity);
	rkvop_enable_output(dev, mode);

	mode_flags = 0;  /* RGB888 */
	if ((data->features & VOP_FEATURE_OUTPUT_10BIT) &&
	    (mode == VOP_MODE_HDMI || mode == VOP_MODE_EDP))
		mode_flags = 15;  /* RGBaaa */

	clrsetbits_le32(&regs->dsp_ctrl0, M_DSP_OUT_MODE,
			V_DSP_OUT_MODE(mode_flags));

	writel(V_HSYNC(hsync_len) |
	       V_HORPRD(hsync_len + hback_porch + hactive + hfront_porch),
			&regs->dsp_htotal_hs_end);

	writel(V_HEAP(hsync_len + hback_porch + hactive) |
	       V_HASP(hsync_len + hback_porch),
	       &regs->dsp_hact_st_end);

	writel(V_VSYNC(vsync_len) |
	       V_VERPRD(vsync_len + vback_porch + vactive + vfront_porch),
	       &regs->dsp_vtotal_vs_end);

	writel(V_VAEP(vsync_len + vback_porch + vactive)|
	       V_VASP(vsync_len + vback_porch),
	       &regs->dsp_vact_st_end);

	writel(V_HEAP(hsync_len + hback_porch + hactive) |
	       V_HASP(hsync_len + hback_porch),
	       &regs->post_dsp_hact_info);

	writel(V_VAEP(vsync_len + vback_porch + vactive)|
	       V_VASP(vsync_len + vback_porch),
	       &regs->post_dsp_vact_info);

	writel(0x01, &regs->reg_cfg_done); /* enable reg config */
}

/**
 * rk_display_init() - Try to enable the given display device
 *
 * This function performs many steps:
 * - Finds the display device being referenced by @ep_node
 * - Puts the VOP's ID into its uclass platform data
 * - Probes the device to set it up
 * - Reads the EDID timing information
 * - Sets up the VOP clocks, etc. for the selected pixel clock and display mode
 * - Enables the display (the display device handles this and will do different
 *     things depending on the display type)
 * - Tells the uclass about the display resolution so that the console will
 *     appear correctly
 *
 * @dev:	VOP device that we want to connect to the display
 * @fbbase:	Frame buffer address
 * @ep_node:	Device tree node to process - this is the offset of an endpoint
 *		node within the VOP's 'port' list.
 * @return 0 if OK, -ve if something went wrong
 */
static int rk_display_init(struct udevice *dev, ulong fbbase, ofnode ep_node)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rk_vop_priv *priv = dev_get_priv(dev);
	int vop_id, remote_vop_id;
	struct rk3288_vop *regs = priv->regs;
	struct display_timing timing;
	struct udevice *disp;
	int ret;
	u32 remote_phandle;
	struct display_plat *disp_uc_plat;
	struct clk clk;
	enum video_log2_bpp l2bpp;
	ofnode remote;

	debug("%s(%s, %lu, %s)\n", __func__,
	      dev_read_name(dev), fbbase, ofnode_get_name(ep_node));

	vop_id = ofnode_read_s32_default(ep_node, "reg", -1);
	debug("vop_id=%d\n", vop_id);
	ret = ofnode_read_u32(ep_node, "remote-endpoint", &remote_phandle);
	if (ret)
		return ret;

	remote = ofnode_get_by_phandle(remote_phandle);
	if (!ofnode_valid(remote))
		return -EINVAL;
	remote_vop_id = ofnode_read_u32_default(remote, "reg", -1);
	debug("remote vop_id=%d\n", remote_vop_id);

	/*
	 * The remote-endpoint references into a subnode of the encoder
	 * (i.e. HDMI, MIPI, etc.) with the DTS looking something like
	 * the following (assume 'hdmi_in_vopl' to be referenced):
	 *
	 * hdmi: hdmi@ff940000 {
	 *   ports {
	 *     hdmi_in: port {
	 *       hdmi_in_vopb: endpoint@0 { ... };
	 *       hdmi_in_vopl: endpoint@1 { ... };
	 *     }
	 *   }
	 * }
	 *
	 * The original code had 3 steps of "walking the parent", but
	 * a much better (as in: less likely to break if the DTS
	 * changes) way of doing this is to "find the enclosing device
	 * of UCLASS_DISPLAY".
	 */
	while (ofnode_valid(remote)) {
		remote = ofnode_get_parent(remote);
		if (!ofnode_valid(remote)) {
			debug("%s(%s): no UCLASS_DISPLAY for remote-endpoint\n",
			      __func__, dev_read_name(dev));
			return -EINVAL;
		}

		uclass_find_device_by_ofnode(UCLASS_DISPLAY, remote, &disp);
		if (disp)
			break;
	};

	disp_uc_plat = dev_get_uclass_platdata(disp);
	debug("Found device '%s', disp_uc_priv=%p\n", disp->name, disp_uc_plat);
	if (display_in_use(disp)) {
		debug("   - device in use\n");
		return -EBUSY;
	}

	disp_uc_plat->source_id = remote_vop_id;
	disp_uc_plat->src_dev = dev;

	ret = device_probe(disp);
	if (ret) {
		debug("%s: device '%s' display won't probe (ret=%d)\n",
		      __func__, dev->name, ret);
		return ret;
	}

	ret = display_read_timing(disp, &timing);
	if (ret) {
		debug("%s: Failed to read timings\n", __func__);
		return ret;
	}

	ret = clk_get_by_index(dev, 1, &clk);
	if (!ret)
		ret = clk_set_rate(&clk, timing.pixelclock.typ);
	if (IS_ERR_VALUE(ret)) {
		debug("%s: Failed to set pixel clock: ret=%d\n", __func__, ret);
		return ret;
	}

	/* Set bitwidth for vop display according to vop mode */
	switch (vop_id) {
	case VOP_MODE_EDP:
	case VOP_MODE_LVDS:
		l2bpp = VIDEO_BPP16;
		break;
	case VOP_MODE_HDMI:
	case VOP_MODE_MIPI:
		l2bpp = VIDEO_BPP32;
		break;
	default:
		l2bpp = VIDEO_BPP16;
	}

	rkvop_mode_set(dev, &timing, vop_id);
	rkvop_enable(regs, fbbase, 1 << l2bpp, &timing);

	ret = display_enable(disp, 1 << l2bpp, &timing);
	if (ret)
		return ret;

	uc_priv->xsize = timing.hactive.typ;
	uc_priv->ysize = timing.vactive.typ;
	uc_priv->bpix = l2bpp;
	debug("fb=%lx, size=%d %d\n", fbbase, uc_priv->xsize, uc_priv->ysize);

	return 0;
}

void rk_vop_probe_regulators(struct udevice *dev,
			     const char * const *names, int cnt)
{
	int i, ret;
	const char *name;
	struct udevice *reg;

	for (i = 0; i < cnt; ++i) {
		name = names[i];
		debug("%s: probing regulator '%s'\n", dev->name, name);

		ret = regulator_autoset_by_name(name, &reg);
		if (!ret)
			ret = regulator_set_enable(reg, true);
	}
}

int rk_vop_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct rk_vop_priv *priv = dev_get_priv(dev);
	int ret = 0;
	ofnode port, node;

	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	priv->regs = (struct rk3288_vop *)dev_read_addr(dev);

	/*
	 * Try all the ports until we find one that works. In practice this
	 * tries EDP first if available, then HDMI.
	 *
	 * Note that rockchip_vop_set_clk() always uses NPLL as the source
	 * clock so it is currently not possible to use more than one display
	 * device simultaneously.
	 */
	port = dev_read_subnode(dev, "port");
	if (!ofnode_valid(port)) {
		debug("%s(%s): 'port' subnode not found\n",
		      __func__, dev_read_name(dev));
		return -EINVAL;
	}

	for (node = ofnode_first_subnode(port);
	     ofnode_valid(node);
	     node = dev_read_next_subnode(node)) {
		ret = rk_display_init(dev, plat->base, node);
		if (ret)
			debug("Device failed: ret=%d\n", ret);
		if (!ret)
			break;
	}
	video_set_flush_dcache(dev, 1);

	return ret;
}

int rk_vop_bind(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);

	plat->size = 4 * (CONFIG_VIDEO_ROCKCHIP_MAX_XRES *
			  CONFIG_VIDEO_ROCKCHIP_MAX_YRES);

	return 0;
}
