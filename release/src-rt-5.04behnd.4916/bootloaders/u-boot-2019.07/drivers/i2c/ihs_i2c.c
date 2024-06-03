// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <i2c.h>
#ifdef CONFIG_DM_I2C
#include <dm.h>
#include <regmap.h>
#else
#include <gdsys_fpga.h>
#endif
#include <asm/unaligned.h>

#ifdef CONFIG_DM_I2C
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

#else /* !CONFIG_DM_I2C */
DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_I2C_IHS_DUAL

#define I2C_SET_REG(fld, val) \
	do { \
		if (I2C_ADAP_HWNR & 0x10) \
			FPGA_SET_REG(I2C_ADAP_HWNR & 0xf, i2c1.fld, val); \
		else \
			FPGA_SET_REG(I2C_ADAP_HWNR, i2c0.fld, val); \
	} while (0)
#else
#define I2C_SET_REG(fld, val) \
		FPGA_SET_REG(I2C_ADAP_HWNR, i2c0.fld, val)
#endif

#ifdef CONFIG_SYS_I2C_IHS_DUAL
#define I2C_GET_REG(fld, val) \
	do {					\
		if (I2C_ADAP_HWNR & 0x10) \
			FPGA_GET_REG(I2C_ADAP_HWNR & 0xf, i2c1.fld, val); \
		else \
			FPGA_GET_REG(I2C_ADAP_HWNR, i2c0.fld, val); \
	} while (0)
#else
#define I2C_GET_REG(fld, val) \
		FPGA_GET_REG(I2C_ADAP_HWNR, i2c0.fld, val)
#endif
#endif /* CONFIG_DM_I2C */

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

#ifdef CONFIG_DM_I2C
static int wait_for_int(struct udevice *dev, int read)
#else
static int wait_for_int(bool read)
#endif
{
	u16 val;
	uint ctr = 0;
#ifdef CONFIG_DM_I2C
	struct ihs_i2c_priv *priv = dev_get_priv(dev);
#endif

#ifdef CONFIG_DM_I2C
	ihs_i2c_get(priv->map, interrupt_status, &val);
#else
	I2C_GET_REG(interrupt_status, &val);
#endif
	/* Wait until error or receive/transmit interrupt was raised */
	while (!(val & (I2CINT_ERROR_EV
	       | (read ? I2CINT_RECEIVE_EV : I2CINT_TRANSMIT_EV)))) {
		udelay(10);
		if (ctr++ > 5000) {
			debug("%s: timed out\n", __func__);
			return -ETIMEDOUT;
		}
#ifdef CONFIG_DM_I2C
		ihs_i2c_get(priv->map, interrupt_status, &val);
#else
		I2C_GET_REG(interrupt_status, &val);
#endif
	}

	return (val & I2CINT_ERROR_EV) ? -EIO : 0;
}

#ifdef CONFIG_DM_I2C
static int ihs_i2c_transfer(struct udevice *dev, uchar chip,
			    uchar *buffer, int len, int read, bool is_last)
#else
static int ihs_i2c_transfer(uchar chip, uchar *buffer, int len, bool read,
			    bool is_last)
#endif
{
	u16 val;
	u16 data;
	int res;
#ifdef CONFIG_DM_I2C
	struct ihs_i2c_priv *priv = dev_get_priv(dev);
#endif

	/* Clear interrupt status */
	data = I2CINT_ERROR_EV | I2CINT_RECEIVE_EV | I2CINT_TRANSMIT_EV;
#ifdef CONFIG_DM_I2C
	ihs_i2c_set(priv->map, interrupt_status, data);
	ihs_i2c_get(priv->map, interrupt_status, &val);
#else
	I2C_SET_REG(interrupt_status, data);
	I2C_GET_REG(interrupt_status, &val);
#endif

	/* If we want to write and have data, write the bytes to the mailbox */
	if (!read && len) {
		val = buffer[0];

		if (len > 1)
			val |= buffer[1] << 8;
#ifdef CONFIG_DM_I2C
		ihs_i2c_set(priv->map, write_mailbox_ext, val);
#else
		I2C_SET_REG(write_mailbox_ext, val);
#endif
	}

	data = I2CMB_NATIVE
	       | (read ? 0 : I2CMB_WRITE)
	       | (chip << 1)
	       | ((len > 1) ? I2CMB_2BYTE : 0)
	       | (is_last ? 0 : I2CMB_HOLD_BUS);

#ifdef CONFIG_DM_I2C
	ihs_i2c_set(priv->map, write_mailbox, data);
#else
	I2C_SET_REG(write_mailbox, data);
#endif

#ifdef CONFIG_DM_I2C
	res = wait_for_int(dev, read);
#else
	res = wait_for_int(read);
#endif
	if (res) {
		if (res == -ETIMEDOUT)
			debug("%s: time out while waiting for event\n", __func__);

		return res;
	}

	/* If we want to read, get the bytes from the mailbox */
	if (read) {
#ifdef CONFIG_DM_I2C
		ihs_i2c_get(priv->map, read_mailbox_ext, &val);
#else
		I2C_GET_REG(read_mailbox_ext, &val);
#endif
		buffer[0] = val & 0xff;
		if (len > 1)
			buffer[1] = val >> 8;
	}

	return 0;
}

