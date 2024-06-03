// SPDX-License-Identifier: GPL-2.0+
/*
 * LPC32xx I2C interface driver
 *
 * (C) Copyright 2014-2015  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD - 3ADEV <albert.aribaud@3adev.fr>
 */

#include <common.h>
#include <asm/io.h>
#include <i2c.h>
#include <linux/errno.h>
#include <asm/arch/clk.h>
#include <asm/arch/i2c.h>
#include <dm.h>
#include <mapmem.h>

/*
 * Provide default speed and slave if target did not
 */

#if !defined(CONFIG_SYS_I2C_LPC32XX_SPEED)
#define CONFIG_SYS_I2C_LPC32XX_SPEED 350000
#endif

#if !defined(CONFIG_SYS_I2C_LPC32XX_SLAVE)
#define CONFIG_SYS_I2C_LPC32XX_SLAVE 0
#endif

/* TX register fields */
#define LPC32XX_I2C_TX_START		0x00000100
#define LPC32XX_I2C_TX_STOP		0x00000200

/* Control register values */
#define LPC32XX_I2C_SOFT_RESET		0x00000100

/* Status register values */
#define LPC32XX_I2C_STAT_TFF		0x00000400
#define LPC32XX_I2C_STAT_RFE		0x00000200
#define LPC32XX_I2C_STAT_DRMI		0x00000008
#define LPC32XX_I2C_STAT_NAI		0x00000004
#define LPC32XX_I2C_STAT_TDI		0x00000001

#ifndef CONFIG_DM_I2C
static struct lpc32xx_i2c_base *lpc32xx_i2c[] = {
	(struct lpc32xx_i2c_base *)I2C1_BASE,
	(struct lpc32xx_i2c_base *)I2C2_BASE,
	(struct lpc32xx_i2c_base *)(USB_BASE + 0x300)
};
#endif

/* Set I2C bus speed */
static unsigned int __i2c_set_bus_speed(struct lpc32xx_i2c_base *base,
					unsigned int speed, unsigned int chip)
{
	int half_period;

	if (speed == 0)
		return -EINVAL;

	/* OTG I2C clock source and CLK registers are different */
	if (chip == 2) {
		half_period = (get_periph_clk_rate() / speed) / 2;
		if (half_period > 0xFF)
			return -EINVAL;
	} else {
		half_period = (get_hclk_clk_rate() / speed) / 2;
		if (half_period > 0x3FF)
			return -EINVAL;
	}

	writel(half_period, &base->clk_hi);
	writel(half_period, &base->clk_lo);
	return 0;
}

/* I2C init called by cmd_i2c when doing 'i2c reset'. */
static void __i2c_init(struct lpc32xx_i2c_base *base,
		       int requested_speed, int slaveadd, unsigned int chip)
{
	/* soft reset (auto-clears) */
	writel(LPC32XX_I2C_SOFT_RESET, &base->ctrl);
	/* set HI and LO periods for half of the default speed */
	__i2c_set_bus_speed(base, requested_speed, chip);
}

/* I2C probe called by cmd_i2c when doing 'i2c probe'. */
static int __i2c_probe_chip(struct lpc32xx_i2c_base *base, u8 dev)
{
	int stat;

	/* Soft-reset the controller */
	writel(LPC32XX_I2C_SOFT_RESET, &base->ctrl);
	while (readl(&base->ctrl) & LPC32XX_I2C_SOFT_RESET)
		;
	/* Addre slave for write with start before and stop after */
	writel((dev<<1) | LPC32XX_I2C_TX_START | LPC32XX_I2C_TX_STOP,
	       &base->tx);
	/* wait for end of transation */
	while (!((stat = readl(&base->stat)) & LPC32XX_I2C_STAT_TDI))
		;
	/* was there no acknowledge? */
	return (stat & LPC32XX_I2C_STAT_NAI) ? -1 : 0;
}

/*
 * I2C read called by cmd_i2c when doing 'i2c read' and by cmd_eeprom.c
 * Begin write, send address byte(s), begin read, receive data bytes, end.
 */
