/* estream-printf.c - Versatile mostly C-99 compliant printf formatting
 * Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014 g10 Code GmbH
 *
 * This file is part of Libestream.
 *
 * Libestream is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libestream is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libestream; if not, see <https://www.gnu.org/licenses/>.
 *
 * ALTERNATIVELY, Libestream may be distributed under the terms of the
 * following license, in which case the provisions of this license are
 * required INSTEAD OF the GNU General Public License. If you wish to
 * allow use of your version of this file only under the terms of the
 * GNU General Public License, and not to allow others to use your
 * version of this file under the terms of the following license,
 * indicate your decision by deleting this paragraph and the license
 * below.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*  Required autoconf tests:

    AC_TYPE_LONG_LONG_INT            defines HAVE_LONG_LONG_INT
    AC_TYPE_LONG_DOUBLE              defines HAVE_LONG_DOUBLE
    AC_TYPE_INTMAX_T                 defines HAVE_INTMAX_T
    AC_TYPE_UINTMAX_T                defines HAVE_UINTMAX_T
    AC_CHECK_TYPES([ptrdiff_t])      defines HAVE_PTRDIFF_T
    AC_CHECK_SIZEOF([unsigned long]) defines SIZEOF_UNSIGNED_LONG
    AC_CHECK_SIZEOF([void *])        defines SIZEOF_VOID_P
                                             HAVE_LANGINFO_THOUSEP

    Note that the file estream.m4 provides the autoconf macro
    ESTREAM_PRINTF_INIT which runs all required checks.
    See estream-printf.h for ways to tune this code.

  Missing stuff:  wchar and wint_t
                  thousep in pr_float.

*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined(_WIN32) && !defined(HAVE_W32_SYSTEM)
# define HAVE_W32_SYSTEM 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stddef.h>
#if defined(HAVE_INTMAX_T) || defined(HAVE_UINTMAX_T)
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#ifdef HAVE_LANGINFO_THOUSEP
#include <langinfo.h>
#endif
#ifdef _ESTREAM_PRINTF_EXTRA_INCLUDE
# include _ESTREAM_PRINTF_EXTRA_INCLUDE
#endif
#include "estream-printf.h"

/* #define DEBUG 1 */


/* Allow redefinition of asprintf used realloc function.  */
#if defined(_ESTREAM_PRINTF_REALLOC)
#define my_printf_realloc(a,b) _ESTREAM_PRINTF_REALLOC((a),(b))
#else
#define my_printf_realloc(a,b) fixed_realloc((a),(b))
#endif

/* A wrapper to set ERRNO.  */
#define _set_errno(a)  do { errno = (a); } while (0)


/* Calculate array dimension.  */
#ifndef DIM
#define DIM(array) (sizeof (array) / sizeof (*array))
#endif


/* We allow for that many args without requiring malloced memory. */
#define DEFAULT_MAX_ARGSPECS  5

/* We allow for that many values without requiring malloced memory. */
#define DEFAULT_MAX_VALUES  8

/* We allocate this many new array argspec elements each time.  */
#define ARGSPECS_BUMP_VALUE   10

/* Special values for the field width and the precision.  */
#define NO_FIELD_VALUE   (-1)
#define STAR_FIELD_VALUE (-2)

/* Bit valuues used for the conversion flags. */
#define FLAG_GROUPING   1
#define FLAG_LEFT_JUST  2
#define FLAG_PLUS_SIGN  4
#define FLAG_SPACE_PLUS 8
#define FLAG_ALT_CONV   16
#define FLAG_ZERO_PAD   32

/* Constants used the length modifiers.  */
typedef enum
  {
    LENMOD_NONE = 0,
    LENMOD_CHAR,     /* "hh" */
    LENMOD_SHORT,    /* "h"  */
    LENMOD_LONG,     /* "l"  */
    LENMOD_LONGLONG, /* "ll" */
    LENMOD_INTMAX,   /* "j"  */
    LENMOD_SIZET,    /* "z"  */
    LENMOD_PTRDIFF,  /* "t"  */
    LENMOD_LONGDBL   /* "L"  */
  } lenmod_t;

/* All the conversion specifiers.  */
typedef enum
  {
    CONSPEC_UNKNOWN = 0,
    CONSPEC_DECIMAL,
    CONSPEC_OCTAL,
    CONSPEC_UNSIGNED,
    CONSPEC_HEX,
    CONSPEC_HEX_UP,
    CONSPEC_FLOAT,
    CONSPEC_FLOAT_UP,
    CONSPEC_EXP,
    CONSPEC_EXP_UP,
    CONSPEC_F_OR_G,
    CONSPEC_F_OR_G_UP,
    CONSPEC_HEX_EXP,
    CONSPEC_HEX_EXP_UP,
    CONSPEC_CHAR,
    CONSPEC_STRING,
    CONSPEC_POINTER,
    CONSPEC_STRERROR,
    CONSPEC_BYTES_SO_FAR
  } conspec_t;


/* Constants describing all the suppoorted types.  Note that we list
   all the types we know about even if certain types are not available
   on this system. */
typedef enum
  {
    VALTYPE_UNSUPPORTED = 0,  /* Artificial type for error detection.  */
    VALTYPE_CHAR,
    VALTYPE_SCHAR,
    VALTYPE_UCHAR,
    VALTYPE_SHORT,
    VALTYPE_USHORT,
    VALTYPE_INT,
    VALTYPE_UINT,
    VALTYPE_LONG,
    VALTYPE_ULONG,
    VALTYPE_LONGLONG,
    VALTYPE_ULONGLONG,
    VALTYPE_DOUBLE,
    VALTYPE_LONGDOUBLE,
    VALTYPE_STRING,
    VALTYPE_INTMAX,
    VALTYPE_UINTMAX,
    VALTYPE_SIZE,
    VALTYPE_PTRDIFF,
    VALTYPE_POINTER,
    VALTYPE_CHAR_PTR,
    VALTYPE_SCHAR_PTR,
    VALTYPE_SHORT_PTR,
    VALTYPE_INT_PTR,
    VALTYPE_LONG_PTR,
    VALTYPE_LONGLONG_PTR,
    VALTYPE_INTMAX_PTR,
    VALTYPE_SIZE_PTR,
    VALTYPE_PTRDIFF_PTR
  } valtype_t;


/* A union used to store the actual values. */
typedef union
{
  char a_char;
  signed char a_schar;
  unsigned char a_uchar;
  short a_short;
  unsigned short a_ushort;
  int a_int;
  unsigned int a_uint;
  long int a_long;
  unsigned long int a_ulong;
#ifdef HAVE_LONG_LONG_INT
  long long int a_longlong;
  unsigned long long int a_ulonglong;
#endif
  double a_double;
#ifdef HAVE_LONG_DOUBLE
  long double a_longdouble;
#endif
  const char *a_string;
#ifdef HAVE_INTMAX_T
  intmax_t a_intmax;
#endif
#ifdef HAVE_UINTMAX_T
  intmax_t a_uintmax;
#endif
  size_t a_size;
#ifdef HAVE_PTRDIFF_T
  ptrdiff_t a_ptrdiff;
#endif
  void *a_void_ptr;
  char *a_char_ptr;
  signed char *a_schar_ptr;
  short *a_short_ptr;
  int  *a_int_ptr;
  long *a_long_ptr;
#ifdef HAVE_LONG_LONG_INT
  long long int *a_longlong_ptr;
#endif
#ifdef HAVE_INTMAX_T
  intmax_t *a_intmax_ptr;
#endif
  size_t *a_size_ptr;
#ifdef HAVE_PTRDIFF_T
  ptrdiff_t *a_ptrdiff_ptr;
#endif
} value_t;

