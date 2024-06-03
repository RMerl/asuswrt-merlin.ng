/*
 * Epson RX8010 RTC driver.
 *
 * Copyright (c) 2017, General Electric Company
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <command.h>
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>

/*---------------------------------------------------------------------*/
/* #undef DEBUG_RTC */

#ifdef DEBUG_RTC
#define DEBUGR(fmt, args...) printf(fmt, ##args)
#else
#define DEBUGR(fmt, args...)
#endif
/*---------------------------------------------------------------------*/

#ifndef CONFIG_SYS_I2C_RTC_ADDR
# define CONFIG_SYS_I2C_RTC_ADDR	0x32
#endif

/*
 * RTC register addresses
 */
#define RX8010_SEC     0x10
#define RX8010_MIN     0x11
#define RX8010_HOUR    0x12
#define RX8010_WDAY    0x13
#define RX8010_MDAY    0x14
#define RX8010_MONTH   0x15
#define RX8010_YEAR    0x16
#define RX8010_YEAR    0x16
#define RX8010_RESV17  0x17
#define RX8010_ALMIN   0x18
#define RX8010_ALHOUR  0x19
#define RX8010_ALWDAY  0x1A
#define RX8010_TCOUNT0 0x1B
#define RX8010_TCOUNT1 0x1C
#define RX8010_EXT     0x1D
#define RX8010_FLAG    0x1E
#define RX8010_CTRL    0x1F
/* 0x20 to 0x2F are user registers */
#define RX8010_RESV30  0x30
#define RX8010_RESV31  0x32
#define RX8010_IRQ     0x32

#define RX8010_EXT_WADA  BIT(3)

#define RX8010_FLAG_VLF  BIT(1)
#define RX8010_FLAG_AF   BIT(3)
#define RX8010_FLAG_TF   BIT(4)
#define RX8010_FLAG_UF   BIT(5)

#define RX8010_CTRL_AIE  BIT(3)
#define RX8010_CTRL_UIE  BIT(5)
#define RX8010_CTRL_STOP BIT(6)
#define RX8010_CTRL_TEST BIT(7)

#define RX8010_ALARM_AE  BIT(7)

#ifdef CONFIG_DM_RTC

#define DEV_TYPE struct udevice

#else

/* Local udevice */
struct ludevice {
	u8 chip;
};

#define DEV_TYPE struct ludevice

#endif

static int rx8010sj_rtc_read8(DEV_TYPE *dev, unsigned int reg)
{
	u8 val;
	int ret;

#ifdef CONFIG_DM_RTC
	ret = dm_i2c_read(dev, reg, &val, sizeof(val));
#else
	ret = i2c_read(dev->chip, reg, 1, &val, 1);
#endif

	return ret < 0 ? ret : val;
}

static int rx8010sj_rtc_write8(DEV_TYPE *dev, unsigned int reg, int val)
{
	int ret;
	u8 lval = val;

#ifdef CONFIG_DM_RTC
	ret = dm_i2c_write(dev, reg, &lval, 1);
#else
	ret = i2c_write(dev->chip, reg, 1, &lval, 1);
#endif

	return ret < 0 ? ret : 0;
}

static int validate_time(const struct rtc_time *tm)
{
	if ((tm->tm_year < 2000) || (tm->tm_year > 2099))
		return -EINVAL;

	if ((tm->tm_mon < 1) || (tm->tm_mon > 12))
		return -EINVAL;

	if ((tm->tm_mday < 1) || (tm->tm_mday > 31))
		return -EINVAL;

	if ((tm->tm_wday < 0) || (tm->tm_wday > 6))
		return -EINVAL;

	if ((tm->tm_hour < 0) || (tm->tm_hour > 23))
		return -EINVAL;

	if ((tm->tm_min < 0) || (tm->tm_min > 59))
		return -EINVAL;

	if ((tm->tm_sec < 0) || (tm->tm_sec > 59))
		return -EINVAL;

	return 0;
}

