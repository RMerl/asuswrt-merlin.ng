// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */
#include <common.h>
#include <display.h>
#include <dm.h>
#include <video_osd.h>

#include "sandbox_osd.h"

struct sandbox_osd_priv {
	uint width;
	uint height;
	u16 *buf;
};

static const struct udevice_id sandbox_osd_ids[] = {
	{ .compatible = "sandbox,sandbox_osd" },
	{ }
};

inline u16 make_memval(u8 chr, u8 color)
{
	return chr * 0x100 + color;
}

int sandbox_osd_get_info(struct udevice *dev, struct video_osd_info *info)
{
	struct sandbox_osd_priv *priv = dev_get_priv(dev);

	info->width = priv->width;
	info->height = priv->height;
	info->major_version = 1;
	info->minor_version = 0;

	return 0;
}

int sandbox_osd_set_mem(struct udevice *dev, uint col, uint row, u8 *buf,
			size_t buflen, uint count)
{
	struct sandbox_osd_priv *priv = dev_get_priv(dev);
	int pos;
	u8 *mem = (u8 *)priv->buf;
	int i;

	pos = 2 * (row * priv->width + col);

	if (pos >= 2 * (priv->width * priv->height))
		return -EINVAL;

	for (i = 0; i < count; i++)
		memcpy(mem + pos + (i * buflen), buf, buflen);

	return 0;
}

int _sandbox_osd_set_size(struct udevice *dev, uint col, uint row)
{
	struct sandbox_osd_priv *priv = dev_get_priv(dev);
	int i;
	uint size;

	priv->width = col;
	priv->height = row;
	size = priv->width * priv->height;
	if (!priv->buf)
		priv->buf = calloc(size, sizeof(u16));
	else
		priv->buf = realloc(priv->buf, size * sizeof(u16));

	if (!priv->buf)
		return -ENOMEM;

	/* Fill OSD with black spaces */
	for (i = 0; i < size; i++)
		priv->buf[i] = make_memval(' ', 'k');

	return 0;
}

int sandbox_osd_set_size(struct udevice *dev, uint col, uint row)
{
	return _sandbox_osd_set_size(dev, col, row);
}

int sandbox_osd_print(struct udevice *dev, uint col, uint row, ulong color,
		      char *text)
{
	struct sandbox_osd_priv *priv = dev_get_priv(dev);
	char cval;
	char *p;
	int pos;

	if (col >= priv->width || row >= priv->height)
		return -EINVAL;

	switch (color) {
	case COLOR_BLACK:
		cval = 'k';
		break;
	case COLOR_WHITE:
		cval = 'w';
		break;
	case COLOR_RED:
		cval = 'r';
		break;
	case COLOR_GREEN:
		cval = 'g';
		break;
	case COLOR_BLUE:
		cval = 'b';
		break;
	default:
		return -EINVAL;
	}

	p = text;
	pos = row * priv->width + col;

	while (*p)
		priv->buf[pos++] = make_memval(*(p++), cval);

	return 0;
}

int sandbox_osd_get_mem(struct udevice *dev, u8 *buf, size_t buflen)
{
	struct sandbox_osd_priv *priv = dev_get_priv(dev);
	uint memsize = 2 * (priv->width * priv->height);

	if (buflen < memsize)
		return -EINVAL;

	memcpy(buf, priv->buf, memsize);

	return 0;
}

static const struct video_osd_ops sandbox_osd_ops = {
	.get_info = sandbox_osd_get_info,
	.set_mem = sandbox_osd_set_mem,
	.set_size = sandbox_osd_set_size,
	.print = sandbox_osd_print,
};

int sandbox_osd_probe(struct udevice *dev)
{
	return _sandbox_osd_set_size(dev, 10, 10);
}

U_BOOT_DRIVER(sandbox_osd_drv) = {
	.name           = "sandbox_osd_drv",
	.id             = UCLASS_VIDEO_OSD,
	.ops		= &sandbox_osd_ops,
	.of_match       = sandbox_osd_ids,
	.probe          = sandbox_osd_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_osd_priv),
};
