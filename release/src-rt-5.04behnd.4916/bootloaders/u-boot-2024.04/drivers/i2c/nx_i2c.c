#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <asm/arch/nexell.h>
#include <asm/arch/reset.h>
#include <asm/arch/clk.h>
#include <asm/arch/nx_gpio.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#define I2C_WRITE       0
#define I2C_READ        1

#define I2CSTAT_MTM     0xC0    /* Master Transmit Mode */
#define I2CSTAT_MRM     0x80    /* Master Receive Mode */
#define I2CSTAT_BSY     0x20    /* Read: Bus Busy */
#define I2CSTAT_SS      0x20    /* Write: START (1) / STOP (0) */
#define I2CSTAT_RXTXEN  0x10    /* Rx/Tx enable */
#define I2CSTAT_ABT	0x08	/* Arbitration bit */
#define I2CSTAT_NACK    0x01    /* Nack bit */
#define I2CCON_IRCLR    0x100   /* Interrupt Clear bit  */
#define I2CCON_ACKGEN   0x80    /* Acknowledge generation */
#define I2CCON_TCP256	0x40    /* Tx-clock prescaler: 16 (0) / 256 (1) */
#define I2CCON_IRENB	0x20	/* Interrupt Enable bit  */
#define I2CCON_IRPND    0x10    /* Interrupt pending bit */
#define I2CCON_TCDMSK	0x0F    /* I2C-bus transmit clock divider bit mask */

#ifdef CONFIG_ARCH_S5P6818
#define SDADLY_CLKSTEP	5	/* SDA delay: Reg. val. is multiple of 5 clks */
#define SDADLY_MAX	3	/* SDA delay: Max. reg. value is 3 */
#define I2CLC_FILTER	0x04	/* SDA filter on */
#else
#define STOPCON_CLR	0x01	/* Clock Line Release */
#define STOPCON_DLR	0x02	/* Data Line Release */
#define STOPCON_NAG	0x04	/* not-ackn. generation and data shift cont. */
#endif

#define I2C_TIMEOUT_MS	10      /* 10 ms */

#define I2C_M_NOSTOP	0x100

#define MAX_I2C_NUM 3

#define DEFAULT_SPEED   100000  /* default I2C speed [Hz] */

DECLARE_GLOBAL_DATA_PTR;

struct nx_i2c_regs {
	uint     iiccon;
	uint     iicstat;
	uint     iicadd;
	uint     iicds;
#ifdef CONFIG_ARCH_S5P6818
	/* S5P6818: Offset 0x10 is Line Control Register (SDA-delay, Filter) */
	uint     iiclc;
#else
	/* S5P4418: Offset 0x10 is Stop Control Register */
	uint     iicstopcon;
#endif
};

struct nx_i2c_bus {
	uint bus_num;
	struct nx_i2c_regs *regs;
	uint speed;
	uint target_speed;
#ifdef CONFIG_ARCH_S5P6818
	uint sda_delay;
#else
	/* setup time for Stop condition [us] */
	uint tsu_stop;
#endif
};

/* s5pxx18 i2c must be reset before enabled */
static void i2c_reset(int ch)
{
	int rst_id = RESET_ID_I2C0 + ch;

	nx_rstcon_setrst(rst_id, 0);
	nx_rstcon_setrst(rst_id, 1);
}

static uint i2c_get_clkrate(struct nx_i2c_bus *bus)
{
	struct clk *clk;
	int index = bus->bus_num;
	char name[50] = {0, };

	sprintf(name, "%s.%d", DEV_NAME_I2C, index);
	clk = clk_get((const char *)name);
	if (!clk)
		return -1;

	return clk_get_rate(clk);
}

static uint i2c_set_clk(struct nx_i2c_bus *bus, uint enb)
{
	struct clk *clk;
	char name[50];

	sprintf(name, "%s.%d", DEV_NAME_I2C, bus->bus_num);
	clk = clk_get((const char *)name);
	if (!clk) {
		debug("%s(): clk_get(%s) error!\n",
		      __func__, (const char *)name);
		return -EINVAL;
	}

	clk_disable(clk);
	if (enb)
		clk_enable(clk);

	return 0;
}

