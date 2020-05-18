#ifndef PORTABLEGETTIMEOFDAY_DEFINED
	#define PORTABLEGETTIMEOFDAY_DEFINED 1

	#ifdef __cplusplus
		extern "C" {
	#endif

	#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(_MSC_VER)
		#if defined(_MSC_VER)
			#include <winsock2.h>
			/*struct timeval
			{
				long tv_sec;
				long tv_usec;
			};*/
		#else
			/* for MingW */
			#include <sys/time.h>
		#endif

		#if defined(__MINGW32__) && (3 < __MINGW32_MAJOR_VERSION || 3 == __MINGW32_MAJOR_VERSION && 9 < __MINGW32_MINOR_VERSION)
		#else
			struct timezone
			{
				int tz_minuteswest; /* of Greenwich */
				int tz_dsttime;     /* type of dst correction to apply */
			};

						#include "Common.h"
			BASEKIT_API extern void gettimeofday(struct timeval *tv, struct timezone *tz);
		#endif

	#else
		#include <sys/time.h>
	#endif

	#ifdef __cplusplus
		}
	#endif

#endif

double secondsSince1970(void);