/* An object used to keep track of a format option and arguments. */
struct argspec_s
{
  size_t length;       /* The length of these args including the percent.  */
  unsigned int flags;  /* The conversion flags (bits defined by FLAG_foo).  */
  int width;           /* The field width.  */
  int precision;       /* The precision.  */
  lenmod_t lenmod;     /* The length modifier.  */
  conspec_t conspec;   /* The conversion specifier.  */
  int arg_pos;         /* The position of the argument.  This one may
                          be -1 to indicate that no value is expected
                          (e.g. for "%m").  */
  int width_pos;       /* The position of the argument for a field
                          width star's value. 0 for not used.  */
  int precision_pos;   /* The position of the argument for the a
                          precision star's value.  0 for not used. */
  valtype_t vt;        /* The type of the corresponding argument.  */
};
typedef struct argspec_s *argspec_t;

/* An object to build up a table of values and their types.  */
struct valueitem_s
{
  valtype_t vt;  /* The type of the value.  */
  value_t value; /* The value.  */
};
typedef struct valueitem_s *valueitem_t;


/* Not all systems have a C-90 compliant realloc.  To cope with this
   we use this simple wrapper. */
#ifndef _ESTREAM_PRINTF_REALLOC
static void *
fixed_realloc (void *a, size_t n)
{
  if (!a)
    return malloc (n);

  if (!n)
    {
      free (a);
      return NULL;
    }

  return realloc (a, n);
}
#endif /*!_ESTREAM_PRINTF_REALLOC*/


#ifdef DEBUG
static void
dump_argspecs (argspec_t arg, size_t argcount)
{
  int idx;

  for (idx=0; argcount; argcount--, arg++, idx++)
    fprintf (stderr,
             "%2d: len=%u flags=%u width=%d prec=%d mod=%d "
             "con=%d vt=%d pos=%d-%d-%d\n",
             idx,
             (unsigned int)arg->length,
             arg->flags,
             arg->width,
             arg->precision,
             arg->lenmod,
             arg->conspec,
             arg->vt,
             arg->arg_pos,
             arg->width_pos,
             arg->precision_pos);
}
#endif /*DEBUG*/


/* Set the vt field for ARG.  */
static void
compute_type (argspec_t arg)
{
  switch (arg->conspec)
    {
    case CONSPEC_UNKNOWN:
      arg->vt = VALTYPE_UNSUPPORTED;
      break;

    case CONSPEC_DECIMAL:
      switch (arg->lenmod)
        {
        case LENMOD_CHAR: arg->vt = VALTYPE_SCHAR; break;
        case LENMOD_SHORT: arg->vt = VALTYPE_SHORT; break;
        case LENMOD_LONG: arg->vt = VALTYPE_LONG; break;
        case LENMOD_LONGLONG: arg->vt = VALTYPE_LONGLONG; break;
        case LENMOD_INTMAX: arg->vt = VALTYPE_INTMAX; break;
        case LENMOD_SIZET: arg->vt = VALTYPE_SIZE; break;
        case LENMOD_PTRDIFF: arg->vt = VALTYPE_PTRDIFF; break;
        default: arg->vt = VALTYPE_INT; break;
        }
      break;

    case CONSPEC_OCTAL:
    case CONSPEC_UNSIGNED:
    case CONSPEC_HEX:
    case CONSPEC_HEX_UP:
      switch (arg->lenmod)
        {
        case LENMOD_CHAR: arg->vt = VALTYPE_UCHAR; break;
        case LENMOD_SHORT: arg->vt = VALTYPE_USHORT; break;
        case LENMOD_LONG: arg->vt = VALTYPE_ULONG; break;
        case LENMOD_LONGLONG: arg->vt = VALTYPE_ULONGLONG; break;
        case LENMOD_INTMAX: arg->vt = VALTYPE_UINTMAX; break;
        case LENMOD_SIZET: arg->vt = VALTYPE_SIZE; break;
        case LENMOD_PTRDIFF: arg->vt = VALTYPE_PTRDIFF; break;
        default: arg->vt = VALTYPE_UINT; break;
        }
      break;

    case CONSPEC_FLOAT:
    case CONSPEC_FLOAT_UP:
    case CONSPEC_EXP:
    case CONSPEC_EXP_UP:
    case CONSPEC_F_OR_G:
    case CONSPEC_F_OR_G_UP:
    case CONSPEC_HEX_EXP:
    case CONSPEC_HEX_EXP_UP:
      switch (arg->lenmod)
        {
        case LENMOD_LONGDBL: arg->vt = VALTYPE_LONGDOUBLE; break;
        case LENMOD_LONG: arg->vt = VALTYPE_DOUBLE; break;
        default: arg->vt = VALTYPE_DOUBLE; break;
        }
      break;

    case CONSPEC_CHAR:
      arg->vt = VALTYPE_INT;
      break;

    case CONSPEC_STRING:
      arg->vt = VALTYPE_STRING;
      break;

    case CONSPEC_POINTER:
      arg->vt = VALTYPE_POINTER;
      break;

    case CONSPEC_STRERROR:
      arg->vt = VALTYPE_STRING;
      break;

    case CONSPEC_BYTES_SO_FAR:
      switch (arg->lenmod)
        {
        case LENMOD_CHAR: arg->vt = VALTYPE_SCHAR_PTR; break;
        case LENMOD_SHORT: arg->vt = VALTYPE_SHORT_PTR; break;
        case LENMOD_LONG: arg->vt = VALTYPE_LONG_PTR; break;
        case LENMOD_LONGLONG: arg->vt = VALTYPE_LONGLONG_PTR; break;
        case LENMOD_INTMAX: arg->vt = VALTYPE_INTMAX_PTR; break;
        case LENMOD_SIZET: arg->vt = VALTYPE_SIZE_PTR; break;
        case LENMOD_PTRDIFF: arg->vt = VALTYPE_PTRDIFF_PTR; break;
        default: arg->vt = VALTYPE_INT_PTR; break;
        }
      break;

    }
}



/* Parse the FORMAT string and populate the specification array stored
   at the address ARGSPECS_ADDR.  The caller has provided enough space
   to store up to MAX_ARGSPECS in that buffer.  The function may
   however ignore the provided buffer and malloc a larger one.  On
   success the address of that larger buffer will be stored at
   ARGSPECS_ADDR.  The actual number of specifications will be
   returned at R_ARGSPECS_COUNT. */
