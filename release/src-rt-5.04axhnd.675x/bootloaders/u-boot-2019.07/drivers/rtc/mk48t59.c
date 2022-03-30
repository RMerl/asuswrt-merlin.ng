// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 */

/*
 * Date & Time support for the MK48T59 RTC
 */

#undef	RTC_DEBUG

#include <common.h>
#include <command.h>
#include <config.h>
#include <rtc.h>
#include <mk48t59.h>

#if defined(CONFIG_BAB7xx)

static uchar rtc_read (short reg)
{
	out8(RTC_PORT_ADDR0, reg & 0xFF);
	out8(RTC_PORT_ADDR1, (reg>>8) & 0xFF);
	return in8(RTC_PORT_DATA);
}

static void rtc_write (short reg, uchar val)
{
	out8(RTC_PORT_ADDR0, reg & 0xFF);
	out8(RTC_PORT_ADDR1, (reg>>8) & 0xFF);
	out8(RTC_PORT_DATA, val);
}

#elif defined(CONFIG_EVAL5200)

static uchar rtc_read (short reg)
{
	return in8(RTC(reg));
}

static void rtc_write (short reg, uchar val)
{
	out8(RTC(reg),val);
}

#else
# error Board specific rtc access functions should be supplied
#endif

/* ------------------------------------------------------------------------- */

void *nvram_read(void *dest, const short src, size_t count)
{
	uchar *d = (uchar *) dest;
	short s = src;

	while (count--)
		*d++ = rtc_read(s++);

	return dest;
}

void nvram_write(short dest, const void *src, size_t count)
{
	short d = dest;
	uchar *s = (uchar *) src;

	while (count--)
		rtc_write(d++, *s++);
}

/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	uchar save_ctrl_a;
	uchar sec, min, hour, mday, wday, mon, year;

	/* Simple: freeze the clock, read it and allow updates again */
	save_ctrl_a = rtc_read(RTC_CONTROLA);

	/* Set the register to read the value. */
	save_ctrl_a |= RTC_CA_READ;
	rtc_write(RTC_CONTROLA, save_ctrl_a);

	sec		= rtc_read (RTC_SECONDS);
	min		= rtc_read (RTC_MINUTES);
	hour	= rtc_read (RTC_HOURS);
	mday	= rtc_read (RTC_DAY_OF_MONTH);
	wday	= rtc_read (RTC_DAY_OF_WEEK);
	mon		= rtc_read (RTC_MONTH);
	year	= rtc_read (RTC_YEAR);

	/* re-enable update */
	save_ctrl_a &= ~RTC_CA_READ;
	rtc_write(RTC_CONTROLA, save_ctrl_a);

#ifdef RTC_DEBUG
	printf ( "Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon, mday, wday,
		hour, min, sec );
#endif
	tmp->tm_sec  = bcd2bin (sec  & 0x7F);
	tmp->tm_min  = bcd2bin (min  & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year);
	tmp->tm_wday = bcd2bin (wday & 0x07);
	if(tmp->tm_year<70)
		tmp->tm_year+=2000;
	else
		tmp->tm_year+=1900;
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;
#ifdef RTC_DEBUG
	printf ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
	uchar save_ctrl_a;

#ifdef RTC_DEBUG
	printf ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif
	save_ctrl_a = rtc_read(RTC_CONTROLA);

	save_ctrl_a |= RTC_CA_WRITE;
	rtc_write(RTC_CONTROLA, save_ctrl_a); /* disables the RTC to update the regs */

	rtc_write (RTC_YEAR, bin2bcd(tmp->tm_year % 100));
	rtc_write (RTC_MONTH, bin2bcd(tmp->tm_mon));

	rtc_write (RTC_DAY_OF_WEEK, bin2bcd(tmp->tm_wday));
	rtc_write (RTC_DAY_OF_MONTH, bin2bcd(tmp->tm_mday));
	rtc_write (RTC_HOURS, bin2bcd(tmp->tm_hour));
	rtc_write (RTC_MINUTES, bin2bcd(tmp->tm_min ));
	rtc_write (RTC_SECONDS, bin2bcd(tmp->tm_sec ));

	save_ctrl_a &= ~RTC_CA_WRITE;
	rtc_write(RTC_CONTROLA, save_ctrl_a); /* enables the RTC to update the regs */

	return 0;
}

void rtc_reset (void)
{
	uchar control_b;

	/*
	 * Start oscillator here.
	 */
	control_b = rtc_read(RTC_CONTROLB);

	control_b &= ~RTC_CB_STOP;
	rtc_write(RTC_CONTROLB, control_b);
}

void rtc_set_watchdog(short multi, short res)
{
	uchar wd_value;

	wd_value = RTC_WDS | ((multi & 0x1F) << 2) | (res & 0x3);
	rtc_write(RTC_WATCHDOG, wd_value);
}
