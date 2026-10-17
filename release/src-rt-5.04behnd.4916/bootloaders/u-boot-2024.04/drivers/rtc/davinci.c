// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 DENX Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
 * Copyright (C) 2021 Dario Binacchi <dariobin@libero.it>
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <clk.h>
#include <log.h>
#include <rtc.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>

/* RTC registers */
#define OMAP_RTC_SECONDS_REG		0x00
#define OMAP_RTC_MINUTES_REG		0x04
#define OMAP_RTC_HOURS_REG		0x08
#define OMAP_RTC_DAYS_REG		0x0C
#define OMAP_RTC_MONTHS_REG		0x10
#define OMAP_RTC_YEARS_REG		0x14
#define OMAP_RTC_WEEKS_REG		0x18

#define OMAP_RTC_CTRL_REG		0x40
#define OMAP_RTC_STATUS_REG		0x44
#define OMAP_RTC_INTERRUPTS_REG		0x48

#define OMAP_RTC_OSC_REG		0x54

#define OMAP_RTC_SCRATCH0_REG		0x60
#define OMAP_RTC_SCRATCH1_REG		0x64
#define OMAP_RTC_SCRATCH2_REG		0x68

#define OMAP_RTC_KICK0_REG		0x6c
#define OMAP_RTC_KICK1_REG		0x70

#define OMAP_RTC_PMIC_REG		0x98

/* OMAP_RTC_CTRL_REG bit fields: */
#define OMAP_RTC_CTRL_SPLIT		BIT(7)
#define OMAP_RTC_CTRL_DISABLE		BIT(6)
#define OMAP_RTC_CTRL_SET_32_COUNTER	BIT(5)
#define OMAP_RTC_CTRL_TEST		BIT(4)
#define OMAP_RTC_CTRL_MODE_12_24	BIT(3)
#define OMAP_RTC_CTRL_AUTO_COMP		BIT(2)
#define OMAP_RTC_CTRL_ROUND_30S		BIT(1)
#define OMAP_RTC_CTRL_STOP		BIT(0)

/* OMAP_RTC_STATUS_REG bit fields */
#define OMAP_RTC_STATUS_POWER_UP	BIT(7)
#define OMAP_RTC_STATUS_ALARM2		BIT(7)
#define OMAP_RTC_STATUS_ALARM		BIT(6)
#define OMAP_RTC_STATUS_1D_EVENT	BIT(5)
#define OMAP_RTC_STATUS_1H_EVENT	BIT(4)
#define OMAP_RTC_STATUS_1M_EVENT	BIT(3)
#define OMAP_RTC_STATUS_1S_EVENT	BIT(2)
#define OMAP_RTC_STATUS_RUN		BIT(1)
#define OMAP_RTC_STATUS_BUSY		BIT(0)

/* OMAP_RTC_OSC_REG bit fields */
#define OMAP_RTC_OSC_32KCLK_EN		BIT(6)
#define OMAP_RTC_OSC_SEL_32KCLK_SRC	BIT(3)
#define OMAP_RTC_OSC_OSC32K_GZ_DISABLE	BIT(4)

/* OMAP_RTC_KICKER values */
#define	OMAP_RTC_KICK0_VALUE		0x83e70b13
#define	OMAP_RTC_KICK1_VALUE		0x95a4f1e0

struct omap_rtc_device_type {
	bool has_32kclk_en;
	bool has_irqwakeen;
	bool has_pmic_mode;
	bool has_power_up_reset;
};

struct omap_rtc_priv {
	fdt_addr_t base;
	u8 max_reg;
	struct udevice *dev;
	struct clk clk;
	bool has_ext_clk;
	const struct omap_rtc_device_type *type;
};

static inline u8 omap_rtc_readb(struct omap_rtc_priv *priv, unsigned int reg)
{
	return readb(priv->base + reg);
}

static inline u32 omap_rtc_readl(struct omap_rtc_priv *priv, unsigned int reg)
{
	return readl(priv->base + reg);
}

static inline void omap_rtc_writeb(struct omap_rtc_priv *priv, unsigned int reg,
				   u8 val)
{
	writeb(val, priv->base + reg);
}

