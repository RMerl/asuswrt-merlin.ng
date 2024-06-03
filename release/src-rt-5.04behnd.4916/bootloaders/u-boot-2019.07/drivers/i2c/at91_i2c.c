// SPDX-License-Identifier: GPL-2.0+
/*
 * Atmel I2C driver.
 *
 * (C) Copyright 2016 Songjun Wu <songjun.wu@atmel.com>
 */

#include <asm/io.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>
#include <linux/bitops.h>
#include <mach/clk.h>

#include "at91_i2c.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_TIMEOUT_MS	100

static int at91_wait_for_xfer(struct at91_i2c_bus *bus, u32 status)
{
	struct at91_i2c_regs *reg = bus->regs;
	ulong start_time = get_timer(0);
	u32 sr;

	bus->status = 0;

	do {
		sr = readl(&reg->sr);
		bus->status |= sr;

		if (sr & TWI_SR_NACK)
			return -EREMOTEIO;
		else if (sr & status)
			return 0;
	} while (get_timer(start_time) < I2C_TIMEOUT_MS);

	return -ETIMEDOUT;
}

static int at91_i2c_xfer_msg(struct at91_i2c_bus *bus, struct i2c_msg *msg)
{
	struct at91_i2c_regs *reg = bus->regs;
	bool is_read = msg->flags & I2C_M_RD;
	u32 i;
	int ret = 0;

	readl(&reg->sr);
	if (is_read) {
		writel(TWI_CR_START, &reg->cr);

		for (i = 0; !ret && i < (msg->len - 1); i++) {
			ret = at91_wait_for_xfer(bus, TWI_SR_RXRDY);
			msg->buf[i] = readl(&reg->rhr);
		}

		if (ret)
			goto error;

		writel(TWI_CR_STOP, &reg->cr);

		ret = at91_wait_for_xfer(bus, TWI_SR_RXRDY);
		if (ret)
			goto error;

		msg->buf[i] = readl(&reg->rhr);

	} else {
		writel(msg->buf[0], &reg->thr);
		ret = at91_wait_for_xfer(bus, TWI_SR_TXRDY);

		for (i = 1; !ret && (i < msg->len); i++) {
			writel(msg->buf[i], &reg->thr);
			ret = at91_wait_for_xfer(bus, TWI_SR_TXRDY);
		}

		if (ret)
			goto error;

		writel(TWI_CR_STOP, &reg->cr);
	}

	if (!ret)
		ret = at91_wait_for_xfer(bus, TWI_SR_TXCOMP);

	if (ret)
		goto error;

	if (bus->status & (TWI_SR_OVRE | TWI_SR_UNRE | TWI_SR_LOCK)) {
		ret = -EIO;
		goto error;
	}

	return 0;

error:
	if (bus->status & TWI_SR_LOCK)
		writel(TWI_CR_LOCKCLR, &reg->cr);

	return ret;
}

static int at91_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct at91_i2c_bus *bus = dev_get_priv(dev);
	struct at91_i2c_regs *reg = bus->regs;
	struct i2c_msg *m_start = msg;
	bool is_read;
	u32 int_addr_flag = 0;
	int ret = 0;

	if (nmsgs == 2) {
		int internal_address = 0;
		int i;

		/* 1st msg is put into the internal address, start with 2nd */
		m_start = &msg[1];

		/* the max length of internal address is 3 bytes */
		if (msg->len > 3)
			return -EFAULT;

		for (i = 0; i < msg->len; ++i) {
			const unsigned addr = msg->buf[msg->len - 1 - i];

			internal_address |= addr << (8 * i);
			int_addr_flag += TWI_MMR_IADRSZ_1;
		}

		writel(internal_address, &reg->iadr);
	}

	is_read = m_start->flags & I2C_M_RD;

	writel((m_start->addr << 16) | int_addr_flag |
	       (is_read ? TWI_MMR_MREAD : 0), &reg->mmr);

	ret = at91_i2c_xfer_msg(bus, m_start);

	return ret;
}

/*
 * Calculate symmetric clock as stated in datasheet:
 * twi_clk = F_MAIN / (2 * (cdiv * (1 << ckdiv) + offset))
 */
static void at91_calc_i2c_clock(struct udevice *dev, int i2c_clk)
{
	struct at91_i2c_bus *bus = dev_get_priv(dev);
	const struct at91_i2c_pdata *pdata = bus->pdata;
	int offset = pdata->clk_offset;
	int max_ckdiv = pdata->clk_max_div;
	int ckdiv, cdiv, div;
	unsigned long src_rate;

	src_rate = bus->bus_clk_rate;

	div = max(0, (int)DIV_ROUND_UP(src_rate, 2 * i2c_clk) - offset);
	ckdiv = fls(div >> 8);
	cdiv = div >> ckdiv;

	if (ckdiv > max_ckdiv) {
		ckdiv = max_ckdiv;
		cdiv = 255;
	}

	bus->speed = DIV_ROUND_UP(src_rate,
				  (cdiv * (1 << ckdiv) + offset) * 2);

	bus->cwgr_val = (ckdiv << 16) | (cdiv << 8) | cdiv;
}

