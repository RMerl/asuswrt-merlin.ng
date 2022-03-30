// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <imx_lpi2c.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>

#define LPI2C_FIFO_SIZE 4
#define LPI2C_NACK_TOUT_MS 1
#define LPI2C_TIMEOUT_MS 100

static int bus_i2c_init(struct udevice *bus, int speed);

/* Weak linked function for overridden by some SoC power function */
int __weak init_i2c_power(unsigned i2c_num)
{
	return 0;
}

static int imx_lpci2c_check_busy_bus(const struct imx_lpi2c_reg *regs)
{
	lpi2c_status_t result = LPI2C_SUCESS;
	u32 status;

	status = readl(&regs->msr);

	if ((status & LPI2C_MSR_BBF_MASK) && !(status & LPI2C_MSR_MBF_MASK))
		result = LPI2C_BUSY;

	return result;
}

static int imx_lpci2c_check_clear_error(struct imx_lpi2c_reg *regs)
{
	lpi2c_status_t result = LPI2C_SUCESS;
	u32 val, status;

	status = readl(&regs->msr);
	/* errors to check for */
	status &= LPI2C_MSR_NDF_MASK | LPI2C_MSR_ALF_MASK |
		LPI2C_MSR_FEF_MASK | LPI2C_MSR_PLTF_MASK;

	if (status) {
		if (status & LPI2C_MSR_PLTF_MASK)
			result = LPI2C_PIN_LOW_TIMEOUT_ERR;
		else if (status & LPI2C_MSR_ALF_MASK)
			result = LPI2C_ARB_LOST_ERR;
		else if (status & LPI2C_MSR_NDF_MASK)
			result = LPI2C_NAK_ERR;
		else if (status & LPI2C_MSR_FEF_MASK)
			result = LPI2C_FIFO_ERR;

		/* clear status flags */
		writel(0x7f00, &regs->msr);
		/* reset fifos */
		val = readl(&regs->mcr);
		val |= LPI2C_MCR_RRF_MASK | LPI2C_MCR_RTF_MASK;
		writel(val, &regs->mcr);
	}

	return result;
}

static int bus_i2c_wait_for_tx_ready(struct imx_lpi2c_reg *regs)
{
	lpi2c_status_t result = LPI2C_SUCESS;
	u32 txcount = 0;
	ulong start_time = get_timer(0);

	do {
		txcount = LPI2C_MFSR_TXCOUNT(readl(&regs->mfsr));
		txcount = LPI2C_FIFO_SIZE - txcount;
		result = imx_lpci2c_check_clear_error(regs);
		if (result) {
			debug("i2c: wait for tx ready: result 0x%x\n", result);
			return result;
		}
		if (get_timer(start_time) > LPI2C_TIMEOUT_MS) {
			debug("i2c: wait for tx ready: timeout\n");
			return -1;
		}
	} while (!txcount);

	return result;
}

static int bus_i2c_send(struct udevice *bus, u8 *txbuf, int len)
{
	struct imx_lpi2c_reg *regs = (struct imx_lpi2c_reg *)devfdt_get_addr(bus);
	lpi2c_status_t result = LPI2C_SUCESS;

	/* empty tx */
	if (!len)
		return result;

	while (len--) {
		result = bus_i2c_wait_for_tx_ready(regs);
		if (result) {
			debug("i2c: send wait for tx ready: %d\n", result);
			return result;
		}
		writel(*txbuf++, &regs->mtdr);
	}

	return result;
}