static int
parse_format (const char *format,
              argspec_t *argspecs_addr, size_t max_argspecs,
              size_t *r_argspecs_count)
{
  const char *s;
  argspec_t argspecs = *argspecs_addr;
  argspec_t arg;
  size_t argcount = 0;

  if (!format)
    goto leave_einval;

  for (; *format; format++)
    {
      unsigned int flags;
      int width, precision;
      lenmod_t lenmod;
      conspec_t conspec;
      int arg_pos, width_pos, precision_pos;

      if (*format != '%')
        continue;
      s = ++format;
      if (!*s)
        goto leave_einval;
      if (*s == '%')
        continue; /* Just a quoted percent.  */

      /* First check whether there is a positional argument.  */
      arg_pos = 0; /* No positional argument given.  */
      if (*s >= '1' && *s <= '9')
        {
          const char *save_s = s;

          arg_pos = (*s++ - '0');
          for (; *s >= '0' && *s <= '9'; s++)
            arg_pos = 10*arg_pos + (*s - '0');
          if (arg_pos < 0)
            goto leave_einval; /* Overflow during conversion.  */
          if (*s == '$')
            s++;
          else
            {
              arg_pos = 0;
              s = save_s;
            }
        }

      /* Parse the flags.  */
      flags = 0;
      for ( ; *s; s++)
        {
          switch (*s)
            {
            case '\'': flags |= FLAG_GROUPING; break;
            case '-': flags |= FLAG_LEFT_JUST; break;
            case '+': flags |= FLAG_PLUS_SIGN; break;
            case ' ': flags |= FLAG_SPACE_PLUS; break;
            case '#': flags |= FLAG_ALT_CONV; break;
            case '0': flags |= FLAG_ZERO_PAD; break;
            default:
              goto flags_parsed;
            }
        }
    flags_parsed:

      /* Parse the field width.  */
      width_pos = 0;
      if (*s == '*')
        {
          width = STAR_FIELD_VALUE;
          s++;
          /* If we have a positional argument, another one might also
             be used to give the position of the star's value. */
          if (arg_pos && *s >= '1' && *s <= '9')
            {
              width_pos = (*s++ - '0');
              for (; *s >= '0' && *s <= '9'; s++)
                width_pos = 10*width_pos + (*s - '0');
              if (width_pos < 1)
                goto leave_einval; /* Overflow during conversion.  */
              if (*s != '$')
                goto leave_einval; /* Not followed by $.  */
              s++;
            }
        }
      else if ( *s >= '0' && *s <= '9')
        {
          width = (*s++ - '0');
          for (; *s >= '0' && *s <= '9'; s++)
            {
              if (!width && *s == '0')
                goto leave_einval; /* Leading zeroes are not allowed.
                                      Fixme: check what other
                                      implementations do. */
              width = 10*width + (*s - '0');
            }
          if (width < 0)
            goto leave_einval; /* Overflow during conversion.  */
        }
      else
        width = NO_FIELD_VALUE;

      /* Parse the precision.  */
      precision_pos = 0;
      precision = NO_FIELD_VALUE;
      if (*s == '.')
        {
          int ignore_value = (s[1] == '-');

          s++;
          if (*s == '*')
            {
              precision = STAR_FIELD_VALUE;
              s++;
              /* If we have a positional argument, another one might also
                 be used to give the position of the star's value. */
              if (arg_pos && *s >= '1' && *s <= '9')
                {
                  precision_pos = (*s++ - '0');
                  for (; *s >= '0' && *s <= '9'; s++)
                    precision_pos = 10*precision_pos + (*s - '0');
                  if (precision_pos < 1)
                    goto leave_einval; /* Overflow during conversion.  */
                  if (*s != '$')
                    goto leave_einval; /* Not followed by $.  */
                  s++;
                }
            }
          else if ( *s >= '0' && *s <= '9')
            {
              precision = (*s++ - '0');
              for (; *s >= '0' && *s <= '9'; s++)
                {
                  if (!precision && *s == '0')
                    goto leave_einval; /* Leading zeroes are not allowed.
                                          Fixme: check what other
                                          implementations do. */
                  precision = 10*precision + (*s - '0');
                }
              if (precision < 0)
                goto leave_einval; /* Overflow during conversion.  */
            }
          else
            precision = 0;
          if (ignore_value)
            precision = NO_FIELD_VALUE;
        }

      /* Parse the length modifiers.  */
      switch (*s)
        {
        case 'h':
          if (s[1] == 'h')
            {
              lenmod = LENMOD_CHAR;
              s++;
            }
          else
            lenmod = LENMOD_SHORT;
          s++;
          break;
        case 'l':
          if (s[1] == 'l')
            {
              lenmod = LENMOD_LONGLONG;
              s++;
            }
          else
            lenmod = LENMOD_LONG;
          s++;
          break;
        case 'j': lenmod = LENMOD_INTMAX; s++; break;
        case 'z': lenmod = LENMOD_SIZET; s++; break;
        case 't': lenmod = LENMOD_PTRDIFF; s++; break;
        case 'L': lenmod = LENMOD_LONGDBL; s++; break;
        default:  lenmod = LENMOD_NONE; break;
        }

      /* Parse the conversion specifier.  */
      switch (*s)
        {
        case 'd':
        case 'i': conspec = CONSPEC_DECIMAL; break;
        case 'o': conspec = CONSPEC_OCTAL; break;
        case 'u': conspec = CONSPEC_UNSIGNED; break;
        case 'x': conspec = CONSPEC_HEX; break;
        case 'X': conspec = CONSPEC_HEX_UP; break;
        case 'f': conspec = CONSPEC_FLOAT; break;
        case 'F': conspec = CONSPEC_FLOAT_UP; break;
        case 'e': conspec = CONSPEC_EXP; break;
        case 'E': conspec = CONSPEC_EXP_UP; break;
        case 'g': conspec = CONSPEC_F_OR_G; break;
        case 'G': conspec = CONSPEC_F_OR_G_UP; break;
        case 'a': conspec = CONSPEC_HEX_EXP; break;
        case 'A': conspec = CONSPEC_HEX_EXP_UP; break;
        case 'c': conspec = CONSPEC_CHAR; break;
        case 's': conspec = CONSPEC_STRING; break;
        case 'p': conspec = CONSPEC_POINTER; break;
        case 'n': conspec = CONSPEC_BYTES_SO_FAR; break;
        case 'C': conspec = CONSPEC_CHAR; lenmod = LENMOD_LONG; break;
        case 'S': conspec = CONSPEC_STRING; lenmod = LENMOD_LONG; break;
        case 'm': conspec = CONSPEC_STRERROR; arg_pos = -1; break;
        default: conspec = CONSPEC_UNKNOWN;
        }

      /* Save the args. */
      if (argcount >= max_argspecs)
        {
          /* We either need to allocate a new array instead of the
             caller provided one or realloc the array.  Instead of
             using realloc we allocate a new one and release the
             original one then. */
          size_t n, newmax;
          argspec_t newarg;

          newmax = max_argspecs + ARGSPECS_BUMP_VALUE;
          if (newmax <= max_argspecs)
            goto leave_einval;  /* Too many arguments. */
          newarg = calloc (newmax, sizeof *newarg);
          if (!newarg)
            goto leave;
          for (n=0; n < argcount; n++)
            newarg[n] = argspecs[n];
          if (argspecs != *argspecs_addr)
            free (argspecs);
          argspecs = newarg;
          max_argspecs = newmax;
        }

      arg = argspecs + argcount;
      arg->length = s - format + 2;
      arg->flags = flags;
      arg->width = width;
      arg->precision = precision;
      arg->lenmod = lenmod;
      arg->conspec = conspec;
      arg->arg_pos = arg_pos;
      arg->width_pos = width_pos;
      arg->precision_pos = precision_pos;
      compute_type (arg);
      argcount++;
      format = s;
    }

  *argspecs_addr = argspecs;
  *r_argspecs_count = argcount;
  return 0; /* Success.  */

 leave_einval:
  _set_errno (EINVAL);
 leave:
  if (argspecs != *argspecs_addr)
    free (argspecs);
  *argspecs_addr = NULL;
  return -1;
}


/* This function reads all the values as specified by VALUETABLE into
   VALUETABLE.  The values are expected in VAARGS.  The function
   returns -1 if a specified type is not supported. */
