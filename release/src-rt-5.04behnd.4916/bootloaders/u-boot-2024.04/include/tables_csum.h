/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _TABLES_CSUM_H_
#define _TABLES_CSUM_H_

/**
 * table_compute_checksum() - Compute a table checksum
 *
 * This computes an 8-bit checksum for the configuration table.
 * All bytes in the configuration table, including checksum itself and
 * reserved bytes must add up to zero.
 *
 * @v:		configuration table base address
 * @len:	configuration table size
 * @return:	the 8-bit checksum
 */
u8 table_compute_checksum(const void *v, const int len);

#endif
