// SPDX-License-Identifier: GPL-2.0+
/*
 * i2c driver for Freescale i.MX series
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 * (c) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on i2c-imx.c from linux kernel:
 *  Copyright (C) 2005 Torsten Koschorrek <koschorrek at synertronixx.de>
 *  Copyright (C) 2005 Matthias Blaschke <blaschke at synertronixx.de>
 *  Copyright (C) 2007 RightHand Technologies, Inc.
 *  Copyright (C) 2008 Darius Augulis <darius.augulis at teltonika.lt>
 *
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <linux/errno.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <i2c.h>
#include <watchdog.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

#define I2C_QUIRK_FLAG		(1 << 0)

#define IMX_I2C_REGSHIFT	2
#define VF610_I2C_REGSHIFT	0

#define I2C_EARLY_INIT_INDEX		0
#ifdef CONFIG_SYS_I2C_IFDR_DIV
#define I2C_IFDR_DIV_CONSERVATIVE	CONFIG_SYS_I2C_IFDR_DIV
#else
#define I2C_IFDR_DIV_CONSERVATIVE	0x7e
#endif

/* Register index */
#define IADR	0
#define IFDR	1
#define I2CR	2
#define I2SR	3
#define I2DR	4

#define I2CR_IIEN	(1 << 6)
#define I2CR_MSTA	(1 << 5)
#define I2CR_MTX	(1 << 4)
#define I2CR_TX_NO_AK	(1 << 3)
#define I2CR_RSTA	(1 << 2)

#define I2SR_ICF	(1 << 7)
#define I2SR_IBB	(1 << 5)
#define I2SR_IAL	(1 << 4)
#define I2SR_IIF	(1 << 1)
#define I2SR_RX_NO_AK	(1 << 0)

#ifdef I2C_QUIRK_REG
#define I2CR_IEN	(0 << 7)
#define I2CR_IDIS	(1 << 7)
#define I2SR_IIF_CLEAR	(1 << 1)
#else
#define I2CR_IEN	(1 << 7)
#define I2CR_IDIS	(0 << 7)
#define I2SR_IIF_CLEAR	(0 << 1)
#endif

#ifdef I2C_QUIRK_REG
static u16 i2c_clk_div[60][2] = {
	{ 20,	0x00 }, { 22,	0x01 }, { 24,	0x02 }, { 26,	0x03 },
	{ 28,	0x04 },	{ 30,	0x05 },	{ 32,	0x09 }, { 34,	0x06 },
	{ 36,	0x0A }, { 40,	0x07 }, { 44,	0x0C }, { 48,	0x0D },
	{ 52,	0x43 },	{ 56,	0x0E }, { 60,	0x45 }, { 64,	0x12 },
	{ 68,	0x0F },	{ 72,	0x13 },	{ 80,	0x14 },	{ 88,	0x15 },
	{ 96,	0x19 },	{ 104,	0x16 },	{ 112,	0x1A },	{ 128,	0x17 },
	{ 136,	0x4F }, { 144,	0x1C },	{ 160,	0x1D }, { 176,	0x55 },
	{ 192,	0x1E }, { 208,	0x56 },	{ 224,	0x22 }, { 228,	0x24 },
	{ 240,	0x1F },	{ 256,	0x23 }, { 288,	0x5C },	{ 320,	0x25 },
	{ 384,	0x26 }, { 448,	0x2A },	{ 480,	0x27 }, { 512,	0x2B },
	{ 576,	0x2C },	{ 640,	0x2D },	{ 768,	0x31 }, { 896,	0x32 },
	{ 960,	0x2F },	{ 1024,	0x33 },	{ 1152,	0x34 }, { 1280,	0x35 },
	{ 1536,	0x36 }, { 1792,	0x3A },	{ 1920,	0x37 },	{ 2048,	0x3B },
	{ 2304,	0x3C },	{ 2560,	0x3D },	{ 3072,	0x3E }, { 3584,	0x7A },
	{ 3840,	0x3F }, { 4096,	0x7B }, { 5120,	0x7D },	{ 6144,	0x7E },
};
#else
static u16 i2c_clk_div[50][2] = {
	{ 22,	0x20 }, { 24,	0x21 }, { 26,	0x22 }, { 28,	0x23 },
	{ 30,	0x00 }, { 32,	0x24 }, { 36,	0x25 }, { 40,	0x26 },
	{ 42,	0x03 }, { 44,	0x27 }, { 48,	0x28 }, { 52,	0x05 },
	{ 56,	0x29 }, { 60,	0x06 }, { 64,	0x2A }, { 72,	0x2B },
	{ 80,	0x2C }, { 88,	0x09 }, { 96,	0x2D }, { 104,	0x0A },
	{ 112,	0x2E }, { 128,	0x2F }, { 144,	0x0C }, { 160,	0x30 },
	{ 192,	0x31 }, { 224,	0x32 }, { 240,	0x0F }, { 256,	0x33 },
	{ 288,	0x10 }, { 320,	0x34 }, { 384,	0x35 }, { 448,	0x36 },
	{ 480,	0x13 }, { 512,	0x37 }, { 576,	0x14 }, { 640,	0x38 },
	{ 768,	0x39 }, { 896,	0x3A }, { 960,	0x17 }, { 1024,	0x3B },
	{ 1152,	0x18 }, { 1280,	0x3C }, { 1536,	0x3D }, { 1792,	0x3E },
	{ 1920,	0x1B }, { 2048,	0x3F }, { 2304,	0x1C }, { 2560,	0x1D },
	{ 3072,	0x1E }, { 3840,	0x1F }
};
#endif

