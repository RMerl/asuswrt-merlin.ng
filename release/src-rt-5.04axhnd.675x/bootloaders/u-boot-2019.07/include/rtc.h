/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Generic RTC interface.
 */
#ifndef _RTC_H_
#define _RTC_H_

/* bcd<->bin functions are needed by almost all the RTC drivers, let's include
 * it there instead of in evey single driver */

#include <bcd.h>
#include <rtc_def.h>

#ifdef CONFIG_DM_RTC

struct rtc_ops {
	/**
	 * get() - get the current time
	 *
	 * Returns the current time read from the RTC device. The driver
	 * is responsible for setting up every field in the structure.
	 *
	 * @dev:	Device to read from
	 * @time:	Place to put the time that is read
	 */
	int (*get)(struct udevice *dev, struct rtc_time *time);

	/**
	 * set() - set the current time
	 *
	 * Sets the time in the RTC device. The driver can expect every
	 * field to be set correctly.
	 *
	 * @dev:	Device to read from
	 * @time:	Time to write
	 */
	int (*set)(struct udevice *dev, const struct rtc_time *time);

	/**
	 * reset() - reset the RTC to a known-good state
	 *
	 * This function resets the RTC to a known-good state. The time may
	 * be unset by this method, so should be set after this method is
	 * called.
	 *
	 * @dev:	Device to read from
	 * @return 0 if OK, -ve on error
	 */
	int (*reset)(struct udevice *dev);

	/**
	 * read8() - Read an 8-bit register
	 *
	 * @dev:	Device to read from
	 * @reg:	Register to read
	 * @return value read, or -ve on error
	 */
	int (*read8)(struct udevice *dev, unsigned int reg);

	/**
	* write8() - Write an 8-bit register
	*
	* @dev:		Device to write to
	* @reg:		Register to write
	* @value:	Value to write
	* @return 0 if OK, -ve on error
	*/
	int (*write8)(struct udevice *dev, unsigned int reg, int val);
};

/* Access the operations for an RTC device */
#define rtc_get_ops(dev)	((struct rtc_ops *)(dev)->driver->ops)

/**
 * dm_rtc_get() - Read the time from an RTC
 *
 * @dev:	Device to read from
 * @time:	Place to put the current time
 * @return 0 if OK, -ve on error
 */
int dm_rtc_get(struct udevice *dev, struct rtc_time *time);

/**
 * dm_rtc_set() - Write a time to an RTC
 *
 * @dev:	Device to read from
 * @time:	Time to write into the RTC
 * @return 0 if OK, -ve on error
 */
int dm_rtc_set(struct udevice *dev, struct rtc_time *time);

/**
 * dm_rtc_reset() - reset the RTC to a known-good state
 *
 * If the RTC appears to be broken (e.g. it is not counting up in seconds)
 * it may need to be reset to a known good state. This function achieves this.
 * After resetting the RTC the time should then be set to a known value by
 * the caller.
 *
 * @dev:	Device to read from
 * @return 0 if OK, -ve on error
 */
int dm_rtc_reset(struct udevice *dev);

/**
 * rtc_read8() - Read an 8-bit register
 *
 * @dev:	Device to read from
 * @reg:	Register to read
 * @return value read, or -ve on error
 */
int rtc_read8(struct udevice *dev, unsigned int reg);

/**
 * rtc_write8() - Write an 8-bit register
 *
 * @dev:	Device to write to
 * @reg:	Register to write
 * @value:	Value to write
 * @return 0 if OK, -ve on error
 */
int rtc_write8(struct udevice *dev, unsigned int reg, int val);

/**
 * rtc_read16() - Read a 16-bit value from the RTC
 *
 * @dev:	Device to read from
 * @reg:	Offset to start reading from
 * @valuep:	Place to put the value that is read
 * @return 0 if OK, -ve on error
 */
int rtc_read16(struct udevice *dev, unsigned int reg, u16 *valuep);

/**
 * rtc_write16() - Write a 16-bit value to the RTC
 *
 * @dev:	Device to write to
 * @reg:	Register to start writing to
 * @value:	Value to write
 * @return 0 if OK, -ve on error
 */
int rtc_write16(struct udevice *dev, unsigned int reg, u16 value);

/**
 * rtc_read32() - Read a 32-bit value from the RTC
 *
 * @dev:	Device to read from
 * @reg:	Offset to start reading from
 * @valuep:	Place to put the value that is read
 * @return 0 if OK, -ve on error
 */
int rtc_read32(struct udevice *dev, unsigned int reg, u32 *valuep);

/**
 * rtc_write32() - Write a 32-bit value to the RTC
 *
 * @dev:	Device to write to
 * @reg:	Register to start writing to
 * @value:	Value to write
 * @return 0 if OK, -ve on error
 */
int rtc_write32(struct udevice *dev, unsigned int reg, u32 value);

#else
int rtc_get (struct rtc_time *);
int rtc_set (struct rtc_time *);
void rtc_reset (void);
void rtc_enable_32khz_output(void);

/**
 * rtc_read8() - Read an 8-bit register
 *
 * @reg:	Register to read
 * @return value read
 */
int rtc_read8(int reg);

/**
 * rtc_write8() - Write an 8-bit register
 *
 * @reg:	Register to write
 * @value:	Value to write
 */
void rtc_write8(int reg, uchar val);

/**
 * rtc_read32() - Read a 32-bit value from the RTC
 *
 * @reg:	Offset to start reading from
 * @return value read
 */
u32 rtc_read32(int reg);

/**
 * rtc_write32() - Write a 32-bit value to the RTC
 *
 * @reg:	Register to start writing to
 * @value:	Value to write
 */
void rtc_write32(int reg, u32 value);

/**
 * rtc_init() - Set up the real time clock ready for use
 */
void rtc_init(void);
#endif /* CONFIG_DM_RTC */

/**
 * is_leap_year - Check if year is a leap year
 *
 * @year	Year
 * @return	1 if leap year
 */
static inline bool is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

/**
 * rtc_calc_weekday() - Work out the weekday from a time
 *
 * This only works for the Gregorian calendar - i.e. after 1752 (in the UK).
 * It sets time->tm_wdaay to the correct day of the week.
 *
 * @time:	Time to inspect. tm_wday is updated
 * @return 0 if OK, -EINVAL if the weekday could not be determined
 */
int rtc_calc_weekday(struct rtc_time *time);

/**
 * rtc_to_tm() - Convert a time_t value into a broken-out time
 *
 * The following fields are set up by this function:
 *	tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday
 *
 * Note that tm_yday and tm_isdst are set to 0.
 *
 * @time_t:	Number of seconds since 1970-01-01 00:00:00
 * @time:	Place to put the broken-out time
 */
void rtc_to_tm(u64 time_t, struct rtc_time *time);

/**
 * rtc_mktime() - Convert a broken-out time into a time_t value
 *
 * The following fields need to be valid for this function to work:
 *	tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year
 *
 * Note that tm_wday and tm_yday are ignored.
 *
 * @time:	Broken-out time to convert
 * @return corresponding time_t value, seconds since 1970-01-01 00:00:00
 */
unsigned long rtc_mktime(const struct rtc_time *time);

/**
 * rtc_month_days() - The number of days in the month
 *
 * @month:	month (January = 0)
 * @year:	year (4 digits)
 */
int rtc_month_days(unsigned int month, unsigned int year);

#endif	/* _RTC_H_ */
