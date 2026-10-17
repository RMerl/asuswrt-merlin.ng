// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * Markus Klotzbuecher, mk@denx.de
 *
 * (C) Copyright 2019 NXP
 * Chuanhua Han <chuanhua.han@nxp.com>
 */

/*
 * Date & Time support (no alarms) for Dallas Semiconductor (now Maxim)
 * Extremly Accurate DS3231 Real Time Clock (RTC).
 *
 * copied from ds1337.c
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <log.h>
#include <rtc.h>
#include <i2c.h>

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x0
#define RTC_MIN_REG_ADDR	0x1
#define RTC_HR_REG_ADDR		0x2
#define RTC_DAY_REG_ADDR	0x3
#define RTC_DATE_REG_ADDR	0x4
#define RTC_MON_REG_ADDR	0x5
#define RTC_YR_REG_ADDR		0x6
#define RTC_CTL_REG_ADDR	0x0e
#define RTC_STAT_REG_ADDR	0x0f


/*
 * RTC control register bits
 */
#define RTC_CTL_BIT_A1IE	0x1	/* Alarm 1 interrupt enable     */
#define RTC_CTL_BIT_A2IE	0x2	/* Alarm 2 interrupt enable     */
#define RTC_CTL_BIT_INTCN	0x4	/* Interrupt control            */
#define RTC_CTL_BIT_RS1		0x8	/* Rate select 1                */
#define RTC_CTL_BIT_RS2		0x10	/* Rate select 2                */
#define RTC_CTL_BIT_DOSC	0x80	/* Disable Oscillator           */

/*
 * RTC status register bits
 */
#define RTC_STAT_BIT_A1F	0x1	/* Alarm 1 flag                 */
#define RTC_STAT_BIT_A2F	0x2	/* Alarm 2 flag                 */
#define RTC_STAT_BIT_OSF	0x80	/* Oscillator stop flag         */
#define RTC_STAT_BIT_BB32KHZ	0x40	/* Battery backed 32KHz Output  */
#define RTC_STAT_BIT_EN32KHZ	0x8	/* Enable 32KHz Output  */


#if !CONFIG_IS_ENABLED(DM_RTC)
static uchar rtc_read (uchar reg);
static void rtc_write (uchar reg, uchar val);


/*
 * Get the current time from the RTC
 */
