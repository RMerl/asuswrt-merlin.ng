// SPDX-License-Identifier: GPL-2.0+
/*
 * ocores-i2c.c: I2C bus driver for OpenCores I2C controller
 * (https://opencores.org/projects/i2c)
 *
 * (C) Copyright Peter Korsgaard <peter@korsgaard.com>
 *
 * Copyright (C) 2020 SiFive, Inc.
 * Pragnesh Patel <pragnesh.patel@sifive.com>
 *
 * Support for the GRLIB port of the controller by
 * Andreas Larsson <andreas@gaisler.com>
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <i2c.h>
#include <linux/io.h>
#include <linux/compat.h>
#include <linux/log2.h>
#include <linux/delay.h>

/* registers */
#define OCI2C_PRELOW		0
#define OCI2C_PREHIGH		1
#define OCI2C_CONTROL		2
#define OCI2C_DATA		3
#define OCI2C_CMD		4 /* write only */
#define OCI2C_STATUS		4 /* read only, same address as OCI2C_CMD */

#define OCI2C_CTRL_IEN		0x40
#define OCI2C_CTRL_EN		0x80

#define OCI2C_CMD_START		0x91
#define OCI2C_CMD_STOP		0x41
#define OCI2C_CMD_READ		0x21
#define OCI2C_CMD_WRITE		0x11
#define OCI2C_CMD_READ_ACK	0x21
#define OCI2C_CMD_READ_NACK	0x29
#define OCI2C_CMD_IACK		0x01

#define OCI2C_STAT_IF		0x01
#define OCI2C_STAT_TIP		0x02
#define OCI2C_STAT_ARBLOST	0x20
#define OCI2C_STAT_BUSY		0x40
#define OCI2C_STAT_NACK		0x80

#define STATE_DONE		0
#define STATE_START		1
#define STATE_WRITE		2
#define STATE_READ		3
#define STATE_ERROR		4

#define TYPE_OCORES		0
#define TYPE_GRLIB		1

#define OCORES_FLAG_BROKEN_IRQ BIT(1) /* Broken IRQ for FU540-C000 SoC */

struct ocores_i2c_bus {
	void __iomem *base;
	u32 reg_shift;
	u32 reg_io_width;
	unsigned long flags;
	struct i2c_msg *msg;
	int pos;
	int nmsgs;
	int state; /* see STATE_ */
	struct clk clk;
	int ip_clk_khz;
	int bus_clk_khz;
	void (*setreg)(struct ocores_i2c_bus *i2c, int reg, u8 value);
	u8 (*getreg)(struct ocores_i2c_bus *i2c, int reg);
};

DECLARE_GLOBAL_DATA_PTR;

/* Boolean attribute values */
enum {
	FALSE = 0,
	TRUE,
};

static void oc_setreg_8(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	writeb(value, i2c->base + (reg << i2c->reg_shift));
}

static void oc_setreg_16(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	writew(value, i2c->base + (reg << i2c->reg_shift));
}

static void oc_setreg_32(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	writel(value, i2c->base + (reg << i2c->reg_shift));
}

static void oc_setreg_16be(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	out_be16(i2c->base + (reg << i2c->reg_shift), value);
}

static void oc_setreg_32be(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	out_be32(i2c->base + (reg << i2c->reg_shift), value);
}

static inline u8 oc_getreg_8(struct ocores_i2c_bus *i2c, int reg)
{
	return readb(i2c->base + (reg << i2c->reg_shift));
}

static inline u8 oc_getreg_16(struct ocores_i2c_bus *i2c, int reg)
{
	return readw(i2c->base + (reg << i2c->reg_shift));
}

static inline u8 oc_getreg_32(struct ocores_i2c_bus *i2c, int reg)
{
	return readl(i2c->base + (reg << i2c->reg_shift));
}

static inline u8 oc_getreg_16be(struct ocores_i2c_bus *i2c, int reg)
{
	return in_be16(i2c->base + (reg << i2c->reg_shift));
}