void rx8010sj_rtc_init(DEV_TYPE *dev)
{
	u8 ctrl[2];
	int need_clear = 0, ret = 0;

	/* Initialize reserved registers as specified in datasheet */
	ret = rx8010sj_rtc_write8(dev, RX8010_RESV17, 0xD8);
	if (ret < 0)
		goto error;

	ret = rx8010sj_rtc_write8(dev, RX8010_RESV30, 0x00);
	if (ret < 0)
		goto error;

	ret = rx8010sj_rtc_write8(dev, RX8010_RESV31, 0x08);
	if (ret < 0)
		goto error;

	ret = rx8010sj_rtc_write8(dev, RX8010_IRQ, 0x00);
	if (ret < 0)
		goto error;

	for (int i = 0; i < 2; i++) {
		ret = rx8010sj_rtc_read8(dev, RX8010_FLAG + i);
		if (ret < 0)
			goto error;

		ctrl[i] = ret;
	}

	if (ctrl[0] & RX8010_FLAG_VLF)
		printf("RTC low voltage detected\n");

	if (ctrl[0] & RX8010_FLAG_AF) {
		printf("Alarm was detected\n");
		need_clear = 1;
	}

	if (ctrl[0] & RX8010_FLAG_TF)
		need_clear = 1;

	if (ctrl[0] & RX8010_FLAG_UF)
		need_clear = 1;

	if (need_clear) {
		ctrl[0] &= ~(RX8010_FLAG_AF | RX8010_FLAG_TF | RX8010_FLAG_UF);
		ret = rx8010sj_rtc_write8(dev, RX8010_FLAG, ctrl[0]);
		if (ret < 0)
			goto error;
	}

	return;

error:
	printf("Error rtc init.\n");
}

