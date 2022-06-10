/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file scanf.c
 * \brief Locale-independent minimal implementation of sscanf().
 **/

#include "lib/string/scanf.h"
#include "lib/string/compat_ctype.h"
#include "lib/cc/torint.h"
#include "lib/err/torerr.h"

#include <stdlib.h>

#define MAX_SCANF_WIDTH 9999

/** Helper: given an ASCII-encoded decimal digit, return its numeric value.
 * NOTE: requires that its input be in-bounds. */
static int
digit_to_num(char d)
{
  int num = ((int)d) - (int)'0';
  raw_assert(num <= 9 && num >= 0);
  return num;
}

/** Helper: Read an unsigned int from *<b>bufp</b> of up to <b>width</b>
 * characters.  (Handle arbitrary width if <b>width</b> is less than 0.)  On
 * success, store the result in <b>out</b>, advance bufp to the next
 * character, and return 0.  On failure, return -1. */
static int
scan_unsigned(const char **bufp, unsigned long *out, int width, unsigned base)
{
  unsigned long result = 0;
  int scanned_so_far = 0;
  const int hex = base==16;
  raw_assert(base == 10 || base == 16);
  if (!bufp || !*bufp || !out)
    return -1;
  if (width<0)
    width=MAX_SCANF_WIDTH;

  while (**bufp && (hex?TOR_ISXDIGIT(**bufp):TOR_ISDIGIT(**bufp))
         && scanned_so_far < width) {
    unsigned digit = hex?hex_decode_digit(*(*bufp)++):digit_to_num(*(*bufp)++);
    // Check for overflow beforehand, without actually causing any overflow
    // This preserves functionality on compilers that don't wrap overflow
    // (i.e. that trap or optimise away overflow)
    // result * base + digit > ULONG_MAX
    // result * base > ULONG_MAX - digit
    if (result > (ULONG_MAX - digit)/base)
      return -1; /* Processing this digit would overflow */
    result = result * base + digit;
    ++scanned_so_far;
  }

  if (!scanned_so_far) /* No actual digits scanned */
    return -1;

  *out = result;
  return 0;
}

/** Helper: Read an signed int from *<b>bufp</b> of up to <b>width</b>
 * characters.  (Handle arbitrary width if <b>width</b> is less than 0.)  On
 * success, store the result in <b>out</b>, advance bufp to the next
 * character, and return 0.  On failure, return -1. */
static int
scan_signed(const char **bufp, long *out, int width)
{
  int neg = 0;
  unsigned long result = 0;

  if (!bufp || !*bufp || !out)
    return -1;
  if (width<0)
    width=MAX_SCANF_WIDTH;

  if (**bufp == '-') {
    neg = 1;
    ++*bufp;
    --width;
  }

  if (scan_unsigned(bufp, &result, width, 10) < 0)
    return -1;

  if (neg && result > 0) {
    if (result > ((unsigned long)LONG_MAX) + 1)
      return -1; /* Underflow */
    else if (result == ((unsigned long)LONG_MAX) + 1)
      *out = LONG_MIN;
    else {
      /* We once had a far more clever no-overflow conversion here, but
       * some versions of GCC apparently ran it into the ground.  Now
       * we just check for LONG_MIN explicitly.
       */
      *out = -(long)result;
    }
  } else {
    if (result > LONG_MAX)
      return -1; /* Overflow */
    *out = (long)result;
  }

  return 0;
}

/** Helper: Read a decimal-formatted double from *<b>bufp</b> of up to
 * <b>width</b> characters.  (Handle arbitrary width if <b>width</b> is less
 * than 0.)  On success, store the result in <b>out</b>, advance bufp to the
 * next character, and return 0.  On failure, return -1. */
static int
scan_double(const char **bufp, double *out, int width)
{
  int neg = 0;
  double result = 0;
  int scanned_so_far = 0;

  if (!bufp || !*bufp || !out)
    return -1;
  if (width<0)
    width=MAX_SCANF_WIDTH;

  if (**bufp == '-') {
    neg = 1;
    ++*bufp;
  }

  while (**bufp && TOR_ISDIGIT(**bufp) && scanned_so_far < width) {
    const int digit = digit_to_num(*(*bufp)++);
    result = result * 10 + digit;
    ++scanned_so_far;
  }
  if (**bufp == '.') {
    double fracval = 0, denominator = 1;
    ++*bufp;
    ++scanned_so_far;
    while (**bufp && TOR_ISDIGIT(**bufp) && scanned_so_far < width) {
      const int digit = digit_to_num(*(*bufp)++);
      fracval = fracval * 10 + digit;
      denominator *= 10;
      ++scanned_so_far;
    }
    result += fracval / denominator;
  }

  if (!scanned_so_far) /* No actual digits scanned */
    return -1;

  *out = neg ? -result : result;
  return 0;
}

