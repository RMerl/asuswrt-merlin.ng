// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2006,2009 Freescale Semiconductor, Inc.
 *
 * 2012, Heiko Schocher, DENX Software Engineering, hs@denx.de.
 * Changes for multibus/multiadapter I2C support.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>		/* Functional interface */
#include <asm/io.h>
#include <asm/fsl_i2c.h>	/* HW definitions */
#include <clk.h>
#include <dm.h>
#include <mapmem.h>

/* The maximum number of microseconds we will wait until another master has
 * released the bus.  If not defined in the board header file, then use a
 * generic value.
 */
#ifndef CONFIG_I2C_MBB_TIMEOUT
#define CONFIG_I2C_MBB_TIMEOUT	100000
#endif

/* The maximum number of microseconds we will wait for a read or write
 * operation to complete.  If not defined in the board header file, then use a
 * generic value.
 */
#ifndef CONFIG_I2C_TIMEOUT
#define CONFIG_I2C_TIMEOUT	100000
#endif

#define I2C_READ_BIT  1
#define I2C_WRITE_BIT 0

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_I2C
static const struct fsl_i2c_base *i2c_base[4] = {
	(struct fsl_i2c_base *)(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_I2C_OFFSET),
#ifdef CONFIG_SYS_FSL_I2C2_OFFSET
	(struct fsl_i2c_base *)(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_I2C2_OFFSET),
#endif
#ifdef CONFIG_SYS_FSL_I2C3_OFFSET
	(struct fsl_i2c_base *)(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_I2C3_OFFSET),
#endif
#ifdef CONFIG_SYS_FSL_I2C4_OFFSET
	(struct fsl_i2c_base *)(CONFIG_SYS_IMMR + CONFIG_SYS_FSL_I2C4_OFFSET)
#endif
};
#endif

/* I2C speed map for a DFSR value of 1 */

#ifdef __M68K__
/*
 * Map I2C frequency dividers to FDR and DFSR values
 *
 * This structure is used to define the elements of a table that maps I2C
 * frequency divider (I2C clock rate divided by I2C bus speed) to a value to be
 * programmed into the Frequency Divider Ratio (FDR) and Digital Filter
 * Sampling Rate (DFSR) registers.
 *
 * The actual table should be defined in the board file, and it must be called
 * fsl_i2c_speed_map[].
 *
 * The last entry of the table must have a value of {-1, X}, where X is same
 * FDR/DFSR values as the second-to-last entry.  This guarantees that any
 * search through the array will always find a match.
 *
 * The values of the divider must be in increasing numerical order, i.e.
 * fsl_i2c_speed_map[x+1].divider > fsl_i2c_speed_map[x].divider.
 *
 * For this table, the values are based on a value of 1 for the DFSR
 * register.  See the application note AN2919 "Determining the I2C Frequency
 * Divider Ratio for SCL"
 *
 * ColdFire I2C frequency dividers for FDR values are different from
 * PowerPC. The protocol to use the I2C module is still the same.
 * A different table is defined and are based on MCF5xxx user manual.
 *
 */
static const struct {
	unsigned short divider;
	u8 fdr;
} fsl_i2c_speed_map[] = {
	{20, 32}, {22, 33}, {24, 34}, {26, 35},
	{28, 0}, {28, 36}, {30, 1}, {32, 37},
	{34, 2}, {36, 38}, {40, 3}, {40, 39},
	{44, 4}, {48, 5}, {48, 40}, {56, 6},
	{56, 41}, {64, 42}, {68, 7}, {72, 43},
	{80, 8}, {80, 44}, {88, 9}, {96, 41},
	{104, 10}, {112, 42}, {128, 11}, {128, 43},
	{144, 12}, {160, 13}, {160, 48}, {192, 14},
	{192, 49}, {224, 50}, {240, 15}, {256, 51},
	{288, 16}, {320, 17}, {320, 52}, {384, 18},
	{384, 53}, {448, 54}, {480, 19}, {512, 55},
	{576, 20}, {640, 21}, {640, 56}, {768, 22},
	{768, 57}, {960, 23}, {896, 58}, {1024, 59},
	{1152, 24}, {1280, 25}, {1280, 60}, {1536, 26},
	{1536, 61}, {1792, 62}, {1920, 27}, {2048, 63},
	{2304, 28}, {2560, 29}, {3072, 30}, {3840, 31},
	{-1, 31}
};
#endif

