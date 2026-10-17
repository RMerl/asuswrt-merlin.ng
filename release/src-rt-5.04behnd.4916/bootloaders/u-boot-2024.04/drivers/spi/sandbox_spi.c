/*
 * Simulate a SPI port
 *
 * Copyright (c) 2011-2013 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#define LOG_CATEGORY UCLASS_SPI

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <os.h>

#include <linux/errno.h>
#include <asm/spi.h>
#include <asm/state.h>
#include <dm/acpi.h>
#include <dm/device-internal.h>

/**
 * struct sandbox_spi_priv - Sandbox SPI private data
 *
 * Helper struct to keep track of the sandbox SPI bus internal state. It is
 * used in unit tests to verify that dm spi functions update the bus
 * speed/mode properly (for instance, when jumping back and forth between spi
 * slaves claiming the bus, we need to make sure that the bus speed is updated
 * accordingly for each slave).
 *
 * @speed:	Current bus speed.
 * @mode:	Current bus mode.
 */
struct sandbox_spi_priv {
	uint speed;
	uint mode;
};

__weak int sandbox_spi_get_emul(struct sandbox_state *state,
				struct udevice *bus, struct udevice *slave,
				struct udevice **emulp)
{
	return -ENOENT;
}

uint sandbox_spi_get_speed(struct udevice *dev)
{
	struct sandbox_spi_priv *priv = dev_get_priv(dev);

	return priv->speed;
}

uint sandbox_spi_get_mode(struct udevice *dev)
{
	struct sandbox_spi_priv *priv = dev_get_priv(dev);

	return priv->mode;
}

static int sandbox_spi_xfer(struct udevice *slave, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = slave->parent;
	struct sandbox_state *state = state_get_current();
	struct dm_spi_emul_ops *ops;
	struct udevice *emul;
	uint bytes = bitlen / 8, i;
	int ret;
	uint busnum, cs;

	if (bitlen == 0)
		return 0;

	/* we can only do 8 bit transfers */
	if (bitlen % 8) {
		printf("sandbox_spi: xfer: invalid bitlen size %u; needs to be 8bit\n",
		       bitlen);
		return -EINVAL;
	}

	busnum = dev_seq(bus);
	cs = spi_chip_select(slave);
	if (busnum >= CONFIG_SANDBOX_SPI_MAX_BUS ||
	    cs >= CONFIG_SANDBOX_SPI_MAX_CS) {
		printf("%s: busnum=%u, cs=%u: out of range\n", __func__,
		       busnum, cs);
		return -ENOENT;
	}
	ret = sandbox_spi_get_emul(state, bus, slave, &emul);
	if (ret) {
		printf("%s: busnum=%u, cs=%u: no emulation available (err=%d)\n",
		       __func__, busnum, cs, ret);
		return -ENOENT;
	}
	ret = device_probe(emul);
	if (ret)
		return ret;

	ops = spi_emul_get_ops(emul);
	ret = ops->xfer(emul, bitlen, dout, din, flags);

	log_content("sandbox_spi: xfer: got back %i (that's %s)\n rx:",
		    ret, ret ? "bad" : "good");
	if (din) {
		for (i = 0; i < bytes; ++i)
			log_content(" %u:%02x", i, ((u8 *)din)[i]);
	}
	log_content("\n");

	return ret;
}

static int sandbox_spi_set_speed(struct udevice *bus, uint speed)
{
	struct sandbox_spi_priv *priv = dev_get_priv(bus);

	priv->speed = speed;

	return 0;
}

static int sandbox_spi_set_mode(struct udevice *bus, uint mode)
{
	struct sandbox_spi_priv *priv = dev_get_priv(bus);

	priv->mode = mode;

	return 0;
}

static int sandbox_cs_info(struct udevice *bus, uint cs,
			   struct spi_cs_info *info)
{
	/* Always allow activity on CS 0, CS 1 */
	if (cs >= 2)
		return -EINVAL;

	return 0;
}

static int sandbox_spi_get_mmap(struct udevice *dev, ulong *map_basep,
				uint *map_sizep, uint *offsetp)
{
	*map_basep = 0x1000;
	*map_sizep = 0x2000;
	*offsetp = 0x100;

	return 0;
}

static const struct dm_spi_ops sandbox_spi_ops = {
	.xfer		= sandbox_spi_xfer,
	.set_speed	= sandbox_spi_set_speed,
	.set_mode	= sandbox_spi_set_mode,
	.cs_info	= sandbox_cs_info,
	.get_mmap	= sandbox_spi_get_mmap,
};

static const struct udevice_id sandbox_spi_ids[] = {
	{ .compatible = "sandbox,spi" },
	{ }
};

U_BOOT_DRIVER(sandbox_spi) = {
	.name	= "sandbox_spi",
	.id	= UCLASS_SPI,
	.of_match = sandbox_spi_ids,
	.ops	= &sandbox_spi_ops,
	.priv_auto = sizeof(struct sandbox_spi_priv),
};
