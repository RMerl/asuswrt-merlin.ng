/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#define CRYPTO_RAND_PRIVATE
#include "core/or/or.h"
#include "test/test.h"
#include "lib/crypt_ops/aes.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/crypt_ops/crypto_rand.h"

/** Run unit tests for our random number generation function and its wrappers.
 */
static void
test_crypto_rng(void *arg)
{
  int i, j, allok;
  char data1[100], data2[100];
  double d;
  char *h=NULL;

  /* Try out RNG. */
  (void)arg;
  tt_assert(! crypto_seed_rng());
  crypto_rand(data1, 100);
  crypto_rand(data2, 100);
  tt_mem_op(data1,OP_NE, data2,100);
  allok = 1;
  for (i = 0; i < 100; ++i) {
    uint64_t big;
    char *host;
    j = crypto_rand_int(100);
    if (j < 0 || j >= 100)
      allok = 0;
    big = crypto_rand_uint64(UINT64_C(1)<<40);
    if (big >= (UINT64_C(1)<<40))
      allok = 0;
    big = crypto_rand_uint64(UINT64_C(5));
    if (big >= 5)
      allok = 0;
    d = crypto_rand_double();
    tt_assert(d >= 0);
    tt_assert(d < 1.0);
    host = crypto_random_hostname(3,8,"www.",".onion");
    if (strcmpstart(host,"www.") ||
        strcmpend(host,".onion") ||
        strlen(host) < 13 ||
        strlen(host) > 18)
      allok = 0;
    tor_free(host);
  }

  /* Make sure crypto_random_hostname clips its inputs properly. */
  h = crypto_random_hostname(20000, 9000, "www.", ".onion");
  tt_assert(! strcmpstart(h,"www."));
  tt_assert(! strcmpend(h,".onion"));
  tt_int_op(63+4+6, OP_EQ, strlen(h));

  tt_assert(allok);
 done:
  tor_free(h);
}

static void
test_crypto_rng_range(void *arg)
{
  int got_smallest = 0, got_largest = 0;
  int i;

  (void)arg;
  for (i = 0; i < 1000; ++i) {
    int x = crypto_rand_int_range(5,9);
    tt_int_op(x, OP_GE, 5);
    tt_int_op(x, OP_LT, 9);
    if (x == 5)
      got_smallest = 1;
    if (x == 8)
      got_largest = 1;
  }
  /* These fail with probability 1/10^603. */
  tt_assert(got_smallest);
  tt_assert(got_largest);

  got_smallest = got_largest = 0;
  const uint64_t ten_billion = 10 * ((uint64_t)1000000000000);
  for (i = 0; i < 1000; ++i) {
    uint64_t x = crypto_rand_uint64_range(ten_billion, ten_billion+10);
    tt_u64_op(x, OP_GE, ten_billion);
    tt_u64_op(x, OP_LT, ten_billion+10);
    if (x == ten_billion)
      got_smallest = 1;
    if (x == ten_billion+9)
      got_largest = 1;
  }

  tt_assert(got_smallest);
  tt_assert(got_largest);

  const time_t now = time(NULL);
  for (i = 0; i < 2000; ++i) {
    time_t x = crypto_rand_time_range(now, now+60);
    tt_i64_op(x, OP_GE, now);
    tt_i64_op(x, OP_LT, now+60);
    if (x == now)
      got_smallest = 1;
    if (x == now+59)
      got_largest = 1;
  }

  tt_assert(got_smallest);
  tt_assert(got_largest);
 done:
  ;
}