static int bus_i2c_receive(struct udevice *bus, u8 *rxbuf, int len)
{
	struct imx_lpi2c_reg *regs = (struct imx_lpi2c_reg *)devfdt_get_addr(bus);
	lpi2c_status_t result = LPI2C_SUCESS;
	u32 val;
	ulong start_time = get_timer(0);

	/* empty read */
	if (!len)
		return result;

	result = bus_i2c_wait_for_tx_ready(regs);
	if (result) {
		debug("i2c: receive wait fot tx ready: %d\n", result);
		return result;
	}

	/* clear all status flags */
	writel(0x7f00, &regs->msr);
	/* send receive command */
	val = LPI2C_MTDR_CMD(0x1) | LPI2C_MTDR_DATA(len - 1);
	writel(val, &regs->mtdr);

	while (len--) {
		do {
			result = imx_lpci2c_check_clear_error(regs);
			if (result) {
				debug("i2c: receive check clear error: %d\n",
				      result);
				return result;
			}
			if (get_timer(start_time) > LPI2C_TIMEOUT_MS) {
				debug("i2c: receive mrdr: timeout\n");
				return -1;
			}
			val = readl(&regs->mrdr);
		} while (val & LPI2C_MRDR_RXEMPTY_MASK);
		*rxbuf++ = LPI2C_MRDR_DATA(val);
	}

	return result;
}

static int bus_i2c_start(struct udevice *bus, u8 addr, u8 dir)
{
	lpi2c_status_t result;
	struct imx_lpi2c_reg *regs =
		(struct imx_lpi2c_reg *)devfdt_get_addr(bus);
	u32 val;

	result = imx_lpci2c_check_busy_bus(regs);
	if (result) {
		debug("i2c: start check busy bus: 0x%x\n", result);

		/* Try to init the lpi2c then check the bus busy again */
		bus_i2c_init(bus, 100000);
		result = imx_lpci2c_check_busy_bus(regs);
		if (result) {
			printf("i2c: Error check busy bus: 0x%x\n", result);
			return result;
		}
	}
	/* clear all status flags */
	writel(0x7f00, &regs->msr);
	/* turn off auto-stop condition */
	val = readl(&regs->mcfgr1) & ~LPI2C_MCFGR1_AUTOSTOP_MASK;
	writel(val, &regs->mcfgr1);
	/* wait tx fifo ready */
	result = bus_i2c_wait_for_tx_ready(regs);
	if (result) {
		debug("i2c: start wait for tx ready: 0x%x\n", result);
		return result;
	}
	/* issue start command */
	val = LPI2C_MTDR_CMD(0x4) | (addr << 0x1) | dir;
	writel(val, &regs->mtdr);

	return result;
}

static int bus_i2c_stop(struct udevice *bus)
{
	lpi2c_status_t result;
	struct imx_lpi2c_reg *regs =
		(struct imx_lpi2c_reg *)devfdt_get_addr(bus);
	u32 status;
	ulong start_time;

	result = bus_i2c_wait_for_tx_ready(regs);
	if (result) {
		debug("i2c: stop wait for tx ready: 0x%x\n", result);
		return result;
	}

	/* send stop command */
	writel(LPI2C_MTDR_CMD(0x2), &regs->mtdr);

	start_time = get_timer(0);
	while (1) {
		status = readl(&regs->msr);
		result = imx_lpci2c_check_clear_error(regs);
		/* stop detect flag */
		if (status & LPI2C_MSR_SDF_MASK) {
			/* clear stop flag */
			status &= LPI2C_MSR_SDF_MASK;
			writel(status, &regs->msr);
			break;
		}

		if (get_timer(start_time) > LPI2C_NACK_TOUT_MS) {
			debug("stop timeout\n");
			return -ETIMEDOUT;
		}
	}

	return result;
}

static int bus_i2c_read(struct udevice *bus, u32 chip, u8 *buf, int len)
{
	lpi2c_status_t result;

	result = bus_i2c_start(bus, chip, 1);
	if (result)
		return result;
	result = bus_i2c_receive(bus, buf, len);
	if (result)
		return result;

	return result;
}

static int bus_i2c_write(struct udevice *bus, u32 chip, u8 *buf, int len)
{
	lpi2c_status_t result;

	result = bus_i2c_start(bus, chip, 0);
	if (result)
		return result;
	result = bus_i2c_send(bus, buf, len);
	if (result)
		return result;

	return result;
}


u32 __weak imx_get_i2cclk(u32 i2c_num)
{
	return 0;
}