/** Helper: copy up to <b>width</b> non-space characters from <b>bufp</b> to
 * <b>out</b>.  Make sure <b>out</b> is nul-terminated. Advance <b>bufp</b>
 * to the next non-space character or the EOS. */
static int
scan_string(const char **bufp, char *out, int width)
{
  int scanned_so_far = 0;
  if (!bufp || !out || width < 0)
    return -1;
  while (**bufp && ! TOR_ISSPACE(**bufp) && scanned_so_far < width) {
    *out++ = *(*bufp)++;
    ++scanned_so_far;
  }
  *out = '\0';
  return 0;
}

/** Locale-independent, minimal, no-surprises scanf variant, accepting only a
 * restricted pattern format.  For more info on what it supports, see
 * tor_sscanf() documentation.  */
int
tor_vsscanf(const char *buf, const char *pattern, va_list ap)
{
  int n_matched = 0;

  while (*pattern) {
    if (*pattern != '%') {
      if (*buf == *pattern) {
        ++buf;
        ++pattern;
        continue;
      } else {
        return n_matched;
      }
    } else {
      int width = -1;
      int longmod = 0;
      ++pattern;
      if (TOR_ISDIGIT(*pattern)) {
        width = digit_to_num(*pattern++);
        while (TOR_ISDIGIT(*pattern)) {
          width *= 10;
          width += digit_to_num(*pattern++);
          if (width > MAX_SCANF_WIDTH)
            return -1;
        }
        if (!width) /* No zero-width things. */
          return -1;
      }
      if (*pattern == 'l') {
        longmod = 1;
        ++pattern;
      }
      if (*pattern == 'u' || *pattern == 'x') {
        unsigned long u;
        const int base = (*pattern == 'u') ? 10 : 16;
        if (!*buf)
          return n_matched;
        if (scan_unsigned(&buf, &u, width, base)<0)
          return n_matched;
        if (longmod) {
          unsigned long *out = va_arg(ap, unsigned long *);
          *out = u;
        } else {
          unsigned *out = va_arg(ap, unsigned *);
          if (u > UINT_MAX)
            return n_matched;
          *out = (unsigned) u;
        }
        ++pattern;
        ++n_matched;
      } else if (*pattern == 'f') {
        double *d = va_arg(ap, double *);
        if (!longmod)
          return -1; /* float not supported */
        if (!*buf)
          return n_matched;
        if (scan_double(&buf, d, width)<0)
          return n_matched;
        ++pattern;
        ++n_matched;
      } else if (*pattern == 'd') {
        long lng=0;
        if (scan_signed(&buf, &lng, width)<0)
          return n_matched;
        if (longmod) {
          long *out = va_arg(ap, long *);
          *out = lng;
        } else {
          int *out = va_arg(ap, int *);
#if LONG_MAX > INT_MAX
          if (lng < INT_MIN || lng > INT_MAX)
            return n_matched;
#endif
          *out = (int)lng;
        }
        ++pattern;
        ++n_matched;
      } else if (*pattern == 's') {
        char *s = va_arg(ap, char *);
        if (longmod)
          return -1;
        if (width < 0)
          return -1;
        if (scan_string(&buf, s, width)<0)
          return n_matched;
        ++pattern;
        ++n_matched;
      } else if (*pattern == 'c') {
        char *ch = va_arg(ap, char *);
        if (longmod)
          return -1;
        if (width != -1)
          return -1;
        if (!*buf)
          return n_matched;
        *ch = *buf++;
        ++pattern;
        ++n_matched;
      } else if (*pattern == '%') {
        if (*buf != '%')
          return n_matched;
        if (longmod)
          return -1;
        ++buf;
        ++pattern;
      } else {
        return -1; /* Unrecognized pattern component. */
      }
    }
  }

  return n_matched;
}

/** Minimal sscanf replacement: parse <b>buf</b> according to <b>pattern</b>
 * and store the results in the corresponding argument fields.  Differs from
 * sscanf in that:
 * <ul><li>It only handles %u, %lu, %x, %lx, %[NUM]s, %d, %ld, %lf, and %c.
 *     <li>It only handles decimal inputs for %lf. (12.3, not 1.23e1)
 *     <li>It does not handle arbitrarily long widths.
 *     <li>Numbers do not consume any space characters.
 *     <li>It is locale-independent.
 *     <li>%u and %x do not consume any space.
 *     <li>It returns -1 on malformed patterns.</ul>
 *
 * (As with other locale-independent functions, we need this to parse data that
 * is in ASCII without worrying that the C library's locale-handling will make
 * miscellaneous characters look like numbers, spaces, and so on.)
 */
int
tor_sscanf(const char *buf, const char *pattern, ...)
{
  int r;
  va_list ap;
  va_start(ap, pattern);
  r = tor_vsscanf(buf, pattern, ap);
  va_end(ap);
  return r;
}
