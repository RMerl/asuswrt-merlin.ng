// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Theobroma Systems Design und Consulting GmbH
 *
 * Based on a the Linux rtc-rv3029c2.c driver written by:
 *   Gregory Hermant <gregory.hermant@calao-systems.com>
 *   Michael Buesch <m@bues.ch>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <eeprom.h>
#include <i2c.h>
#include <log.h>
#include <rtc.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define RTC_RV3029_PAGE_LEN             7

/* control section */
#define RV3029_ONOFF_CTRL		0x00
#define RV3029_ONOFF_CTRL_WE		BIT(0)
#define RV3029_ONOFF_CTRL_TE		BIT(1)
#define RV3029_ONOFF_CTRL_TAR		BIT(2)
#define RV3029_ONOFF_CTRL_EERE		BIT(3)
#define RV3029_ONOFF_CTRL_SRON		BIT(4)
#define RV3029_ONOFF_CTRL_TD0		BIT(5)
#define RV3029_ONOFF_CTRL_TD1		BIT(6)
#define RV3029_ONOFF_CTRL_CLKINT	BIT(7)
#define RV3029_IRQ_CTRL			0x01
#define RV3029_IRQ_CTRL_AIE		BIT(0)
#define RV3029_IRQ_CTRL_TIE		BIT(1)
#define RV3029_IRQ_CTRL_V1IE		BIT(2)
#define RV3029_IRQ_CTRL_V2IE		BIT(3)
#define RV3029_IRQ_CTRL_SRIE		BIT(4)
#define RV3029_IRQ_FLAGS		0x02
#define RV3029_IRQ_FLAGS_AF		BIT(0)
#define RV3029_IRQ_FLAGS_TF		BIT(1)
#define RV3029_IRQ_FLAGS_V1IF		BIT(2)
#define RV3029_IRQ_FLAGS_V2IF		BIT(3)
#define RV3029_IRQ_FLAGS_SRF		BIT(4)
#define RV3029_STATUS			0x03
#define RV3029_STATUS_VLOW1		BIT(2)
#define RV3029_STATUS_VLOW2		BIT(3)
#define RV3029_STATUS_SR		BIT(4)
#define RV3029_STATUS_PON		BIT(5)
#define RV3029_STATUS_EEBUSY		BIT(7)
#define RV3029_RST_CTRL			0x04
#define RV3029_RST_CTRL_SYSR		BIT(4)
#define RV3029_CONTROL_SECTION_LEN	0x05

/* watch section */
#define RV3029_W_SEC			0x08
#define RV3029_W_MINUTES		0x09
#define RV3029_W_HOURS			0x0A
#define RV3029_REG_HR_12_24		BIT(6) /* 24h/12h mode */
#define RV3029_REG_HR_PM		BIT(5) /* PM/AM bit in 12h mode */
#define RV3029_W_DATE			0x0B
#define RV3029_W_DAYS			0x0C
#define RV3029_W_MONTHS			0x0D
#define RV3029_W_YEARS			0x0E

/* eeprom control section */
#define RV3029_CONTROL_E2P_EECTRL	0x30
#define RV3029_TRICKLE_1K		BIT(4) /* 1.5K resistance */
#define RV3029_TRICKLE_5K		BIT(5) /* 5K   resistance */
#define RV3029_TRICKLE_20K		BIT(6) /* 20K  resistance */
#define RV3029_TRICKLE_80K		BIT(7) /* 80K  resistance */
#define RV3029_TRICKLE_MASK		(RV3029_TRICKLE_1K |\
					 RV3029_TRICKLE_5K |\
					 RV3029_TRICKLE_20K |\
					 RV3029_TRICKLE_80K)
#define RV3029_TRICKLE_SHIFT		4


