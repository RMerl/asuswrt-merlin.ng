// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar IIC driver
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on
 * Copyright (C) 2011, 2013 Renesas Solutions Corp.
 * Copyright (C) 2011, 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>

struct rcar_iic_priv {
	void __iomem		*base;
	struct clk		clk;
	u8			iccl;
	u8			icch;
};

#define RCAR_IIC_ICDR		0x00
#define RCAR_IIC_ICCR		0x04
#define RCAR_IIC_ICSR		0x08
#define RCAR_IIC_ICIC		0x0c
#define RCAR_IIC_ICCL		0x10
#define RCAR_IIC_ICCH		0x14

/* ICCR */
#define RCAR_IIC_ICCR_ICE	BIT(7)
#define RCAR_IIC_ICCR_RACK	BIT(6)
#define RCAR_IIC_ICCR_RTS	BIT(4)
#define RCAR_IIC_ICCR_BUSY	BIT(2)
#define RCAR_IIC_ICCR_SCP	BIT(0)

/* ICSR / ICIC */
#define RCAR_IC_BUSY		BIT(4)
#define RCAR_IC_TACK		BIT(2)
#define RCAR_IC_DTE		BIT(0)

#define IRQ_WAIT 1000

static void sh_irq_dte(struct udevice *dev)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if (RCAR_IC_DTE & readb(priv->base + RCAR_IIC_ICSR))
			break;
		udelay(10);
	}
}

static int sh_irq_dte_with_tack(struct udevice *dev)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	u8 icsr;
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		icsr = readb(priv->base + RCAR_IIC_ICSR);
		if (RCAR_IC_DTE & icsr)
			break;
		if (RCAR_IC_TACK & icsr)
			return -ETIMEDOUT;
		udelay(10);
	}
	return 0;
}

static void sh_irq_busy(struct udevice *dev)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if (!(RCAR_IC_BUSY & readb(priv->base + RCAR_IIC_ICSR)))
			break;
		udelay(10);
	}
}

static int rcar_iic_set_addr(struct udevice *dev, u8 chip, u8 read)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);

	clrbits_8(priv->base + RCAR_IIC_ICCR, RCAR_IIC_ICCR_ICE);
	setbits_8(priv->base + RCAR_IIC_ICCR, RCAR_IIC_ICCR_ICE);

	writeb(priv->iccl, priv->base + RCAR_IIC_ICCL);
	writeb(priv->icch, priv->base + RCAR_IIC_ICCH);
	writeb(RCAR_IC_TACK, priv->base + RCAR_IIC_ICIC);

	writeb(RCAR_IIC_ICCR_ICE | RCAR_IIC_ICCR_RTS | RCAR_IIC_ICCR_BUSY,
	       priv->base + RCAR_IIC_ICCR);
	sh_irq_dte(dev);

	clrbits_8(priv->base + RCAR_IIC_ICSR, RCAR_IC_TACK);
	writeb(chip << 1 | read, priv->base + RCAR_IIC_ICDR);
	return sh_irq_dte_with_tack(dev);
}

static void rcar_iic_finish(struct udevice *dev)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);

	writeb(0, priv->base + RCAR_IIC_ICSR);
	clrbits_8(priv->base + RCAR_IIC_ICCR, RCAR_IIC_ICCR_ICE);
}

static int rcar_iic_read_common(struct udevice *dev, struct i2c_msg *msg)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	int i, ret = -EREMOTEIO;

	if (rcar_iic_set_addr(dev, msg->addr, 1) != 0)
		goto err;

	udelay(10);

	writeb(RCAR_IIC_ICCR_ICE | RCAR_IIC_ICCR_SCP,
	       priv->base + RCAR_IIC_ICCR);

	for (i = 0; i < msg->len; i++) {
		if (sh_irq_dte_with_tack(dev) != 0)
			goto err;

		msg->buf[i] = readb(priv->base + RCAR_IIC_ICDR) & 0xff;

		if (msg->len - 1 == i) {
			writeb(RCAR_IIC_ICCR_ICE | RCAR_IIC_ICCR_RACK,
			       priv->base + RCAR_IIC_ICCR);
		}
	}

	sh_irq_busy(dev);
	ret = 0;

err:
	rcar_iic_finish(dev);
	return ret;
}

static int rcar_iic_write_common(struct udevice *dev, struct i2c_msg *msg)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	int i, ret = -EREMOTEIO;

	if (rcar_iic_set_addr(dev, msg->addr, 0) != 0)
		goto err;

	udelay(10);

	for (i = 0; i < msg->len; i++) {
		writeb(msg->buf[i], priv->base + RCAR_IIC_ICDR);
		if (sh_irq_dte_with_tack(dev) != 0)
			goto err;
	}

	if (msg->flags & I2C_M_STOP) {
		writeb(RCAR_IIC_ICCR_ICE | RCAR_IIC_ICCR_RTS,
		       priv->base + RCAR_IIC_ICCR);
		if (sh_irq_dte_with_tack(dev) != 0)
			goto err;
	}

	sh_irq_busy(dev);
	ret = 0;

err:
	rcar_iic_finish(dev);
	return ret;
}

static int rcar_iic_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	int ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD)
			ret = rcar_iic_read_common(dev, msg);
		else
			ret = rcar_iic_write_common(dev, msg);

		if (ret)
			return -EREMOTEIO;
	}

	return ret;
}

static int rcar_iic_set_speed(struct udevice *dev, uint speed)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	const unsigned int ratio_high = 4;
	const unsigned int ratio_low = 5;
	int clkrate, denom;

	clkrate = clk_get_rate(&priv->clk);
	if (clkrate < 0)
		return clkrate;

	/*
	 * Calculate the value for ICCL and ICCH. From the data sheet:
	 * iccl = (p-clock / transfer-rate) * (L / (L + H))
	 * icch = (p clock / transfer rate) * (H / (L + H))
	 * where L and H are the SCL low and high ratio.
	 */
	denom = speed * (ratio_high + ratio_low);
	priv->iccl = DIV_ROUND_CLOSEST(clkrate * ratio_low, denom);
	priv->icch = DIV_ROUND_CLOSEST(clkrate * ratio_high, denom);

	return 0;
}

static int rcar_iic_probe_chip(struct udevice *dev, uint addr, uint flags)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	int ret;

	rcar_iic_set_addr(dev, addr, 1);
	writeb(RCAR_IIC_ICCR_ICE | RCAR_IIC_ICCR_SCP,
	       priv->base + RCAR_IIC_ICCR);
	ret = sh_irq_dte_with_tack(dev);
	rcar_iic_finish(dev);

	return ret;
}

static int rcar_iic_probe(struct udevice *dev)
{
	struct rcar_iic_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	rcar_iic_finish(dev);

	return rcar_iic_set_speed(dev, 100000);
}

static const struct dm_i2c_ops rcar_iic_ops = {
	.xfer		= rcar_iic_xfer,
	.probe_chip	= rcar_iic_probe_chip,
	.set_bus_speed	= rcar_iic_set_speed,
};

static const struct udevice_id rcar_iic_ids[] = {
	{ .compatible = "renesas,rmobile-iic" },
	{ }
};

U_BOOT_DRIVER(iic_rcar) = {
	.name		= "iic_rcar",
	.id		= UCLASS_I2C,
	.of_match	= rcar_iic_ids,
	.probe		= rcar_iic_probe,
	.priv_auto_alloc_size = sizeof(struct rcar_iic_priv),
	.ops		= &rcar_iic_ops,
};
