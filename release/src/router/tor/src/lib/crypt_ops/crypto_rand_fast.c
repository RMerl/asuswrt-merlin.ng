/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_rand_fast.c
 *
 * \brief A fast strong PRNG for use when our underlying cryptographic
 *   library's PRNG isn't fast enough.
 **/

/* This library is currently implemented to use the same implementation
 * technique as libottery, using AES-CTR-256 as our underlying stream cipher.
 * It's backtracking-resistant immediately, and prediction-resistant after
 * a while.
 *
 * Here's how it works:
 *
 * We generate pseudorandom bytes using AES-CTR-256.  We generate BUFLEN bytes
 * at a time.  When we do this, we keep the first SEED_LEN bytes as the key
 * and the IV for our next invocation of AES_CTR, and yield the remaining
 * BUFLEN - SEED_LEN bytes to the user as they invoke the PRNG.  As we yield
 * bytes to the user, we clear them from the buffer.
 *
 * After we have refilled the buffer RESEED_AFTER times, we mix in an
 * additional SEED_LEN bytes from our strong PRNG into the seed.
 *
 * If the user ever asks for a huge number of bytes at once, we pull SEED_LEN
 * bytes from the PRNG and use them with our stream cipher to fill the user's
 * request.
 */

#define CRYPTO_PRIVATE

#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_digest.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/intmath/cmp.h"
#include "lib/cc/ctassert.h"
#include "lib/malloc/map_anon.h"
#include "lib/thread/threads.h"

#include "lib/log/util_bug.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>

#ifdef NOINHERIT_CAN_FAIL
#define CHECK_PID
#endif

#ifdef CHECK_PID
#define PID_FIELD_LEN sizeof(pid_t)
#else
#define PID_FIELD_LEN 0
#endif

/* Alias for CRYPTO_FAST_RNG_SEED_LEN to make our code shorter.
 */
#define SEED_LEN (CRYPTO_FAST_RNG_SEED_LEN)

/* The amount of space that we mmap for a crypto_fast_rng_t.
 */
#define MAPLEN 4096

/* The number of random bytes that we can yield to the user after each
 * time we fill a crypto_fast_rng_t's buffer.
 */
#define BUFLEN (MAPLEN - 2*sizeof(uint16_t) - SEED_LEN - PID_FIELD_LEN)

/* The number of buffer refills after which we should fetch more
 * entropy from crypto_strongest_rand().
 */
#define RESEED_AFTER 16

/* The length of the stream cipher key we will use for the PRNG, in bytes.
 */
#define KEY_LEN (CRYPTO_FAST_RNG_SEED_LEN - CIPHER_IV_LEN)
/* The length of the stream cipher key we will use for the PRNG, in bits.
 */
#define KEY_BITS (KEY_LEN * 8)

/* Make sure that we have a key length we can actually use with AES. */
CTASSERT(KEY_BITS == 128 || KEY_BITS == 192 || KEY_BITS == 256);

struct crypto_fast_rng_t {
  /** How many more fills does this buffer have before we should mix
   * in the output of crypto_strongest_rand()?
   *
   * This value may be negative if unit tests are enabled.  If so, it
   * indicates that we should never mix in extra data from
   * crypto_strongest_rand().
   */
  int16_t n_till_reseed;
  /** How many bytes are remaining in cbuf_t.bytes? */
  uint16_t bytes_left;
#ifdef CHECK_PID
  /** Which process owns this fast_rng?  If this value is zero, we do not
   * need to test the owner. */
  pid_t owner;
#endif
  struct cbuf_t {
    /** The seed (key and IV) that we will use the next time that we refill
     * cbuf_t. */
    uint8_t seed[SEED_LEN];
    /**
     * Bytes that we are yielding to the user.  The next byte to be
     * yielded is at bytes[BUFLEN-bytes_left]; all other bytes in this
     * array are set to zero.
     */
    uint8_t bytes[BUFLEN];
  } buf;
};

/* alignof(uint8_t) should be 1, so there shouldn't be any padding in cbuf_t.
 */
CTASSERT(sizeof(struct cbuf_t) == BUFLEN+SEED_LEN);
/* We're trying to fit all of the RNG state into a nice mmapable chunk.
 */
CTASSERT(sizeof(crypto_fast_rng_t) <= MAPLEN);

/**
 * Initialize and return a new fast PRNG, using a strong random seed.
 *
 * Note that this object is NOT thread-safe.  If you need a thread-safe
 * prng, use crypto_rand(), or wrap this in a mutex.
 **/
crypto_fast_rng_t *
crypto_fast_rng_new(void)
{
  uint8_t seed[SEED_LEN];
  crypto_strongest_rand(seed, sizeof(seed));
  crypto_fast_rng_t *result = crypto_fast_rng_new_from_seed(seed);
  memwipe(seed, 0, sizeof(seed));
  return result;
}

/**
 * Initialize and return a new fast PRNG, using a seed value specified
 * in <b>seed</b>.  This value must be CRYPTO_FAST_RNG_SEED_LEN bytes
 * long.
 *
 * Note that this object is NOT thread-safe.  If you need a thread-safe
 * prng, you should probably look at get_thread_fast_rng().  Alternatively,
 * use crypto_rand(), wrap this in a mutex.
 **/
