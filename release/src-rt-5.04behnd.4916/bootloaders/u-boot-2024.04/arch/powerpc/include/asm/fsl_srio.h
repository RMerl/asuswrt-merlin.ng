/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 */

#ifndef _FSL_SRIO_H_
#define _FSL_SRIO_H_

#include <linux/log2.h>

enum atmu_size {
	ATMU_SIZE_4K = 0xb,
	ATMU_SIZE_8K,
	ATMU_SIZE_16K,
	ATMU_SIZE_32K,
	ATMU_SIZE_64K,
	ATMU_SIZE_128K,
	ATMU_SIZE_256K,
	ATMU_SIZE_512K,
	ATMU_SIZE_1M,
	ATMU_SIZE_2M,
	ATMU_SIZE_4M,
	ATMU_SIZE_8M,
	ATMU_SIZE_16M,
	ATMU_SIZE_32M,
	ATMU_SIZE_64M,
	ATMU_SIZE_128M,
	ATMU_SIZE_256M,
	ATMU_SIZE_512M,
	ATMU_SIZE_1G,
	ATMU_SIZE_2G,
	ATMU_SIZE_4G,
	ATMU_SIZE_8G,
	ATMU_SIZE_16G,
	ATMU_SIZE_32G,
	ATMU_SIZE_64G,
};

#define atmu_size_mask(sz)	(__ilog2_u64(sz) - 1)
#define atmu_size_bytes(x)	(1ULL << ((x & 0x3f) + 1))

extern void srio_init(void);
#ifdef CONFIG_FSL_CORENET
extern void srio_boot_master(int port);
extern void srio_boot_master_release_slave(int port);
#endif
#endif