static inline void omap_rtc_writel(struct omap_rtc_priv *priv, unsigned int reg,
				   u32 val)
{
	writel(val, priv->base + reg);
}

static inline void omap_rtc_unlock(struct omap_rtc_priv *priv)
{
	omap_rtc_writel(priv, OMAP_RTC_KICK0_REG, OMAP_RTC_KICK0_VALUE);
	omap_rtc_writel(priv, OMAP_RTC_KICK1_REG, OMAP_RTC_KICK1_VALUE);
}

static inline void omap_rtc_lock(struct omap_rtc_priv *priv)
{
	omap_rtc_writel(priv, OMAP_RTC_KICK0_REG, 0);
	omap_rtc_writel(priv, OMAP_RTC_KICK1_REG, 0);
}

static int omap_rtc_wait_not_busy(struct omap_rtc_priv *priv)
{
	int count;
	u8 status;

	status = omap_rtc_readb(priv, OMAP_RTC_STATUS_REG);
	if ((status & OMAP_RTC_STATUS_RUN) != OMAP_RTC_STATUS_RUN) {
		printf("RTC doesn't run\n");
		return -1;
	}

	/* BUSY may stay active for 1/32768 second (~30 usec) */
	for (count = 0; count < 50; count++) {
		if (!(status & OMAP_RTC_STATUS_BUSY))
			break;

		udelay(1);
		status = omap_rtc_readb(priv, OMAP_RTC_STATUS_REG);
	}

	/* now we have ~15 usec to read/write various registers */
	return 0;
}

static int omap_rtc_reset(struct udevice *dev)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);

	/* run RTC counter */
	omap_rtc_writeb(priv, OMAP_RTC_CTRL_REG, 0x01);
	return 0;
}

static int omap_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = omap_rtc_wait_not_busy(priv);
	if (ret)
		return ret;

	omap_rtc_unlock(priv);
	omap_rtc_writeb(priv, OMAP_RTC_YEARS_REG, bin2bcd(tm->tm_year % 100));
	omap_rtc_writeb(priv, OMAP_RTC_MONTHS_REG, bin2bcd(tm->tm_mon));
	omap_rtc_writeb(priv, OMAP_RTC_WEEKS_REG, bin2bcd(tm->tm_wday));
	omap_rtc_writeb(priv, OMAP_RTC_DAYS_REG, bin2bcd(tm->tm_mday));
	omap_rtc_writeb(priv, OMAP_RTC_HOURS_REG, bin2bcd(tm->tm_hour));
	omap_rtc_writeb(priv, OMAP_RTC_MINUTES_REG, bin2bcd(tm->tm_min));
	omap_rtc_writeb(priv, OMAP_RTC_SECONDS_REG, bin2bcd(tm->tm_sec));
	omap_rtc_lock(priv);

	dev_dbg(dev, "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday, tm->tm_hour,
		tm->tm_min, tm->tm_sec);

	return 0;
}

static int omap_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);
	unsigned long sec, min, hour, mday, wday, mon_cent, year;
	int ret;

	ret = omap_rtc_wait_not_busy(priv);
	if (ret)
		return ret;

	sec = omap_rtc_readb(priv, OMAP_RTC_SECONDS_REG);
	min = omap_rtc_readb(priv, OMAP_RTC_MINUTES_REG);
	hour = omap_rtc_readb(priv, OMAP_RTC_HOURS_REG);
	mday = omap_rtc_readb(priv, OMAP_RTC_DAYS_REG);
	wday = omap_rtc_readb(priv, OMAP_RTC_WEEKS_REG);
	mon_cent = omap_rtc_readb(priv, OMAP_RTC_MONTHS_REG);
	year = omap_rtc_readb(priv, OMAP_RTC_YEARS_REG);

	dev_dbg(dev,
		"Get RTC year: %02lx mon/cent: %02lx mday: %02lx wday: %02lx "
		"hr: %02lx min: %02lx sec: %02lx\n",
		year, mon_cent, mday, wday,
		hour, min, sec);

	tm->tm_sec  = bcd2bin(sec  & 0x7F);
	tm->tm_min  = bcd2bin(min  & 0x7F);
	tm->tm_hour = bcd2bin(hour & 0x3F);
	tm->tm_mday = bcd2bin(mday & 0x3F);
	tm->tm_mon  = bcd2bin(mon_cent & 0x1F);
	tm->tm_year = bcd2bin(year) + 2000;
	tm->tm_wday = bcd2bin(wday & 0x07);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	dev_dbg(dev, "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday, tm->tm_hour,
		tm->tm_min, tm->tm_sec);

	return 0;
}

