// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011, 2013 Renesas Solutions Corp.
 * Copyright (C) 2011, 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * NOTE: This driver should be converted to driver model before June 2017.
 * Please see doc/driver-model/i2c-howto.txt for instructions.
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* Every register is 32bit aligned, but only 8bits in size */
#define ureg(name) u8 name; u8 __pad_##name##0; u16 __pad_##name##1;
struct sh_i2c {
	ureg(icdr);
	ureg(iccr);
	ureg(icsr);
	ureg(icic);
	ureg(iccl);
	ureg(icch);
};
#undef ureg

/* ICCR */
#define SH_I2C_ICCR_ICE		(1 << 7)
#define SH_I2C_ICCR_RACK	(1 << 6)
#define SH_I2C_ICCR_RTS		(1 << 4)
#define SH_I2C_ICCR_BUSY	(1 << 2)
#define SH_I2C_ICCR_SCP		(1 << 0)

/* ICSR / ICIC */
#define SH_IC_BUSY	(1 << 4)
#define SH_IC_TACK	(1 << 2)
#define SH_IC_WAIT	(1 << 1)
#define SH_IC_DTE	(1 << 0)

#ifdef CONFIG_SH_I2C_8BIT
/* store 8th bit of iccl and icch in ICIC register */
#define SH_I2C_ICIC_ICCLB8	(1 << 7)
#define SH_I2C_ICIC_ICCHB8	(1 << 6)
#endif

static const struct sh_i2c *i2c_dev[CONFIG_SYS_I2C_SH_NUM_CONTROLLERS] = {
	(struct sh_i2c *)CONFIG_SYS_I2C_SH_BASE0,
#ifdef CONFIG_SYS_I2C_SH_BASE1
	(struct sh_i2c *)CONFIG_SYS_I2C_SH_BASE1,
#endif
#ifdef CONFIG_SYS_I2C_SH_BASE2
	(struct sh_i2c *)CONFIG_SYS_I2C_SH_BASE2,
#endif
#ifdef CONFIG_SYS_I2C_SH_BASE3
	(struct sh_i2c *)CONFIG_SYS_I2C_SH_BASE3,
#endif
#ifdef CONFIG_SYS_I2C_SH_BASE4
	(struct sh_i2c *)CONFIG_SYS_I2C_SH_BASE4,
#endif
};

static u16 iccl, icch;

#define IRQ_WAIT 1000

static void sh_irq_dte(struct sh_i2c *dev)
{
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if (SH_IC_DTE & readb(&dev->icsr))
			break;
		udelay(10);
	}
}

static int sh_irq_dte_with_tack(struct sh_i2c *dev)
{
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if (SH_IC_DTE & readb(&dev->icsr))
			break;
		if (SH_IC_TACK & readb(&dev->icsr))
			return -1;
		udelay(10);
	}
	return 0;
}

static void sh_irq_busy(struct sh_i2c *dev)
{
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if (!(SH_IC_BUSY & readb(&dev->icsr)))
			break;
		udelay(10);
	}
}

static int sh_i2c_set_addr(struct sh_i2c *dev, u8 chip, u8 addr, int stop)
{
	u8 icic = SH_IC_TACK;

	debug("%s: chip: %x, addr: %x iccl: %x, icch %x\n",
				__func__, chip, addr, iccl, icch);
	clrbits_8(&dev->iccr, SH_I2C_ICCR_ICE);
	setbits_8(&dev->iccr, SH_I2C_ICCR_ICE);

	writeb(iccl & 0xff, &dev->iccl);
	writeb(icch & 0xff, &dev->icch);
#ifdef CONFIG_SH_I2C_8BIT
	if (iccl > 0xff)
		icic |= SH_I2C_ICIC_ICCLB8;
	if (icch > 0xff)
		icic |= SH_I2C_ICIC_ICCHB8;
#endif
	writeb(icic, &dev->icic);

	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RTS|SH_I2C_ICCR_BUSY), &dev->iccr);
	sh_irq_dte(dev);

	clrbits_8(&dev->icsr, SH_IC_TACK);
	writeb(chip << 1, &dev->icdr);
	if (sh_irq_dte_with_tack(dev) != 0)
		return -1;

	writeb(addr, &dev->icdr);
	if (stop)
		writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RTS), &dev->iccr);

	if (sh_irq_dte_with_tack(dev) != 0)
		return -1;
	return 0;
}

static void sh_i2c_finish(struct sh_i2c *dev)
{
	writeb(0, &dev->icsr);
	clrbits_8(&dev->iccr, SH_I2C_ICCR_ICE);
}

static int
sh_i2c_raw_write(struct sh_i2c *dev, u8 chip, uint addr, u8 val)
{
	int ret = -1;
	if (sh_i2c_set_addr(dev, chip, addr, 0) != 0)
		goto exit0;
	udelay(10);

	writeb(val, &dev->icdr);
	if (sh_irq_dte_with_tack(dev) != 0)
		goto exit0;

	writeb((SH_I2C_ICCR_ICE | SH_I2C_ICCR_RTS), &dev->iccr);
	if (sh_irq_dte_with_tack(dev) != 0)
		goto exit0;
	sh_irq_busy(dev);
	ret = 0;

exit0:
	sh_i2c_finish(dev);
	return ret;
}

