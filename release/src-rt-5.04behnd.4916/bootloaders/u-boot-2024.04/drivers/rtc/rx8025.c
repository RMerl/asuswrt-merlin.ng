// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
 */

/*
 * Epson RX8025 RTC driver.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>

/*---------------------------------------------------------------------*/
#undef DEBUG_RTC

#ifdef DEBUG_RTC
#define DEBUGR(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGR(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

enum rx_model {
	model_rx_8025,
	model_rx_8035,
};

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x00
#define RTC_MIN_REG_ADDR	0x01
#define RTC_HR_REG_ADDR		0x02
#define RTC_DAY_REG_ADDR	0x03
#define RTC_DATE_REG_ADDR	0x04
#define RTC_MON_REG_ADDR	0x05
#define RTC_YR_REG_ADDR		0x06
#define RTC_OFFSET_REG_ADDR	0x07

#define RTC_CTL1_REG_ADDR	0x0e
#define RTC_CTL2_REG_ADDR	0x0f

/*
 * Control register 1 bits
 */
#define RTC_CTL1_BIT_2412	0x20

/*
 * Control register 2 bits
 */
#define RTC_CTL2_BIT_PON	0x10
#define RTC_CTL2_BIT_VDET	0x40
#define RTC_CTL2_BIT_XST	0x20
#define RTC_CTL2_BIT_VDSL	0x80

/*
 * Note: the RX8025 I2C RTC requires register
 * reads and write to consist of a single bus
 * cycle. It is not allowed to write the register
 * address in a first cycle that is terminated by
 * a STOP condition. The chips needs a 'restart'
 * sequence (start sequence without a prior stop).
 */

#define rtc_read(reg) buf[(reg) & 0xf]

static int rtc_write(struct udevice *dev, uchar reg, uchar val);

static int rx8025_is_osc_stopped(enum rx_model model, int ctrl2)
{
	int xstp = ctrl2 & RTC_CTL2_BIT_XST;
	/* XSTP bit has different polarity on RX-8025 vs RX-8035.
	 * RX-8025: 0 == oscillator stopped
	 * RX-8035: 1 == oscillator stopped
	 */

	if (model == model_rx_8025)
		xstp = !xstp;

	return xstp;
}

/*
 * Get the current time from the RTC
 */