static int omap_rtc_scratch_read(struct udevice *dev, uint offset,
				 u8 *buffer, uint len)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);
	u32 *val = (u32 *)buffer;
	unsigned int reg;
	int i;

	if (len & 3)
		return -EFAULT;

	for (i = 0; i < len / 4; i++) {
		reg = OMAP_RTC_SCRATCH0_REG + offset + (i * 4);
		if (reg >= OMAP_RTC_KICK0_REG)
			return -EFAULT;

		val[i] = omap_rtc_readl(priv, reg);
	}

	return 0;
}

static int omap_rtc_scratch_write(struct udevice *dev, uint offset,
				  const u8 *buffer, uint len)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);
	u32 *val = (u32 *)buffer;
	unsigned int reg;
	int i;

	if (len & 3)
		return -EFAULT;

	omap_rtc_unlock(priv);
	for (i = 0; i < len / 4; i++) {
		reg = OMAP_RTC_SCRATCH0_REG + offset + (i * 4);
		if (reg >= OMAP_RTC_KICK0_REG)
			return -EFAULT;

		omap_rtc_writel(priv, reg, val[i]);
	}
	omap_rtc_lock(priv);

	return 0;
}

static int omap_rtc_remove(struct udevice *dev)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);
	u8 reg;

	if (priv->clk.dev)
		clk_disable(&priv->clk);

	omap_rtc_unlock(priv);

	/* leave rtc running, but disable irqs */
	omap_rtc_writeb(priv, OMAP_RTC_INTERRUPTS_REG, 0);

	if (priv->has_ext_clk) {
		reg = omap_rtc_readb(priv, OMAP_RTC_OSC_REG);
		reg &= ~OMAP_RTC_OSC_SEL_32KCLK_SRC;
		omap_rtc_writeb(priv, OMAP_RTC_OSC_REG, reg);
	}

	omap_rtc_lock(priv);
	return 0;
}