int rtc_get (struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon_cent, year, control, status;

	control = rtc_read (RTC_CTL_REG_ADDR);
	status = rtc_read (RTC_STAT_REG_ADDR);
	sec = rtc_read (RTC_SEC_REG_ADDR);
	min = rtc_read (RTC_MIN_REG_ADDR);
	hour = rtc_read (RTC_HR_REG_ADDR);
	wday = rtc_read (RTC_DAY_REG_ADDR);
	mday = rtc_read (RTC_DATE_REG_ADDR);
	mon_cent = rtc_read (RTC_MON_REG_ADDR);
	year = rtc_read (RTC_YR_REG_ADDR);

	debug("Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x control: %02x status: %02x\n",
		year, mon_cent, mday, wday, hour, min, sec, control, status);

	if (status & RTC_STAT_BIT_OSF) {
		printf ("### Warning: RTC oscillator has stopped\n");
		/* clear the OSF flag */
		rtc_write (RTC_STAT_REG_ADDR,
			   rtc_read (RTC_STAT_REG_ADDR) & ~RTC_STAT_BIT_OSF);
		rel = -1;
	}

	tmp->tm_sec  = bcd2bin (sec & 0x7F);
	tmp->tm_min  = bcd2bin (min & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon_cent & 0x1F);
	tmp->tm_year = bcd2bin (year) + ((mon_cent & 0x80) ? 2000 : 1900);
	tmp->tm_wday = bcd2bin ((wday - 1) & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}


/*
 * Set the RTC
 */
int rtc_set (struct rtc_time *tmp)
{
	uchar century;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write (RTC_YR_REG_ADDR, bin2bcd (tmp->tm_year % 100));

	century = (tmp->tm_year >= 2000) ? 0x80 : 0;
	rtc_write (RTC_MON_REG_ADDR, bin2bcd (tmp->tm_mon) | century);

	rtc_write (RTC_DAY_REG_ADDR, bin2bcd (tmp->tm_wday + 1));
	rtc_write (RTC_DATE_REG_ADDR, bin2bcd (tmp->tm_mday));
	rtc_write (RTC_HR_REG_ADDR, bin2bcd (tmp->tm_hour));
	rtc_write (RTC_MIN_REG_ADDR, bin2bcd (tmp->tm_min));
	rtc_write (RTC_SEC_REG_ADDR, bin2bcd (tmp->tm_sec));

	return 0;
}


/*
 * Reset the RTC.  We also enable the oscillator output on the
 * SQW/INTB* pin and program it for 32,768 Hz output. Note that
 * according to the datasheet, turning on the square wave output
 * increases the current drain on the backup battery from about
 * 600 nA to 2uA.
 */
void rtc_reset (void)
{
	rtc_write (RTC_CTL_REG_ADDR, RTC_CTL_BIT_RS1 | RTC_CTL_BIT_RS2);
}

/*
 * Enable 32KHz output
 */
#ifdef CONFIG_RTC_ENABLE_32KHZ_OUTPUT
void rtc_enable_32khz_output(void)
{
	rtc_write(RTC_STAT_REG_ADDR,
		  RTC_STAT_BIT_BB32KHZ | RTC_STAT_BIT_EN32KHZ);
}
#endif

/*
 * Helper functions
 */

static
uchar rtc_read (uchar reg)
{
	return (i2c_reg_read (CFG_SYS_I2C_RTC_ADDR, reg));
}


static void rtc_write (uchar reg, uchar val)
{
	i2c_reg_write (CFG_SYS_I2C_RTC_ADDR, reg, val);
}
#else
static int ds3231_rtc_get(struct udevice *dev, struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, wday, mon_cent, year, status;

	status = dm_i2c_reg_read(dev, RTC_STAT_REG_ADDR);
	sec = dm_i2c_reg_read(dev, RTC_SEC_REG_ADDR);
	min = dm_i2c_reg_read(dev, RTC_MIN_REG_ADDR);
	hour = dm_i2c_reg_read(dev, RTC_HR_REG_ADDR);
	wday = dm_i2c_reg_read(dev, RTC_DAY_REG_ADDR);
	mday = dm_i2c_reg_read(dev, RTC_DATE_REG_ADDR);
	mon_cent = dm_i2c_reg_read(dev, RTC_MON_REG_ADDR);
	year = dm_i2c_reg_read(dev, RTC_YR_REG_ADDR);

	if (status & RTC_STAT_BIT_OSF) {
		printf("### Warning: RTC oscillator has stopped\n");
		/* clear the OSF flag */
		dm_i2c_reg_write(dev, RTC_STAT_REG_ADDR,
				 dm_i2c_reg_read(dev, RTC_STAT_REG_ADDR)
						& ~RTC_STAT_BIT_OSF);
		return -EINVAL;
	}

	tmp->tm_sec  = bcd2bin(sec & 0x7F);
	tmp->tm_min  = bcd2bin(min & 0x7F);
	tmp->tm_hour = bcd2bin(hour & 0x3F);
	tmp->tm_mday = bcd2bin(mday & 0x3F);
	tmp->tm_mon  = bcd2bin(mon_cent & 0x1F);
	tmp->tm_year = bcd2bin(year) + ((mon_cent & 0x80) ? 2000 : 1900);
	tmp->tm_wday = bcd2bin((wday - 1) & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

static int ds3231_rtc_set(struct udevice *dev, const struct rtc_time *tmp)
{
	uchar century;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	dm_i2c_reg_write(dev, RTC_YR_REG_ADDR, bin2bcd(tmp->tm_year % 100));

	century = (tmp->tm_year >= 2000) ? 0x80 : 0;
	dm_i2c_reg_write(dev, RTC_MON_REG_ADDR, bin2bcd(tmp->tm_mon) | century);

	dm_i2c_reg_write(dev, RTC_DAY_REG_ADDR, bin2bcd(tmp->tm_wday + 1));
	dm_i2c_reg_write(dev, RTC_DATE_REG_ADDR, bin2bcd(tmp->tm_mday));
	dm_i2c_reg_write(dev, RTC_HR_REG_ADDR, bin2bcd(tmp->tm_hour));
	dm_i2c_reg_write(dev, RTC_MIN_REG_ADDR, bin2bcd(tmp->tm_min));
	dm_i2c_reg_write(dev, RTC_SEC_REG_ADDR, bin2bcd(tmp->tm_sec));

	return 0;
}

static int ds3231_rtc_reset(struct udevice *dev)
{
	int ret;

	ret = dm_i2c_reg_write(dev, RTC_CTL_REG_ADDR,
			       RTC_CTL_BIT_RS1 | RTC_CTL_BIT_RS2);
	if (ret < 0)
		return ret;

	return 0;
}

static int ds3231_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

#ifdef CONFIG_RTC_ENABLE_32KHZ_OUTPUT
int rtc_enable_32khz_output(int busnum, int chip_addr)
{
	int ret;
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(busnum, chip_addr, 1, &dev);
	if (!ret) {
		ret = dm_i2c_reg_write(dev, RTC_STAT_REG_ADDR,
				       RTC_STAT_BIT_BB32KHZ |
				       RTC_STAT_BIT_EN32KHZ);
	}
	return ret;
}
#endif

static const struct rtc_ops ds3231_rtc_ops = {
	.get = ds3231_rtc_get,
	.set = ds3231_rtc_set,
	.reset = ds3231_rtc_reset,
};

static const struct udevice_id ds3231_rtc_ids[] = {
	{ .compatible = "dallas,ds3231" },
	{ .compatible = "dallas,ds3232" },
	{ }
};

U_BOOT_DRIVER(rtc_ds3231) = {
	.name   = "rtc-ds3231",
	.id     = UCLASS_RTC,
	.probe  = ds3231_probe,
	.of_match = ds3231_rtc_ids,
	.ops    = &ds3231_rtc_ops,
};
#endif