#ifndef CONFIG_SYS_MXC_I2C1_SPEED
#define CONFIG_SYS_MXC_I2C1_SPEED 100000
#endif
#ifndef CONFIG_SYS_MXC_I2C2_SPEED
#define CONFIG_SYS_MXC_I2C2_SPEED 100000
#endif
#ifndef CONFIG_SYS_MXC_I2C3_SPEED
#define CONFIG_SYS_MXC_I2C3_SPEED 100000
#endif
#ifndef CONFIG_SYS_MXC_I2C4_SPEED
#define CONFIG_SYS_MXC_I2C4_SPEED 100000
#endif

#ifndef CONFIG_SYS_MXC_I2C1_SLAVE
#define CONFIG_SYS_MXC_I2C1_SLAVE 0
#endif
#ifndef CONFIG_SYS_MXC_I2C2_SLAVE
#define CONFIG_SYS_MXC_I2C2_SLAVE 0
#endif
#ifndef CONFIG_SYS_MXC_I2C3_SLAVE
#define CONFIG_SYS_MXC_I2C3_SLAVE 0
#endif
#ifndef CONFIG_SYS_MXC_I2C4_SLAVE
#define CONFIG_SYS_MXC_I2C4_SLAVE 0
#endif

/*
 * Calculate and set proper clock divider
 */
static uint8_t i2c_imx_get_clk(struct mxc_i2c_bus *i2c_bus, unsigned int rate)
{
	unsigned int i2c_clk_rate;
	unsigned int div;
	u8 clk_div;

#if defined(CONFIG_MX31)
	struct clock_control_regs *sc_regs =
		(struct clock_control_regs *)CCM_BASE;

	/* start the required I2C clock */
	writel(readl(&sc_regs->cgr0) | (3 << CONFIG_SYS_I2C_CLK_OFFSET),
		&sc_regs->cgr0);
#endif

	/* Divider value calculation */
	i2c_clk_rate = mxc_get_clock(MXC_I2C_CLK);
	div = (i2c_clk_rate + rate - 1) / rate;
	if (div < i2c_clk_div[0][0])
		clk_div = 0;
	else if (div > i2c_clk_div[ARRAY_SIZE(i2c_clk_div) - 1][0])
		clk_div = ARRAY_SIZE(i2c_clk_div) - 1;
	else
		for (clk_div = 0; i2c_clk_div[clk_div][0] < div; clk_div++)
			;

	/* Store divider value */
	return clk_div;
}

/*
 * Set I2C Bus speed
 */
static int bus_i2c_set_bus_speed(struct mxc_i2c_bus *i2c_bus, int speed)
{
	ulong base = i2c_bus->base;
	bool quirk = i2c_bus->driver_data & I2C_QUIRK_FLAG ? true : false;
	u8 clk_idx = i2c_imx_get_clk(i2c_bus, speed);
	u8 idx = i2c_clk_div[clk_idx][1];
	int reg_shift = quirk ? VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;

	if (!base)
		return -EINVAL;

	/* Store divider value */
	writeb(idx, base + (IFDR << reg_shift));

	/* Reset module */
	writeb(I2CR_IDIS, base + (I2CR << reg_shift));
	writeb(0, base + (I2SR << reg_shift));
	return 0;
}

#define ST_BUS_IDLE (0 | (I2SR_IBB << 8))
#define ST_BUS_BUSY (I2SR_IBB | (I2SR_IBB << 8))
#define ST_IIF (I2SR_IIF | (I2SR_IIF << 8))

