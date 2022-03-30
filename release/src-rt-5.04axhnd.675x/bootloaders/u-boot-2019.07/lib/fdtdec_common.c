// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on lib/fdtdec.c:
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <linux/libfdt.h>
#include <fdtdec.h>
#else
#include "libfdt.h"
#include "fdt_support.h"

#define debug(...)
#endif

int fdtdec_get_int(const void *blob, int node, const char *prop_name,
		int default_val)
{
	const int *cell;
	int len;

	debug("%s: %s: ", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && len >= sizeof(int)) {
		int val = fdt32_to_cpu(cell[0]);

		debug("%#x (%d)\n", val, val);
		return val;
	}
	debug("(not found)\n");
	return default_val;
}

unsigned int fdtdec_get_uint(const void *blob, int node, const char *prop_name,
			unsigned int default_val)
{
	const int *cell;
	int len;

	debug("%s: %s: ", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && len >= sizeof(unsigned int)) {
		unsigned int val = fdt32_to_cpu(cell[0]);

		debug("%#x (%d)\n", val, val);
		return val;
	}
	debug("(not found)\n");
	return default_val;
}