crypto_fast_rng_t *
crypto_fast_rng_new_from_seed(const uint8_t *seed)
{
  unsigned inherit = INHERIT_RES_KEEP;
  /* We try to allocate this object as securely as we can, to avoid
   * having it get dumped, swapped, or shared after fork.
   */
  crypto_fast_rng_t *result = tor_mmap_anonymous(sizeof(*result),
                                ANONMAP_PRIVATE | ANONMAP_NOINHERIT,
                                &inherit);
  memcpy(result->buf.seed, seed, SEED_LEN);
  /* Causes an immediate refill once the user asks for data. */
  result->bytes_left = 0;
  result->n_till_reseed = RESEED_AFTER;
#ifdef CHECK_PID
  if (inherit == INHERIT_RES_KEEP) {
    /* This value will neither be dropped nor zeroed after fork, so we need to
     * check our pid to make sure we are not sharing it across a fork.  This
     * can be expensive if the pid value isn't cached, sadly.
     */
    result->owner = getpid();
  }
#elif defined(_WIN32)
  /* Windows can't fork(), so there's no need to noinherit. */
#else
  /* We decided above that noinherit would always do _something_. Assert here
   * that we were correct. */
  tor_assertf(inherit != INHERIT_RES_KEEP,
              "We failed to create a non-inheritable memory region, even "
              "though we believed such a failure to be impossible! This is "
              "probably a bug in Tor support for your platform; please report "
              "it.");
#endif /* defined(CHECK_PID) || ... */
  return result;
}

#ifdef TOR_UNIT_TESTS
/**
 * Unit tests only: prevent a crypto_fast_rng_t from ever mixing in more
 * entropy.
 */
void
crypto_fast_rng_disable_reseed(crypto_fast_rng_t *rng)
{
  rng->n_till_reseed = -1;
}
#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Helper: create a crypto_cipher_t object from SEED_LEN bytes of
 * input.  The first KEY_LEN bytes are used as the stream cipher's key,
 * and the remaining CIPHER_IV_LEN bytes are used as its IV.
 **/
static inline crypto_cipher_t *
cipher_from_seed(const uint8_t *seed)
{
  return crypto_cipher_new_with_iv_and_bits(seed, seed+KEY_LEN, KEY_BITS);
}

/**
 * Helper: mix additional entropy into <b>rng</b> by using our XOF to mix the
 * old value for the seed with some additional bytes from
 * crypto_strongest_rand().
 **/
static void
crypto_fast_rng_add_entopy(crypto_fast_rng_t *rng)
{
  crypto_xof_t *xof = crypto_xof_new();
  crypto_xof_add_bytes(xof, rng->buf.seed, SEED_LEN);
  {
    uint8_t seedbuf[SEED_LEN];
    crypto_strongest_rand(seedbuf, SEED_LEN);
    crypto_xof_add_bytes(xof, seedbuf, SEED_LEN);
    memwipe(seedbuf, 0, SEED_LEN);
  }
  crypto_xof_squeeze_bytes(xof, rng->buf.seed, SEED_LEN);
  crypto_xof_free(xof);
}

/**
 * Helper: refill the seed bytes and output buffer of <b>rng</b>, using
 * the input seed bytes as input (key and IV) for the stream cipher.
 *
 * If the n_till_reseed counter has reached zero, mix more random bytes into
 * the seed before refilling the buffer.
 **/
static void
crypto_fast_rng_refill(crypto_fast_rng_t *rng)
{
  rng->n_till_reseed--;
  if (rng->n_till_reseed == 0) {
    /* It's time to reseed the RNG. */
    crypto_fast_rng_add_entopy(rng);
    rng->n_till_reseed = RESEED_AFTER;
  } else if (rng->n_till_reseed < 0) {
#ifdef TOR_UNIT_TESTS
    /* Reseeding is disabled for testing; never do it on this prng. */
    rng->n_till_reseed = -1;
#else
    /* If testing is disabled, this shouldn't be able to become negative. */
    tor_assert_unreached();
#endif /* defined(TOR_UNIT_TESTS) */
  }
  /* Now fill rng->buf with output from our stream cipher, initialized from
   * that seed value. */
  crypto_cipher_t *c = cipher_from_seed(rng->buf.seed);
  memset(&rng->buf, 0, sizeof(rng->buf));
  crypto_cipher_crypt_inplace(c, (char*)&rng->buf, sizeof(rng->buf));
  crypto_cipher_free(c);

  rng->bytes_left = sizeof(rng->buf.bytes);
}

/**
 * Release all storage held by <b>rng</b>.
 **/
void
crypto_fast_rng_free_(crypto_fast_rng_t *rng)
{
  if (!rng)
    return;
  memwipe(rng, 0, sizeof(*rng));
  tor_munmap_anonymous(rng, sizeof(*rng));
}

/**
 * Helper: extract bytes from the PRNG, refilling it as necessary. Does not
 * optimize the case when the user has asked for a huge output.
 **/
