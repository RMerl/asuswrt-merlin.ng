/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file weakrng.c
 *
 * \brief A weak but fast PRNG based on a linear congruential generator.
 *
 * We don't want to use the platform random(), since some of them are even
 * worse than this.
 **/

#include "lib/intmath/weakrng.h"
#include "lib/err/torerr.h"

#include <stdlib.h>

/** Initialize the insecure RNG <b>rng</b> from a seed value <b>seed</b>. */
void
tor_init_weak_random(tor_weak_rng_t *rng, unsigned seed)
{
  rng->state = (uint32_t)(seed & 0x7fffffff);
}

/** Return a randomly chosen value in the range 0..TOR_WEAK_RANDOM_MAX based
 * on the RNG state of <b>rng</b>.  This entropy will not be cryptographically
 * strong; do not rely on it for anything an adversary should not be able to
 * predict. */
int32_t
tor_weak_random(tor_weak_rng_t *rng)
{
  /* Here's a linear congruential generator. OpenBSD and glibc use these
   * parameters; they aren't too bad, and should have maximal period over the
   * range 0..INT32_MAX. We don't want to use the platform rand() or random(),
   * since some platforms have bad weak RNGs that only return values in the
   * range 0..INT16_MAX, which just isn't enough. */
  rng->state = (rng->state * 1103515245 + 12345) & 0x7fffffff;
  return (int32_t) rng->state;
}

/** Return a random number in the range [0 , <b>top</b>). {That is, the range
 * of integers i such that 0 <= i < top.}  Chooses uniformly.  Requires that
 * top is greater than 0. This randomness is not cryptographically strong; do
 * not rely on it for anything an adversary should not be able to predict. */
int32_t
tor_weak_random_range(tor_weak_rng_t *rng, int32_t top)
{
  /* We don't want to just do tor_weak_random() % top, since random() is often
   * implemented with an LCG whose modulus is a power of 2, and those are
   * cyclic in their low-order bits. */
  int divisor, result;
  raw_assert(top > 0);
  divisor = TOR_WEAK_RANDOM_MAX / top;
  do {
    result = (int32_t)(tor_weak_random(rng) / divisor);
  } while (result >= top);
  return result;
}