static int
read_values (valueitem_t valuetable, size_t valuetable_len, va_list vaargs)
{
  int validx;

  for (validx=0; validx < valuetable_len; validx++)
    {
      value_t *value = &valuetable[validx].value;
      valtype_t vt = valuetable[validx].vt;

      switch (vt)
        {
        case VALTYPE_CHAR: value->a_char = va_arg (vaargs, int); break;
        case VALTYPE_CHAR_PTR:
          value->a_char_ptr = va_arg (vaargs, char *);
          break;
        case VALTYPE_SCHAR: value->a_schar = va_arg (vaargs, int); break;
        case VALTYPE_SCHAR_PTR:
          value->a_schar_ptr = va_arg (vaargs, signed char *);
          break;
        case VALTYPE_UCHAR: value->a_uchar = va_arg (vaargs, int); break;
        case VALTYPE_SHORT: value->a_short = va_arg (vaargs, int); break;
        case VALTYPE_USHORT: value->a_ushort = va_arg (vaargs, int); break;
        case VALTYPE_SHORT_PTR:
          value->a_short_ptr = va_arg (vaargs, short *);
          break;
        case VALTYPE_INT:
          value->a_int = va_arg (vaargs, int);
          break;
        case VALTYPE_INT_PTR:
          value->a_int_ptr = va_arg (vaargs, int *);
          break;
        case VALTYPE_UINT:
          value->a_uint = va_arg (vaargs, unsigned int);
          break;
        case VALTYPE_LONG:
          value->a_long = va_arg (vaargs, long);
          break;
        case VALTYPE_ULONG:
          value->a_ulong = va_arg (vaargs, unsigned long);
          break;
        case VALTYPE_LONG_PTR:
          value->a_long_ptr = va_arg (vaargs, long *);
          break;
#ifdef HAVE_LONG_LONG_INT
        case VALTYPE_LONGLONG:
          value->a_longlong = va_arg (vaargs, long long int);
          break;
        case VALTYPE_ULONGLONG:
          value->a_ulonglong = va_arg (vaargs, unsigned long long int);
          break;
        case VALTYPE_LONGLONG_PTR:
          value->a_longlong_ptr = va_arg (vaargs, long long *);
          break;
#endif
        case VALTYPE_DOUBLE:
          value->a_double = va_arg (vaargs, double);
          break;
#ifdef HAVE_LONG_DOUBLE
        case VALTYPE_LONGDOUBLE:
          value->a_longdouble = va_arg (vaargs, long double);
          break;
#endif
        case VALTYPE_STRING:
          value->a_string = va_arg (vaargs, const char *);
          break;
        case VALTYPE_POINTER:
          value->a_void_ptr = va_arg (vaargs, void *);
          break;
#ifdef HAVE_INTMAX_T
        case VALTYPE_INTMAX:
          value->a_intmax = va_arg (vaargs, intmax_t);
          break;
        case VALTYPE_INTMAX_PTR:
          value->a_intmax_ptr = va_arg (vaargs, intmax_t *);
          break;
#endif
#ifdef HAVE_UINTMAX_T
        case VALTYPE_UINTMAX:
          value->a_uintmax = va_arg (vaargs, uintmax_t);
          break;
#endif
        case VALTYPE_SIZE:
          value->a_size = va_arg (vaargs, size_t);
          break;
        case VALTYPE_SIZE_PTR:
          value->a_size_ptr = va_arg (vaargs, size_t *);
          break;
#ifdef HAVE_PTRDIFF_T
        case VALTYPE_PTRDIFF:
          value->a_ptrdiff = va_arg (vaargs, ptrdiff_t);
          break;
        case VALTYPE_PTRDIFF_PTR:
          value->a_ptrdiff_ptr = va_arg (vaargs, ptrdiff_t *);
          break;
#endif
        default: /* Unsupported type.  */
          return -1;
        }
    }
  return 0;
}



/* Output COUNT padding characters PADCHAR and update NBYTES by the
   number of bytes actually written.  */
static int
pad_out (estream_printf_out_t outfnc, void *outfncarg,
         int padchar, int count, size_t *nbytes)
{
  char buf[32];
  size_t n;
  int rc;

  while (count > 0)
    {
      n = (count <= sizeof buf)? count : sizeof buf;
      memset (buf, padchar, n);
      rc = outfnc (outfncarg, buf, n);
      if (rc)
        return rc;
      *nbytes += n;
      count -= n;
    }

  return 0;
}


/* "d,i,o,u,x,X" formatting.  OUTFNC and OUTFNCARG describes the
   output routine, ARG gives the argument description and VALUE the
   actual value (its type is available through arg->vt).  */