static int omap_rtc_probe(struct udevice *dev)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);
	struct rtc_time tm;
	u8 reg, mask, new_ctrl;

	priv->dev = dev;
	priv->type = (struct omap_rtc_device_type *)dev_get_driver_data(dev);
	priv->max_reg = OMAP_RTC_PMIC_REG;

	if (!clk_get_by_name(dev, "ext-clk", &priv->clk))
		priv->has_ext_clk = true;
	else
		clk_get_by_name(dev, "int-clk", &priv->clk);

	if (priv->clk.dev)
		clk_enable(&priv->clk);
	else
		dev_warn(dev, "missing clock\n");

	omap_rtc_unlock(priv);

	/*
	 * disable interrupts
	 *
	 * NOTE: ALARM2 is not cleared on AM3352 if rtc_write (writeb) is used
	 */
	omap_rtc_writel(priv, OMAP_RTC_INTERRUPTS_REG, 0);

	if (priv->type->has_32kclk_en) {
		reg = omap_rtc_readb(priv, OMAP_RTC_OSC_REG);
		omap_rtc_writeb(priv, OMAP_RTC_OSC_REG,
				reg | OMAP_RTC_OSC_32KCLK_EN);
	}

	/* clear old status */
	reg = omap_rtc_readb(priv, OMAP_RTC_STATUS_REG);

	mask = OMAP_RTC_STATUS_ALARM;

	if (priv->type->has_pmic_mode)
		mask |= OMAP_RTC_STATUS_ALARM2;

	if (priv->type->has_power_up_reset) {
		mask |= OMAP_RTC_STATUS_POWER_UP;
		if (reg & OMAP_RTC_STATUS_POWER_UP)
			dev_info(dev, "RTC power up reset detected\n");
	}

	if (reg & mask)
		omap_rtc_writeb(priv, OMAP_RTC_STATUS_REG, reg & mask);

	/* On boards with split power, RTC_ON_NOFF won't reset the RTC */
	reg = omap_rtc_readb(priv, OMAP_RTC_CTRL_REG);
	if (reg & OMAP_RTC_CTRL_STOP)
		dev_info(dev, "already running\n");

	/* force to 24 hour mode */
	new_ctrl = reg & (OMAP_RTC_CTRL_SPLIT | OMAP_RTC_CTRL_AUTO_COMP);
	new_ctrl |= OMAP_RTC_CTRL_STOP;

	/*
	 * BOARD-SPECIFIC CUSTOMIZATION CAN GO HERE:
	 *
	 *  - Device wake-up capability setting should come through chip
	 *    init logic. OMAP1 boards should initialize the "wakeup capable"
	 *    flag in the platform device if the board is wired right for
	 *    being woken up by RTC alarm. For OMAP-L138, this capability
	 *    is built into the SoC by the "Deep Sleep" capability.
	 *
	 *  - Boards wired so RTC_ON_nOFF is used as the reset signal,
	 *    rather than nPWRON_RESET, should forcibly enable split
	 *    power mode.  (Some chip errata report that RTC_CTRL_SPLIT
	 *    is write-only, and always reads as zero...)
	 */

	if (new_ctrl & OMAP_RTC_CTRL_SPLIT)
		dev_info(dev, "split power mode\n");

	if (reg != new_ctrl)
		omap_rtc_writeb(priv, OMAP_RTC_CTRL_REG, new_ctrl);

	/*
	 * If we have the external clock then switch to it so we can keep
	 * ticking across suspend.
	 */
	if (priv->has_ext_clk) {
		reg = omap_rtc_readb(priv, OMAP_RTC_OSC_REG);
		reg &= ~OMAP_RTC_OSC_OSC32K_GZ_DISABLE;
		reg |= OMAP_RTC_OSC_32KCLK_EN | OMAP_RTC_OSC_SEL_32KCLK_SRC;
		omap_rtc_writeb(priv, OMAP_RTC_OSC_REG, reg);
	}

	omap_rtc_lock(priv);

	if (omap_rtc_get(dev, &tm)) {
		dev_err(dev, "failed to get datetime\n");
	} else if (tm.tm_year == 2000 && tm.tm_mon == 1 && tm.tm_mday == 1 &&
		   tm.tm_wday == 0) {
		tm.tm_wday = 6;
		omap_rtc_set(dev, &tm);
	}

	return 0;
}

static int omap_rtc_of_to_plat(struct udevice *dev)
{
	struct omap_rtc_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE) {
		dev_err(dev, "invalid address\n");
		return -EINVAL;
	}

	dev_dbg(dev, "base=%pa\n", &priv->base);
	return 0;
}

static const struct rtc_ops omap_rtc_ops = {
	.get = omap_rtc_get,
	.set = omap_rtc_set,
	.reset = omap_rtc_reset,
	.read = omap_rtc_scratch_read,
	.write = omap_rtc_scratch_write,
};

static const struct omap_rtc_device_type omap_rtc_am3352_type = {
	.has_32kclk_en	= true,
	.has_irqwakeen	= true,
	.has_pmic_mode	= true,
};

static const struct omap_rtc_device_type omap_rtc_da830_type = {
	.has_32kclk_en	= false,
	.has_irqwakeen	= false,
	.has_pmic_mode	= false,
};

static const struct udevice_id omap_rtc_ids[] = {
	{.compatible = "ti,am3352-rtc", .data = (ulong)&omap_rtc_am3352_type},
	{.compatible = "ti,da830-rtc", .data = (ulong)&omap_rtc_da830_type }
};

U_BOOT_DRIVER(omap_rtc) = {
	.name = "omap_rtc",
	.id = UCLASS_RTC,
	.of_match = omap_rtc_ids,
	.ops = &omap_rtc_ops,
	.of_to_plat = omap_rtc_of_to_plat,
	.probe = omap_rtc_probe,
	.remove = omap_rtc_remove,
	.priv_auto = sizeof(struct omap_rtc_priv),
};
