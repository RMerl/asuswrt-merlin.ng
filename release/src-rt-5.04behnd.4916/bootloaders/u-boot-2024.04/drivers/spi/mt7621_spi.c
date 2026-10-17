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
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <spi.h>
#include <wait_bit.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/printk.h>

#define MT7621_RX_FIFO_LEN	32
#define MT7621_TX_FIFO_LEN	36

#define MT7621_SPI_TRANS	0x00
#define MT7621_SPI_TRANS_START	BIT(8)
#define MT7621_SPI_TRANS_BUSY	BIT(16)
#define TRANS_ADDR_SZ		GENMASK(20, 19)
#define TRANS_ADDR_SZ_SHIFT	19
#define TRANS_MOSI_BCNT		GENMASK(3, 0)
#define TRANS_MOSI_BCNT_SHIFT	0

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

#define MOREBUF_CMD_CNT		GENMASK(29, 24)
#define MOREBUF_CMD_CNT_SHIFT	24
#define MOREBUF_MISO_CNT	GENMASK(20, 12)
#define MOREBUF_MISO_CNT_SHIFT	12
#define MOREBUF_MOSI_CNT	GENMASK(8, 0)
#define MOREBUF_MOSI_CNT_SHIFT	0

struct mt7621_spi {
	void __iomem *base;
	unsigned int sys_freq;
};

static void mt7621_spi_set_cs(struct mt7621_spi *rs, int cs, int enable)
{
	debug("%s: cs#%d -> %s\n", __func__, cs, enable ? "enable" : "disable");

	if (enable) {
		setbits_le32(rs->base + MT7621_SPI_MASTER,
			     MASTER_RS_SLAVE_SEL | MASTER_MORE_BUFMODE);
		iowrite32(BIT(cs), rs->base + MT7621_SPI_POLAR);
	} else {
		iowrite32(0, rs->base + MT7621_SPI_POLAR);
		iowrite32((2 << TRANS_ADDR_SZ_SHIFT) |
			  (1 << TRANS_MOSI_BCNT_SHIFT),
			  rs->base + MT7621_SPI_TRANS);
		clrbits_le32(rs->base + MT7621_SPI_MASTER,
			     MASTER_RS_SLAVE_SEL | MASTER_MORE_BUFMODE);
	}
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

static int mt7621_spi_read(struct mt7621_spi *rs, u8 *buf, size_t len)
{
	size_t rx_len;
	int i, ret;
	u32 val = 0;

	while (len) {
		rx_len = min_t(size_t, len, MT7621_RX_FIFO_LEN);

		iowrite32((rx_len * 8) << MOREBUF_MISO_CNT_SHIFT,
			  rs->base + MT7621_SPI_MOREBUF);
		iowrite32(MT7621_SPI_TRANS_START, rs->base + MT7621_SPI_TRANS);

		ret = mt7621_spi_wait_till_ready(rs);
		if (ret)
			return ret;

		for (i = 0; i < rx_len; i++) {
			if ((i % 4) == 0)
				val = ioread32(rs->base + MT7621_SPI_DATA0 + i);
			*buf++ = val & 0xff;
			val >>= 8;
		}

		len -= rx_len;
	}

	return ret;
}

static int mt7621_spi_write(struct mt7621_spi *rs, const u8 *buf, size_t len)
{
	size_t tx_len, opcode_len, dido_len;
	int i, ret;
	u32 val;

	while (len) {
		tx_len = min_t(size_t, len, MT7621_TX_FIFO_LEN);

		opcode_len = min_t(size_t, tx_len, 4);
		dido_len = tx_len - opcode_len;

		val = 0;
		for (i = 0; i < opcode_len; i++) {
			val <<= 8;
			val |= *buf++;
		}

		iowrite32(val, rs->base + MT7621_SPI_OPCODE);

		val = 0;
		for (i = 0; i < dido_len; i++) {
			val |= (*buf++) << ((i % 4) * 8);

			if ((i % 4 == 3) || (i == dido_len - 1)) {
				iowrite32(val, rs->base + MT7621_SPI_DATA0 +
					  (i & ~3));
				val = 0;
			}
		}

		iowrite32(((opcode_len * 8) << MOREBUF_CMD_CNT_SHIFT) |
			  ((dido_len * 8) << MOREBUF_MOSI_CNT_SHIFT),
			  rs->base + MT7621_SPI_MOREBUF);
		iowrite32(MT7621_SPI_TRANS_START, rs->base + MT7621_SPI_TRANS);

		ret = mt7621_spi_wait_till_ready(rs);
		if (ret)
			return ret;

		len -= tx_len;
	}

	return 0;
}

static int mt7621_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mt7621_spi *rs = dev_get_priv(bus);
	int total_size = bitlen >> 3;
	int ret = 0;

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

	mt7621_spi_wait_till_ready(rs);

	/*
	 * Set CS active upon start of SPI message. This message can
	 * be split upon multiple calls to this xfer function
	 */
	if (flags & SPI_XFER_BEGIN)
		mt7621_spi_set_cs(rs, spi_chip_select(dev), 1);

	if (din)
		ret = mt7621_spi_read(rs, din, total_size);
	else if (dout)
		ret = mt7621_spi_write(rs, dout, total_size);

	if (flags & SPI_XFER_END)
		mt7621_spi_set_cs(rs, spi_chip_select(dev), 0);

	return ret;
}

static int mt7621_spi_probe(struct udevice *dev)
{
	struct mt7621_spi *rs = dev_get_priv(dev);
	struct clk clk;
	int ret;

	rs->base = dev_remap_addr(dev);
	if (!rs->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		printf("Please provide a clock!\n");
		return ret;
	}

	clk_enable(&clk);

	rs->sys_freq = clk_get_rate(&clk);
	if (!rs->sys_freq) {
		printf("Please provide a valid clock!\n");
		return -EINVAL;
	}

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
	.priv_auto	= sizeof(struct mt7621_spi),
	.probe = mt7621_spi_probe,
};