static int
pr_integer (estream_printf_out_t outfnc, void *outfncarg,
            argspec_t arg, value_t value, size_t *nbytes)
{
  int rc;
#ifdef HAVE_LONG_LONG_INT
  unsigned long long aulong;
#else
  unsigned long aulong;
#endif
  char numbuf[100];
  char *p, *pend;
  size_t n;
  char signchar = 0;
  int n_prec;  /* Number of extra precision digits required.  */
  int n_extra; /* Extra number of prefix or sign characters.  */

  if (arg->conspec == CONSPEC_DECIMAL)
    {
#ifdef HAVE_LONG_LONG_INT
      long long along;
#else
      long along;
#endif

      switch (arg->vt)
        {
        case VALTYPE_SHORT: along = value.a_short; break;
        case VALTYPE_INT: along = value.a_int; break;
        case VALTYPE_LONG: along = value.a_long; break;
#ifdef HAVE_LONG_LONG_INT
        case VALTYPE_LONGLONG: along = value.a_longlong; break;
        case VALTYPE_SIZE: along = value.a_size; break;
# ifdef HAVE_INTMAX_T
        case VALTYPE_INTMAX: along = value.a_intmax; break;
# endif
# ifdef HAVE_PTRDIFF_T
        case VALTYPE_PTRDIFF: along = value.a_ptrdiff; break;
# endif
#endif /*HAVE_LONG_LONG_INT*/
        default:
          return -1;
        }
      if (along < 0)
        {
          aulong = -along;
          signchar = '-';
        }
      else
        aulong = along;
    }
  else
    {
      switch (arg->vt)
        {
        case VALTYPE_USHORT: aulong = value.a_ushort; break;
        case VALTYPE_UINT: aulong = value.a_uint; break;
        case VALTYPE_ULONG: aulong = value.a_ulong; break;
#ifdef HAVE_LONG_LONG_INT
        case VALTYPE_ULONGLONG: aulong = value.a_ulonglong; break;
        case VALTYPE_SIZE: aulong = value.a_size; break;
# ifdef HAVE_UINTMAX_T
        case VALTYPE_UINTMAX: aulong = value.a_uintmax; break;
# endif
# ifdef HAVE_PTRDIFF_T
        case VALTYPE_PTRDIFF: aulong = value.a_ptrdiff; break;
# endif
#endif /*HAVE_LONG_LONG_INT*/
        default:
          return -1;
        }
    }

  if (signchar == '-')
    ;
  else if ((arg->flags & FLAG_PLUS_SIGN))
    signchar = '+';
  else if ((arg->flags & FLAG_SPACE_PLUS))
    signchar = ' ';

  n_extra = !!signchar;

  /* We build the string up backwards.  */
  p = pend = numbuf + DIM(numbuf);
  if ((!aulong && !arg->precision))
    ;
  else if (arg->conspec == CONSPEC_DECIMAL
           || arg->conspec == CONSPEC_UNSIGNED)
    {
      int grouping = -1;
      const char * grouping_string =
#ifdef HAVE_LANGINFO_THOUSEP
        nl_langinfo(THOUSEP);
#else
        "'";
#endif

      do
        {
          if ((arg->flags & FLAG_GROUPING)
              && (++grouping == 3) && *grouping_string)
            {
              *--p = *grouping_string;
              grouping = 0;
            }
          *--p = '0' + (aulong % 10);
          aulong /= 10;
        }
      while (aulong);
    }
  else if (arg->conspec == CONSPEC_OCTAL)
    {
      do
        {
          *--p = '0' + (aulong % 8);
          aulong /= 8;
        }
      while (aulong);
      if ((arg->flags & FLAG_ALT_CONV) && *p != '0')
        *--p = '0';
    }
  else /* HEX or HEXUP */
    {
      const char *digits = ((arg->conspec == CONSPEC_HEX)
                            ? "0123456789abcdef" : "0123456789ABCDEF");
      do
        {
          *--p = digits[(aulong % 16)];
          aulong /= 16;
        }
      while (aulong);
      if ((arg->flags & FLAG_ALT_CONV))
        n_extra += 2;
    }

  n = pend - p;

  if ((arg->flags & FLAG_ZERO_PAD)
      && arg->precision == NO_FIELD_VALUE && !(arg->flags & FLAG_LEFT_JUST)
      && n && arg->width - n_extra > n )
    n_prec = arg->width - n_extra - n;
  else if (arg->precision > 0 && arg->precision > n)
    n_prec = arg->precision - n;
  else
    n_prec = 0;

  if (!(arg->flags & FLAG_LEFT_JUST)
      && arg->width >= 0 && arg->width - n_extra > n
      && arg->width - n_extra - n >= n_prec )
    {
      rc = pad_out (outfnc, outfncarg, ' ',
                    arg->width - n_extra - n - n_prec, nbytes);
      if (rc)
        return rc;
    }

  if (signchar)
    {
      rc = outfnc (outfncarg, &signchar, 1);
      if (rc)
        return rc;
      *nbytes += 1;
    }

  if ((arg->flags & FLAG_ALT_CONV)
      && (arg->conspec == CONSPEC_HEX || arg->conspec == CONSPEC_HEX_UP))
    {
      rc = outfnc (outfncarg, arg->conspec == CONSPEC_HEX? "0x": "0X", 2);
      if (rc)
        return rc;
      *nbytes += 2;
    }

  if (n_prec)
    {
      rc = pad_out (outfnc, outfncarg, '0', n_prec, nbytes);
      if (rc)
        return rc;
    }

  rc = outfnc (outfncarg, p, pend - p);
  if (rc)
    return rc;
  *nbytes += pend - p;

  if ((arg->flags & FLAG_LEFT_JUST)
      && arg->width >= 0 && arg->width - n_extra - n_prec > n)
    {
      rc = pad_out (outfnc, outfncarg, ' ',
                    arg->width - n_extra - n_prec - n, nbytes);
      if (rc)
        return rc;
    }

  return 0;
}


/* "e,E,f,F,g,G,a,A" formatting.  OUTFNC and OUTFNCARG describes the
   output routine, ARG gives the argument description and VALUE the
   actual value (its type is available through arg->vt).  For
   portability reasons sprintf is used for the actual formatting.
   This is useful because sprint is the only standard function to
   convert a floating number into its ascii representation.  To avoid
   using malloc we just pass the precision to sprintf and do the final
   formatting with our own code.  */
static int
pr_float (estream_printf_out_t outfnc, void *outfncarg,
          argspec_t arg, value_t value, size_t *nbytes)
{
  int rc;
#ifdef HAVE_LONG_DOUBLE
  long double adblfloat = 0; /* Just to please gcc.  */
  int use_dbl = 0;
#endif
  double afloat;
  char numbuf[350];
  char formatstr[20];
  char *p, *pend;
  size_t n;
  char signchar = 0;
  int n_extra;  /* Extra number of prefix or sign characters.  */

  switch (arg->vt)
    {
    case VALTYPE_DOUBLE: afloat = value.a_double; break;
#ifdef HAVE_LONG_DOUBLE
    case VALTYPE_LONGDOUBLE:
      afloat = 0;  /* Just to please gcc.  */
      adblfloat = value.a_longdouble;
      use_dbl=1; break;
#endif
    default:
      return -1;
    }

  /* We build the string using sprint.  */
  p = formatstr + sizeof formatstr;
  *--p = 0;
  switch (arg->conspec)
    {
    case CONSPEC_FLOAT:      *--p = 'f'; break;
    case CONSPEC_FLOAT_UP:   *--p = 'F'; break;
    case CONSPEC_EXP:        *--p = 'e'; break;
    case CONSPEC_EXP_UP:     *--p = 'E'; break;
    case CONSPEC_F_OR_G:     *--p = 'g'; break;
    case CONSPEC_F_OR_G_UP:  *--p = 'G'; break;
    case CONSPEC_HEX_EXP:    *--p = 'a'; break;
    case CONSPEC_HEX_EXP_UP: *--p = 'A'; break;
    default:
      return -1; /* Actually a bug.  */
    }
#ifdef HAVE_LONG_DOUBLE
  if (use_dbl)
    *--p = 'L';
#endif
  if (arg->precision != NO_FIELD_VALUE)
    {
      /* Limit it to a meaningful value so that even a stupid sprintf
         won't overflow our buffer.  */
      n = arg->precision <= 100? arg->precision : 100;
      do
        {
          *--p = '0' + (n % 10);
          n /= 10;
        }
      while (n);
      *--p = '.';
    }
  if ((arg->flags & FLAG_ALT_CONV))
    *--p = '#';
  *--p = '%';
#ifdef HAVE_LONG_DOUBLE
  if (use_dbl)
    sprintf (numbuf, p, adblfloat);
  else
#endif /*HAVE_LONG_DOUBLE*/
    sprintf (numbuf, p, afloat);
  p = numbuf;
  n = strlen (numbuf);
  pend = p + n;

  if (*p =='-')
    {
      signchar = '-';
      p++;
      n--;
    }
  else if ((arg->flags & FLAG_PLUS_SIGN))
    signchar = '+';
  else if ((arg->flags & FLAG_SPACE_PLUS))
    signchar = ' ';

  n_extra = !!signchar;

  if (!(arg->flags & FLAG_LEFT_JUST)
      && arg->width >= 0 && arg->width - n_extra > n)
    {
      rc = pad_out (outfnc, outfncarg, ' ', arg->width - n_extra - n, nbytes);
      if (rc)
        return rc;
    }

  if (signchar)
    {
      rc = outfnc (outfncarg, &signchar, 1);
      if (rc)
        return rc;
      *nbytes += 1;
    }

  rc = outfnc (outfncarg, p, pend - p);
  if (rc)
    return rc;
  *nbytes += pend - p;

  if ((arg->flags & FLAG_LEFT_JUST)
      && arg->width >= 0 && arg->width - n_extra > n)
    {
      rc = pad_out (outfnc, outfncarg, ' ', arg->width - n_extra - n, nbytes);
      if (rc)
        return rc;
    }

  return 0;
}


