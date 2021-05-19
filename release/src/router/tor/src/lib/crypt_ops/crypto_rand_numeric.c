/**
 * \file crypto_rand_numeric.c
 *
 * \brief Functions for retrieving uniformly distributed numbers
 *   from our PRNGs.
 **/

#include "lib/crypt_ops/crypto_rand.h"
#include "lib/log/util_bug.h"

/**
 * Implementation macro: yields code that returns a uniform unbiased
 * random number between 0 and limit.  "type" is the type of the number to
 * return; "maxval" is the largest possible value of "type"; and "fill_stmt"
 * is a code snippet that fills an object named "val" with random bits.
 **/
#define IMPLEMENT_RAND_UNSIGNED(type, maxval, limit, fill_stmt)         \
  do {                                                                  \
    type val;                                                           \
    type cutoff;                                                        \
    tor_assert((limit) > 0);                                            \
                                                                        \
    /* We ignore any values that are >= 'cutoff,' to avoid biasing */   \
    /* the distribution with clipping at the upper end of the type's */ \
    /* range. */                                                        \
    cutoff = (maxval) - ((maxval)%(limit));                             \
    while (1) {                                                         \
      fill_stmt;                                                        \
      if (val < cutoff)                                                 \
        return val % (limit);                                           \
    }                                                                   \
  } while (0)

/**
 * Return a pseudorandom integer chosen uniformly from the values between 0
 * and <b>limit</b>-1 inclusive. limit must be strictly greater than 0, and
 * less than UINT_MAX. */
unsigned
crypto_rand_uint(unsigned limit)
{
  tor_assert(limit < UINT_MAX);
  IMPLEMENT_RAND_UNSIGNED(unsigned, UINT_MAX, limit,
                          crypto_rand((char*)&val, sizeof(val)));
}

/**
 * Return a pseudorandom integer, chosen uniformly from the values
 * between 0 and <b>max</b>-1 inclusive.  <b>max</b> must be between 1 and
 * INT_MAX+1, inclusive.
 */
int
crypto_rand_int(unsigned int max)
{
  /* We can't use IMPLEMENT_RAND_UNSIGNED directly, since we're trying
   * to return a signed type. Instead we make sure that the range is
   * reasonable for a nonnegative int, use crypto_rand_uint(), and cast.
   */
  tor_assert(max <= ((unsigned int)INT_MAX)+1);

  return (int)crypto_rand_uint(max);
}

/**
 * Return a pseudorandom integer, chosen uniformly from the values i such
 * that min <= i < max.
 *
 * <b>min</b> MUST be in range [0, <b>max</b>).
 * <b>max</b> MUST be in range (min, INT_MAX].
 **/
int
crypto_rand_int_range(unsigned int min, unsigned int max)
{
  tor_assert(min < max);
  tor_assert(max <= INT_MAX);

  /* The overflow is avoided here because crypto_rand_int() returns a value
   * between 0 and (max - min) inclusive. */
  return min + crypto_rand_int(max - min);
}

/**
 * As crypto_rand_int_range, but supports uint64_t.
 **/
uint64_t
crypto_rand_uint64_range(uint64_t min, uint64_t max)
{
  tor_assert(min < max);
  return min + crypto_rand_uint64(max - min);
}

/**
 * As crypto_rand_int_range, but supports time_t.
 **/
time_t
crypto_rand_time_range(time_t min, time_t max)
{
  tor_assert(min < max);
  return min + (time_t)crypto_rand_uint64(max - min);
}

/**
 * Return a pseudorandom 64-bit integer, chosen uniformly from the values
 * between 0 and <b>max</b>-1 inclusive.
 **/
uint64_t
crypto_rand_uint64(uint64_t max)
{
  tor_assert(max < UINT64_MAX);
  IMPLEMENT_RAND_UNSIGNED(uint64_t, UINT64_MAX, max,
                          crypto_rand((char*)&val, sizeof(val)));
}

#if SIZEOF_INT == 4
#define UINT_MAX_AS_DOUBLE 4294967296.0
#elif SIZEOF_INT == 8
#define UINT_MAX_AS_DOUBLE 1.8446744073709552e+19
#else
#error SIZEOF_INT is neither 4 nor 8
#endif /* SIZEOF_INT == 4 || ... */

/**
 * Return a pseudorandom double d, chosen uniformly from the range
 * 0.0 <= d < 1.0.
 **/
double
crypto_rand_double(void)
{
  /* We just use an unsigned int here; we don't really care about getting
   * more than 32 bits of resolution */
  unsigned int u;
  crypto_rand((char*)&u, sizeof(u));
  return ((double)u) / UINT_MAX_AS_DOUBLE;
}

/**
 * As crypto_rand_uint, but extract the result from a crypto_fast_rng_t
 */
unsigned
crypto_fast_rng_get_uint(crypto_fast_rng_t *rng, unsigned limit)
{
  tor_assert(limit < UINT_MAX);
  IMPLEMENT_RAND_UNSIGNED(unsigned, UINT_MAX, limit,
                  crypto_fast_rng_getbytes(rng, (void*)&val, sizeof(val)));
}

/**
 * As crypto_rand_uint64, but extract the result from a crypto_fast_rng_t.
 */
uint64_t
crypto_fast_rng_get_uint64(crypto_fast_rng_t *rng, uint64_t limit)
{
  tor_assert(limit < UINT64_MAX);
  IMPLEMENT_RAND_UNSIGNED(uint64_t, UINT64_MAX, limit,
                  crypto_fast_rng_getbytes(rng, (void*)&val, sizeof(val)));
}

/**
 * As crypto_rand_u32, but extract the result from a crypto_fast_rng_t.
 */
uint32_t
crypto_fast_rng_get_u32(crypto_fast_rng_t *rng)
{
  uint32_t val;
  crypto_fast_rng_getbytes(rng, (void*)&val, sizeof(val));
  return val;
}

/**
 * As crypto_rand_uint64_range(), but extract the result from a
 * crypto_fast_rng_t.
 */
uint64_t
crypto_fast_rng_uint64_range(crypto_fast_rng_t *rng,
                             uint64_t min, uint64_t max)
{
  /* Handle corrupted input */
  if (BUG(min >= max)) {
    return min;
  }

  return min + crypto_fast_rng_get_uint64(rng, max - min);
}

/**
 * As crypto_rand_get_double() but extract the result from a crypto_fast_rng_t.
 */
double
crypto_fast_rng_get_double(crypto_fast_rng_t *rng)
{
  unsigned int u;
  crypto_fast_rng_getbytes(rng, (void*)&u, sizeof(u));
  return ((double)u) / UINT_MAX_AS_DOUBLE;
}

