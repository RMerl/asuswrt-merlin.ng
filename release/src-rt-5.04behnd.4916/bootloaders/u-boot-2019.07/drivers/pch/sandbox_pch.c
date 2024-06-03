// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <pch.h>

struct sandbox_pch_priv {
	bool protect;
};

int sandbox_get_pch_spi_protect(struct udevice *dev)
{
	struct sandbox_pch_priv *priv = dev_get_priv(dev);

	return priv->protect;
}

static int sandbox_pch_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	*sbasep = 0x10;

	return 0;
}

static int sandbox_pch_set_spi_protect(struct udevice *dev, bool protect)
{
	struct sandbox_pch_priv *priv = dev_get_priv(dev);

	priv->protect = protect;

	return 0;
}

static int sandbox_pch_get_gpio_base(struct udevice *dev, u32 *gbasep)
{
	*gbasep = 0x20;

	return 0;
}

static int sandbox_pch_get_io_base(struct udevice *dev, u32 *iobasep)
{
	*iobasep = 0x30;

	return 0;
}

int sandbox_pch_ioctl(struct udevice *dev, enum pch_req_t req, void *data,
		      int size)
{
	switch (req) {
	case PCH_REQ_TEST1:
		return -ENOSYS;
	case PCH_REQ_TEST2:
		return *(char *)data;
	case PCH_REQ_TEST3:
		*(char *)data = 'x';
		return 1;
	default:
		return -ENOSYS;
	}
}

static const struct pch_ops sandbox_pch_ops = {
	.get_spi_base	= sandbox_pch_get_spi_base,
	.set_spi_protect = sandbox_pch_set_spi_protect,
	.get_gpio_base	= sandbox_pch_get_gpio_base,
	.get_io_base = sandbox_pch_get_io_base,
	.ioctl		= sandbox_pch_ioctl,
};

static const struct udevice_id sandbox_pch_ids[] = {
	{ .compatible = "sandbox,pch" },
	{ }
};

U_BOOT_DRIVER(sandbox_pch_drv) = {
	.name		= "sandbox-pch",
	.id		= UCLASS_PCH,
	.of_match	= sandbox_pch_ids,
	.ops		= &sandbox_pch_ops,
	.priv_auto_alloc_size	= sizeof(struct sandbox_pch_priv),
};