static void
crypto_fast_rng_getbytes_impl(crypto_fast_rng_t *rng, uint8_t *out,
                              const size_t n)
{
#ifdef CHECK_PID
  if (rng->owner) {
    /* Note that we only need to do this check when we have owner set: that
     * is, when our attempt to block inheriting failed, and the result was
     * INHERIT_RES_KEEP.
     *
     * If the result was INHERIT_RES_DROP, then any attempt to access the rng
     * memory after forking will crash.
     *
     * If the result was INHERIT_RES_ZERO, then forking will set the bytes_left
     * and n_till_reseed fields to zero.  This function will call
     * crypto_fast_rng_refill(), which will in turn reseed the PRNG.
     *
     * So we only need to do this test in the case when mmap_anonymous()
     * returned INHERIT_KEEP.  We avoid doing it needlessly, since getpid() is
     * often a system call, and that can be slow.
     */
    tor_assert(rng->owner == getpid());
  }
#endif /* defined(CHECK_PID) */

  size_t bytes_to_yield = n;

  while (bytes_to_yield) {
    if (rng->bytes_left == 0)
      crypto_fast_rng_refill(rng);

    const size_t to_copy = MIN(rng->bytes_left, bytes_to_yield);

    tor_assert(sizeof(rng->buf.bytes) >= rng->bytes_left);
    uint8_t *copy_from = rng->buf.bytes +
      (sizeof(rng->buf.bytes) - rng->bytes_left);
    memcpy(out, copy_from, to_copy);
    memset(copy_from, 0, to_copy);

    out += to_copy;
    bytes_to_yield -= to_copy;
    rng->bytes_left -= to_copy;
  }
}

/**
 * Extract <b>n</b> bytes from <b>rng</b> into the buffer at <b>out</b>.
 **/
void
crypto_fast_rng_getbytes(crypto_fast_rng_t *rng, uint8_t *out, size_t n)
{
  if (PREDICT_UNLIKELY(n > BUFLEN)) {
    /* The user has asked for a lot of output; generate it from a stream
     * cipher seeded by the PRNG rather than by pulling it out of the PRNG
     * directly.
     */
    uint8_t seed[SEED_LEN];
    crypto_fast_rng_getbytes_impl(rng, seed, SEED_LEN);
    crypto_cipher_t *c = cipher_from_seed(seed);
    memset(out, 0, n);
    crypto_cipher_crypt_inplace(c, (char*)out, n);
    crypto_cipher_free(c);
    memwipe(seed, 0, sizeof(seed));
    return;
  }

  crypto_fast_rng_getbytes_impl(rng, out, n);
}

#if defined(TOR_UNIT_TESTS)
/** for white-box testing: return the number of bytes that are returned from
 * the user for each invocation of the stream cipher in this RNG. */
size_t
crypto_fast_rng_get_bytes_used_per_stream(void)
{
  return BUFLEN;
}
#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Thread-local instance for our fast RNG.
 **/
static tor_threadlocal_t thread_rng;

/**
 * Return a per-thread fast RNG, initializing it if necessary.
 *
 * You do not need to free this yourself.
 *
 * It is NOT safe to share this value across threads.
 **/
crypto_fast_rng_t *
get_thread_fast_rng(void)
{
  crypto_fast_rng_t *rng = tor_threadlocal_get(&thread_rng);

  if (PREDICT_UNLIKELY(rng == NULL)) {
    rng = crypto_fast_rng_new();
    tor_threadlocal_set(&thread_rng, rng);
  }

  return rng;
}

/**
 * Used when a thread is exiting: free the per-thread fast RNG if needed.
 * Invoked from the crypto subsystem's thread-cleanup code.
 **/
void
destroy_thread_fast_rng(void)
{
  crypto_fast_rng_t *rng = tor_threadlocal_get(&thread_rng);
  if (!rng)
    return;
  crypto_fast_rng_free(rng);
  tor_threadlocal_set(&thread_rng, NULL);
}

#ifdef TOR_UNIT_TESTS
/**
 * Replace the current thread's rng with <b>rng</b>. For use by the
 * unit tests only.  Returns the previous thread rng.
 **/
crypto_fast_rng_t *
crypto_replace_thread_fast_rng(crypto_fast_rng_t *rng)
{
  crypto_fast_rng_t *old_rng =  tor_threadlocal_get(&thread_rng);
  tor_threadlocal_set(&thread_rng, rng);
  return old_rng;
}
#endif /* defined(TOR_UNIT_TESTS) */

/**
 * Initialize the global thread-local key that will be used to keep track
 * of per-thread fast RNG instances.  Called from the crypto subsystem's
 * initialization code.
 **/
void
crypto_rand_fast_init(void)
{
  tor_threadlocal_init(&thread_rng);
}

/**
 * Initialize the global thread-local key that will be used to keep track
 * of per-thread fast RNG instances.  Called from the crypto subsystem's
 * shutdown code.
 **/
void
crypto_rand_fast_shutdown(void)
{
  destroy_thread_fast_rng();
  tor_threadlocal_destroy(&thread_rng);
}
