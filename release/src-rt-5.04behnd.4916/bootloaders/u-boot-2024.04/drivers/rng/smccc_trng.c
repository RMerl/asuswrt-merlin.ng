// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_RNG

#include <common.h>
#include <dm.h>
#include <linker_lists.h>
#include <log.h>
#include <rng.h>
#include <dm/device_compat.h>
#include <linux/arm-smccc.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/psci.h>

#define DRIVER_NAME	"smccc-trng"

/**
 * Arm SMCCC TRNG firmware interface specification:
 * https://developer.arm.com/documentation/den0098/latest/
 */
#define ARM_SMCCC_TRNG_VERSION		0x84000050
#define ARM_SMCCC_TRNG_FEATURES		0x84000051
#define ARM_SMCCC_TRNG_GET_UUID		0x84000052
#define ARM_SMCCC_TRNG_RND_32		0x84000053
#define ARM_SMCCC_TRNG_RND_64		0xC4000053

#define ARM_SMCCC_RET_TRNG_SUCCESS		((ulong)0)
#define ARM_SMCCC_RET_TRNG_NOT_SUPPORTED	((ulong)-1)
#define ARM_SMCCC_RET_TRNG_INVALID_PARAMETER	((ulong)-2)
#define ARM_SMCCC_RET_TRNG_NO_ENTROPY		((ulong)-3)

#define TRNG_MAJOR_MASK		GENMASK(30, 16)
#define TRNG_MAJOR_SHIFT	16
#define TRNG_MINOR_MASK		GENMASK(15, 0)
#define TRNG_MINOR_SHIFT	0

#define TRNG_MAX_RND_64		(192 / 8)
#define TRNG_MAX_RND_32		(96 / 8)

/**
 * struct smccc_trng_priv - Private data for SMCCC TRNG support
 *
 * @smc64 - True if TRNG_RND_64 is supported, false if TRNG_RND_32 is supported
 */
struct smccc_trng_priv {
	bool smc64;
};

/*
 * Copy random bytes from ulong SMCCC output register to target buffer
 * Defines 2 function flavors for whether ARM_SMCCC_TRNG_RND_32 or
 * ARM_SMCCC_TRNG_RND_64 was used to invoke the service.
 */
static size_t smc32_copy_sample(u8 **ptr, size_t size, ulong *rnd)
{
	size_t len = min(size, sizeof(u32));
	u32 sample = *rnd;

	memcpy(*ptr, &sample, len);
	*ptr += len;

	return size - len;
}

static size_t smc64_copy_sample(u8 **ptr, size_t size, ulong *rnd)
{
	size_t len = min(size, sizeof(u64));
	u64 sample = *rnd;

	memcpy(*ptr, &sample, len);
	*ptr += len;

	return size - len;
}

static int smccc_trng_read(struct udevice *dev, void *data, size_t len)
{
	struct psci_plat_data *smccc = dev_get_parent_plat(dev);
	struct smccc_trng_priv *priv = dev_get_priv(dev);
	struct arm_smccc_res res;
	u32 func_id;
	u8 *ptr = data;
	size_t rem = len;
	size_t max_sz;
	size_t (*copy_sample)(u8 **ptr, size_t size, ulong *rnd);

	if (priv->smc64) {
		copy_sample = smc64_copy_sample;
		func_id = ARM_SMCCC_TRNG_RND_64;
		max_sz = TRNG_MAX_RND_64;
	} else {
		copy_sample = smc32_copy_sample;
		func_id = ARM_SMCCC_TRNG_RND_32;
		max_sz = TRNG_MAX_RND_32;
	}

	while (rem) {
		size_t sz = min(rem, max_sz);

		smccc->invoke_fn(func_id, sz * 8, 0, 0, 0, 0, 0, 0, &res);

		switch (res.a0) {
		case ARM_SMCCC_RET_TRNG_SUCCESS:
			break;
		case ARM_SMCCC_RET_TRNG_NO_ENTROPY:
			continue;
		default:
			return -EIO;
		}

		rem -= sz;

		sz = copy_sample(&ptr, sz, &res.a3);
		if (sz)
			sz = copy_sample(&ptr, sz, &res.a2);
		if (sz)
			sz = copy_sample(&ptr, sz, &res.a1);
	}

	return 0;
}

