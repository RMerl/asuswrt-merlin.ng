// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip I2C controller driver
 *
 * Copyright (C) 2021-2022 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 * Conor Dooley <conor.dooley@microchip.com>
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

#define	MICROCHIP_I2C_TIMEOUT	(1000 * 60)

#define MPFS_I2C_CTRL	(0x00)
#define	CTRL_CR0		(0x00)
#define	CTRL_CR1		(0x01)
#define	CTRL_AA			BIT(2)
#define	CTRL_SI			BIT(3)
#define	CTRL_STO		BIT(4)
#define	CTRL_STA		BIT(5)
#define	CTRL_ENS1		BIT(6)
#define	CTRL_CR2		(0x07)
#define MPFS_I2C_STATUS							(0x04)
#define	STATUS_BUS_ERROR						(0x00)
#define	STATUS_M_START_SENT						(0x08)
#define	STATUS_M_REPEATED_START_SENT			(0x10)
#define	STATUS_M_SLAW_ACK						(0x18)
#define	STATUS_M_SLAW_NACK						(0x20)
#define	STATUS_M_TX_DATA_ACK					(0x28)
#define	STATUS_M_TX_DATA_NACK					(0x30)
#define	STATUS_M_ARB_LOST						(0x38)
#define	STATUS_M_SLAR_ACK						(0x40)
#define	STATUS_M_SLAR_NACK						(0x48)
#define	STATUS_M_RX_DATA_ACKED					(0x50)
#define	STATUS_M_RX_DATA_NACKED					(0x58)
#define	STATUS_S_SLAW_ACKED						(0x60)
#define	STATUS_S_ARB_LOST_SLAW_ACKED			(0x68)
#define	STATUS_S_GENERAL_CALL_ACKED				(0x70)
#define	STATUS_S_ARB_LOST_GENERAL_CALL_ACKED	(0x78)
#define	STATUS_S_RX_DATA_ACKED					(0x80)
#define	STATUS_S_RX_DATA_NACKED					(0x88)
#define	STATUS_S_GENERAL_CALL_RX_DATA_ACKED		(0x90)
#define	STATUS_S_GENERAL_CALL_RX_DATA_NACKED	(0x98)
#define	STATUS_S_RX_STOP						(0xA0)
#define	STATUS_S_SLAR_ACKED						(0xA8)
#define	STATUS_S_ARB_LOST_SLAR_ACKED			(0xB0)
#define	STATUS_S_TX_DATA_ACK					(0xb8)
#define	STATUS_S_TX_DATA_NACK					(0xC0)
#define	STATUS_LAST_DATA_ACK					(0xC8)
#define	STATUS_M_SMB_MASTER_RESET				(0xD0)
#define	STATUS_S_SCL_LOW_TIMEOUT				(0xD8)
#define	STATUS_NO_STATE_INFO					(0xF8)
#define MPFS_I2C_DATA			(0x08)
#define MPFS_I2C_SLAVE0_ADDR	(0x0c)
#define MPFS_I2C_SMBUS			(0x10)
#define MPFS_I2C_FREQ			(0x14)
#define MPFS_I2C_GLITCHREG		(0x18)
#define MPFS_I2C_SLAVE1_ADDR	(0x1c)

#define PCLK_DIV_256	((0 << CTRL_CR0) | (0 << CTRL_CR1) | (0 << CTRL_CR2))
#define PCLK_DIV_224	((1 << CTRL_CR0) | (0 << CTRL_CR1) | (0 << CTRL_CR2))
#define PCLK_DIV_192	((0 << CTRL_CR0) | (1 << CTRL_CR1) | (0 << CTRL_CR2))
#define PCLK_DIV_160	((1 << CTRL_CR0) | (1 << CTRL_CR1) | (0 << CTRL_CR2))
#define PCLK_DIV_960	((0 << CTRL_CR0) | (0 << CTRL_CR1) | (1 << CTRL_CR2))
#define PCLK_DIV_120	((1 << CTRL_CR0) | (0 << CTRL_CR1) | (1 << CTRL_CR2))
#define PCLK_DIV_60		((0 << CTRL_CR0) | (1 << CTRL_CR1) | (1 << CTRL_CR2))
#define BCLK_DIV_8		((1 << CTRL_CR0) | (1 << CTRL_CR1) | (1 << CTRL_CR2))
#define CLK_MASK		((1 << CTRL_CR0) | (1 << CTRL_CR1) | (1 << CTRL_CR2))

