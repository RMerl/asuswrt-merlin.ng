/* Copyright (c) 2018-2020, The Tor Project, Inc. */
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
static const uint32_t START_TS = UINT32_MAX - 1000;
static const uint32_t RATE = 10;
static const uint32_t BURST = 50;

static void
test_token_bucket_ctr_init(void *arg)
{
  (void) arg;
  token_bucket_ctr_t tb;

  token_bucket_ctr_init(&tb, RATE, BURST, START_TS);
  tt_uint_op(tb.cfg.rate, OP_EQ, RATE);
  tt_uint_op(tb.cfg.burst, OP_EQ, BURST);
  tt_uint_op(tb.last_refilled_at_timestamp, OP_EQ, START_TS);
  tt_int_op(tb.counter.bucket, OP_EQ, BURST);

 done:
  ;
}

static void
test_token_bucket_ctr_adjust(void *arg)
{
  (void) arg;
  token_bucket_ctr_t tb;

  token_bucket_ctr_init(&tb, RATE, BURST, START_TS);

  /* Increase burst. */
  token_bucket_ctr_adjust(&tb, RATE, BURST * 2);
  tt_uint_op(tb.cfg.rate, OP_EQ, RATE);
  tt_uint_op(tb.counter.bucket, OP_EQ, BURST);
  tt_uint_op(tb.cfg.burst, OP_EQ, BURST * 2);

  /* Decrease burst but still above bucket value. */
  token_bucket_ctr_adjust(&tb, RATE, BURST + 10);
  tt_uint_op(tb.cfg.rate, OP_EQ, RATE);
  tt_uint_op(tb.counter.bucket, OP_EQ, BURST);
  tt_uint_op(tb.cfg.burst, OP_EQ, BURST + 10);

  /* Decrease burst below bucket value. */
  token_bucket_ctr_adjust(&tb, RATE, BURST - 1);
  tt_uint_op(tb.cfg.rate, OP_EQ, RATE);
  tt_uint_op(tb.counter.bucket, OP_EQ, BURST - 1);
  tt_uint_op(tb.cfg.burst, OP_EQ, BURST - 1);

  /* Change rate. */
  token_bucket_ctr_adjust(&tb, RATE * 2, BURST);
  tt_uint_op(tb.cfg.rate, OP_EQ, RATE * 2);
  tt_uint_op(tb.counter.bucket, OP_EQ, BURST - 1);
  tt_uint_op(tb.cfg.burst, OP_EQ, BURST);

 done:
  ;
}

static void
test_token_bucket_ctr_dec(void *arg)
{
  (void) arg;
  token_bucket_ctr_t tb;

  token_bucket_ctr_init(&tb, RATE, BURST, START_TS);

  /* Simple decrement by one. */
  tt_uint_op(0, OP_EQ, token_bucket_ctr_dec(&tb, 1));
  tt_uint_op(tb.counter.bucket, OP_EQ, BURST - 1);

  /* Down to 0. Becomes empty. */
  tt_uint_op(true, OP_EQ, token_bucket_ctr_dec(&tb, BURST - 1));
  tt_uint_op(tb.counter.bucket, OP_EQ, 0);

  /* Reset and try to underflow. */
  token_bucket_ctr_init(&tb, RATE, BURST, START_TS);
  tt_uint_op(true, OP_EQ, token_bucket_ctr_dec(&tb, BURST + 1));
  tt_int_op(tb.counter.bucket, OP_EQ, -1);

  /* Keep underflowing shouldn't flag the bucket as empty. */
  tt_uint_op(false, OP_EQ, token_bucket_ctr_dec(&tb, BURST));
  tt_int_op(tb.counter.bucket, OP_EQ, - (int32_t) (BURST + 1));

 done:
  ;
}

static void
test_token_bucket_ctr_refill(void *arg)
{
  (void) arg;
  token_bucket_ctr_t tb;

  token_bucket_ctr_init(&tb, RATE, BURST, START_TS);

  /* Reduce of half the bucket and let a single second go before refill. */
  token_bucket_ctr_dec(&tb, BURST / 2);
  tt_int_op(tb.counter.bucket, OP_EQ, BURST / 2);
  token_bucket_ctr_refill(&tb, START_TS + 1);
  tt_int_op(tb.counter.bucket, OP_EQ, (BURST / 2) + RATE);
  tt_int_op(tb.last_refilled_at_timestamp, OP_EQ, START_TS + 1);

  /* No time change, nothing should move. */
  token_bucket_ctr_refill(&tb, START_TS + 1);
  tt_int_op(tb.counter.bucket, OP_EQ, (BURST / 2) + RATE);
  tt_int_op(tb.last_refilled_at_timestamp, OP_EQ, START_TS + 1);

  /* Add 99 seconds, bucket should be back to a full BURST. */
  token_bucket_ctr_refill(&tb, START_TS + 99);
  tt_int_op(tb.counter.bucket, OP_EQ, BURST);
  tt_int_op(tb.last_refilled_at_timestamp, OP_EQ, START_TS + 99);

  /* Empty bucket at once. */
  token_bucket_ctr_dec(&tb, BURST);
  tt_int_op(tb.counter.bucket, OP_EQ, 0);
  /* On second passes. */
  token_bucket_ctr_refill(&tb, START_TS + 100);
  tt_int_op(tb.last_refilled_at_timestamp, OP_EQ, START_TS + 100);
  tt_int_op(tb.counter.bucket, OP_EQ, RATE);
  /* A second second passes. */
  token_bucket_ctr_refill(&tb, START_TS + 101);
  tt_int_op(tb.last_refilled_at_timestamp, OP_EQ, START_TS + 101);
  tt_int_op(tb.counter.bucket, OP_EQ, RATE * 2);

 done:
  ;
}

#define TOKEN_BUCKET(name)                                          \
  { #name, test_token_bucket_ ## name , 0, NULL, NULL }

struct testcase_t token_bucket_tests[] = {
  TOKEN_BUCKET(ctr_init),
  TOKEN_BUCKET(ctr_adjust),
  TOKEN_BUCKET(ctr_dec),
  TOKEN_BUCKET(ctr_refill),
  END_OF_TESTCASES
};
