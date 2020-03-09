/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file unitparse.c
 * @brief Functions for parsing values with units from a configuration file.
 **/

#include "orconfig.h"
#include "lib/confmgt/unitparse.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/string/parse_int.h"
#include "lib/string/util_string.h"

#include <string.h>

/** Table to map the names of memory units to the number of bytes they
 * contain. */
const struct unit_table_t memory_units[] = {
  { "",          1 },
  { "b",         1<< 0 },
  { "byte",      1<< 0 },
  { "bytes",     1<< 0 },
  { "kb",        1<<10 },
  { "kbyte",     1<<10 },
  { "kbytes",    1<<10 },
  { "kilobyte",  1<<10 },
  { "kilobytes", 1<<10 },
  { "kilobits",  1<<7  },
  { "kilobit",   1<<7  },
  { "kbits",     1<<7  },
  { "kbit",      1<<7  },
  { "m",         1<<20 },
  { "mb",        1<<20 },
  { "mbyte",     1<<20 },
  { "mbytes",    1<<20 },
  { "megabyte",  1<<20 },
  { "megabytes", 1<<20 },
  { "megabits",  1<<17 },
  { "megabit",   1<<17 },
  { "mbits",     1<<17 },
  { "mbit",      1<<17 },
  { "gb",        1<<30 },
  { "gbyte",     1<<30 },
  { "gbytes",    1<<30 },
  { "gigabyte",  1<<30 },
  { "gigabytes", 1<<30 },
  { "gigabits",  1<<27 },
  { "gigabit",   1<<27 },
  { "gbits",     1<<27 },
  { "gbit",      1<<27 },
  { "tb",        UINT64_C(1)<<40 },
  { "tbyte",     UINT64_C(1)<<40 },
  { "tbytes",    UINT64_C(1)<<40 },
  { "terabyte",  UINT64_C(1)<<40 },
  { "terabytes", UINT64_C(1)<<40 },
  { "terabits",  UINT64_C(1)<<37 },
  { "terabit",   UINT64_C(1)<<37 },
  { "tbits",     UINT64_C(1)<<37 },
  { "tbit",      UINT64_C(1)<<37 },
  { NULL, 0 },
};

/** Table to map the names of time units to the number of seconds they
 * contain. */
const struct unit_table_t time_units[] = {
  { "",         1 },
  { "second",   1 },
  { "seconds",  1 },
  { "minute",   60 },
  { "minutes",  60 },
  { "hour",     60*60 },
  { "hours",    60*60 },
  { "day",      24*60*60 },
  { "days",     24*60*60 },
  { "week",     7*24*60*60 },
  { "weeks",    7*24*60*60 },
  { "month",    2629728, }, /* about 30.437 days */
  { "months",   2629728, },
  { NULL, 0 },
};

/** Table to map the names of time units to the number of milliseconds
 * they contain. */
const struct unit_table_t time_msec_units[] = {
  { "",         1 },
  { "msec",     1 },
  { "millisecond", 1 },
  { "milliseconds", 1 },
  { "second",   1000 },
  { "seconds",  1000 },
  { "minute",   60*1000 },
  { "minutes",  60*1000 },
  { "hour",     60*60*1000 },
  { "hours",    60*60*1000 },
  { "day",      24*60*60*1000 },
  { "days",     24*60*60*1000 },
  { "week",     7*24*60*60*1000 },
  { "weeks",    7*24*60*60*1000 },
  { NULL, 0 },
};

/** Parse a string <b>val</b> containing a number, zero or more
 * spaces, and an optional unit string.  If the unit appears in the
 * table <b>u</b>, then multiply the number by the unit multiplier.
 * On success, set *<b>ok</b> to 1 and return this product.
 * Otherwise, set *<b>ok</b> to 0.
 */
uint64_t
config_parse_units(const char *val, const unit_table_t *u, int *ok)
{
  uint64_t v = 0;
  double d = 0;
  int use_float = 0;
  char *cp;

  tor_assert(ok);

  v = tor_parse_uint64(val, 10, 0, UINT64_MAX, ok, &cp);
  if (!*ok || (cp && *cp == '.')) {
    d = tor_parse_double(val, 0, (double)UINT64_MAX, ok, &cp);
    if (!*ok)
      goto done;
    use_float = 1;
  }

  if (BUG(!cp)) {
    // cp should always be non-NULL if the parse operation succeeds.

    // LCOV_EXCL_START
    *ok = 1;
    v = use_float ? ((uint64_t)d) :  v;
    goto done;
    // LCOV_EXCL_STOP
  }

  cp = (char*) eat_whitespace(cp);

  for ( ;u->unit;++u) {
    if (!strcasecmp(u->unit, cp)) {
      if (use_float)
        v = (uint64_t)(u->multiplier * d);
      else
        v *= u->multiplier;
      *ok = 1;
      goto done;
    }
  }
  log_warn(LD_CONFIG, "Unknown unit '%s'.", cp);
  *ok = 0;
 done:

  if (*ok)
    return v;
  else
    return 0;
}

/** Parse a string in the format "number unit", where unit is a unit of
 * information (byte, KB, M, etc).  On success, set *<b>ok</b> to true
 * and return the number of bytes specified.  Otherwise, set
 * *<b>ok</b> to false and return 0. */
uint64_t
config_parse_memunit(const char *s, int *ok)
{
  uint64_t u = config_parse_units(s, memory_units, ok);
  return u;
}

/** Parse a string in the format "number unit", where unit is a unit of
 * time in milliseconds.  On success, set *<b>ok</b> to true and return
 * the number of milliseconds in the provided interval.  Otherwise, set
 * *<b>ok</b> to 0 and return -1. */
int
config_parse_msec_interval(const char *s, int *ok)
{
  uint64_t r;
  r = config_parse_units(s, time_msec_units, ok);
  if (r > INT_MAX) {
    log_warn(LD_CONFIG, "Msec interval '%s' is too long", s);
    *ok = 0;
    return -1;
  }
  return (int)r;
}

/** Parse a string in the format "number unit", where unit is a unit of time.
 * On success, set *<b>ok</b> to true and return the number of seconds in
 * the provided interval.  Otherwise, set *<b>ok</b> to 0 and return -1.
 */
int
config_parse_interval(const char *s, int *ok)
{
  uint64_t r;
  r = config_parse_units(s, time_units, ok);
  if (r > INT_MAX) {
    log_warn(LD_CONFIG, "Interval '%s' is too long", s);
    *ok = 0;
    return -1;
  }
  return (int)r;
}
