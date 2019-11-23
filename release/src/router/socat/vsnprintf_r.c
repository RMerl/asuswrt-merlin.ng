/* vsnprintf_r.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */

/* a reduced but async-signal-safe and thread-safe version of vsnprintf */

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

#include "vsnprintf_r.h"

/* helper functions for vsnprintf_r():
   e.g. convert an unsigned long to decimal string.
   in: field (must be long enough for all digits and \0
       n: length of field in bytes
       ulo: the value
   returns: the pointer to the result string (need not be ==field)
*/

/* this function converts an unsigned long number to decimal ASCII
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it terminates result with \0
 */
static char *_diag_ulong_to_dec(char *field, size_t n, unsigned long ulo) {
   char *np = field+n;	/* point to the end */

   if (n == 0)  return NULL;
   *--np = '\0';	/* \0 in last char of string */
   /* this is not optimal - uses much CPU, but simple to implement */
   /*  calculate the result from right to left */
   do { if (np==field) return NULL; *--np = '0'+(ulo%10); } while (ulo/=10);
   return np;
}

/* this function converts an unsigned long number to decimal ASCII
   and pads it with space or '0' when size and leading0 are set appropriately
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it reduces size to n-1 if it is greater or equal
   it terminates result with \0
 */
static char *diag_ulong_to_dec(char *field, size_t n, unsigned long ulo, int leading0, int size) {
   char *np;
   char c;
   int i;

   if (n == 0)  return NULL;
   np = _diag_ulong_to_dec(field, n, ulo);
   if (np == NULL) return np;
   if (size) {
      if (size >= n)  size = n-1;
      if (leading0) {
	 c = '0';
      } else {
	 c = ' ';
      }
      i = size - strlen(np);
      while (--i >= 0) {
	 *--np = c;
      }
   }	     
   return np;
}

/* this function converts a signed long number to decimal ASCII  
   and pads it with space or '0' when size and leading0 are set appropriately
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it reduces size to n-1 if it is greater or equal
   it terminates result with \0
 */
/* like diag_ulong_to_dec but signed; fields need also space for '-' */
static char *diag_long_to_dec(char *field, size_t n, long lo, int leading0, int size) {
   char *np;
   int minus;
   unsigned long ulo;
   int i;

   if ((minus = (lo < 0))) {
      ulo = (~lo)+1;
   } else {
      ulo = lo;
   }
   np = _diag_ulong_to_dec(field, n, ulo);
   if (np == NULL)  return np;

   if (size) {
      if (size >= n)  size = n-1;
      i = size - strlen(np);
      if (leading0) {
	 if (minus) --i; 
	 while (--i >= 0) {
	    *--np = '0';
	 }
	 if (minus)  *--np = '-';
      } else {
	 if (minus)  { *--np = '-'; --i; }
	 while (--i >= 0) {
	    *--np = ' ';
	 }
      }
   } else {
      if (minus)  *--np = '-';
   }
   return np;
}

/* this function converts an unsigned long number to hexadecimal ASCII
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it terminates result with \0
 */
static char *diag_ulong_to_hex(char *field, size_t n, unsigned long ulo, int leading0, size_t size) {
   char *np = field+n;	/* point to the end */
   int i;
   char c;

   if (n == 0)  return NULL;
   *--np = '\0';	/* \0 in last char of string */
   /*  calculate the result from right to left */
   do { if (np==field) return NULL; i = (ulo&0x0f);
      *--np = (i<10?'0':('a'-10))+i; }
   while (ulo>>=4);
   if (size) {
      if (size >= n)  size = n-1;
      if (leading0) {
	 c = '0';
      } else {
	 c = ' ';
      }
      i = size - strlen(np);
      while (--i >= 0) {
	 *--np = c;
      }
   }	     
   return np;
}

/* this function converts an unsigned long number to octal ASCII
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it terminates result with \0
 */
