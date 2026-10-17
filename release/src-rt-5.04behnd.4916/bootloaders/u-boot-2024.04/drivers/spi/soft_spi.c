// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 *
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * Influenced by code from:
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

struct soft_spi_plat {
	struct gpio_desc cs;
	struct gpio_desc sclk;
	struct gpio_desc mosi;
	struct gpio_desc miso;
	int spi_delay_us;
	int flags;
};

#define SPI_MASTER_NO_RX        BIT(0)
#define SPI_MASTER_NO_TX        BIT(1)

struct soft_spi_priv {
	unsigned int mode;
};

static int soft_spi_scl(struct udevice *dev, int bit)
{
	struct udevice *bus = dev_get_parent(dev);
	struct soft_spi_plat *plat = dev_get_plat(bus);

	dm_gpio_set_value(&plat->sclk, bit);

	return 0;
}

static int soft_spi_sda(struct udevice *dev, int bit)
{
	struct udevice *bus = dev_get_parent(dev);
	struct soft_spi_plat *plat = dev_get_plat(bus);

	dm_gpio_set_value(&plat->mosi, bit);

	return 0;
}

static int soft_spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct soft_spi_priv *priv = dev_get_priv(bus);
	struct soft_spi_plat *plat = dev_get_plat(bus);
	int cidle = !!(priv->mode & SPI_CPOL);

	dm_gpio_set_value(&plat->cs, 0);
	dm_gpio_set_value(&plat->sclk, cidle); /* to idle */
	dm_gpio_set_value(&plat->cs, 1);

	return 0;
}

static int soft_spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct soft_spi_plat *plat = dev_get_plat(bus);

	dm_gpio_set_value(&plat->cs, 0);

	return 0;
}

static int soft_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct soft_spi_priv *priv = dev_get_priv(bus);
	int cidle = !!(priv->mode & SPI_CPOL);
	/*
	 * Make sure the SPI clock is in idle state as defined for
	 * this slave.
	 */
	return soft_spi_scl(dev, cidle);
}

static int soft_spi_release_bus(struct udevice *dev)
{
	/* Nothing to do */
	return 0;
}

/*-----------------------------------------------------------------------
 * SPI transfer
 *
 * This writes "bitlen" bits out the SPI MOSI port and simultaneously clocks
 * "bitlen" bits in the SPI MISO port.  That's just the way SPI works.
 *
 * The source of the outgoing bits is the "dout" parameter and the
 * destination of the input bits is the "din" parameter.  Note that "dout"
 * and "din" can point to the same memory location, in which case the
 * input data overwrites the output data (since both are buffered by
 * temporary variables, this is OK).
 */
static int soft_spi_xfer(struct udevice *dev, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct soft_spi_priv *priv = dev_get_priv(bus);
	struct soft_spi_plat *plat = dev_get_plat(bus);
	uchar		tmpdin  = 0;
	uchar		tmpdout = 0;
	const u8	*txd = dout;
	u8		*rxd = din;
	int		cpha = !!(priv->mode & SPI_CPHA);
	int		cidle = !!(priv->mode & SPI_CPOL);
	unsigned int	j;

	debug("spi_xfer: slave %s:%s dout %08X din %08X bitlen %u\n",
	      dev->parent->name, dev->name, *(uint *)txd, *(uint *)rxd,
	      bitlen);

	if (flags & SPI_XFER_BEGIN)
		soft_spi_cs_activate(dev);

	for (j = 0; j < bitlen; j++) {
		/*
		 * Check if it is time to work on a new byte.
		 */
		if ((j % 8) == 0) {
			if (txd)
				tmpdout = *txd++;
			else
				tmpdout = 0;
			if (j != 0) {
				if (rxd)
					*rxd++ = tmpdin;
			}
			tmpdin  = 0;
		}

		/*
		 * CPOL 0: idle is low (0), active is high (1)
		 * CPOL 1: idle is high (1), active is low (0)
		 */

		/*
		 * drive bit
		 *  CPHA 1: CLK from idle to active
		 */
		if (cpha)
			soft_spi_scl(dev, !cidle);
		if ((plat->flags & SPI_MASTER_NO_TX) == 0)
			soft_spi_sda(dev, !!(tmpdout & 0x80));
		udelay(plat->spi_delay_us);

		/*
		 * sample bit
		 *  CPHA 0: CLK from idle to active
		 *  CPHA 1: CLK from active to idle
		 */
		if (!cpha)
			soft_spi_scl(dev, !cidle);
		else
			soft_spi_scl(dev, cidle);
		tmpdin	<<= 1;
		if ((plat->flags & SPI_MASTER_NO_RX) == 0)
			tmpdin	|= dm_gpio_get_value(&plat->miso);
		tmpdout	<<= 1;
		udelay(plat->spi_delay_us);

		/*
		 * drive bit
		 *  CPHA 0: CLK from active to idle
		 */
		if (!cpha)
			soft_spi_scl(dev, cidle);
	}
	/*
	 * If the number of bits isn't a multiple of 8, shift the last
	 * bits over to left-justify them.  Then store the last byte
	 * read in.
	 */
	if (rxd) {
		if ((bitlen % 8) != 0)
			tmpdin <<= 8 - (bitlen % 8);
		*rxd++ = tmpdin;
	}

	if (flags & SPI_XFER_END)
		soft_spi_cs_deactivate(dev);

	return 0;
}