static int __i2c_read(struct lpc32xx_i2c_base *base, u8 dev, uint addr,
		      int alen, u8 *data, int length)
{
	int stat, wlen;

	/* Soft-reset the controller */
	writel(LPC32XX_I2C_SOFT_RESET, &base->ctrl);
	while (readl(&base->ctrl) & LPC32XX_I2C_SOFT_RESET)
		;
	/* do we need to write an address at all? */
	if (alen) {
		/* Address slave in write mode */
		writel((dev<<1) | LPC32XX_I2C_TX_START, &base->tx);
		/* write address bytes */
		while (alen--) {
			/* compute address byte + stop for the last one */
			int a = (addr >> (8 * alen)) & 0xff;
			if (!alen)
				a |= LPC32XX_I2C_TX_STOP;
			/* Send address byte */
			writel(a, &base->tx);
		}
		/* wait for end of transation */
		while (!((stat = readl(&base->stat)) & LPC32XX_I2C_STAT_TDI))
			;
		/* clear end-of-transaction flag */
		writel(1, &base->stat);
	}
	/* do we have to read data at all? */
	if (length) {
		/* Address slave in read mode */
		writel(1 | (dev<<1) | LPC32XX_I2C_TX_START, &base->tx);
		wlen = length;
		/* get data */
		while (length | wlen) {
			/* read status for TFF and RFE */
			stat = readl(&base->stat);
			/* must we, can we write a trigger byte? */
			if ((wlen > 0)
			   & (!(stat & LPC32XX_I2C_STAT_TFF))) {
				wlen--;
				/* write trigger byte + stop if last */
				writel(wlen ? 0 :
				LPC32XX_I2C_TX_STOP, &base->tx);
			}
			/* must we, can we read a data byte? */
			if ((length > 0)
			   & (!(stat & LPC32XX_I2C_STAT_RFE))) {
				length--;
				/* read byte */
				*(data++) = readl(&base->rx);
			}
		}
		/* wait for end of transation */
		while (!((stat = readl(&base->stat)) & LPC32XX_I2C_STAT_TDI))
			;
		/* clear end-of-transaction flag */
		writel(1, &base->stat);
	}
	/* success */
	return 0;
}

/*
 * I2C write called by cmd_i2c when doing 'i2c write' and by cmd_eeprom.c
 * Begin write, send address byte(s), send data bytes, end.
 */
static int __i2c_write(struct lpc32xx_i2c_base *base, u8 dev, uint addr,
		       int alen, u8 *data, int length)
{
	int stat;

	/* Soft-reset the controller */
	writel(LPC32XX_I2C_SOFT_RESET, &base->ctrl);
	while (readl(&base->ctrl) & LPC32XX_I2C_SOFT_RESET)
		;
	/* do we need to write anything at all? */
	if (alen | length)
		/* Address slave in write mode */
		writel((dev<<1) | LPC32XX_I2C_TX_START, &base->tx);
	else
		return 0;
	/* write address bytes */
	while (alen) {
		/* wait for transmit fifo not full */
		stat = readl(&base->stat);
		if (!(stat & LPC32XX_I2C_STAT_TFF)) {
			alen--;
			int a = (addr >> (8 * alen)) & 0xff;
			if (!(alen | length))
				a |= LPC32XX_I2C_TX_STOP;
			/* Send address byte */
			writel(a, &base->tx);
		}
	}
	while (length) {
		/* wait for transmit fifo not full */
		stat = readl(&base->stat);
		if (!(stat & LPC32XX_I2C_STAT_TFF)) {
			/* compute data byte, add stop if length==0 */
			length--;
			int d = *(data++);
			if (!length)
				d |= LPC32XX_I2C_TX_STOP;
			/* Send data byte */
			writel(d, &base->tx);
		}
	}
	/* wait for end of transation */
	while (!((stat = readl(&base->stat)) & LPC32XX_I2C_STAT_TDI))
		;
	/* clear end-of-transaction flag */
	writel(1, &base->stat);
	return 0;
}

#ifndef CONFIG_DM_I2C
static void lpc32xx_i2c_init(struct i2c_adapter *adap,
			     int requested_speed, int slaveadd)
{
	__i2c_init(lpc32xx_i2c[adap->hwadapnr], requested_speed, slaveadd,
		   adap->hwadapnr);
}

static int lpc32xx_i2c_probe_chip(struct i2c_adapter *adap, u8 dev)
{
	return __i2c_probe_chip(lpc32xx_i2c[adap->hwadapnr], dev);
}

static int lpc32xx_i2c_read(struct i2c_adapter *adap, u8 dev, uint addr,
			    int alen, u8 *data, int length)
{
	return __i2c_read(lpc32xx_i2c[adap->hwadapnr], dev, addr,
			 alen, data, length);
}