#ifdef CONFIG_ARCH_S5P6818
/* Set SDA line delay, not available at S5P4418 */
static int nx_i2c_set_sda_delay(struct nx_i2c_bus *bus)
{
	struct nx_i2c_regs *i2c = bus->regs;
	uint pclk = 0;
	uint t_pclk = 0;
	uint delay = 0;

	/* get input clock of the I2C-controller */
	pclk = i2c_get_clkrate(bus);

	if (bus->sda_delay) {
		/* t_pclk = period time of one pclk [ns] */
		t_pclk = DIV_ROUND_UP(1000, pclk / 1000000);
		/* delay = number of pclks required for sda_delay [ns] */
		delay = DIV_ROUND_UP(bus->sda_delay, t_pclk);
		/* delay = register value (step of 5 clocks) */
		delay = DIV_ROUND_UP(delay, SDADLY_CLKSTEP);
		/* max. possible register value = 3 */
		if (delay > SDADLY_MAX) {
			delay = SDADLY_MAX;
			debug("%s(): sda-delay des.: %dns, sat. to max.: %dns (granularity: %dns)\n",
			      __func__, bus->sda_delay, t_pclk * delay * SDADLY_CLKSTEP,
			      t_pclk * SDADLY_CLKSTEP);
		} else {
			debug("%s(): sda-delay des.: %dns, act.: %dns (granularity: %dns)\n",
			      __func__, bus->sda_delay, t_pclk * delay * SDADLY_CLKSTEP,
			      t_pclk * SDADLY_CLKSTEP);
		}

		delay |= I2CLC_FILTER;
	} else {
		delay = 0;
		debug("%s(): sda-delay = 0\n", __func__);
	}

	delay &= 0x7;
	writel(delay, &i2c->iiclc);

	return 0;
}
#endif

static int nx_i2c_set_bus_speed(struct udevice *dev, uint speed)
{
	struct nx_i2c_bus *bus = dev_get_priv(dev);
	struct nx_i2c_regs *i2c = bus->regs;
	unsigned long pclk, pres = 16, div;

	if (i2c_set_clk(bus, 1))
		return -EINVAL;

	/* get input clock of the I2C-controller */
	pclk = i2c_get_clkrate(bus);

	/* calculate prescaler and divisor values */
	if ((pclk / pres / (16 + 1)) > speed)
		/* prescaler value 16 is too less --> set to 256 */
		pres = 256;

	div = 0;
	/* actual divider = div + 1 */
	while ((pclk / pres / (div + 1)) > speed)
		div++;

	if (div > 0xF) {
		debug("%s(): pres==%ld, div==0x%lx is saturated to 0xF !)\n",
		      __func__, pres, div);
		div = 0xF;
	} else {
		debug("%s(): pres==%ld, div==0x%lx)\n", __func__, pres, div);
	}

	/* set Tx-clock divisor and prescaler values */
	writel((div & I2CCON_TCDMSK) | ((pres == 256) ? I2CCON_TCP256 : 0),
	       &i2c->iiccon);

	/* init to SLAVE REVEIVE and set slaveaddr */
	writel(0, &i2c->iicstat);
	writel(0x00, &i2c->iicadd);

	/* program Master Transmit (and implicit STOP) */
	writel(I2CSTAT_MTM | I2CSTAT_RXTXEN, &i2c->iicstat);

	/* calculate actual I2C speed [Hz] */
	bus->speed = pclk / ((div + 1) * pres);
	debug("%s(): speed des.: %dHz, act.: %dHz\n",
	      __func__, speed, bus->speed);

#ifdef CONFIG_ARCH_S5P6818
	nx_i2c_set_sda_delay(bus);
#else
	/* setup time for Stop condition [us], min. 4us @ 100kHz I2C-clock */
	bus->tsu_stop = DIV_ROUND_UP(400, bus->speed / 1000);
#endif

	if (i2c_set_clk(bus, 0))
		return -EINVAL;
	return 0;
}