/* Get the current time from the RTC */
static int rx8010sj_rtc_get(DEV_TYPE *dev, struct rtc_time *tmp)
{
	u8 date[7];
	int flagreg;
	int ret;

	flagreg = rx8010sj_rtc_read8(dev, RX8010_FLAG);
	if (flagreg < 0) {
		DEBUGR("Error reading from RTC. err: %d\n", flagreg);
		return -EIO;
	}

	if (flagreg & RX8010_FLAG_VLF) {
		DEBUGR("RTC low voltage detected\n");
		return -EINVAL;
	}

	for (int i = 0; i < 7; i++) {
		ret = rx8010sj_rtc_read8(dev, RX8010_SEC + i);
		if (ret < 0) {
			DEBUGR("Error reading from RTC. err: %d\n", ret);
			return -EIO;
		}
		date[i] = ret;
	}

	tmp->tm_sec = bcd2bin(date[RX8010_SEC - RX8010_SEC] & 0x7f);
	tmp->tm_min = bcd2bin(date[RX8010_MIN - RX8010_SEC] & 0x7f);
	tmp->tm_hour = bcd2bin(date[RX8010_HOUR - RX8010_SEC] & 0x3f);
	tmp->tm_mday = bcd2bin(date[RX8010_MDAY - RX8010_SEC] & 0x3f);
	tmp->tm_mon = bcd2bin(date[RX8010_MONTH - RX8010_SEC] & 0x1f);
	tmp->tm_year = bcd2bin(date[RX8010_YEAR - RX8010_SEC]) + 2000;
	tmp->tm_wday = 0;
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	DEBUGR("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

/* Set the RTC */
static int rx8010sj_rtc_set(DEV_TYPE *dev, const struct rtc_time *tm)
{
	u8 date[7];
	int ctrl, flagreg;
	int ret;

	ret = validate_time(tm);
	if (ret < 0)
		return -EINVAL;

	/* set STOP bit before changing clock/calendar */
	ctrl = rx8010sj_rtc_read8(dev, RX8010_CTRL);
	if (ctrl < 0)
		return ctrl;
	ret = rx8010sj_rtc_write8(dev, RX8010_CTRL, ctrl | RX8010_CTRL_STOP);
	if (ret < 0)
		return ret;

	date[RX8010_SEC - RX8010_SEC] = bin2bcd(tm->tm_sec);
	date[RX8010_MIN - RX8010_SEC] = bin2bcd(tm->tm_min);
	date[RX8010_HOUR - RX8010_SEC] = bin2bcd(tm->tm_hour);
	date[RX8010_MDAY - RX8010_SEC] = bin2bcd(tm->tm_mday);
	date[RX8010_MONTH - RX8010_SEC] = bin2bcd(tm->tm_mon);
	date[RX8010_YEAR - RX8010_SEC] = bin2bcd(tm->tm_year - 2000);
	date[RX8010_WDAY - RX8010_SEC] = bin2bcd(tm->tm_wday);

	for (int i = 0; i < 7; i++) {
		ret = rx8010sj_rtc_write8(dev, RX8010_SEC + i, date[i]);
		if (ret < 0) {
			DEBUGR("Error writing to RTC. err: %d\n", ret);
			return -EIO;
		}
	}

	/* clear STOP bit after changing clock/calendar */
	ctrl = rx8010sj_rtc_read8(dev, RX8010_CTRL);
	if (ctrl < 0)
		return ctrl;

	ret = rx8010sj_rtc_write8(dev, RX8010_CTRL, ctrl & ~RX8010_CTRL_STOP);
	if (ret < 0)
		return ret;

	flagreg = rx8010sj_rtc_read8(dev, RX8010_FLAG);
	if (flagreg < 0)
		return flagreg;

	if (flagreg & RX8010_FLAG_VLF)
		ret = rx8010sj_rtc_write8(dev, RX8010_FLAG,
					  flagreg & ~RX8010_FLAG_VLF);

	return 0;
}

/* Reset the RTC. */
static int rx8010sj_rtc_reset(DEV_TYPE *dev)
{
	/* Not needed */
	return 0;
}

#ifndef CONFIG_DM_RTC

int rtc_get(struct rtc_time *tm)
{
	struct ludevice dev = {
			.chip = CONFIG_SYS_I2C_RTC_ADDR,
	};

	return rx8010sj_rtc_get(&dev, tm);
}

int rtc_set(struct rtc_time *tm)
{
	struct ludevice dev = {
			.chip = CONFIG_SYS_I2C_RTC_ADDR,
	};

	return rx8010sj_rtc_set(&dev, tm);
}

void rtc_reset(void)
{
	struct ludevice dev = {
			.chip = CONFIG_SYS_I2C_RTC_ADDR,
	};

	rx8010sj_rtc_reset(&dev);
}

void rtc_init(void)
{
	struct ludevice dev = {
			.chip = CONFIG_SYS_I2C_RTC_ADDR,
	};

	rx8010sj_rtc_init(&dev);
}

#else

static int rx8010sj_probe(struct udevice *dev)
{
	rx8010sj_rtc_init(&dev);

	return 0;
}

static const struct rtc_ops rx8010sj_rtc_ops = {
	.get = rx8010sj_rtc_get,
	.set = rx8010sj_rtc_set,
	.read8 = rx8010sj_rtc_read8,
	.write8 = rx8010sj_rtc_write8,
	.reset = rx8010sj_rtc_reset,
};

static const struct udevice_id rx8010sj_rtc_ids[] = {
	{ .compatible = "epson,rx8010sj-rtc" },
	{ }
};

U_BOOT_DRIVER(rx8010sj_rtc) = {
	.name	  = "rx8010sj_rtc",
	.id	      = UCLASS_RTC,
	.probe    = rx8010sj_probe,
	.of_match = rx8010sj_rtc_ids,
	.ops	  = &rx8010sj_rtc_ops,
};

#endif