/**
 * Set the I2C bus speed for a given I2C device
 *
 * @param base: the I2C device registers
 * @i2c_clk: I2C bus clock frequency
 * @speed: the desired speed of the bus
 *
 * The I2C device must be stopped before calling this function.
 *
 * The return value is the actual bus speed that is set.
 */
static uint set_i2c_bus_speed(const struct fsl_i2c_base *base,
			      uint i2c_clk, uint speed)
{
	ushort divider = min(i2c_clk / speed, (uint)USHRT_MAX);

	/*
	 * We want to choose an FDR/DFSR that generates an I2C bus speed that
	 * is equal to or lower than the requested speed.  That means that we
	 * want the first divider that is equal to or greater than the
	 * calculated divider.
	 */
#ifdef __PPC__
	u8 dfsr, fdr = 0x31; /* Default if no FDR found */
	/* a, b and dfsr matches identifiers A,B and C respectively in AN2919 */
	ushort a, b, ga, gb;
	ulong c_div, est_div;

#ifdef CONFIG_FSL_I2C_CUSTOM_DFSR
	dfsr = CONFIG_FSL_I2C_CUSTOM_DFSR;
#else
	/* Condition 1: dfsr <= 50/T */
	dfsr = (5 * (i2c_clk / 1000)) / 100000;
#endif
#ifdef CONFIG_FSL_I2C_CUSTOM_FDR
	fdr = CONFIG_FSL_I2C_CUSTOM_FDR;
	speed = i2c_clk / divider; /* Fake something */
#else
	debug("Requested speed:%d, i2c_clk:%d\n", speed, i2c_clk);
	if (!dfsr)
		dfsr = 1;

	est_div = ~0;
	for (ga = 0x4, a = 10; a <= 30; ga++, a += 2) {
		for (gb = 0; gb < 8; gb++) {
			b = 16 << gb;
			c_div = b * (a + ((3 * dfsr) / b) * 2);
			if (c_div > divider && c_div < est_div) {
				ushort bin_gb, bin_ga;

				est_div = c_div;
				bin_gb = gb << 2;
				bin_ga = (ga & 0x3) | ((ga & 0x4) << 3);
				fdr = bin_gb | bin_ga;
				speed = i2c_clk / est_div;

				debug("FDR: 0x%.2x, ", fdr);
				debug("div: %ld, ", est_div);
				debug("ga: 0x%x, gb: 0x%x, ", ga, gb);
				debug("a: %d, b: %d, speed: %d\n", a, b, speed);

				/* Condition 2 not accounted for */
				debug("Tr <= %d ns\n",
				      (b - 3 * dfsr) * 1000000 /
				      (i2c_clk / 1000));
			}
		}
		if (a == 20)
			a += 2;
		if (a == 24)
			a += 4;
	}
	debug("divider: %d, est_div: %ld, DFSR: %d\n", divider, est_div, dfsr);
	debug("FDR: 0x%.2x, speed: %d\n", fdr, speed);
#endif
	writeb(dfsr, &base->dfsrr);	/* set default filter */
	writeb(fdr, &base->fdr);	/* set bus speed */
#else
	uint i;

	for (i = 0; i < ARRAY_SIZE(fsl_i2c_speed_map); i++)
		if (fsl_i2c_speed_map[i].divider >= divider) {
			u8 fdr;

			fdr = fsl_i2c_speed_map[i].fdr;
			speed = i2c_clk / fsl_i2c_speed_map[i].divider;
			writeb(fdr, &base->fdr);	/* set bus speed */

			break;
		}
#endif
	return speed;
}

