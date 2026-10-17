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
#include <linux/errno.h>
#include <linux/types.h>

typedef int64_t time64_t;
struct udevice;

#if CONFIG_IS_ENABLED(DM_RTC)
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
	 * read() - Read multiple 8-bit registers
	 *
	 * @dev:	Device to read from
	 * @reg:	First register to read
	 * @buf:	Output buffer
	 * @len:	Number of registers to read
	 * @return 0 if OK, -ve on error
	 */
	int (*read)(struct udevice *dev, unsigned int reg,
		    u8 *buf, unsigned int len);

	/**
	 * write() - Write multiple 8-bit registers
	 *
	 * @dev:	Device to write to
	 * @reg:	First register to write
	 * @buf:	Input buffer
	 * @len:	Number of registers to write
	 * @return 0 if OK, -ve on error
	 */
	int (*write)(struct udevice *dev, unsigned int reg,
		     const u8 *buf, unsigned int len);

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
	* Return: 0 if OK, -ve on error
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
 * Return: 0 if OK, -ve on error
 */
int dm_rtc_get(struct udevice *dev, struct rtc_time *time);

/**
 * dm_rtc_set() - Write a time to an RTC
 *
 * @dev:	Device to read from
 * @time:	Time to write into the RTC
 * Return: 0 if OK, -ve on error
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
 * Return: 0 if OK, -ve on error
 */
int dm_rtc_reset(struct udevice *dev);

/**
 * dm_rtc_read() - Read multiple 8-bit registers
 *
 * @dev:	Device to read from
 * @reg:	First register to read
 * @buf:	Output buffer
 * @len:	Number of registers to read
 * Return: 0 if OK, -ve on error
 */
int dm_rtc_read(struct udevice *dev, unsigned int reg, u8 *buf, unsigned int len);

/**
 * dm_rtc_write() - Write multiple 8-bit registers
 *
 * @dev:	Device to write to
 * @reg:	First register to write
 * @buf:	Input buffer
 * @len:	Number of registers to write
 * Return: 0 if OK, -ve on error
 */
int dm_rtc_write(struct udevice *dev, unsigned int reg,
		 const u8 *buf, unsigned int len);

/**
 * rtc_read8() - Read an 8-bit register
 *
 * @dev:	Device to read from
 * @reg:	Register to read
 * Return: value read, or -ve on error
 */
int rtc_read8(struct udevice *dev, unsigned int reg);

/**
 * rtc_write8() - Write an 8-bit register
 *
 * @dev:	Device to write to
 * @reg:	Register to write
 * @value:	Value to write
 * Return: 0 if OK, -ve on error
 */
int rtc_write8(struct udevice *dev, unsigned int reg, int val);

/**
 * rtc_read16() - Read a 16-bit value from the RTC
 *
 * @dev:	Device to read from
 * @reg:	Offset to start reading from
 * @valuep:	Place to put the value that is read
 * Return: 0 if OK, -ve on error
 */
int rtc_read16(struct udevice *dev, unsigned int reg, u16 *valuep);

/**
 * rtc_write16() - Write a 16-bit value to the RTC
 *
 * @dev:	Device to write to
 * @reg:	Register to start writing to
 * @value:	Value to write
 * Return: 0 if OK, -ve on error
 */
int rtc_write16(struct udevice *dev, unsigned int reg, u16 value);

/**
 * rtc_read32() - Read a 32-bit value from the RTC
 *
 * @dev:	Device to read from
 * @reg:	Offset to start reading from
 * @valuep:	Place to put the value that is read
 * Return: 0 if OK, -ve on error
 */
int rtc_read32(struct udevice *dev, unsigned int reg, u32 *valuep);

/**
 * rtc_write32() - Write a 32-bit value to the RTC
 *
 * @dev:	Device to write to
 * @reg:	Register to start writing to
 * @value:	Value to write
 * Return: 0 if OK, -ve on error
 */
int rtc_write32(struct udevice *dev, unsigned int reg, u32 value);

#ifdef CONFIG_RTC_ENABLE_32KHZ_OUTPUT
int rtc_enable_32khz_output(int busnum, int chip_addr);
#endif

#else
static inline int dm_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	return -ENOSYS;
}

static inline int dm_rtc_set(struct udevice *dev, struct rtc_time *time)
{
	return -ENOSYS;
}

static inline int dm_rtc_reset(struct udevice *dev)
{
	return -ENOSYS;
}

static inline int dm_rtc_read(struct udevice *dev, unsigned int reg, u8 *buf,
			      unsigned int len)
{
	return -ENOSYS;
}

static inline int dm_rtc_write(struct udevice *dev, unsigned int reg,
			       const u8 *buf, unsigned int len)
{
	return -ENOSYS;
}

int rtc_get (struct rtc_time *);
int rtc_set (struct rtc_time *);
void rtc_reset (void);
#ifdef CONFIG_RTC_ENABLE_32KHZ_OUTPUT
void rtc_enable_32khz_output(void);
#endif

/**
 * rtc_read8() - Read an 8-bit register
 *
 * @reg:	Register to read
 * Return: value read
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
 * Return: value read
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
 * Return:	1 if leap year
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
 * Return: 0 if OK, -EINVAL if the weekday could not be determined
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
 * rtc_mktime() - Convert a broken-out time into a time64_t value
 *
 * The following fields need to be valid for this function to work:
 *	tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year
 *
 * Note that tm_wday and tm_yday are ignored.
 *
 * @time:	Broken-out time to convert
 * Return: corresponding time64_t value, seconds since 1970-01-01 00:00:00
 */
time64_t rtc_mktime(const struct rtc_time *time);

/**
 * rtc_month_days() - The number of days in the month
 *
 * @month:	month (January = 0)
 * @year:	year (4 digits)
 */
int rtc_month_days(unsigned int month, unsigned int year);

#endif	/* _RTC_H_ */
