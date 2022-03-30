// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <errno.h>

/* Board-specific header. */
#include <qts/pinmux_config.h>

void sysmgr_get_pinmux_table(const u8 **table, unsigned int *table_len)
{
	*table = sys_mgr_init_table;
	*table_len = ARRAY_SIZE(sys_mgr_init_table);
}