/*
 * mpfs_i2c_bus - I2C bus context
 * @base: pointer to register struct
 * @msg_len: number of bytes transferred in msg
 * @msg_err: error code for completed message
 * @i2c_clk: clock reference for i2c input clock
 * @clk_rate: current i2c bus clock rate
 * @buf: ptr to msg buffer for easier use.
 * @addr: i2c address.
 * @isr_status: cached copy of local ISR status.
 */
struct mpfs_i2c_bus {
	void __iomem *base;
	size_t msg_len;
	int msg_err;
	struct clk i2c_clk;
	u32 clk_rate;
	u8 *buf;
	u8 addr;
	u32 isr_status;
};

static inline u8 i2c_8bit_addr_from_msg(const struct i2c_msg *msg)
{
	return (msg->addr << 1) | (msg->flags & I2C_M_RD ? 1 : 0);
}

static void mpfs_i2c_int_clear(struct mpfs_i2c_bus *bus)
{
	u8 ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl &= ~CTRL_SI;
	writel(ctrl, bus->base + MPFS_I2C_CTRL);
}

static void mpfs_i2c_core_disable(struct mpfs_i2c_bus *bus)
{
	u8 ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl &= ~CTRL_ENS1;
	writel(ctrl, bus->base + MPFS_I2C_CTRL);
}

static void mpfs_i2c_core_enable(struct mpfs_i2c_bus *bus)
{
	u8 ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl |= CTRL_ENS1;
	writel(ctrl, bus->base + MPFS_I2C_CTRL);
}

static void mpfs_i2c_reset(struct mpfs_i2c_bus *bus)
{
	mpfs_i2c_core_disable(bus);
	mpfs_i2c_core_enable(bus);
}

static inline void mpfs_i2c_stop(struct mpfs_i2c_bus *bus)
{
	u8 ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl |= CTRL_STO;
	writel(ctrl, bus->base + MPFS_I2C_CTRL);
}

static inline int mpfs_generate_divisor(u32 rate, u8 *code)
{
	int ret = 0;

	if (rate >= 960)
		*code = PCLK_DIV_960;
	else if (rate >= 256)
		*code = PCLK_DIV_256;
	else if (rate >= 224)
		*code = PCLK_DIV_224;
	else if (rate >= 192)
		*code = PCLK_DIV_192;
	else if (rate >= 160)
		*code = PCLK_DIV_160;
	else if (rate >= 120)
		*code = PCLK_DIV_120;
	else if (rate >= 60)
		*code = PCLK_DIV_60;
	else if (rate >= 8)
		*code = BCLK_DIV_8;
	else
		ret = -EINVAL;

	return ret;
}

static int mpfs_i2c_init(struct mpfs_i2c_bus *bus, struct udevice *dev)
{
	u32 clk_rate, divisor;
	u8 clkval, ctrl;
	int ret;

	ret = clk_get_by_index(dev, 0, &bus->i2c_clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&bus->i2c_clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&bus->i2c_clk);
	if (!clk_rate)
		return -EINVAL;

	divisor = clk_rate / bus->clk_rate;

	ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl &= ~CLK_MASK;

	ret = mpfs_generate_divisor(divisor, &clkval);
	if (ret)
		return -EINVAL;

	ctrl |= clkval;

	writel(ctrl, bus->base + MPFS_I2C_CTRL);

	ctrl = readl(bus->base + MPFS_I2C_CTRL);

	/* Reset I2C core */
	mpfs_i2c_reset(bus);

	return 0;
}

static void mpfs_i2c_transfer(struct mpfs_i2c_bus *bus, u32 data)
{
	if (bus->msg_len > 0)
		writel(data, bus->base + MPFS_I2C_DATA);
}