static int bus_i2c_set_bus_speed(struct udevice *bus, int speed)
{
	struct imx_lpi2c_bus *i2c_bus = dev_get_priv(bus);
	struct imx_lpi2c_reg *regs;
	u32 val;
	u32 preescale = 0, best_pre = 0, clkhi = 0;
	u32 best_clkhi = 0, abs_error = 0, rate;
	u32 error = 0xffffffff;
	u32 clock_rate;
	bool mode;
	int i;

	regs = (struct imx_lpi2c_reg *)devfdt_get_addr(bus);

	if (IS_ENABLED(CONFIG_CLK)) {
		clock_rate = clk_get_rate(&i2c_bus->per_clk);
		if (clock_rate <= 0) {
			dev_err(bus, "Failed to get i2c clk: %d\n", clock_rate);
			return clock_rate;
		}
	} else {
		clock_rate = imx_get_i2cclk(bus->seq);
		if (!clock_rate)
			return -EPERM;
	}

	mode = (readl(&regs->mcr) & LPI2C_MCR_MEN_MASK) >> LPI2C_MCR_MEN_SHIFT;
	/* disable master mode */
	val = readl(&regs->mcr) & ~LPI2C_MCR_MEN_MASK;
	writel(val | LPI2C_MCR_MEN(0), &regs->mcr);

	for (preescale = 1; (preescale <= 128) &&
		(error != 0); preescale = 2 * preescale) {
		for (clkhi = 1; clkhi < 32; clkhi++) {
			if (clkhi == 1)
				rate = (clock_rate / preescale) / (1 + 3 + 2 + 2 / preescale);
			else
				rate = (clock_rate / preescale / (3 * clkhi + 2 + 2 / preescale));

			abs_error = speed > rate ? speed - rate : rate - speed;

			if (abs_error < error) {
				best_pre = preescale;
				best_clkhi = clkhi;
				error = abs_error;
				if (abs_error == 0)
					break;
			}
		}
	}

	/* Standard, fast, fast mode plus and ultra-fast transfers. */
	val = LPI2C_MCCR0_CLKHI(best_clkhi);
	if (best_clkhi < 2)
		val |= LPI2C_MCCR0_CLKLO(3) | LPI2C_MCCR0_SETHOLD(2) | LPI2C_MCCR0_DATAVD(1);
	else
		val |= LPI2C_MCCR0_CLKLO(2 * best_clkhi) | LPI2C_MCCR0_SETHOLD(best_clkhi) |
			LPI2C_MCCR0_DATAVD(best_clkhi / 2);
	writel(val, &regs->mccr0);

	for (i = 0; i < 8; i++) {
		if (best_pre == (1 << i)) {
			best_pre = i;
			break;
		}
	}

	val = readl(&regs->mcfgr1) & ~LPI2C_MCFGR1_PRESCALE_MASK;
	writel(val | LPI2C_MCFGR1_PRESCALE(best_pre), &regs->mcfgr1);

	if (mode) {
		val = readl(&regs->mcr) & ~LPI2C_MCR_MEN_MASK;
		writel(val | LPI2C_MCR_MEN(1), &regs->mcr);
	}

	return 0;
}

static int bus_i2c_init(struct udevice *bus, int speed)
{
	struct imx_lpi2c_reg *regs;
	u32 val;
	int ret;

	regs = (struct imx_lpi2c_reg *)devfdt_get_addr(bus);
	/* reset peripheral */
	writel(LPI2C_MCR_RST_MASK, &regs->mcr);
	writel(0x0, &regs->mcr);
	/* Disable Dozen mode */
	writel(LPI2C_MCR_DBGEN(0) | LPI2C_MCR_DOZEN(1), &regs->mcr);
	/* host request disable, active high, external pin */
	val = readl(&regs->mcfgr0);
	val &= (~(LPI2C_MCFGR0_HREN_MASK | LPI2C_MCFGR0_HRPOL_MASK |
				LPI2C_MCFGR0_HRSEL_MASK));
	val |= LPI2C_MCFGR0_HRPOL(0x1);
	writel(val, &regs->mcfgr0);
	/* pincfg and ignore ack */
	val = readl(&regs->mcfgr1);
	val &= ~(LPI2C_MCFGR1_PINCFG_MASK | LPI2C_MCFGR1_IGNACK_MASK);
	val |= LPI2C_MCFGR1_PINCFG(0x0); /* 2 pin open drain */
	val |= LPI2C_MCFGR1_IGNACK(0x0); /* ignore nack */
	writel(val, &regs->mcfgr1);

	ret = bus_i2c_set_bus_speed(bus, speed);

	/* enable lpi2c in master mode */
	val = readl(&regs->mcr) & ~LPI2C_MCR_MEN_MASK;
	writel(val | LPI2C_MCR_MEN(1), &regs->mcr);

	debug("i2c : controller bus %d, speed %d:\n", bus->seq, speed);

	return ret;
}

