/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file digestset.c
 * \brief Implementation for a set of digests
 **/

#include "orconfig.h"
#include "lib/container/bloomfilt.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/defs/digest_sizes.h"
#include "lib/crypt_ops/digestset.h"
#include "ext/siphash.h"

/* Wrap our hash function to have the signature that the bloom filter
 * needs. */
static uint64_t
bloomfilt_digest_hash(const struct sipkey *key,
                      const void *item)
{
  return siphash24(item, DIGEST_LEN, key);
}

/**
 * Allocate and return an digestset, suitable for holding up to
 * <b>max_guess</b> distinct values.
 */
digestset_t *
digestset_new(int max_guess)
{
  uint8_t k[BLOOMFILT_KEY_LEN];
  crypto_rand((void*)k, sizeof(k));
  return bloomfilt_new(max_guess, bloomfilt_digest_hash, k);
}

/**
 * Add <b>digest</b> to <b>set</b>.
 *
 * All future queries for <b>digest</b> in set will return true. Removing
 * items is not possible.
 */
void
digestset_add(digestset_t *set, const char *digest)
{
  bloomfilt_add(set, digest);
}

/**
 * Return true if <b>digest</b> is a member of <b>set</b>.  (And probably,
 * return false if <b>digest</b> is not a member of set.)
 */
int
digestset_probably_contains(const digestset_t *set,
                            const char *digest)
{
  return bloomfilt_probably_contains(set, digest);
}