static const struct dm_rng_ops smccc_trng_ops = {
	.read = smccc_trng_read,
};

static bool smccc_trng_is_supported(void (*invoke_fn)(unsigned long a0, unsigned long a1,
						      unsigned long a2, unsigned long a3,
						      unsigned long a4, unsigned long a5,
						      unsigned long a6, unsigned long a7,
						      struct arm_smccc_res *res))
{
	struct arm_smccc_res res;

	(*invoke_fn)(ARM_SMCCC_ARCH_FEATURES, ARM_SMCCC_TRNG_VERSION, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 == ARM_SMCCC_RET_NOT_SUPPORTED)
		return false;

	(*invoke_fn)(ARM_SMCCC_TRNG_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 & BIT(31))
		return false;

	/* Test 64bit interface and fallback to 32bit interface */
	invoke_fn(ARM_SMCCC_TRNG_FEATURES, ARM_SMCCC_TRNG_RND_64,
		  0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == ARM_SMCCC_RET_TRNG_NOT_SUPPORTED)
		invoke_fn(ARM_SMCCC_TRNG_FEATURES, ARM_SMCCC_TRNG_RND_32,
			  0, 0, 0, 0, 0, 0, &res);

	return res.a0 == ARM_SMCCC_RET_TRNG_SUCCESS;
}

ARM_SMCCC_FEATURE_DRIVER(smccc_trng) = {
	.driver_name = DRIVER_NAME,
	.is_supported = smccc_trng_is_supported,
};

static int smccc_trng_probe(struct udevice *dev)
{
	struct psci_plat_data *smccc = dev_get_parent_plat(dev);
	struct smccc_trng_priv *priv = dev_get_priv(dev);
	struct arm_smccc_res res;

	if (!(smccc_trng_is_supported(smccc->invoke_fn)))
		return -ENODEV;

	/* At least one of 64bit and 32bit interfaces is available */
	smccc->invoke_fn(ARM_SMCCC_TRNG_FEATURES, ARM_SMCCC_TRNG_RND_64,
			 0, 0, 0, 0, 0, 0, &res);
	priv->smc64 = (res.a0 == ARM_SMCCC_RET_TRNG_SUCCESS);

#ifdef DEBUG
	smccc->invoke_fn(ARM_SMCCC_TRNG_GET_UUID, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != ARM_SMCCC_RET_TRNG_NOT_SUPPORTED) {
		unsigned long uuid_a0 = res.a0;
		unsigned long uuid_a1 = res.a1;
		unsigned long uuid_a2 = res.a2;
		unsigned long uuid_a3 = res.a3;
		unsigned long major, minor;

		smccc->invoke_fn(ARM_SMCCC_TRNG_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);
		major = (res.a0 & TRNG_MAJOR_MASK) >> TRNG_MAJOR_SHIFT;
		minor = (res.a0 & TRNG_MINOR_MASK) >> TRNG_MINOR_SHIFT;

		dev_dbg(dev, "Version %lu.%lu, UUID %08lx-%04lx-%04lx-%04lx-%04lx%08lx\n",
			major, minor, uuid_a0, uuid_a1 >> 16, uuid_a1 & GENMASK(16, 0),
			uuid_a2 >> 16, uuid_a2 & GENMASK(16, 0), uuid_a3);
	} else {
		dev_warn(dev, "Can't get TRNG UUID\n");
	}
#endif

	return 0;
}

U_BOOT_DRIVER(smccc_trng) = {
	.name = DRIVER_NAME,
	.id = UCLASS_RNG,
	.ops = &smccc_trng_ops,
	.probe = smccc_trng_probe,
	.priv_auto = sizeof(struct smccc_trng_priv),
};
