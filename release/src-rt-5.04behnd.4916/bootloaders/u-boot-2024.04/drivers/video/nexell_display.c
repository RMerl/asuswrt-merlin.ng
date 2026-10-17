// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 *
 * Copyright (C) 2020  Stefan Bosch <stefan_b@posteo.net>
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <dm.h>
#include <mapmem.h>
#include <malloc.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <video.h>		/* For struct video_uc_plat */
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/display.h>
#include <asm/arch/display_dev.h>
#include "videomodes.h"

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_DM) && !defined(CONFIG_OF_CONTROL)
static struct nx_display_dev *dp_dev;
#endif

static char *const dp_dev_str[] = {
	[DP_DEVICE_RESCONV] = "RESCONV",
	[DP_DEVICE_RGBLCD] = "LCD",
	[DP_DEVICE_HDMI] = "HDMI",
	[DP_DEVICE_MIPI] = "MiPi",
	[DP_DEVICE_LVDS] = "LVDS",
	[DP_DEVICE_CVBS] = "TVOUT",
	[DP_DEVICE_DP0] = "DP0",
	[DP_DEVICE_DP1] = "DP1",
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static void nx_display_parse_dp_sync(ofnode node, struct dp_sync_info *sync)
{
	sync->h_active_len = ofnode_read_s32_default(node, "h_active_len", 0);
	sync->h_sync_width = ofnode_read_s32_default(node, "h_sync_width", 0);
	sync->h_back_porch = ofnode_read_s32_default(node, "h_back_porch", 0);
	sync->h_front_porch = ofnode_read_s32_default(node, "h_front_porch", 0);
	sync->h_sync_invert = ofnode_read_s32_default(node, "h_sync_invert", 0);
	sync->v_active_len = ofnode_read_s32_default(node, "v_active_len", 0);
	sync->v_sync_width = ofnode_read_s32_default(node, "v_sync_width", 0);
	sync->v_back_porch = ofnode_read_s32_default(node, "v_back_porch", 0);
	sync->v_front_porch = ofnode_read_s32_default(node, "v_front_porch", 0);
	sync->v_sync_invert = ofnode_read_s32_default(node, "v_sync_invert", 0);
	sync->pixel_clock_hz = ofnode_read_s32_default(node, "pixel_clock_hz", 0);

	debug("DP: sync ->\n");
	debug("ha:%d, hs:%d, hb:%d, hf:%d, hi:%d\n",
	      sync->h_active_len, sync->h_sync_width,
	      sync->h_back_porch, sync->h_front_porch, sync->h_sync_invert);
	debug("va:%d, vs:%d, vb:%d, vf:%d, vi:%d\n",
	      sync->v_active_len, sync->v_sync_width,
	      sync->v_back_porch, sync->v_front_porch, sync->v_sync_invert);
}

static void nx_display_parse_dp_ctrl(ofnode node, struct dp_ctrl_info *ctrl)
{
	/* clock gen */
	ctrl->clk_src_lv0 = ofnode_read_s32_default(node, "clk_src_lv0", 0);
	ctrl->clk_div_lv0 = ofnode_read_s32_default(node, "clk_div_lv0", 0);
	ctrl->clk_src_lv1 = ofnode_read_s32_default(node, "clk_src_lv1", 0);
	ctrl->clk_div_lv1 = ofnode_read_s32_default(node, "clk_div_lv1", 0);

	/* scan format */
	ctrl->interlace = ofnode_read_s32_default(node, "interlace", 0);

	/* syncgen format */
	ctrl->out_format = ofnode_read_s32_default(node, "out_format", 0);
	ctrl->invert_field = ofnode_read_s32_default(node, "invert_field", 0);
	ctrl->swap_RB = ofnode_read_s32_default(node, "swap_RB", 0);
	ctrl->yc_order = ofnode_read_s32_default(node, "yc_order", 0);

	/* extern sync delay */
	ctrl->delay_mask = ofnode_read_s32_default(node, "delay_mask", 0);
	ctrl->d_rgb_pvd = ofnode_read_s32_default(node, "d_rgb_pvd", 0);
	ctrl->d_hsync_cp1 = ofnode_read_s32_default(node, "d_hsync_cp1", 0);
	ctrl->d_vsync_fram = ofnode_read_s32_default(node, "d_vsync_fram", 0);
	ctrl->d_de_cp2 = ofnode_read_s32_default(node, "d_de_cp2", 0);

	/* extern sync delay */
	ctrl->vs_start_offset =
	    ofnode_read_s32_default(node, "vs_start_offset", 0);
	ctrl->vs_end_offset = ofnode_read_s32_default(node, "vs_end_offset", 0);
	ctrl->ev_start_offset =
	    ofnode_read_s32_default(node, "ev_start_offset", 0);
	ctrl->ev_end_offset = ofnode_read_s32_default(node, "ev_end_offset", 0);

	/* pad clock seletor */
	ctrl->vck_select = ofnode_read_s32_default(node, "vck_select", 0);
	ctrl->clk_inv_lv0 = ofnode_read_s32_default(node, "clk_inv_lv0", 0);
	ctrl->clk_delay_lv0 = ofnode_read_s32_default(node, "clk_delay_lv0", 0);
	ctrl->clk_inv_lv1 = ofnode_read_s32_default(node, "clk_inv_lv1", 0);
	ctrl->clk_delay_lv1 = ofnode_read_s32_default(node, "clk_delay_lv1", 0);
	ctrl->clk_sel_div1 = ofnode_read_s32_default(node, "clk_sel_div1", 0);

	debug("DP: ctrl [%s] ->\n",
	      ctrl->interlace ? "Interlace" : " Progressive");
	debug("cs0:%d, cd0:%d, cs1:%d, cd1:%d\n",
	      ctrl->clk_src_lv0, ctrl->clk_div_lv0,
	      ctrl->clk_src_lv1, ctrl->clk_div_lv1);
	debug("fmt:0x%x, inv:%d, swap:%d, yb:0x%x\n",
	      ctrl->out_format, ctrl->invert_field,
	      ctrl->swap_RB, ctrl->yc_order);
	debug("dm:0x%x, drp:%d, dhs:%d, dvs:%d, dde:0x%x\n",
	      ctrl->delay_mask, ctrl->d_rgb_pvd,
	      ctrl->d_hsync_cp1, ctrl->d_vsync_fram, ctrl->d_de_cp2);
	debug("vss:%d, vse:%d, evs:%d, eve:%d\n",
	      ctrl->vs_start_offset, ctrl->vs_end_offset,
	      ctrl->ev_start_offset, ctrl->ev_end_offset);
	debug("sel:%d, i0:%d, d0:%d, i1:%d, d1:%d, s1:%d\n",
	      ctrl->vck_select, ctrl->clk_inv_lv0, ctrl->clk_delay_lv0,
	      ctrl->clk_inv_lv1, ctrl->clk_delay_lv1, ctrl->clk_sel_div1);
}

static void nx_display_parse_dp_top_layer(ofnode node, struct dp_plane_top *top)
{
	top->screen_width = ofnode_read_s32_default(node, "screen_width", 0);
	top->screen_height = ofnode_read_s32_default(node, "screen_height", 0);
	top->video_prior = ofnode_read_s32_default(node, "video_prior", 0);
	top->interlace = ofnode_read_s32_default(node, "interlace", 0);
	top->back_color = ofnode_read_s32_default(node, "back_color", 0);
	top->plane_num = DP_PLANS_NUM;

	debug("DP: top [%s] ->\n",
	      top->interlace ? "Interlace" : " Progressive");
	debug("w:%d, h:%d, prior:%d, bg:0x%x\n",
	      top->screen_width, top->screen_height,
	      top->video_prior, top->back_color);
}

static void nx_display_parse_dp_layer(ofnode node, struct dp_plane_info *plane)
{
	plane->left = ofnode_read_s32_default(node, "left", 0);
	plane->width = ofnode_read_s32_default(node, "width", 0);
	plane->top = ofnode_read_s32_default(node, "top", 0);
	plane->height = ofnode_read_s32_default(node, "height", 0);
	plane->pixel_byte = ofnode_read_s32_default(node, "pixel_byte", 0);
	plane->format = ofnode_read_s32_default(node, "format", 0);
	plane->alpha_on = ofnode_read_s32_default(node, "alpha_on", 0);
	plane->alpha_depth = ofnode_read_s32_default(node, "alpha", 0);
	plane->tp_on = ofnode_read_s32_default(node, "tp_on", 0);
	plane->tp_color = ofnode_read_s32_default(node, "tp_color", 0);

	/* enable layer */
	if (plane->fb_base)
		plane->enable = 1;
	else
		plane->enable = 0;

	if (plane->fb_base == 0) {
		printf("fail : dp plane.%d invalid fb base [0x%x] ->\n",
		       plane->layer, plane->fb_base);
		return;
	}

	debug("DP: plane.%d [0x%x] ->\n", plane->layer, plane->fb_base);
	debug("f:0x%x, l:%d, t:%d, %d * %d, bpp:%d, a:%d(%d), t:%d(0x%x)\n",
	      plane->format, plane->left, plane->top, plane->width,
	      plane->height, plane->pixel_byte, plane->alpha_on,
	      plane->alpha_depth, plane->tp_on, plane->tp_color);
}

static void nx_display_parse_dp_planes(ofnode node,
				       struct nx_display_dev *dp,
				       struct video_uc_plat *plat)
{
	const char *name;
	ofnode subnode;

	ofnode_for_each_subnode(subnode, node) {
		name = ofnode_get_name(subnode);

		if (strcmp(name, "layer_top") == 0)
			nx_display_parse_dp_top_layer(subnode, &dp->top);

		/*
		 * TODO: Is it sure that only one layer is used? Otherwise
		 * fb_base must be different?
		 */
		if (strcmp(name, "layer_0") == 0) {
			dp->planes[0].fb_base =
			      (uint)map_sysmem(plat->base, plat->size);
			debug("%s(): dp->planes[0].fb_base == 0x%x\n", __func__,
			      (uint)dp->planes[0].fb_base);
			nx_display_parse_dp_layer(subnode, &dp->planes[0]);
		}

		if (strcmp(name, "layer_1") == 0) {
			dp->planes[1].fb_base =
			      (uint)map_sysmem(plat->base, plat->size);
			debug("%s(): dp->planes[1].fb_base == 0x%x\n", __func__,
			      (uint)dp->planes[1].fb_base);
			nx_display_parse_dp_layer(subnode, &dp->planes[1]);
		}

		if (strcmp(name, "layer_2") == 0) {
			dp->planes[2].fb_base =
			      (uint)map_sysmem(plat->base, plat->size);
			debug("%s(): dp->planes[2].fb_base == 0x%x\n", __func__,
			      (uint)dp->planes[2].fb_base);
			nx_display_parse_dp_layer(subnode, &dp->planes[2]);
		}
	}
}

static int nx_display_parse_dp_lvds(ofnode node, struct nx_display_dev *dp)
{
	struct dp_lvds_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev) {
		printf("failed to allocate display LVDS object.\n");
		return -ENOMEM;
	}

	dp->device = dev;

	dev->lvds_format = ofnode_read_s32_default(node, "format", 0);
	dev->pol_inv_hs = ofnode_read_s32_default(node, "pol_inv_hs", 0);
	dev->pol_inv_vs = ofnode_read_s32_default(node, "pol_inv_vs", 0);
	dev->pol_inv_de = ofnode_read_s32_default(node, "pol_inv_de", 0);
	dev->pol_inv_ck = ofnode_read_s32_default(node, "pol_inv_ck", 0);
	dev->voltage_level = ofnode_read_s32_default(node, "voltage_level", 0);

	if (!dev->voltage_level)
		dev->voltage_level = DEF_VOLTAGE_LEVEL;

	debug("DP: LVDS -> %s, voltage LV:0x%x\n",
	      dev->lvds_format == DP_LVDS_FORMAT_VESA ? "VESA" :
	      dev->lvds_format == DP_LVDS_FORMAT_JEIDA ? "JEIDA" : "LOC",
	      dev->voltage_level);
	debug("pol inv hs:%d, vs:%d, de:%d, ck:%d\n",
	      dev->pol_inv_hs, dev->pol_inv_vs,
	      dev->pol_inv_de, dev->pol_inv_ck);

	return 0;
}

