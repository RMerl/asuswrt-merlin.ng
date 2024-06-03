// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <video_osd.h>

int video_osd_get_info(struct udevice *dev, struct video_osd_info *info)
{
	struct video_osd_ops *ops = video_osd_get_ops(dev);

	return ops->get_info(dev, info);
}

int video_osd_set_mem(struct udevice *dev, uint col, uint row, u8 *buf,
		      size_t buflen, uint count)
{
	struct video_osd_ops *ops = video_osd_get_ops(dev);

	return ops->set_mem(dev, col, row, buf, buflen, count);
}

int video_osd_set_size(struct udevice *dev, uint col, uint row)
{
	struct video_osd_ops *ops = video_osd_get_ops(dev);

	return ops->set_size(dev, col, row);
}

int video_osd_print(struct udevice *dev, uint col, uint row, ulong color,
		    char *text)
{
	struct video_osd_ops *ops = video_osd_get_ops(dev);

	return ops->print(dev, col, row, color, text);
}

UCLASS_DRIVER(video_osd) = {
	.id		= UCLASS_VIDEO_OSD,
	.name		= "video_osd",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};
