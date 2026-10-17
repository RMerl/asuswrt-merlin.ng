// SPDX-License-Identifier: GPL-2.0+
/*
 * RTC driver for the Micro Crystal RV3028
 *
 * based on linux driver from
 * Copyright (C) 2019 Micro Crystal SA
 *
 * Alexandre Belloni <alexandre.belloni@bootlin.com>
 *
 */

#include <dm.h>
#include <i2c.h>
#include <rtc.h>

#define RV3028_SEC			0x00
#define RV3028_MIN			0x01
#define RV3028_HOUR			0x02
#define RV3028_WDAY			0x03
#define RV3028_DAY			0x04
#define RV3028_MONTH			0x05
#define RV3028_YEAR			0x06
#define RV3028_ALARM_MIN		0x07
#define RV3028_ALARM_HOUR		0x08
#define RV3028_ALARM_DAY		0x09
#define RV3028_STATUS			0x0E
#define RV3028_CTRL1			0x0F
#define RV3028_CTRL2			0x10
#define RV3028_EVT_CTRL			0x13
#define RV3028_TS_COUNT			0x14
#define RV3028_TS_SEC			0x15
#define RV3028_RAM1			0x1F
#define RV3028_EEPROM_ADDR		0x25
#define RV3028_EEPROM_DATA		0x26
#define RV3028_EEPROM_CMD		0x27
#define RV3028_CLKOUT			0x35
#define RV3028_OFFSET			0x36
#define RV3028_BACKUP			0x37

#define RV3028_STATUS_PORF		BIT(0)
#define RV3028_STATUS_EVF		BIT(1)
#define RV3028_STATUS_AF		BIT(2)
#define RV3028_STATUS_TF		BIT(3)
#define RV3028_STATUS_UF		BIT(4)
#define RV3028_STATUS_BSF		BIT(5)
#define RV3028_STATUS_CLKF		BIT(6)
#define RV3028_STATUS_EEBUSY		BIT(7)

#define RV3028_CLKOUT_FD_MASK		GENMASK(2, 0)
#define RV3028_CLKOUT_PORIE		BIT(3)
#define RV3028_CLKOUT_CLKSY		BIT(6)
#define RV3028_CLKOUT_CLKOE		BIT(7)

#define RV3028_CTRL1_EERD		BIT(3)
#define RV3028_CTRL1_WADA		BIT(5)

#define RV3028_CTRL2_RESET		BIT(0)
#define RV3028_CTRL2_12_24		BIT(1)
#define RV3028_CTRL2_EIE		BIT(2)
#define RV3028_CTRL2_AIE		BIT(3)
#define RV3028_CTRL2_TIE		BIT(4)
#define RV3028_CTRL2_UIE		BIT(5)
#define RV3028_CTRL2_TSE		BIT(7)

#define RV3028_EVT_CTRL_TSR		BIT(2)

#define RV3028_EEPROM_CMD_UPDATE	0x11
#define RV3028_EEPROM_CMD_WRITE		0x21
#define RV3028_EEPROM_CMD_READ		0x22

#define RV3028_EEBUSY_POLL		10000
#define RV3028_EEBUSY_TIMEOUT		100000

#define RV3028_BACKUP_TCE		BIT(5)
#define RV3028_BACKUP_TCR_MASK		GENMASK(1, 0)

#define OFFSET_STEP_PPT			953674

#define RTC_RV3028_LEN			7

static int rv3028_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	u8 regs[RTC_RV3028_LEN];
	u8 status;
	int ret;

	ret = dm_i2c_read(dev, RV3028_STATUS, &status, 1);
	if (ret < 0) {
		printf("%s: error reading RTC status: %x\n", __func__, ret);
		return -EIO;
	}

	if (status & RV3028_STATUS_PORF) {
		printf("Voltage low, data is invalid.\n");
		return -EINVAL;
	}

	ret = dm_i2c_read(dev, RV3028_SEC, regs, sizeof(regs));
	if (ret < 0) {
		printf("%s: error reading RTC: %x\n", __func__, ret);
		return -EIO;
	}

	tm->tm_sec = bcd2bin(regs[RV3028_SEC] & 0x7f);
	tm->tm_min = bcd2bin(regs[RV3028_MIN] & 0x7f);
	tm->tm_hour = bcd2bin(regs[RV3028_HOUR] & 0x3f);
	tm->tm_wday = regs[RV3028_WDAY] & 0x7;
	tm->tm_mday = bcd2bin(regs[RV3028_DAY] & 0x3f);
	tm->tm_mon  = bcd2bin(regs[RV3028_MONTH] & 0x1f);
	tm->tm_year = bcd2bin(regs[RV3028_YEAR]) + 2000;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("%s: %4d-%02d-%02d (wday=%d) %2d:%02d:%02d\n",
	      __func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
	      tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int rv3028_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	u8 regs[RTC_RV3028_LEN];
	u8 status;
	int ret;

	debug("%s: %4d-%02d-%02d (wday=%d( %2d:%02d:%02d\n",
	      __func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
	      tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 2000) {
		printf("%s: year %d (before 2000) not supported\n",
		       __func__, tm->tm_year);
		return -EINVAL;
	}

	regs[RV3028_SEC] = bin2bcd(tm->tm_sec);
	regs[RV3028_MIN] = bin2bcd(tm->tm_min);
	regs[RV3028_HOUR] = bin2bcd(tm->tm_hour);
	regs[RV3028_WDAY]  = tm->tm_wday;
	regs[RV3028_DAY]   = bin2bcd(tm->tm_mday);
	regs[RV3028_MONTH] = bin2bcd(tm->tm_mon);
	regs[RV3028_YEAR]  = bin2bcd(tm->tm_year - 2000);

	ret = dm_i2c_write(dev, RV3028_SEC, regs, sizeof(regs));
	if (ret) {
		printf("%s: set rtc error: %d\n", __func__, ret);
		return ret;
	}

	ret = dm_i2c_read(dev, RV3028_STATUS, &status, 1);
	if (ret < 0) {
		printf("%s: error reading RTC status: %x\n", __func__, ret);
		return -EIO;
	}
	status |= RV3028_STATUS_PORF;
	return dm_i2c_write(dev, RV3028_STATUS, &status, 1);
}

static int rv3028_rtc_reset(struct udevice *dev)
{
	return 0;
}

static int rv3028_rtc_read8(struct udevice *dev, unsigned int reg)
{
	u8 data;
	int ret;

	ret = dm_i2c_read(dev, reg, &data, sizeof(data));
	return ret < 0 ? ret : data;
}

static int rv3028_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	u8 data = val;

	return dm_i2c_write(dev, reg, &data, 1);
}

static int rv3028_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
				DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops rv3028_rtc_ops = {
	.get = rv3028_rtc_get,
	.set = rv3028_rtc_set,
	.read8 = rv3028_rtc_read8,
	.write8 = rv3028_rtc_write8,
	.reset = rv3028_rtc_reset,
};

static const struct udevice_id rv3028_rtc_ids[] = {
	{ .compatible = "microcrystal,rv3028" },
	{ }
};

U_BOOT_DRIVER(rtc_rv3028) = {
	.name	= "rtc-rv3028",
	.id	= UCLASS_RTC,
	.probe	= rv3028_probe,
	.of_match = rv3028_rtc_ids,
	.ops	= &rv3028_rtc_ops,
};