static int nx_display_parse_dp_rgb(ofnode node, struct nx_display_dev *dp)
{
	struct dp_rgb_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev) {
		printf("failed to allocate display RGB LCD object.\n");
		return -ENOMEM;
	}
	dp->device = dev;

	dev->lcd_mpu_type = ofnode_read_s32_default(node, "lcd_mpu_type", 0);

	debug("DP: RGB -> MPU[%s]\n", dev->lcd_mpu_type ? "O" : "X");
	return 0;
}

static int nx_display_parse_dp_mipi(ofnode node, struct nx_display_dev *dp)
{
	struct dp_mipi_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev) {
		printf("failed to allocate display MiPi object.\n");
		return -ENOMEM;
	}
	dp->device = dev;

	dev->lp_bitrate = ofnode_read_s32_default(node, "lp_bitrate", 0);
	dev->hs_bitrate = ofnode_read_s32_default(node, "hs_bitrate", 0);
	dev->lpm_trans = 1;
	dev->command_mode = 0;

	debug("DP: MIPI ->\n");
	debug("lp:%dmhz, hs:%dmhz\n", dev->lp_bitrate, dev->hs_bitrate);

	return 0;
}

static int nx_display_parse_dp_hdmi(ofnode node, struct nx_display_dev *dp)
{
	struct dp_hdmi_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev) {
		printf("failed to allocate display HDMI object.\n");
		return -ENOMEM;
	}
	dp->device = dev;

	dev->preset = ofnode_read_s32_default(node, "preset", 0);

	debug("DP: HDMI -> %d\n", dev->preset);

	return 0;
}

