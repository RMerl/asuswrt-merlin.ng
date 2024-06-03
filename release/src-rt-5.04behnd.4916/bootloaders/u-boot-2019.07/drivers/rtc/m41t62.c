// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de.
 *
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * based on a the Linux rtc-m41t80.c driver which is:
 *   Alexander Bigga <ab@mycable.de>, 2006 (c) mycable GmbH
 */

/*
 * Date & Time support for STMicroelectronics M41T62
 */

/* #define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <i2c.h>

#define M41T62_REG_SSEC	0
#define M41T62_REG_SEC	1
#define M41T62_REG_MIN	2
#define M41T62_REG_HOUR	3
#define M41T62_REG_WDAY	4
#define M41T62_REG_DAY	5
#define M41T62_REG_MON	6
#define M41T62_REG_YEAR	7
#define M41T62_REG_ALARM_MON	0xa
#define M41T62_REG_ALARM_DAY	0xb
#define M41T62_REG_ALARM_HOUR	0xc
#define M41T62_REG_ALARM_MIN	0xd
#define M41T62_REG_ALARM_SEC	0xe
#define M41T62_REG_FLAGS	0xf

#define M41T62_DATETIME_REG_SIZE	(M41T62_REG_YEAR + 1)
#define M41T62_ALARM_REG_SIZE	\
	(M41T62_REG_ALARM_SEC + 1 - M41T62_REG_ALARM_MON)

#define M41T62_SEC_ST		(1 << 7)	/* ST: Stop Bit */
#define M41T62_ALMON_AFE	(1 << 7)	/* AFE: AF Enable Bit */
#define M41T62_ALMON_SQWE	(1 << 6)	/* SQWE: SQW Enable Bit */
#define M41T62_ALHOUR_HT	(1 << 6)	/* HT: Halt Update Bit */
#define M41T62_FLAGS_AF		(1 << 6)	/* AF: Alarm Flag Bit */
#define M41T62_FLAGS_BATT_LOW	(1 << 4)	/* BL: Battery Low Bit */

#define M41T62_FEATURE_HT	(1 << 0)
#define M41T62_FEATURE_BL	(1 << 1)

#define M41T80_ALHOUR_HT	(1 << 6)	/* HT: Halt Update Bit */

static void m41t62_update_rtc_time(struct rtc_time *tm, u8 *buf)
{
	debug("%s: raw read data - sec=%02x, min=%02x, hr=%02x, "
	      "mday=%02x, mon=%02x, year=%02x, wday=%02x, y2k=%02x\n",
	      __FUNCTION__,
	      buf[0], buf[1], buf[2], buf[3],
	      buf[4], buf[5], buf[6], buf[7]);

	tm->tm_sec = bcd2bin(buf[M41T62_REG_SEC] & 0x7f);
	tm->tm_min = bcd2bin(buf[M41T62_REG_MIN] & 0x7f);
	tm->tm_hour = bcd2bin(buf[M41T62_REG_HOUR] & 0x3f);
	tm->tm_mday = bcd2bin(buf[M41T62_REG_DAY] & 0x3f);
	tm->tm_wday = buf[M41T62_REG_WDAY] & 0x07;
	tm->tm_mon = bcd2bin(buf[M41T62_REG_MON] & 0x1f);

	/* assume 20YY not 19YY, and ignore the Century Bit */
	/* U-Boot needs to add 1900 here */
	tm->tm_year = bcd2bin(buf[M41T62_REG_YEAR]) + 100 + 1900;

	debug("%s: tm is secs=%d, mins=%d, hours=%d, "
	      "mday=%d, mon=%d, year=%d, wday=%d\n",
	      __FUNCTION__,
	      tm->tm_sec, tm->tm_min, tm->tm_hour,
	      tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);
}