static inline u8 oc_getreg_32be(struct ocores_i2c_bus *i2c, int reg)
{
	return in_be32(i2c->base + (reg << i2c->reg_shift));
}

static inline void oc_setreg(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	i2c->setreg(i2c, reg, value);
}

static inline u8 oc_getreg(struct ocores_i2c_bus *i2c, int reg)
{
	return i2c->getreg(i2c, reg);
}

static inline u8 i2c_8bit_addr_from_msg(const struct i2c_msg *msg)
{
	return (msg->addr << 1) | (msg->flags & I2C_M_RD ? 1 : 0);
}

static void ocores_process(struct ocores_i2c_bus *i2c, u8 stat)
{
	struct i2c_msg *msg = i2c->msg;

	if (i2c->state == STATE_DONE || i2c->state == STATE_ERROR) {
		/* stop has been sent */
		oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_IACK);
		return;
	}

	/* error? */
	if (stat & OCI2C_STAT_ARBLOST) {
		i2c->state = STATE_ERROR;
		oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_STOP);
		return;
	}

	if (i2c->state == STATE_START || i2c->state == STATE_WRITE) {
		i2c->state =
			(msg->flags & I2C_M_RD) ? STATE_READ : STATE_WRITE;

		if (stat & OCI2C_STAT_NACK) {
			i2c->state = STATE_ERROR;
			oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_STOP);
			return;
		}
	} else {
		msg->buf[i2c->pos++] = oc_getreg(i2c, OCI2C_DATA);
	}

	/* end of msg? */
	if (i2c->pos == msg->len) {
		i2c->nmsgs--;
		i2c->msg++;
		i2c->pos = 0;
		msg = i2c->msg;

		if (i2c->nmsgs) {       /* end? */
			/* send start? */
			if (!(msg->flags & I2C_M_NOSTART)) {
				u8 addr = i2c_8bit_addr_from_msg(msg);

				i2c->state = STATE_START;

				oc_setreg(i2c, OCI2C_DATA, addr);
				oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_START);
				return;
			}
			i2c->state = (msg->flags & I2C_M_RD)
				? STATE_READ : STATE_WRITE;
		} else {
			i2c->state = STATE_DONE;
			oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_STOP);
			return;
		}
	}

	if (i2c->state == STATE_READ) {
		oc_setreg(i2c, OCI2C_CMD, i2c->pos == (msg->len - 1) ?
				OCI2C_CMD_READ_NACK : OCI2C_CMD_READ_ACK);
	} else {
		oc_setreg(i2c, OCI2C_DATA, msg->buf[i2c->pos++]);
		oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_WRITE);
	}
}

static irqreturn_t ocores_isr(int irq, void *dev_id)
{
	struct ocores_i2c_bus *i2c = dev_id;
	u8 stat = oc_getreg(i2c, OCI2C_STATUS);

	if (i2c->flags & OCORES_FLAG_BROKEN_IRQ) {
		if ((stat & OCI2C_STAT_IF) && !(stat & OCI2C_STAT_BUSY))
			return IRQ_NONE;
	} else if (!(stat & OCI2C_STAT_IF)) {
		return IRQ_NONE;
	}
	ocores_process(i2c, stat);

	return IRQ_HANDLED;
}

/**
 * Wait until something change in a given register
 * @i2c: ocores I2C device instance
 * @reg: register to query
 * @mask: bitmask to apply on register value
 * @val: expected result
 * @msec: timeout in msec
 *
 * Timeout is necessary to avoid to stay here forever when the chip
 * does not answer correctly.
 *
 * Return: 0 on success, -ETIMEDOUT on timeout
 */
static int ocores_wait(struct ocores_i2c_bus *i2c,
		       int reg, u8 mask, u8 val,
		       const unsigned long msec)
{
	u32 count = 0;

	while (1) {
		u8 status = oc_getreg(i2c, reg);

		if ((status & mask) == val)
			break;

		udelay(1);
		count += 1;

		if (count == (1000 * msec))
			return -ETIMEDOUT;
	}
	return 0;
}

