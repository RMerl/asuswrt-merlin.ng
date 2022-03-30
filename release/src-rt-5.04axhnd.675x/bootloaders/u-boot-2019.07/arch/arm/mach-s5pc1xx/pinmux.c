// SPDX-License-Identifier: GPL-2.0+
/*
 * Dummy functions to keep s5p_goni building (although it won't work)
 *
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <asm/arch/pinmux.h>

int exynos_pinmux_config(int peripheral, int flags)
{
	return 0;
}

int pinmux_decode_periph_id(const void *blob, int node)
{
	return 0;
}
