//metadoc Date copyright Steve Dekorte 2002
//metadoc Date license BSD revised

#include "Base.h"

#ifndef DATE_DEFINED
#define DATE_DEFINED 1

#include "Common.h"
#include "Duration.h"
#include "PortableGettimeofday.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	struct timeval  tv;
	struct timezone tz;
} Date;

BASEKIT_API double Date_SecondsFrom1970ToNow(void);

BASEKIT_API Date *Date_new(void);
BASEKIT_API void Date_copy_(Date *self, const Date *other);
BASEKIT_API void Date_free(Date *self);
BASEKIT_API int Date_compare(const Date *self, const Date *other);

BASEKIT_API void Date_now(Date *self);
BASEKIT_API void Date_setToLocalTimeZone(Date *self);
BASEKIT_API double Date_Clock(void);

BASEKIT_API void Date_fromLocalTime_(Date *self, struct tm *t);
BASEKIT_API void Date_fromTime_(Date *self, time_t t);
BASEKIT_API time_t Date_asTime(const Date *self);

// zone

BASEKIT_API void Date_setToLocalTimeZone(Date *self);
struct timezone Date_timeZone(const Date *self);
BASEKIT_API void Date_setTimeZone_(Date *self, struct timezone tz);
BASEKIT_API void Date_convertToTimeZone_(Date *self, struct timezone tz);

// components

BASEKIT_API void Date_setYear_(Date *self, long y);
BASEKIT_API long Date_year(const Date *self);

BASEKIT_API void Date_setMonth_(Date *self, int m);
BASEKIT_API int Date_month(const Date *self);

BASEKIT_API void Date_setDay_(Date *self, int d);
BASEKIT_API int Date_day(const Date *self);

BASEKIT_API void Date_setHour_(Date *self, int h);
BASEKIT_API int Date_hour(const Date *self);

BASEKIT_API void Date_setMinute_(Date *self, int m);
BASEKIT_API int Date_minute(const Date *self);

BASEKIT_API void Date_setSecond_(Date *self, double s);
BASEKIT_API double Date_second(const Date *self);

BASEKIT_API unsigned char Date_isDaylightSavingsTime(const Date *self);
BASEKIT_API int Date_isLeapYear(const Date *self);

// seconds

BASEKIT_API double Date_asSeconds(const Date *self);
BASEKIT_API void Date_fromSeconds_(Date *self, double s);

BASEKIT_API void Date_addSeconds_(Date *self, double s);
BASEKIT_API double Date_secondsSince_(const Date *self, const Date *other);

// format

BASEKIT_API void Date_fromString_format_(Date *self, const char *s, const char *format);

// durations

BASEKIT_API Duration *Date_newDurationBySubtractingDate_(const Date *self, const Date *other);
BASEKIT_API void Date_addDuration_(Date *self, const Duration *d);
BASEKIT_API void Date_subtractDuration_(Date *self, const Duration *d);

BASEKIT_API double Date_secondsSinceNow(const Date *self);

BASEKIT_API UArray *Date_asString(const Date *self, const char *format);

#ifdef __cplusplus
}
#endif
#endif