/**
 * Wait until is possible to process some data
 * @i2c: ocores I2C device instance
 *
 * Used when the device is in polling mode (interrupts disabled).
 *
 * Return: 0 on success, -ETIMEDOUT on timeout
 */
static int ocores_poll_wait(struct ocores_i2c_bus *i2c)
{
	u8 mask;
	int err;

	if (i2c->state == STATE_DONE || i2c->state == STATE_ERROR) {
		/* transfer is over */
		mask = OCI2C_STAT_BUSY;
	} else {
		/* on going transfer */
		mask = OCI2C_STAT_TIP;
		/*
		 * We wait for the data to be transferred (8bit),
		 * then we start polling on the ACK/NACK bit
		 */
		udelay((8 * 1000) / i2c->bus_clk_khz);
	}

	/*
	 * once we are here we expect to get the expected result immediately
	 * so if after 1ms we timeout then something is broken.
	 */
	err = ocores_wait(i2c, OCI2C_STATUS, mask, 0, 1);
	if (err)
		debug("%s: STATUS timeout, bit 0x%x did not clear in 1ms\n",
		      __func__, mask);
	return err;
}

/**
 * It handles an IRQ-less transfer
 * @i2c: ocores I2C device instance
 *
 * Even if IRQ are disabled, the I2C OpenCore IP behavior is exactly the same
 * (only that IRQ are not produced). This means that we can re-use entirely
 * ocores_isr(), we just add our polling code around it.
 *
 * It can run in atomic context
 */
static void ocores_process_polling(struct ocores_i2c_bus *i2c)
{
	while (1) {
		irqreturn_t ret;
		int err;

		err = ocores_poll_wait(i2c);
		if (err) {
			i2c->state = STATE_ERROR;
			break; /* timeout */
		}

		ret = ocores_isr(-1, i2c);
		if (ret == IRQ_NONE) {
			break; /* all messages have been transferred */
		} else {
			if (i2c->flags & OCORES_FLAG_BROKEN_IRQ)
				if (i2c->state == STATE_DONE)
					break;
		}
	}
}

static int ocores_xfer_core(struct ocores_i2c_bus *i2c,
			    struct i2c_msg *msgs, int num, bool polling)
{
	u8 ctrl;

	ctrl = oc_getreg(i2c, OCI2C_CONTROL);

	if (polling)
		oc_setreg(i2c, OCI2C_CONTROL, ctrl & ~OCI2C_CTRL_IEN);

	i2c->msg = msgs;
	i2c->pos = 0;
	i2c->nmsgs = num;
	i2c->state = STATE_START;

	oc_setreg(i2c, OCI2C_DATA, i2c_8bit_addr_from_msg(i2c->msg));
	oc_setreg(i2c, OCI2C_CMD, OCI2C_CMD_START);

	if (polling)
		ocores_process_polling(i2c);

	return (i2c->state == STATE_DONE) ? num : -EIO;
}

static int ocores_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct ocores_i2c_bus *bus = dev_get_priv(dev);
	int ret;

	debug("i2c_xfer: %d messages\n", nmsgs);

	ret = ocores_xfer_core(bus, msg, nmsgs, 1);

	if (ret != nmsgs) {
		debug("i2c_write: error sending\n");
		return -EREMOTEIO;
	}

	return 0;
}

static int ocores_i2c_enable_clk(struct udevice *dev)
{
	struct ocores_i2c_bus *bus = dev_get_priv(dev);
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &bus->clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&bus->clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&bus->clk);
	if (!clk_rate)
		return -EINVAL;

	bus->ip_clk_khz = clk_rate / 1000;

	return 0;
}

