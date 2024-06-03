// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/errno.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <dm.h>
#include <i2c.h>
#include <fdtdec.h>

struct uniphier_fi2c_regs {
	u32 cr;				/* control register */
#define I2C_CR_MST	(1 << 3)	/* master mode */
#define I2C_CR_STA	(1 << 2)	/* start condition */
#define I2C_CR_STO	(1 << 1)	/* stop condition */
#define I2C_CR_NACK	(1 << 0)	/* not ACK */
	u32 dttx;			/* send FIFO (write-only) */
#define dtrx		dttx		/* receive FIFO (read-only) */
#define I2C_DTTX_CMD	(1 << 8)	/* send command (slave addr) */
#define I2C_DTTX_RD	(1 << 0)	/* read */
	u32 __reserved;			/* no register at offset 0x08 */
	u32 slad;			/* slave address */
	u32 cyc;			/* clock cycle control */
	u32 lctl;			/* clock low period control */
	u32 ssut;			/* restart/stop setup time control */
	u32 dsut;			/* data setup time control */
	u32 intr;			/* interrupt status */
	u32 ie;				/* interrupt enable */
	u32 ic;				/* interrupt clear */
#define I2C_INT_TE	(1 << 9)	/* TX FIFO empty */
#define I2C_INT_RB	(1 << 4)	/* received specified bytes */
#define I2C_INT_NA	(1 << 2)	/* no answer */
#define I2C_INT_AL	(1 << 1)	/* arbitration lost */
	u32 sr;				/* status register */
#define I2C_SR_DB	(1 << 12)	/* device busy */
#define I2C_SR_BB	(1 << 8)	/* bus busy */
#define I2C_SR_RFF	(1 << 3)	/* Rx FIFO full */
#define I2C_SR_RNE	(1 << 2)	/* Rx FIFO not empty */
#define I2C_SR_TNF	(1 << 1)	/* Tx FIFO not full */
#define I2C_SR_TFE	(1 << 0)	/* Tx FIFO empty */
	u32 __reserved2;		/* no register at offset 0x30 */
	u32 rst;			/* reset control */
#define I2C_RST_TBRST	(1 << 2)	/* clear Tx FIFO */
#define I2C_RST_RBRST	(1 << 1)	/* clear Rx FIFO */
#define I2C_RST_RST	(1 << 0)	/* forcible bus reset */
	u32 bm;				/* bus monitor */
	u32 noise;			/* noise filter control */
	u32 tbc;			/* Tx byte count setting */
	u32 rbc;			/* Rx byte count setting */
	u32 tbcm;			/* Tx byte count monitor */
	u32 rbcm;			/* Rx byte count monitor */
	u32 brst;			/* bus reset */
#define I2C_BRST_FOEN	(1 << 1)	/* normal operation */
#define I2C_BRST_RSCLO	(1 << 0)	/* release SCL low fixing */
};

#define FIOCLK	50000000

struct uniphier_fi2c_priv {
	struct udevice *dev;
	struct uniphier_fi2c_regs __iomem *regs;	/* register base */
	unsigned long fioclk;			/* internal operation clock */
	unsigned long timeout;			/* time out (us) */
};

static void uniphier_fi2c_reset(struct uniphier_fi2c_priv *priv)
{
	writel(I2C_RST_RST, &priv->regs->rst);
}

static int uniphier_fi2c_check_bus_busy(struct uniphier_fi2c_priv *priv)
{
	u32 val;
	int ret;

	ret = readl_poll_timeout(&priv->regs->sr, val, !(val & I2C_SR_DB), 100);
	if (ret < 0) {
		dev_dbg(priv->dev, "error: device busy too long. reset...\n");
		uniphier_fi2c_reset(priv);
	}

	return ret;
}

static int uniphier_fi2c_probe(struct udevice *dev)
{
	fdt_addr_t addr;
	struct uniphier_fi2c_priv *priv = dev_get_priv(dev);

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = devm_ioremap(dev, addr, SZ_128);
	if (!priv->regs)
		return -ENOMEM;

	priv->fioclk = FIOCLK;

	priv->dev = dev;

	/* bus forcible reset */
	uniphier_fi2c_reset(priv);

	writel(I2C_BRST_FOEN | I2C_BRST_RSCLO, &priv->regs->brst);

	return 0;
}

static int wait_for_irq(struct uniphier_fi2c_priv *priv, u32 flags,
			bool *stop)
{
	u32 irq;
	int ret;

	ret = readl_poll_timeout(&priv->regs->intr, irq, irq & flags,
				 priv->timeout);
	if (ret < 0) {
		dev_dbg(priv->dev, "error: time out\n");
		return ret;
	}

	if (irq & I2C_INT_AL) {
		dev_dbg(priv->dev, "error: arbitration lost\n");
		*stop = false;
		return ret;
	}

	if (irq & I2C_INT_NA) {
		dev_dbg(priv->dev, "error: no answer\n");
		return ret;
	}

	return 0;
}

static int issue_stop(struct uniphier_fi2c_priv *priv, int old_ret)
{
	int ret;

	dev_dbg(priv->dev, "stop condition\n");
	writel(I2C_CR_MST | I2C_CR_STO, &priv->regs->cr);

	ret = uniphier_fi2c_check_bus_busy(priv);
	if (ret < 0)
		dev_dbg(priv->dev, "error: device busy after operation\n");

	return old_ret ? old_ret : ret;
}