static char *diag_ulong_to_oct(char *field, size_t n, unsigned long ulo, int leading0, size_t size) {
   char *np = field+n;	/* point to the end */
   int i;
   char c;

   if (n == 0)  return NULL;
   *--np = '\0';	/* \0 in last char of string */
   /* calculate the result from right to left */
   do { if (np==field) return NULL;  i = (ulo&0x07); *--np = '0'+i; }
   while (ulo>>=3);
   if (size) {
      if (size >= n)  size = n-1;
      if (leading0) {
	 c = '0';
      } else {
	 c = ' ';
      }
      i = size - strlen(np);
      while (--i >= 0) {
	 *--np = c;
      }
   }	     
   return np;
}


#if HAVE_TYPE_LONGLONG

/* this function converts an unsigned long long number to decimal ASCII
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it terminates result with \0
 */
static char *_diag_ulonglong_to_dec(char *field, size_t n, unsigned long long ull) {
   char *np = field+n;	/* point to the end */

   if (n == 0)  return NULL;
   *--np = '\0';	/* \0 in last char of string */
   /* this is not optimal - uses much CPU, but simple to implement */
   /* calculate the result from right to left */
   do { if (np==field) return NULL; *--np = '0'+(ull%10); } while (ull/=10);
   return np;
}

/* this function converts an unsigned long long number to decimal ASCII
   and pads it with space or '0' when size and leading0 are set appropriately
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it reduces size to n-1 if it is greater or equal
   it terminates result with \0
 */
static char *diag_ulonglong_to_dec(char *field, size_t n, unsigned long long ull, int leading0, int size) {
   char *np;
   char c;
   int i;

   if (n == 0)  return NULL;
   np = _diag_ulonglong_to_dec(field, n, ull);
   if (size) {
      if (size >= n)  size = n-1;
      if (leading0) {
	 c = '0';
      } else {
	 c = ' ';
      }
      i = size - strlen(np);
      while (i-- > 0) {
	 *--np = c;
      }
   }	     
   return np;
}

/* this function converts a signed long long number to decimal ASCII  
   and pads it with space or '0' when size and leading0 are set appropriately
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it reduces size to n-1 if it is greater or equal
   it terminates result with \0
 */
/* like diag_ulonglong_to_dec but signed; fields need also space for '-' */
static char *diag_longlong_to_dec(char *field, size_t n, long long ll, int leading0, int size) {
   char *np;
   int minus;
   unsigned long ull;
   int i;

   if ((minus = (ll < 0))) {
      ull = (~ll)+1;
   } else {
      ull = ll;
   }
   np = _diag_ulonglong_to_dec(field, n, ull);
   if (np == NULL)  return np;

   if (size) {
      if (size >= n)  size = n-1;
      i = size - strlen(np);
      if (leading0) {
	 if (minus) --i; 
	 while (--i >= 0) {
	    *--np = '0';
	 }
	 if (minus)  *--np = '-';
      } else {
	 if (minus)  { *--np = '-'; --i; }
	 while (--i >= 0) {
	    *--np = ' ';
	 }
      }
   }	     
   return np;
}

/* this function converts an unsigned long long number to hexadecimal ASCII
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it terminates result with \0
 */
static char *diag_ulonglong_to_hex(char *field, size_t n, unsigned long long ull, int leading0, size_t size) {
   char *np = field+n;	/* point to the end */
   ptrdiff_t i;
   char c;

   if (n == 0)  return NULL;
   *--np = '\0';	/* \0 in last char of string */
   /* calculate the result from right to left */
   do { if (np==field) return NULL; i = (ull&0x0f);
      *--np = (i<10?'0':('a'-10))+i; }
   while (ull>>=4);
   if (size) {
      if (size >= n)  size = n-1;
      if (leading0) {
	 c = '0';
      } else {
	 c = ' ';
      }
      i = size - strlen(np);
      while (i-- > 0) {
	 *--np = c;
      }
   }	     
   return np;
}

