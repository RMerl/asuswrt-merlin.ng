// SPDX-License-Identifier: GPL-2.0
/*
 * GXP SPI driver
 *
 * (C) Copyright 2022 Hewlett Packard Enterprise Development LP.
 * Author: Nick Hawkins <nick.hawkins@hpe.com>
 * Author: Jean-Marie Verdun <verdun@hpe.com>
 */

#include <spi.h>
#include <asm/io.h>
#include <dm.h>

#define GXP_SPI0_MAX_CHIPSELECT		2

#define MANUAL_MODE	0
#define AUTO_MODE		1
#define OFFSET_SPIMCFG	0x00
#define OFFSET_SPIMCTRL	0x04
#define OFFSET_SPICMD		0x05
#define OFFSET_SPIDCNT	0x06
#define OFFSET_SPIADDR	0x08
#define OFFSET_SPILDAT	0x40
#define GXP_SPILDAT_SIZE 64

#define SPIMCTRL_START	0x01
#define SPIMCTRL_BUSY		0x02

#define CMD_READ_ARRAY_FAST		0x0b

struct gxp_spi_priv {
	struct spi_slave	slave;
	void __iomem *base;
	unsigned int mode;

};

static void spi_set_mode(struct gxp_spi_priv *priv, int mode)
{
	unsigned char value;

	value = readb(priv->base + OFFSET_SPIMCTRL);
	if (mode == MANUAL_MODE) {
		writeb(0x55, priv->base + OFFSET_SPICMD);
		writeb(0xaa, priv->base + OFFSET_SPICMD);
		/* clear bit5 and bit4, auto_start and start_mask */
		value &= ~(0x03 << 4);
	} else {
		value |= (0x03 << 4);
	}
	writeb(value, priv->base + OFFSET_SPIMCTRL);
}

static int gxp_spi_xfer(struct udevice *dev, unsigned int bitlen, const void *dout, void *din,
			unsigned long flags)
{
	struct gxp_spi_priv *priv = dev_get_priv(dev->parent);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	unsigned int len = bitlen / 8;
	unsigned int value;
	unsigned int addr = 0;
	unsigned char uchar_out[len];
	unsigned char *uchar_in = (unsigned char *)din;
	int read_len;
	int read_ptr;

	if (dout && din) {
		/*
		 * error: gxp spi engin cannot send data to dout and read data from din at the same
		 * time
		 */
		return -1;
	}

	memset(uchar_out, 0, sizeof(uchar_out));
	if (dout)
		memcpy(uchar_out, dout, len);

	if (flags & SPI_XFER_BEGIN) {
		/* the dout is cmd + addr, cmd=dout[0], add1~3=dout[1~3]. */
		/* cmd reg */
		writeb(uchar_out[0], priv->base + OFFSET_SPICMD);

		/* config reg */
		value = readl(priv->base + OFFSET_SPIMCFG);
		value &= ~(1 << 24);
		/* set chipselect */
		value |= (slave_plat->cs << 24);

		/* addr reg and addr size */
		if (len >= 4) {
			addr = uchar_out[1] << 16 | uchar_out[2] << 8 | uchar_out[3];
			writel(addr, priv->base + OFFSET_SPIADDR);
			value &= ~(0x07 << 16);
			/* set the address size to 3 byte */
			value |= (3 << 16);
		} else {
			writel(0, priv->base + OFFSET_SPIADDR);
			/* set the address size to 0 byte */
			value &= ~(0x07 << 16);
		}

		/* dummy */
		/* clear dummy_cnt to */
		value &= ~(0x1f << 19);
		if (uchar_out[0] == CMD_READ_ARRAY_FAST) {
			/* fast read needs 8 dummy clocks */
			value |= (8 << 19);
		}

		writel(value, priv->base + OFFSET_SPIMCFG);

		if (flags & SPI_XFER_END) {
			/* no data cmd just start it */
			/* set the data direction bit to 1 */
			value = readb(priv->base + OFFSET_SPIMCTRL);
			value |= (1 << 3);
			writeb(value, priv->base + OFFSET_SPIMCTRL);

			/* set the data byte count */
			writeb(0, priv->base + OFFSET_SPIDCNT);

			/* set the start bit */
			value = readb(priv->base + OFFSET_SPIMCTRL);
			value |= SPIMCTRL_START;
			writeb(value, priv->base + OFFSET_SPIMCTRL);

			/* wait busy bit is cleared */
			do {
				value = readb(priv->base + OFFSET_SPIMCTRL);
			} while (value & SPIMCTRL_BUSY);
			return 0;
		}
	}

	if (!(flags & SPI_XFER_END) && (flags & SPI_XFER_BEGIN)) {
		/* first of spi_xfer calls */
		return 0;
	}

	/* if dout != null, write data to buf and start transaction */
	if (dout) {
		if (len > slave->max_write_size) {
			printf("SF: write length is too big(>%d)\n", slave->max_write_size);
			return -1;
		}

		/* load the data bytes */
		memcpy((u8 *)priv->base + OFFSET_SPILDAT, dout, len);

		/* write: set the data direction bit to 1 */
		value = readb(priv->base + OFFSET_SPIMCTRL);
		value |= (1 << 3);
		writeb(value, priv->base + OFFSET_SPIMCTRL);

		/* set the data byte count */
		writeb(len, priv->base + OFFSET_SPIDCNT);

		/* set the start bit */
		value = readb(priv->base + OFFSET_SPIMCTRL);
		value |= SPIMCTRL_START;
		writeb(value, priv->base + OFFSET_SPIMCTRL);

		/* wait busy bit is cleared */
		do {
			value = readb(priv->base + OFFSET_SPIMCTRL);
		} while (value & SPIMCTRL_BUSY);

		return 0;
	}

	/* if din !=null, start and read data */
	if (uchar_in) {
		read_ptr = 0;

		while (read_ptr < len) {
			read_len = len - read_ptr;
			if (read_len > GXP_SPILDAT_SIZE)
				read_len = GXP_SPILDAT_SIZE;

			/* read: set the data direction bit to 0 */
			value = readb(priv->base + OFFSET_SPIMCTRL);
			value &= ~(1 << 3);
			writeb(value, priv->base + OFFSET_SPIMCTRL);

			/* set the data byte count */
			writeb(read_len, priv->base + OFFSET_SPIDCNT);

			/* set the start bit */
			value = readb(priv->base + OFFSET_SPIMCTRL);
			value |= SPIMCTRL_START;
			writeb(value, priv->base + OFFSET_SPIMCTRL);

			/* wait busy bit is cleared */
			do {
				value = readb(priv->base + OFFSET_SPIMCTRL);
			} while (value & SPIMCTRL_BUSY);

			/* store the data bytes */
			memcpy(uchar_in + read_ptr, (u8 *)priv->base + OFFSET_SPILDAT, read_len);
			/* update read_ptr and addr reg */
			read_ptr += read_len;

			addr = readl(priv->base + OFFSET_SPIADDR);
			addr += read_len;
			writel(addr, priv->base + OFFSET_SPIADDR);
		}

		return 0;
	}
	return -2;
}