/* "c" formatting.  */
static int
pr_char (estream_printf_out_t outfnc, void *outfncarg,
            argspec_t arg, value_t value, size_t *nbytes)
{
  int rc;
  char buf[1];

  if (arg->vt != VALTYPE_INT)
    return -1;
  buf[0] = (unsigned int)value.a_int;
  rc = outfnc (outfncarg, buf, 1);
  if(rc)
    return rc;
  *nbytes += 1;

  return 0;
}


/* "s" formatting.  */
static int
pr_string (estream_printf_out_t outfnc, void *outfncarg,
           argspec_t arg, value_t value, size_t *nbytes,
           gpgrt_string_filter_t sf, void *sfvalue, int string_no)
{
  int rc;
  size_t n;
  const char *string, *s;

  if (arg->vt != VALTYPE_STRING)
    return -1;
  if (sf)
    string = sf (value.a_string, string_no, sfvalue);
  else
    string = value.a_string;

  if (!string)
    string = "(null)";
  if (arg->precision >= 0)
    {
      /* Test for nul after N so that we can pass a non-nul terminated
         string.  */
      for (n=0,s=string; n < arg->precision && *s; s++)
        n++;
    }
  else
    n = strlen (string);

  if (!(arg->flags & FLAG_LEFT_JUST)
      && arg->width >= 0 && arg->width > n )
    {
      rc = pad_out (outfnc, outfncarg, ' ', arg->width - n, nbytes);
      if (rc)
        goto leave;
    }

  rc = outfnc (outfncarg, string, n);
  if (rc)
    goto leave;
  *nbytes += n;

  if ((arg->flags & FLAG_LEFT_JUST)
      && arg->width >= 0 && arg->width > n)
    {
      rc = pad_out (outfnc, outfncarg, ' ', arg->width - n, nbytes);
      if (rc)
        goto leave;
    }

  rc = 0;

 leave:
  if (sf) /* Tell the filter to release resources.  */
    sf (value.a_string, -1, sfvalue);

  return rc;
}


/* "p" formatting.  */
static int
pr_pointer (estream_printf_out_t outfnc, void *outfncarg,
            argspec_t arg, value_t value, size_t *nbytes)
{
  int rc;
#if defined(HAVE_LONG_LONG_INT) && (SIZEOF_UNSIGNED_LONG < SIZEOF_VOID_P)
  unsigned long long aulong;
#else
  unsigned long aulong;
#endif
  char numbuf[100];
  char *p, *pend;

  if (arg->vt != VALTYPE_POINTER)
    return -1;
  /* We assume that a pointer can be converted to an unsigned long.
     That is not correct for a 64 bit Windows, but then we assume that
     long long is supported and usable for storing a pointer.  */
#if defined(HAVE_LONG_LONG_INT) && (SIZEOF_UNSIGNED_LONG < SIZEOF_VOID_P)
  aulong = (unsigned long long)value.a_void_ptr;
#else
  aulong = (unsigned long)value.a_void_ptr;
#endif

  p = pend = numbuf + DIM(numbuf);
  do
    {
      *--p = "0123456789abcdefx"[(aulong % 16)];
      aulong /= 16;
    }
  while (aulong);
  while ((pend-p) < 2*sizeof (aulong))
    *--p = '0';
  *--p = 'x';
  *--p = '0';

  rc = outfnc (outfncarg, p, pend - p);
  if (rc)
    return rc;
  *nbytes += pend - p;

  return 0;
}

/* "n" pesudo format operation.  */
static int
pr_bytes_so_far (estream_printf_out_t outfnc, void *outfncarg,
                 argspec_t arg, value_t value, size_t *nbytes)
{
  (void)outfnc;
  (void)outfncarg;

  switch (arg->vt)
    {
    case VALTYPE_SCHAR_PTR:
      *value.a_schar_ptr = (signed char)(unsigned int)(*nbytes);
      break;
    case VALTYPE_SHORT_PTR:
      *value.a_short_ptr = (short)(unsigned int)(*nbytes);
      break;
    case VALTYPE_LONG_PTR:
      *value.a_long_ptr = (long)(*nbytes);
      break;
#ifdef HAVE_LONG_LONG_INT
    case VALTYPE_LONGLONG_PTR:
      *value.a_longlong_ptr = (long long)(*nbytes);
      break;
#endif
#ifdef HAVE_INTMAX_T
    case VALTYPE_INTMAX_PTR:
      *value.a_intmax_ptr = (intmax_t)(*nbytes);
      break;
#endif
    case VALTYPE_SIZE_PTR:
      *value.a_size_ptr = (*nbytes);
      break;
#ifdef HAVE_PTRDIFF_T
    case VALTYPE_PTRDIFF_PTR:
      *value.a_ptrdiff_ptr = (ptrdiff_t)(*nbytes);
      break;
#endif
    case VALTYPE_INT_PTR:
      *value.a_int_ptr = (int)(*nbytes);
      break;
    default:
      return -1; /* An unsupported type has been used.  */
    }

  return 0;
}



/* Run the actual formatting.  OUTFNC and OUTFNCARG are the output
 * functions.  FORMAT is format string ARGSPECS is the parsed format
 * string, ARGSPECS_LEN the number of items in ARGSPECS.
 * STRING_FILTER is an optional function to filter string (%s) args;
 * it is called with the original string and the count of already
 * processed %s arguments.  Its return value will be used instead of
 * the original string.  VALUETABLE holds the values and may be
 * directly addressed using the position arguments given by ARGSPECS.
 * MYERRNO is used for the "%m" conversion. NBYTES well be updated to
 * reflect the number of bytes send to the output function. */
