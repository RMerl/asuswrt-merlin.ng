/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file parse_int.c
 * \brief Convert strings into the integers they encode, with bounds checking.
 **/

#include "lib/string/parse_int.h"
#include "lib/cc/compat_compiler.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Helper: common code to check whether the result of a strtol or strtoul or
 * strtoll is correct. */
#define CHECK_STRTOX_RESULT()                           \
  STMT_BEGIN                                            \
  /* Did an overflow occur? */                          \
  if (errno == ERANGE)                                  \
    goto err;                                           \
  /* Was at least one character converted? */           \
  if (endptr == s)                                      \
    goto err;                                           \
  /* Were there unexpected unconverted characters? */   \
  if (!next && *endptr)                                 \
    goto err;                                           \
  /* Illogical (max, min) inputs? */                    \
  if (max < min)                                        \
    goto err;                                           \
  /* Is r within limits? */                             \
  if (r < min || r > max)                               \
    goto err;                                           \
  if (ok) *ok = 1;                                      \
  if (next) *next = endptr;                             \
  return r;                                             \
 err:                                                   \
  if (ok) *ok = 0;                                      \
  if (next) *next = endptr;                             \
  return 0;                                             \
  STMT_END

/** Extract a long from the start of <b>s</b>, in the given numeric
 * <b>base</b>.  If <b>base</b> is 0, <b>s</b> is parsed as a decimal,
 * octal, or hex number in the syntax of a C integer literal.  If
 * there is unconverted data and <b>next</b> is provided, set
 * *<b>next</b> to the first unconverted character.  An error has
 * occurred if no characters are converted; or if there are
 * unconverted characters and <b>next</b> is NULL; or if the parsed
 * value is not between <b>min</b> and <b>max</b>.  When no error
 * occurs, return the parsed value and set *<b>ok</b> (if provided) to
 * 1.  When an error occurs, return 0 and set *<b>ok</b> (if provided)
 * to 0.
 */
long
tor_parse_long(const char *s, int base, long min, long max,
               int *ok, char **next)
{
  char *endptr;
  long r;

  if (base < 0) {
    if (ok)
      *ok = 0;
    return 0;
  }

  errno = 0;
  r = strtol(s, &endptr, base);
  CHECK_STRTOX_RESULT();
}

/** As tor_parse_long(), but return an unsigned long. */
unsigned long
tor_parse_ulong(const char *s, int base, unsigned long min,
                unsigned long max, int *ok, char **next)
{
  char *endptr;
  unsigned long r;

  if (base < 0) {
    if (ok)
      *ok = 0;
    return 0;
  }

  errno = 0;
  r = strtoul(s, &endptr, base);
  CHECK_STRTOX_RESULT();
}

/** As tor_parse_long(), but return a double. */
double
tor_parse_double(const char *s, double min, double max, int *ok, char **next)
{
  char *endptr;
  double r;

  errno = 0;
  r = strtod(s, &endptr);
  CHECK_STRTOX_RESULT();
}

/** As tor_parse_long, but return a uint64_t.  Only base 10 is guaranteed to
 * work for now. */
uint64_t
tor_parse_uint64(const char *s, int base, uint64_t min,
                 uint64_t max, int *ok, char **next)
{
  char *endptr;
  uint64_t r;

  if (base < 0) {
    if (ok)
      *ok = 0;
    return 0;
  }

  errno = 0;
#ifdef HAVE_STRTOULL
  r = (uint64_t)strtoull(s, &endptr, base);
#elif defined(_WIN32)
  r = (uint64_t)_strtoui64(s, &endptr, base);
#elif SIZEOF_LONG == 8
  r = (uint64_t)strtoul(s, &endptr, base);
#else
#error "I don't know how to parse 64-bit numbers."
#endif /* defined(HAVE_STRTOULL) || ... */

  CHECK_STRTOX_RESULT();
}
