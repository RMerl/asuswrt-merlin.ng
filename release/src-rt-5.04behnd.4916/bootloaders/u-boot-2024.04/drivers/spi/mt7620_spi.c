// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 * Generic SPI driver for MediaTek MT7620 SoC
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <spi.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/io.h>
#include <linux/log2.h>

#define MT7620_SPI_NUM_CS	2
#define MT7620_SPI_MASTER1_OFF	0x00
#define MT7620_SPI_MASTER2_OFF	0x40

/* SPI_STAT */
#define   SPI_BUSY		BIT(0)

/* SPI_CFG */
#define   MSB_FIRST		BIT(8)
#define   SPI_CLK_POL		BIT(6)
#define   RX_CLK_EDGE		BIT(5)
#define   TX_CLK_EDGE		BIT(4)
#define   SPI_CLK_S		0
#define   SPI_CLK_M		GENMASK(2, 0)

/* SPI_CTL */
#define   START_WR		BIT(2)
#define   START_RD		BIT(1)
#define   SPI_HIGH		BIT(0)

#define SPI_ARB			0xf0
#define   ARB_EN		BIT(31)

#define POLLING_SCALE		10
#define POLLING_FRAC_USEC	100

struct mt7620_spi_master_regs {
	u32 stat;
	u32 reserved0[3];
	u32 cfg;
	u32 ctl;
	u32 reserved1[2];
	u32 data;
};

struct mt7620_spi {
	void __iomem *regs;
	struct mt7620_spi_master_regs *m[MT7620_SPI_NUM_CS];
	unsigned int sys_freq;
	u32 wait_us;
	uint mode;
	uint speed;
};

static void mt7620_spi_master_setup(struct mt7620_spi *ms, int cs)
{
	u32 rate, prescale, freq, tmo, cfg;

	/* Calculate the clock divsior */
	rate = DIV_ROUND_UP(ms->sys_freq, ms->speed);
	rate = roundup_pow_of_two(rate);

	prescale = ilog2(rate / 2);
	if (prescale > 6)
		prescale = 6;

	/* Calculate the real clock, and usecs for one byte transaction */
	freq = ms->sys_freq >> (prescale + 1);
	tmo = DIV_ROUND_UP(8 * 1000000, freq);

	/* 10 times tolerance plus 100us */
	ms->wait_us = POLLING_SCALE * tmo + POLLING_FRAC_USEC;

	/* set SPI_CFG */
	cfg = prescale << SPI_CLK_S;

	switch (ms->mode & (SPI_CPOL | SPI_CPHA)) {
	case SPI_MODE_0:
		cfg |= TX_CLK_EDGE;
		break;
	case SPI_MODE_1:
		cfg |= RX_CLK_EDGE;
		break;
	case SPI_MODE_2:
		cfg |= SPI_CLK_POL | RX_CLK_EDGE;
		break;
	case SPI_MODE_3:
		cfg |= SPI_CLK_POL | TX_CLK_EDGE;
		break;
	}

	if (!(ms->mode & SPI_LSB_FIRST))
		cfg |= MSB_FIRST;

	writel(cfg, &ms->m[cs]->cfg);

	writel(SPI_HIGH, &ms->m[cs]->ctl);
}

static void mt7620_spi_set_cs(struct mt7620_spi *ms, int cs, bool enable)
{
	if (enable)
		mt7620_spi_master_setup(ms, cs);

	if (ms->mode & SPI_CS_HIGH)
		enable = !enable;

	if (enable)
		clrbits_32(&ms->m[cs]->ctl, SPI_HIGH);
	else
		setbits_32(&ms->m[cs]->ctl, SPI_HIGH);
}

static int mt7620_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mt7620_spi *ms = dev_get_priv(bus);

	ms->mode = mode;

	/* Mode 0 is buggy. Force to use mode 3 */
	if ((mode & SPI_MODE_3) == SPI_MODE_0)
		ms->mode |= SPI_MODE_3;

	return 0;
}

