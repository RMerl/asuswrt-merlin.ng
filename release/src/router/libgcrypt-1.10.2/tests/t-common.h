/* t-common.h - Common code for the tests.
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>

#include "../src/gcrypt.h"

#ifndef PGM
# error Macro PGM not defined.
#endif
#ifndef _GCRYPT_CONFIG_H_INCLUDED
# error config.h not included
#endif

/* A couple of useful macros.  */
#ifndef DIM
# define DIM(v)		     (sizeof(v)/sizeof((v)[0]))
#endif
#define DIMof(type,member)   DIM(((type *)0)->member)
#define xmalloc(a)    gcry_xmalloc ((a))
#define xcalloc(a,b)  gcry_xcalloc ((a),(b))
#define xstrdup(a)    gcry_xstrdup ((a))
#define xfree(a)      gcry_free ((a))
#define my_isascii(c) (!((c) & 0x80))
#define digitp(p)     (*(p) >= '0' && *(p) <= '9')
#define hexdigitp(a)  (digitp (a)                     \
                       || (*(a) >= 'A' && *(a) <= 'F')  \
                       || (*(a) >= 'a' && *(a) <= 'f'))
#define xtoi_1(p)     (*(p) <= '9'? (*(p)- '0'): \
                       *(p) <= 'F'? (*(p)-'A'+10):(*(p)-'a'+10))
#define xtoi_2(p)     ((xtoi_1(p) * 16) + xtoi_1((p)+1))
#define xmalloc(a)    gcry_xmalloc ((a))
#define xcalloc(a,b)  gcry_xcalloc ((a),(b))
#define xstrdup(a)    gcry_xstrdup ((a))
#define xfree(a)      gcry_free ((a))
#define pass()        do { ; } while (0)


/* Standard global variables.  */
static const char *wherestr;
static int verbose;
static int debug;
static int error_count;
static int die_on_error;

/* If we have a decent libgpg-error we can use some gcc attributes.  */
#ifdef GPGRT_ATTR_NORETURN
static void die (const char *format, ...)
  GPGRT_ATTR_UNUSED GPGRT_ATTR_NR_PRINTF(1,2);
static void fail (const char *format, ...)
  GPGRT_ATTR_UNUSED GPGRT_ATTR_PRINTF(1,2);
static void info (const char *format, ...) \
  GPGRT_ATTR_UNUSED GPGRT_ATTR_PRINTF(1,2);
#endif /*GPGRT_ATTR_NORETURN*/


/* Reporting functions.  */
static void
die (const char *format, ...)
{
  va_list arg_ptr ;

  /* Avoid warning.  */
  (void) debug;

  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  flockfile (stderr);
#endif
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format) ;
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
#ifdef HAVE_FLOCKFILE
  funlockfile (stderr);
#endif
  exit (1);
}


static void
fail (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
#ifdef HAVE_FLOCKFILE
  flockfile (stderr);
#endif
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
#ifdef HAVE_FLOCKFILE
  funlockfile (stderr);
#endif
  if (die_on_error)
    exit (1);
  error_count++;
  if (error_count >= 50)
    die ("stopped after 50 errors.");
}


static void
info (const char *format, ...)
{
  va_list arg_ptr;

  if (!verbose)
    return;
#ifdef HAVE_FLOCKFILE
  flockfile (stderr);
#endif
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
  va_end (arg_ptr);
#ifdef HAVE_FLOCKFILE
  funlockfile (stderr);
#endif
}


/* Convenience macro for initializing gcrypt with error checking.  */
#define xgcry_control(cmd)                                      \
  do {                                                          \
    gcry_error_t err__ = gcry_control cmd;                      \
    if (err__)                                                  \
      die ("line %d: gcry_control (%s) failed: %s",             \
           __LINE__, #cmd, gcry_strerror (err__));              \
  } while (0)


/* Split a string into colon delimited fields A pointer to each field
 * is stored in ARRAY.  Stop splitting at ARRAYSIZE fields.  The
 * function modifies STRING.  The number of parsed fields is returned.
 * Note that leading and trailing spaces are not removed from the fields.
 * Example:
 *
 *   char *fields[2];
 *   if (split_fields (string, fields, DIM (fields)) < 2)
 *     return  // Not enough args.
 *   foo (fields[0]);
 *   foo (fields[1]);
 */
#ifdef NEED_EXTRA_TEST_SUPPORT
static int
split_fields_colon (char *string, char **array, int arraysize)
{
  int n = 0;
  char *p, *pend;

  p = string;
  do
    {
      if (n == arraysize)
        break;
      array[n++] = p;
      pend = strchr (p, ':');
      if (!pend)
        break;
      *pend++ = 0;
      p = pend;
    }
  while (*p);

  return n;
}
#endif /*NEED_EXTRA_TEST_SUPPORT*/