static int wait_for_sr_state(struct mxc_i2c_bus *i2c_bus, unsigned state)
{
	unsigned sr;
	ulong elapsed;
	bool quirk = i2c_bus->driver_data & I2C_QUIRK_FLAG ? true : false;
	int reg_shift = quirk ? VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;
	ulong base = i2c_bus->base;
	ulong start_time = get_timer(0);
	for (;;) {
		sr = readb(base + (I2SR << reg_shift));
		if (sr & I2SR_IAL) {
			if (quirk)
				writeb(sr | I2SR_IAL, base +
				       (I2SR << reg_shift));
			else
				writeb(sr & ~I2SR_IAL, base +
				       (I2SR << reg_shift));
			printf("%s: Arbitration lost sr=%x cr=%x state=%x\n",
				__func__, sr, readb(base + (I2CR << reg_shift)),
				state);
			return -ERESTART;
		}
		if ((sr & (state >> 8)) == (unsigned char)state)
			return sr;
		WATCHDOG_RESET();
		elapsed = get_timer(start_time);
		if (elapsed > (CONFIG_SYS_HZ / 10))	/* .1 seconds */
			break;
	}
	printf("%s: failed sr=%x cr=%x state=%x\n", __func__,
	       sr, readb(base + (I2CR << reg_shift)), state);
	return -ETIMEDOUT;
}

static int tx_byte(struct mxc_i2c_bus *i2c_bus, u8 byte)
{
	int ret;
	int reg_shift = i2c_bus->driver_data & I2C_QUIRK_FLAG ?
			VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;
	ulong base = i2c_bus->base;

	writeb(I2SR_IIF_CLEAR, base + (I2SR << reg_shift));
	writeb(byte, base + (I2DR << reg_shift));

	ret = wait_for_sr_state(i2c_bus, ST_IIF);
	if (ret < 0)
		return ret;
	if (ret & I2SR_RX_NO_AK)
		return -EREMOTEIO;
	return 0;
}

/*
 * Stub implementations for outer i2c slave operations.
 */
void __i2c_force_reset_slave(void)
{
}
void i2c_force_reset_slave(void)
	__attribute__((weak, alias("__i2c_force_reset_slave")));

/*
 * Stop I2C transaction
 */
static void i2c_imx_stop(struct mxc_i2c_bus *i2c_bus)
{
	int ret;
	int reg_shift = i2c_bus->driver_data & I2C_QUIRK_FLAG ?
			VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;
	ulong base = i2c_bus->base;
	unsigned int temp = readb(base + (I2CR << reg_shift));

	temp &= ~(I2CR_MSTA | I2CR_MTX);
	writeb(temp, base + (I2CR << reg_shift));
	ret = wait_for_sr_state(i2c_bus, ST_BUS_IDLE);
	if (ret < 0)
		printf("%s:trigger stop failed\n", __func__);
}

/*
 * Send start signal, chip address and
 * write register address
 */
