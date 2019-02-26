/* Copyright (c) 2018-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_bwmgt.c
 * \brief tests for bandwidth management / token bucket functions
 */

#define TOKEN_BUCKET_PRIVATE

#include "core/or/or.h"
#include "test/test.h"

#include "lib/evloop/token_bucket.h"

// an imaginary time, in timestamp units. Chosen so it will roll over.
static const uint32_t START_TS = UINT32_MAX-10;
static const int32_t KB = 1024;
static const uint32_t GB = (UINT64_C(1) << 30);

static void
test_bwmgt_token_buf_init(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;

  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);
  // Burst is correct
  tt_uint_op(b.cfg.burst, OP_EQ, 64*KB);
  // Rate is correct, within 1 percent.
  {
    uint32_t ticks_per_sec =
      (uint32_t) monotime_msec_to_approx_coarse_stamp_units(1000);
    uint32_t rate_per_sec = (b.cfg.rate * ticks_per_sec / TICKS_PER_STEP);

    tt_uint_op(rate_per_sec, OP_GT, 16*KB-160);
    tt_uint_op(rate_per_sec, OP_LT, 16*KB+160);
  }
  // Bucket starts out full:
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ, START_TS);
  tt_int_op(b.read_bucket.bucket, OP_EQ, 64*KB);

 done:
  ;
}

static void
test_bwmgt_token_buf_adjust(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;

  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);

  uint32_t rate_orig = b.cfg.rate;
  // Increasing burst
  token_bucket_rw_adjust(&b, 16*KB, 128*KB);
  tt_uint_op(b.cfg.rate, OP_EQ, rate_orig);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 64*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 128*KB);

  // Decreasing burst but staying above bucket
  token_bucket_rw_adjust(&b, 16*KB, 96*KB);
  tt_uint_op(b.cfg.rate, OP_EQ, rate_orig);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 64*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 96*KB);

  // Decreasing burst below bucket,
  token_bucket_rw_adjust(&b, 16*KB, 48*KB);
  tt_uint_op(b.cfg.rate, OP_EQ, rate_orig);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 48*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 48*KB);

  // Changing rate.
  token_bucket_rw_adjust(&b, 32*KB, 48*KB);
  tt_uint_op(b.cfg.rate, OP_GE, rate_orig*2 - 10);
  tt_uint_op(b.cfg.rate, OP_LE, rate_orig*2 + 10);
  tt_uint_op(b.read_bucket.bucket, OP_EQ, 48*KB);
  tt_uint_op(b.cfg.burst, OP_EQ, 48*KB);

 done:
  ;
}

static void
test_bwmgt_token_buf_dec(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);

  // full-to-not-full.
  tt_int_op(0, OP_EQ, token_bucket_rw_dec_read(&b, KB));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 63*KB);

  // Full to almost-not-full
  tt_int_op(0, OP_EQ, token_bucket_rw_dec_read(&b, 63*KB - 1));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 1);

  // almost-not-full to empty.
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, 1));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 0);

  // reset bucket, try full-to-empty
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, 64*KB));
  tt_int_op(b.read_bucket.bucket, OP_EQ, 0);

  // reset bucket, try underflow.
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, 64*KB + 1));
  tt_int_op(b.read_bucket.bucket, OP_EQ, -1);

  // A second underflow does not make the bucket empty.
  tt_int_op(0, OP_EQ, token_bucket_rw_dec_read(&b, 1000));
  tt_int_op(b.read_bucket.bucket, OP_EQ, -1001);

 done:
  ;
}

