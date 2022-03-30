// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on the gdsys osd driver, which is
 *
 * (C) Copyright 2010
 * Dirk Eibach, Guntermann & Drunck GmbH, dirk.eibach@gdsys.de
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <regmap.h>
#include <video_osd.h>
#include <asm/gpio.h>

static const uint MAX_X_CHARS = 53;
static const uint MAX_Y_CHARS = 26;
static const uint MAX_VIDEOMEM_WIDTH = 64;
static const uint MAX_VIDEOMEM_HEIGHT = 32;
static const uint CHAR_WIDTH = 12;
static const uint CHAR_HEIGHT = 18;

static const u16 BASE_WIDTH_MASK = 0x3f00;
static const uint BASE_WIDTH_SHIFT = 8;
static const u16 BASE_HEIGTH_MASK = 0x001f;
static const uint BASE_HEIGTH_SHIFT;

struct ihs_video_out_regs {
	/* Device version register */
	u16 versions;
	/* Device feature register */
	u16 features;
	/* Device control register */
	u16 control;
	/* Register controlling screen size */
	u16 xy_size;
	/* Register controlling screen scaling */
	u16 xy_scale;
	/* Register controlling screen x position */
	u16 x_pos;
	/* Register controlling screen y position */
	u16 y_pos;
};

#define ihs_video_out_set(map, member, val) \
	regmap_range_set(map, 1, struct ihs_video_out_regs, member, val)

#define ihs_video_out_get(map, member, valp) \
	regmap_range_get(map, 1, struct ihs_video_out_regs, member, valp)

enum {
	CONTROL_FILTER_BLACK = (0 << 0),
	CONTROL_FILTER_ORIGINAL = (1 << 0),
	CONTROL_FILTER_DARKER = (2 << 0),
	CONTROL_FILTER_GRAY = (3 << 0),

	CONTROL_MODE_PASSTHROUGH = (0 << 3),
	CONTROL_MODE_OSD = (1 << 3),
	CONTROL_MODE_AUTO = (2 << 3),
	CONTROL_MODE_OFF = (3 << 3),

	CONTROL_ENABLE_OFF = (0 << 6),
	CONTROL_ENABLE_ON = (1 << 6),
};

struct ihs_video_out_priv {
	/* Register map for OSD device */
	struct regmap *map;
	/* Pointer to video memory */
	u16 *vidmem;
	/* Display width in text columns */
	uint base_width;
	/* Display height in text rows */
	uint base_height;
	/* x-resolution of the display in pixels */
	uint res_x;
	/* y-resolution of the display in pixels */
	uint res_y;
	/* OSD's sync mode (resolution + frequency) */
	int sync_src;
	/* The display port output for this OSD */
	struct udevice *video_tx;
	/* The pixel clock generator for the display */
	struct udevice *clk_gen;
};

static const struct udevice_id ihs_video_out_ids[] = {
	{ .compatible = "gdsys,ihs_video_out" },
	{ }
};

/**
 * set_control() - Set the control register to a given value
 *
 * The current value of sync_src is preserved by the function automatically.
 *
 * @dev: the OSD device whose control register to set
 * @value: the 16-bit value to write to the control register
 * Return: 0
 */
static int set_control(struct udevice *dev, u16 value)
{
	struct ihs_video_out_priv *priv = dev_get_priv(dev);

	if (priv->sync_src)
		value |= ((priv->sync_src & 0x7) << 8);

	ihs_video_out_set(priv->map, control, value);

	return 0;
}

int ihs_video_out_get_info(struct udevice *dev, struct video_osd_info *info)
{
	struct ihs_video_out_priv *priv = dev_get_priv(dev);
	u16 versions;

	ihs_video_out_get(priv->map, versions, &versions);

	info->width = priv->base_width;
	info->height = priv->base_height;
	info->major_version = versions / 100;
	info->minor_version = versions % 100;

	return 0;
}

int ihs_video_out_set_mem(struct udevice *dev, uint col, uint row, u8 *buf,
			  size_t buflen, uint count)
{
	struct ihs_video_out_priv *priv = dev_get_priv(dev);
	int res;
	uint offset;
	uint k, rep;
	u16 data;

	/* Repetitions (controlled via count parmeter) */
	for (rep = 0; rep < count; ++rep) {
		offset = row * priv->base_width + col + rep * (buflen / 2);

		/* Write a single buffer copy */
		for (k = 0; k < buflen / 2; ++k) {
			uint max_size = priv->base_width * priv->base_height;

			if (offset + k >= max_size) {
				debug("%s: Write would be out of OSD bounds\n",
				      dev->name);
				return -E2BIG;
			}

			data = buf[2 * k + 1] + 256 * buf[2 * k];
			out_le16(priv->vidmem + offset + k, data);
		}
	}

	res = set_control(dev, CONTROL_FILTER_ORIGINAL |
			       CONTROL_MODE_OSD |
			       CONTROL_ENABLE_ON);
	if (res) {
		debug("%s: Could not set control register\n", dev->name);
		return res;
	}

	return 0;
}

/**
 * div2_u16() - Approximately divide a 16-bit number by 2
 *
 * @val: The 16-bit value to divide by two
 * Return: The approximate division of val by two
 */
static inline u16 div2_u16(u16 val)
{
	return (32767 * val) / 65535;
}

