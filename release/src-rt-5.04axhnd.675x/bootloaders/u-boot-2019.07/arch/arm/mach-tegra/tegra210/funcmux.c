// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra210 high-level function multiplexing */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	/*
	 * Add other periph IDs here as needed.
	 * Note that all pinmux/pads should have already
	 * been set up in the board pinmux table in
	 * pinmux-config-<board>.h for all periphs.
	 * Leave this in for the odd case where a mux
	 * needs to be changed on-the-fly.
	 */

	default:
		debug("%s: invalid periph_id %d", __func__, id);
		return -1;
	}

	if (bad_config) {
		debug("%s: invalid config %d for periph_id %d", __func__,
		      config, id);
		return -1;
	}
	return 0;
}
