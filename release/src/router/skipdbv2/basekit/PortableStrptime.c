/*
 * code found at: http://www.uni-paderborn.de/info/solaris_porting_faq
 */
/*
 * Copyright (c) 1994 Powerdog Industries.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *      This product includes software developed by Powerdog Industries.
 * 4. The name of Powerdog Industries may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY POWERDOG INDUSTRIES ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE POWERDOG INDUSTRIES BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /* A few changes by Steve Dekorte:
  * - renamed function and moved En_US into function
  */
#include "Base.h"
#include "PortableStrptime.h"

/*#ifdef IO_NEEDS_STRPTIME*/

#include <time.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>

#define asizeof(a)      (sizeof (a) / sizeof ((a)[0]))

/*#ifndef sun*/
struct dtconv {
		char    *abbrev_month_names[12];
		char    *month_names[12];
		char    *abbrev_weekday_names[7];
		char    *weekday_names[7];
		char    *time_format;
		char    *sdate_format;
		char    *dtime_format;
		char    *am_string;
		char    *pm_string;
		char    *ldate_format;
};
/*#endif*/

#ifdef SUNOS4
	extern int strncasecmp();
#endif

#if defined(_MSC_VER) && !defined(__SYMBIAN32__)
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
#endif

int readndigits(char **const buf, const size_t count)
{
	int result = 0;
	int i;

	for (i = 0; i < count; i++, (*buf)++) {
		const char digit = **buf;
		if (digit == 0 || !isdigit(digit)) {
			break;
		}
		result *= 10;
		result += digit - '0';
	}

	return result;
}

// TODO rename function when I understand what this function does.
void somethingToDoWithSpaces(const char *const buf, char **const ptr)
{
	if (*buf != 0 && isspace((int)*buf)) {
		while (**ptr != 0 && !isspace((int)**ptr)) {
			(*ptr)++;
		}
	}
}

