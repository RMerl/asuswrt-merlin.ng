// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <linux/errno.h>
#include "nvmxip.h"

#include <asm/global_data.h>
DECLARE_GLOBAL_DATA_PTR;

#define NVMXIP_QSPI_DRV_NAME "nvmxip_qspi"

/**
 * nvmxip_qspi_of_to_plat() -read from DT
 * @dev:	the NVMXIP device
 *
 * Read from the DT the NVMXIP information.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int nvmxip_qspi_of_to_plat(struct udevice *dev)
{
	struct nvmxip_plat *plat = dev_get_plat(dev);
	int ret;

	plat->phys_base = (phys_addr_t)dev_read_addr(dev);
	if (plat->phys_base == FDT_ADDR_T_NONE) {
		log_err("[%s]: can not get base address from device tree\n", dev->name);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "lba_shift", &plat->lba_shift);
	if (ret) {
		log_err("[%s]: can not get lba_shift from device tree\n", dev->name);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "lba", (u32 *)&plat->lba);
	if (ret) {
		log_err("[%s]: can not get lba from device tree\n", dev->name);
		return -EINVAL;
	}

	log_debug("[%s]: XIP device base addr: 0x%p , lba_shift: %d , lbas: %lu\n",
		  dev->name, (void *)(uintptr_t)plat->phys_base, plat->lba_shift, plat->lba);

	return 0;
}

static const struct udevice_id nvmxip_qspi_ids[] = {
	{ .compatible = "nvmxip,qspi" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(nvmxip_qspi) = {
	.name = NVMXIP_QSPI_DRV_NAME,
	.id = UCLASS_NVMXIP,
	.of_match = nvmxip_qspi_ids,
	.of_to_plat = nvmxip_qspi_of_to_plat,
	.probe = nvmxip_probe,
	.plat_auto = sizeof(struct nvmxip_plat),
};
