// SPDX-License-Identifier: GPL-2.0+
/*
 * drivers/i2c/rcar_i2c.c
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 *
 * Clock configuration based on Linux i2c-rcar.c:
 * Copyright (C) 2014-15 Wolfram Sang <wsa@sang-engineering.com>
 * Copyright (C) 2011-2015 Renesas Electronics Corporation
 * Copyright (C) 2012-14 Renesas Solutions Corp.
 *   Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <wait_bit.h>

#define RCAR_I2C_ICSCR			0x00 /* slave ctrl */
#define RCAR_I2C_ICMCR			0x04 /* master ctrl */
#define RCAR_I2C_ICMCR_MDBS		BIT(7) /* non-fifo mode switch */
#define RCAR_I2C_ICMCR_FSCL		BIT(6) /* override SCL pin */
#define RCAR_I2C_ICMCR_FSDA		BIT(5) /* override SDA pin */
#define RCAR_I2C_ICMCR_OBPC		BIT(4) /* override pins */
#define RCAR_I2C_ICMCR_MIE		BIT(3) /* master if enable */
#define RCAR_I2C_ICMCR_TSBE		BIT(2)
#define RCAR_I2C_ICMCR_FSB		BIT(1) /* force stop bit */
#define RCAR_I2C_ICMCR_ESG		BIT(0) /* enable start bit gen */
#define RCAR_I2C_ICSSR			0x08 /* slave status */
#define RCAR_I2C_ICMSR			0x0c /* master status */
#define RCAR_I2C_ICMSR_MASK		0x7f
#define RCAR_I2C_ICMSR_MNR		BIT(6) /* Nack */
#define RCAR_I2C_ICMSR_MAL		BIT(5) /* Arbitration lost */
#define RCAR_I2C_ICMSR_MST		BIT(4) /* Stop */
#define RCAR_I2C_ICMSR_MDE		BIT(3)
#define RCAR_I2C_ICMSR_MDT		BIT(2)
#define RCAR_I2C_ICMSR_MDR		BIT(1)
#define RCAR_I2C_ICMSR_MAT		BIT(0)
#define RCAR_I2C_ICSIER			0x10 /* slave irq enable */
#define RCAR_I2C_ICMIER			0x14 /* master irq enable */
#define RCAR_I2C_ICCCR			0x18 /* clock dividers */
#define RCAR_I2C_ICCCR_SCGD_OFF		3
#define RCAR_I2C_ICSAR			0x1c /* slave address */
#define RCAR_I2C_ICMAR			0x20 /* master address */
#define RCAR_I2C_ICRXD_ICTXD		0x24 /* data port */
/*
 * First Bit Setup Cycle (Gen3).
 * Defines 1st bit delay between SDA and SCL.
 */
#define RCAR_I2C_ICFBSCR		0x38
#define RCAR_I2C_ICFBSCR_TCYC17		0x0f /* 17*Tcyc */


enum rcar_i2c_type {
	RCAR_I2C_TYPE_GEN2,
	RCAR_I2C_TYPE_GEN3,
};

struct rcar_i2c_priv {
	void __iomem		*base;
	struct clk		clk;
	u32			intdelay;
	u32			icccr;
	enum rcar_i2c_type	type;
};

static int rcar_i2c_finish(struct udevice *dev)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	ret = wait_for_bit_le32(priv->base + RCAR_I2C_ICMSR, RCAR_I2C_ICMSR_MST,
				true, 10, true);

	writel(0, priv->base + RCAR_I2C_ICSSR);
	writel(0, priv->base + RCAR_I2C_ICMSR);
	writel(0, priv->base + RCAR_I2C_ICMCR);

	return ret;
}

static int rcar_i2c_recover(struct udevice *dev)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	u32 mcr = RCAR_I2C_ICMCR_MDBS | RCAR_I2C_ICMCR_OBPC;
	u32 mcra = mcr | RCAR_I2C_ICMCR_FSDA;
	int i;
	u32 mstat;

	/* Send 9 SCL pulses */
	for (i = 0; i < 9; i++) {
		writel(mcra | RCAR_I2C_ICMCR_FSCL, priv->base + RCAR_I2C_ICMCR);
		udelay(5);
		writel(mcra, priv->base + RCAR_I2C_ICMCR);
		udelay(5);
	}

	/* Send stop condition */
	udelay(5);
	writel(mcra, priv->base + RCAR_I2C_ICMCR);
	udelay(5);
	writel(mcr, priv->base + RCAR_I2C_ICMCR);
	udelay(5);
	writel(mcr | RCAR_I2C_ICMCR_FSCL, priv->base + RCAR_I2C_ICMCR);
	udelay(5);
	writel(mcra | RCAR_I2C_ICMCR_FSCL, priv->base + RCAR_I2C_ICMCR);
	udelay(5);

	mstat = readl(priv->base + RCAR_I2C_ICMSR);
	return mstat & RCAR_I2C_ICMCR_FSDA ? -EBUSY : 0;
}

