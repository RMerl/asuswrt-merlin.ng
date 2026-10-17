// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020
 * Arthur Li, Cortina Access, arthur.li@cortina-access.com.
 */

#include <common.h>
#include <i2c.h>
#include <log.h>
#include <asm/io.h>
#include <dm.h>
#include <mapmem.h>
#include "i2c-cortina.h"

static void set_speed(struct i2c_regs *regs, int i2c_spd)
{
	union ca_biw_cfg i2c_cfg;

	i2c_cfg.wrd = readl(&regs->i2c_cfg);
	i2c_cfg.bf.core_en = 0;
	writel(i2c_cfg.wrd, &regs->i2c_cfg);

	switch (i2c_spd) {
	case IC_SPEED_MODE_FAST_PLUS:
		i2c_cfg.bf.prer = CORTINA_PER_IO_FREQ /
				  (5 * I2C_SPEED_FAST_PLUS_RATE) - 1;
		break;

	case IC_SPEED_MODE_STANDARD:
		i2c_cfg.bf.prer = CORTINA_PER_IO_FREQ /
				  (5 * I2C_SPEED_STANDARD_RATE) - 1;
		break;

	case IC_SPEED_MODE_FAST:
	default:
		i2c_cfg.bf.prer = CORTINA_PER_IO_FREQ /
				  (5 * I2C_SPEED_FAST_RATE) - 1;
		break;
	}

	i2c_cfg.bf.core_en = 1;
	writel(i2c_cfg.wrd, &regs->i2c_cfg);
}

static int ca_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct ca_i2c *priv = dev_get_priv(bus);
	int i2c_spd;

	if (speed >= I2C_SPEED_FAST_PLUS_RATE) {
		i2c_spd = IC_SPEED_MODE_FAST_PLUS;
		priv->speed = I2C_SPEED_FAST_PLUS_RATE;
	} else if (speed >= I2C_SPEED_FAST_RATE) {
		i2c_spd = IC_SPEED_MODE_FAST;
		priv->speed = I2C_SPEED_FAST_RATE;
	} else {
		i2c_spd = IC_SPEED_MODE_STANDARD;
		priv->speed = I2C_SPEED_STANDARD_RATE;
	}

	set_speed(priv->regs, i2c_spd);

	return 0;
}

static int ca_i2c_get_bus_speed(struct udevice *bus)
{
	struct ca_i2c *priv = dev_get_priv(bus);

	return priv->speed;
}

static void ca_i2c_init(struct i2c_regs *regs)
{
	union ca_biw_cfg i2c_cfg;

	i2c_cfg.wrd = readl(&regs->i2c_cfg);
	i2c_cfg.bf.core_en = 0;
	i2c_cfg.bf.biw_soft_reset = 1;
	writel(i2c_cfg.wrd, &regs->i2c_cfg);
	mdelay(10);
	i2c_cfg.bf.biw_soft_reset = 0;
	writel(i2c_cfg.wrd, &regs->i2c_cfg);

	set_speed(regs, IC_SPEED_MODE_STANDARD);

	i2c_cfg.wrd = readl(&regs->i2c_cfg);
	i2c_cfg.bf.core_en = 1;
	writel(i2c_cfg.wrd, &regs->i2c_cfg);
}

static int i2c_wait_complete(struct i2c_regs *regs)
{
	union ca_biw_ctrl i2c_ctrl;
	unsigned long start_time_bb = get_timer(0);

	i2c_ctrl.wrd = readl(&regs->i2c_ctrl);

	while (i2c_ctrl.bf.biwdone == 0) {
		i2c_ctrl.wrd = readl(&regs->i2c_ctrl);

		if (get_timer(start_time_bb) >
		   (unsigned long)(I2C_BYTE_TO_BB)) {
			printf("%s not done!!!\n", __func__);
			return -ETIMEDOUT;
		}
	}

	/* Clear done bit */
	writel(i2c_ctrl.wrd, &regs->i2c_ctrl);

	return 0;
}

static void i2c_setaddress(struct i2c_regs *regs, unsigned int i2c_addr,
			   int write_read)
{
	writel(i2c_addr | write_read, &regs->i2c_txr);

	writel(BIW_CTRL_START | BIW_CTRL_WRITE,
	       &regs->i2c_ctrl);

	i2c_wait_complete(regs);
}

