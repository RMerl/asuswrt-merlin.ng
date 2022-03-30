/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _TABLES_CSUM_H_
#define _TABLES_CSUM_H_

static inline u8 table_compute_checksum(void *v, int len)
{
	u8 *bytes = v;
	u8 checksum = 0;
	int i;

	for (i = 0; i < len; i++)
		checksum -= bytes[i];

	return checksum;
}

#endif