static int i2c_init_transfer_(struct mxc_i2c_bus *i2c_bus, u8 chip,
			      u32 addr, int alen)
{
	unsigned int temp;
	int ret;
	bool quirk = i2c_bus->driver_data & I2C_QUIRK_FLAG ? true : false;
	ulong base = i2c_bus->base;
	int reg_shift = quirk ? VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;

	/* Reset i2c slave */
	i2c_force_reset_slave();

	/* Enable I2C controller */
	if (quirk)
		ret = readb(base + (I2CR << reg_shift)) & I2CR_IDIS;
	else
		ret = !(readb(base + (I2CR << reg_shift)) & I2CR_IEN);

	if (ret) {
		writeb(I2CR_IEN, base + (I2CR << reg_shift));
		/* Wait for controller to be stable */
		udelay(50);
	}

	if (readb(base + (IADR << reg_shift)) == (chip << 1))
		writeb((chip << 1) ^ 2, base + (IADR << reg_shift));
	writeb(I2SR_IIF_CLEAR, base + (I2SR << reg_shift));
	ret = wait_for_sr_state(i2c_bus, ST_BUS_IDLE);
	if (ret < 0)
		return ret;

	/* Start I2C transaction */
	temp = readb(base + (I2CR << reg_shift));
	temp |= I2CR_MSTA;
	writeb(temp, base + (I2CR << reg_shift));

	ret = wait_for_sr_state(i2c_bus, ST_BUS_BUSY);
	if (ret < 0)
		return ret;

	temp |= I2CR_MTX | I2CR_TX_NO_AK;
	writeb(temp, base + (I2CR << reg_shift));

	if (alen >= 0)	{
		/* write slave address */
		ret = tx_byte(i2c_bus, chip << 1);
		if (ret < 0)
			return ret;

		while (alen--) {
			ret = tx_byte(i2c_bus, (addr >> (alen * 8)) & 0xff);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

#ifndef CONFIG_DM_I2C
int i2c_idle_bus(struct mxc_i2c_bus *i2c_bus)
{
	if (i2c_bus && i2c_bus->idle_bus_fn)
		return i2c_bus->idle_bus_fn(i2c_bus->idle_bus_data);
	return 0;
}
#else
/*
 * See Linux Documentation/devicetree/bindings/i2c/i2c-imx.txt
 * "
 *  scl-gpios: specify the gpio related to SCL pin
 *  sda-gpios: specify the gpio related to SDA pin
 *  add pinctrl to configure i2c pins to gpio function for i2c
 *  bus recovery, call it "gpio" state
 * "
 *
 * The i2c_idle_bus is an implementation following Linux Kernel.
 */
int i2c_idle_bus(struct mxc_i2c_bus *i2c_bus)
{
	struct udevice *bus = i2c_bus->bus;
	struct dm_i2c_bus *i2c = dev_get_uclass_priv(bus);
	struct gpio_desc *scl_gpio = &i2c_bus->scl_gpio;
	struct gpio_desc *sda_gpio = &i2c_bus->sda_gpio;
	int sda, scl, idle_sclks;
	int i, ret = 0;
	ulong elapsed, start_time;

	if (pinctrl_select_state(bus, "gpio")) {
		dev_dbg(bus, "Can not to switch to use gpio pinmux\n");
		/*
		 * GPIO pinctrl for i2c force idle is not a must,
		 * but it is strongly recommended to be used.
		 * Because it can help you to recover from bad
		 * i2c bus state. Do not return failure, because
		 * it is not a must.
		 */
		return 0;
	}

	dm_gpio_set_dir_flags(scl_gpio, GPIOD_IS_IN);
	dm_gpio_set_dir_flags(sda_gpio, GPIOD_IS_IN);
	scl = dm_gpio_get_value(scl_gpio);
	sda = dm_gpio_get_value(sda_gpio);

	if ((sda & scl) == 1)
		goto exit;		/* Bus is idle already */

	/*
	 * In most cases it is just enough to generate 8 + 1 SCLK
	 * clocks to recover I2C slave device from 'stuck' state
	 * (when for example SW reset was performed, in the middle of
	 * I2C transmission).
	 *
	 * However, there are devices which send data in packets of
	 * N bytes (N > 1). In such case we do need N * 8 + 1 SCLK
	 * clocks.
	 */
	idle_sclks = 8 + 1;

	if (i2c->max_transaction_bytes > 0)
		idle_sclks = i2c->max_transaction_bytes * 8 + 1;
	/* Send high and low on the SCL line */
	for (i = 0; i < idle_sclks; i++) {
		dm_gpio_set_dir_flags(scl_gpio, GPIOD_IS_OUT);
		dm_gpio_set_value(scl_gpio, 0);
		udelay(50);
		dm_gpio_set_dir_flags(scl_gpio, GPIOD_IS_IN);
		udelay(50);
	}
	start_time = get_timer(0);
	for (;;) {
		dm_gpio_set_dir_flags(scl_gpio, GPIOD_IS_IN);
		dm_gpio_set_dir_flags(sda_gpio, GPIOD_IS_IN);
		scl = dm_gpio_get_value(scl_gpio);
		sda = dm_gpio_get_value(sda_gpio);
		if ((sda & scl) == 1)
			break;
		WATCHDOG_RESET();
		elapsed = get_timer(start_time);
		if (elapsed > (CONFIG_SYS_HZ / 5)) {	/* .2 seconds */
			ret = -EBUSY;
			printf("%s: failed to clear bus, sda=%d scl=%d\n", __func__, sda, scl);
			break;
		}
	}

exit:
	pinctrl_select_state(bus, "default");
	return ret;
}
#endif

static int i2c_init_transfer(struct mxc_i2c_bus *i2c_bus, u8 chip,
			     u32 addr, int alen)
{
	int retry;
	int ret;
	int reg_shift = i2c_bus->driver_data & I2C_QUIRK_FLAG ?
			VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;

	if (!i2c_bus->base)
		return -EINVAL;

	for (retry = 0; retry < 3; retry++) {
		ret = i2c_init_transfer_(i2c_bus, chip, addr, alen);
		if (ret >= 0)
			return 0;
		i2c_imx_stop(i2c_bus);
		if (ret == -EREMOTEIO)
			return ret;

		printf("%s: failed for chip 0x%x retry=%d\n", __func__, chip,
				retry);
		if (ret != -ERESTART)
			/* Disable controller */
			writeb(I2CR_IDIS, i2c_bus->base + (I2CR << reg_shift));
		udelay(100);
		if (i2c_idle_bus(i2c_bus) < 0)
			break;
	}
	printf("%s: give up i2c_regs=0x%lx\n", __func__, i2c_bus->base);
	return ret;
}


static int i2c_write_data(struct mxc_i2c_bus *i2c_bus, u8 chip, const u8 *buf,
			  int len)
{
	int i, ret = 0;

	debug("i2c_write_data: chip=0x%x, len=0x%x\n", chip, len);
	debug("write_data: ");
	/* use rc for counter */
	for (i = 0; i < len; ++i)
		debug(" 0x%02x", buf[i]);
	debug("\n");

	for (i = 0; i < len; i++) {
		ret = tx_byte(i2c_bus, buf[i]);
		if (ret < 0) {
			debug("i2c_write_data(): rc=%d\n", ret);
			break;
		}
	}

	return ret;
}

/* Will generate a STOP after the last byte if "last" is true, i.e. this is the
 * final message of a transaction.  If not, it switches the bus back to TX mode
 * and does not send a STOP, leaving the bus in a state where a repeated start
 * and address can be sent for another message.
 */
static int i2c_read_data(struct mxc_i2c_bus *i2c_bus, uchar chip, uchar *buf,
			 int len, bool last)
{
	int ret;
	unsigned int temp;
	int i;
	int reg_shift = i2c_bus->driver_data & I2C_QUIRK_FLAG ?
			VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;
	ulong base = i2c_bus->base;

	debug("i2c_read_data: chip=0x%x, len=0x%x\n", chip, len);

	/* setup bus to read data */
	temp = readb(base + (I2CR << reg_shift));
	temp &= ~(I2CR_MTX | I2CR_TX_NO_AK);
	if (len == 1)
		temp |= I2CR_TX_NO_AK;
	writeb(temp, base + (I2CR << reg_shift));
	writeb(I2SR_IIF_CLEAR, base + (I2SR << reg_shift));
	/* dummy read to clear ICF */
	readb(base + (I2DR << reg_shift));

	/* read data */
	for (i = 0; i < len; i++) {
		ret = wait_for_sr_state(i2c_bus, ST_IIF);
		if (ret < 0) {
			debug("i2c_read_data(): ret=%d\n", ret);
			i2c_imx_stop(i2c_bus);
			return ret;
		}

		if (i == (len - 1)) {
			/* Final byte has already been received by master!  When
			 * we read it from I2DR, the master will start another
			 * cycle.  We must program it first to send a STOP or
			 * switch to TX to avoid this.
			 */
			if (last) {
				i2c_imx_stop(i2c_bus);
			} else {
				/* Final read, no stop, switch back to tx */
				temp = readb(base + (I2CR << reg_shift));
				temp |= I2CR_MTX | I2CR_TX_NO_AK;
				writeb(temp, base + (I2CR << reg_shift));
			}
		} else if (i == (len - 2)) {
			/* Master has already recevied penultimate byte.  When
			 * we read it from I2DR, master will start RX of final
			 * byte.  We must set TX_NO_AK now so it does not ACK
			 * that final byte.
			 */
			temp = readb(base + (I2CR << reg_shift));
			temp |= I2CR_TX_NO_AK;
			writeb(temp, base + (I2CR << reg_shift));
		}

		writeb(I2SR_IIF_CLEAR, base + (I2SR << reg_shift));
		buf[i] = readb(base + (I2DR << reg_shift));
	}

	/* reuse ret for counter*/
	for (ret = 0; ret < len; ++ret)
		debug(" 0x%02x", buf[ret]);
	debug("\n");

	/* It is not clear to me that this is necessary */
	if (last)
		i2c_imx_stop(i2c_bus);
	return 0;
}

#ifndef CONFIG_DM_I2C
/*
 * Read data from I2C device
 *
 * The transactions use the syntax defined in the Linux kernel I2C docs.
 *
 * If alen is > 0, then this function will send a transaction of the form:
 *     S Chip Wr [A] Addr [A] S Chip Rd [A] [data] A ... NA P
 * This is a normal I2C register read: writing the register address, then doing
 * a repeated start and reading the data.
 *
 * If alen == 0, then we get this transaction:
 *     S Chip Wr [A] S Chip Rd [A] [data] A ... NA P
 * This is somewhat unusual, though valid, transaction.  It addresses the chip
 * in write mode, but doesn't actually write any register address or data, then
 * does a repeated start and reads data.
 *
 * If alen < 0, then we get this transaction:
 *     S Chip Rd [A] [data] A ... NA P
 * The chip is addressed in read mode and then data is read.  No register
 * address is written first.  This is perfectly valid on most devices and
 * required on some (usually those that don't act like an array of registers).
 */
static int bus_i2c_read(struct mxc_i2c_bus *i2c_bus, u8 chip, u32 addr,
			int alen, u8 *buf, int len)
{
	int ret = 0;
	u32 temp;
	int reg_shift = i2c_bus->driver_data & I2C_QUIRK_FLAG ?
		VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;
	ulong base = i2c_bus->base;

	ret = i2c_init_transfer(i2c_bus, chip, addr, alen);
	if (ret < 0)
		return ret;

	if (alen >= 0) {
		temp = readb(base + (I2CR << reg_shift));
		temp |= I2CR_RSTA;
		writeb(temp, base + (I2CR << reg_shift));
	}

	ret = tx_byte(i2c_bus, (chip << 1) | 1);
	if (ret < 0) {
		i2c_imx_stop(i2c_bus);
		return ret;
	}

	ret = i2c_read_data(i2c_bus, chip, buf, len, true);

	i2c_imx_stop(i2c_bus);
	return ret;
}

/*
 * Write data to I2C device
 *
 * If alen > 0, we get this transaction:
 *    S Chip Wr [A] addr [A] data [A] ... [A] P
 * An ordinary write register command.
 *
 * If alen == 0, then we get this:
 *    S Chip Wr [A] data [A] ... [A] P
 * This is a simple I2C write.
 *
 * If alen < 0, then we get this:
 *    S data [A] ... [A] P
 * This is most likely NOT something that should be used.  It doesn't send the
 * chip address first, so in effect, the first byte of data will be used as the
 * address.
 */
static int bus_i2c_write(struct mxc_i2c_bus *i2c_bus, u8 chip, u32 addr,
			 int alen, const u8 *buf, int len)
{
	int ret = 0;

	ret = i2c_init_transfer(i2c_bus, chip, addr, alen);
	if (ret < 0)
		return ret;

	ret = i2c_write_data(i2c_bus, chip, buf, len);

	i2c_imx_stop(i2c_bus);

	return ret;
}

#if !defined(I2C2_BASE_ADDR)
#define I2C2_BASE_ADDR	0
#endif

#if !defined(I2C3_BASE_ADDR)
#define I2C3_BASE_ADDR	0
#endif

#if !defined(I2C4_BASE_ADDR)
#define I2C4_BASE_ADDR	0
#endif

#if !defined(I2C5_BASE_ADDR)
#define I2C5_BASE_ADDR 0
#endif

#if !defined(I2C6_BASE_ADDR)
#define I2C6_BASE_ADDR 0
#endif

#if !defined(I2C7_BASE_ADDR)
#define I2C7_BASE_ADDR 0
#endif

#if !defined(I2C8_BASE_ADDR)
#define I2C8_BASE_ADDR 0
#endif

static struct mxc_i2c_bus mxc_i2c_buses[] = {
#if defined(CONFIG_ARCH_LS1021A) || defined(CONFIG_VF610) || \
	defined(CONFIG_FSL_LAYERSCAPE)
	{ 0, I2C1_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 1, I2C2_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 2, I2C3_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 3, I2C4_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 4, I2C5_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 5, I2C6_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 6, I2C7_BASE_ADDR, I2C_QUIRK_FLAG },
	{ 7, I2C8_BASE_ADDR, I2C_QUIRK_FLAG },
#else
	{ 0, I2C1_BASE_ADDR, 0 },
	{ 1, I2C2_BASE_ADDR, 0 },
	{ 2, I2C3_BASE_ADDR, 0 },
	{ 3, I2C4_BASE_ADDR, 0 },
	{ 4, I2C5_BASE_ADDR, 0 },
	{ 5, I2C6_BASE_ADDR, 0 },
	{ 6, I2C7_BASE_ADDR, 0 },
	{ 7, I2C8_BASE_ADDR, 0 },
#endif
};

struct mxc_i2c_bus *i2c_get_base(struct i2c_adapter *adap)
{
	return &mxc_i2c_buses[adap->hwadapnr];
}

static int mxc_i2c_read(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len)
{
	return bus_i2c_read(i2c_get_base(adap), chip, addr, alen, buffer, len);
}

static int mxc_i2c_write(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len)
{
	return bus_i2c_write(i2c_get_base(adap), chip, addr, alen, buffer, len);
}

/*
 * Test if a chip at a given address responds (probe the chip)
 */
static int mxc_i2c_probe(struct i2c_adapter *adap, uint8_t chip)
{
	return bus_i2c_write(i2c_get_base(adap), chip, 0, 0, NULL, 0);
}

int __enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	return 1;
}
int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
	__attribute__((weak, alias("__enable_i2c_clk")));

void bus_i2c_init(int index, int speed, int unused,
		  int (*idle_bus_fn)(void *p), void *idle_bus_data)
{
	int ret;

	if (index >= ARRAY_SIZE(mxc_i2c_buses)) {
		debug("Error i2c index\n");
		return;
	}

	/*
	 * Warning: Be careful to allow the assignment to a static
	 * variable here. This function could be called while U-Boot is
	 * still running in flash memory. So such assignment is equal
	 * to write data to flash without erasing.
	 */
	if (idle_bus_fn)
		mxc_i2c_buses[index].idle_bus_fn = idle_bus_fn;
	if (idle_bus_data)
		mxc_i2c_buses[index].idle_bus_data = idle_bus_data;

	ret = enable_i2c_clk(1, index);
	if (ret < 0) {
		debug("I2C-%d clk fail to enable.\n", index);
		return;
	}

	bus_i2c_set_bus_speed(&mxc_i2c_buses[index], speed);
}

/*
 * Early init I2C for prepare read the clk through I2C.
 */
void i2c_early_init_f(void)
{
	ulong base = mxc_i2c_buses[I2C_EARLY_INIT_INDEX].base;
	bool quirk = mxc_i2c_buses[I2C_EARLY_INIT_INDEX].driver_data
					& I2C_QUIRK_FLAG ? true : false;
	int reg_shift = quirk ? VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;

	/* Set I2C divider value */
	writeb(I2C_IFDR_DIV_CONSERVATIVE, base + (IFDR << reg_shift));
	/* Reset module */
	writeb(I2CR_IDIS, base + (I2CR << reg_shift));
	writeb(0, base + (I2SR << reg_shift));
	/* Enable I2C */
	writeb(I2CR_IEN, base + (I2CR << reg_shift));
}

/*
 * Init I2C Bus
 */
static void mxc_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
	bus_i2c_init(adap->hwadapnr, speed, slaveaddr, NULL, NULL);
}

/*
 * Set I2C Speed
 */
static u32 mxc_i2c_set_bus_speed(struct i2c_adapter *adap, uint speed)
{
	return bus_i2c_set_bus_speed(i2c_get_base(adap), speed);
}

/*
 * Register mxc i2c adapters
 */
#ifdef CONFIG_SYS_I2C_MXC_I2C1
U_BOOT_I2C_ADAP_COMPLETE(mxc0, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C1_SPEED,
			 CONFIG_SYS_MXC_I2C1_SLAVE, 0)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C2
U_BOOT_I2C_ADAP_COMPLETE(mxc1, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C2_SPEED,
			 CONFIG_SYS_MXC_I2C2_SLAVE, 1)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C3
U_BOOT_I2C_ADAP_COMPLETE(mxc2, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C3_SPEED,
			 CONFIG_SYS_MXC_I2C3_SLAVE, 2)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C4
U_BOOT_I2C_ADAP_COMPLETE(mxc3, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C4_SPEED,
			 CONFIG_SYS_MXC_I2C4_SLAVE, 3)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C5
U_BOOT_I2C_ADAP_COMPLETE(mxc4, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C5_SPEED,
			 CONFIG_SYS_MXC_I2C5_SLAVE, 4)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C6
U_BOOT_I2C_ADAP_COMPLETE(mxc5, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C6_SPEED,
			 CONFIG_SYS_MXC_I2C6_SLAVE, 5)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C7
U_BOOT_I2C_ADAP_COMPLETE(mxc6, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C7_SPEED,
			 CONFIG_SYS_MXC_I2C7_SLAVE, 6)
#endif

#ifdef CONFIG_SYS_I2C_MXC_I2C8
U_BOOT_I2C_ADAP_COMPLETE(mxc7, mxc_i2c_init, mxc_i2c_probe,
			 mxc_i2c_read, mxc_i2c_write,
			 mxc_i2c_set_bus_speed,
			 CONFIG_SYS_MXC_I2C8_SPEED,
			 CONFIG_SYS_MXC_I2C8_SLAVE, 7)
#endif

#else

static int mxc_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct mxc_i2c_bus *i2c_bus = dev_get_priv(bus);

	return bus_i2c_set_bus_speed(i2c_bus, speed);
}

