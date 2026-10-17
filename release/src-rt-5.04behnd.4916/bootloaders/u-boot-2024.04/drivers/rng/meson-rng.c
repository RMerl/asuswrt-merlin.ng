// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Driver for Amlogic hardware random number generator
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <rng.h>
#include <asm/io.h>
#include <linux/iopoll.h>

#define RNG_DATA	0x00
#define RNG_S4_DATA	0x08
#define RNG_S4_CFG	0x00

#define RUN_BIT		BIT(0)
#define SEED_READY_STS_BIT	BIT(31)

struct meson_rng_priv {
	u32 (*read)(fdt_addr_t base);
};

struct meson_rng_plat {
	fdt_addr_t base;
	struct clk clk;
	struct meson_rng_priv *priv;
};

/**
 * meson_rng_read() - fill buffer with random bytes
 *
 * @buffer:	buffer to receive data
 * @size:	size of buffer
 *
 * Return:	0
 */
static int meson_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct meson_rng_plat *pdata = dev_get_plat(dev);
	struct meson_rng_priv *priv = pdata->priv;
	char *buffer = (char *)data;

	while (len) {
		u32 rand = priv->read(pdata->base);
		size_t step;

		if (len >= 4)
			step = 4;
		else
			step = len;
		memcpy(buffer, &rand, step);
		buffer += step;
		len -= step;
	}
	return 0;
}

static int meson_rng_wait_status(void __iomem *cfg_addr, int bit)
{
	u32 status = 0;
	int ret;

	ret = readl_relaxed_poll_timeout(cfg_addr,
					 status, !(status & bit),
					 10000);
	if (ret)
		return -EBUSY;

	return 0;
}

static u32 meson_common_rng_read(fdt_addr_t base)
{
	return readl(base);
}

static u32 meson_s4_rng_read(fdt_addr_t base)
{
	void __iomem *cfg_addr = (void *)base + RNG_S4_CFG;
	int err;

	writel_relaxed(readl_relaxed(cfg_addr) | SEED_READY_STS_BIT, cfg_addr);

	err = meson_rng_wait_status(cfg_addr, SEED_READY_STS_BIT);
	if (err) {
		pr_err("Seed isn't ready, try again\n");
		return err;
	}

	err = meson_rng_wait_status(cfg_addr, RUN_BIT);
	if (err) {
		pr_err("Can't get random number, try again\n");
		return err;
	}

	return readl_relaxed(base + RNG_S4_DATA);
}

/**
 * meson_rng_probe() - probe rng device
 *
 * @dev:	device
 * Return:	0 if ok
 */
static int meson_rng_probe(struct udevice *dev)
{
	struct meson_rng_plat *pdata = dev_get_plat(dev);
	int err;

	err = clk_enable(&pdata->clk);
	if (err)
		return err;

	pdata->priv = (struct meson_rng_priv *)dev_get_driver_data(dev);

	return 0;
}

/**
 * meson_rng_remove() - deinitialize rng device
 *
 * @dev:	device
 * Return:	0 if ok
 */
static int meson_rng_remove(struct udevice *dev)
{
	struct meson_rng_plat *pdata = dev_get_plat(dev);

	return clk_disable(&pdata->clk);
}

/**
 * meson_rng_of_to_plat() - transfer device tree data to plaform data
 *
 * @dev:	device
 * Return:	0 if ok
 */
static int meson_rng_of_to_plat(struct udevice *dev)
{
	struct meson_rng_plat *pdata = dev_get_plat(dev);
	int err;

	pdata->base = dev_read_addr(dev);
	if (!pdata->base)
		return -ENODEV;

	/* Get optional "core" clock */
	err = clk_get_by_name_optional(dev, "core", &pdata->clk);
	if (err)
		return err;

	return 0;
}

static const struct dm_rng_ops meson_rng_ops = {
	.read = meson_rng_read,
};

static const struct meson_rng_priv meson_rng_priv = {
	.read = meson_common_rng_read,
};

static const struct meson_rng_priv meson_rng_priv_s4 = {
	.read = meson_s4_rng_read,
};

static const struct udevice_id meson_rng_match[] = {
	{
		.compatible = "amlogic,meson-rng",
		.data = (ulong)&meson_rng_priv,
	},
	{
		.compatible = "amlogic,meson-s4-rng",
		.data = (ulong)&meson_rng_priv_s4,
	},
	{},
};

U_BOOT_DRIVER(meson_rng) = {
	.name = "meson-rng",
	.id = UCLASS_RNG,
	.of_match = meson_rng_match,
	.ops = &meson_rng_ops,
	.probe = meson_rng_probe,
	.remove = meson_rng_remove,
	.plat_auto	= sizeof(struct meson_rng_plat),
	.of_to_plat = meson_rng_of_to_plat,
};