static void i2c_process_node(struct udevice *dev)
{
	struct nx_i2c_bus *bus = dev_get_priv(dev);

	bus->target_speed = dev_read_s32_default(dev, "clock-frequency",
						 DEFAULT_SPEED);
#ifdef CONFIG_ARCH_S5P6818
	bus->sda_delay = dev_read_s32_default(dev, "i2c-sda-delay-ns", 0);
#endif
}

static int nx_i2c_probe(struct udevice *dev)
{
	struct nx_i2c_bus *bus = dev_get_priv(dev);
	fdt_addr_t addr;

	/* get regs = i2c base address */
	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	bus->regs = (struct nx_i2c_regs *)addr;

	bus->bus_num = dev_seq(dev);

	/* i2c node parsing */
	i2c_process_node(dev);
	if (!bus->target_speed)
		return -ENODEV;

	/* reset */
	i2c_reset(bus->bus_num);

	return 0;
}

/* i2c bus busy check */
static int i2c_is_busy(struct nx_i2c_regs *i2c)
{
	ulong start_time;

	start_time = get_timer(0);
	while (readl(&i2c->iicstat) & I2CSTAT_BSY) {
		if (get_timer(start_time) > I2C_TIMEOUT_MS) {
			debug("Timeout\n");
			return -EBUSY;
		}
	}
	return 0;
}

/* irq enable/disable functions */
static void i2c_enable_irq(struct nx_i2c_regs *i2c)
{
	unsigned int reg;

	reg = readl(&i2c->iiccon);
	reg |= I2CCON_IRENB;
	writel(reg, &i2c->iiccon);
}

/* irq clear function */
static void i2c_clear_irq(struct nx_i2c_regs *i2c)
{
	unsigned int reg;

	reg = readl(&i2c->iiccon);
	/* reset interrupt pending flag */
	reg &= ~(I2CCON_IRPND);
	/*
	 * Interrupt must also be cleared!
	 * Otherwise linux boot may hang after:
	 *     [    0.436000] NetLabel:  unlabeled traffic allowed by default
	 * Next would be:
	 *     [    0.442000] clocksource: Switched to clocksource source timer
	 */
	reg |= I2CCON_IRCLR;
	writel(reg, &i2c->iiccon);
}

/* ack enable functions */
static void i2c_enable_ack(struct nx_i2c_regs *i2c)
{
	unsigned int reg;

	reg = readl(&i2c->iiccon);
	reg |= I2CCON_ACKGEN;
	writel(reg, &i2c->iiccon);
}

static void i2c_send_stop(struct nx_i2c_bus *bus)
{
	struct nx_i2c_regs *i2c = bus->regs;

	if (IS_ENABLED(CONFIG_ARCH_S5P6818)) {
		unsigned int reg;

		reg = readl(&i2c->iicstat);
		reg |= I2CSTAT_MRM | I2CSTAT_RXTXEN;
		reg &= (~I2CSTAT_SS);

		writel(reg, &i2c->iicstat);
		i2c_clear_irq(i2c);
	} else {  /* S5P4418 */
		writel(STOPCON_NAG, &i2c->iicstopcon);

		i2c_clear_irq(i2c);

		/*
		 * Clock Line Release --> SDC changes from Low to High and
		 * SDA from High to Low
		 */
		writel(STOPCON_CLR, &i2c->iicstopcon);

		/* Hold SDA Low (Setup Time for Stop condition) */
		udelay(bus->tsu_stop);

		i2c_clear_irq(i2c);

		/* Master Receive Mode Stop --> SDA becomes High */
		writel(I2CSTAT_MRM, &i2c->iicstat);
	}
}

