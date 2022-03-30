// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#if (defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pinmux.h>
#else
#include <asm/arch/s3c24x0_cpu.h>
#endif
#include <asm/io.h>
#include <i2c.h>
#include "s3c24x0_i2c.h"

#ifndef CONFIG_SYS_I2C_S3C24X0_SLAVE
#define SYS_I2C_S3C24X0_SLAVE_ADDR	0
#else
#define SYS_I2C_S3C24X0_SLAVE_ADDR	CONFIG_SYS_I2C_S3C24X0_SLAVE
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * Wait til the byte transfer is completed.
 *
 * @param i2c- pointer to the appropriate i2c register bank.
 * @return I2C_OK, if transmission was ACKED
 *         I2C_NACK, if transmission was NACKED
 *         I2C_NOK_TIMEOUT, if transaction did not complete in I2C_TIMEOUT_MS
 */

static int WaitForXfer(struct s3c24x0_i2c *i2c)
{
	ulong start_time = get_timer(0);

	do {
		if (readl(&i2c->iiccon) & I2CCON_IRPND)
			return (readl(&i2c->iicstat) & I2CSTAT_NACK) ?
				I2C_NACK : I2C_OK;
	} while (get_timer(start_time) < I2C_TIMEOUT_MS);

	return I2C_NOK_TOUT;
}

static void read_write_byte(struct s3c24x0_i2c *i2c)
{
	clrbits_le32(&i2c->iiccon, I2CCON_IRPND);
}

static void i2c_ch_init(struct s3c24x0_i2c *i2c, int speed, int slaveadd)
{
	ulong freq, pres = 16, div;
#if (defined CONFIG_EXYNOS4 || defined CONFIG_EXYNOS5)
	freq = get_i2c_clk();
#else
	freq = get_PCLK();
#endif
	/* calculate prescaler and divisor values */
	if ((freq / pres / (16 + 1)) > speed)
		/* set prescaler to 512 */
		pres = 512;

	div = 0;
	while ((freq / pres / (div + 1)) > speed)
		div++;

	/* set prescaler, divisor according to freq, also set ACKGEN, IRQ */
	writel((div & 0x0F) | 0xA0 | ((pres == 512) ? 0x40 : 0), &i2c->iiccon);

	/* init to SLAVE REVEIVE and set slaveaddr */
	writel(0, &i2c->iicstat);
	writel(slaveadd, &i2c->iicadd);
	/* program Master Transmit (and implicit STOP) */
	writel(I2C_MODE_MT | I2C_TXRX_ENA, &i2c->iicstat);
}

static int s3c24x0_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);

	i2c_bus->clock_frequency = speed;

	i2c_ch_init(i2c_bus->regs, i2c_bus->clock_frequency,
		    SYS_I2C_S3C24X0_SLAVE_ADDR);

	return 0;
}

/*
 * cmd_type is 0 for write, 1 for read.
 *
 * addr_len can take any value from 0-255, it is only limited
 * by the char, we could make it larger if needed. If it is
 * 0 we skip the address write cycle.
 */
static int i2c_transfer(struct s3c24x0_i2c *i2c,
			unsigned char cmd_type,
			unsigned char chip,
			unsigned char addr[],
			unsigned char addr_len,
			unsigned char data[],
			unsigned short data_len)
{
	int i = 0, result;
	ulong start_time = get_timer(0);

	if (data == 0 || data_len == 0) {
		/*Don't support data transfer of no length or to address 0 */
		debug("i2c_transfer: bad call\n");
		return I2C_NOK;
	}

	while (readl(&i2c->iicstat) & I2CSTAT_BSY) {
		if (get_timer(start_time) > I2C_TIMEOUT_MS)
			return I2C_NOK_TOUT;
	}

	writel(readl(&i2c->iiccon) | I2CCON_ACKGEN, &i2c->iiccon);

	/* Get the slave chip address going */
	writel(chip, &i2c->iicds);
	if ((cmd_type == I2C_WRITE) || (addr && addr_len))
		writel(I2C_MODE_MT | I2C_TXRX_ENA | I2C_START_STOP,
		       &i2c->iicstat);
	else
		writel(I2C_MODE_MR | I2C_TXRX_ENA | I2C_START_STOP,
		       &i2c->iicstat);

	/* Wait for chip address to transmit. */
	result = WaitForXfer(i2c);
	if (result != I2C_OK)
		goto bailout;

	/* If register address needs to be transmitted - do it now. */
	if (addr && addr_len) {
		while ((i < addr_len) && (result == I2C_OK)) {
			writel(addr[i++], &i2c->iicds);
			read_write_byte(i2c);
			result = WaitForXfer(i2c);
		}
		i = 0;
		if (result != I2C_OK)
			goto bailout;
	}

	switch (cmd_type) {
	case I2C_WRITE:
		while ((i < data_len) && (result == I2C_OK)) {
			writel(data[i++], &i2c->iicds);
			read_write_byte(i2c);
			result = WaitForXfer(i2c);
		}
		break;

	case I2C_READ:
		if (addr && addr_len) {
			/*
			 * Register address has been sent, now send slave chip
			 * address again to start the actual read transaction.
			 */
			writel(chip, &i2c->iicds);

			/* Generate a re-START. */
			writel(I2C_MODE_MR | I2C_TXRX_ENA | I2C_START_STOP,
				&i2c->iicstat);
			read_write_byte(i2c);
			result = WaitForXfer(i2c);

			if (result != I2C_OK)
				goto bailout;
		}

		while ((i < data_len) && (result == I2C_OK)) {
			/* disable ACK for final READ */
			if (i == data_len - 1)
				writel(readl(&i2c->iiccon)
				       & ~I2CCON_ACKGEN,
				       &i2c->iiccon);
			read_write_byte(i2c);
			result = WaitForXfer(i2c);
			data[i++] = readl(&i2c->iicds);
		}
		if (result == I2C_NACK)
			result = I2C_OK; /* Normal terminated read. */
		break;

	default:
		debug("i2c_transfer: bad call\n");
		result = I2C_NOK;
		break;
	}

bailout:
	/* Send STOP. */
	writel(I2C_MODE_MR | I2C_TXRX_ENA, &i2c->iicstat);
	read_write_byte(i2c);

	return result;
}

