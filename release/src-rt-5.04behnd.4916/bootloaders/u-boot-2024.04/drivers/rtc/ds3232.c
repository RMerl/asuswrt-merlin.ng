// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019, Vaisala Oyj
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x00
#define RTC_MIN_REG_ADDR	0x01
#define RTC_HR_REG_ADDR	0x02
#define RTC_DAY_REG_ADDR	0x03
#define RTC_DATE_REG_ADDR	0x04
#define RTC_MON_REG_ADDR	0x05
#define RTC_YR_REG_ADDR	0x06
#define RTC_CTL_REG_ADDR	0x0e
#define RTC_STAT_REG_ADDR	0x0f
#define RTC_TEST_REG_ADDR	0x13

/*
 * RTC control register bits
 */
#define RTC_CTL_BIT_A1IE	BIT(0)	/* Alarm 1 interrupt enable     */
#define RTC_CTL_BIT_A2IE	BIT(1)	/* Alarm 2 interrupt enable     */
#define RTC_CTL_BIT_INTCN	BIT(2)	/* Interrupt control            */
#define RTC_CTL_BIT_DOSC	BIT(7)	/* Disable Oscillator           */

/*
 * RTC status register bits
 */
#define RTC_STAT_BIT_A1F	BIT(0)	/* Alarm 1 flag                 */
#define RTC_STAT_BIT_A2F	BIT(1)	/* Alarm 2 flag                 */
#define RTC_STAT_BIT_EN32KHZ	BIT(3)	/* Enable 32KHz Output  */
#define RTC_STAT_BIT_BB32KHZ	BIT(6)	/* Battery backed 32KHz Output  */
#define RTC_STAT_BIT_OSF	BIT(7)	/* Oscillator stop flag         */

/*
 * RTC test register bits
 */
#define RTC_TEST_BIT_SWRST	BIT(7)	/* Software reset */

#define RTC_DATE_TIME_REG_SIZE 7
#define RTC_SRAM_START 0x14
#define RTC_SRAM_END 0xFF
#define RTC_SRAM_SIZE 236

struct ds3232_priv_data {
	u8 max_register;
	u8 sram_start;
	int sram_size;
};

static int ds3232_rtc_read8(struct udevice *dev, unsigned int reg)
{
	int ret;
	u8 buf;
	struct ds3232_priv_data *priv_data;

	priv_data = dev_get_priv(dev);
	if (!priv_data)
		return -EINVAL;

	if (reg > priv_data->max_register)
		return -EINVAL;

	ret = dm_i2c_read(dev, reg, &buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return buf;
}

static int ds3232_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	u8 buf = (u8)val;
	struct ds3232_priv_data *priv_data;

	priv_data = dev_get_priv(dev);
	if (!priv_data)
		return -EINVAL;

	if (reg > priv_data->max_register)
		return -EINVAL;

	return dm_i2c_write(dev, reg, &buf, sizeof(buf));
}