int ihs_video_out_set_size(struct udevice *dev, uint col, uint row)
{
	struct ihs_video_out_priv *priv = dev_get_priv(dev);

	if (!col || col > MAX_VIDEOMEM_WIDTH || col > MAX_X_CHARS ||
	    !row || row > MAX_VIDEOMEM_HEIGHT || row > MAX_Y_CHARS) {
		debug("%s: Desired OSD size invalid\n", dev->name);
		return -EINVAL;
	}

	ihs_video_out_set(priv->map, xy_size, ((col - 1) << 8) | (row - 1));
	/* Center OSD on screen */
	ihs_video_out_set(priv->map, x_pos,
			  div2_u16(priv->res_x - CHAR_WIDTH * col));
	ihs_video_out_set(priv->map, y_pos,
			  div2_u16(priv->res_y - CHAR_HEIGHT * row));

	return 0;
}

int ihs_video_out_print(struct udevice *dev, uint col, uint row, ulong color,
			char *text)
{
	int res;
	u8 buffer[2 * MAX_VIDEOMEM_WIDTH];
	uint k;
	uint charcount = strlen(text);
	uint len = min(charcount, 2 * MAX_VIDEOMEM_WIDTH);

	for (k = 0; k < len; ++k) {
		buffer[2 * k] = text[k];
		buffer[2 * k + 1] = color;
	}

	res = ihs_video_out_set_mem(dev, col, row, buffer, 2 * len, 1);
	if (res < 0) {
		debug("%s: Could not write to video memory\n", dev->name);
		return res;
	}

	return 0;
}

static const struct video_osd_ops ihs_video_out_ops = {
	.get_info = ihs_video_out_get_info,
	.set_mem = ihs_video_out_set_mem,
	.set_size = ihs_video_out_set_size,
	.print = ihs_video_out_print,
};

int ihs_video_out_probe(struct udevice *dev)
{
	struct ihs_video_out_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args phandle_args;
	const char *mode;
	u16 features;
	struct display_timing timing;
	int res;

	res = regmap_init_mem(dev_ofnode(dev), &priv->map);
	if (res) {
		debug("%s: Could not initialize regmap (err = %d)\n", dev->name,
		      res);
		return res;
	}

	/* Range with index 2 is video memory */
	priv->vidmem = regmap_get_range(priv->map, 2);

	mode = dev_read_string(dev, "mode");
	if (!mode) {
		debug("%s: Could not read mode property\n", dev->name);
		return -EINVAL;
	}

	if (!strcmp(mode, "1024_768_60")) {
		priv->sync_src = 2;
		priv->res_x = 1024;
		priv->res_y = 768;
		timing.hactive.typ = 1024;
		timing.vactive.typ = 768;
	} else if (!strcmp(mode, "720_400_70")) {
		priv->sync_src = 1;
		priv->res_x = 720;
		priv->res_y = 400;
		timing.hactive.typ = 720;
		timing.vactive.typ = 400;
	} else {
		priv->sync_src = 0;
		priv->res_x = 640;
		priv->res_y = 480;
		timing.hactive.typ = 640;
		timing.vactive.typ = 480;
	}

	ihs_video_out_get(priv->map, features, &features);

	res = set_control(dev, CONTROL_FILTER_ORIGINAL |
			       CONTROL_MODE_OSD |
			       CONTROL_ENABLE_OFF);
	if (res) {
		debug("%s: Could not set control register (err = %d)\n",
		      dev->name, res);
		return res;
	}

	priv->base_width = ((features & BASE_WIDTH_MASK)
			    >> BASE_WIDTH_SHIFT) + 1;
	priv->base_height = ((features & BASE_HEIGTH_MASK)
			     >> BASE_HEIGTH_SHIFT) + 1;

	res = dev_read_phandle_with_args(dev, "clk_gen", NULL, 0, 0,
					 &phandle_args);
	if (res) {
		debug("%s: Could not get clk_gen node (err = %d)\n",
		      dev->name, res);
		return -EINVAL;
	}

	res = uclass_get_device_by_ofnode(UCLASS_CLK, phandle_args.node,
					  &priv->clk_gen);
	if (res) {
		debug("%s: Could not get clk_gen dev (err = %d)\n",
		      dev->name, res);
		return -EINVAL;
	}

	res = dev_read_phandle_with_args(dev, "video_tx", NULL, 0, 0,
					 &phandle_args);
	if (res) {
		debug("%s: Could not get video_tx (err = %d)\n",
		      dev->name, res);
		return -EINVAL;
	}

	res = uclass_get_device_by_ofnode(UCLASS_DISPLAY, phandle_args.node,
					  &priv->video_tx);
	if (res) {
		debug("%s: Could not get video_tx dev (err = %d)\n",
		      dev->name, res);
		return -EINVAL;
	}

	res = display_enable(priv->video_tx, 8, &timing);
	if (res && res != -EIO) { /* Ignore missing DP sink error */
		debug("%s: Could not enable the display (err = %d)\n",
		      dev->name, res);
		return res;
	}

	return 0;
}

U_BOOT_DRIVER(ihs_video_out_drv) = {
	.name           = "ihs_video_out_drv",
	.id             = UCLASS_VIDEO_OSD,
	.ops		= &ihs_video_out_ops,
	.of_match       = ihs_video_out_ids,
	.probe          = ihs_video_out_probe,
	.priv_auto_alloc_size = sizeof(struct ihs_video_out_priv),
};
