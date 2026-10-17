// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Broadcom
 */
/*
 *
 */

#include <dm.h>
#include <malloc.h>
#include <linux/delay.h>
#include <rng.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/arm-smccc.h>
#include <dm/device_compat.h>
#include "ba_svc.h"

static int rng_smc_read(struct udevice *dev, void *buf, size_t size)
{
	return ba_rng_get_rand(buf, size);
}

static int rng_smc_remove(struct udevice *dev) { return 0; }

static int rng_smc_probe(struct udevice *dev)
{
	dev_info(dev, "hwrng smc registered\n");
	return 0;
}

static const struct udevice_id rng_smc_match[] = {
	{ .compatible = "brcm,rng-smc", },
	{},
};

static const struct dm_rng_ops rng_smc_ops = {
	.read = rng_smc_read,
};

U_BOOT_DRIVER(rng_smc) = {
	.name = "rng-smc",
	.id = UCLASS_RNG,
	.of_match = rng_smc_match,
	.ops = &rng_smc_ops,
	.probe = rng_smc_probe,
	.remove = rng_smc_remove,
};