static int ocores_init(struct udevice *dev, struct ocores_i2c_bus *bus)
{
	int prescale;
	int diff;
	u8 ctrl = oc_getreg(bus, OCI2C_CONTROL);

	/* make sure the device is disabled */
	ctrl &= ~(OCI2C_CTRL_EN | OCI2C_CTRL_IEN);
	oc_setreg(bus, OCI2C_CONTROL, ctrl);

	prescale = (bus->ip_clk_khz / (5 * bus->bus_clk_khz)) - 1;
	prescale = clamp(prescale, 0, 0xffff);

	diff = bus->ip_clk_khz / (5 * (prescale + 1)) - bus->bus_clk_khz;
	if (abs(diff) > bus->bus_clk_khz / 10) {
		debug("Unsupported clock settings: core: %d KHz, bus: %d KHz\n",
		      bus->ip_clk_khz, bus->bus_clk_khz);
		return -EINVAL;
	}

	oc_setreg(bus, OCI2C_PRELOW, prescale & 0xff);
	oc_setreg(bus, OCI2C_PREHIGH, prescale >> 8);

	/* Init the device */
	oc_setreg(bus, OCI2C_CMD, OCI2C_CMD_IACK);
	oc_setreg(bus, OCI2C_CONTROL, ctrl | OCI2C_CTRL_EN);

	return 0;
}

/*
 * Read and write functions for the GRLIB port of the controller. Registers are
 * 32-bit big endian and the PRELOW and PREHIGH registers are merged into one
 * register. The subsequent registers have their offsets decreased accordingly.
 */
static u8 oc_getreg_grlib(struct ocores_i2c_bus *i2c, int reg)
{
	u32 rd;
	int rreg = reg;

	if (reg != OCI2C_PRELOW)
		rreg--;
	rd = in_be32(i2c->base + (rreg << i2c->reg_shift));
	if (reg == OCI2C_PREHIGH)
		return (u8)(rd >> 8);
	else
		return (u8)rd;
}

static void oc_setreg_grlib(struct ocores_i2c_bus *i2c, int reg, u8 value)
{
	u32 curr, wr;
	int rreg = reg;

	if (reg != OCI2C_PRELOW)
		rreg--;
	if (reg == OCI2C_PRELOW || reg == OCI2C_PREHIGH) {
		curr = in_be32(i2c->base + (rreg << i2c->reg_shift));
		if (reg == OCI2C_PRELOW)
			wr = (curr & 0xff00) | value;
		else
			wr = (((u32)value) << 8) | (curr & 0xff);
	} else {
		wr = value;
	}
	out_be32(i2c->base + (rreg << i2c->reg_shift), wr);
}

static int ocores_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	int prescale;
	int diff;
	struct ocores_i2c_bus *bus = dev_get_priv(dev);

	/* speed in Khz */
	speed = speed / 1000;

	prescale = (bus->ip_clk_khz / (5 * speed)) - 1;
	prescale = clamp(prescale, 0, 0xffff);

	diff = bus->ip_clk_khz / (5 * (prescale + 1)) - speed;
	if (abs(diff) > speed / 10) {
		debug("Unsupported clock settings: core: %d KHz, bus: %d KHz\n",
		      bus->ip_clk_khz, speed);
		return -EINVAL;
	}

	oc_setreg(bus, OCI2C_PRELOW, prescale & 0xff);
	oc_setreg(bus, OCI2C_PREHIGH, prescale >> 8);

	bus->bus_clk_khz = speed;
	return 0;
}

int ocores_i2c_get_bus_speed(struct udevice *dev)
{
	struct ocores_i2c_bus *bus = dev_get_priv(dev);

	return (bus->bus_clk_khz * 1000);
}

static const struct dm_i2c_ops ocores_i2c_ops = {
	.xfer		= ocores_i2c_xfer,
	.set_bus_speed	= ocores_i2c_set_bus_speed,
	.get_bus_speed	= ocores_i2c_get_bus_speed,
};

