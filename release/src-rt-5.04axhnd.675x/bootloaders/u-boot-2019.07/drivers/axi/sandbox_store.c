// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <axi.h>
#include <dm.h>

/**
 * struct sandbox_store_priv - Private data structure of a AXI store device
 * @store: The buffer holding the device's internal memory, which is read from
 *	   and written to using the driver's methods
 */
struct sandbox_store_priv {
	u8 *store;
};

/**
 * copy_axi_data() - Copy data from source to destination with a given AXI
 *		     transfer width
 * @src:  Pointer to the data source from where data will be read
 * @dst:  Pointer to the data destination where data will be written to
 * @size: Size of the data to be copied given by a axi_size_t enum value
 *
 * Return: 0 if OK, -ve on error
 */
static int copy_axi_data(void *src, void *dst, enum axi_size_t size)
{
	switch (size) {
	case AXI_SIZE_8:
		*((u8 *)dst) = *((u8 *)src);
		return 0;
	case AXI_SIZE_16:
		*((u16 *)dst) = be16_to_cpu(*((u16 *)src));
		return 0;
	case AXI_SIZE_32:
		*((u32 *)dst) = be32_to_cpu(*((u32 *)src));
		return 0;
	default:
		debug("%s: Unknown AXI transfer size '%d'\n", __func__, size);
		return -EINVAL;
	}
}

static int sandbox_store_read(struct udevice *dev, ulong address, void *data,
			      enum axi_size_t size)
{
	struct sandbox_store_priv *priv = dev_get_priv(dev);

	return copy_axi_data(priv->store + address, data, size);
}

static int sandbox_store_write(struct udevice *dev, ulong address, void *data,
			       enum axi_size_t size)
{
	struct sandbox_store_priv *priv = dev_get_priv(dev);

	return copy_axi_data(data, priv->store + address, size);
}

static int sandbox_store_get_store(struct udevice *dev, u8 **store)
{
	struct sandbox_store_priv *priv = dev_get_priv(dev);

	*store = priv->store;

	return 0;
}

static const struct udevice_id sandbox_store_ids[] = {
	{ .compatible = "sandbox,sandbox_store" },
	{ /* sentinel */ }
};

static const struct axi_emul_ops sandbox_store_ops = {
	.read = sandbox_store_read,
	.write = sandbox_store_write,
	.get_store = sandbox_store_get_store,
};

static int sandbox_store_probe(struct udevice *dev)
{
	struct sandbox_store_priv *priv = dev_get_priv(dev);
	u32 reg[2];
	int ret;

	ret = dev_read_u32_array(dev, "reg", reg, ARRAY_SIZE(reg));
	if (ret) {
		debug("%s: Could not read 'reg' property\n", dev->name);
		return -EINVAL;
	}

	/*
	 * Allocate the device's internal storage that will be read
	 * from/written to
	 */
	priv->store = calloc(reg[1], 1);
	if (!priv->store)
		return -ENOMEM;

	return 0;
}

static int sandbox_store_remove(struct udevice *dev)
{
	struct sandbox_store_priv *priv = dev_get_priv(dev);

	free(priv->store);

	return 0;
}

U_BOOT_DRIVER(sandbox_axi_store) = {
	.name           = "sandbox_axi_store",
	.id             = UCLASS_AXI_EMUL,
	.of_match       = sandbox_store_ids,
	.ops		= &sandbox_store_ops,
	.priv_auto_alloc_size = sizeof(struct sandbox_store_priv),
	.probe          = sandbox_store_probe,
	.remove		= sandbox_store_remove,
};