static int at91_i2c_enable_clk(struct udevice *dev)
{
	struct at91_i2c_bus *bus = dev_get_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate)
		return -EINVAL;

	bus->bus_clk_rate = clk_rate;

	clk_free(&clk);

	return 0;
}

static int at91_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct at91_i2c_bus *bus = dev_get_priv(dev);

	at91_calc_i2c_clock(dev, speed);

	writel(bus->cwgr_val, &bus->regs->cwgr);

	return 0;
}

int at91_i2c_get_bus_speed(struct udevice *dev)
{
	struct at91_i2c_bus *bus = dev_get_priv(dev);

	return bus->speed;
}

static int at91_i2c_ofdata_to_platdata(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	struct at91_i2c_bus *bus = dev_get_priv(dev);
	int node = dev_of_offset(dev);

	bus->regs = (struct at91_i2c_regs *)devfdt_get_addr(dev);
	bus->pdata = (struct at91_i2c_pdata *)dev_get_driver_data(dev);
	bus->clock_frequency = fdtdec_get_int(blob, node,
					      "clock-frequency", 100000);

	return 0;
}

static const struct dm_i2c_ops at91_i2c_ops = {
	.xfer		= at91_i2c_xfer,
	.set_bus_speed	= at91_i2c_set_bus_speed,
	.get_bus_speed	= at91_i2c_get_bus_speed,
};

static int at91_i2c_probe(struct udevice *dev)
{
	struct at91_i2c_bus *bus = dev_get_priv(dev);
	struct at91_i2c_regs *reg = bus->regs;
	int ret;

	ret = at91_i2c_enable_clk(dev);
	if (ret)
		return ret;

	writel(TWI_CR_SWRST, &reg->cr);

	at91_calc_i2c_clock(dev, bus->clock_frequency);

	writel(bus->cwgr_val, &reg->cwgr);
	writel(TWI_CR_MSEN, &reg->cr);
	writel(TWI_CR_SVDIS, &reg->cr);

	return 0;
}

static const struct at91_i2c_pdata at91rm9200_config = {
	.clk_max_div = 5,
	.clk_offset = 3,
};

static const struct at91_i2c_pdata at91sam9261_config = {
	.clk_max_div = 5,
	.clk_offset = 4,
};

static const struct at91_i2c_pdata at91sam9260_config = {
	.clk_max_div = 7,
	.clk_offset = 4,
};

static const struct at91_i2c_pdata at91sam9g20_config = {
	.clk_max_div = 7,
	.clk_offset = 4,
};

static const struct at91_i2c_pdata at91sam9g10_config = {
	.clk_max_div = 7,
	.clk_offset = 4,
};

static const struct at91_i2c_pdata at91sam9x5_config = {
	.clk_max_div = 7,
	.clk_offset = 4,
};

static const struct at91_i2c_pdata sama5d4_config = {
	.clk_max_div = 7,
	.clk_offset = 4,
};

static const struct at91_i2c_pdata sama5d2_config = {
	.clk_max_div = 7,
	.clk_offset = 3,
};

static const struct udevice_id at91_i2c_ids[] = {
{ .compatible = "atmel,at91rm9200-i2c", .data = (long)&at91rm9200_config },
{ .compatible = "atmel,at91sam9260-i2c", .data = (long)&at91sam9260_config },
{ .compatible = "atmel,at91sam9261-i2c", .data = (long)&at91sam9261_config },
{ .compatible = "atmel,at91sam9g20-i2c", .data = (long)&at91sam9g20_config },
{ .compatible = "atmel,at91sam9g10-i2c", .data = (long)&at91sam9g10_config },
{ .compatible = "atmel,at91sam9x5-i2c", .data = (long)&at91sam9x5_config },
{ .compatible = "atmel,sama5d4-i2c", .data = (long)&sama5d4_config },
{ .compatible = "atmel,sama5d2-i2c", .data = (long)&sama5d2_config },
{ }
};

U_BOOT_DRIVER(i2c_at91) = {
	.name	= "i2c_at91",
	.id	= UCLASS_I2C,
	.of_match = at91_i2c_ids,
	.probe = at91_i2c_probe,
	.ofdata_to_platdata = at91_i2c_ofdata_to_platdata,
	.per_child_auto_alloc_size = sizeof(struct dm_i2c_chip),
	.priv_auto_alloc_size = sizeof(struct at91_i2c_bus),
	.ops	= &at91_i2c_ops,
};