static int ocores_i2c_probe(struct udevice *dev)
{
	struct ocores_i2c_bus *bus = dev_get_priv(dev);
	bool clock_frequency_present;
	u32 val;
	u32 clock_frequency_khz;
	int ret;

	bus->base = dev_read_addr_ptr(dev);

	if (dev_read_u32(dev, "reg-shift", &bus->reg_shift)) {
		/* no 'reg-shift', check for deprecated 'regstep' */
		ret = dev_read_u32(dev, "regstep", &val);
		if (ret) {
			dev_err(dev,
				"missing both reg-shift and regstep property: %d\n", ret);
			return -EINVAL;
		} else {
			bus->reg_shift = ilog2(val);
			dev_warn(dev,
				 "regstep property deprecated, use reg-shift\n");
		}
	}

	if (dev_read_u32(dev, "clock-frequency", &val)) {
		bus->bus_clk_khz = 100;
		clock_frequency_present = FALSE;
	} else {
		bus->bus_clk_khz = val / 1000;
		clock_frequency_khz = val / 1000;
		clock_frequency_present = TRUE;
	}

	ret = ocores_i2c_enable_clk(dev);
	if (ret)
		return ret;

	if (bus->ip_clk_khz == 0) {
		if (dev_read_u32(dev, "opencores,ip-clock-frequency", &val)) {
			if (!clock_frequency_present) {
				dev_err(dev,
					"Missing required parameter 'opencores,ip-clock-frequency'\n");
				clk_disable(&bus->clk);
				return -ENODEV;
			}

			bus->ip_clk_khz = clock_frequency_khz;
			dev_warn(dev,
				 "Deprecated usage of the 'clock-frequency' property, please update to 'opencores,ip-clock-frequency'\n");
		} else {
			bus->ip_clk_khz = val / 1000;
			if (clock_frequency_present)
				bus->bus_clk_khz = clock_frequency_khz;
		}
	}

	bus->reg_io_width = dev_read_u32_default(dev, "reg-io-width", 1);

	if (dev_get_driver_data(dev) == TYPE_GRLIB) {
		debug("GRLIB variant of i2c-ocores\n");
		bus->setreg = oc_setreg_grlib;
		bus->getreg = oc_getreg_grlib;
	}

	if (!bus->setreg || !bus->getreg) {
		bool be = (cpu_to_be32(0x12345678) == 0x12345678);

		switch (bus->reg_io_width) {
		case 1:
			bus->setreg = oc_setreg_8;
			bus->getreg = oc_getreg_8;
			break;

		case 2:
			bus->setreg = be ? oc_setreg_16be : oc_setreg_16;
			bus->getreg = be ? oc_getreg_16be : oc_getreg_16;
			break;

		case 4:
			bus->setreg = be ? oc_setreg_32be : oc_setreg_32;
			bus->getreg = be ? oc_getreg_32be : oc_getreg_32;
			break;

		default:
			debug("Unsupported I/O width (%d)\n",
			      bus->reg_io_width);
			ret = -EINVAL;
			goto err_clk;
		}
	}

	/*
	 * Set OCORES_FLAG_BROKEN_IRQ to enable workaround for
	 * FU540-C000 SoC in polling mode.
	 * Since the SoC does have an interrupt, its DT has an interrupt
	 * property - But this should be bypassed as the IRQ logic in this
	 * SoC is broken.
	 */

	if (device_is_compatible(dev, "sifive,fu540-c000-i2c"))
		bus->flags |= OCORES_FLAG_BROKEN_IRQ;

	ret = ocores_init(dev, bus);
	if (ret)
		goto err_clk;

	return 0;

err_clk:
	clk_disable(&bus->clk);
	return ret;
}

static const struct udevice_id ocores_i2c_ids[] = {
{ .compatible = "opencores,i2c-ocores", .data = TYPE_OCORES },
{ .compatible = "aeroflexgaisler,i2cmst", .data = TYPE_GRLIB },
{ .compatible = "sifive,fu540-c000-i2c" },
{ .compatible = "sifive,i2c0" },
{ }
};

U_BOOT_DRIVER(i2c_ocores) = {
	.name	= "i2c_ocores",
	.id	= UCLASS_I2C,
	.of_match = ocores_i2c_ids,
	.probe = ocores_i2c_probe,
	.priv_auto = sizeof(struct ocores_i2c_bus),
	.ops	= &ocores_i2c_ops,
};