static int rcar_i2c_set_addr(struct udevice *dev, u8 chip, u8 read)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	u32 mask = RCAR_I2C_ICMSR_MAT |
		   (read ? RCAR_I2C_ICMSR_MDR : RCAR_I2C_ICMSR_MDE);
	int ret;

	writel(0, priv->base + RCAR_I2C_ICMIER);
	writel(RCAR_I2C_ICMCR_MDBS, priv->base + RCAR_I2C_ICMCR);
	writel(0, priv->base + RCAR_I2C_ICMSR);
	writel(priv->icccr, priv->base + RCAR_I2C_ICCCR);

	/* Wait for the bus */
	ret = wait_for_bit_le32(priv->base + RCAR_I2C_ICMCR,
				RCAR_I2C_ICMCR_FSDA, false, 2, true);
	if (ret) {
		if (rcar_i2c_recover(dev)) {
			dev_err(dev, "Bus busy, aborting\n");
			return ret;
		}
	}

	writel((chip << 1) | read, priv->base + RCAR_I2C_ICMAR);
	/* Reset */
	writel(RCAR_I2C_ICMCR_MDBS | RCAR_I2C_ICMCR_MIE | RCAR_I2C_ICMCR_ESG,
	       priv->base + RCAR_I2C_ICMCR);
	/* Clear Status */
	writel(0, priv->base + RCAR_I2C_ICMSR);

	ret = wait_for_bit_le32(priv->base + RCAR_I2C_ICMSR, mask,
				true, 100, true);
	if (ret)
		return ret;

	/* Check NAK */
	if (readl(priv->base + RCAR_I2C_ICMSR) & RCAR_I2C_ICMSR_MNR)
		return -EREMOTEIO;

	return 0;
}

static int rcar_i2c_read_common(struct udevice *dev, struct i2c_msg *msg)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	u32 icmcr = RCAR_I2C_ICMCR_MDBS | RCAR_I2C_ICMCR_MIE;
	int i, ret = -EREMOTEIO;

	for (i = 0; i < msg->len; i++) {
		if (msg->len - 1 == i)
			icmcr |= RCAR_I2C_ICMCR_FSB;

		writel(icmcr, priv->base + RCAR_I2C_ICMCR);
		writel((u32)~RCAR_I2C_ICMSR_MDR, priv->base + RCAR_I2C_ICMSR);

		ret = wait_for_bit_le32(priv->base + RCAR_I2C_ICMSR,
					RCAR_I2C_ICMSR_MDR, true, 100, true);
		if (ret)
			return ret;

		msg->buf[i] = readl(priv->base + RCAR_I2C_ICRXD_ICTXD) & 0xff;
	}

	writel((u32)~RCAR_I2C_ICMSR_MDR, priv->base + RCAR_I2C_ICMSR);

	return rcar_i2c_finish(dev);
}

static int rcar_i2c_write_common(struct udevice *dev, struct i2c_msg *msg)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	u32 icmcr = RCAR_I2C_ICMCR_MDBS | RCAR_I2C_ICMCR_MIE;
	int i, ret = -EREMOTEIO;

	for (i = 0; i < msg->len; i++) {
		writel(msg->buf[i], priv->base + RCAR_I2C_ICRXD_ICTXD);
		writel(icmcr, priv->base + RCAR_I2C_ICMCR);
		writel((u32)~RCAR_I2C_ICMSR_MDE, priv->base + RCAR_I2C_ICMSR);

		ret = wait_for_bit_le32(priv->base + RCAR_I2C_ICMSR,
					RCAR_I2C_ICMSR_MDE, true, 100, true);
		if (ret)
			return ret;
	}

	writel((u32)~RCAR_I2C_ICMSR_MDE, priv->base + RCAR_I2C_ICMSR);
	icmcr |= RCAR_I2C_ICMCR_FSB;
	writel(icmcr, priv->base + RCAR_I2C_ICMCR);

	return rcar_i2c_finish(dev);
}

static int rcar_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	int ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		ret = rcar_i2c_set_addr(dev, msg->addr, 1);
		if (ret)
			return ret;

		if (msg->flags & I2C_M_RD)
			ret = rcar_i2c_read_common(dev, msg);
		else
			ret = rcar_i2c_write_common(dev, msg);

		if (ret)
			return ret;
	}

	return 0;
}

static int rcar_i2c_probe_chip(struct udevice *dev, uint addr, uint flags)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	/* Ignore address 0, slave address */
	if (addr == 0)
		return -EINVAL;

	ret = rcar_i2c_set_addr(dev, addr, 1);
	writel(0, priv->base + RCAR_I2C_ICMSR);
	return ret;
}