static int i2c_wait_for_bus_busy(struct i2c_regs *regs)
{
	union ca_biw_ack i2c_ack;
	unsigned long start_time_bb = get_timer(0);

	i2c_ack.wrd = readl(&regs->i2c_ack);

	while (i2c_ack.bf.biw_busy) {
		i2c_ack.wrd = readl(&regs->i2c_ack);

		if (get_timer(start_time_bb) >
		   (unsigned long)(I2C_BYTE_TO_BB)) {
			printf("%s: timeout!\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int i2c_xfer_init(struct i2c_regs *regs, uint8_t chip, uint addr,
			 int alen, int write_read)
{
	int addr_len = alen;

	if (i2c_wait_for_bus_busy(regs))
		return 1;

	/* First cycle must write addr + offset */
	chip = ((chip & 0x7F) << 1);
	if (alen == 0 && write_read == I2C_CMD_RD)
		i2c_setaddress(regs, chip, I2C_CMD_RD);
	else
		i2c_setaddress(regs, chip, I2C_CMD_WT);

	while (alen) {
		alen--;
		writel(addr, &regs->i2c_txr);
		if (write_read == I2C_CMD_RD)
			writel(BIW_CTRL_WRITE | BIW_CTRL_STOP,
			       &regs->i2c_ctrl);
		else
			writel(BIW_CTRL_WRITE, &regs->i2c_ctrl);
		i2c_wait_complete(regs);
	}

	/* Send address again with Read flag if it's read command */
	if (write_read == I2C_CMD_RD && addr_len > 0)
		i2c_setaddress(regs, chip, I2C_CMD_RD);

	return 0;
}

static int i2c_xfer_finish(struct i2c_regs *regs)
{
	/* Dummy read makes bus free */
	writel(BIW_CTRL_READ | BIW_CTRL_STOP, &regs->i2c_ctrl);
	i2c_wait_complete(regs);

	if (i2c_wait_for_bus_busy(regs)) {
		printf("Timed out waiting for bus\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int ca_i2c_read(struct i2c_regs *regs, uint8_t chip, uint addr,
		       int alen, uint8_t *buffer, int len)
{
	unsigned long start_time_rx;
	int rc = 0;

	rc = i2c_xfer_init(regs, chip, addr, alen, I2C_CMD_RD);
	if (rc)
		return rc;

	start_time_rx = get_timer(0);
	while (len) {
		/* ACK_IN is ack value to send during read.
		 * ack high only on the very last byte!
		 */
		if (len == 1)
			writel(BIW_CTRL_READ | BIW_CTRL_ACK_IN | BIW_CTRL_STOP,
			       &regs->i2c_ctrl);
		else
			writel(BIW_CTRL_READ, &regs->i2c_ctrl);

		rc = i2c_wait_complete(regs);
		udelay(1);

		if (rc == 0) {
			*buffer++ =
				(uchar) readl(&regs->i2c_rxr);
			len--;
			start_time_rx = get_timer(0);

		} else if (get_timer(start_time_rx) > I2C_BYTE_TO) {
			return -ETIMEDOUT;
		}
	}
	i2c_xfer_finish(regs);
	return rc;
}

static int ca_i2c_write(struct i2c_regs *regs, uint8_t chip, uint addr,
			int alen, uint8_t *buffer, int len)
{
	int rc, nb = len;
	unsigned long start_time_tx;

	rc = i2c_xfer_init(regs, chip, addr, alen, I2C_CMD_WT);
	if (rc)
		return rc;

	start_time_tx = get_timer(0);
	while (len) {
		writel(*buffer, &regs->i2c_txr);
		if (len == 1)
			writel(BIW_CTRL_WRITE | BIW_CTRL_STOP,
			       &regs->i2c_ctrl);
		else
			writel(BIW_CTRL_WRITE, &regs->i2c_ctrl);

		rc = i2c_wait_complete(regs);

		if (rc == 0) {
			len--;
			buffer++;
			start_time_tx = get_timer(0);
		} else if (get_timer(start_time_tx) > (nb * I2C_BYTE_TO)) {
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int ca_i2c_probe_chip(struct udevice *bus, uint chip_addr,
			     uint chip_flags)
{
	struct ca_i2c *priv = dev_get_priv(bus);
	int ret;
	u32 tmp;

	/* Try to read the first location of the chip */
	ret = ca_i2c_read(priv->regs, chip_addr, 0, 1, (uchar *)&tmp, 1);
	if (ret)
		ca_i2c_init(priv->regs);

	return ret;
}

static int ca_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct ca_i2c *priv = dev_get_priv(bus);
	int ret;

	debug("i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD)
			ret = ca_i2c_read(priv->regs, msg->addr, 0, 0,
					  msg->buf, msg->len);
		else
			ret = ca_i2c_write(priv->regs, msg->addr, 0, 0,
					   msg->buf, msg->len);

		if (ret) {
			printf("i2c_xfer: %s error\n",
			       msg->flags & I2C_M_RD ? "read" : "write");
			return ret;
		}
	}

	return 0;
}

static const struct dm_i2c_ops ca_i2c_ops = {
	.xfer		= ca_i2c_xfer,
	.probe_chip	= ca_i2c_probe_chip,
	.set_bus_speed	= ca_i2c_set_bus_speed,
	.get_bus_speed	= ca_i2c_get_bus_speed,
};

static const struct udevice_id ca_i2c_ids[] = {
	{ .compatible = "cortina,ca-i2c", },
	{ }
};

static int ca_i2c_probe(struct udevice *bus)
{
	struct ca_i2c *priv = dev_get_priv(bus);

	ca_i2c_init(priv->regs);

	return 0;
}

static int ca_i2c_of_to_plat(struct udevice *bus)
{
	struct ca_i2c *priv = dev_get_priv(bus);

	priv->regs = map_sysmem(dev_read_addr(bus), sizeof(struct i2c_regs));
	if (!priv->regs) {
		printf("I2C: base address is invalid\n");
		return -EINVAL;
	}

	return 0;
}

U_BOOT_DRIVER(i2c_cortina) = {
	.name	= "i2c_cortina",
	.id	= UCLASS_I2C,
	.of_match = ca_i2c_ids,
	.of_to_plat = ca_i2c_of_to_plat,
	.probe	= ca_i2c_probe,
	.priv_auto	= sizeof(struct ca_i2c),
	.ops	= &ca_i2c_ops,
	.flags  = DM_FLAG_PRE_RELOC,
};