static int mxc_i2c_probe(struct udevice *bus)
{
	struct mxc_i2c_bus *i2c_bus = dev_get_priv(bus);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(bus);
	fdt_addr_t addr;
	int ret, ret2;

	i2c_bus->driver_data = dev_get_driver_data(bus);

	addr = devfdt_get_addr(bus);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	i2c_bus->base = addr;
	i2c_bus->index = bus->seq;
	i2c_bus->bus = bus;

	/* Enable clk */
	ret = enable_i2c_clk(1, bus->seq);
	if (ret < 0)
		return ret;

	/*
	 * See Documentation/devicetree/bindings/i2c/i2c-imx.txt
	 * Use gpio to force bus idle when necessary.
	 */
	ret = fdt_stringlist_search(fdt, node, "pinctrl-names", "gpio");
	if (ret < 0) {
		debug("i2c bus %d at 0x%2lx, no gpio pinctrl state.\n", bus->seq, i2c_bus->base);
	} else {
		ret = gpio_request_by_name_nodev(offset_to_ofnode(node),
				"scl-gpios", 0, &i2c_bus->scl_gpio,
				GPIOD_IS_OUT);
		ret2 = gpio_request_by_name_nodev(offset_to_ofnode(node),
				"sda-gpios", 0, &i2c_bus->sda_gpio,
				GPIOD_IS_OUT);
		if (!dm_gpio_is_valid(&i2c_bus->sda_gpio) ||
		    !dm_gpio_is_valid(&i2c_bus->scl_gpio) ||
		    ret || ret2) {
			dev_err(dev, "i2c bus %d at %lu, fail to request scl/sda gpio\n", bus->seq, i2c_bus->base);
			return -EINVAL;
		}
	}

	ret = i2c_idle_bus(i2c_bus);
	if (ret < 0) {
		/* Disable clk */
		enable_i2c_clk(0, bus->seq);
		return ret;
	}

	/*
	 * Pinmux settings are in board file now, until pinmux is supported,
	 * we can set pinmux here in probe function.
	 */

	debug("i2c : controller bus %d at %lu , speed %d: ",
	      bus->seq, i2c_bus->base,
	      i2c_bus->speed);

	return 0;
}