#ifndef CONFIG_DM_I2C
static uint get_i2c_clock(int bus)
{
	if (bus)
		return gd->arch.i2c2_clk;	/* I2C2 clock */
	else
		return gd->arch.i2c1_clk;	/* I2C1 clock */
}
#endif

static int fsl_i2c_fixup(const struct fsl_i2c_base *base)
{
	const unsigned long long timeout = usec2ticks(CONFIG_I2C_MBB_TIMEOUT);
	unsigned long long timeval = 0;
	int ret = -1;
	uint flags = 0;

#ifdef CONFIG_SYS_FSL_ERRATUM_I2C_A004447
	uint svr = get_svr();

	if ((SVR_SOC_VER(svr) == SVR_8548 && IS_SVR_REV(svr, 3, 1)) ||
	    (SVR_REV(svr) <= CONFIG_SYS_FSL_A004447_SVR_REV))
		flags = I2C_CR_BIT6;
#endif

	writeb(I2C_CR_MEN | I2C_CR_MSTA, &base->cr);

	timeval = get_ticks();
	while (!(readb(&base->sr) & I2C_SR_MBB)) {
		if ((get_ticks() - timeval) > timeout)
			goto err;
	}

	if (readb(&base->sr) & I2C_SR_MAL) {
		/* SDA is stuck low */
		writeb(0, &base->cr);
		udelay(100);
		writeb(I2C_CR_MSTA | flags, &base->cr);
		writeb(I2C_CR_MEN | I2C_CR_MSTA | flags, &base->cr);
	}

	readb(&base->dr);

	timeval = get_ticks();
	while (!(readb(&base->sr) & I2C_SR_MIF)) {
		if ((get_ticks() - timeval) > timeout)
			goto err;
	}
	ret = 0;

err:
	writeb(I2C_CR_MEN | flags, &base->cr);
	writeb(0, &base->sr);
	udelay(100);

	return ret;
}

static void __i2c_init(const struct fsl_i2c_base *base, int speed, int
		       slaveadd, int i2c_clk, int busnum)
{
	const unsigned long long timeout = usec2ticks(CONFIG_I2C_MBB_TIMEOUT);
	unsigned long long timeval;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/* Call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	 */
	i2c_init_board();
#endif
	writeb(0, &base->cr);		/* stop I2C controller */
	udelay(5);			/* let it shutdown in peace */
	set_i2c_bus_speed(base, i2c_clk, speed);
	writeb(slaveadd << 1, &base->adr);/* write slave address */
	writeb(0x0, &base->sr);		/* clear status register */
	writeb(I2C_CR_MEN, &base->cr);	/* start I2C controller */

	timeval = get_ticks();
	while (readb(&base->sr) & I2C_SR_MBB) {
		if ((get_ticks() - timeval) < timeout)
			continue;

		if (fsl_i2c_fixup(base))
			debug("i2c_init: BUS#%d failed to init\n",
			      busnum);

		break;
	}
}

static int i2c_wait4bus(const struct fsl_i2c_base *base)
{
	unsigned long long timeval = get_ticks();
	const unsigned long long timeout = usec2ticks(CONFIG_I2C_MBB_TIMEOUT);

	while (readb(&base->sr) & I2C_SR_MBB) {
		if ((get_ticks() - timeval) > timeout)
			return -1;
	}

	return 0;
}