#ifdef CONFIG_DM_I2C
static int ihs_i2c_send_buffer(struct udevice *dev, uchar chip, u8 *data, int len, bool hold_bus, int read)
#else
static int ihs_i2c_send_buffer(uchar chip, u8 *data, int len, bool hold_bus,
			       int read)
#endif
{
	int res;

	while (len) {
		int transfer = min(len, 2);
		bool is_last = len <= transfer;

#ifdef CONFIG_DM_I2C
		res = ihs_i2c_transfer(dev, chip, data, transfer, read,
				       hold_bus ? false : is_last);
#else
		res = ihs_i2c_transfer(chip, data, transfer, read,
				       hold_bus ? false : is_last);
#endif
		if (res)
			return res;

		data += transfer;
		len -= transfer;
	}

	return 0;
}

#ifdef CONFIG_DM_I2C
static int ihs_i2c_address(struct udevice *dev, uchar chip, u8 *addr, int alen,
			   bool hold_bus)
#else
static int ihs_i2c_address(uchar chip, u8 *addr, int alen, bool hold_bus)
#endif
{
#ifdef CONFIG_DM_I2C
	return ihs_i2c_send_buffer(dev, chip, addr, alen, hold_bus, I2COP_WRITE);
#else
	return ihs_i2c_send_buffer(chip, addr, alen, hold_bus, I2COP_WRITE);
#endif
}

#ifdef CONFIG_DM_I2C
static int ihs_i2c_access(struct udevice *dev, uchar chip, u8 *addr,
			  int alen, uchar *buffer, int len, int read)
#else
static int ihs_i2c_access(struct i2c_adapter *adap, uchar chip, u8 *addr,
			  int alen, uchar *buffer, int len, int read)
#endif
{
	int res;

	/* Don't hold the bus if length of data to send/receive is zero */
	if (len <= 0)
		return -EINVAL;

#ifdef CONFIG_DM_I2C
	res = ihs_i2c_address(dev, chip, addr, alen, len);
#else
	res = ihs_i2c_address(chip, addr, alen, len);
#endif
	if (res)
		return res;

#ifdef CONFIG_DM_I2C
	return ihs_i2c_send_buffer(dev, chip, buffer, len, false, read);
#else
	return ihs_i2c_send_buffer(chip, buffer, len, false, read);
#endif
}

#ifdef CONFIG_DM_I2C

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
	 * actucal data) or one message (just data)
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
	.priv_auto_alloc_size = sizeof(struct ihs_i2c_priv),
	.ops = &ihs_i2c_ops,
};

#else /* CONFIG_DM_I2C */

static void ihs_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/*
	 * Call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	 */
	i2c_init_board();
#endif
}

static int ihs_i2c_probe(struct i2c_adapter *adap, uchar chip)
{
	uchar buffer[2];
	int res;

	res = ihs_i2c_transfer(chip, buffer, 0, I2COP_READ, true);
	if (res)
		return res;

	return 0;
}

static int ihs_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			int alen, uchar *buffer, int len)
{
	u8 addr_bytes[4];

	put_unaligned_le32(addr, addr_bytes);

	return ihs_i2c_access(adap, chip, addr_bytes, alen, buffer, len,
			      I2COP_READ);
}

static int ihs_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			 int alen, uchar *buffer, int len)
{
	u8 addr_bytes[4];

	put_unaligned_le32(addr, addr_bytes);

	return ihs_i2c_access(adap, chip, addr_bytes, alen, buffer, len,
			      I2COP_WRITE);
}

static unsigned int ihs_i2c_set_bus_speed(struct i2c_adapter *adap,
					  unsigned int speed)
{
	if (speed != adap->speed)
		return -EINVAL;
	return speed;
}

/*
 * Register IHS i2c adapters
 */
#ifdef CONFIG_SYS_I2C_IHS_CH0
U_BOOT_I2C_ADAP_COMPLETE(ihs0, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_0,
			 CONFIG_SYS_I2C_IHS_SLAVE_0, 0)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs0_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_0_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_0_1, 16)
#endif
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH1
U_BOOT_I2C_ADAP_COMPLETE(ihs1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_1, 1)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs1_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_1_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_1_1, 17)
#endif
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH2
U_BOOT_I2C_ADAP_COMPLETE(ihs2, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_2,
			 CONFIG_SYS_I2C_IHS_SLAVE_2, 2)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs2_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_2_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_2_1, 18)
#endif
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH3
U_BOOT_I2C_ADAP_COMPLETE(ihs3, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_3,
			 CONFIG_SYS_I2C_IHS_SLAVE_3, 3)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs3_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_3_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_3_1, 19)
#endif
#endif
#endif /* CONFIG_DM_I2C */
