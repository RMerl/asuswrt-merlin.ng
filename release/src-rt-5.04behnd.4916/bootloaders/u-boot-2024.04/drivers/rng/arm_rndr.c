// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023, Arm Ltd.
 *
 * Use the (optional) ARMv8.5 RNDR register to provide random numbers to
 * U-Boot's UCLASS_RNG users.
 * Detection is done at runtime using the CPU ID registers.
 */

#define LOG_CATEGORY UCLASS_RNG

#include <common.h>
#include <dm.h>
#include <linux/kernel.h>
#include <rng.h>
#include <asm/system.h>

#define DRIVER_NAME	"arm-rndr"

static bool cpu_has_rndr(void)
{
	uint64_t reg;

	__asm__ volatile("mrs %0, ID_AA64ISAR0_EL1\n" : "=r" (reg));
	return !!(reg & ID_AA64ISAR0_EL1_RNDR);
}

/*
 * The system register name is RNDR, but this isn't widely known among older
 * toolchains, and also triggers errors because of it being an architecture
 * extension. Since we check the availability of the register before, it's
 * fine to use here, though.
 */
#define RNDR	"S3_3_C2_C4_0"

static uint64_t read_rndr(void)
{
	uint64_t reg;

	__asm__ volatile("mrs %0, " RNDR "\n" : "=r" (reg));

	return reg;
}

static int arm_rndr_read(struct udevice *dev, void *data, size_t len)
{
	uint64_t random;

	while (len) {
		int tocopy = min(sizeof(uint64_t), len);

		random = read_rndr();
		memcpy(data, &random, tocopy);
		len -= tocopy;
		data += tocopy;
	}

	return 0;
}

static const struct dm_rng_ops arm_rndr_ops = {
	.read = arm_rndr_read,
};

static int arm_rndr_bind(struct udevice *dev)
{
	if (!cpu_has_rndr())
		return -ENOENT;

	return 0;
}

U_BOOT_DRIVER(arm_rndr) = {
	.name = DRIVER_NAME,
	.id = UCLASS_RNG,
	.ops = &arm_rndr_ops,
	.bind = arm_rndr_bind,
};

U_BOOT_DRVINFO(cpu_arm_rndr) = {
	.name = DRIVER_NAME,
};
