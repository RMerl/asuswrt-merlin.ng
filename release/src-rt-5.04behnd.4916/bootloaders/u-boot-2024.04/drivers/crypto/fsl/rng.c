// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Michael Walle <michael@walle.cc>
 *
 * Driver for Freescale Cryptographic Accelerator and Assurance
 * Module (CAAM) hardware random number generator.
 */

#include <asm/cache.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <rng.h>
#include <linux/kernel.h>
#include "desc_constr.h"
#include "jobdesc.h"
#include "jr.h"

#define CAAM_RNG_MAX_FIFO_STORE_SIZE 16
#define CAAM_RNG_DESC_LEN (3 * CAAM_CMD_SZ + CAAM_PTR_SZ)

struct caam_rng_priv {
	u32 desc[CAAM_RNG_DESC_LEN / 4];
	u8 data[CAAM_RNG_MAX_FIFO_STORE_SIZE] __aligned(ARCH_DMA_MINALIGN);
};

static int caam_rng_read_one(struct caam_rng_priv *priv)
{
	int size = ALIGN(CAAM_RNG_MAX_FIFO_STORE_SIZE, ARCH_DMA_MINALIGN);
	int ret;

	ret = run_descriptor_jr(priv->desc);
	if (ret < 0)
		return -EIO;

	invalidate_dcache_range((unsigned long)priv->data,
				(unsigned long)priv->data + size);

	return 0;
}

static int caam_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct caam_rng_priv *priv = dev_get_priv(dev);
	u8 *buffer = data;
	size_t size;
	int ret;

	while (len) {
		ret = caam_rng_read_one(priv);
		if (ret)
			return ret;

		size = min(len, (size_t)CAAM_RNG_MAX_FIFO_STORE_SIZE);

		memcpy(buffer, priv->data, size);
		buffer += size;
		len -= size;
	}

	return 0;
}

static int caam_rng_probe(struct udevice *dev)
{
	struct caam_rng_priv *priv = dev_get_priv(dev);
	ulong size = ALIGN(CAAM_RNG_DESC_LEN, ARCH_DMA_MINALIGN);

	inline_cnstr_jobdesc_rng(priv->desc, priv->data,
				 CAAM_RNG_MAX_FIFO_STORE_SIZE);
	flush_dcache_range((unsigned long)priv->desc,
			   (unsigned long)priv->desc + size);

	return 0;
}

static const struct dm_rng_ops caam_rng_ops = {
	.read = caam_rng_read,
};

U_BOOT_DRIVER(caam_rng) = {
	.name = "caam-rng",
	.id = UCLASS_RNG,
	.ops = &caam_rng_ops,
	.probe = caam_rng_probe,
	.priv_auto	= sizeof(struct caam_rng_priv),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
