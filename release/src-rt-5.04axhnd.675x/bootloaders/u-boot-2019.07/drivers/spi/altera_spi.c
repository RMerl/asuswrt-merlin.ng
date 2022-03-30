// SPDX-License-Identifier: GPL-2.0+
/*
 * Altera SPI driver
 *
 * based on bfin_spi.c
 * Copyright (c) 2005-2008 Analog Devices Inc.
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <fdtdec.h>
#include <spi.h>
#include <asm/io.h>

#define ALTERA_SPI_STATUS_RRDY_MSK	BIT(7)
#define ALTERA_SPI_CONTROL_SSO_MSK	BIT(10)

#ifndef CONFIG_ALTERA_SPI_IDLE_VAL
#define CONFIG_ALTERA_SPI_IDLE_VAL	0xff
#endif

struct altera_spi_regs {
	u32	rxdata;
	u32	txdata;
	u32	status;
	u32	control;
	u32	_reserved;
	u32	slave_sel;
};

struct altera_spi_platdata {
	struct altera_spi_regs *regs;
};

struct altera_spi_priv {
	struct altera_spi_regs *regs;
};

static void spi_cs_activate(struct udevice *dev, uint cs)
{
	struct udevice *bus = dev->parent;
	struct altera_spi_priv *priv = dev_get_priv(bus);
	struct altera_spi_regs *const regs = priv->regs;

	writel(1 << cs, &regs->slave_sel);
	writel(ALTERA_SPI_CONTROL_SSO_MSK, &regs->control);
}

static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct altera_spi_priv *priv = dev_get_priv(bus);
	struct altera_spi_regs *const regs = priv->regs;

	writel(0, &regs->control);
	writel(0, &regs->slave_sel);
}

static int altera_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct altera_spi_priv *priv = dev_get_priv(bus);
	struct altera_spi_regs *const regs = priv->regs;

	writel(0, &regs->control);
	writel(0, &regs->slave_sel);

	return 0;
}

static int altera_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct altera_spi_priv *priv = dev_get_priv(bus);
	struct altera_spi_regs *const regs = priv->regs;

	writel(0, &regs->slave_sel);

	return 0;
}

static int altera_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct altera_spi_priv *priv = dev_get_priv(bus);
	struct altera_spi_regs *const regs = priv->regs;
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

	/* assume spi core configured to do 8 bit transfers */
	unsigned int bytes = bitlen / 8;
	const unsigned char *txp = dout;
	unsigned char *rxp = din;
	uint32_t reg, data, start;

	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
	      bus->seq, slave_plat->cs, bitlen, bytes, flags);

	if (bitlen == 0)
		goto done;

	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	/* empty read buffer */
	if (readl(&regs->status) & ALTERA_SPI_STATUS_RRDY_MSK)
		readl(&regs->rxdata);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev, slave_plat->cs);

	while (bytes--) {
		if (txp)
			data = *txp++;
		else
			data = CONFIG_ALTERA_SPI_IDLE_VAL;

		debug("%s: tx:%x ", __func__, data);
		writel(data, &regs->txdata);

		start = get_timer(0);
		while (1) {
			reg = readl(&regs->status);
			if (reg & ALTERA_SPI_STATUS_RRDY_MSK)
				break;
			if (get_timer(start) > (CONFIG_SYS_HZ / 1000)) {
				debug("%s: Transmission timed out!\n", __func__);
				return -1;
			}
		}

		data = readl(&regs->rxdata);
		if (rxp)
			*rxp++ = data & 0xff;

		debug("rx:%x\n", data);
	}

done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	return 0;
}

static int altera_spi_set_speed(struct udevice *bus, uint speed)
{
	return 0;
}

static int altera_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static int altera_spi_probe(struct udevice *bus)
{
	struct altera_spi_platdata *plat = dev_get_platdata(bus);
	struct altera_spi_priv *priv = dev_get_priv(bus);

	priv->regs = plat->regs;

	return 0;
}

static int altera_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct altera_spi_platdata *plat = dev_get_platdata(bus);

	plat->regs = map_physmem(devfdt_get_addr(bus),
				 sizeof(struct altera_spi_regs),
				 MAP_NOCACHE);

	return 0;
}

static const struct dm_spi_ops altera_spi_ops = {
	.claim_bus	= altera_spi_claim_bus,
	.release_bus	= altera_spi_release_bus,
	.xfer		= altera_spi_xfer,
	.set_speed	= altera_spi_set_speed,
	.set_mode	= altera_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id altera_spi_ids[] = {
	{ .compatible = "altr,spi-1.0" },
	{}
};

U_BOOT_DRIVER(altera_spi) = {
	.name	= "altera_spi",
	.id	= UCLASS_SPI,
	.of_match = altera_spi_ids,
	.ops	= &altera_spi_ops,
	.ofdata_to_platdata = altera_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct altera_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct altera_spi_priv),
	.probe	= altera_spi_probe,
};