static int imx_lpi2c_probe_chip(struct udevice *bus, u32 chip,
				u32 chip_flags)
{
	lpi2c_status_t result;

	result = bus_i2c_start(bus, chip, 0);
	if (result) {
		bus_i2c_stop(bus);
		bus_i2c_init(bus, 100000);
		return result;
	}

	result = bus_i2c_stop(bus);
	if (result)
		bus_i2c_init(bus, 100000);

	return result;
}

static int imx_lpi2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	int ret = 0, ret_stop;

	for (; nmsgs > 0; nmsgs--, msg++) {
		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD)
			ret = bus_i2c_read(bus, msg->addr, msg->buf, msg->len);
		else {
			ret = bus_i2c_write(bus, msg->addr, msg->buf,
					    msg->len);
			if (ret)
				break;
		}
	}

	if (ret)
		debug("i2c_write: error sending\n");

	ret_stop = bus_i2c_stop(bus);
	if (ret_stop)
		debug("i2c_xfer: stop bus error\n");

	ret |= ret_stop;

	return ret;
}

static int imx_lpi2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	return bus_i2c_set_bus_speed(bus, speed);
}

__weak int enable_i2c_clk(unsigned char enable, unsigned int i2c_num)
{
	return 0;
}

static int imx_lpi2c_probe(struct udevice *bus)
{
	struct imx_lpi2c_bus *i2c_bus = dev_get_priv(bus);
	fdt_addr_t addr;
	int ret;

	i2c_bus->driver_data = dev_get_driver_data(bus);

	addr = devfdt_get_addr(bus);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	i2c_bus->base = addr;
	i2c_bus->index = bus->seq;
	i2c_bus->bus = bus;

	/* power up i2c resource */
	ret = init_i2c_power(bus->seq);
	if (ret) {
		debug("init_i2c_power err = %d\n", ret);
		return ret;
	}

	if (IS_ENABLED(CONFIG_CLK)) {
		ret = clk_get_by_name(bus, "per", &i2c_bus->per_clk);
		if (ret) {
			dev_err(bus, "Failed to get per clk\n");
			return ret;
		}
		ret = clk_enable(&i2c_bus->per_clk);
		if (ret) {
			dev_err(bus, "Failed to enable per clk\n");
			return ret;
		}
	} else {
		/* To i.MX7ULP, only i2c4-7 can be handled by A7 core */
		ret = enable_i2c_clk(1, bus->seq);
		if (ret < 0)
			return ret;
	}

	ret = bus_i2c_init(bus, 100000);
	if (ret < 0)
		return ret;

	debug("i2c : controller bus %d at 0x%lx , speed %d: ",
	      bus->seq, i2c_bus->base,
	      i2c_bus->speed);

	return 0;
}

static const struct dm_i2c_ops imx_lpi2c_ops = {
	.xfer		= imx_lpi2c_xfer,
	.probe_chip	= imx_lpi2c_probe_chip,
	.set_bus_speed	= imx_lpi2c_set_bus_speed,
};

static const struct udevice_id imx_lpi2c_ids[] = {
	{ .compatible = "fsl,imx7ulp-lpi2c", },
	{ .compatible = "fsl,imx8qm-lpi2c", },
	{}
};

U_BOOT_DRIVER(imx_lpi2c) = {
	.name = "imx_lpi2c",
	.id = UCLASS_I2C,
	.of_match = imx_lpi2c_ids,
	.probe = imx_lpi2c_probe,
	.priv_auto_alloc_size = sizeof(struct imx_lpi2c_bus),
	.ops = &imx_lpi2c_ops,
};