static int mt7620_spi_set_speed(struct udevice *bus, uint speed)
{
	struct mt7620_spi *ms = dev_get_priv(bus);

	ms->speed = speed;

	return 0;
}

static inline int mt7620_spi_busy_poll(struct mt7620_spi *ms, int cs)
{
	u32 val;

	return readl_poll_timeout(&ms->m[cs]->stat, val, !(val & SPI_BUSY),
				  ms->wait_us);
}

static int mt7620_spi_read(struct mt7620_spi *ms, int cs, u8 *buf, size_t len)
{
	int ret;

	while (len) {
		setbits_32(&ms->m[cs]->ctl, START_RD);

		ret = mt7620_spi_busy_poll(ms, cs);
		if (ret)
			return ret;

		*buf++ = (u8)readl(&ms->m[cs]->data);

		len--;
	}

	return 0;
}

static int mt7620_spi_write(struct mt7620_spi *ms, int cs, const u8 *buf,
			    size_t len)
{
	int ret;

	while (len) {
		writel(*buf++, &ms->m[cs]->data);
		setbits_32(&ms->m[cs]->ctl, START_WR);

		ret = mt7620_spi_busy_poll(ms, cs);
		if (ret)
			return ret;

		len--;
	}

	return 0;
}

static int mt7620_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mt7620_spi *ms = dev_get_priv(bus);
	int total_size = bitlen >> 3;
	int cs, ret = 0;

	/*
	 * This driver only supports half-duplex, so complain and bail out
	 * upon full-duplex messages
	 */
	if (dout && din) {
		dev_err(dev, "mt7620_spi: Only half-duplex is supported\n");
		return -EIO;
	}

	cs = spi_chip_select(dev);
	if (cs < 0 || cs >= MT7620_SPI_NUM_CS) {
		dev_err(dev, "mt7620_spi: Invalid chip select %d\n", cs);
		return -EINVAL;
	}

	if (flags & SPI_XFER_BEGIN)
		mt7620_spi_set_cs(ms, cs, true);

	if (din)
		ret = mt7620_spi_read(ms, cs, din, total_size);
	else if (dout)
		ret = mt7620_spi_write(ms, cs, dout, total_size);

	if (ret)
		dev_err(dev, "mt7620_spi: %s transaction timeout\n",
			din ? "read" : "write");

	if (flags & SPI_XFER_END)
		mt7620_spi_set_cs(ms, cs, false);

	return ret;
}

static int mt7620_spi_probe(struct udevice *dev)
{
	struct mt7620_spi *ms = dev_get_priv(dev);
	struct clk clk;
	int ret;

	ms->regs = dev_remap_addr(dev);
	if (!ms->regs)
		return -EINVAL;

	ms->m[0] = ms->regs + MT7620_SPI_MASTER1_OFF;
	ms->m[1] = ms->regs + MT7620_SPI_MASTER2_OFF;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "mt7620_spi: Please provide a clock!\n");
		return ret;
	}

	clk_enable(&clk);

	ms->sys_freq = clk_get_rate(&clk);
	if (!ms->sys_freq) {
		dev_err(dev, "mt7620_spi: Please provide a valid bus clock!\n");
		return -EINVAL;
	}

	writel(ARB_EN, ms->regs + SPI_ARB);

	return 0;
}

static const struct dm_spi_ops mt7620_spi_ops = {
	.set_mode = mt7620_spi_set_mode,
	.set_speed = mt7620_spi_set_speed,
	.xfer = mt7620_spi_xfer,
};

static const struct udevice_id mt7620_spi_ids[] = {
	{ .compatible = "mediatek,mt7620-spi" },
	{ }
};

U_BOOT_DRIVER(mt7620_spi) = {
	.name = "mt7620_spi",
	.id = UCLASS_SPI,
	.of_match = mt7620_spi_ids,
	.ops = &mt7620_spi_ops,
	.priv_auto = sizeof(struct mt7620_spi),
	.probe = mt7620_spi_probe,
};
