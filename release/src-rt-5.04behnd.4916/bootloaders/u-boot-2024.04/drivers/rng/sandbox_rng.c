// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019, Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <rand.h>
#include <rng.h>

#include <linux/string.h>

static int sandbox_rng_read(struct udevice *dev, void *data, size_t len)
{
	unsigned int i, seed, random;
	unsigned char *buf = data;
	size_t nrem, nloops;

	if (!len)
		return 0;

	nloops = len / sizeof(random);
	seed = get_timer(0) ^ rand();
	srand(seed);

	for (i = 0, nrem = len; i < nloops; i++) {
		random = rand();
		memcpy(buf, &random, sizeof(random));
		buf += sizeof(random);
		nrem -= sizeof(random);
	}

	if (nrem) {
		random = rand();
		memcpy(buf, &random, nrem);
	}

	return 0;
}

static const struct dm_rng_ops sandbox_rng_ops = {
	.read = sandbox_rng_read,
};

static const struct udevice_id sandbox_rng_match[] = {
	{
		.compatible = "sandbox,sandbox-rng",
	},
	{},
};

U_BOOT_DRIVER(sandbox_rng) = {
	.name = "sandbox-rng",
	.id = UCLASS_RNG,
	.of_match = sandbox_rng_match,
	.ops = &sandbox_rng_ops,
};