static int
do_format (estream_printf_out_t outfnc, void *outfncarg,
           gpgrt_string_filter_t sf, void *sfvalue,
           const char *format, argspec_t argspecs, size_t argspecs_len,
           valueitem_t valuetable, int myerrno, size_t *nbytes)
{
  int rc = 0;
  const char *s;
  argspec_t arg = argspecs;
  int argidx = 0; /* Only used for assertion.  */
  size_t n;
  value_t value;
  int string_no = 0;  /* Number of processed "%s" args.  */

  s = format;
  while ( *s )
    {
      if (*s != '%')
        {
          s++;
          continue;
        }
      if (s != format)
        {
          rc = outfnc (outfncarg, format, (n=s-format));
          if (rc)
            return rc;
          *nbytes += n;
        }
      if (s[1] == '%')
        {
          /* Note that this code ignores one trailing percent escape -
             this is however okay as the args parser must have
             detected this already.  */
          rc = outfnc (outfncarg, s, 1);
          if (rc)
            return rc;
          *nbytes += 1;
          s += 2;
          format = s;
          continue;
        }

      /* Save the next start.  */
      s += arg->length;
      format = s;

      gpgrt_assert (argidx < argspecs_len);
      argidx++;

      /* Apply indirect field width and precision values.  */
      if (arg->width == STAR_FIELD_VALUE)
        {
          gpgrt_assert (valuetable[arg->width_pos-1].vt == VALTYPE_INT);
          arg->width = valuetable[arg->width_pos-1].value.a_int;
          if (arg->width < 0)
            {
              arg->width = -arg->width;
              arg->flags |= FLAG_LEFT_JUST;
            }
        }
      if (arg->precision == STAR_FIELD_VALUE)
        {
          gpgrt_assert (valuetable[arg->precision_pos-1].vt == VALTYPE_INT);
          arg->precision = valuetable[arg->precision_pos-1].value.a_int;
          if (arg->precision < 0)
            arg->precision = NO_FIELD_VALUE;
        }

      if (arg->arg_pos == -1 && arg->conspec == CONSPEC_STRERROR)
        value.a_string = strerror (myerrno);
      else
        {
          gpgrt_assert (arg->vt == valuetable[arg->arg_pos-1].vt);
          value = valuetable[arg->arg_pos-1].value;
        }

      switch (arg->conspec)
        {
        case CONSPEC_UNKNOWN: gpgrt_assert (!"bug"); break;

        case CONSPEC_DECIMAL:
        case CONSPEC_UNSIGNED:
        case CONSPEC_OCTAL:
        case CONSPEC_HEX:
        case CONSPEC_HEX_UP:
          rc = pr_integer (outfnc, outfncarg, arg, value, nbytes);
          break;
        case CONSPEC_FLOAT:
        case CONSPEC_FLOAT_UP:
        case CONSPEC_EXP:
        case CONSPEC_EXP_UP:
        case CONSPEC_F_OR_G:
        case CONSPEC_F_OR_G_UP:
        case CONSPEC_HEX_EXP:
        case CONSPEC_HEX_EXP_UP:
          rc = pr_float (outfnc, outfncarg, arg, value, nbytes);
          break;
        case CONSPEC_CHAR:
          rc = pr_char (outfnc, outfncarg, arg, value, nbytes);
          break;
        case CONSPEC_STRING:
          rc = pr_string (outfnc, outfncarg, arg, value, nbytes,
                          sf, sfvalue, string_no++);
          break;
        case CONSPEC_STRERROR:
          rc = pr_string (outfnc, outfncarg, arg, value, nbytes,
                          NULL, NULL, 0);
          break;
        case CONSPEC_POINTER:
          rc = pr_pointer (outfnc, outfncarg, arg, value, nbytes);
          break;
        case CONSPEC_BYTES_SO_FAR:
          rc = pr_bytes_so_far (outfnc, outfncarg, arg, value, nbytes);
          break;
        }
      if (rc)
        return rc;
      arg++;
    }

  /* Print out any trailing stuff. */
  n = s - format;
  rc = n? outfnc (outfncarg, format, n) : 0;
  if (!rc)
    *nbytes += n;

  return rc;
}




/* The versatile printf formatting routine.  It expects a callback
   function OUTFNC and an opaque argument OUTFNCARG used for actual
   output of the formatted stuff.  FORMAT is the format specification
   and VAARGS a variable argumemt list matching the arguments of
   FORMAT.  */
int
_gpgrt_estream_format (estream_printf_out_t outfnc,
                       void *outfncarg,
                       gpgrt_string_filter_t sf, void *sfvalue,
                       const char *format, va_list vaargs)
{
  /* Buffer to hold the argspecs and a pointer to it.*/
  struct argspec_s argspecs_buffer[DEFAULT_MAX_ARGSPECS];
  argspec_t argspecs = argspecs_buffer;
  size_t argspecs_len;  /* Number of specifications in ARGSPECS.  */

  /* Buffer to hold the description for the values.  */
  struct valueitem_s valuetable_buffer[DEFAULT_MAX_VALUES];
  valueitem_t valuetable = valuetable_buffer;

  int rc;        /* Return code. */
  size_t argidx; /* Used to index the argspecs array.  */
  size_t validx; /* Used to index the valuetable.  */
  int max_pos;   /* Highest argument position.  */

  size_t nbytes = 0; /* Keep track of the number of bytes passed to
                        the output function.  */

  int myerrno = errno; /* Save the errno for use with "%m". */


  /* Parse the arguments to come up with descriptive list.  We can't
     do this on the fly because we need to support positional
     arguments. */
  rc = parse_format (format, &argspecs, DIM(argspecs_buffer), &argspecs_len);
  if (rc)
    goto leave;

  /* Check that all ARG_POS fields are set.  */
  for (argidx=0,max_pos=0; argidx < argspecs_len; argidx++)
    {
      if (argspecs[argidx].arg_pos != -1
          && argspecs[argidx].arg_pos > max_pos)
        max_pos = argspecs[argidx].arg_pos;
      if (argspecs[argidx].width_pos > max_pos)
        max_pos = argspecs[argidx].width_pos;
      if (argspecs[argidx].precision_pos > max_pos)
        max_pos = argspecs[argidx].precision_pos;
    }
  if (!max_pos)
    {
      /* Fill in all the positions.  */
      for (argidx=0; argidx < argspecs_len; argidx++)
        {
          if (argspecs[argidx].width == STAR_FIELD_VALUE)
            argspecs[argidx].width_pos = ++max_pos;
          if (argspecs[argidx].precision == STAR_FIELD_VALUE)
            argspecs[argidx].precision_pos = ++max_pos;
          if (argspecs[argidx].arg_pos != -1 )
            argspecs[argidx].arg_pos = ++max_pos;
        }
    }
  else
    {
      /* Check that they are all filled.   More test are done later.  */
      for (argidx=0; argidx < argspecs_len; argidx++)
        {
          if (!argspecs[argidx].arg_pos
              || (argspecs[argidx].width == STAR_FIELD_VALUE
                  && !argspecs[argidx].width_pos)
              || (argspecs[argidx].precision == STAR_FIELD_VALUE
                  && !argspecs[argidx].precision_pos))
            goto leave_einval;
        }
    }
  /* Check that there is no overflow in max_pos and that it has a
     reasonable length.  There may never be more elements than the
     number of characters in FORMAT.  */
  if (max_pos < 0 || max_pos >= strlen (format))
    goto leave_einval;

#ifdef DEBUG
    dump_argspecs (argspecs, argspecs_len);
#endif

  /* Allocate a table to hold the values.  If it is small enough we
     use a stack allocated buffer.  */
  if (max_pos > DIM(valuetable_buffer))
    {
      valuetable = calloc (max_pos, sizeof *valuetable);
      if (!valuetable)
        goto leave_error;
    }
  else
    {
      for (validx=0; validx < DIM(valuetable_buffer); validx++)
        {
          valuetable[validx].vt = VALTYPE_UNSUPPORTED;
          memset (&valuetable[validx].value, 0,
                  sizeof valuetable[validx].value);
        }
    }
  for (argidx=0; argidx < argspecs_len; argidx++)
    {
      if (argspecs[argidx].arg_pos != - 1)
        {
          validx = argspecs[argidx].arg_pos - 1;
          if (valuetable[validx].vt)
            goto leave_einval; /* Already defined. */
          valuetable[validx].vt = argspecs[argidx].vt;
        }
      if (argspecs[argidx].width == STAR_FIELD_VALUE)
        {
          validx = argspecs[argidx].width_pos - 1;
          if (valuetable[validx].vt)
            goto leave_einval; /* Already defined.  */
          valuetable[validx].vt = VALTYPE_INT;
        }
      if (argspecs[argidx].precision == STAR_FIELD_VALUE)
        {
          validx = argspecs[argidx].precision_pos - 1;
          if (valuetable[validx].vt)
            goto leave_einval; /* Already defined.  */
          valuetable[validx].vt = VALTYPE_INT;
        }
    }

  /* Read all the arguments.  This will error out for unsupported
     types and for not given positional arguments. */
  rc = read_values (valuetable, max_pos, vaargs);
  if (rc)
    goto leave_einval;

/*   for (validx=0; validx < max_pos; validx++) */
/*     fprintf (stderr, "%2d: vt=%d\n", validx, valuetable[validx].vt); */

  /* Everything has been collected, go ahead with the formatting.  */
  rc = do_format (outfnc, outfncarg, sf, sfvalue, format,
                  argspecs, argspecs_len, valuetable, myerrno, &nbytes);

  goto leave;

 leave_einval:
  _set_errno (EINVAL);
 leave_error:
  rc = -1;
 leave:
  if (valuetable != valuetable_buffer)
    free (valuetable);
  if (argspecs != argspecs_buffer)
    free (argspecs);
  return rc;
}




