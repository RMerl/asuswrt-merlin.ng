/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file token_bucket.c
 * \brief Functions to use and manipulate token buckets, used for
 *    rate-limiting on connections and globally.
 *
 * Tor uses these token buckets to keep track of bandwidth usage, and
 * sometimes other things too.
 *
 * There are two layers of abstraction here: "raw" token buckets, in which all
 * the pieces are decoupled, and "read-write" token buckets, which combine all
 * the moving parts into one.
 *
 * Token buckets may become negative.
 **/

#define TOKEN_BUCKET_PRIVATE

#include "lib/evloop/token_bucket.h"
#include "lib/log/util_bug.h"
#include "lib/intmath/cmp.h"
#include "lib/time/compat_time.h"

#include <string.h>

/**
 * Set the <b>rate</b> and <b>burst</b> value in a token_bucket_cfg.
 *
 * Note that the <b>rate</b> value is in arbitrary units, but those units will
 * determine the units of token_bucket_raw_dec(), token_bucket_raw_refill, and
 * so on.
 */
void
token_bucket_cfg_init(token_bucket_cfg_t *cfg,
                      uint32_t rate,
                      uint32_t burst)
{
  tor_assert_nonfatal(rate > 0);
  tor_assert_nonfatal(burst > 0);
  if (burst > TOKEN_BUCKET_MAX_BURST)
    burst = TOKEN_BUCKET_MAX_BURST;

  cfg->rate = rate;
  cfg->burst = burst;
}

/**
 * Initialize a raw token bucket and its associated timestamp to the "full"
 * state, according to <b>cfg</b>.
 */
void
token_bucket_raw_reset(token_bucket_raw_t *bucket,
                       const token_bucket_cfg_t *cfg)
{
  bucket->bucket = cfg->burst;
}

/**
 * Adust a preexisting token bucket to respect the new configuration
 * <b>cfg</b>, by decreasing its current level if needed. */
void
token_bucket_raw_adjust(token_bucket_raw_t *bucket,
                        const token_bucket_cfg_t *cfg)
{
  bucket->bucket = MIN(bucket->bucket, cfg->burst);
}

/**
 * Given an amount of <b>elapsed</b> time units, and a bucket configuration
 * <b>cfg</b>, refill the level of <b>bucket</b> accordingly.  Note that the
 * units of time in <b>elapsed</b> must correspond to those used to set the
 * rate in <b>cfg</b>, or the result will be illogical.
 */
int
token_bucket_raw_refill_steps(token_bucket_raw_t *bucket,
                              const token_bucket_cfg_t *cfg,
                              const uint32_t elapsed)
{
  const int was_empty = (bucket->bucket <= 0);
  /* The casts here prevent an underflow.
   *
   * Note that even if the bucket value is negative, subtracting it from
   * "burst" will still produce a correct result.  If this result is
   * ridiculously high, then the "elapsed > gap / rate" check below
   * should catch it. */
  const size_t gap = ((size_t)cfg->burst) - ((size_t)bucket->bucket);

  if (elapsed > gap / cfg->rate) {
    bucket->bucket = cfg->burst;
  } else {
    bucket->bucket += cfg->rate * elapsed;
  }

  return was_empty && bucket->bucket > 0;
}

/**
 * Decrement a provided bucket by <b>n</b> units.  Note that <b>n</b>
 * must be nonnegative.
 */
int
token_bucket_raw_dec(token_bucket_raw_t *bucket,
                     ssize_t n)
{
  if (BUG(n < 0))
    return 0;
  const int becomes_empty = bucket->bucket > 0 && n >= bucket->bucket;
  bucket->bucket -= n;
  return becomes_empty;
}

/** Convert a rate in bytes per second to a rate in bytes per step */
STATIC uint32_t
rate_per_sec_to_rate_per_step(uint32_t rate)
{
  /*
    The precise calculation we'd want to do is

    (rate / 1000) * to_approximate_msec(TICKS_PER_STEP).  But to minimize
    rounding error, we do it this way instead, and divide last.
  */
  uint64_t units = (uint64_t) rate * TICKS_PER_STEP;
  uint32_t val = (uint32_t)
    (monotime_coarse_stamp_units_to_approx_msec(units) / 1000);
  return val ? val : 1;
}

/**
 * Initialize a token bucket in *<b>bucket</b>, set up to allow <b>rate</b>
 * bytes per second, with a maximum burst of <b>burst</b> bytes. The bucket
 * is created such that <b>now_ts</b> is the current timestamp.  The bucket
 * starts out full.
 */
void
token_bucket_rw_init(token_bucket_rw_t *bucket,
                     uint32_t rate,
                     uint32_t burst,
                     uint32_t now_ts)
{
  memset(bucket, 0, sizeof(token_bucket_rw_t));
  token_bucket_rw_adjust(bucket, rate, burst);
  token_bucket_rw_reset(bucket, now_ts);
}

/**
 * Change the configured rate (in bytes per second) and burst (in bytes)
 * for the token bucket in *<b>bucket</b>.
 */
void
token_bucket_rw_adjust(token_bucket_rw_t *bucket,
                       uint32_t rate,
                       uint32_t burst)
{
  token_bucket_cfg_init(&bucket->cfg,
                        rate_per_sec_to_rate_per_step(rate),
                        burst);
  token_bucket_raw_adjust(&bucket->read_bucket, &bucket->cfg);
  token_bucket_raw_adjust(&bucket->write_bucket, &bucket->cfg);
}