static void mpfs_i2c_empty_rx(struct mpfs_i2c_bus *bus)
{
	u8 ctrl;
	u8 data_read;

	if (bus->msg_len > 0) {
		data_read = readl(bus->base + MPFS_I2C_DATA);
		*bus->buf++ = data_read;
		bus->msg_len--;
	}

	if (bus->msg_len <= 1) {
		ctrl = readl(bus->base + MPFS_I2C_CTRL);
		ctrl &= ~CTRL_AA;
		writel(ctrl, bus->base + MPFS_I2C_CTRL);
	}
}

static int mpfs_i2c_fill_tx(struct mpfs_i2c_bus *bus)
{
	mpfs_i2c_transfer(bus, *bus->buf++);
	bus->msg_len--;

	return 0;
}

static int mpfs_i2c_service_handler(struct mpfs_i2c_bus *bus)
{
	bool finish = false;
	u32 status;
	u8 ctrl;

	status = bus->isr_status;

	switch (status)	{
	case STATUS_M_START_SENT:
	case STATUS_M_REPEATED_START_SENT:
		ctrl = readl(bus->base + MPFS_I2C_CTRL);
		ctrl &= ~CTRL_STA;
		writel(bus->addr, bus->base + MPFS_I2C_DATA);
		writel(ctrl, bus->base + MPFS_I2C_CTRL);
		break;
	case STATUS_M_SLAW_ACK:
	case STATUS_M_TX_DATA_ACK:
		if (bus->msg_len > 0) {
			mpfs_i2c_fill_tx(bus);
		} else {
			/* On the last byte to be transmitted, send STOP */
			mpfs_i2c_stop(bus);
			finish = true;
		}
		break;
	case STATUS_M_SLAR_ACK:
		if (bus->msg_len > 1u) {
			ctrl = readl(bus->base + MPFS_I2C_CTRL);
			ctrl |= CTRL_AA;
			writel(ctrl, bus->base + MPFS_I2C_CTRL);
		} else if (bus->msg_len == 1u) {
			ctrl = readl(bus->base + MPFS_I2C_CTRL);
			ctrl &= ~CTRL_AA;
			writel(ctrl, bus->base + MPFS_I2C_CTRL);
		} else {
			ctrl = readl(bus->base + MPFS_I2C_CTRL);
			ctrl |= CTRL_AA;
			writel(ctrl, bus->base + MPFS_I2C_CTRL);
			/* On the last byte to be transmitted, send STOP */
			mpfs_i2c_stop(bus);
			finish = true;
		}
		break;
	case STATUS_M_RX_DATA_ACKED:
		mpfs_i2c_empty_rx(bus);
		break;
	case STATUS_M_RX_DATA_NACKED:
		mpfs_i2c_empty_rx(bus);
		if (bus->msg_len == 0) {
			/* On the last byte to be transmitted, send STOP */
			mpfs_i2c_stop(bus);
			finish = true;
		}
		break;
	case STATUS_M_TX_DATA_NACK:
	case STATUS_M_SLAR_NACK:
	case STATUS_M_SLAW_NACK:
		bus->msg_err = -ENXIO;
		mpfs_i2c_stop(bus);
		finish = true;
		break;

	case STATUS_M_ARB_LOST:
		/* Handle Lost Arbitration */
		bus->msg_err = -EAGAIN;
		finish = true;
		break;
	default:
		break;
	}

	if (finish) {
		ctrl = readl(bus->base + MPFS_I2C_CTRL);
		ctrl &= ~CTRL_AA;
		writel(ctrl, bus->base + MPFS_I2C_CTRL);
		return 0;
	}

	return 1;
}

static int mpfs_i2c_service(struct mpfs_i2c_bus *bus)
{
	int ret = 0;
	int si_bit;

	si_bit = readl(bus->base + MPFS_I2C_CTRL);
	if (si_bit & CTRL_SI) {
		bus->isr_status = readl(bus->base + MPFS_I2C_STATUS);
		ret = mpfs_i2c_service_handler(bus);
	}
	/* Clear the si flag */
	mpfs_i2c_int_clear(bus);
	si_bit = readl(bus->base + MPFS_I2C_CTRL);

	return ret;
}