static int i2c_wait(const struct fsl_i2c_base *base, int write)
{
	u32 csr;
	unsigned long long timeval = get_ticks();
	const unsigned long long timeout = usec2ticks(CONFIG_I2C_TIMEOUT);

	do {
		csr = readb(&base->sr);
		if (!(csr & I2C_SR_MIF))
			continue;
		/* Read again to allow register to stabilise */
		csr = readb(&base->sr);

		writeb(0x0, &base->sr);

		if (csr & I2C_SR_MAL) {
			debug("%s: MAL\n", __func__);
			return -1;
		}

		if (!(csr & I2C_SR_MCF))	{
			debug("%s: unfinished\n", __func__);
			return -1;
		}

		if (write == I2C_WRITE_BIT && (csr & I2C_SR_RXAK)) {
			debug("%s: No RXACK\n", __func__);
			return -1;
		}

		return 0;
	} while ((get_ticks() - timeval) < timeout);

	debug("%s: timed out\n", __func__);
	return -1;
}

static int i2c_write_addr(const struct fsl_i2c_base *base, u8 dev,
			  u8 dir, int rsta)
{
	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX
	       | (rsta ? I2C_CR_RSTA : 0),
	       &base->cr);

	writeb((dev << 1) | dir, &base->dr);

	if (i2c_wait(base, I2C_WRITE_BIT) < 0)
		return 0;

	return 1;
}

static int __i2c_write_data(const struct fsl_i2c_base *base, u8 *data,
			    int length)
{
	int i;

	for (i = 0; i < length; i++) {
		writeb(data[i], &base->dr);

		if (i2c_wait(base, I2C_WRITE_BIT) < 0)
			break;
	}

	return i;
}

static int __i2c_read_data(const struct fsl_i2c_base *base, u8 *data,
			   int length)
{
	int i;

	writeb(I2C_CR_MEN | I2C_CR_MSTA | ((length == 1) ? I2C_CR_TXAK : 0),
	       &base->cr);

	/* dummy read */
	readb(&base->dr);

	for (i = 0; i < length; i++) {
		if (i2c_wait(base, I2C_READ_BIT) < 0)
			break;

		/* Generate ack on last next to last byte */
		if (i == length - 2)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_TXAK,
			       &base->cr);

		/* Do not generate stop on last byte */
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,
			       &base->cr);

		data[i] = readb(&base->dr);
	}

	return i;
}

static int __i2c_read(const struct fsl_i2c_base *base, u8 chip_addr, u8 *offset,
		      int olen, u8 *data, int dlen)
{
	int ret = -1; /* signal error */

	if (i2c_wait4bus(base) < 0)
		return -1;

	/* Some drivers use offset lengths in excess of 4 bytes. These drivers
	 * adhere to the following convention:
	 * - the offset length is passed as negative (that is, the absolute
	 *   value of olen is the actual offset length)
	 * - the offset itself is passed in data, which is overwritten by the
	 *   subsequent read operation
	 */
	if (olen < 0) {
		if (i2c_write_addr(base, chip_addr, I2C_WRITE_BIT, 0) != 0)
			ret = __i2c_write_data(base, data, -olen);

		if (ret != -olen)
			return -1;

		if (dlen && i2c_write_addr(base, chip_addr,
					   I2C_READ_BIT, 1) != 0)
			ret = __i2c_read_data(base, data, dlen);
	} else {
		if ((!dlen || olen > 0) &&
		    i2c_write_addr(base, chip_addr, I2C_WRITE_BIT, 0) != 0  &&
		    __i2c_write_data(base, offset, olen) == olen)
			ret = 0; /* No error so far */

		if (dlen && i2c_write_addr(base, chip_addr, I2C_READ_BIT,
					   olen ? 1 : 0) != 0)
			ret = __i2c_read_data(base, data, dlen);
	}

	writeb(I2C_CR_MEN, &base->cr);

	if (i2c_wait4bus(base)) /* Wait until STOP */
		debug("i2c_read: wait4bus timed out\n");

	if (ret == dlen)
		return 0;

	return -1;
}