static int rv3029_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	u8 regs[RTC_RV3029_PAGE_LEN];
	int ret;

	ret = dm_i2c_read(dev, RV3029_W_SEC, regs, sizeof(regs));
	if (ret < 0) {
		printf("%s: error reading RTC: %x\n", __func__, ret);
		return -EIO;
	}

	tm->tm_sec = bcd2bin(regs[RV3029_W_SEC - RV3029_W_SEC]);
	tm->tm_min = bcd2bin(regs[RV3029_W_MINUTES - RV3029_W_SEC]);

	/* HR field has a more complex interpretation */
	{
		const u8 _hr = regs[RV3029_W_HOURS - RV3029_W_SEC];

		if (_hr & RV3029_REG_HR_12_24) {
			/* 12h format */
			tm->tm_hour = bcd2bin(_hr & 0x1f);
			if (_hr & RV3029_REG_HR_PM)	/* PM flag set */
				tm->tm_hour += 12;
		} else {
			/* 24h format */
			tm->tm_hour = bcd2bin(_hr & 0x3f);
		}
	}

	tm->tm_mday = bcd2bin(regs[RV3029_W_DATE - RV3029_W_SEC]);
	tm->tm_mon = bcd2bin(regs[RV3029_W_MONTHS - RV3029_W_SEC]) - 1;
	/* RTC supports only years > 1999 */
	tm->tm_year = bcd2bin(regs[RV3029_W_YEARS - RV3029_W_SEC]) + 2000;
	tm->tm_wday = bcd2bin(regs[RV3029_W_DAYS - RV3029_W_SEC]) - 1;

	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("%s: %4d-%02d-%02d (wday=%d) %2d:%02d:%02d\n",
	      __func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
	      tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int rv3029_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	u8 regs[RTC_RV3029_PAGE_LEN];

	debug("%s: %4d-%02d-%02d (wday=%d( %2d:%02d:%02d\n",
	      __func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
	      tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);


	if (tm->tm_year < 2000) {
		printf("%s: year %d (before 2000) not supported\n",
		       __func__, tm->tm_year);
		return -EINVAL;
	}

	regs[RV3029_W_SEC - RV3029_W_SEC] = bin2bcd(tm->tm_sec);
	regs[RV3029_W_MINUTES - RV3029_W_SEC] = bin2bcd(tm->tm_min);
	regs[RV3029_W_HOURS - RV3029_W_SEC] = bin2bcd(tm->tm_hour);
	regs[RV3029_W_DATE - RV3029_W_SEC] = bin2bcd(tm->tm_mday);
	regs[RV3029_W_MONTHS - RV3029_W_SEC] = bin2bcd(tm->tm_mon + 1);
	regs[RV3029_W_DAYS - RV3029_W_SEC] = bin2bcd(tm->tm_wday + 1) & 0x7;
	regs[RV3029_W_YEARS - RV3029_W_SEC] = bin2bcd(tm->tm_year - 2000);

	return dm_i2c_write(dev, RV3029_W_SEC, regs, sizeof(regs));
}

static int rv3029_rtc_reset(struct udevice *dev)
{
	u8 ctrl = RV3029_RST_CTRL_SYSR;
	unsigned long start;
	const unsigned long timeout_ms = 10000;
	int ret;

	/* trigger the system-reset */
	ret = dm_i2c_write(dev, RV3029_RST_CTRL, &ctrl, 1);
	if (ret < 0)
		return -EIO;

	/* wait for the system-reset to complete */
	start = get_timer(0);
	do {
		if (get_timer(start) > timeout_ms)
			return -ETIMEDOUT;

		ret = dm_i2c_read(dev, RV3029_RST_CTRL, &ctrl, 1);
		if (ret < 0)
			return -EIO;
	} while (ctrl & RV3029_RST_CTRL_SYSR);

	return 0;
}

static int rv3029_rtc_read8(struct udevice *dev, unsigned int reg)
{
	u8 data;
	int ret;

	ret = dm_i2c_read(dev, reg, &data, sizeof(data));
	return ret < 0 ? ret : data;
}

static int rv3029_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	u8 data = val;

	return dm_i2c_write(dev, reg, &data, 1);
}

#if defined(OF_CONTROL)
static int rv3029_get_sr(struct udevice *dev, u8 *buf)
{
	int ret = dm_i2c_read(dev, RV3029_STATUS, buf, 1);

	if (ret < 0)
		return -EIO;

	dev_dbg(dev, "status = 0x%.2x (%d)\n", buf[0], buf[0]);
	return 0;
}

static int rv3029_set_sr(struct udevice *dev, u8 val)
{
	int ret;

	ret = dm_i2c_read(dev, RV3029_STATUS, &val, 1);
	if (ret < 0)
		return -EIO;

	dev_dbg(dev, "status = 0x%.2x (%d)\n", val, val);
	return 0;
}