static int nx_display_parse_dp_lcds(ofnode node, const char *type,
				    struct nx_display_dev *dp)
{
	if (strcmp(type, "lvds") == 0) {
		dp->dev_type = DP_DEVICE_LVDS;
		return nx_display_parse_dp_lvds(node, dp);
	} else if (strcmp(type, "rgb") == 0) {
		dp->dev_type = DP_DEVICE_RGBLCD;
		return nx_display_parse_dp_rgb(node, dp);
	} else if (strcmp(type, "mipi") == 0) {
		dp->dev_type = DP_DEVICE_MIPI;
		return nx_display_parse_dp_mipi(node, dp);
	} else if (strcmp(type, "hdmi") == 0) {
		dp->dev_type = DP_DEVICE_HDMI;
		return nx_display_parse_dp_hdmi(node, dp);
	}

	printf("%s: node %s unknown display type\n", __func__,
	       ofnode_get_name(node));
	return -EINVAL;

	return 0;
}

#define	DT_SYNC		(1 << 0)
#define	DT_CTRL		(1 << 1)
#define	DT_PLANES	(1 << 2)
#define	DT_DEVICE	(1 << 3)

static int nx_display_parse_dt(struct udevice *dev,
			       struct nx_display_dev *dp,
			       struct video_uc_plat *plat)
{
	const char *name, *dtype;
	int ret = 0;
	unsigned int dt_status = 0;
	ofnode subnode;