/* A simple output handler utilizing stdio.  */
static int
plain_stdio_out (void *outfncarg, const char *buf, size_t buflen)
{
  FILE *fp = (FILE*)outfncarg;

  if ( fwrite (buf, buflen, 1, fp) != 1 )
    return -1;
  return 0;
}


/* A replacement for printf.  */
int
_gpgrt_estream_printf (const char *format, ...)
{
  int rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = _gpgrt_estream_format (plain_stdio_out, stderr, NULL, NULL,
                              format, arg_ptr);
  va_end (arg_ptr);

  return rc;
}

/* A replacement for fprintf.  */
int
_gpgrt_estream_fprintf (FILE *fp, const char *format, ...)
{
  int rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = _gpgrt_estream_format (plain_stdio_out, fp, NULL, NULL,
                              format, arg_ptr);
  va_end (arg_ptr);

  return rc;
}

/* A replacement for vfprintf.  */
int
_gpgrt_estream_vfprintf (FILE *fp, const char *format, va_list arg_ptr)
{
  return _gpgrt_estream_format (plain_stdio_out, fp, NULL, NULL,
                                format, arg_ptr);
}



/* Communication object used between estream_snprintf and
   fixed_buffer_out.  */
struct fixed_buffer_parm_s
{
  size_t size;    /* Size of the buffer.  */
  size_t count;   /* Number of bytes requested for output.  */
  size_t used;    /* Used size of the buffer.  */
  char *buffer;   /* Provided buffer.  */
};

/* A simple malloced buffer output handler.  */
static int
fixed_buffer_out (void *outfncarg, const char *buf, size_t buflen)
{
  struct fixed_buffer_parm_s *parm = outfncarg;

  parm->count += buflen;

  if (!parm->buffer)
    ;
  else if (parm->used + buflen < parm->size)
    {
      /* Handle the common case that everything fits into the buffer
         separately.  */
      memcpy (parm->buffer + parm->used, buf, buflen);
      parm->used += buflen;
    }
  else
    {
      /* The slow version of above.  */
      for ( ;buflen && parm->used < parm->size; buflen--)
        parm->buffer[parm->used++] = *buf++;
    }

  return 0;
}


/* A replacement for vsnprintf. */
int
_gpgrt_estream_vsnprintf (char *buf, size_t bufsize,
                   const char *format, va_list arg_ptr)
{
  struct fixed_buffer_parm_s parm;
  int rc;

  parm.size = bufsize;
  parm.count = 0;
  parm.used = 0;
  parm.buffer = bufsize?buf:NULL;
  rc = _gpgrt_estream_format (fixed_buffer_out, &parm, NULL, NULL,
                              format, arg_ptr);
  if (!rc)
    rc = fixed_buffer_out (&parm, "", 1); /* Print terminating Nul.  */
  if (rc == -1)
    return -1;
  if (bufsize && buf && parm.size && parm.count >= parm.size)
    buf[parm.size-1] = 0;

  parm.count--; /* Do not count the trailing nul.  */
  return (int)parm.count; /* Return number of bytes which would have
                             been written.  */
}

/* A replacement for snprintf.  */
int
_gpgrt_estream_snprintf (char *buf, size_t bufsize, const char *format, ...)
{
  int rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = _gpgrt_estream_vsnprintf (buf, bufsize, format, arg_ptr);
  va_end (arg_ptr);

  return rc;
}



/* Communication object used between estream_asprintf and
   dynamic_buffer_out.  */
struct dynamic_buffer_parm_s
{
  int error_flag; /* Internal helper.  */
  size_t alloced; /* Allocated size of the buffer.  */
  size_t used;    /* Used size of the buffer.  */
  char *buffer;   /* Malloced buffer.  */
};

/* A simple malloced buffer output handler.  */
static int
dynamic_buffer_out (void *outfncarg, const char *buf, size_t buflen)
{
  struct dynamic_buffer_parm_s *parm = outfncarg;

  if (parm->error_flag)
    {
      /* Just in case some formatting routine did not checked for an
         error. */
      _set_errno (parm->error_flag);
      return -1;
    }

  if (parm->used + buflen >= parm->alloced)
    {
      char *p;

      parm->alloced += buflen + 512;
      p = my_printf_realloc (parm->buffer, parm->alloced);
      if (!p)
        {
          parm->error_flag = errno ? errno : ENOMEM;
          /* Wipe out what we already accumulated.  This is useful in
             case sensitive data is formatted.  */
          memset (parm->buffer, 0, parm->used);
          return -1;
        }
      parm->buffer = p;
    }
  memcpy (parm->buffer + parm->used, buf, buflen);
  parm->used += buflen;

  return 0;
}


/* A replacement for vasprintf.  As with the BSD version of vasprintf
   -1 will be returned on error and NULL stored at BUFP.  On success
   the number of bytes printed will be returned. */
int
_gpgrt_estream_vasprintf (char **bufp, const char *format, va_list arg_ptr)
{
  struct dynamic_buffer_parm_s parm;
  int rc;

  parm.error_flag = 0;
  parm.alloced = 512;
  parm.used = 0;
  parm.buffer = my_printf_realloc (NULL, parm.alloced);
  if (!parm.buffer)
    {
      *bufp = NULL;
      return -1;
    }

  rc = _gpgrt_estream_format (dynamic_buffer_out, &parm, NULL, NULL,
                              format, arg_ptr);
  if (!rc)
    rc = dynamic_buffer_out (&parm, "", 1); /* Print terminating Nul.  */
  /* Fixme: Should we shrink the resulting buffer?  */
  if (rc != -1 && parm.error_flag)
    {
      rc = -1;
      _set_errno (parm.error_flag);
    }
  if (rc == -1)
    {
      memset (parm.buffer, 0, parm.used);
      if (parm.buffer)
        my_printf_realloc (parm.buffer, 0);
      *bufp = NULL;
      return -1;
    }
  gpgrt_assert (parm.used);   /* We have at least the terminating Nul.  */
  *bufp = parm.buffer;
  return parm.used - 1; /* Do not include that Nul. */
}

/* A replacement for asprintf.  As with the BSD of asprintf version -1
   will be returned on error and NULL stored at BUFP.  On success the
   number of bytes printed will be returned. */
int
_gpgrt_estream_asprintf (char **bufp, const char *format, ...)
{
  int rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = _gpgrt_estream_vasprintf (bufp, format, arg_ptr);
  va_end (arg_ptr);

  return rc;
}

/* A variant of asprintf.  The function returns the allocated buffer
   or NULL on error; ERRNO is set in the error case.  The caller
   should use es_free to release the buffer.  This function actually
   belongs into estream-printf but we put it here as a convenience
   and because es_free is required anyway.  */
char *
_gpgrt_estream_bsprintf (const char *format, ...)
{
  int rc;
  va_list ap;
  char *buf;

  va_start (ap, format);
  rc = _gpgrt_estream_vasprintf (&buf, format, ap);
  va_end (ap);
  if (rc < 0)
    return NULL;
  return buf;
}