static int sh_i2c_raw_read(struct sh_i2c *dev, u8 chip, u8 addr)
{
	int ret = -1;

#if defined(CONFIG_SH73A0)
	if (sh_i2c_set_addr(dev, chip, addr, 0) != 0)
		goto exit0;
#else
	if (sh_i2c_set_addr(dev, chip, addr, 1) != 0)
		goto exit0;
	udelay(100);
#endif

	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RTS|SH_I2C_ICCR_BUSY), &dev->iccr);
	sh_irq_dte(dev);

	writeb(chip << 1 | 0x01, &dev->icdr);
	if (sh_irq_dte_with_tack(dev) != 0)
		goto exit0;

	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_SCP), &dev->iccr);
	if (sh_irq_dte_with_tack(dev) != 0)
		goto exit0;

	ret = readb(&dev->icdr) & 0xff;

	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RACK), &dev->iccr);
	readb(&dev->icdr); /* Dummy read */
	sh_irq_busy(dev);

exit0:
	sh_i2c_finish(dev);

	return ret;
}

static void
sh_i2c_init(struct i2c_adapter *adap, int speed, int slaveadd)
{
	int num, denom, tmp;

	/* No i2c support prior to relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return;

	/*
	 * Calculate the value for iccl. From the data sheet:
	 * iccl = (p-clock / transfer-rate) * (L / (L + H))
	 * where L and H are the SCL low and high ratio.
	 */
	num = CONFIG_SH_I2C_CLOCK * CONFIG_SH_I2C_DATA_LOW;
	denom = speed * (CONFIG_SH_I2C_DATA_HIGH + CONFIG_SH_I2C_DATA_LOW);
	tmp = num * 10 / denom;
	if (tmp % 10 >= 5)
		iccl = (u16)((num/denom) + 1);
	else
		iccl = (u16)(num/denom);

	/* Calculate the value for icch. From the data sheet:
	   icch = (p clock / transfer rate) * (H / (L + H)) */
	num = CONFIG_SH_I2C_CLOCK * CONFIG_SH_I2C_DATA_HIGH;
	tmp = num * 10 / denom;
	if (tmp % 10 >= 5)
		icch = (u16)((num/denom) + 1);
	else
		icch = (u16)(num/denom);

	debug("clock: %d, speed %d, iccl: %x, icch: %x\n",
			CONFIG_SH_I2C_CLOCK, speed, iccl, icch);
}

static int sh_i2c_read(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, u8 *data, int len)
{
	int ret, i;
	struct sh_i2c *dev = (struct sh_i2c *)i2c_dev[adap->hwadapnr];

	for (i = 0; i < len; i++) {
		ret = sh_i2c_raw_read(dev, chip, addr + i);
		if (ret < 0)
			return -1;

		data[i] = ret & 0xff;
		debug("%s: data[%d]: %02x\n", __func__, i, data[i]);
	}

	return 0;
}

static int sh_i2c_write(struct i2c_adapter *adap, uint8_t chip, uint addr,
				int alen, u8 *data, int len)
{
	struct sh_i2c *dev = (struct sh_i2c *)i2c_dev[adap->hwadapnr];
	int i;

	for (i = 0; i < len; i++) {
		debug("%s: data[%d]: %02x\n", __func__, i, data[i]);
		if (sh_i2c_raw_write(dev, chip, addr + i, data[i]) != 0)
			return -1;
	}
	return 0;
}

static int
sh_i2c_probe(struct i2c_adapter *adap, u8 dev)
{
	u8 dummy[1];

	return sh_i2c_read(adap, dev, 0, 0, dummy, sizeof dummy);
}

static unsigned int sh_i2c_set_bus_speed(struct i2c_adapter *adap,
			unsigned int speed)
{
	struct sh_i2c *dev = (struct sh_i2c *)i2c_dev[adap->hwadapnr];

	sh_i2c_finish(dev);
	sh_i2c_init(adap, speed, 0);

	return 0;
}

/*
 * Register RCAR i2c adapters
 */
U_BOOT_I2C_ADAP_COMPLETE(sh_0, sh_i2c_init, sh_i2c_probe, sh_i2c_read,
	sh_i2c_write, sh_i2c_set_bus_speed, CONFIG_SYS_I2C_SH_SPEED0, 0, 0)
#ifdef CONFIG_SYS_I2C_SH_BASE1
U_BOOT_I2C_ADAP_COMPLETE(sh_1, sh_i2c_init, sh_i2c_probe, sh_i2c_read,
	sh_i2c_write, sh_i2c_set_bus_speed, CONFIG_SYS_I2C_SH_SPEED1, 0, 1)
#endif
#ifdef CONFIG_SYS_I2C_SH_BASE2
U_BOOT_I2C_ADAP_COMPLETE(sh_2, sh_i2c_init, sh_i2c_probe, sh_i2c_read,
	sh_i2c_write, sh_i2c_set_bus_speed, CONFIG_SYS_I2C_SH_SPEED2, 0, 2)
#endif
#ifdef CONFIG_SYS_I2C_SH_BASE3
U_BOOT_I2C_ADAP_COMPLETE(sh_3, sh_i2c_init, sh_i2c_probe, sh_i2c_read,
	sh_i2c_write, sh_i2c_set_bus_speed, CONFIG_SYS_I2C_SH_SPEED3, 0, 3)
#endif
#ifdef CONFIG_SYS_I2C_SH_BASE4
U_BOOT_I2C_ADAP_COMPLETE(sh_4, sh_i2c_init, sh_i2c_probe, sh_i2c_read,
	sh_i2c_write, sh_i2c_set_bus_speed, CONFIG_SYS_I2C_SH_SPEED4, 0, 4)
#endif
