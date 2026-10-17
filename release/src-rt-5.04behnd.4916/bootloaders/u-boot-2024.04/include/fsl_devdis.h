/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_DEVDIS_H_
#define __FSL_DEVDIS_H_

struct devdis_table {
	char name[32];
	u32 offset;
	u32 mask;
};

void device_disable(const struct devdis_table *tbl, uint32_t num);

#endif