static void
test_crypto_rng_strongest(void *arg)
{
  const char *how = arg;
  int broken = 0;

  if (how == NULL) {
    ;
  } else if (!strcmp(how, "nosyscall")) {
    break_strongest_rng_syscall = 1;
  } else if (!strcmp(how, "nofallback")) {
    break_strongest_rng_fallback = 1;
  } else if (!strcmp(how, "broken")) {
    broken = break_strongest_rng_syscall = break_strongest_rng_fallback = 1;
  }

#define N 128
  uint8_t combine_and[N];
  uint8_t combine_or[N];
  int i, j;

  memset(combine_and, 0xff, N);
  memset(combine_or, 0, N);

  for (i = 0; i < 100; ++i) { /* 2^-100 chances just don't happen. */
    uint8_t output[N];
    memset(output, 0, N);
    if (how == NULL) {
      /* this one can't fail. */
      crypto_strongest_rand(output, sizeof(output));
    } else {
      int r = crypto_strongest_rand_raw(output, sizeof(output));
      if (r == -1) {
        if (broken) {
          goto done; /* we're fine. */
        }
        /* This function is allowed to break, but only if it always breaks. */
        tt_int_op(i, OP_EQ, 0);
        tt_skip();
      } else {
        tt_assert(! broken);
      }
    }
    for (j = 0; j < N; ++j) {
      combine_and[j] &= output[j];
      combine_or[j] |= output[j];
    }
  }

  for (j = 0; j < N; ++j) {
    tt_int_op(combine_and[j], OP_EQ, 0);
    tt_int_op(combine_or[j], OP_EQ, 0xff);
  }
 done:
  ;
#undef N
}

static void
test_crypto_rng_fast(void *arg)
{
  (void)arg;
  crypto_fast_rng_t *rng = crypto_fast_rng_new();
  tt_assert(rng);

  /* Rudimentary black-block test to make sure that our prng outputs
   * have all bits sometimes on and all bits sometimes off. */
  uint64_t m1 = 0, m2 = ~(uint64_t)0;
  const int N = 128;

  for (int i=0; i < N; ++i) {
    uint64_t v;
    crypto_fast_rng_getbytes(rng, (void*)&v, sizeof(v));
    m1 |= v;
    m2 &= v;
  }

  tt_u64_op(m1, OP_EQ, ~(uint64_t)0);
  tt_u64_op(m2, OP_EQ, 0);

  /* Check range functions. */
  int counts[5];
  memset(counts, 0, sizeof(counts));
  for (int i=0; i < N; ++i) {
    unsigned u = crypto_fast_rng_get_uint(rng, 5);
    tt_int_op(u, OP_GE, 0);
    tt_int_op(u, OP_LT, 5);
    counts[u]++;

    uint64_t u64 = crypto_fast_rng_get_uint64(rng, UINT64_C(1)<<40);
    tt_u64_op(u64, OP_GE, 0);
    tt_u64_op(u64, OP_LT, UINT64_C(1)<<40);

    double d = crypto_fast_rng_get_double(rng);
    tt_assert(d >= 0.0);
    tt_assert(d < 1.0);
  }

  /* All values should have come up once. */
  for (int i=0; i<5; ++i) {
    tt_int_op(counts[i], OP_GT, 0);
  }

  /* per-thread rand_fast shouldn't crash or leak. */
  crypto_fast_rng_t *t_rng = get_thread_fast_rng();
  for (int i = 0; i < N; ++i) {
    uint64_t u64 = crypto_fast_rng_get_uint64(t_rng, UINT64_C(1)<<40);
    tt_u64_op(u64, OP_GE, 0);
    tt_u64_op(u64, OP_LT, UINT64_C(1)<<40);
  }

 done:
  crypto_fast_rng_free(rng);
}