static int rx8025_rtc_get(struct udevice *dev, struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon, year, ctl2;
	uchar buf[16];

	if (dm_i2c_read(dev, 0, buf, sizeof(buf))) {
		printf("Error reading from RTC\n");
		return -EIO;
	}

	sec = rtc_read(RTC_SEC_REG_ADDR);
	min = rtc_read(RTC_MIN_REG_ADDR);
	hour = rtc_read(RTC_HR_REG_ADDR);
	wday = rtc_read(RTC_DAY_REG_ADDR);
	mday = rtc_read(RTC_DATE_REG_ADDR);
	mon = rtc_read(RTC_MON_REG_ADDR);
	year = rtc_read(RTC_YR_REG_ADDR);

	DEBUGR("Get RTC year: %02x mon: %02x mday: %02x wday: %02x "
	       "hr: %02x min: %02x sec: %02x\n",
	       year, mon, mday, wday, hour, min, sec);

	/* dump status */
	ctl2 = rtc_read(RTC_CTL2_REG_ADDR);
	if (ctl2 & RTC_CTL2_BIT_PON) {
		printf("RTC: power-on detected\n");
		rel = -1;
	}

	if (ctl2 & RTC_CTL2_BIT_VDET) {
		printf("RTC: voltage drop detected\n");
		rel = -1;
	}
	if (rx8025_is_osc_stopped(dev->driver_data, ctl2)) {
		printf("RTC: oscillator stop detected\n");
		rel = -1;
	}

	tmp->tm_sec  = bcd2bin(sec & 0x7F);
	tmp->tm_min  = bcd2bin(min & 0x7F);
	if (rtc_read(RTC_CTL1_REG_ADDR) & RTC_CTL1_BIT_2412)
		tmp->tm_hour = bcd2bin(hour & 0x3F);
	else
		tmp->tm_hour = bcd2bin(hour & 0x1F) % 12 +
			       ((hour & 0x20) ? 12 : 0);

	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year) + ( bcd2bin (year) >= 70 ? 1900 : 2000);
	tmp->tm_wday = bcd2bin (wday & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	DEBUGR("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}

/*
 * Set the RTC
 */
static int rx8025_rtc_set(struct udevice *dev, const struct rtc_time *tmp)
{
	/* To work around the read/write cycle issue mentioned
	 * at the top of this file, write all the time registers
	 * in one I2C transaction
	 */
	u8 write_op[8];

	/* 2412 flag must be set before doing a RTC write,
	 * otherwise the seconds and minute register
	 * will be cleared when the flag is set
	 */
	if (rtc_write(dev, RTC_CTL1_REG_ADDR, RTC_CTL1_BIT_2412))
		return -EIO;

	DEBUGR("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	if (tmp->tm_year < 1970 || tmp->tm_year > 2069)
		printf("WARNING: year should be between 1970 and 2069!\n");

	write_op[RTC_SEC_REG_ADDR]  = bin2bcd(tmp->tm_sec);
	write_op[RTC_MIN_REG_ADDR]  = bin2bcd(tmp->tm_min);
	write_op[RTC_HR_REG_ADDR]   = bin2bcd(tmp->tm_hour);
	write_op[RTC_DAY_REG_ADDR]	= bin2bcd(tmp->tm_wday);
	write_op[RTC_DATE_REG_ADDR]	= bin2bcd(tmp->tm_mday);
	write_op[RTC_MON_REG_ADDR]  = bin2bcd(tmp->tm_mon);
	write_op[RTC_YR_REG_ADDR]	= bin2bcd(tmp->tm_year % 100);
	write_op[RTC_OFFSET_REG_ADDR] = 0;

	return dm_i2c_write(dev, 0, &write_op[0], 8);
}

/*
 * Reset the RTC
 */
static int rx8025_rtc_reset(struct udevice *dev)
{
	uchar buf[16];
	uchar ctl2;

	if (dm_i2c_read(dev, 0, buf, sizeof(buf))) {
		printf("Error reading from RTC\n");
		return -EIO;
	}

	ctl2 = rtc_read(RTC_CTL2_REG_ADDR);
	ctl2 &= ~(RTC_CTL2_BIT_PON | RTC_CTL2_BIT_VDET);

	if (dev->driver_data == model_rx_8035)
		ctl2 &= ~(RTC_CTL2_BIT_XST);
	else
		ctl2 |= RTC_CTL2_BIT_XST;

	return rtc_write(dev, RTC_CTL2_REG_ADDR, ctl2);
}

/*
 * Helper functions
 */
static int rtc_write(struct udevice *dev, uchar reg, uchar val)
{
	/* The RX8025/RX8035 uses the top 4 bits of the
	 * 'offset' byte as the start register address,
	 * and the bottom 4 bits as a 'transfer' mode setting
	 * (only applicable for reads)
	 */
	u8 offset = (reg << 4);

	if (dm_i2c_reg_write(dev, offset, val)) {
		printf("Error writing to RTC\n");
		return -EIO;
	}

	return 0;
}

static int rx8025_probe(struct udevice *dev)
{
	uchar buf[16];
	int ret = 0;

	if (i2c_get_chip_offset_len(dev) != 1)
		ret = i2c_set_chip_offset_len(dev, 1);

	if (ret)
		return ret;

	return dm_i2c_read(dev, 0, buf, sizeof(buf));
}

static const struct rtc_ops rx8025_rtc_ops = {
	.get = rx8025_rtc_get,
	.set = rx8025_rtc_set,
	.reset = rx8025_rtc_reset,
};

static const struct udevice_id rx8025_rtc_ids[] = {
	{ .compatible = "epson,rx8025", .data = model_rx_8025 },
	{ .compatible = "epson,rx8035", .data = model_rx_8035 },
	{ }
};

U_BOOT_DRIVER(rx8025_rtc) = {
	.name	  = "rx8025_rtc",
	.id	      = UCLASS_RTC,
	.probe    = rx8025_probe,
	.of_match = rx8025_rtc_ids,
	.ops	  = &rx8025_rtc_ops,
};
