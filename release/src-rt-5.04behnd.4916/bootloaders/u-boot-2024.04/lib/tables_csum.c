// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <linux/types.h>

u8 table_compute_checksum(const void *v, const int len)
{
	const u8 *bytes = v;
	u8 checksum = 0;
	int i;

	for (i = 0; i < len; i++)
		checksum -= bytes[i];

	return checksum;
}
