
//metadoc Date copyright Steve Dekorte 2002
//metadoc Date license BSD revised

#define DATE_C
#include "Date.h"
#undef DATE_C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "PortableStrptime.h"
#include "UArray.h"

Date *Date_new(void)
{
	Date *self = (Date *)io_calloc(1, sizeof(Date));
	Date_now(self);
	return self;
}

void Date_copy_(Date *self, const Date *other)
{
	memcpy(self, other, sizeof(Date));
}

void Date_free(Date *self)
{
	io_free(self);
}

int Date_compare(const Date *self, const Date *other)
{
	double s1 = Date_asSeconds(self);
	double s2 = Date_asSeconds(other);

	if (s1 == s2)
	{
		return 0;
	}

	return s1 > s2 ? 1 : -1;
}

double Date_SecondsFrom1970ToNow(void)
{
	double s, us;
	struct timeval timeval;
	struct timezone timezone;

	gettimeofday(&timeval, &timezone);
	s = timeval.tv_sec;
	//s -= timezone.tz_minuteswest * 60;
	us = timeval.tv_usec;

	return s + (us/1000000.0); /* + (60*60);*/
}

void Date_now(Date *self)
{
	double s, us;
	struct timeval timeval;
	struct timezone timezone;

	gettimeofday(&timeval, &timezone);
	s = timeval.tv_sec;
	s -= timezone.tz_minuteswest * 60;
	us = timeval.tv_usec;

	self->tv = timeval;
	self->tz = timezone;
}

double Date_Clock(void)
{
	return ((double)clock())/((double)CLOCKS_PER_SEC);
}

// zone --------------------------------------------------------

void Date_setToLocalTimeZone(Date *self)
{
	struct timeval timeval;
	gettimeofday(&timeval, &(self->tz));
}

struct timezone Date_timeZone(const Date *self)
{
	return self->tz;
}

void Date_setTimeZone_(Date *self, struct timezone tz)
{
	self->tz = tz;
}

void Date_convertToTimeZone_(Date *self, struct timezone tz)
{
	double s = Date_asSeconds(self) +
	((self->tz.tz_minuteswest - tz.tz_minuteswest) * 60);
	Date_fromSeconds_(self, s);
	Date_setTimeZone_(self, tz);
}

// time --------------------------------------------------------

void Date_fromLocalTime_(Date *self, struct tm *t)
{
	Date_fromTime_(self, mktime(t));
}

void Date_fromTime_(Date *self, time_t t)
{
	Date_fromSeconds_(self, (double)t);
}

time_t Date_asTime(const Date *self)
{
	return (time_t)self->tv.tv_sec;
}

// sconds --------------------------------------------------------

double Date_asSeconds(const Date *self)
{
	return ((double)self->tv.tv_sec) + (((double)self->tv.tv_usec) / 1000000.0);
}

void Date_fromSeconds_(Date *self, double s)
{
	long secs = s;
	self->tv.tv_sec = secs;
	self->tv.tv_usec = (s - secs)*1000000;
}

void Date_addSeconds_(Date *self, double s)
{
	long secs = s;
	self->tv.tv_sec += secs;
	self->tv.tv_usec += (s - secs)*1000000;
}

double Date_secondsSince_(const Date *self, const Date *other)
{
	return Date_asSeconds(self) - Date_asSeconds(other);
}

// components --------------------------------------------------------

long Date_year(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return tm->tm_year + 1900;
}

void Date_setYear_(Date *self, long v)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	tm->tm_year = v - 1900;
	self->tv.tv_sec = mktime(tm);
}

int Date_month(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return tm->tm_mon;
}

void Date_setMonth_(Date *self, int v)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	tm->tm_mon = v;
	self->tv.tv_sec = mktime(tm);
}

int Date_day(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return tm->tm_mday;
}

void Date_setDay_(Date *self, int v)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	tm->tm_mday = v;
	self->tv.tv_sec = mktime(tm);
}

int Date_hour(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return tm->tm_hour;
}

void Date_setHour_(Date *self, int v)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	tm->tm_hour = v;
	self->tv.tv_sec = mktime(tm);
}

int Date_minute(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return tm->tm_min;
}

void Date_setMinute_(Date *self, int v)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	tm->tm_min = v;
	self->tv.tv_sec = mktime(tm);
}

double Date_second(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return ((double)tm->tm_sec) + ((double)self->tv.tv_usec)/1000000.0;
}

void Date_setSecond_(Date *self, double v)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	tm->tm_sec = v;
	self->tv.tv_sec = mktime(tm);
	self->tv.tv_usec = (v - ((long)v))*1000000;
}

unsigned char Date_isDaylightSavingsTime(const Date *self)
{
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);
	return (unsigned char)tm->tm_isdst;
}

int Date_isLeapYear(const Date *self)
{
	int year = Date_year(self);

	if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// format --------------------------------------------------------

static struct tm EmptyTM(void)
{
	time_t tmp = 0;
	struct tm *tt = localtime(&tmp);
	struct tm t;

	memcpy(&t, tt, sizeof(struct tm));
	t.tm_sec  = 0;
	t.tm_min  = 0;
	t.tm_hour = 0;
	t.tm_mday = 0;
	t.tm_mon  = 0;
	t.tm_year = 0;
	t.tm_wday = 0;
	t.tm_yday = 0;
	return t;
}

void Date_fromString_format_(Date *self, const char *s, const char *format)
{
	struct tm tm = EmptyTM();
	io_strptime((char *)s, (char *)format, &tm);
	/*
	printf("year  = %i\n", t.tm_year);
	printf("month = %i\n", t.tm_mon);
	printf("day   = %i\n", t.tm_mday);
	printf("hour  = %i\n", t.tm_hour);
	printf("min   = %i\n", t.tm_min);
	printf("sec   = %i\n", t.tm_sec);
	*/
	Date_fromSeconds_(self, mktime(&tm));
}

// durations --------------------------------------------------------

Duration *Date_newDurationBySubtractingDate_(const Date *self, const Date *other)
{
	double d = Date_secondsSince_(self, other);
	return Duration_newWithSeconds_(d);
}

void Date_addDuration_(Date *self, const Duration *d)
{
	Date_addSeconds_(self, Duration_asSeconds(d));
}

void Date_subtractDuration_(Date *self, const Duration *d)
{
	Date_addSeconds_(self, -Duration_asSeconds(d));
}

// -----------------------------------------------------------

double Date_secondsSinceNow(const Date *self)
{
	double n = Date_SecondsFrom1970ToNow();
	double s = Date_asSeconds(self);
	return n - s;
}

// format --------------------------------------------------------

UArray *Date_asString(const Date *self, const char *format)
{
	UArray *u = UArray_new();
	time_t t = self->tv.tv_sec;
	struct tm *tm = localtime(&t);

	// what about unicode formats?
	UArray_setSize_(u, 1024 + strlen(format));
	strftime((char *)UArray_bytes(u), 1024, format, tm);
	UArray_setSize_(u, strlen((char *)UArray_bytes(u)));

	return u;
}
