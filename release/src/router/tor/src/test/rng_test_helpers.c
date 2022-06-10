/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rng_test_helpers.c
 * \brief Helpers for overriding PRNGs during unit tests.
 *
 * We define two PRNG overrides: a "reproducible PRNG" where the seed is
 * chosen randomly but the stream can be replayed later on in case a bug is
 * found, and a "deterministic PRNG" where the seed is fixed in the unit
 * tests.
 *
 * Obviously, this code is testing-only.
 */

#include "orconfig.h"
#include "core/or/or.h"

#include "lib/crypt_ops/crypto_rand.h"
#include "ext/tinytest.h"

#include "test/rng_test_helpers.h"

#ifndef TOR_UNIT_TESTS
#error "No. Never link this code into Tor proper."
#endif

/**
 * True iff the RNG is currently replaced.  Prevents double-replacement.
 **/
static bool rng_is_replaced = false;

/**
 * Mutex to protect deterministic prng.
 *
 * Note that if you actually _use_ the prng from two threads at the same time,
 * the results will probably be nondeterministic anyway.
 */
static tor_mutex_t *rng_mutex = NULL;

/**
 * Cached old value for the thread prng.
 **/
static crypto_fast_rng_t *stored_fast_rng = NULL;

/** replacement for crypto_strongest_rand that delegates to crypto_rand. */
static void
mock_crypto_strongest_rand(uint8_t *out, size_t len)
{
  crypto_rand((char *)out, len);
}

/* This is the seed of the deterministic randomness. */
static uint8_t rng_seed[16];
static crypto_xof_t *rng_xof = NULL;

/**
 * Print the seed for our PRNG to stdout.  We use this when we're failed
 * test that had a reproducible RNG set.
 **/
void
testing_dump_reproducible_rng_seed(void)
{
  printf("\n"
         "Seed: %s\n",
         hex_str((const char*)rng_seed, sizeof(rng_seed)));
}

/** Produce deterministic randomness for the stochastic tests using the global
 * rng_xof output.
 *
 * This function produces deterministic data over multiple calls iff it's
 * called in the same call order with the same 'n' parameter.
 * If not, outputs will deviate. */
static void
crypto_rand_deterministic(char *out, size_t n)
{
  tor_assert(rng_xof);
  tor_mutex_acquire(rng_mutex);
  crypto_xof_squeeze_bytes(rng_xof, (uint8_t*)out, n);
  tor_mutex_release(rng_mutex);
}

/**
 * Implementation helper: override our crypto_rand() PRNG with a given seed of
 * length <b>seed_len</b>.  Overlong seeds are truncated; short ones are
 * padded.
 **/
static void
enable_deterministic_rng_impl(const uint8_t *seed, size_t seed_len)
{
  tor_assert(!rng_is_replaced);
  tor_assert(crypto_rand == crypto_rand__real);

  memset(rng_seed, 0, sizeof(rng_seed));
  memcpy(rng_seed, seed, MIN(seed_len, sizeof(rng_seed)));

  rng_mutex = tor_mutex_new();

  crypto_xof_free(rng_xof);
  rng_xof = crypto_xof_new();
  crypto_xof_add_bytes(rng_xof, rng_seed, sizeof(rng_seed));
  MOCK(crypto_rand, crypto_rand_deterministic);
  MOCK(crypto_strongest_rand_, mock_crypto_strongest_rand);

  uint8_t fast_rng_seed[CRYPTO_FAST_RNG_SEED_LEN];
  memset(fast_rng_seed, 0xff, sizeof(fast_rng_seed));
  memcpy(fast_rng_seed, rng_seed, MIN(sizeof(rng_seed),
                                      sizeof(fast_rng_seed)));
  crypto_fast_rng_t *fast_rng = crypto_fast_rng_new_from_seed(fast_rng_seed);
  crypto_fast_rng_disable_reseed(fast_rng);
  stored_fast_rng = crypto_replace_thread_fast_rng(fast_rng);

  rng_is_replaced = true;
}

/**
 * Replace our get_thread_fast_rng(), crypto_rand() and
 * crypto_strongest_rand() prngs with a variant that generates all of its
 * output deterministically from a randomly chosen seed.  In the event of an
 * error, you can log the seed later on with
 * testing_dump_reproducible_rng_seed.
 **/