static int rv3029_eeprom_busywait(struct udevice *dev)
{
	int i, ret;
	u8 sr;

	for (i = 100; i > 0; i--) {
		ret = rv3029_get_sr(dev, &sr);
		if (ret < 0)
			break;
		if (!(sr & RV3029_STATUS_EEBUSY))
			break;
		udelay(10000);
	}
	if (i <= 0) {
		dev_err(dev, "EEPROM busy wait timeout.\n");
		return -ETIMEDOUT;
	}

	return ret;
}

static int rv3029_update_bits(struct udevice *dev, u8 reg, u8 mask, u8 set)
{
	u8 buf;
	int ret;

	ret = dm_i2c_read(dev, reg, &buf, 1);
	if (ret < 0)
		return ret;

	if ((buf & mask) == (set && mask))
		return 0;

	buf = (buf & ~mask) | (set & mask);
	ret = dm_i2c_read(dev, reg, &buf, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int rv3029_eeprom_exit(struct udevice *dev)
{
	/* Re-enable eeprom refresh */
	return rv3029_update_bits(dev, RV3029_ONOFF_CTRL,
				  RV3029_ONOFF_CTRL_EERE,
				  RV3029_ONOFF_CTRL_EERE);
}

static int rv3029_eeprom_enter(struct udevice *dev)
{
	int ret;
	u8 sr;

	/* Check whether we are in the allowed voltage range. */
	ret = rv3029_get_sr(dev, &sr);
	if (ret < 0)
		return ret;
	if (sr & (RV3029_STATUS_VLOW1 | RV3029_STATUS_VLOW2)) {
		/* We clear the bits and retry once just in case
		 * we had a brown out in early startup.
		 */
		sr &= ~RV3029_STATUS_VLOW1;
		sr &= ~RV3029_STATUS_VLOW2;
		ret = rv3029_set_sr(dev, sr);
		if (ret < 0)
			return ret;
		udelay(10000);
		ret = rv3029_get_sr(dev, &sr);
		if (ret < 0)
			return ret;
		if (sr & (RV3029_STATUS_VLOW1 | RV3029_STATUS_VLOW2)) {
			dev_err(dev, "Supply voltage is too low to safely access the EEPROM.\n");
			return -ENODEV;
		}
	}

	/* Disable eeprom refresh. */
	ret = rv3029_update_bits(dev,
				 RV3029_ONOFF_CTRL, RV3029_ONOFF_CTRL_EERE, 0);
	if (ret < 0)
		return ret;

	/* Wait for any previous eeprom accesses to finish. */
	ret = rv3029_eeprom_busywait(dev);
	if (ret < 0)
		rv3029_eeprom_exit(dev);

	return ret;
}

static int rv3029_eeprom_read(struct udevice *dev, u8 reg,
			      u8 buf[], size_t len)
{
	int ret, err;

	err = rv3029_eeprom_enter(dev);
	if (err < 0)
		return err;

	ret = dm_i2c_read(dev, reg, buf, len);

	err = rv3029_eeprom_exit(dev);
	if (err < 0)
		return err;

	return ret;
}

static int rv3029_eeprom_write(struct udevice *dev, u8 reg,
			       u8 const buf[], size_t len)
{
	int ret;
	size_t i;
	u8 tmp;

	ret = rv3029_eeprom_enter(dev);
	if (ret < 0)
		return ret;

	for (i = 0; i < len; i++, reg++) {
		ret = dm_i2c_read(dev, reg, &tmp, 1);
		if (ret < 0)
			break;
		if (tmp != buf[i]) {
			ret = dm_i2c_write(dev, reg, &buf[i], 1);
			if (ret < 0)
				break;
		}
		ret = rv3029_eeprom_busywait(dev);
		if (ret < 0)
			break;
	}

	ret = rv3029_eeprom_exit(dev);
	if (ret < 0)
		return ret;

	return 0;
}

static int rv3029_eeprom_update_bits(struct udevice *dev,
				     u8 reg, u8 mask, u8 set)
{
	u8 buf;
	int ret;

	ret = rv3029_eeprom_read(dev, reg, &buf, 1);
	if (ret < 0)
		return ret;

	/*
	 * If the EEPROM already reads the correct bitpattern, we don't need
	 * to update it.
	 */
	if ((buf & mask) == (set & mask))
		return 0;

	buf = (buf & ~mask) | (set & mask);
	ret = rv3029_eeprom_write(dev, reg, &buf, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static void rv3029_trickle_config(struct udevice *dev)
{
	static const struct rv3029_trickle_tab_elem {
		u32 r;		/* resistance in ohms */
		u8 conf;	/* trickle config bits */
	} rv3029_trickle_tab[] = {
		{
			.r	= 1076,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_5K |
				  RV3029_TRICKLE_20K | RV3029_TRICKLE_80K,
		}, {
			.r	= 1091,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_5K |
				  RV3029_TRICKLE_20K,
		}, {
			.r	= 1137,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_5K |
				  RV3029_TRICKLE_80K,
		}, {
			.r	= 1154,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_5K,
		}, {
			.r	= 1371,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_20K |
				  RV3029_TRICKLE_80K,
		}, {
			.r	= 1395,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_20K,
		}, {
			.r	= 1472,
			.conf	= RV3029_TRICKLE_1K | RV3029_TRICKLE_80K,
		}, {
			.r	= 1500,
			.conf	= RV3029_TRICKLE_1K,
		}, {
			.r	= 3810,
			.conf	= RV3029_TRICKLE_5K | RV3029_TRICKLE_20K |
				  RV3029_TRICKLE_80K,
		}, {
			.r	= 4000,
			.conf	= RV3029_TRICKLE_5K | RV3029_TRICKLE_20K,
		}, {
			.r	= 4706,
			.conf	= RV3029_TRICKLE_5K | RV3029_TRICKLE_80K,
		}, {
			.r	= 5000,
			.conf	= RV3029_TRICKLE_5K,
		}, {
			.r	= 16000,
			.conf	= RV3029_TRICKLE_20K | RV3029_TRICKLE_80K,
		}, {
			.r	= 20000,
			.conf	= RV3029_TRICKLE_20K,
		}, {
			.r	= 80000,
			.conf	= RV3029_TRICKLE_80K,
		},
	};
	int err;
	u32 ohms;
	u8 trickle_set_bits = 0;

	/* Configure the trickle charger. */
	err = dev_read_u32(dev, "trickle-resistor-ohms", &ohms);

	if (!err) {
		/* Find trickle-charger config */
		for (int i = 0; i < ARRAY_SIZE(rv3029_trickle_tab); i++)
			if (rv3029_trickle_tab[i].r >= ohms) {
				dev_dbg(dev, "trickle charger at %d ohms\n",
					rv3029_trickle_tab[i].r);
				trickle_set_bits = rv3029_trickle_tab[i].conf;
				break;
			}
	}

	dev_dbg(dev, "trickle charger config 0x%x\n", trickle_set_bits);
	err = rv3029_eeprom_update_bits(dev, RV3029_CONTROL_E2P_EECTRL,
					RV3029_TRICKLE_MASK,
					trickle_set_bits);
	if (err < 0)
		dev_dbg(dev, "failed to update trickle charger\n");
}
#else
static inline void rv3029_trickle_config(struct udevice *dev)
{
}
#endif

static int rv3029_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
				DM_I2C_CHIP_WR_ADDRESS);

	rv3029_trickle_config(dev);
	return 0;
}

static const struct rtc_ops rv3029_rtc_ops = {
	.get = rv3029_rtc_get,
	.set = rv3029_rtc_set,
	.read8 = rv3029_rtc_read8,
	.write8 = rv3029_rtc_write8,
	.reset = rv3029_rtc_reset,
};

static const struct udevice_id rv3029_rtc_ids[] = {
	{ .compatible = "mc,rv3029" },
	{ .compatible = "mc,rv3029c2" },
	{ }
};

U_BOOT_DRIVER(rtc_rv3029) = {
	.name	= "rtc-rv3029",
	.id	= UCLASS_RTC,
	.probe	= rv3029_probe,
	.of_match = rv3029_rtc_ids,
	.ops	= &rv3029_rtc_ops,
};