static void
test_crypto_rng_fast_whitebox(void *arg)
{
  (void)arg;
  const size_t buflen = crypto_fast_rng_get_bytes_used_per_stream();
  char *buf = tor_malloc_zero(buflen);
  char *buf2 = tor_malloc_zero(buflen);
  char *buf3 = NULL, *buf4 = NULL;

  crypto_cipher_t *cipher = NULL, *cipher2 = NULL;
  uint8_t seed[CRYPTO_FAST_RNG_SEED_LEN];
  memset(seed, 0, sizeof(seed));

  /* Start with a prng with zero key and zero IV. */
  crypto_fast_rng_t *rng = crypto_fast_rng_new_from_seed(seed);
  tt_assert(rng);

  /* We'll use a stream cipher to keep in sync */
  cipher = crypto_cipher_new_with_iv_and_bits(seed, seed+32, 256);

  /* The first 48 bytes are used for the next seed -- let's make sure we have
   * them.
   */
  memset(seed, 0, sizeof(seed));
  crypto_cipher_crypt_inplace(cipher, (char*)seed, sizeof(seed));

  /* if we get 128 bytes, they should match the bytes from the aes256-counter
   * stream, starting at position 48.
   */
  crypto_fast_rng_getbytes(rng, (uint8_t*)buf, 128);
  memset(buf2, 0, 128);
  crypto_cipher_crypt_inplace(cipher, buf2, 128);
  tt_mem_op(buf, OP_EQ, buf2, 128);

  /* Try that again, with an odd number of bytes. */
  crypto_fast_rng_getbytes(rng, (uint8_t*)buf, 199);
  memset(buf2, 0, 199);
  crypto_cipher_crypt_inplace(cipher, buf2, 199);
  tt_mem_op(buf, OP_EQ, buf2, 199);

  /* Make sure that refilling works as expected: skip all but the last 5 bytes
   * of this steam. */
  size_t skip = buflen - (199+128) - 5;
  crypto_fast_rng_getbytes(rng, (uint8_t*)buf, skip);
  crypto_cipher_crypt_inplace(cipher, buf2, skip);

  /* Now get the next 128 bytes. The first 5 will come from this stream, and
   * the next 5 will come from the stream keyed by the new value of 'seed'. */
  crypto_fast_rng_getbytes(rng, (uint8_t*)buf, 128);
  memset(buf2, 0, 128);
  crypto_cipher_crypt_inplace(cipher, buf2, 5);
  crypto_cipher_free(cipher);
  cipher = crypto_cipher_new_with_iv_and_bits(seed, seed+32, 256);
  memset(seed, 0, sizeof(seed));
  crypto_cipher_crypt_inplace(cipher, (char*)seed, sizeof(seed));
  crypto_cipher_crypt_inplace(cipher, buf2+5, 128-5);
  tt_mem_op(buf, OP_EQ, buf2, 128);

  /* And check the next 7 bytes to make sure we didn't discard anything. */
  crypto_fast_rng_getbytes(rng, (uint8_t*)buf, 7);
  memset(buf2, 0, 7);
  crypto_cipher_crypt_inplace(cipher, buf2, 7);
  tt_mem_op(buf, OP_EQ, buf2, 7);

  /* Now try the optimization for long outputs. */
  buf3 = tor_malloc(65536);
  crypto_fast_rng_getbytes(rng, (uint8_t*)buf3, 65536);

  buf4 = tor_malloc_zero(65536);
  uint8_t seed2[CRYPTO_FAST_RNG_SEED_LEN];
  memset(seed2, 0, sizeof(seed2));
  crypto_cipher_crypt_inplace(cipher, (char*)seed2, sizeof(seed2));
  cipher2 = crypto_cipher_new_with_iv_and_bits(seed2, seed2+32, 256);
  crypto_cipher_crypt_inplace(cipher2, buf4, 65536);
  tt_mem_op(buf3, OP_EQ, buf4, 65536);

 done:
  crypto_fast_rng_free(rng);
  crypto_cipher_free(cipher);
  crypto_cipher_free(cipher2);
  tor_free(buf);
  tor_free(buf2);
  tor_free(buf3);
  tor_free(buf4);
}

struct testcase_t crypto_rng_tests[] = {
  { "rng", test_crypto_rng, 0, NULL, NULL },
  { "rng_range", test_crypto_rng_range, 0, NULL, NULL },
  { "rng_strongest", test_crypto_rng_strongest, TT_FORK, NULL, NULL },
  { "rng_strongest_nosyscall", test_crypto_rng_strongest, TT_FORK,
    &passthrough_setup, (void*)"nosyscall" },
  { "rng_strongest_nofallback", test_crypto_rng_strongest, TT_FORK,
    &passthrough_setup, (void*)"nofallback" },
  { "rng_strongest_broken", test_crypto_rng_strongest, TT_FORK,
    &passthrough_setup, (void*)"broken" },
  { "fast", test_crypto_rng_fast, 0, NULL, NULL },
  { "fast_whitebox", test_crypto_rng_fast_whitebox, 0, NULL, NULL },
  END_OF_TESTCASES
};
