// SPDX-License-Identifier: GPL-2.0+
/*
 * Common code for Amlogic SimpleFB with pipeline.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2017 Icenowy Zheng <icenowy@aosc.io>
 */

#include <fdtdec.h>

int meson_simplefb_fdt_match(void *blob, const char *pipeline)
{
	int offset, ret;

	/* Find a prefilled simpefb node, matching out pipeline config */
	offset = fdt_node_offset_by_compatible(blob, -1,
					       "amlogic,simple-framebuffer");
	while (offset >= 0) {
		ret = fdt_stringlist_search(blob, offset, "amlogic,pipeline",
					    pipeline);
		if (ret == 0)
			break;
		offset = fdt_node_offset_by_compatible(blob, offset,
						"amlogic,simple-framebuffer");
	}

	return offset;
}
