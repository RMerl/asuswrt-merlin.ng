// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * ARIO Data Networks, Inc. dchiu@ariodata.com
 *
 * Based on MontaVista DS1743 code and U-Boot mc146818 code
 */

/*
 * Date & Time support for the DS174x RTC
 */

/*#define	DEBUG*/

#include <common.h>
#include <command.h>
#include <rtc.h>

static uchar rtc_read( unsigned int addr );
static void  rtc_write( unsigned int addr, uchar val);

#define RTC_BASE		( CONFIG_SYS_NVRAM_SIZE + CONFIG_SYS_NVRAM_BASE_ADDR )

#define RTC_YEAR		( RTC_BASE + 7 )
#define RTC_MONTH		( RTC_BASE + 6 )
#define RTC_DAY_OF_MONTH	( RTC_BASE + 5 )
#define RTC_DAY_OF_WEEK		( RTC_BASE + 4 )
#define RTC_HOURS		( RTC_BASE + 3 )
#define RTC_MINUTES		( RTC_BASE + 2 )
#define RTC_SECONDS		( RTC_BASE + 1 )
#define RTC_CENTURY		( RTC_BASE + 0 )

#define RTC_CONTROLA		RTC_CENTURY
#define RTC_CONTROLB		RTC_SECONDS
#define RTC_CONTROLC		RTC_DAY_OF_WEEK

#define RTC_CA_WRITE		0x80
#define RTC_CA_READ		0x40

#define RTC_CB_OSC_DISABLE	0x80

#define RTC_CC_BATTERY_FLAG	0x80
#define RTC_CC_FREQ_TEST	0x40

/* ------------------------------------------------------------------------- */

int rtc_get( struct rtc_time *tmp )
{
	uchar sec, min, hour;
	uchar mday, wday, mon, year;

	int century;

	uchar reg_a;

	reg_a = rtc_read( RTC_CONTROLA );
	/* lock clock registers for read */
	rtc_write( RTC_CONTROLA, ( reg_a | RTC_CA_READ ));

	sec     = rtc_read( RTC_SECONDS );
	min     = rtc_read( RTC_MINUTES );
	hour    = rtc_read( RTC_HOURS );
	mday    = rtc_read( RTC_DAY_OF_MONTH );
	wday    = rtc_read( RTC_DAY_OF_WEEK );
	mon     = rtc_read( RTC_MONTH );
	year    = rtc_read( RTC_YEAR );
	century = rtc_read( RTC_CENTURY );

	/* unlock clock registers after read */
	rtc_write( RTC_CONTROLA, ( reg_a & ~RTC_CA_READ ));

#ifdef RTC_DEBUG
	printf( "Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon_cent, mday, wday,
		hour, min, sec );
#endif
	tmp->tm_sec  = bcd2bin( sec  & 0x7F );
	tmp->tm_min  = bcd2bin( min  & 0x7F );
	tmp->tm_hour = bcd2bin( hour & 0x3F );
	tmp->tm_mday = bcd2bin( mday & 0x3F );
	tmp->tm_mon  = bcd2bin( mon & 0x1F );
	tmp->tm_wday = bcd2bin( wday & 0x07 );

	/* glue year from century and year in century */
	tmp->tm_year = bcd2bin( year ) +
		( bcd2bin( century & 0x3F ) * 100 );

	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;
#ifdef RTC_DEBUG
	printf( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec );
#endif
	return 0;
}

int rtc_set( struct rtc_time *tmp )
{
	uchar reg_a;
#ifdef RTC_DEBUG
	printf( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif
	/* lock clock registers for write */
	reg_a = rtc_read( RTC_CONTROLA );
	rtc_write( RTC_CONTROLA, ( reg_a | RTC_CA_WRITE ));

	rtc_write( RTC_MONTH, bin2bcd( tmp->tm_mon ));

	rtc_write( RTC_DAY_OF_WEEK, bin2bcd( tmp->tm_wday ));
	rtc_write( RTC_DAY_OF_MONTH, bin2bcd( tmp->tm_mday ));
	rtc_write( RTC_HOURS, bin2bcd( tmp->tm_hour ));
	rtc_write( RTC_MINUTES, bin2bcd( tmp->tm_min ));
	rtc_write( RTC_SECONDS, bin2bcd( tmp->tm_sec ));

	/* break year up into century and year in century */
	rtc_write( RTC_YEAR, bin2bcd( tmp->tm_year % 100 ));
	rtc_write( RTC_CENTURY, bin2bcd( tmp->tm_year / 100 ));

	/* unlock clock registers after read */
	rtc_write( RTC_CONTROLA, ( reg_a  & ~RTC_CA_WRITE ));

	return 0;
}

void rtc_reset (void)
{
	uchar reg_a, reg_b, reg_c;

	reg_a = rtc_read( RTC_CONTROLA );
	reg_b = rtc_read( RTC_CONTROLB );

	if ( reg_b & RTC_CB_OSC_DISABLE )
	{
		printf( "real-time-clock was stopped. Now starting...\n" );
		reg_a |= RTC_CA_WRITE;
		reg_b &= ~RTC_CB_OSC_DISABLE;

		rtc_write( RTC_CONTROLA, reg_a );
		rtc_write( RTC_CONTROLB, reg_b );
	}

	/* make sure read/write clock register bits are cleared */
	reg_a &= ~( RTC_CA_WRITE | RTC_CA_READ );
	rtc_write( RTC_CONTROLA, reg_a );

	reg_c = rtc_read( RTC_CONTROLC );
	if (( reg_c & RTC_CC_BATTERY_FLAG ) == 0 )
		printf( "RTC battery low. Clock setting may not be reliable.\n" );
}

/* ------------------------------------------------------------------------- */

static uchar rtc_read( unsigned int addr )
{
	uchar val = in8( addr );
#ifdef RTC_DEBUG
	printf( "rtc_read: %x:%x\n", addr, val );
#endif
	return( val );
}

static void rtc_write( unsigned int addr, uchar val )
{
#ifdef RTC_DEBUG
	printf( "rtc_write: %x:%x\n", addr, val );
#endif
	out8( addr, val );
}