static void m41t62_set_rtc_buf(const struct rtc_time *tm, u8 *buf)
{
	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	/* Merge time-data and register flags into buf[0..7] */
	buf[M41T62_REG_SSEC] = 0;
	buf[M41T62_REG_SEC] =
		bin2bcd(tm->tm_sec) | (buf[M41T62_REG_SEC] & ~0x7f);
	buf[M41T62_REG_MIN] =
		bin2bcd(tm->tm_min) | (buf[M41T62_REG_MIN] & ~0x7f);
	buf[M41T62_REG_HOUR] =
		bin2bcd(tm->tm_hour) | (buf[M41T62_REG_HOUR] & ~0x3f) ;
	buf[M41T62_REG_WDAY] =
		(tm->tm_wday & 0x07) | (buf[M41T62_REG_WDAY] & ~0x07);
	buf[M41T62_REG_DAY] =
		bin2bcd(tm->tm_mday) | (buf[M41T62_REG_DAY] & ~0x3f);
	buf[M41T62_REG_MON] =
		bin2bcd(tm->tm_mon) | (buf[M41T62_REG_MON] & ~0x1f);
	/* assume 20YY not 19YY */
	buf[M41T62_REG_YEAR] = bin2bcd(tm->tm_year % 100);
}

#ifdef CONFIG_DM_RTC
static int m41t62_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	u8 buf[M41T62_DATETIME_REG_SIZE];
	int ret;

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret)
		return ret;

	m41t62_update_rtc_time(tm, buf);

	return 0;
}

static int m41t62_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	u8 buf[M41T62_DATETIME_REG_SIZE];
	int ret;

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret)
		return ret;

	m41t62_set_rtc_buf(tm, buf);

	ret = dm_i2c_write(dev, 0, buf, sizeof(buf));
	if (ret) {
		printf("I2C write failed in %s()\n", __func__);
		return ret;
	}

	return 0;
}

static int m41t62_rtc_reset(struct udevice *dev)
{
	u8 val;

	/*
	 * M41T82: Make sure HT (Halt Update) bit is cleared.
	 * This bit is 0 in M41T62 so its save to clear it always.
	 */

	int ret = dm_i2c_read(dev, M41T62_REG_ALARM_HOUR, &val, sizeof(val));

	val &= ~M41T80_ALHOUR_HT;
	ret |= dm_i2c_write(dev, M41T62_REG_ALARM_HOUR, &val, sizeof(val));

	return ret;
}

/*
 * Make sure HT bit is cleared. This bit is set on entering battery backup
 * mode, so do this before the first read access.
 */
static int m41t62_rtc_probe(struct udevice *dev)
{
	return m41t62_rtc_reset(dev);
}

static const struct rtc_ops m41t62_rtc_ops = {
	.get = m41t62_rtc_get,
	.set = m41t62_rtc_set,
	.reset = m41t62_rtc_reset,
};

static const struct udevice_id m41t62_rtc_ids[] = {
	{ .compatible = "st,m41t62" },
	{ .compatible = "st,m41t82" },
	{ .compatible = "microcrystal,rv4162" },
	{ }
};

U_BOOT_DRIVER(rtc_m41t62) = {
	.name	= "rtc-m41t62",
	.id	= UCLASS_RTC,
	.of_match = m41t62_rtc_ids,
	.ops	= &m41t62_rtc_ops,
	.probe  = &m41t62_rtc_probe,
};

#else /* NON DM RTC code - will be removed */
int rtc_get(struct rtc_time *tm)
{
	u8 buf[M41T62_DATETIME_REG_SIZE];

	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, buf, M41T62_DATETIME_REG_SIZE);
	m41t62_update_rtc_time(tm, buf);

	return 0;
}

int rtc_set(struct rtc_time *tm)
{
	u8 buf[M41T62_DATETIME_REG_SIZE];

	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, buf, M41T62_DATETIME_REG_SIZE);
	m41t62_set_rtc_buf(tm, buf);

	if (i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0, 1, buf,
		      M41T62_DATETIME_REG_SIZE)) {
		printf("I2C write failed in %s()\n", __func__);
		return -1;
	}

	return 0;
}

void rtc_reset(void)
{
	u8 val;

	/*
	 * M41T82: Make sure HT (Halt Update) bit is cleared.
	 * This bit is 0 in M41T62 so its save to clear it always.
	 */
	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, M41T62_REG_ALARM_HOUR, 1, &val, 1);
	val &= ~M41T80_ALHOUR_HT;
	i2c_write(CONFIG_SYS_I2C_RTC_ADDR, M41T62_REG_ALARM_HOUR, 1, &val, 1);
}
#endif /* CONFIG_DM_RTC */
