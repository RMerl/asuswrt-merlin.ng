// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <api_public.h>
#include <log.h>

/* TODO(clchiou): add support of video device */

int display_get_info(int type, struct display_info *di)
{
	if (!di)
		return API_EINVAL;

	switch (type) {
	default:
		debug("%s: unsupport display device type: %d\n",
				__FILE__, type);
		return API_ENODEV;
	}

	di->type = type;
	return 0;
}

int display_draw_bitmap(ulong bitmap, int x, int y)
{
	if (!bitmap)
		return API_EINVAL;
	return API_ENODEV;
}

void display_clear(void)
{
}
