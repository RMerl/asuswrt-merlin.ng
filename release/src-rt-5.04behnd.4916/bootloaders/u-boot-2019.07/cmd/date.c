// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * RTC, Date & Time support: get and set date & time
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

static const char * const weekdays[] = {
	"Sun", "Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur",
};

#ifdef CONFIG_NEEDS_MANUAL_RELOC
#define RELOC(a)	((typeof(a))((unsigned long)(a) + gd->reloc_off))
#else
#define RELOC(a)	a
#endif

int mk_date (const char *, struct rtc_time *);

static struct rtc_time default_tm = { 0, 0, 0, 1, 1, 2000, 6, 0, 0 };

static int do_date(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct rtc_time tm;
	int rcode = 0;
	int old_bus __maybe_unused;

	/* switch to correct I2C bus */
#ifdef CONFIG_DM_RTC
	struct udevice *dev;

	rcode = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (rcode) {
		printf("Cannot find RTC: err=%d\n", rcode);
		return CMD_RET_FAILURE;
	}
#elif defined(CONFIG_SYS_I2C)
	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_RTC_BUS_NUM);
#else
	old_bus = I2C_GET_BUS();
	I2C_SET_BUS(CONFIG_SYS_RTC_BUS_NUM);
#endif

	switch (argc) {
	case 2:			/* set date & time */
		if (strcmp(argv[1],"reset") == 0) {
			puts ("Reset RTC...\n");
#ifdef CONFIG_DM_RTC
			rcode = dm_rtc_reset(dev);
			if (!rcode)
				rcode = dm_rtc_set(dev, &default_tm);
#else
			rtc_reset();
			rcode = rtc_set(&default_tm);
#endif
			if (rcode)
				puts("## Failed to set date after RTC reset\n");
		} else {
			/* initialize tm with current time */
#ifdef CONFIG_DM_RTC
			rcode = dm_rtc_get(dev, &tm);
#else
			rcode = rtc_get(&tm);
#endif
			if (!rcode) {
				/* insert new date & time */
				if (mk_date(argv[1], &tm) != 0) {
					puts ("## Bad date format\n");
					break;
				}
				/* and write to RTC */
#ifdef CONFIG_DM_RTC
				rcode = dm_rtc_set(dev, &tm);
#else
				rcode = rtc_set(&tm);
#endif
				if (rcode) {
					printf("## Set date failed: err=%d\n",
					       rcode);
				}
			} else {
				puts("## Get date failed\n");
			}
		}
		/* FALL TROUGH */
	case 1:			/* get date & time */
#ifdef CONFIG_DM_RTC
		rcode = dm_rtc_get(dev, &tm);
#else
		rcode = rtc_get(&tm);
#endif
		if (rcode) {
			puts("## Get date failed\n");
			break;
		}

		printf ("Date: %4d-%02d-%02d (%sday)    Time: %2d:%02d:%02d\n",
			tm.tm_year, tm.tm_mon, tm.tm_mday,
			(tm.tm_wday<0 || tm.tm_wday>6) ?
				"unknown " : RELOC(weekdays[tm.tm_wday]),
			tm.tm_hour, tm.tm_min, tm.tm_sec);

		break;
	default:
		rcode = CMD_RET_USAGE;
	}

	/* switch back to original I2C bus */
#ifdef CONFIG_SYS_I2C
	i2c_set_bus_num(old_bus);
#elif !defined(CONFIG_DM_RTC)
	I2C_SET_BUS(old_bus);
#endif

	return rcode ? CMD_RET_FAILURE : 0;
}

/*
 * simple conversion of two-digit string with error checking
 */
static int cnvrt2 (const char *str, int *valp)
{
	int val;

	if ((*str < '0') || (*str > '9'))
		return (-1);

	val = *str - '0';

	++str;

	if ((*str < '0') || (*str > '9'))
		return (-1);

	*valp = 10 * val + (*str - '0');

	return (0);
}

/*
 * Convert date string: MMDDhhmm[[CC]YY][.ss]
 *
 * Some basic checking for valid values is done, but this will not catch
 * all possible error conditions.
 */
int mk_date (const char *datestr, struct rtc_time *tmp)
{
	int len, val;
	char *ptr;

	ptr = strchr(datestr, '.');
	len = strlen(datestr);

	/* Set seconds */
	if (ptr) {
		int sec;

		ptr++;
		if ((len - (ptr - datestr)) != 2)
			return (-1);

		len -= 3;

		if (cnvrt2 (ptr, &sec))
			return (-1);

		tmp->tm_sec = sec;
	} else {
		tmp->tm_sec = 0;
	}

	if (len == 12) {		/* MMDDhhmmCCYY	*/
		int year, century;

		if (cnvrt2 (datestr+ 8, &century) ||
		    cnvrt2 (datestr+10, &year) ) {
			return (-1);
		}
		tmp->tm_year = 100 * century + year;
	} else if (len == 10) {		/* MMDDhhmmYY	*/
		int year, century;

		century = tmp->tm_year / 100;
		if (cnvrt2 (datestr+ 8, &year))
			return (-1);
		tmp->tm_year = 100 * century + year;
	}

	switch (len) {
	case 8:			/* MMDDhhmm	*/
		/* fall thru */
	case 10:		/* MMDDhhmmYY	*/
		/* fall thru */
	case 12:		/* MMDDhhmmCCYY	*/
		if (cnvrt2 (datestr+0, &val) ||
		    val > 12) {
			break;
		}
		tmp->tm_mon  = val;
		if (cnvrt2 (datestr+2, &val) ||
		    val > ((tmp->tm_mon==2) ? 29 : 31)) {
			break;
		}
		tmp->tm_mday = val;

		if (cnvrt2 (datestr+4, &val) ||
		    val > 23) {
			break;
		}
		tmp->tm_hour = val;

		if (cnvrt2 (datestr+6, &val) ||
		    val > 59) {
			break;
		}
		tmp->tm_min  = val;

		/* calculate day of week */
		rtc_calc_weekday(tmp);

		return (0);
	default:
		break;
	}

	return (-1);
}

/***************************************************/

U_BOOT_CMD(
	date,	2,	1,	do_date,
	"get/set/reset date & time",
	"[MMDDhhmm[[CC]YY][.ss]]\ndate reset\n"
	"  - without arguments: print date & time\n"
	"  - with numeric argument: set the system date & time\n"
	"  - with 'reset' argument: reset the RTC"
);
