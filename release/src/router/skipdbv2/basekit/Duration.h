//metadoc Duration copyright Steve Dekorte 2002
//metadoc Duration license BSD revised

#ifndef DURATION_DEFINED
#define DURATION_DEFINED 1

#include "Common.h"
#include "UArray.h"
#include "PortableGettimeofday.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	double seconds;
} Duration;

BASEKIT_API Duration *Duration_new(void);
BASEKIT_API Duration *Duration_newWithSeconds_(double s);
BASEKIT_API Duration *Duration_clone(const Duration *self);
BASEKIT_API void Duration_copy_(Duration *self, const Duration *other);

BASEKIT_API void Duration_free(Duration *self);
BASEKIT_API int Duration_compare(const Duration *self, const Duration *other);

// components

BASEKIT_API int Duration_years(const Duration *self);
BASEKIT_API void Duration_setYears_(Duration *self, double y);

BASEKIT_API int Duration_days(const Duration *self);
BASEKIT_API void Duration_setDays_(Duration *self, double d);

BASEKIT_API int Duration_hours(const Duration *self);
BASEKIT_API void Duration_setHours_(Duration *self, double m);

BASEKIT_API int Duration_minutes(const Duration *self);
BASEKIT_API void Duration_setMinutes_(Duration *self, double m);

BASEKIT_API double Duration_seconds(const Duration *self);
BASEKIT_API void Duration_setSeconds_(Duration *self, double s);

// total seconds

BASEKIT_API double Duration_asSeconds(const Duration *self);
BASEKIT_API void Duration_fromSeconds_(Duration *self, double s);

// strings

BASEKIT_API UArray *Duration_asUArrayWithFormat_(const Duration *self, const char *format);
BASEKIT_API void Duration_print(const Duration *self);

// math

BASEKIT_API void Duration_add_(Duration *self, const Duration *other);
BASEKIT_API void Duration_subtract_(Duration *self, const Duration *other);

#ifdef __cplusplus
}
#endif
#endif
