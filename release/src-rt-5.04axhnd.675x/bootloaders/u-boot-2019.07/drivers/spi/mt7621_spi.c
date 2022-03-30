// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 *
 * Derived from the Linux driver version drivers/spi/spi-mt7621.c
 *   Copyright (C) 2011 Sergiy <piratfm@gmail.com>
 *   Copyright (C) 2011-2013 Gabor Juhos <juhosg@openwrt.org>
 *   Copyright (C) 2014-2015 Felix Fietkau <nbd@nbd.name>
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <wait_bit.h>
#include <linux/io.h>

#define SPI_MSG_SIZE_MAX	32	/* SPI message chunk size */
/* Enough for SPI NAND page read / write with page size 2048 bytes */
#define SPI_MSG_SIZE_OVERALL	(2048 + 16)

#define MT7621_SPI_TRANS	0x00
#define MT7621_SPI_TRANS_START	BIT(8)
#define MT7621_SPI_TRANS_BUSY	BIT(16)

#define MT7621_SPI_OPCODE	0x04
#define MT7621_SPI_DATA0	0x08
#define MT7621_SPI_DATA4	0x18
#define MT7621_SPI_MASTER	0x28
#define MT7621_SPI_MOREBUF	0x2c
#define MT7621_SPI_POLAR	0x38

#define MT7621_LSB_FIRST	BIT(3)
#define MT7621_CPOL		BIT(4)
#define MT7621_CPHA		BIT(5)

#define MASTER_MORE_BUFMODE	BIT(2)
#define MASTER_RS_CLK_SEL	GENMASK(27, 16)
#define MASTER_RS_CLK_SEL_SHIFT	16
#define MASTER_RS_SLAVE_SEL	GENMASK(31, 29)

struct mt7621_spi {
	void __iomem *base;
	unsigned int sys_freq;
	u32 data[(SPI_MSG_SIZE_OVERALL / 4) + 1];
	int tx_len;
};

static void mt7621_spi_reset(struct mt7621_spi *rs, int duplex)
{
	setbits_le32(rs->base + MT7621_SPI_MASTER,
		     MASTER_RS_SLAVE_SEL | MASTER_MORE_BUFMODE);
}

static void mt7621_spi_set_cs(struct mt7621_spi *rs, int cs, int enable)
{
	u32 val = 0;

	debug("%s: cs#%d -> %s\n", __func__, cs, enable ? "enable" : "disable");
	if (enable)
		val = BIT(cs);
	iowrite32(val, rs->base + MT7621_SPI_POLAR);
}

static int mt7621_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mt7621_spi *rs = dev_get_priv(bus);
	u32 reg;

	debug("%s: mode=0x%08x\n", __func__, mode);
	reg = ioread32(rs->base + MT7621_SPI_MASTER);

	reg &= ~MT7621_LSB_FIRST;
	if (mode & SPI_LSB_FIRST)
		reg |= MT7621_LSB_FIRST;

	reg &= ~(MT7621_CPHA | MT7621_CPOL);
	switch (mode & (SPI_CPOL | SPI_CPHA)) {
	case SPI_MODE_0:
		break;
	case SPI_MODE_1:
		reg |= MT7621_CPHA;
		break;
	case SPI_MODE_2:
		reg |= MT7621_CPOL;
		break;
	case SPI_MODE_3:
		reg |= MT7621_CPOL | MT7621_CPHA;
		break;
	}
	iowrite32(reg, rs->base + MT7621_SPI_MASTER);

	return 0;
}

static int mt7621_spi_set_speed(struct udevice *bus, uint speed)
{
	struct mt7621_spi *rs = dev_get_priv(bus);
	u32 rate;
	u32 reg;

	debug("%s: speed=%d\n", __func__, speed);
	rate = DIV_ROUND_UP(rs->sys_freq, speed);
	debug("rate:%u\n", rate);

	if (rate > 4097)
		return -EINVAL;

	if (rate < 2)
		rate = 2;

	reg = ioread32(rs->base + MT7621_SPI_MASTER);
	reg &= ~MASTER_RS_CLK_SEL;
	reg |= (rate - 2) << MASTER_RS_CLK_SEL_SHIFT;
	iowrite32(reg, rs->base + MT7621_SPI_MASTER);

	return 0;
}

static inline int mt7621_spi_wait_till_ready(struct mt7621_spi *rs)
{
	int ret;

	ret =  wait_for_bit_le32(rs->base + MT7621_SPI_TRANS,
				 MT7621_SPI_TRANS_BUSY, 0, 10, 0);
	if (ret)
		pr_err("Timeout in %s!\n", __func__);

	return ret;
}