/* this function converts an unsigned long long number to octal ASCII
   it is async signal safe and thread safe
   it returns NULL if n==0
   it returns NULL if field is too short to hold the result
   it returns a pointer to the result string (somewhere within field)
   it terminates result with \0
 */
static char *diag_ulonglong_to_oct(char *field, size_t n, unsigned long long ull, int leading0, size_t size) {
   char *np = field+n;	/* point to the end */
   int i;
   char c;

   if (n == 0)  return NULL;
   *--np = '\0';	/* \0 in last char of string */
   /* calculate the result from right to left */
   do { if (np==field) return NULL;  i = (ull&0x07); *--np = '0'+i; }
   while (ull>>=3);
   if (size) {
      if (size >= n)  size = n-1;
      if (leading0) {
	 c = '0';
      } else {
	 c = ' ';
      }
      i = size - strlen(np);
      while (--i >= 0) {
	 *--np = c;
      }
   }	     
   return np;
}

#endif /* HAVE_TYPE_LONGLONG */


/* this function is designed as a variant of vsnprintf(3) but async signal safe
   and thread safe
   it currently only implements a subset of the format directives
   returns <0 if an error occurred (no scenario know yet)
   returns >=size if output is truncated (conforming to C99 standard)
*/
int vsnprintf_r(char *str, size_t size, const char *format, va_list ap) {
   size_t i = 0;
   char c;
   int full = 0;		/* indicate if output buffer full */

   --size;	/* without trailing \0 */
   while (c = *format++) {
      if (c == '\\') {
	 
      } else if (c == '%') {
#if HAVE_TYPE_LONGLONG
#	 define num_buff_len ((sizeof(unsigned long long)*8+2)/3+1)	/* hold up to u long long in octal w/ \0 */
#else
#	 define num_buff_len ((sizeof(unsigned long)*8+2)/3+1)];	/* hold up to u long in octal w/ \0 */
#endif
	 char lengthmod = '\0';	/* 'h' 'l' 'L' 'z' */
	 int leading0 = 0;	/* or 1 */
	 size_t fsize = 0;	/* size of field */
	 const char *st;		/* string */
	 long  lo; unsigned long  ulo;
#if HAVE_TYPE_LONGLONG
	 long long ll; unsigned long long ull;
#endif
	 char field[num_buff_len];	/* result of number conversion */
	 char *np;			/* num pointer */

	 c = *format++;
	 if (c == '\0')  { break; }
	 
	 /* flag characters */
	 switch (c) {
	 case '0': leading0 = 1;  c = *format++;  break;
	    /* not handled: '#' '-' ' ' '+' '\'' */
	 }
	 if (c == '\0')  { break; }

	 /* field width */
	 switch (c) {
	 case '1': case '2': case '3': case '4':
	 case '5': case '6': case '7': case '8': case '9':
	    do {
	       fsize = 10*fsize+(c-'0');
	       c = *format++;
	    } while (c && isdigit(c));
	    break;
	 }
	 if (c == '\0')  { break; }

	 /* precision - not handles */

	 /* length modifier */
	 switch (c) {
	    /* not handled: 'q' 'j' 't' */
	    /* handled: 'h' 'hh'->'H' 'z' 'Z'->'z' 'l' 'll'->'L' 'L' */
	 case 'Z': c = 'z'; /* fall through */
#if HAVE_TYPE_LONGLONG
	 case 'L':
#endif
	 case 'z': lengthmod = c; c = *format++; break;
	 case 'h':
	    lengthmod = c;
	    if ((c = *format++) == 'h') {
	       lengthmod = 'H'; c = *format++; 
	    }
	    break;
	 case 'l': 
	    lengthmod = c;
	    if ((c = *format++) == 'l') {
	       lengthmod = 'L'; c = *format++; 
	    }
	    break;
	 }
	 if (c == '\0')  { break; }

	 /* conversion specifier */
	 switch (c) {
	 case 'c': c = va_arg(ap, int); /* fall through */
	 case '%': *str++ = c; if (++i == size) { full = 1; } break;

	 case 's': st = va_arg(ap, const char *);
	    /* no modifier handled! */
	    while (c = *st++) {
	       *str++ = c;
	       if (++i == size) { full = 1; break; }
	    }
	    break;
	 case 'd':
#if HAVE_TYPE_LONGLONG
	    if (lengthmod == 'L') {
	       ll = va_arg(ap, long long);
	       np = diag_longlong_to_dec(field, num_buff_len, ll, leading0, fsize);
	       while (c = *np++)  {
		  *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    } else
#endif
	    {
	       switch (lengthmod) {
	       case 'l': lo = va_arg(ap, long); break;
	       case 'z': lo = va_arg(ap, ptrdiff_t); break;
	       default: lo = va_arg(ap, int); break;
	       }
	       np = diag_long_to_dec(field, num_buff_len, lo, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    }
	    break;
	 case 'u':
#if HAVE_TYPE_LONGLONG
	    if (lengthmod == 'L') {
	       ull = va_arg(ap, unsigned long long);
	       np = diag_ulonglong_to_dec(field, num_buff_len, ull, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    } else
#endif
	    {
	       switch (lengthmod) {
	       case 'l': ulo = va_arg(ap, unsigned long); break;
	       case 'z': ulo = va_arg(ap, size_t); break;
	       default: ulo = va_arg(ap, unsigned int); break;
	       }
	       np = diag_ulong_to_dec(field, num_buff_len, ulo, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    }
	    break;
	 case 'p':
	    ulo = va_arg(ap, size_t);
	    np = diag_ulong_to_hex(field, num_buff_len, ulo, leading0, fsize);
	    *str++ = '0'; if (++i == size) { full = 1; break; }
	    *str++ = 'x'; if (++i == size) { full = 1; break; }
	    while (c = *np++)  { *str++ = c;
	       if (++i == size) { full = 1; break; }
	    }
	    break;
	 case 'x':
#if HAVE_TYPE_LONGLONG
	    if (lengthmod == 'L') {
	       ull = va_arg(ap, unsigned long long);
	       np = diag_ulonglong_to_hex(field, num_buff_len, ull, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    } else
#endif
	    {
	       switch (lengthmod) {
	       case 'l': ulo = va_arg(ap, unsigned long); break;
	       case 'z': ulo = va_arg(ap, size_t); break;
	       default: ulo = va_arg(ap, unsigned int); break;
	       }
	       np = diag_ulong_to_hex(field, num_buff_len, ulo, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    }
	    break;
	 case 'o':
#if HAVE_TYPE_LONGLONG
	    if (lengthmod == 'L') {
	       ull = va_arg(ap, unsigned long long);
	       np = diag_ulonglong_to_oct(field, num_buff_len, ull, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) break;
	       }
	    } else
#endif
	    {
	       switch (lengthmod) {
	       case 'l': ulo = va_arg(ap, unsigned long); break;
	       case 'z': ulo = va_arg(ap, size_t); break;
	       default: ulo = va_arg(ap, unsigned int); break;
	       }
	       np = diag_ulong_to_oct(field, num_buff_len, ulo, leading0, fsize);
	       while (c = *np++)  { *str++ = c;
		  if (++i == size) { full = 1; break; }
	       }
	    }
	    break;
	 default:
	    *str++ = c;  if (++i == size) { full = 1; break; }
	 }
	 if (full)  break;
      } else {
	 *str++ = c;
	 if (++i == size)  break;
      }
   }
   *str = '\0';
   return i;
}

int snprintf_r(char *str, size_t size, const char *format, ...) {
   int result;
   va_list ap;
   va_start(ap, format);
   result = vsnprintf_r(str, size, format, ap);
   va_end(ap);
   return result;
}

