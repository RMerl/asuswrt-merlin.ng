#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

#include <linux/types.h>

#define _DEFUN(a,b,c) a(c)
#define _CONST const
#define _AND ,

#define _REENT_ONLY

#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK	7
#define MONSPERYEAR	12

#define YEAR_BASE	1900
#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)


/* Used by other time functions.  */
struct tm {
    int tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
    int tm_min;                   /* Minutes.     [0-59] */
    int tm_hour;                  /* Hours.       [0-23] */
    int tm_mday;                  /* Day.         [1-31] */
    int tm_mon;                   /* Month.       [0-11] */
    int tm_year;                  /* Year - 1900.  */
    int tm_wday;                  /* Day of week. [0-6] */
    int tm_yday;                  /* Days in year.[0-365] */
    int tm_isdst;                 /* DST.         [-1/0/1]*/

# ifdef __USE_BSD
    long int tm_gmtoff;           /* Seconds east of UTC.  */
    __const char *tm_zone;        /* Timezone abbreviation.  */
# else
    long int __tm_gmtoff;         /* Seconds east of UTC.  */
    __const char *__tm_zone;      /* Timezone abbreviation.  */
# endif
};

static inline char *
_DEFUN (asctime_r, (tim_p, result),
	_CONST struct tm *tim_p _AND
	char *result)
{
    static _CONST char day_name[7][3] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static _CONST char mon_name[12][3] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    sprintf (result, "%.3s %.3s %.2d %.2d:%.2d:%.2d %d\n",
	    day_name[tim_p->tm_wday],
	    mon_name[tim_p->tm_mon],
	    tim_p->tm_mday, tim_p->tm_hour, tim_p->tm_min,
	    tim_p->tm_sec, 1900 + tim_p->tm_year);
    return result;
}

static inline struct tm *
_DEFUN (localtime_r, (tim_p, res),
	_CONST time_t * tim_p _AND
	struct tm *res)
{
    static _CONST int mon_lengths[2][MONSPERYEAR] = {
      {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    } ;

    static _CONST int year_lengths[2] = {
      365,
      366
    } ;

    long days, rem;
    int y;
    int yleap;
    _CONST int *ip;

    days = ((long) *tim_p) / SECSPERDAY;
    rem = ((long) *tim_p) % SECSPERDAY;
    while (rem < 0)
    {
	rem += SECSPERDAY;
	--days;
    }

    /* compute hour, min, and sec */
    res->tm_hour = (int) (rem / SECSPERHOUR);
    rem %= SECSPERHOUR;
    res->tm_min = (int) (rem / SECSPERMIN);
    res->tm_sec = (int) (rem % SECSPERMIN);

    /* compute day of week */
    if ((res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
	res->tm_wday += DAYSPERWEEK;

    /* compute year & day of year */
    y = EPOCH_YEAR;
    if (days >= 0)
    {
	for (;;)
	{
	    yleap = isleap(y);
	    if (days < year_lengths[yleap])
		break;
	    y++;
	    days -= year_lengths[yleap];
	}
    }
    else
    {
	do
	{
	    --y;
	    yleap = isleap(y);
	    days += year_lengths[yleap];
	} while (days < 0);
    }

    res->tm_year = y - YEAR_BASE;
    res->tm_yday = days;
    ip = mon_lengths[yleap];
    for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
	days -= ip[res->tm_mon];
    res->tm_mday = days + 1;

    /* set daylight saving time flag */
    res->tm_isdst = -1;

    return (res);
}

static inline char *
_DEFUN (ctime_r, (tim_p, result),
	_CONST time_t * tim_p _AND
	char * result)

{
    struct tm tm;
    return asctime_r (localtime_r (tim_p, &tm), result);
}

#endif