static int mt7621_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mt7621_spi *rs = dev_get_priv(bus);
	const u8 *tx_buf = dout;
	u8 *ptr = (u8 *)dout;
	u8 *rx_buf = din;
	int total_size = bitlen >> 3;
	int chunk_size;
	int rx_len = 0;
	u32 data[(SPI_MSG_SIZE_MAX / 4) + 1] = { 0 };
	u32 val;
	int i;

	debug("%s: dout=%p, din=%p, len=%x, flags=%lx\n", __func__, dout, din,
	      total_size, flags);

	/*
	 * This driver only supports half-duplex, so complain and bail out
	 * upon full-duplex messages
	 */
	if (dout && din) {
		printf("Only half-duplex SPI transfer supported\n");
		return -EIO;
	}

	if (dout) {
		debug("TX-DATA: ");
		for (i = 0; i < total_size; i++)
			debug("%02x ", *ptr++);
		debug("\n");
	}

	mt7621_spi_wait_till_ready(rs);

	/*
	 * Set CS active upon start of SPI message. This message can
	 * be split upon multiple calls to this xfer function
	 */
	if (flags & SPI_XFER_BEGIN)
		mt7621_spi_set_cs(rs, spi_chip_select(dev), 1);

	while (total_size > 0) {
		/* Don't exceed the max xfer size */
		chunk_size = min_t(int, total_size, SPI_MSG_SIZE_MAX);

		/*
		 * We might have some TX data buffered from the last xfer
		 * message. Make sure, that this does not exceed the max
		 * xfer size
		 */
		if (rs->tx_len > 4)
			chunk_size -= rs->tx_len;
		if (din)
			rx_len = chunk_size;

		if (tx_buf) {
			/* Check if this message does not exceed the buffer */
			if ((chunk_size + rs->tx_len) > SPI_MSG_SIZE_OVERALL) {
				printf("TX message size too big (%d)\n",
				       chunk_size + rs->tx_len);
				return -EMSGSIZE;
			}

			/*
			 * Write all TX data into internal buffer to collect
			 * all TX messages into one buffer (might be split into
			 * multiple calls to this function)
			 */
			for (i = 0; i < chunk_size; i++, rs->tx_len++) {
				rs->data[rs->tx_len / 4] |=
					tx_buf[i] << (8 * (rs->tx_len & 3));
			}
		}

		if (flags & SPI_XFER_END) {
			/* Write TX data into controller */
			if (rs->tx_len) {
				rs->data[0] = swab32(rs->data[0]);
				if (rs->tx_len < 4)
					rs->data[0] >>= (4 - rs->tx_len) * 8;

				for (i = 0; i < rs->tx_len; i += 4) {
					iowrite32(rs->data[i / 4], rs->base +
						  MT7621_SPI_OPCODE + i);
				}
			}

			/* Write length into controller */
			val = (min_t(int, rs->tx_len, 4) * 8) << 24;
			if (rs->tx_len > 4)
				val |= (rs->tx_len - 4) * 8;
			val |= (rx_len * 8) << 12;
			iowrite32(val, rs->base + MT7621_SPI_MOREBUF);

			/* Start the xfer */
			setbits_le32(rs->base + MT7621_SPI_TRANS,
				     MT7621_SPI_TRANS_START);

			/* Wait until xfer is finished on bus */
			mt7621_spi_wait_till_ready(rs);

			/* Reset TX length and TX buffer for next xfer */
			rs->tx_len = 0;
			memset(rs->data, 0, sizeof(rs->data));
		}

		for (i = 0; i < rx_len; i += 4)
			data[i / 4] = ioread32(rs->base + MT7621_SPI_DATA0 + i);

		if (rx_len) {
			debug("RX-DATA: ");
			for (i = 0; i < rx_len; i++) {
				rx_buf[i] = data[i / 4] >> (8 * (i & 3));
				debug("%02x ", rx_buf[i]);
			}
			debug("\n");
		}

		if (tx_buf)
			tx_buf += chunk_size;
		if (rx_buf)
			rx_buf += chunk_size;
		total_size -= chunk_size;
	}

	/* Wait until xfer is finished on bus and de-assert CS */
	mt7621_spi_wait_till_ready(rs);
	if (flags & SPI_XFER_END)
		mt7621_spi_set_cs(rs, spi_chip_select(dev), 0);

	return 0;
}

static int mt7621_spi_probe(struct udevice *dev)
{
	struct mt7621_spi *rs = dev_get_priv(dev);

	rs->base = dev_remap_addr(dev);
	if (!rs->base)
		return -EINVAL;

	/*
	 * Read input clock via DT for now. At some point this should be
	 * replaced by implementing a clock driver for this SoC and getting
	 * the SPI frequency via this clock driver.
	 */
	rs->sys_freq = dev_read_u32_default(dev, "clock-frequency", 0);
	if (!rs->sys_freq) {
		printf("Please provide clock-frequency!\n");
		return -EINVAL;
	}

	mt7621_spi_reset(rs, 0);

	return 0;
}

static const struct dm_spi_ops mt7621_spi_ops = {
	.set_mode = mt7621_spi_set_mode,
	.set_speed = mt7621_spi_set_speed,
	.xfer = mt7621_spi_xfer,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id mt7621_spi_ids[] = {
	{ .compatible = "ralink,mt7621-spi" },
	{ }
};

U_BOOT_DRIVER(mt7621_spi) = {
	.name = "mt7621_spi",
	.id = UCLASS_SPI,
	.of_match = mt7621_spi_ids,
	.ops = &mt7621_spi_ops,
	.priv_auto_alloc_size = sizeof(struct mt7621_spi),
	.probe = mt7621_spi_probe,
};
