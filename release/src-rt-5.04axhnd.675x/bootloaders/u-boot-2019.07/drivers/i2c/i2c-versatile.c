// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Arm Ltd.
 * Author: Liviu Dudau <liviu.dudau@foss.arm.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <asm/io.h>
#include <clk.h>
#include <linux/io.h>

#define I2C_CONTROL_REG		0x00
#define I2C_SET_REG		0x00
#define I2C_CLEAR_REG		0x04

#define SCL	BIT(0)
#define SDA	BIT(1)

struct versatile_i2c_priv {
	phys_addr_t base;
	u32 delay;
};

static inline void versatile_sda_set(struct versatile_i2c_priv *priv, u8 state)
{
	writel(SDA, priv->base + (state ? I2C_SET_REG : I2C_CLEAR_REG));
	udelay(priv->delay);
}

static inline int versatile_sda_get(struct versatile_i2c_priv *priv)
{
	int v = !!(readl(priv->base + I2C_CONTROL_REG) & SDA);

	udelay(priv->delay);
	return v;
}

static inline void versatile_scl_set(struct versatile_i2c_priv *priv, u8 state)
{
	writel(SCL, priv->base + (state ? I2C_SET_REG : I2C_CLEAR_REG));
	udelay(priv->delay);
}

static inline int versatile_scl_get(struct versatile_i2c_priv *priv)
{
	int v = !!(readl(priv->base + I2C_CONTROL_REG) & SCL);

	udelay(priv->delay);
	return v;
}

/* start: SDA goes from high to low while SCL is high */
static void versatile_i2c_start(struct versatile_i2c_priv *priv)
{
	udelay(priv->delay);
	versatile_sda_set(priv, 1);
	versatile_scl_set(priv, 1);
	versatile_sda_set(priv, 0);
}

/* stop: SDA goes from low to high while SCL is high */
static void versatile_i2c_stop(struct versatile_i2c_priv *priv)
{
	versatile_scl_set(priv, 0);
	versatile_sda_set(priv, 0);
	versatile_scl_set(priv, 1);
	versatile_sda_set(priv, 1);
}

/* read a bit from the SDA line (data or ACK/NACK) */
static u8 versatile_i2c_read_bit(struct versatile_i2c_priv *priv)
{
	versatile_scl_set(priv, 0);
	versatile_sda_set(priv, 1);
	versatile_scl_set(priv, 1);
	udelay(priv->delay);
	return (u8)versatile_sda_get(priv);
}

/* write a bit on the SDA line */
static void versatile_i2c_write_bit(struct versatile_i2c_priv *priv, u8 bit)
{
	versatile_scl_set(priv, 0);
	versatile_sda_set(priv, bit);
	versatile_scl_set(priv, 1);
	udelay(priv->delay);
}

/* send a reset sequence of 9 clocks with SDA high */
static void versatile_i2c_reset_bus(struct versatile_i2c_priv *priv)
{
	int i;

	for (i = 0; i < 9; i++)
		versatile_i2c_write_bit(priv, 1);

	versatile_i2c_stop(priv);
}

/* write byte without start/stop sequence */
static int versatile_i2c_write_byte(struct versatile_i2c_priv *priv, u8 byte)
{
	u8 nak, i;

	for (i = 0; i < 8; i++) {
		versatile_i2c_write_bit(priv, byte & 0x80);
		byte <<= 1;
	}

	/* read ACK */
	nak = versatile_i2c_read_bit(priv);
	versatile_scl_set(priv, 0);

	return nak;	/* not a nack is an ack */
}

static int versatile_i2c_read_byte(struct versatile_i2c_priv *priv,
				   u8 *byte, u8 ack)
{
	u8 i;

	*byte = 0;
	for (i = 0; i < 8; i++) {
		*byte <<= 1;
		*byte |= versatile_i2c_read_bit(priv);
	}
	/* write the nack */
	versatile_i2c_write_bit(priv, ack);

	return 0;
}

static int versatile_i2c_send_slave_addr(struct versatile_i2c_priv *priv,
					 struct i2c_msg *msg)
{
	u8 addr;
	int ret;