static int reset_sram(struct udevice *dev)
{
	int ret, sram_end, reg;
	struct ds3232_priv_data *priv_data;

	priv_data = dev_get_priv(dev);
	if (!priv_data)
		return -EINVAL;

	sram_end = priv_data->sram_start + priv_data->sram_size;

	for (reg = priv_data->sram_start; reg < sram_end; reg++) {
		ret = ds3232_rtc_write8(dev, reg, 0x00);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int verify_osc(struct udevice *dev)
{
	int ret, rtc_status;

	ret = ds3232_rtc_read8(dev, RTC_STAT_REG_ADDR);
	if (ret < 0)
		return ret;

	rtc_status = ret;

	if (rtc_status & RTC_STAT_BIT_OSF) {
		dev_warn(dev,
			 "oscillator discontinuity flagged, time unreliable\n");
		/*
		 * In case OSC was off we cannot trust the SRAM data anymore.
		 * Reset it to 0x00.
		 */
		ret = reset_sram(dev);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int ds3232_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	u8 buf[RTC_DATE_TIME_REG_SIZE];
	u8 is_century;

	if (tm->tm_year < 1900 || tm->tm_year > 2099)
		dev_warn(dev, "WARNING: year should be between 1900 and 2099!\n");

	is_century = (tm->tm_year >= 2000) ? 0x80 : 0;

	buf[RTC_SEC_REG_ADDR] = bin2bcd(tm->tm_sec);
	buf[RTC_MIN_REG_ADDR] = bin2bcd(tm->tm_min);
	buf[RTC_HR_REG_ADDR] = bin2bcd(tm->tm_hour);
	buf[RTC_DAY_REG_ADDR] = bin2bcd(tm->tm_wday + 1);
	buf[RTC_DATE_REG_ADDR] = bin2bcd(tm->tm_mday);
	buf[RTC_MON_REG_ADDR] = bin2bcd(tm->tm_mon) | is_century;
	buf[RTC_YR_REG_ADDR] = bin2bcd(tm->tm_year % 100);

	return dm_i2c_write(dev, 0, buf, sizeof(buf));
}

static int ds3232_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	int ret;
	u8 buf[RTC_DATE_TIME_REG_SIZE];
	u8 is_twelve_hr;
	u8 is_pm;
	u8 is_century;

	ret = verify_osc(dev);
	if (ret < 0)
		return ret;

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	/* Extract additional information for AM/PM and century */
	is_twelve_hr = buf[RTC_HR_REG_ADDR] & 0x40;
	is_pm = buf[RTC_HR_REG_ADDR] & 0x20;
	is_century = buf[RTC_MON_REG_ADDR] & 0x80;

	tm->tm_sec  = bcd2bin(buf[RTC_SEC_REG_ADDR] & 0x7F);
	tm->tm_min  = bcd2bin(buf[RTC_MIN_REG_ADDR] & 0x7F);

	if (is_twelve_hr)
		tm->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR] & 0x1F)
			+ (is_pm ? 12 : 0);
	else
		tm->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR]);

	tm->tm_wday = bcd2bin((buf[RTC_DAY_REG_ADDR] & 0x07) - 1);
	tm->tm_mday = bcd2bin(buf[RTC_DATE_REG_ADDR] & 0x3F);
	tm->tm_mon  = bcd2bin((buf[RTC_MON_REG_ADDR] & 0x7F));
	tm->tm_year = bcd2bin(buf[RTC_YR_REG_ADDR])
		+ (is_century ? 2000 : 1900);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	return 0;
}

static int ds3232_rtc_reset(struct udevice *dev)
{
	int ret;

	ret = reset_sram(dev);
	if (ret < 0)
		return ret;

	/*
	 * From datasheet
	 * (https://datasheets.maximintegrated.com/en/ds/DS3232M.pdf):
	 *
	 * The device reset occurs during the normal acknowledge time slot
	 * following the receipt of the data byte carrying that
	 * SWRST instruction a NACK occurs due to the resetting action.
	 *
	 * Therefore we don't verify the result of I2C write operation since it
	 * will fail due the NACK.
	 */
	ds3232_rtc_write8(dev, RTC_TEST_REG_ADDR, RTC_TEST_BIT_SWRST);

	return 0;
}

static int ds3232_probe(struct udevice *dev)
{
	int rtc_status;
	int ret;
	struct ds3232_priv_data *priv_data;

	priv_data = dev_get_priv(dev);
	if (!priv_data)
		return -EINVAL;

	priv_data->sram_start = RTC_SRAM_START;
	priv_data->max_register = RTC_SRAM_END;
	priv_data->sram_size = RTC_SRAM_SIZE;

	ret = ds3232_rtc_read8(dev, RTC_STAT_REG_ADDR);
	if (ret < 0)
		return ret;

	rtc_status = ret;

	ret = verify_osc(dev);
	if (ret < 0)
		return ret;

	rtc_status &= ~(RTC_STAT_BIT_OSF | RTC_STAT_BIT_A1F | RTC_STAT_BIT_A2F);

	return ds3232_rtc_write8(dev, RTC_STAT_REG_ADDR, rtc_status);
}

static const struct rtc_ops ds3232_rtc_ops = {
	.get = ds3232_rtc_get,
	.set = ds3232_rtc_set,
	.reset = ds3232_rtc_reset,
	.read8 = ds3232_rtc_read8,
	.write8 = ds3232_rtc_write8
};

static const struct udevice_id ds3232_rtc_ids[] = {
	{ .compatible = "dallas,ds3232" },
	{ }
};

U_BOOT_DRIVER(rtc_ds3232) = {
	.name = "rtc-ds3232",
	.id = UCLASS_RTC,
	.probe = ds3232_probe,
	.of_match = ds3232_rtc_ids,
	.ops = &ds3232_rtc_ops,
	.priv_auto	= sizeof(struct ds3232_priv_data),
};
