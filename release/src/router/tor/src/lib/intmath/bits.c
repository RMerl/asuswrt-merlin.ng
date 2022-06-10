/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file bits.c
 *
 * \brief Count the bits in an integer, manipulate powers of 2, etc.
 **/

#include "lib/intmath/bits.h"

/** Returns floor(log2(u64)).  If u64 is 0, (incorrectly) returns 0. */
int
tor_log2(uint64_t u64)
{
  int r = 0;
  if (u64 >= (UINT64_C(1)<<32)) {
    u64 >>= 32;
    r = 32;
  }
  if (u64 >= (UINT64_C(1)<<16)) {
    u64 >>= 16;
    r += 16;
  }
  if (u64 >= (UINT64_C(1)<<8)) {
    u64 >>= 8;
    r += 8;
  }
  if (u64 >= (UINT64_C(1)<<4)) {
    u64 >>= 4;
    r += 4;
  }
  if (u64 >= (UINT64_C(1)<<2)) {
    u64 >>= 2;
    r += 2;
  }
  if (u64 >= (UINT64_C(1)<<1)) {
    // u64 >>= 1; // not using this any more.
    r += 1;
  }
  return r;
}

/** Return the power of 2 in range [1,UINT64_MAX] closest to <b>u64</b>.  If
 * there are two powers of 2 equally close, round down. */
uint64_t
round_to_power_of_2(uint64_t u64)
{
  int lg2;
  uint64_t low;
  uint64_t high;
  if (u64 == 0)
    return 1;

  lg2 = tor_log2(u64);
  low = UINT64_C(1) << lg2;

  if (lg2 == 63)
    return low;

  high = UINT64_C(1) << (lg2+1);
  if (high - u64 < u64 - low)
    return high;
  else
    return low;
}

/** Return the number of bits set in <b>v</b>. */
int
n_bits_set_u8(uint8_t v)
{
  static const int nybble_table[] = {
    0, /* 0000 */
    1, /* 0001 */
    1, /* 0010 */
    2, /* 0011 */
    1, /* 0100 */
    2, /* 0101 */
    2, /* 0110 */
    3, /* 0111 */
    1, /* 1000 */
    2, /* 1001 */
    2, /* 1010 */
    3, /* 1011 */
    2, /* 1100 */
    3, /* 1101 */
    3, /* 1110 */
    4, /* 1111 */
  };

  return nybble_table[v & 15] + nybble_table[v>>4];
}