static int mpfs_i2c_check_service_change(struct mpfs_i2c_bus *bus)
{
	u8 ctrl;
	u32 count = 0;

	while (1) {
		ctrl = readl(bus->base + MPFS_I2C_CTRL);
		if (ctrl & CTRL_SI)
			break;
		udelay(1);
		count += 1;
		if (count == MICROCHIP_I2C_TIMEOUT)
			return -ETIMEDOUT;
	}
	return 0;
}

static int mpfs_i2c_poll_device(struct mpfs_i2c_bus *bus)
{
	int ret;

	while (1) {
		ret = mpfs_i2c_check_service_change(bus);
		if (ret)
			return ret;

		ret = mpfs_i2c_service(bus);
		if (!ret)
			/* all messages have been transferred */
			return ret;
	}
}

static int mpfs_i2c_xfer_msg(struct mpfs_i2c_bus *bus, struct i2c_msg *msg)
{
	u8 ctrl;
	int ret;

	if (!msg->len || !msg->buf)
		return -EINVAL;

	bus->addr = i2c_8bit_addr_from_msg(msg);
	bus->msg_len = msg->len;
	bus->buf = msg->buf;
	bus->msg_err = 0;

	mpfs_i2c_core_enable(bus);

	ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl |= CTRL_STA;

	writel(ctrl, bus->base + MPFS_I2C_CTRL);

	ret = mpfs_i2c_poll_device(bus);
	if (ret)
		return ret;

	return bus->msg_err;
}

static int mpfs_i2c_xfer(struct udevice *dev, struct i2c_msg *msgs, int num_msgs)
{
	struct mpfs_i2c_bus *bus = dev_get_priv(dev);
	int idx, ret;

	if (!msgs || !num_msgs)
		return -EINVAL;

	for (idx = 0; idx < num_msgs; idx++) {
		ret = mpfs_i2c_xfer_msg(bus, msgs++);
		if (ret)
			return ret;
	}

	return ret;
}

static int mpfs_i2c_probe_chip(struct udevice *dev, uint addr, uint flags)
{
	struct mpfs_i2c_bus *bus = dev_get_priv(dev);
	int ret;
	u8 ctrl, reg = 0;

	/*
	 * Send the chip address and verify that the
	 * address was <ACK>ed.
	 */
	bus->addr = addr << 1 | I2C_M_RD;
	bus->buf = &reg;
	bus->msg_len = 0;
	bus->msg_err = 0;

	mpfs_i2c_core_enable(bus);

	ctrl = readl(bus->base + MPFS_I2C_CTRL);

	ctrl |= CTRL_STA;

	writel(ctrl, bus->base + MPFS_I2C_CTRL);

	ret = mpfs_i2c_poll_device(bus);
	if (ret)
		return ret;

	return bus->msg_err;
}

static int mpfs_i2c_probe(struct udevice *dev)
{
	int ret;
	u32 val;
	struct mpfs_i2c_bus *bus = dev_get_priv(dev);

	bus->base = dev_read_addr_ptr(dev);
	if (!bus->base)
		return -EINVAL;

	val = dev_read_u32(dev, "clock-frequency", &bus->clk_rate);
	if (val) {
		printf("Default to 100kHz\n");
		/* default clock rate */
		bus->clk_rate = 100000;
	}

	if (bus->clk_rate > 400000 || bus->clk_rate <= 0) {
		printf("Invalid clock-frequency %d\n", bus->clk_rate);
		return -EINVAL;
	}

	ret = mpfs_i2c_init(bus, dev);

	return ret;
}

static const struct dm_i2c_ops mpfs_i2c_ops = {
	.xfer = mpfs_i2c_xfer,
	.probe_chip = mpfs_i2c_probe_chip,
};

static const struct udevice_id mpfs_i2c_ids[] = {
	{.compatible = "microchip,mpfs-i2c"},
	{}
};

U_BOOT_DRIVER(mpfs_i2c) = {
	.name = "mpfs_i2c",
	.id = UCLASS_I2C,
	.of_match = mpfs_i2c_ids,
	.ops = &mpfs_i2c_ops,
	.probe = mpfs_i2c_probe,
	.priv_auto = sizeof(struct mpfs_i2c_bus),
};
