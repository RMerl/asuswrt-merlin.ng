// SPDX-License-Identifier: GPL-2.0+
/*
 * PRNG driver for Qualcomm IPQ40xx
 *
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 * Based on Linux driver
 */

#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <linux/bitops.h>
#include <rng.h>

/* Device specific register offsets */
#define PRNG_DATA_OUT		0x0000
#define PRNG_STATUS			0x0004
#define PRNG_LFSR_CFG		0x0100
#define PRNG_CONFIG			0x0104

/* Device specific register masks and config values */
#define PRNG_LFSR_CFG_MASK		0x0000ffff
#define PRNG_LFSR_CFG_CLOCKS	0x0000dddd
#define PRNG_CONFIG_HW_ENABLE	BIT(1)
#define PRNG_STATUS_DATA_AVAIL	BIT(0)

#define MAX_HW_FIFO_DEPTH		16
#define MAX_HW_FIFO_SIZE		(MAX_HW_FIFO_DEPTH * 4)
#define WORD_SZ					4

struct msm_rng_priv {
	phys_addr_t base;
	struct clk clk;
};

static int msm_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct msm_rng_priv *priv = dev_get_priv(dev);
	size_t currsize = 0;
	u32 *retdata = data;
	size_t maxsize;
	u32 val;

	/* calculate max size bytes to transfer back to caller */
	maxsize = min_t(size_t, MAX_HW_FIFO_SIZE, len);

	/* read random data from hardware */
	do {
		val = readl_relaxed(priv->base + PRNG_STATUS);
		if (!(val & PRNG_STATUS_DATA_AVAIL))
			break;

		val = readl_relaxed(priv->base + PRNG_DATA_OUT);
		if (!val)
			break;

		*retdata++ = val;
		currsize += WORD_SZ;

		/* make sure we stay on 32bit boundary */
		if ((maxsize - currsize) < WORD_SZ)
			break;
	} while (currsize < maxsize);

	return 0;
}

static int msm_rng_enable(struct msm_rng_priv *priv, int enable)
{
	u32 val;

	if (enable) {
		/* Enable PRNG only if it is not already enabled */
		val = readl_relaxed(priv->base + PRNG_CONFIG);
		if (val & PRNG_CONFIG_HW_ENABLE) {
			val = readl_relaxed(priv->base + PRNG_LFSR_CFG);
			val &= ~PRNG_LFSR_CFG_MASK;
			val |= PRNG_LFSR_CFG_CLOCKS;
			writel(val, priv->base + PRNG_LFSR_CFG);

			val = readl_relaxed(priv->base + PRNG_CONFIG);
			val |= PRNG_CONFIG_HW_ENABLE;
			writel(val, priv->base + PRNG_CONFIG);
		}
	} else {
		val = readl_relaxed(priv->base + PRNG_CONFIG);
		val &= ~PRNG_CONFIG_HW_ENABLE;
		writel(val, priv->base + PRNG_CONFIG);
	}

	return 0;
}

static int msm_rng_probe(struct udevice *dev)
{
	struct msm_rng_priv *priv = dev_get_priv(dev);

	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret < 0)
		return ret;

	return msm_rng_enable(priv, 1);
}

static int msm_rng_remove(struct udevice *dev)
{
	struct msm_rng_priv *priv = dev_get_priv(dev);

	return msm_rng_enable(priv, 0);
}

static const struct dm_rng_ops msm_rng_ops = {
	.read = msm_rng_read,
};

static const struct udevice_id msm_rng_match[] = {
	{ .compatible = "qcom,prng", },
	{},
};

U_BOOT_DRIVER(msm_rng) = {
	.name = "msm-rng",
	.id = UCLASS_RNG,
	.of_match = msm_rng_match,
	.ops = &msm_rng_ops,
	.probe = msm_rng_probe,
	.remove = msm_rng_remove,
	.priv_auto	= sizeof(struct msm_rng_priv),
};
