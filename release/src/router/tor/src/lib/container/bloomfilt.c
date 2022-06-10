/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file bloomfilt.c
 * \brief Uses bitarray_t to implement a bloom filter.
 **/

#include <stdlib.h>

#include "lib/malloc/malloc.h"
#include "lib/container/bloomfilt.h"
#include "lib/intmath/bits.h"
#include "lib/log/util_bug.h"
#include "ext/siphash.h"

/** How many bloom-filter bits we set per address. This is twice the
 * BLOOMFILT_N_HASHES value, since we split the siphash output into two 32-bit
 * values. */
#define N_BITS_PER_ITEM (BLOOMFILT_N_HASHES * 2)

struct bloomfilt_t {
  /** siphash keys to make BLOOMFILT_N_HASHES independent hashes for each
   * items. */
  struct sipkey key[BLOOMFILT_N_HASHES];
  bloomfilt_hash_fn hashfn; /**< Function used to generate hashes */
  uint32_t mask; /**< One less than the number of bits in <b>ba</b>; always
                  * one less than a power of two. */
  bitarray_t *ba; /**< A bit array to implement the Bloom filter. */
};

#define BIT(set, n) ((n) & (set)->mask)

/** Add the element <b>item</b> to <b>set</b>. */
void
bloomfilt_add(bloomfilt_t *set,
              const void *item)
{
  int i;
  for (i = 0; i < BLOOMFILT_N_HASHES; ++i) {
    uint64_t h = set->hashfn(&set->key[i], item);
    uint32_t high_bits = (uint32_t)(h >> 32);
    uint32_t low_bits = (uint32_t)(h);
    bitarray_set(set->ba, BIT(set, high_bits));
    bitarray_set(set->ba, BIT(set, low_bits));
  }
}

/** If <b>item</b> is in <b>set</b>, return nonzero.  Otherwise,
 * <em>probably</em> return zero. */
int
bloomfilt_probably_contains(const bloomfilt_t *set,
                            const void *item)
{
  int i, matches = 0;
  for (i = 0; i < BLOOMFILT_N_HASHES; ++i) {
    uint64_t h = set->hashfn(&set->key[i], item);
    uint32_t high_bits = (uint32_t)(h >> 32);
    uint32_t low_bits = (uint32_t)(h);
    // Note that !! is necessary here, since bitarray_is_set does not
    // necessarily return 1 on true.
    matches += !! bitarray_is_set(set->ba, BIT(set, high_bits));
    matches += !! bitarray_is_set(set->ba, BIT(set, low_bits));
  }
  return matches == N_BITS_PER_ITEM;
}

/** Return a newly allocated bloomfilt_t, optimized to hold a total of
 * <b>max_elements</b> elements with a reasonably low false positive weight.
 *
 * Uses the siphash-based function <b>hashfn</b> to compute hard-to-collide
 * functions of the items, and the key material <b>random_key</b> to
 * key the hash.  There must be BLOOMFILT_KEY_LEN bytes in the supplied key.
 **/
bloomfilt_t *
bloomfilt_new(int max_elements,
              bloomfilt_hash_fn hashfn,
              const uint8_t *random_key)
{
  /* The probability of false positives is about P=(1 - exp(-kn/m))^k, where k
   * is the number of hash functions per entry, m is the bits in the array,
   * and n is the number of elements inserted.  For us, k==4, n<=max_elements,
   * and m==n_bits= approximately max_elements*32.  This gives
   *   P<(1-exp(-4*n/(32*n)))^4 == (1-exp(1/-8))^4 == .00019
   *
   * It would be more optimal in space vs false positives to get this false
   * positive rate by going for k==13, and m==18.5n, but we also want to
   * conserve CPU, and k==13 is pretty big.
   */
  int n_bits = 1u << (tor_log2(max_elements)+5);
  bloomfilt_t *r = tor_malloc(sizeof(bloomfilt_t));
  r->mask = n_bits - 1;
  r->ba = bitarray_init_zero(n_bits);

  tor_assert(sizeof(r->key) == BLOOMFILT_KEY_LEN);
  memcpy(r->key, random_key, sizeof(r->key));

  r->hashfn = hashfn;

  return r;
}

/** Free all storage held in <b>set</b>. */
void
bloomfilt_free_(bloomfilt_t *set)
{
  if (!set)
    return;
  bitarray_free(set->ba);
  tor_free(set);
}