static int s3c24x0_i2c_probe(struct udevice *dev, uint chip, uint chip_flags)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	uchar buf[1];
	int ret;

	buf[0] = 0;

	/*
	 * What is needed is to send the chip address and verify that the
	 * address was <ACK>ed (i.e. there was a chip at that address which
	 * drove the data line low).
	 */
	ret = i2c_transfer(i2c_bus->regs, I2C_READ, chip << 1, 0, 0, buf, 1);

	return ret != I2C_OK;
}

static int s3c24x0_do_msg(struct s3c24x0_i2c_bus *i2c_bus, struct i2c_msg *msg,
			  int seq)
{
	struct s3c24x0_i2c *i2c = i2c_bus->regs;
	bool is_read = msg->flags & I2C_M_RD;
	uint status;
	uint addr;
	int ret, i;

	if (!seq)
		setbits_le32(&i2c->iiccon, I2CCON_ACKGEN);

	/* Get the slave chip address going */
	addr = msg->addr << 1;
	writel(addr, &i2c->iicds);
	status = I2C_TXRX_ENA | I2C_START_STOP;
	if (is_read)
		status |= I2C_MODE_MR;
	else
		status |= I2C_MODE_MT;
	writel(status, &i2c->iicstat);
	if (seq)
		read_write_byte(i2c);

	/* Wait for chip address to transmit */
	ret = WaitForXfer(i2c);
	if (ret)
		goto err;

	if (is_read) {
		for (i = 0; !ret && i < msg->len; i++) {
			/* disable ACK for final READ */
			if (i == msg->len - 1)
				clrbits_le32(&i2c->iiccon, I2CCON_ACKGEN);
			read_write_byte(i2c);
			ret = WaitForXfer(i2c);
			msg->buf[i] = readl(&i2c->iicds);
		}
		if (ret == I2C_NACK)
			ret = I2C_OK; /* Normal terminated read */
	} else {
		for (i = 0; !ret && i < msg->len; i++) {
			writel(msg->buf[i], &i2c->iicds);
			read_write_byte(i2c);
			ret = WaitForXfer(i2c);
		}
	}

err:
	return ret;
}

static int s3c24x0_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			    int nmsgs)
{
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	struct s3c24x0_i2c *i2c = i2c_bus->regs;
	ulong start_time;
	int ret, i;

	start_time = get_timer(0);
	while (readl(&i2c->iicstat) & I2CSTAT_BSY) {
		if (get_timer(start_time) > I2C_TIMEOUT_MS) {
			debug("Timeout\n");
			return -ETIMEDOUT;
		}
	}

	for (ret = 0, i = 0; !ret && i < nmsgs; i++)
		ret = s3c24x0_do_msg(i2c_bus, &msg[i], i);

	/* Send STOP */
	writel(I2C_MODE_MR | I2C_TXRX_ENA, &i2c->iicstat);
	read_write_byte(i2c);

	return ret ? -EREMOTEIO : 0;
}

static int s3c_i2c_ofdata_to_platdata(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	struct s3c24x0_i2c_bus *i2c_bus = dev_get_priv(dev);
	int node;

	node = dev_of_offset(dev);

	i2c_bus->regs = (struct s3c24x0_i2c *)devfdt_get_addr(dev);

	i2c_bus->id = pinmux_decode_periph_id(blob, node);

	i2c_bus->clock_frequency = fdtdec_get_int(blob, node,
						  "clock-frequency", 100000);
	i2c_bus->node = node;
	i2c_bus->bus_num = dev->seq;

	exynos_pinmux_config(i2c_bus->id, 0);

	i2c_bus->active = true;

	return 0;
}

static const struct dm_i2c_ops s3c_i2c_ops = {
	.xfer		= s3c24x0_i2c_xfer,
	.probe_chip	= s3c24x0_i2c_probe,
	.set_bus_speed	= s3c24x0_i2c_set_bus_speed,
};

static const struct udevice_id s3c_i2c_ids[] = {
	{ .compatible = "samsung,s3c2440-i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_s3c) = {
	.name	= "i2c_s3c",
	.id	= UCLASS_I2C,
	.of_match = s3c_i2c_ids,
	.ofdata_to_platdata = s3c_i2c_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct s3c24x0_i2c_bus),
	.ops	= &s3c_i2c_ops,
};