void
testing_enable_reproducible_rng(void)
{
  const char *provided_seed = getenv("TOR_TEST_RNG_SEED");
  if (provided_seed) {
    size_t hexlen = strlen(provided_seed);
    size_t seedlen = hexlen / 2;
    uint8_t *seed = tor_malloc(hexlen / 2);
    if (base16_decode((char*)seed, seedlen, provided_seed, hexlen) < 0) {
      puts("Cannot decode value in TOR_TEST_RNG_SEED");
      exit(1);
    }
    enable_deterministic_rng_impl(seed, seedlen);
    tor_free(seed);
  } else {
    uint8_t seed[16];
    crypto_rand((char*)seed, sizeof(seed));
    enable_deterministic_rng_impl(seed, sizeof(seed));
  }
}

/**
 * Replace our get_thread_fast_rng(), crypto_rand() and
 * crypto_strongest_rand() prngs with a variant that generates all of its
 * output deterministically from a fixed seed.  This variant is mainly useful
 * for cases when we don't want coverage to change between runs.
 *
 * USAGE NOTE: Test correctness SHOULD NOT depend on the specific output of
 * this "rng".  If you need a specific output, use
 * testing_enable_prefilled_rng() instead.
 **/
void
testing_enable_deterministic_rng(void)
{
  static const uint8_t quotation[] =
    "What will it be? A tree? A weed? "
    "Each one is started from a seed."; // -- Mary Ann Hoberman
  enable_deterministic_rng_impl(quotation, sizeof(quotation));
}

static uint8_t *prefilled_rng_buffer = NULL;
static size_t prefilled_rng_buflen;
static size_t prefilled_rng_idx;

/**
 * crypto_rand() replacement that returns canned data.
 **/
static void
crypto_rand_prefilled(char *out, size_t n)
{
  tor_mutex_acquire(rng_mutex);
  while (n) {
    size_t n_to_copy = MIN(prefilled_rng_buflen - prefilled_rng_idx, n);
    memcpy(out, prefilled_rng_buffer + prefilled_rng_idx, n_to_copy);
    out += n_to_copy;
    n -= n_to_copy;
    prefilled_rng_idx += n_to_copy;

    if (prefilled_rng_idx == prefilled_rng_buflen) {
      prefilled_rng_idx = 0;
    }
  }
  tor_mutex_release(rng_mutex);
}

/**
 * Replace our crypto_rand() and crypto_strongest_rand() prngs with a variant
 * that yields output from a buffer.  If it reaches the end of the buffer, it
 * starts over.
 *
 * Note: the get_thread_fast_rng() prng is not replaced by this; we'll need
 * more code to support that.
 **/
void
testing_enable_prefilled_rng(const void *buffer, size_t buflen)
{
  tor_assert(buflen > 0);
  tor_assert(!rng_mutex);
  rng_mutex = tor_mutex_new();

  tor_mutex_acquire(rng_mutex);

  prefilled_rng_buffer = tor_memdup(buffer, buflen);
  prefilled_rng_buflen = buflen;
  prefilled_rng_idx = 0;

  tor_mutex_release(rng_mutex);

  MOCK(crypto_rand, crypto_rand_prefilled);
  MOCK(crypto_strongest_rand_, mock_crypto_strongest_rand);
}

/**
 * Reset the position in the prefilled RNG buffer to the start.
 */
void
testing_prefilled_rng_reset(void)
{
  tor_mutex_acquire(rng_mutex);
  prefilled_rng_idx = 0;
  tor_mutex_release(rng_mutex);
}

/**
 * Undo the overrides for our PRNG.  To be used at the end of testing.
 *
 * Note that this function should be safe to call even if the rng has not
 * yet been replaced.
 **/
void
testing_disable_rng_override(void)
{
  crypto_xof_free(rng_xof);
  tor_free(prefilled_rng_buffer);
  UNMOCK(crypto_rand);
  UNMOCK(crypto_strongest_rand_);
  tor_mutex_free(rng_mutex);

  crypto_fast_rng_t *rng = crypto_replace_thread_fast_rng(stored_fast_rng);
  crypto_fast_rng_free(rng);

  rng_is_replaced = false;
}

/**
 * As testing_disable_rng_override(), but dump the seed if the current
 * test has failed.
 */
void
testing_disable_reproducible_rng(void)
{
  if (tinytest_cur_test_has_failed()) {
    testing_dump_reproducible_rng_seed();
  }
  testing_disable_rng_override();
}
