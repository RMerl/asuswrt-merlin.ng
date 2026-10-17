// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023, Yixun Lan <dlan@gentoo.org>
 *
 */

#include <common.h>
#include <cpu_func.h>

int board_init(void)
{
	enable_caches();

	return 0;
}
