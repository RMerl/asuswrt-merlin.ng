// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Andrew May, Viasat Inc, amay@viasat.com
 */

/*
 * M41T11 Serial Access Timekeeper(R) SRAM
 * can you believe a trademark on that?
 */

/* #define DEBUG 1 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

/*
	I Don't have an example config file but this
	is what should be done.

#define CONFIG_RTC_M41T11 1
#define CONFIG_SYS_I2C_RTC_ADDR 0x68
#if 0
#define CONFIG_SYS_M41T11_EXT_CENTURY_DATA
#else
#define CONFIG_SYS_M41T11_BASE_YEAR 2000
#endif
*/

/* ------------------------------------------------------------------------- */
/*
  these are simple defines for the chip local to here so they aren't too
  verbose
  DAY/DATE aren't nice but that is how they are on the data sheet
*/
#define RTC_SEC_ADDR       0x0
#define RTC_MIN_ADDR       0x1
#define RTC_HOUR_ADDR      0x2
#define RTC_DAY_ADDR       0x3
#define RTC_DATE_ADDR      0x4
#define RTC_MONTH_ADDR     0x5
#define RTC_YEARS_ADDR     0x6

#define RTC_REG_CNT        7

#define RTC_CONTROL_ADDR   0x7


#ifndef CONFIG_SYS_M41T11_EXT_CENTURY_DATA

#define REG_CNT            (RTC_REG_CNT+1)

/*
  you only get 00-99 for the year we will asume you
  want from the year 2000 if you don't set the config
*/
#ifndef CONFIG_SYS_M41T11_BASE_YEAR
#define CONFIG_SYS_M41T11_BASE_YEAR 2000
#endif

#else
/* we will store extra year info in byte 9*/
#define M41T11_YEAR_DATA   0x8
#define M41T11_YEAR_SIZE   1
#define REG_CNT            (RTC_REG_CNT+1+M41T11_YEAR_SIZE)
#endif

#define M41T11_STORAGE_SZ  (64-REG_CNT)

int rtc_get (struct rtc_time *tmp)
{
	int rel = 0;
	uchar data[RTC_REG_CNT];

	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_SEC_ADDR, 1, data, RTC_REG_CNT);

	if( data[RTC_SEC_ADDR] & 0x80 ){
		printf( "m41t11 RTC Clock stopped!!!\n" );
		rel = -1;
	}
	tmp->tm_sec  = bcd2bin (data[RTC_SEC_ADDR]  & 0x7F);
	tmp->tm_min  = bcd2bin (data[RTC_MIN_ADDR]  & 0x7F);
	tmp->tm_hour = bcd2bin (data[RTC_HOUR_ADDR] & 0x3F);
	tmp->tm_mday = bcd2bin (data[RTC_DATE_ADDR] & 0x3F);
	tmp->tm_mon  = bcd2bin (data[RTC_MONTH_ADDR]& 0x1F);
#ifndef CONFIG_SYS_M41T11_EXT_CENTURY_DATA
	tmp->tm_year = CONFIG_SYS_M41T11_BASE_YEAR
		+ bcd2bin(data[RTC_YEARS_ADDR])
		+ ((data[RTC_HOUR_ADDR]&0x40) ? 100 : 0);
#else
	{
		unsigned char cent;
		i2c_read(CONFIG_SYS_I2C_RTC_ADDR, M41T11_YEAR_DATA, 1, &cent, M41T11_YEAR_SIZE);
		if( !(data[RTC_HOUR_ADDR] & 0x80) ){
			printf( "m41t11 RTC: cann't keep track of years without CEB set\n" );
			rel = -1;
		}
		if( (cent & 0x1) != ((data[RTC_HOUR_ADDR]&0x40)>>7) ){
			/*century flip store off new year*/
			cent += 1;
			i2c_write(CONFIG_SYS_I2C_RTC_ADDR, M41T11_YEAR_DATA, 1, &cent, M41T11_YEAR_SIZE);
		}
		tmp->tm_year =((int)cent*100)+bcd2bin(data[RTC_YEARS_ADDR]);
	}
#endif
	tmp->tm_wday = bcd2bin (data[RTC_DAY_ADDR]  & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}

int rtc_set (struct rtc_time *tmp)
{
	uchar data[RTC_REG_CNT];

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	data[RTC_SEC_ADDR]    = bin2bcd(tmp->tm_sec) &  0x7F;/*just in case*/
	data[RTC_MIN_ADDR]    = bin2bcd(tmp->tm_min);
	data[RTC_HOUR_ADDR]   = bin2bcd(tmp->tm_hour) & 0x3F;/*handle cent stuff later*/
	data[RTC_DATE_ADDR]   = bin2bcd(tmp->tm_mday) & 0x3F;
	data[RTC_MONTH_ADDR]  = bin2bcd(tmp->tm_mon);
	data[RTC_DAY_ADDR]    = bin2bcd(tmp->tm_wday) & 0x07;

	data[RTC_HOUR_ADDR]   |= 0x80;/*we will always use CEB*/

	data[RTC_YEARS_ADDR]  = bin2bcd(tmp->tm_year%100);/*same thing either way*/
#ifndef CONFIG_SYS_M41T11_EXT_CENTURY_DATA
	if( ((tmp->tm_year - CONFIG_SYS_M41T11_BASE_YEAR) > 200) ||
	    (tmp->tm_year < CONFIG_SYS_M41T11_BASE_YEAR) ){
		printf( "m41t11 RTC setting year out of range!!need recompile\n" );
	}
	data[RTC_HOUR_ADDR] |= (tmp->tm_year - CONFIG_SYS_M41T11_BASE_YEAR) > 100 ? 0x40 : 0;
#else
	{
		unsigned char cent;
		cent = tmp->tm_year ? tmp->tm_year / 100 : 0;
		data[RTC_HOUR_ADDR] |= (cent & 0x1) ? 0x40 : 0;
		i2c_write(CONFIG_SYS_I2C_RTC_ADDR, M41T11_YEAR_DATA, 1, &cent, M41T11_YEAR_SIZE);
	}
#endif
	i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_SEC_ADDR, 1, data, RTC_REG_CNT);

	return 0;
}

void rtc_reset (void)
{
	unsigned char val;
	/* clear all control & status registers */
	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_SEC_ADDR, 1, &val, 1);
	val = val & 0x7F;/*make sure we are running*/
	i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_SEC_ADDR, 1, &val, RTC_REG_CNT);

	i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_CONTROL_ADDR, 1, &val, 1);
	val = val & 0x3F;/*turn off freq test keep calibration*/
	i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_CONTROL_ADDR, 1, &val, 1);
}