static int __i2c_write(const struct fsl_i2c_base *base, u8 chip_addr,
		       u8 *offset, int olen, u8 *data, int dlen)
{
	int ret = -1; /* signal error */

	if (i2c_wait4bus(base) < 0)
		return -1;

	if (i2c_write_addr(base, chip_addr, I2C_WRITE_BIT, 0) != 0 &&
	    __i2c_write_data(base, offset, olen) == olen) {
		ret = __i2c_write_data(base, data, dlen);
	}

	writeb(I2C_CR_MEN, &base->cr);
	if (i2c_wait4bus(base)) /* Wait until STOP */
		debug("i2c_write: wait4bus timed out\n");

	if (ret == dlen)
		return 0;

	return -1;
}

static int __i2c_probe_chip(const struct fsl_i2c_base *base, uchar chip)
{
	/* For unknown reason the controller will ACK when
	 * probing for a slave with the same address, so skip
	 * it.
	 */
	if (chip == (readb(&base->adr) >> 1))
		return -1;

	return __i2c_read(base, chip, 0, 0, NULL, 0);
}

static uint __i2c_set_bus_speed(const struct fsl_i2c_base *base,
				uint speed, int i2c_clk)
{
	writeb(0, &base->cr);		/* stop controller */
	set_i2c_bus_speed(base, i2c_clk, speed);
	writeb(I2C_CR_MEN, &base->cr);	/* start controller */

	return 0;
}

#ifndef CONFIG_DM_I2C
static void fsl_i2c_init(struct i2c_adapter *adap, int speed, int slaveadd)
{
	__i2c_init(i2c_base[adap->hwadapnr], speed, slaveadd,
		   get_i2c_clock(adap->hwadapnr), adap->hwadapnr);
}

static int fsl_i2c_probe_chip(struct i2c_adapter *adap, uchar chip)
{
	return __i2c_probe_chip(i2c_base[adap->hwadapnr], chip);
}

static int fsl_i2c_read(struct i2c_adapter *adap, u8 chip_addr, uint offset,
			int olen, u8 *data, int dlen)
{
	u8 *o = (u8 *)&offset;

	return __i2c_read(i2c_base[adap->hwadapnr], chip_addr, &o[4 - olen],
			  olen, data, dlen);
}

static int fsl_i2c_write(struct i2c_adapter *adap, u8 chip_addr, uint offset,
			 int olen, u8 *data, int dlen)
{
	u8 *o = (u8 *)&offset;

	return __i2c_write(i2c_base[adap->hwadapnr], chip_addr, &o[4 - olen],
			   olen, data, dlen);
}

static uint fsl_i2c_set_bus_speed(struct i2c_adapter *adap, uint speed)
{
	return __i2c_set_bus_speed(i2c_base[adap->hwadapnr], speed,
				   get_i2c_clock(adap->hwadapnr));
}

/*
 * Register fsl i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(fsl_0, fsl_i2c_init, fsl_i2c_probe_chip, fsl_i2c_read,
			 fsl_i2c_write, fsl_i2c_set_bus_speed,
			 CONFIG_SYS_FSL_I2C_SPEED, CONFIG_SYS_FSL_I2C_SLAVE,
			 0)
#ifdef CONFIG_SYS_FSL_I2C2_OFFSET
U_BOOT_I2C_ADAP_COMPLETE(fsl_1, fsl_i2c_init, fsl_i2c_probe_chip, fsl_i2c_read,
			 fsl_i2c_write, fsl_i2c_set_bus_speed,
			 CONFIG_SYS_FSL_I2C2_SPEED, CONFIG_SYS_FSL_I2C2_SLAVE,
			 1)
#endif
#ifdef CONFIG_SYS_FSL_I2C3_OFFSET
U_BOOT_I2C_ADAP_COMPLETE(fsl_2, fsl_i2c_init, fsl_i2c_probe_chip, fsl_i2c_read,
			 fsl_i2c_write, fsl_i2c_set_bus_speed,
			 CONFIG_SYS_FSL_I2C3_SPEED, CONFIG_SYS_FSL_I2C3_SLAVE,
			 2)
#endif
#ifdef CONFIG_SYS_FSL_I2C4_OFFSET
U_BOOT_I2C_ADAP_COMPLETE(fsl_3, fsl_i2c_init, fsl_i2c_probe_chip, fsl_i2c_read,
			 fsl_i2c_write, fsl_i2c_set_bus_speed,
			 CONFIG_SYS_FSL_I2C4_SPEED, CONFIG_SYS_FSL_I2C4_SLAVE,
			 3)
#endif
#else /* CONFIG_DM_I2C */
static int fsl_i2c_probe_chip(struct udevice *bus, u32 chip_addr,
			      u32 chip_flags)
{
	struct fsl_i2c_dev *dev = dev_get_priv(bus);

	return __i2c_probe_chip(dev->base, chip_addr);
}