static int wait_for_xfer(struct nx_i2c_regs *i2c)
{
	unsigned long start_time = get_timer(0);

	do {
		if (readl(&i2c->iiccon) & I2CCON_IRPND)
			/* return -EREMOTEIO if not Acknowledged, otherwise 0 */
			return (readl(&i2c->iicstat) & I2CSTAT_NACK) ?
				-EREMOTEIO : 0;
	} while (get_timer(start_time) < I2C_TIMEOUT_MS);

	return -ETIMEDOUT;
}

static int i2c_transfer(struct nx_i2c_regs *i2c,
			uchar cmd_type,
			uchar chip_addr,
			uchar addr[],
			uchar addr_len,
			uchar data[],
			unsigned short data_len,
			uint seq)
{
	uint status;
	int i = 0, result;

	/* Note: data_len = 0 is supported for "probe_chip" */

	i2c_enable_irq(i2c);
	i2c_enable_ack(i2c);

	/* Get the slave chip address going */
	/* Enable Rx/Tx */
	writel(I2CSTAT_RXTXEN, &i2c->iicstat);

	writel(chip_addr, &i2c->iicds);
	status = I2CSTAT_RXTXEN | I2CSTAT_SS;
	if (cmd_type == I2C_WRITE || (addr && addr_len))
		status |= I2CSTAT_MTM;
	else
		status |= I2CSTAT_MRM;

	writel(status, &i2c->iicstat);
	if (seq)
		i2c_clear_irq(i2c);

	/* Wait for chip address to transmit. */
	result = wait_for_xfer(i2c);
	if (result) {
		debug("%s: transmitting chip address failed\n", __func__);
		goto bailout;
	}

	/* If register address needs to be transmitted - do it now. */
	if (addr && addr_len) {  /* register addr */
		while ((i < addr_len) && !result) {
			writel(addr[i++], &i2c->iicds);
			i2c_clear_irq(i2c);
			result = wait_for_xfer(i2c);
		}

		i = 0;
		if (result) {
			debug("%s: transmitting register address failed\n",
			      __func__);
			goto bailout;
		}
	}

	switch (cmd_type) {
	case I2C_WRITE:
		while ((i < data_len) && !result) {
			writel(data[i++], &i2c->iicds);
			i2c_clear_irq(i2c);
			result = wait_for_xfer(i2c);
		}
		break;
	case I2C_READ:
		if (addr && addr_len) {
			/*
			 * Register address has been sent, now send slave chip
			 * address again to start the actual read transaction.
			 */
			writel(chip_addr, &i2c->iicds);

			/* Generate a re-START. */
			writel(I2CSTAT_MRM | I2CSTAT_RXTXEN |
			       I2CSTAT_SS, &i2c->iicstat);
			i2c_clear_irq(i2c);
			result = wait_for_xfer(i2c);
			if (result) {
				debug("%s: I2C_READ: sending chip addr. failed\n",
				      __func__);
				goto bailout;
			}
		}

		while ((i < data_len) && !result) {
			/* disable ACK for final READ */
			if (i == data_len - 1)
				clrbits_le32(&i2c->iiccon, I2CCON_ACKGEN);

			i2c_clear_irq(i2c);
			result = wait_for_xfer(i2c);
			data[i++] = readb(&i2c->iicds);
		}

		if (result == -EREMOTEIO)
			 /* Not Acknowledged --> normal terminated read. */
			result = 0;
		else if (result == -ETIMEDOUT)
			debug("%s: I2C_READ: time out\n", __func__);
		else
			debug("%s: I2C_READ: read not terminated with NACK\n",
			      __func__);
		break;

	default:
		debug("%s: bad call\n", __func__);
		result = -EINVAL;
		break;
	}

bailout:
	return result;
}