static int uniphier_fi2c_transmit(struct uniphier_fi2c_priv *priv, uint addr,
				  uint len, const u8 *buf, bool *stop)
{
	int ret;
	const u32 irq_flags = I2C_INT_TE | I2C_INT_NA | I2C_INT_AL;
	struct uniphier_fi2c_regs __iomem *regs = priv->regs;

	dev_dbg(priv->dev, "%s: addr = %x, len = %d\n", __func__, addr, len);

	writel(I2C_DTTX_CMD | addr << 1, &regs->dttx);

	writel(irq_flags, &regs->ie);
	writel(irq_flags, &regs->ic);

	dev_dbg(priv->dev, "start condition\n");
	writel(I2C_CR_MST | I2C_CR_STA, &regs->cr);

	ret = wait_for_irq(priv, irq_flags, stop);
	if (ret < 0)
		goto error;

	while (len--) {
		dev_dbg(priv->dev, "sending %x\n", *buf);
		writel(*buf++, &regs->dttx);

		writel(irq_flags, &regs->ic);

		ret = wait_for_irq(priv, irq_flags, stop);
		if (ret < 0)
			goto error;
	}

error:
	writel(irq_flags, &regs->ic);

	if (*stop)
		ret = issue_stop(priv, ret);

	return ret;
}

static int uniphier_fi2c_receive(struct uniphier_fi2c_priv *priv, uint addr,
				 uint len, u8 *buf, bool *stop)
{
	int ret = 0;
	const u32 irq_flags = I2C_INT_RB | I2C_INT_NA | I2C_INT_AL;
	struct uniphier_fi2c_regs __iomem *regs = priv->regs;

	dev_dbg(priv->dev, "%s: addr = %x, len = %d\n", __func__, addr, len);

	/*
	 * In case 'len == 0', only the slave address should be sent
	 * for probing, which is covered by the transmit function.
	 */
	if (len == 0)
		return uniphier_fi2c_transmit(priv, addr, len, buf, stop);

	writel(I2C_DTTX_CMD | I2C_DTTX_RD | addr << 1, &regs->dttx);

	writel(0, &regs->rbc);
	writel(irq_flags, &regs->ie);
	writel(irq_flags, &regs->ic);

	dev_dbg(priv->dev, "start condition\n");
	writel(I2C_CR_MST | I2C_CR_STA | (len == 1 ? I2C_CR_NACK : 0),
	       &regs->cr);

	while (len--) {
		ret = wait_for_irq(priv, irq_flags, stop);
		if (ret < 0)
			goto error;

		*buf++ = readl(&regs->dtrx);
		dev_dbg(priv->dev, "received %x\n", *(buf - 1));

		if (len == 1)
			writel(I2C_CR_MST | I2C_CR_NACK, &regs->cr);

		writel(irq_flags, &regs->ic);
	}

error:
	writel(irq_flags, &regs->ic);

	if (*stop)
		ret = issue_stop(priv, ret);

	return ret;
}

static int uniphier_fi2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			     int nmsgs)
{
	int ret;
	struct uniphier_fi2c_priv *priv = dev_get_priv(bus);
	bool stop;

	ret = uniphier_fi2c_check_bus_busy(priv);
	if (ret < 0)
		return ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		/* If next message is read, skip the stop condition */
		stop = nmsgs > 1 && msg[1].flags & I2C_M_RD ? false : true;

		if (msg->flags & I2C_M_RD)
			ret = uniphier_fi2c_receive(priv, msg->addr, msg->len,
						    msg->buf, &stop);
		else
			ret = uniphier_fi2c_transmit(priv, msg->addr, msg->len,
						     msg->buf, &stop);

		if (ret < 0)
			break;
	}

	return ret;
}

static int uniphier_fi2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	int ret;
	unsigned int clk_count;
	struct uniphier_fi2c_priv *priv = dev_get_priv(bus);
	struct uniphier_fi2c_regs __iomem *regs = priv->regs;

	/* max supported frequency is 400 kHz */
	if (speed > 400000)
		return -EINVAL;

	ret = uniphier_fi2c_check_bus_busy(priv);
	if (ret < 0)
		return ret;

	/* make sure the bus is idle when changing the frequency */
	writel(I2C_BRST_RSCLO, &regs->brst);

	clk_count = priv->fioclk / speed;

	writel(clk_count, &regs->cyc);
	writel(clk_count / 2, &regs->lctl);
	writel(clk_count / 2, &regs->ssut);
	writel(clk_count / 16, &regs->dsut);

	writel(I2C_BRST_FOEN | I2C_BRST_RSCLO, &regs->brst);

	/*
	 * Theoretically, each byte can be transferred in
	 * 1000000 * 9 / speed usec.
	 * This time out value is long enough.
	 */
	priv->timeout = 100000000L / speed;

	return 0;
}

static const struct dm_i2c_ops uniphier_fi2c_ops = {
	.xfer = uniphier_fi2c_xfer,
	.set_bus_speed = uniphier_fi2c_set_bus_speed,
};

static const struct udevice_id uniphier_fi2c_of_match[] = {
	{ .compatible = "socionext,uniphier-fi2c" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_fi2c) = {
	.name = "uniphier-fi2c",
	.id = UCLASS_I2C,
	.of_match = uniphier_fi2c_of_match,
	.probe = uniphier_fi2c_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_fi2c_priv),
	.ops = &uniphier_fi2c_ops,
};
