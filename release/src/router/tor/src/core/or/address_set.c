/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file address_set.c
 * \brief Implementation for a set of addresses.
 *
 * This module was first written on a semi-emergency basis to improve the
 * robustness of the anti-DoS module.  As such, it's written in a pretty
 * conservative way, and should be susceptible to improvement later on.
 **/

#include "orconfig.h"
#include "core/or/address_set.h"
#include "lib/net/address.h"
#include "lib/container/bloomfilt.h"
#include "lib/crypt_ops/crypto_rand.h"

/** Wrap our hash function to have the signature that the bloom filter
 * needs. */
static uint64_t
bloomfilt_addr_hash(const struct sipkey *key,
                    const void *item)
{
  return tor_addr_keyed_hash(key, item);
}

/**
 * Allocate and return an address_set, suitable for holding up to
 * <b>max_address_guess</b> distinct values.
 */
address_set_t *
address_set_new(int max_addresses_guess)
{
  uint8_t k[BLOOMFILT_KEY_LEN];
  crypto_rand((void*)k, sizeof(k));
  return bloomfilt_new(max_addresses_guess, bloomfilt_addr_hash, k);
}

/**
 * Add <b>addr</b> to <b>set</b>.
 *
 * All future queries for <b>addr</b> in set will return true. Removing
 * items is not possible.
 */
void
address_set_add(address_set_t *set, const struct tor_addr_t *addr)
{
  bloomfilt_add(set, addr);
}

/** As address_set_add(), but take an ipv4 address in host order. */
void
address_set_add_ipv4h(address_set_t *set, uint32_t addr)
{
  tor_addr_t a;
  tor_addr_from_ipv4h(&a, addr);
  address_set_add(set, &a);
}

/**
 * Return true if <b>addr</b> is a member of <b>set</b>.  (And probably,
 * return false if <b>addr</b> is not a member of set.)
 */
int
address_set_probably_contains(const address_set_t *set,
                              const struct tor_addr_t *addr)
{
  return bloomfilt_probably_contains(set, addr);
}