static int nx_i2c_read(struct udevice *dev, uchar chip_addr, uint addr,
		       uint alen, uchar *buffer, uint len, uint seq)
{
	struct nx_i2c_bus *i2c;
	uchar xaddr[4];
	int ret;

	i2c = dev_get_priv(dev);
	if (!i2c)
		return -EFAULT;

	if (alen > 4) {
		debug("I2C read: addr len %d not supported\n", alen);
		return -EADDRNOTAVAIL;
	}

	if (alen > 0)
		xaddr[0] = (addr >> 24) & 0xFF;

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}

	ret = i2c_transfer(i2c->regs, I2C_READ, chip_addr << 1,
			   &xaddr[4 - alen], alen, buffer, len, seq);

	if (ret) {
		debug("I2C read failed %d\n", ret);
		return -EIO;
	}

	return 0;
}

static int nx_i2c_write(struct udevice *dev, uchar chip_addr, uint addr,
			uint alen, uchar *buffer, uint len, uint seq)
{
	struct nx_i2c_bus *i2c;
	uchar xaddr[4];
	int ret;

	i2c = dev_get_priv(dev);
	if (!i2c)
		return -EFAULT;

	if (alen > 4) {
		debug("I2C write: addr len %d not supported\n", alen);
		return -EINVAL;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}

	ret = i2c_transfer(i2c->regs, I2C_WRITE, chip_addr << 1,
			   &xaddr[4 - alen], alen, buffer, len, seq);
	if (ret) {
		debug("I2C write failed %d\n", ret);
		return -EIO;
	}

	return 0;
}

static int nx_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct nx_i2c_bus *bus = dev_get_priv(dev);
	struct nx_i2c_regs *i2c = bus->regs;
	int ret;
	int i;

	/* The power loss by the clock, only during on/off. */
	ret = i2c_set_clk(bus, 1);

	if (!ret)
		/* Bus State(Busy) check  */
		ret = i2c_is_busy(i2c);
	if (!ret) {
		for (i = 0; i < nmsgs; msg++, i++) {
			if (msg->flags & I2C_M_RD) {
				ret = nx_i2c_read(dev, msg->addr, 0, 0,
						  msg->buf, msg->len, i);
			} else {
				ret = nx_i2c_write(dev, msg->addr, 0, 0,
						   msg->buf, msg->len, i);
			}

			if (ret) {
				debug("i2c_xfer: error sending\n");
				ret = -EREMOTEIO;
			}
		}

		i2c_send_stop(bus);
		if (i2c_set_clk(bus, 0))
			ret = -EINVAL;
	}

	return ret;
};

static int nx_i2c_probe_chip(struct udevice *dev, u32 chip_addr,
			     u32 chip_flags)
{
	int ret;
	struct nx_i2c_bus *bus = dev_get_priv(dev);

	ret = i2c_set_clk(bus, 1);

	if (!ret) {
		/*
		 * Send Chip Address only
		 * --> I2C transfer with data length and address length = 0.
		 * If there is a Slave, i2c_transfer() returns 0 (acknowledge
		 * transfer).
		 * I2C_WRITE must be used in order Master Transmit Mode is
		 * selected. Otherwise (in Master Receive Mode, I2C_READ)
		 * sending the stop condition below is not working (SDA does
		 * not transit to High).
		 */
		ret = i2c_transfer(bus->regs, I2C_WRITE, (uchar)chip_addr << 1,
				   NULL, 0, NULL, 0, 0);

		i2c_send_stop(bus);
		if (i2c_set_clk(bus, 0))
			ret = -EINVAL;
	}

	return ret;
}

static const struct dm_i2c_ops nx_i2c_ops = {
	.xfer		= nx_i2c_xfer,
	.probe_chip	= nx_i2c_probe_chip,
	.set_bus_speed	= nx_i2c_set_bus_speed,
};

static const struct udevice_id nx_i2c_ids[] = {
	{ .compatible = "nexell,s5pxx18-i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_nexell) = {
	.name		= "i2c_nexell",
	.id		= UCLASS_I2C,
	.of_match	= nx_i2c_ids,
	.probe		= nx_i2c_probe,
	.priv_auto	= sizeof(struct nx_i2c_bus),
	.ops		= &nx_i2c_ops,
};
