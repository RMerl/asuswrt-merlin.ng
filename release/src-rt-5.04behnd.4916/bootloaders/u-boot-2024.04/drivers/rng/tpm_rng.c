// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <dm.h>
#include <rng.h>
#include <tpm_api.h>

static int rng_tpm_random_read(struct udevice *dev, void *data, size_t count)
{
	return tpm_get_random(dev_get_parent(dev), data, count);
}

static const struct dm_rng_ops tpm_rng_ops = {
	.read = rng_tpm_random_read,
};

U_BOOT_DRIVER(tpm_rng) = {
	.name	= "tpm-rng",
	.id	= UCLASS_RNG,
	.ops	= &tpm_rng_ops,
};