	if (msg->flags & I2C_M_TEN) {
		/* 10-bit address, send extended address code first */
		addr = 0xf0 | ((msg->addr >> 7) & 0x06);
		ret = versatile_i2c_write_byte(priv, addr);
		if (ret) {
			versatile_i2c_stop(priv);
			return -EIO;
		}

		/* remaining bits */
		ret = versatile_i2c_write_byte(priv, msg->addr & 0xff);
		if (ret) {
			versatile_i2c_stop(priv);
			return -EIO;
		}
		/* reads need to resend the addr */
		if (msg->flags & I2C_M_RD) {
			versatile_i2c_start(priv);
			addr |= 1;
			ret = versatile_i2c_write_byte(priv, addr);
			if (ret) {
				versatile_i2c_stop(priv);
				return -EIO;
			}
		}
	} else {
		/* normal 7-bit address */
		addr = msg->addr << 1;
		if (msg->flags & I2C_M_RD)
			addr |= 1;
		ret = versatile_i2c_write_byte(priv, addr);
		if (ret) {
			versatile_i2c_stop(priv);
			return -EIO;
		}
	}

	return 0;
}

static int versatile_i2c_message_xfer(struct versatile_i2c_priv *priv,
				      struct i2c_msg *msg)
{
	int i, ret;
	u8 ack;

	versatile_i2c_start(priv);
	if (versatile_i2c_send_slave_addr(priv, msg))
		return -EIO;

	for (i = 0; i < msg->len; i++) {
		if (msg->flags & I2C_M_RD) {
			ack = (msg->len - i - 1) == 0 ? 1 : 0;
			ret = versatile_i2c_read_byte(priv, &msg->buf[i], ack);
		} else {
			ret = versatile_i2c_write_byte(priv, msg->buf[i]);
		}

		if (ret)
			break;
	}

	versatile_i2c_stop(priv);

	return ret;
}

static int versatile_i2c_xfer(struct udevice *bus,
			      struct i2c_msg *msg, int nmsgs)
{
	struct versatile_i2c_priv *priv = dev_get_priv(bus);
	int ret;

	for ( ; nmsgs > 0; nmsgs--, msg++) {
		ret = versatile_i2c_message_xfer(priv, msg);
		if (ret)
			return -EREMOTEIO;
	}

	return 0;
}

static int versatile_i2c_chip_probe(struct udevice *bus,
				    uint chip, uint chip_flags)
{
	/* probe the presence of a slave by writing a 0-size message */
	struct i2c_msg msg = { .addr = chip, .flags = chip_flags,
			       .len = 0, .buf = NULL };
	struct versatile_i2c_priv *priv = dev_get_priv(bus);

	return versatile_i2c_message_xfer(priv, &msg);
}

static int versatile_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct versatile_i2c_priv *priv = dev_get_priv(bus);

	priv->delay = 1000000 / (speed << 2);

	versatile_i2c_reset_bus(priv);

	return 0;
}

static int versatile_i2c_probe(struct udevice *dev)
{
	struct versatile_i2c_priv *priv = dev_get_priv(dev);

	priv->base = (phys_addr_t)dev_read_addr(dev);
	priv->delay = 25;	/* 25us * 4 = 100kHz */
	/*
	 * U-Boot still doesn't assign automatically
	 * sequence numbers to devices
	 */
	dev->req_seq = 1;

	return 0;
}

static const struct dm_i2c_ops versatile_i2c_ops = {
	.xfer = versatile_i2c_xfer,
	.probe_chip = versatile_i2c_chip_probe,
	.set_bus_speed = versatile_i2c_set_bus_speed,
};

static const struct udevice_id versatile_i2c_of_match[] = {
	{ .compatible = "arm,versatile-i2c" },
	{ }
};

U_BOOT_DRIVER(versatile_i2c) = {
	.name = "i2c-bus-versatile",
	.id = UCLASS_I2C,
	.of_match = versatile_i2c_of_match,
	.probe = versatile_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct versatile_i2c_priv),
	.ops = &versatile_i2c_ops,
};