static int soft_spi_set_speed(struct udevice *dev, unsigned int speed)
{
	/* Ignore any speed settings. Speed is implemented via "spi-delay-us" */
	return 0;
}

static int soft_spi_set_mode(struct udevice *dev, unsigned int mode)
{
	struct soft_spi_priv *priv = dev_get_priv(dev);

	priv->mode = mode;

	return 0;
}

static const struct dm_spi_ops soft_spi_ops = {
	.claim_bus	= soft_spi_claim_bus,
	.release_bus	= soft_spi_release_bus,
	.xfer		= soft_spi_xfer,
	.set_speed	= soft_spi_set_speed,
	.set_mode	= soft_spi_set_mode,
};

static int soft_spi_of_to_plat(struct udevice *dev)
{
	struct soft_spi_plat *plat = dev_get_plat(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);

	plat->spi_delay_us = fdtdec_get_int(blob, node, "spi-delay-us", 0);

	return 0;
}

static int soft_spi_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct soft_spi_plat *plat = dev_get_plat(dev);
	int cs_flags, clk_flags;
	int ret;

	cs_flags = (slave && slave->mode & SPI_CS_HIGH) ? 0 : GPIOD_ACTIVE_LOW;
	clk_flags = (slave && slave->mode & SPI_CPOL) ? GPIOD_ACTIVE_LOW : 0;

	ret = gpio_request_by_name(dev, "cs-gpios", 0, &plat->cs,
				   GPIOD_IS_OUT | cs_flags);
	if (ret)
		return -EINVAL;

	ret = gpio_request_by_name(dev, "gpio-sck", 0, &plat->sclk,
				   GPIOD_IS_OUT | clk_flags);
	if (ret)
		ret = gpio_request_by_name(dev, "sck-gpios", 0, &plat->sclk,
					   GPIOD_IS_OUT | clk_flags);
	if (ret)
		return -EINVAL;

	ret = gpio_request_by_name(dev, "gpio-mosi", 0, &plat->mosi,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret)
		ret = gpio_request_by_name(dev, "mosi-gpios", 0, &plat->mosi,
					   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret)
		plat->flags |= SPI_MASTER_NO_TX;

	ret = gpio_request_by_name(dev, "gpio-miso", 0, &plat->miso,
				   GPIOD_IS_IN);
	if (ret)
		ret = gpio_request_by_name(dev, "gpio-miso", 0, &plat->miso,
					   GPIOD_IS_IN);
	if (ret)
		plat->flags |= SPI_MASTER_NO_RX;

	if ((plat->flags & (SPI_MASTER_NO_RX | SPI_MASTER_NO_TX)) ==
	    (SPI_MASTER_NO_RX | SPI_MASTER_NO_TX))
		return -EINVAL;

	return 0;
}

static const struct udevice_id soft_spi_ids[] = {
	{ .compatible = "spi-gpio" },
	{ }
};

U_BOOT_DRIVER(soft_spi) = {
	.name	= "soft_spi",
	.id	= UCLASS_SPI,
	.of_match = soft_spi_ids,
	.ops	= &soft_spi_ops,
	.of_to_plat = soft_spi_of_to_plat,
	.plat_auto	= sizeof(struct soft_spi_plat),
	.priv_auto	= sizeof(struct soft_spi_priv),
	.probe	= soft_spi_probe,
};