static void
test_bwmgt_token_buf_refill(void *arg)
{
  (void)arg;
  token_bucket_rw_t b;
  const uint32_t BW_SEC =
    (uint32_t)monotime_msec_to_approx_coarse_stamp_units(1000);
  token_bucket_rw_init(&b, 16*KB, 64*KB, START_TS);

  /* Make the buffer much emptier, then let one second elapse. */
  token_bucket_rw_dec_read(&b, 48*KB);
  tt_int_op(b.read_bucket.bucket, OP_EQ, 16*KB);
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC));
  tt_int_op(b.read_bucket.bucket, OP_GT, 32*KB - 300);
  tt_int_op(b.read_bucket.bucket, OP_LT, 32*KB + 300);

  /* Another half second. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*3/2));
  tt_int_op(b.read_bucket.bucket, OP_GT, 40*KB - 400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 40*KB + 400);
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ, START_TS + BW_SEC*3/2);

  /* No time: nothing happens. */
  {
    const uint32_t bucket_orig = b.read_bucket.bucket;
    tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*3/2));
    tt_int_op(b.read_bucket.bucket, OP_EQ, bucket_orig);
  }

  /* Another 30 seconds: fill the bucket. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*30));
  tt_int_op(b.read_bucket.bucket, OP_EQ, b.cfg.burst);
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ,
             START_TS + BW_SEC*3/2 + BW_SEC*30);

  /* Another 30 seconds: nothing happens. */
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*60));
  tt_int_op(b.read_bucket.bucket, OP_EQ, b.cfg.burst);
  tt_uint_op(b.last_refilled_at_timestamp, OP_EQ,
             START_TS + BW_SEC*3/2 + BW_SEC*60);

  /* Empty the bucket, let two seconds pass, and make sure that a refill is
   * noticed. */
  tt_int_op(1, OP_EQ, token_bucket_rw_dec_read(&b, b.cfg.burst));
  tt_int_op(0, OP_EQ, b.read_bucket.bucket);
  tt_int_op(1, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*61));
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b,
                                     START_TS + BW_SEC*3/2 + BW_SEC*62));
  tt_int_op(b.read_bucket.bucket, OP_GT, 32*KB-400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 32*KB+400);

  /* Underflow the bucket, make sure we detect when it has tokens again. */
  tt_int_op(1, OP_EQ,
            token_bucket_rw_dec_read(&b, b.read_bucket.bucket+16*KB));
  tt_int_op(-16*KB, OP_EQ, b.read_bucket.bucket);
  // half a second passes...
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*64));
  tt_int_op(b.read_bucket.bucket, OP_GT, -8*KB-300);
  tt_int_op(b.read_bucket.bucket, OP_LT, -8*KB+300);
  // a second passes
  tt_int_op(1, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*65));
  tt_int_op(b.read_bucket.bucket, OP_GT, 8*KB-400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 8*KB+400);

  // We step a second backwards, and nothing happens.
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, START_TS + BW_SEC*64));
  tt_int_op(b.read_bucket.bucket, OP_GT, 8*KB-400);
  tt_int_op(b.read_bucket.bucket, OP_LT, 8*KB+400);

  // A ridiculous amount of time passes.
  tt_int_op(0, OP_EQ, token_bucket_rw_refill(&b, INT32_MAX));
  tt_int_op(b.read_bucket.bucket, OP_EQ, b.cfg.burst);

 done:
  ;
}

/* Test some helper functions we use within the token bucket interface. */
static void
test_bwmgt_token_buf_helpers(void *arg)
{
  uint32_t ret;

  (void) arg;

  /* The returned value will be OS specific but in any case, it should be
   * greater than 1 since we are passing 1GB/sec rate. */
  ret = rate_per_sec_to_rate_per_step(1 * GB);
  tt_u64_op(ret, OP_GT, 1);

  /* We default to 1 in case rate is 0. */
  ret = rate_per_sec_to_rate_per_step(0);
  tt_u64_op(ret, OP_EQ, 1);

 done:
  ;
}

#define BWMGT(name)                                          \
  { #name, test_bwmgt_ ## name , 0, NULL, NULL }

struct testcase_t bwmgt_tests[] = {
  BWMGT(token_buf_init),
  BWMGT(token_buf_adjust),
  BWMGT(token_buf_dec),
  BWMGT(token_buf_refill),
  BWMGT(token_buf_helpers),
  END_OF_TESTCASES
};
