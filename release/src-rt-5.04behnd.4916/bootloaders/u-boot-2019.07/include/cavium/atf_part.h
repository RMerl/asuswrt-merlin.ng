/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#ifndef __ATF_PART_H__
#define __ATF_PART_H__

struct storage_partition {
	unsigned int type;
	unsigned int size;
	unsigned long offset;
};

enum {
	PARTITION_NBL1FW_REST = 0,
	PARTITION_BL2_BL31 = 1,
	PARTITION_UBOOT = 2,
	PARTITION_UEFI = 2,
	PARTITION_KERNEL = 3,
	PARTITION_DEVICE_TREE = 4,
	PARTITION_LAST,
};

#endif