/* Sends: S Addr Wr [A|NA] P */
static int mxc_i2c_probe_chip(struct udevice *bus, u32 chip_addr,
			      u32 chip_flags)
{
	int ret;
	struct mxc_i2c_bus *i2c_bus = dev_get_priv(bus);

	ret = i2c_init_transfer(i2c_bus, chip_addr, 0, 0);
	if (ret < 0) {
		debug("%s failed, ret = %d\n", __func__, ret);
		return ret;
	}

	i2c_imx_stop(i2c_bus);

	return 0;
}

static int mxc_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct mxc_i2c_bus *i2c_bus = dev_get_priv(bus);
	int ret = 0;
	ulong base = i2c_bus->base;
	int reg_shift = i2c_bus->driver_data & I2C_QUIRK_FLAG ?
		VF610_I2C_REGSHIFT : IMX_I2C_REGSHIFT;
	int read_mode;

	/* Here address len is set to -1 to not send any address at first.
	 * Otherwise i2c_init_transfer will send the chip address with write
	 * mode set.  This is wrong if the 1st message is read.
	 */
	ret = i2c_init_transfer(i2c_bus, msg->addr, 0, -1);
	if (ret < 0) {
		debug("i2c_init_transfer error: %d\n", ret);
		return ret;
	}

	read_mode = -1; /* So it's always different on the first message */
	for (; nmsgs > 0; nmsgs--, msg++) {
		const int msg_is_read = !!(msg->flags & I2C_M_RD);

		debug("i2c_xfer: chip=0x%x, len=0x%x, dir=%c\n", msg->addr,
		      msg->len, msg_is_read ? 'R' : 'W');

		if (msg_is_read != read_mode) {
			/* Send repeated start if not 1st message */
			if (read_mode != -1) {
				debug("i2c_xfer: [RSTART]\n");
				ret = readb(base + (I2CR << reg_shift));
				ret |= I2CR_RSTA;
				writeb(ret, base + (I2CR << reg_shift));
			}
			debug("i2c_xfer: [ADDR %02x | %c]\n", msg->addr,
			      msg_is_read ? 'R' : 'W');
			ret = tx_byte(i2c_bus, (msg->addr << 1) | msg_is_read);
			if (ret < 0) {
				debug("i2c_xfer: [STOP]\n");
				i2c_imx_stop(i2c_bus);
				break;
			}
			read_mode = msg_is_read;
		}

		if (msg->flags & I2C_M_RD)
			ret = i2c_read_data(i2c_bus, msg->addr, msg->buf,
					    msg->len, nmsgs == 1 ||
						      (msg->flags & I2C_M_STOP));
		else
			ret = i2c_write_data(i2c_bus, msg->addr, msg->buf,
					     msg->len);

		if (ret < 0)
			break;
	}

	if (ret)
		debug("i2c_write: error sending\n");

	i2c_imx_stop(i2c_bus);

	return ret;
}

static const struct dm_i2c_ops mxc_i2c_ops = {
	.xfer		= mxc_i2c_xfer,
	.probe_chip	= mxc_i2c_probe_chip,
	.set_bus_speed	= mxc_i2c_set_bus_speed,
};

static const struct udevice_id mxc_i2c_ids[] = {
	{ .compatible = "fsl,imx21-i2c", },
	{ .compatible = "fsl,vf610-i2c", .data = I2C_QUIRK_FLAG, },
	{}
};

U_BOOT_DRIVER(i2c_mxc) = {
	.name = "i2c_mxc",
	.id = UCLASS_I2C,
	.of_match = mxc_i2c_ids,
	.probe = mxc_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct mxc_i2c_bus),
	.ops = &mxc_i2c_ops,
};
#endif
