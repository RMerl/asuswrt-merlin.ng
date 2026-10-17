/*
* Copyright (C) 2023 Broadcom Corporation
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation version 2.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/
/*
 * DESCRIPTION: The Broadcom RNG Shim Driver
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

#define ARM_SMCCC_RNG_MIN_VERSION			0x10000UL

#define ARM_SMCCC_RNG_VERSION					\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_32,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x50)

#define ARM_SMCCC_RNG_RND32					\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_32,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x53)

#define ARM_SMCCC_RNG_RND64					\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_64,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x53)
#if defined(__aarch64__)
#define READ_RNG	ARM_SMCCC_RNG_RND64
#define READ_SIZE	8
#else
#define READ_RNG	ARM_SMCCC_RNG_RND32
#define READ_SIZE	4
#endif

static bool smccc_rng_shim_available = false;

static int rng_shim_read(struct udevice *dev, void *buf, size_t max)
{
	uint32_t num_remaining = max;

	if (smccc_rng_shim_available) {
		struct arm_smccc_res smc_res;

		while (num_remaining > 0) {
			arm_smccc_smc(READ_RNG, READ_SIZE * 8, 0, 0, 0, 0, 0, 0, &smc_res);
			memcpy(buf, &smc_res.a3 , (num_remaining > READ_SIZE) ? READ_SIZE : num_remaining);
			num_remaining -= READ_SIZE;
			buf += READ_SIZE;
		}
		return 0;
	}
	return -EINVAL;
}

static int rng_shim_remove(struct udevice *dev) { return 0; }

static int rng_shim_probe(struct udevice *dev)
{
	struct arm_smccc_res smc_res;

	arm_smccc_smc(ARM_SMCCC_RNG_VERSION, 0, 0, 0, 0, 0, 0, 0, &smc_res);
	if ((s32)smc_res.a0 < 0)
		smccc_rng_shim_available = false;
	else
		smccc_rng_shim_available = (smc_res.a0 >= ARM_SMCCC_RNG_MIN_VERSION);

	if (smccc_rng_shim_available) {
	  	dev_info(dev, "Using hwrng shim\n");
	}
	else {
		dev_err(dev, "failed to get rng resources\n");
		return -EINVAL;
	}
	dev_info(dev, "hwrng shim registered\n");

	return 0;
}

static const struct udevice_id rng_shim_match[] = {
	{ .compatible = "brcm,rng-shim", },
	{},
};

static const struct dm_rng_ops rng_shim_ops = {
	.read = rng_shim_read,
};

U_BOOT_DRIVER(iproc_rng200_rng) = {
	.name = "rng-shim",
	.id = UCLASS_RNG,
	.of_match = rng_shim_match,
	.ops = &rng_shim_ops,
	.probe = rng_shim_probe,
	.remove = rng_shim_remove,
};
