// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <video_bridge.h>

static int ptn3460_attach(struct udevice *dev)
{
	debug("%s: %s\n", __func__, dev->name);

	return video_bridge_set_active(dev, true);
}

struct video_bridge_ops ptn3460_ops = {
	.attach = ptn3460_attach,
};

static const struct udevice_id ptn3460_ids[] = {
	{ .compatible = "nxp,ptn3460", },
	{ }
};

U_BOOT_DRIVER(parade_ptn3460) = {
	.name	= "nmp_ptn3460",
	.id	= UCLASS_VIDEO_BRIDGE,
	.of_match = ptn3460_ids,
	.ops	= &ptn3460_ops,
};