	if (!dev)
		return -ENODEV;

	dp->module = dev_read_s32_default(dev, "module", -1);
	if (dp->module == -1)
		dp->module = dev_read_s32_default(dev, "index", 0);

	dtype = dev_read_string(dev, "lcd-type");

	ofnode_for_each_subnode(subnode, dev_ofnode(dev)) {
		name = ofnode_get_name(subnode);

		if (strcmp("dp-sync", name) == 0) {
			dt_status |= DT_SYNC;
			nx_display_parse_dp_sync(subnode, &dp->sync);
		}

		if (strcmp("dp-ctrl", name) == 0) {
			dt_status |= DT_CTRL;
			nx_display_parse_dp_ctrl(subnode, &dp->ctrl);
		}

		if (strcmp("dp-planes", name) == 0) {
			dt_status |= DT_PLANES;
			nx_display_parse_dp_planes(subnode, dp, plat);
		}

		if (strcmp("dp-device", name) == 0) {
			dt_status |= DT_DEVICE;
			ret = nx_display_parse_dp_lcds(subnode, dtype, dp);
		}
	}

	if (dt_status != (DT_SYNC | DT_CTRL | DT_PLANES | DT_DEVICE)) {
		printf("Not enough DT config for display [0x%x]\n", dt_status);
		return -ENODEV;
	}

	return ret;
}
#endif

__weak int nx_display_fixup_dp(struct nx_display_dev *dp)
{
	return 0;
}

static struct nx_display_dev *nx_display_setup(void)
{
	struct nx_display_dev *dp;
	int i, ret;
	int node = 0;
	struct video_uc_plat *plat = NULL;

	struct udevice *dev;

	/* call driver probe */
	debug("DT: uclass device call...\n");

	ret = uclass_get_device(UCLASS_VIDEO, 0, &dev);
	if (ret) {
		debug("%s(): uclass_get_device(UCLASS_VIDEO, 0, &dev) != 0 --> return NULL\n",
		      __func__);
		return NULL;
	}
	plat = dev_get_uclass_plat(dev);
	if (!dev) {
		debug("%s(): dev_get_uclass_plat(dev) == NULL --> return NULL\n",
		      __func__);
		return NULL;
	}
	dp = dev_get_priv(dev);
	if (!dp) {
		debug("%s(): dev_get_priv(dev) == NULL --> return NULL\n",
		      __func__);
		return NULL;
	}
	node = dev_ofnode(dev).of_offset;

	if (CONFIG_IS_ENABLED(OF_CONTROL)) {
		ret = nx_display_parse_dt(dev, dp, plat);
		if (ret)
			goto err_setup;
	}

	nx_display_fixup_dp(dp);

	for (i = 0; dp->top.plane_num > i; i++) {
		dp->planes[i].layer = i;
		if (dp->planes[i].enable && !dp->fb_plane) {
			dp->fb_plane = &dp->planes[i];
			dp->fb_addr = dp->fb_plane->fb_base;
			dp->depth = dp->fb_plane->pixel_byte;
		}
	}