static int rcar_i2c_set_speed(struct udevice *dev, uint bus_freq_hz)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	u32 scgd, cdf, round, ick, sum, scl;
	unsigned long rate;

	/*
	 * calculate SCL clock
	 * see
	 *	ICCCR
	 *
	 * ick	= clkp / (1 + CDF)
	 * SCL	= ick / (20 + SCGD * 8 + F[(ticf + tr + intd) * ick])
	 *
	 * ick  : I2C internal clock < 20 MHz
	 * ticf : I2C SCL falling time
	 * tr   : I2C SCL rising  time
	 * intd : LSI internal delay
	 * clkp : peripheral_clk
	 * F[]  : integer up-valuation
	 */
	rate = clk_get_rate(&priv->clk);
	cdf = rate / 20000000;
	if (cdf >= 8) {
		dev_err(dev, "Input clock %lu too high\n", rate);
		return -EIO;
	}
	ick = rate / (cdf + 1);

	/*
	 * it is impossible to calculate large scale
	 * number on u32. separate it
	 *
	 * F[(ticf + tr + intd) * ick] with sum = (ticf + tr + intd)
	 *  = F[sum * ick / 1000000000]
	 *  = F[(ick / 1000000) * sum / 1000]
	 */
	sum = 35 + 200 + priv->intdelay;
	round = (ick + 500000) / 1000000 * sum;
	round = (round + 500) / 1000;

	/*
	 * SCL	= ick / (20 + SCGD * 8 + F[(ticf + tr + intd) * ick])
	 *
	 * Calculation result (= SCL) should be less than
	 * bus_speed for hardware safety
	 *
	 * We could use something along the lines of
	 *	div = ick / (bus_speed + 1) + 1;
	 *	scgd = (div - 20 - round + 7) / 8;
	 *	scl = ick / (20 + (scgd * 8) + round);
	 * (not fully verified) but that would get pretty involved
	 */
	for (scgd = 0; scgd < 0x40; scgd++) {
		scl = ick / (20 + (scgd * 8) + round);
		if (scl <= bus_freq_hz)
			goto scgd_find;
	}
	dev_err(dev, "it is impossible to calculate best SCL\n");
	return -EIO;

scgd_find:
	dev_dbg(dev, "clk %d/%d(%lu), round %u, CDF:0x%x, SCGD: 0x%x\n",
		scl, bus_freq_hz, clk_get_rate(&priv->clk), round, cdf, scgd);

	priv->icccr = (scgd << RCAR_I2C_ICCCR_SCGD_OFF) | cdf;
	writel(priv->icccr, priv->base + RCAR_I2C_ICCCR);

	if (priv->type == RCAR_I2C_TYPE_GEN3) {
		/* Set SCL/SDA delay */
		writel(RCAR_I2C_ICFBSCR_TCYC17, priv->base + RCAR_I2C_ICFBSCR);
	}

	return 0;
}

static int rcar_i2c_probe(struct udevice *dev)
{
	struct rcar_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	priv->intdelay = dev_read_u32_default(dev,
					      "i2c-scl-internal-delay-ns", 5);
	priv->type = dev_get_driver_data(dev);

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	/* reset slave mode */
	writel(0, priv->base + RCAR_I2C_ICSIER);
	writel(0, priv->base + RCAR_I2C_ICSAR);
	writel(0, priv->base + RCAR_I2C_ICSCR);
	writel(0, priv->base + RCAR_I2C_ICSSR);

	/* reset master mode */
	writel(0, priv->base + RCAR_I2C_ICMIER);
	writel(0, priv->base + RCAR_I2C_ICMCR);
	writel(0, priv->base + RCAR_I2C_ICMSR);
	writel(0, priv->base + RCAR_I2C_ICMAR);

	ret = rcar_i2c_set_speed(dev, 100000);
	if (ret)
		clk_disable(&priv->clk);

	return ret;
}

static const struct dm_i2c_ops rcar_i2c_ops = {
	.xfer		= rcar_i2c_xfer,
	.probe_chip	= rcar_i2c_probe_chip,
	.set_bus_speed	= rcar_i2c_set_speed,
};

static const struct udevice_id rcar_i2c_ids[] = {
	{ .compatible = "renesas,rcar-gen2-i2c", .data = RCAR_I2C_TYPE_GEN2 },
	{ .compatible = "renesas,rcar-gen3-i2c", .data = RCAR_I2C_TYPE_GEN3 },
	{ }
};

U_BOOT_DRIVER(i2c_rcar) = {
	.name		= "i2c_rcar",
	.id		= UCLASS_I2C,
	.of_match	= rcar_i2c_ids,
	.probe		= rcar_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct rcar_i2c_priv),
	.ops		= &rcar_i2c_ops,
};
