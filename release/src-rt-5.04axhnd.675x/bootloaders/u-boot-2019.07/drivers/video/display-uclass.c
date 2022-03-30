// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Google Inc.
 */

#include <common.h>
#include <dm.h>
#include <display.h>
#include <edid.h>
#include <errno.h>

int display_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct dm_display_ops *ops = display_get_ops(dev);

	if (!ops || !ops->read_edid)
		return -ENOSYS;
	return ops->read_edid(dev, buf, buf_size);
}

int display_enable(struct udevice *dev, int panel_bpp,
			const struct display_timing *timing)
{
	struct dm_display_ops *ops = display_get_ops(dev);
	struct display_plat *disp_uc_plat;
	int ret;

	if (!ops || !ops->enable)
		return -ENOSYS;
	ret = ops->enable(dev, panel_bpp, timing);
	if (ret)
		return ret;

	disp_uc_plat = dev_get_uclass_platdata(dev);
	disp_uc_plat->in_use = true;

	return 0;
}

int display_read_timing(struct udevice *dev, struct display_timing *timing)
{
	struct dm_display_ops *ops = display_get_ops(dev);
	int panel_bits_per_colour;
	u8 buf[EDID_EXT_SIZE];
	int ret;

	if (ops && ops->read_timing)
		return ops->read_timing(dev, timing);

	if (!ops || !ops->read_edid)
		return -ENOSYS;
	ret = ops->read_edid(dev, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return edid_get_timing(buf, ret, timing, &panel_bits_per_colour);
}

bool display_in_use(struct udevice *dev)
{
	struct display_plat *disp_uc_plat = dev_get_uclass_platdata(dev);

	return disp_uc_plat->in_use;
}

UCLASS_DRIVER(display) = {
	.id		= UCLASS_DISPLAY,
	.name		= "display",
	.per_device_platdata_auto_alloc_size	= sizeof(struct display_plat),
};
