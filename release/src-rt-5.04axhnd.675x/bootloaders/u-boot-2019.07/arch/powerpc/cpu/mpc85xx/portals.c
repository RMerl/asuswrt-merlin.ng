// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

#include <asm/processor.h>
#include <asm/io.h>

#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>

/* Update portal containter to match LAW setup of portal in phy map */
void fdt_portal(void *blob, const char *compat, const char *container,
			u64 addr, u32 size)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, compat);
	if (off < 0)
		return ;

	off = fdt_parent_offset(blob, off);
	/* if non-zero assume we have a container */
	if (off > 0) {
		char buf[60];
		const char *p, *name;
		u32 *range;
		int len;

		/* fixup ranges */
		range = fdt_getprop_w(blob, off, "ranges", &len);
		if (range == NULL) {
			printf("ERROR: container for %s has no ranges", compat);
			return ;
		}

		range[0] = 0;
		if (len == 16) {
			range[1] = addr >> 32;
			range[2] = addr & 0xffffffff;
			range[3] = size;
		} else {
			range[1] = addr & 0xffffffff;
			range[2] = size;
		}
		fdt_setprop_inplace(blob, off, "ranges", range, len);

		/* fixup the name */
		name = fdt_get_name(blob, off, &len);
		p = memchr(name, '@', len);

		if (p)
			len = p - name;

		/* if we are given a container name check it
		 * against what we found, if it doesnt match exit out */
		if (container && (memcmp(container, name, len))) {
			printf("WARNING: container names didn't match %s %s\n",
				container, name);
			return ;
		}

		memcpy(&buf, name, len);
		len += sprintf(&buf[len], "@%llx", addr);
		fdt_set_name(blob, off, buf);
		return ;
	}

	printf("ERROR: %s isn't in a container.  Not supported\n", compat);
}
