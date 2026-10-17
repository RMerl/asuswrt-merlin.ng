// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <i2c.h>
#include <dm.h>
#include <regmap.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/unaligned.h>
#include <linux/bitops.h>
#include <linux/delay.h>

struct ihs_i2c_priv {
	uint speed;
	struct regmap *map;
};

struct ihs_i2c_regs {
	u16 interrupt_status;
	u16 interrupt_enable_control;
	u16 write_mailbox_ext;
	u16 write_mailbox;
	u16 read_mailbox_ext;
	u16 read_mailbox;
};

#define ihs_i2c_set(map, member, val) \
	regmap_set(map, struct ihs_i2c_regs, member, val)

#define ihs_i2c_get(map, member, valp) \
	regmap_get(map, struct ihs_i2c_regs, member, valp)

enum {
	I2CINT_ERROR_EV = BIT(13),
	I2CINT_TRANSMIT_EV = BIT(14),
	I2CINT_RECEIVE_EV = BIT(15),
};

enum {
	I2CMB_READ = 0 << 10,
	I2CMB_WRITE = 1 << 10,
	I2CMB_1BYTE = 0 << 11,
	I2CMB_2BYTE = 1 << 11,
	I2CMB_DONT_HOLD_BUS = 0 << 13,
	I2CMB_HOLD_BUS = 1 << 13,
	I2CMB_NATIVE = 2 << 14,
};

enum {
	I2COP_WRITE = 0,
	I2COP_READ = 1,
};

static int wait_for_int(struct udevice *dev, int read)
{
	u16 val;
	uint ctr = 0;
	struct ihs_i2c_priv *priv = dev_get_priv(dev);

	ihs_i2c_get(priv->map, interrupt_status, &val);
	/* Wait until error or receive/transmit interrupt was raised */
	while (!(val & (I2CINT_ERROR_EV
	       | (read ? I2CINT_RECEIVE_EV : I2CINT_TRANSMIT_EV)))) {
		udelay(10);
		if (ctr++ > 5000) {
			debug("%s: timed out\n", __func__);
			return -ETIMEDOUT;
		}
		ihs_i2c_get(priv->map, interrupt_status, &val);
	}

	return (val & I2CINT_ERROR_EV) ? -EIO : 0;
}

static int ihs_i2c_transfer(struct udevice *dev, uchar chip,
			    uchar *buffer, int len, int read, bool is_last)
{
	u16 val;
	u16 data;
	int res;
	struct ihs_i2c_priv *priv = dev_get_priv(dev);

	/* Clear interrupt status */
	data = I2CINT_ERROR_EV | I2CINT_RECEIVE_EV | I2CINT_TRANSMIT_EV;
	ihs_i2c_set(priv->map, interrupt_status, data);
	ihs_i2c_get(priv->map, interrupt_status, &val);

	/* If we want to write and have data, write the bytes to the mailbox */
	if (!read && len) {
		val = buffer[0];

		if (len > 1)
			val |= buffer[1] << 8;
		ihs_i2c_set(priv->map, write_mailbox_ext, val);
	}

	data = I2CMB_NATIVE
	       | (read ? 0 : I2CMB_WRITE)
	       | (chip << 1)
	       | ((len > 1) ? I2CMB_2BYTE : 0)
	       | (is_last ? 0 : I2CMB_HOLD_BUS);

	ihs_i2c_set(priv->map, write_mailbox, data);

	res = wait_for_int(dev, read);
	if (res) {
		if (res == -ETIMEDOUT)
			debug("%s: time out while waiting for event\n", __func__);

		return res;
	}

	/* If we want to read, get the bytes from the mailbox */
	if (read) {
		ihs_i2c_get(priv->map, read_mailbox_ext, &val);
		buffer[0] = val & 0xff;
		if (len > 1)
			buffer[1] = val >> 8;
	}

	return 0;
}

static int ihs_i2c_send_buffer(struct udevice *dev, uchar chip, u8 *data, int len, bool hold_bus, int read)
{
	int res;

	while (len) {
		int transfer = min(len, 2);
		bool is_last = len <= transfer;

		res = ihs_i2c_transfer(dev, chip, data, transfer, read,
				       hold_bus ? false : is_last);
		if (res)
			return res;

		data += transfer;
		len -= transfer;
	}

	return 0;
}

static int ihs_i2c_address(struct udevice *dev, uchar chip, u8 *addr, int alen,
			   bool hold_bus)
{
	return ihs_i2c_send_buffer(dev, chip, addr, alen, hold_bus, I2COP_WRITE);
}

static int ihs_i2c_access(struct udevice *dev, uchar chip, u8 *addr,
			  int alen, uchar *buffer, int len, int read)
{
	int res;

	/* Don't hold the bus if length of data to send/receive is zero */
	if (len <= 0)
		return -EINVAL;

	res = ihs_i2c_address(dev, chip, addr, alen, len);
	if (res)
		return res;

	return ihs_i2c_send_buffer(dev, chip, buffer, len, false, read);
}

int ihs_i2c_probe(struct udevice *bus)
{
	struct ihs_i2c_priv *priv = dev_get_priv(bus);

	regmap_init_mem(dev_ofnode(bus), &priv->map);

	return 0;
}

static int ihs_i2c_set_bus_speed(struct udevice *bus, uint speed)
{
	struct ihs_i2c_priv *priv = dev_get_priv(bus);

	if (speed != priv->speed && priv->speed != 0)
		return -EINVAL;

	priv->speed = speed;

	return 0;
}

static int ihs_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct i2c_msg *dmsg, *omsg, dummy;

	memset(&dummy, 0, sizeof(struct i2c_msg));

	/* We expect either two messages (one with an offset and one with the
	 * actual data) or one message (just data)
	 */
	if (nmsgs > 2 || nmsgs == 0) {
		debug("%s: Only one or two messages are supported\n", __func__);
		return -ENOTSUPP;
	}

	omsg = nmsgs == 1 ? &dummy : msg;
	dmsg = nmsgs == 1 ? msg : msg + 1;

	if (dmsg->flags & I2C_M_RD)
		return ihs_i2c_access(bus, dmsg->addr, omsg->buf,
				      omsg->len, dmsg->buf, dmsg->len,
				      I2COP_READ);
	else
		return ihs_i2c_access(bus, dmsg->addr, omsg->buf,
				      omsg->len, dmsg->buf, dmsg->len,
				      I2COP_WRITE);
}

static int ihs_i2c_probe_chip(struct udevice *bus, u32 chip_addr,
			      u32 chip_flags)
{
	uchar buffer[2];
	int res;

	res = ihs_i2c_transfer(bus, chip_addr, buffer, 0, I2COP_READ, true);
	if (res)
		return res;

	return 0;
}

static const struct dm_i2c_ops ihs_i2c_ops = {
	.xfer           = ihs_i2c_xfer,
	.probe_chip     = ihs_i2c_probe_chip,
	.set_bus_speed  = ihs_i2c_set_bus_speed,
};

static const struct udevice_id ihs_i2c_ids[] = {
	{ .compatible = "gdsys,ihs_i2cmaster", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(i2c_ihs) = {
	.name = "i2c_ihs",
	.id = UCLASS_I2C,
	.of_match = ihs_i2c_ids,
	.probe = ihs_i2c_probe,
	.priv_auto	= sizeof(struct ihs_i2c_priv),
	.ops = &ihs_i2c_ops,
};