static int gxp_spi_set_speed(struct udevice *dev, unsigned int speed)
{
	/* Accept any speed */
	return 0;
}

static int gxp_spi_set_mode(struct udevice *dev, unsigned int mode)
{
	struct gxp_spi_priv *priv = dev_get_priv(dev->parent);

	priv->mode = mode;

	return 0;
}

static int gxp_spi_claim_bus(struct udevice *dev)
{
	struct gxp_spi_priv *priv = dev_get_priv(dev->parent);
	unsigned char cmd;

	spi_set_mode(priv, MANUAL_MODE);

	/* exit 4 bytes addr mode, uboot spi_flash only supports 3 byets address mode */
	cmd = 0xe9;
	gxp_spi_xfer(dev, 1 * 8, &cmd, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
	return 0;
}

static int gxp_spi_release_bus(struct udevice *dev)
{
	struct gxp_spi_priv *priv = dev_get_priv(dev->parent);

	spi_set_mode(priv, AUTO_MODE);

	return 0;
}

int gxp_spi_cs_info(struct udevice *bus, unsigned int cs, struct spi_cs_info *info)
{
	if (cs < GXP_SPI0_MAX_CHIPSELECT)
		return 0;
	else
		return -ENODEV;
}

static int gxp_spi_probe(struct udevice *bus)
{
	struct gxp_spi_priv *priv = dev_get_priv(bus);

	priv->base = dev_read_addr_ptr(bus);
	if (!priv->base)
		return -ENOENT;

	return 0;
}

static int gxp_spi_child_pre_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);

	slave->max_write_size = GXP_SPILDAT_SIZE;

	return 0;
}

static const struct dm_spi_ops gxp_spi_ops = {
	.claim_bus = gxp_spi_claim_bus,
	.release_bus = gxp_spi_release_bus,
	.xfer = gxp_spi_xfer,
	.set_speed = gxp_spi_set_speed,
	.set_mode = gxp_spi_set_mode,
	.cs_info = gxp_spi_cs_info,
};

static const struct udevice_id gxp_spi_ids[] = {
	{ .compatible = "hpe,gxp-spi" },
	{ }
};

U_BOOT_DRIVER(gxp_spi) = {
	.name	= "gxp_spi",
	.id	= UCLASS_SPI,
	.of_match = gxp_spi_ids,
	.ops	= &gxp_spi_ops,
	.priv_auto = sizeof(struct gxp_spi_priv),
	.probe	= gxp_spi_probe,
	.child_pre_probe = gxp_spi_child_pre_probe,
};

