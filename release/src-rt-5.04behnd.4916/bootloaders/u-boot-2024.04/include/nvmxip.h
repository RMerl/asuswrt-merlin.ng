/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#ifndef __DRIVER_NVMXIP_H__
#define __DRIVER_NVMXIP_H__

#include <blk.h>

#define NVMXIP_BLKDRV_NAME    "nvmxip-blk"
#define NVMXIP_BLKDEV_NAME_SZ 20

/**
 * struct nvmxip_plat - the NVMXIP driver plat
 *
 * @phys_base:	NVM XIP device base address
 * @lba_shift:	block size shift count
 * @lba:	number of blocks
 *
 * The NVMXIP information read from the DT.
 */
struct nvmxip_plat {
	phys_addr_t phys_base;
	u32 lba_shift;
	lbaint_t lba;
};

/**
 * nvmxip_bind() - post binding treatments
 * @dev:	the NVMXIP device
 *
 * Create and probe a child block device.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int nvmxip_probe(struct udevice *udev);

#endif /* __DRIVER_NVMXIP_H__ */