char *io_strptime(char *buf, char *fmt, struct tm *tm)
{
		struct dtconv En_US = {
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" },
		{ "January", "February", "March", "April",
		  "May", "June", "July", "August",
		  "September", "October", "November", "December" },
		{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
		{ "Sunday", "Monday", "Tuesday", "Wednesday",
		  "Thursday", "Friday", "Saturday" },
		"%H:%M:%S",
		"%m/%d/%y",
		"%a %b %e %T %Z %Y",
		"AM",
		"PM",
		"%A, %B, %e, %Y"
		 };

		char    c,
				*ptr;
		int     i,
				len;

	len = 0;
		ptr = fmt;
		while (*ptr != 0) {
				if (*buf == 0)
						break;

				c = *ptr++;

				if (c != '%') {
						if (isspace((int)c))
								while (*buf != 0 && isspace((int)*buf))
										buf++;
						else if (c != *buf++)
								return NULL;
						continue;
				}

				c = *ptr++;
				switch (c) {
				case 0:
						return NULL;

				case '%':
						if (*buf++ != '%')
								return NULL;
						break;

				case 'C':
						buf = io_strptime(buf, En_US.ldate_format, tm);
						if (buf == 0)
								return NULL;
						break;

				case 'c':
						buf = io_strptime(buf, "%x %X", tm);
						if (buf == 0)
								return NULL;
						break;

				case 'D':
						buf = io_strptime(buf, "%m/%d/%y", tm);
						if (buf == 0)
								return NULL;
						break;

				case 'R':
						buf = io_strptime(buf, "%H:%M", tm);
						if (buf == 0)
								return NULL;
						break;

				case 'r':
						buf = io_strptime(buf, "%I:%M:%S %p", tm);
						if (buf == 0)
								return NULL;
						break;

				case 'T':
						buf = io_strptime(buf, "%H:%M:%S", tm);
						if (buf == 0)
								return NULL;
						break;

				case 'X':
						buf = io_strptime(buf, En_US.time_format, tm);
						if (buf == 0)
								return NULL;
						break;

				case 'x':
						buf = io_strptime(buf, En_US.sdate_format, tm);
						if (buf == 0)
								return NULL;
						break;

				case 'j':
						if (*buf == 0 || isspace((int)*buf))
								break;

						i = readndigits(&buf, 3);
						if (i < 0 || i > 366)
								return NULL;

						tm->tm_yday = i;
						break;

				case 'M':
						if (*buf == 0 || isspace((int)*buf))
								break;

						i = readndigits(&buf, 2);
						if (i < 0 || i > 59)
								return NULL;

						tm->tm_min = i;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'S':
						if (*buf == 0 || isspace((int)*buf))
								break;

						i = readndigits(&buf, 2);
						if (i < 0 || i > 60) // Earlier 61 was also allowed.
								return NULL;

						tm->tm_sec = i;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'H':
				case 'k':
						if (!isdigit((int)*buf))
								return NULL;

						i = readndigits(&buf, 2);
						if (i < 0 || i > 23)
								return NULL;

						tm->tm_hour = i;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'I':
				case 'l':
						if (!isdigit((int)*buf))
								return NULL;

						i = readndigits(&buf, 2);
						if (i < 1 || i > 12)
								return NULL;

						tm->tm_hour = i;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'd':
				case 'e':
						if (!isdigit((int)*buf))
								return NULL;

						i = readndigits(&buf, 2);
						if (i < 1 || i > 31)
								return NULL;

						tm->tm_mday = i;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'm':
						if (!isdigit((int)*buf))
								return NULL;

						i = readndigits(&buf, 2);
						if (i < 1 || i > 12)
								return NULL;

						tm->tm_mon = i - 1;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'Y':
						if (*buf == 0 || isspace((int)*buf))
								break;

						if (!isdigit((int)*buf))
								return NULL;

						i = readndigits(&buf, 4);
						if (i < 0 || i > 9999)
								return NULL;

						tm->tm_year = i - 1900;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'y':
						if (*buf == 0 || isspace((int)*buf))
								break;

						if (!isdigit((int)*buf))
								return NULL;

						i = readndigits(&buf, 2);
						if (i < 0 || i > 99)
								return NULL;

						tm->tm_year = i;

						somethingToDoWithSpaces(buf, &ptr);
						break;

				case 'P':
				case 'p':
						len = strlen(En_US.am_string);
						if (strncasecmp(buf, En_US.am_string, len) == 0) {
								if (tm->tm_hour > 12)
										return NULL;
								if (tm->tm_hour == 12)
										tm->tm_hour = 0;
								buf += len;
								break;
						}

						len = strlen(En_US.pm_string);
						if (strncasecmp(buf, En_US.pm_string, len) == 0) {
								if (tm->tm_hour > 12)
										return NULL;
								if (tm->tm_hour != 12)
										tm->tm_hour += 12;
								buf += len;
								break;
						}

						return NULL;

				case 'A':
				case 'a':
						for (i = 0; i < (int)asizeof(En_US.weekday_names); i ++) {
								len = strlen(En_US.weekday_names[i]);
								if (strncasecmp(buf,
												En_US.weekday_names[i],
												len) == 0)
										break;

								len = strlen(En_US.abbrev_weekday_names[i]);
								if (strncasecmp(buf,
												En_US.abbrev_weekday_names[i],
												len) == 0)
										break;
						}
						if (i == asizeof(En_US.weekday_names))
								return NULL;

						tm->tm_wday = i;
						buf += len;
						break;

				case 'B':
				case 'b':
				case 'h':
						for (i = 0; i < (int)asizeof(En_US.month_names); i ++) {
								len = strlen(En_US.month_names[i]);
								if (strncasecmp(buf,
												En_US.month_names[i],
												len) == 0)
										break;

								len = strlen(En_US.abbrev_month_names[i]);
								if (strncasecmp(buf,
												En_US.abbrev_month_names[i],
												len) == 0)
										break;
						}
						if (i == asizeof(En_US.month_names))
								return NULL;

						tm->tm_mon = i;
						buf += len;
						break;
				}
		}

		return buf;
}

/*#endif*/