static int lpc32xx_i2c_write(struct i2c_adapter *adap, u8 dev, uint addr,
			     int alen, u8 *data, int length)
{
	return __i2c_write(lpc32xx_i2c[adap->hwadapnr], dev, addr,
			  alen, data, length);
}

static unsigned int lpc32xx_i2c_set_bus_speed(struct i2c_adapter *adap,
					      unsigned int speed)
{
	return __i2c_set_bus_speed(lpc32xx_i2c[adap->hwadapnr], speed,
				  adap->hwadapnr);
}

U_BOOT_I2C_ADAP_COMPLETE(lpc32xx_0, lpc32xx_i2c_init, lpc32xx_i2c_probe_chip,
			 lpc32xx_i2c_read, lpc32xx_i2c_write,
			 lpc32xx_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_LPC32XX_SPEED,
			 CONFIG_SYS_I2C_LPC32XX_SLAVE,
			 0)

U_BOOT_I2C_ADAP_COMPLETE(lpc32xx_1, lpc32xx_i2c_init, lpc32xx_i2c_probe_chip,
			 lpc32xx_i2c_read, lpc32xx_i2c_write,
			 lpc32xx_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_LPC32XX_SPEED,
			 CONFIG_SYS_I2C_LPC32XX_SLAVE,
			 1)

U_BOOT_I2C_ADAP_COMPLETE(lpc32xx_2, lpc32xx_i2c_init, NULL,
			 lpc32xx_i2c_read, lpc32xx_i2c_write,
			 lpc32xx_i2c_set_bus_speed,
			 100000,
			 0,
			 2)
#else /* CONFIG_DM_I2C */
static int lpc32xx_i2c_probe(struct udevice *bus)
{
	struct lpc32xx_i2c_dev *dev = dev_get_platdata(bus);
	bus->seq = dev->index;

	__i2c_init(dev->base, dev->speed, 0, dev->index);
	return 0;
}

static int lpc32xx_i2c_probe_chip(struct udevice *bus, u32 chip_addr,
				  u32 chip_flags)
{
	struct lpc32xx_i2c_dev *dev = dev_get_platdata(bus);
	return __i2c_probe_chip(dev->base, chip_addr);
}

static int lpc32xx_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
		int nmsgs)
{
	struct lpc32xx_i2c_dev *dev = dev_get_platdata(bus);
	struct i2c_msg *dmsg, *omsg, dummy;
	uint i = 0, address = 0;

	memset(&dummy, 0, sizeof(struct i2c_msg));

	/* We expect either two messages (one with an offset and one with the
	 * actual data) or one message (just data)
	 */
	if (nmsgs > 2 || nmsgs == 0) {
		debug("%s: Only one or two messages are supported.", __func__);
		return -1;
	}

	omsg = nmsgs == 1 ? &dummy : msg;
	dmsg = nmsgs == 1 ? msg : msg + 1;

	/* the address is expected to be a uint, not a array. */
	address = omsg->buf[0];
	for (i = 1; i < omsg->len; i++)
		address = (address << 8) + omsg->buf[i];

	if (dmsg->flags & I2C_M_RD)
		return __i2c_read(dev->base, dmsg->addr, address,
				  omsg->len, dmsg->buf, dmsg->len);
	else
		return __i2c_write(dev->base, dmsg->addr, address,
				   omsg->len, dmsg->buf, dmsg->len);
}

static int lpc32xx_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct lpc32xx_i2c_dev *dev = dev_get_platdata(bus);
	return __i2c_set_bus_speed(dev->base, speed, dev->index);
}

static int lpc32xx_i2c_reset(struct udevice *bus)
{
	struct lpc32xx_i2c_dev *dev = dev_get_platdata(bus);

	__i2c_init(dev->base, dev->speed, 0, dev->index);
	return 0;
}

static const struct dm_i2c_ops lpc32xx_i2c_ops = {
	.xfer          = lpc32xx_i2c_xfer,
	.probe_chip    = lpc32xx_i2c_probe_chip,
	.deblock       = lpc32xx_i2c_reset,
	.set_bus_speed = lpc32xx_i2c_set_bus_speed,
};

U_BOOT_DRIVER(i2c_lpc32xx) = {
	.id                   = UCLASS_I2C,
	.name                 = "i2c_lpc32xx",
	.probe                = lpc32xx_i2c_probe,
	.ops                  = &lpc32xx_i2c_ops,
};
#endif /* CONFIG_DM_I2C */