/**
 * Reset <b>bucket</b> to be full, as of timestamp <b>now_ts</b>.
 */
void
token_bucket_rw_reset(token_bucket_rw_t *bucket,
                      uint32_t now_ts)
{
  token_bucket_raw_reset(&bucket->read_bucket, &bucket->cfg);
  token_bucket_raw_reset(&bucket->write_bucket, &bucket->cfg);
  bucket->last_refilled_at_timestamp = now_ts;
}

/**
 * Refill <b>bucket</b> as appropriate, given that the current timestamp
 * is <b>now_ts</b>.
 *
 * Return a bitmask containing TB_READ iff read bucket was empty and became
 * nonempty, and TB_WRITE iff the write bucket was empty and became nonempty.
 */
int
token_bucket_rw_refill(token_bucket_rw_t *bucket,
                       uint32_t now_ts)
{
  const uint32_t elapsed_ticks =
    (now_ts - bucket->last_refilled_at_timestamp);
  if (elapsed_ticks > UINT32_MAX-(300*1000)) {
    /* Either about 48 days have passed since the last refill, or the
     * monotonic clock has somehow moved backwards. (We're looking at you,
     * Windows.).  We accept up to a 5 minute jump backwards as
     * "unremarkable".
     */
    return 0;
  }
  const uint32_t elapsed_steps = elapsed_ticks / TICKS_PER_STEP;

  if (!elapsed_steps) {
    /* Note that if less than one whole step elapsed, we don't advance the
     * time in last_refilled_at. That's intentional: we want to make sure
     * that we add some bytes to it eventually. */
    return 0;
  }

  int flags = 0;
  if (token_bucket_raw_refill_steps(&bucket->read_bucket,
                                    &bucket->cfg, elapsed_steps))
    flags |= TB_READ;
  if (token_bucket_raw_refill_steps(&bucket->write_bucket,
                                    &bucket->cfg, elapsed_steps))
    flags |= TB_WRITE;

  bucket->last_refilled_at_timestamp = now_ts;
  return flags;
}

/**
 * Decrement the read token bucket in <b>bucket</b> by <b>n</b> bytes.
 *
 * Return true if the bucket was nonempty and became empty; return false
 * otherwise.
 */
int
token_bucket_rw_dec_read(token_bucket_rw_t *bucket,
                         ssize_t n)
{
  return token_bucket_raw_dec(&bucket->read_bucket, n);
}

/**
 * Decrement the write token bucket in <b>bucket</b> by <b>n</b> bytes.
 *
 * Return true if the bucket was nonempty and became empty; return false
 * otherwise.
 */
int
token_bucket_rw_dec_write(token_bucket_rw_t *bucket,
                          ssize_t n)
{
  return token_bucket_raw_dec(&bucket->write_bucket, n);
}

/**
 * As token_bucket_rw_dec_read and token_bucket_rw_dec_write, in a single
 * operation.  Return a bitmask of TB_READ and TB_WRITE to indicate
 * which buckets became empty.
 */
int
token_bucket_rw_dec(token_bucket_rw_t *bucket,
                    ssize_t n_read, ssize_t n_written)
{
  int flags = 0;
  if (token_bucket_rw_dec_read(bucket, n_read))
    flags |= TB_READ;
  if (token_bucket_rw_dec_write(bucket, n_written))
    flags |= TB_WRITE;
  return flags;
}

/** Initialize a token bucket in <b>bucket</b>, set up to allow <b>rate</b>
 * per second, with a maximum burst of <b>burst</b>. The bucket is created
 * such that <b>now_ts</b> is the current timestamp. The bucket starts out
 * full. */
void
token_bucket_ctr_init(token_bucket_ctr_t *bucket, uint32_t rate,
                      uint32_t burst, uint32_t now_ts)
{
  memset(bucket, 0, sizeof(token_bucket_ctr_t));
  token_bucket_ctr_adjust(bucket, rate, burst);
  token_bucket_ctr_reset(bucket, now_ts);
}

/** Change the configured rate and burst of the given token bucket object in
 * <b>bucket</b>. */
void
token_bucket_ctr_adjust(token_bucket_ctr_t *bucket, uint32_t rate,
                        uint32_t burst)
{
  token_bucket_cfg_init(&bucket->cfg, rate, burst);
  token_bucket_raw_adjust(&bucket->counter, &bucket->cfg);
}

/** Reset <b>bucket</b> to be full, as of timestamp <b>now_ts</b>. */
void
token_bucket_ctr_reset(token_bucket_ctr_t *bucket, uint32_t now_ts)
{
  token_bucket_raw_reset(&bucket->counter, &bucket->cfg);
  bucket->last_refilled_at_timestamp = now_ts;
}

/** Refill <b>bucket</b> as appropriate, given that the current timestamp is
 * <b>now_ts</b>. */
void
token_bucket_ctr_refill(token_bucket_ctr_t *bucket, uint32_t now_ts)
{
  const uint32_t elapsed_ticks =
    (now_ts - bucket->last_refilled_at_timestamp);
  if (elapsed_ticks > UINT32_MAX-(300*1000)) {
    /* Either about 48 days have passed since the last refill, or the
     * monotonic clock has somehow moved backwards. (We're looking at you,
     * Windows.).  We accept up to a 5 minute jump backwards as
     * "unremarkable".
     */
    return;
  }

  token_bucket_raw_refill_steps(&bucket->counter, &bucket->cfg,
                                elapsed_ticks);
  bucket->last_refilled_at_timestamp = now_ts;
}
