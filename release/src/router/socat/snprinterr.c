/* snprinterr.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */

/* a function similar to vsnprintf() but it just handles %m */

#include "config.h"

#include <stddef.h>	/* ptrdiff_t */
#include <ctype.h>	/* isdigit() */
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif
#include <sys/utsname.h>
#include <time.h>	/* time_t, strftime() */
#include <sys/time.h>	/* gettimeofday() */
#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "snprinterr.h"

#define HAVE_STRERROR_R 0
/* replace %m in format with actual strerror() message, write result to *str.
   keep other % formats unchanged!
   writes at most size chars including the terminating \0 to *str
   returns the number of bytes in the output without terminating \0
   result is always \0 terminated except when size==0
 */
int snprinterr(char *str, size_t size, const char *format) {
   char c;
   int full = 0;	/* 1 means: there is no space left in * str for more data or \0 */
   int count = 0;
   if (size == 0)  return 0;
   if (count >= size)  full = 1;
   while (c = *format++) {
      if (c == '%') {
	 c = *format++;
	 switch (c) {
	 case '\0':
	    ++count; if (!full) { (*str++ = '%'); if (count+1 >= size) full = 1; }
	    break;
	 default:
	    ++count; if (!full) { (*str++ = '%'); if (count+1 >= size) full = 1; }
	    ++count; if (!full) { (*str++ = c);   if (count+1 >= size) full = 1; }
	    break;
	 case 'm':
	    {
#if HAVE_STRERROR_R
#              define BUFLEN 64
	       char buf[BUFLEN] = "";
#endif /* HAVE_STRERROR_R */
	       char *bufp;
#if !HAVE_STRERROR_R
	       bufp = strerror(errno);
#else
	       /* there are two versions floating around... */
#  if 1	/* GNU version */
	       bufp = strerror_r(errno, buf, BUFLEN);
#  else	/* standard version */
	       strerror_r(errno, buf, BUFLEN);
	       bufp = buf;
#  endif
#endif /* HAVE_STRERROR_R */
	       while ((c = *bufp++) != '\0') {
		  ++count; if (!full) { (*str++ = c);   if (count+1 >= size) full = 1; }
	       }
	    }
	    c = ' ';	/* not \0 ! */
	    break;
	 }
	 if (c == '\0')  break;
      } else {
	 ++count; if (!full) { (*str++ = c); if (count+1 >= size)  full = 1; }
      }
   }
   *str++ = '\0';	/* always write terminating \0 */
   return count;
#undef BUFLEN
}

