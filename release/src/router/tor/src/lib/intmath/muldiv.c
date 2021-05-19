/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file muldiv.c
 *
 * \brief Integer math related to multiplication, division, and rounding.
 **/

#include "lib/intmath/muldiv.h"
#include "lib/err/torerr.h"

#include <stdlib.h>

/** Return the lowest x such that x is at least <b>number</b>, and x modulo
 * <b>divisor</b> == 0.  If no such x can be expressed as an unsigned, return
 * UINT_MAX. Asserts if divisor is zero. */
unsigned
round_to_next_multiple_of(unsigned number, unsigned divisor)
{
  raw_assert(divisor > 0);
  if (UINT_MAX - divisor + 1 < number)
    return UINT_MAX;
  number += divisor - 1;
  number -= number % divisor;
  return number;
}

/** Return the lowest x such that x is at least <b>number</b>, and x modulo
 * <b>divisor</b> == 0. If no such x can be expressed as a uint32_t, return
 * UINT32_MAX. Asserts if divisor is zero. */
uint32_t
round_uint32_to_next_multiple_of(uint32_t number, uint32_t divisor)
{
  raw_assert(divisor > 0);
  if (UINT32_MAX - divisor + 1 < number)
    return UINT32_MAX;

  number += divisor - 1;
  number -= number % divisor;
  return number;
}

/** Return the lowest x such that x is at least <b>number</b>, and x modulo
 * <b>divisor</b> == 0. If no such x can be expressed as a uint64_t, return
 * UINT64_MAX. Asserts if divisor is zero. */
uint64_t
round_uint64_to_next_multiple_of(uint64_t number, uint64_t divisor)
{
  raw_assert(divisor > 0);
  if (UINT64_MAX - divisor + 1 < number)
    return UINT64_MAX;
  number += divisor - 1;
  number -= number % divisor;
  return number;
}

/* Helper: return greatest common divisor of a,b */
static uint64_t
gcd64(uint64_t a, uint64_t b)
{
  while (b) {
    uint64_t t = b;
    b = a % b;
    a = t;
  }
  return a;
}

/** Return the unsigned integer product of <b>a</b> and <b>b</b>. If overflow
 * is detected, return UINT64_MAX instead. */
uint64_t
tor_mul_u64_nowrap(uint64_t a, uint64_t b)
{
  if (a == 0 || b == 0) {
    return 0;
  } else if (PREDICT_UNLIKELY(UINT64_MAX / a < b)) {
    return UINT64_MAX;
  } else {
    return a*b;
  }
}

/* Given a fraction *<b>numer</b> / *<b>denom</b>, simplify it.
 * Requires that the denominator is greater than 0. */
void
simplify_fraction64(uint64_t *numer, uint64_t *denom)
{
  raw_assert(denom);
  uint64_t gcd = gcd64(*numer, *denom);
  *numer /= gcd;
  *denom /= gcd;
}