static int fsl_i2c_set_bus_speed(struct udevice *bus, uint speed)
{
	struct fsl_i2c_dev *dev = dev_get_priv(bus);

	return __i2c_set_bus_speed(dev->base, speed, dev->i2c_clk);
}

static int fsl_i2c_ofdata_to_platdata(struct udevice *bus)
{
	struct fsl_i2c_dev *dev = dev_get_priv(bus);
	struct clk clock;

	dev->base = map_sysmem(dev_read_addr(bus), sizeof(struct fsl_i2c_base));

	if (!dev->base)
		return -ENOMEM;

	dev->index = dev_read_u32_default(bus, "cell-index", -1);
	dev->slaveadd = dev_read_u32_default(bus, "u-boot,i2c-slave-addr",
					     0x7f);
	dev->speed = dev_read_u32_default(bus, "clock-frequency", 400000);

	if (!clk_get_by_index(bus, 0, &clock))
		dev->i2c_clk = clk_get_rate(&clock);
	else
		dev->i2c_clk = dev->index ? gd->arch.i2c2_clk :
					    gd->arch.i2c1_clk;

	return 0;
}

static int fsl_i2c_probe(struct udevice *bus)
{
	struct fsl_i2c_dev *dev = dev_get_priv(bus);

	__i2c_init(dev->base, dev->speed, dev->slaveadd, dev->i2c_clk,
		   dev->index);
	return 0;
}

static int fsl_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct fsl_i2c_dev *dev = dev_get_priv(bus);
	struct i2c_msg *dmsg, *omsg, dummy;

	memset(&dummy, 0, sizeof(struct i2c_msg));

	/* We expect either two messages (one with an offset and one with the
	 * actual data) or one message (just data)
	 */
	if (nmsgs > 2 || nmsgs == 0) {
		debug("%s: Only one or two messages are supported.", __func__);
		return -1;
	}

	omsg = nmsgs == 1 ? &dummy : msg;
	dmsg = nmsgs == 1 ? msg : msg + 1;

	if (dmsg->flags & I2C_M_RD)
		return __i2c_read(dev->base, dmsg->addr, omsg->buf, omsg->len,
				  dmsg->buf, dmsg->len);
	else
		return __i2c_write(dev->base, dmsg->addr, omsg->buf, omsg->len,
				   dmsg->buf, dmsg->len);
}

static const struct dm_i2c_ops fsl_i2c_ops = {
	.xfer           = fsl_i2c_xfer,
	.probe_chip     = fsl_i2c_probe_chip,
	.set_bus_speed  = fsl_i2c_set_bus_speed,
};

static const struct udevice_id fsl_i2c_ids[] = {
	{ .compatible = "fsl-i2c", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(i2c_fsl) = {
	.name = "i2c_fsl",
	.id = UCLASS_I2C,
	.of_match = fsl_i2c_ids,
	.probe = fsl_i2c_probe,
	.ofdata_to_platdata = fsl_i2c_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct fsl_i2c_dev),
	.ops = &fsl_i2c_ops,
};

#endif /* CONFIG_DM_I2C */