	switch (dp->dev_type) {
#ifdef CONFIG_VIDEO_NX_RGB
	case DP_DEVICE_RGBLCD:
		nx_rgb_display(dp->module,
			       &dp->sync, &dp->ctrl, &dp->top,
			       dp->planes, (struct dp_rgb_dev *)dp->device);
		break;
#endif
#ifdef CONFIG_VIDEO_NX_LVDS
	case DP_DEVICE_LVDS:
		nx_lvds_display(dp->module,
				&dp->sync, &dp->ctrl, &dp->top,
				dp->planes, (struct dp_lvds_dev *)dp->device);
		break;
#endif
#ifdef CONFIG_VIDEO_NX_MIPI
	case DP_DEVICE_MIPI:
		nx_mipi_display(dp->module,
				&dp->sync, &dp->ctrl, &dp->top,
				dp->planes, (struct dp_mipi_dev *)dp->device);
		break;
#endif
#ifdef CONFIG_VIDEO_NX_HDMI
	case DP_DEVICE_HDMI:
		nx_hdmi_display(dp->module,
				&dp->sync, &dp->ctrl, &dp->top,
				dp->planes, (struct dp_hdmi_dev *)dp->device);
		break;
#endif
	default:
		printf("fail : not support lcd type %d !!!\n", dp->dev_type);
		goto err_setup;
	};

	printf("LCD:   [%s] dp.%d.%d %dx%d %dbpp FB:0x%08x\n",
	       dp_dev_str[dp->dev_type], dp->module, dp->fb_plane->layer,
	       dp->fb_plane->width, dp->fb_plane->height, dp->depth * 8,
	       dp->fb_addr);

	return dp;

err_setup:
	kfree(dp);

	return NULL;
}

static int nx_display_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct nx_display_plat *plat = dev_get_plat(dev);
	char addr[64];

	debug("%s()\n", __func__);

	if (!dev)
		return -EINVAL;

	if (!uc_plat) {
		debug("%s(): video_uc_plat *plat == NULL --> return -EINVAL\n",
		      __func__);
		return -EINVAL;
	}

	if (!uc_priv) {
		debug("%s(): video_priv *uc_priv == NULL --> return -EINVAL\n",
		      __func__);
		return -EINVAL;
	}

	if (!plat) {
		debug("%s(): nx_display_plat *plat == NULL --> return -EINVAL\n",
		      __func__);
		return -EINVAL;
	}

	struct nx_display_dev *dp;

	dp = nx_display_setup();
	if (!dp) {
		debug("%s(): nx_display_setup() == 0 --> return -EINVAL\n",
		      __func__);
		return -EINVAL;
	}

	switch (dp->depth) {
	case 2:
		uc_priv->bpix = VIDEO_BPP16;
		break;
	case 3:
		/* There is no VIDEO_BPP24 because these values are of
		 * type video_log2_bpp
		 */
	case 4:
		uc_priv->bpix = VIDEO_BPP32;
		break;
	default:
		printf("fail : not support LCD bit per pixel %d\n",
		       dp->depth * 8);
		return -EINVAL;
	}

	uc_priv->xsize = dp->fb_plane->width;
	uc_priv->ysize = dp->fb_plane->height;
	uc_priv->rot = 0;

	/*
	 * set environment variable "fb_addr" (frame buffer address), required
	 * for splash image, which is not set if CONFIG_VIDEO is enabled).
	 */
	sprintf(addr, "0x%x", dp->fb_addr);
	debug("%s(): env_set(\"fb_addr\", %s) ...\n", __func__, addr);
	env_set("fb_addr", addr);

	return 0;
}

static int nx_display_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	debug("%s()\n", __func__);

	/* Datasheet S5p4418:
	 *   Resolution up to 2048 x 1280, up to 12 Bit per color (HDMI)
	 * Actual (max.) size is 0x1000000 because in U-Boot nanopi2-2016.01
	 * "#define CFG_FB_ADDR  0x77000000" and next address is
	 * "#define BMP_LOAD_ADDR  0x78000000"
	 */
	plat->size = 0x1000000;

	return 0;
}

static const struct udevice_id nx_display_ids[] = {
	{.compatible = "nexell,nexell-display", },
	{}
};

U_BOOT_DRIVER(nexell_display) = {
	.name = "nexell-display",
	.id = UCLASS_VIDEO,
	.of_match = nx_display_ids,
	.plat_auto	= sizeof(struct nx_display_plat),
	.bind = nx_display_bind,
	.probe = nx_display_probe,
	.priv_auto	= sizeof(struct nx_display_dev),
};
