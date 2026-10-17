// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004 Freescale Semiconductor.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include "cadmus.h"

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif
