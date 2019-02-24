/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_time.h
 *
 * \brief Functions and types for monotonic times.
 *
 * monotime_* functions try to provide a high-resolution monotonic timer with
 * something the best resolution the system provides.  monotime_coarse_*
 * functions run faster (if the operating system gives us a way to do that)
 * but produce a less accurate timer: accuracy will probably be on the order
 * of tens of milliseconds.
 */

#ifndef TOR_COMPAT_TIME_H
#define TOR_COMPAT_TIME_H

#include "orconfig.h"
#include "lib/cc/torint.h"

#include "lib/wallclock/tor_gettimeofday.h"

#ifdef _WIN32
#undef HAVE_CLOCK_GETTIME
#endif

#if defined(HAVE_CLOCK_GETTIME)
/* to ensure definition of CLOCK_MONOTONIC_COARSE if it's there */
#include <time.h>
#endif

#if !defined(HAVE_STRUCT_TIMEVAL_TV_SEC)
/** Implementation of timeval for platforms that don't have it. */
struct timeval {
  time_t tv_sec;
  unsigned int tv_usec;
};
#endif /* !defined(HAVE_STRUCT_TIMEVAL_TV_SEC) */

/** Represents a monotonic timer in a platform-dependent way. */
typedef struct monotime_t {
#ifdef __APPLE__
  /* On apple, there is a 64-bit counter whose precision we must look up. */
  uint64_t abstime_;
#elif defined(HAVE_CLOCK_GETTIME)
  /* It sure would be nice to use clock_gettime(). Posix is a nice thing. */
  struct timespec ts_;
#elif defined (_WIN32)
  /* On Windows, there is a 64-bit counter whose precision we must look up. */
  int64_t pcount_;
#else
#define MONOTIME_USING_GETTIMEOFDAY
  /* Otherwise, we will be stuck using gettimeofday. */
  struct timeval tv_;
#endif /* defined(__APPLE__) || ... */
} monotime_t;

#if defined(CLOCK_MONOTONIC_COARSE) && \
  defined(HAVE_CLOCK_GETTIME)
#define MONOTIME_COARSE_FN_IS_DIFFERENT
#define monotime_coarse_t monotime_t
#elif defined(_WIN32)
#define MONOTIME_COARSE_FN_IS_DIFFERENT
#define MONOTIME_COARSE_TYPE_IS_DIFFERENT
/** Represents a coarse monotonic time in a platform-independent way. */
typedef struct monotime_coarse_t {
  uint64_t tick_count_;
} monotime_coarse_t;
#elif defined(__APPLE__) && defined(HAVE_MACH_APPROXIMATE_TIME)
#define MONOTIME_COARSE_FN_IS_DIFFERENT
#define monotime_coarse_t monotime_t
#else
#define monotime_coarse_t monotime_t
#endif /* defined(CLOCK_MONOTONIC_COARSE) && ... || ... */

/**
 * Initialize the timing subsystem. This function is idempotent.
 */
void monotime_init(void);
/**
 * Set <b>out</b> to the current time.
 */
void monotime_get(monotime_t *out);
/**
 * Return the number of nanoseconds between <b>start</b> and <b>end</b>.
 */
int64_t monotime_diff_nsec(const monotime_t *start, const monotime_t *end);
/**
 * Return the number of microseconds between <b>start</b> and <b>end</b>.
 */
int64_t monotime_diff_usec(const monotime_t *start, const monotime_t *end);
/**
 * Return the number of milliseconds between <b>start</b> and <b>end</b>.
 */
int64_t monotime_diff_msec(const monotime_t *start, const monotime_t *end);
/**
 * Return the number of nanoseconds since the timer system was initialized.
 */
uint64_t monotime_absolute_nsec(void);
/**
 * Return the number of microseconds since the timer system was initialized.
 */
uint64_t monotime_absolute_usec(void);
/**
 * Return the number of milliseconds since the timer system was initialized.
 */
uint64_t monotime_absolute_msec(void);

/**
 * Set <b>out</b> to zero.
 */
void monotime_zero(monotime_t *out);
/**
 * Return true iff <b>out</b> is zero
 */
int monotime_is_zero(const monotime_t *out);

/**
 * Set <b>out</b> to N milliseconds after <b>val</b>.
 */
/* XXXX We should add a more generic function here if we ever need to */
void monotime_add_msec(monotime_t *out, const monotime_t *val, uint32_t msec);

#if defined(MONOTIME_COARSE_FN_IS_DIFFERENT)
/**
 * Set <b>out</b> to the current coarse time.
 */
void monotime_coarse_get(monotime_coarse_t *out);
uint64_t monotime_coarse_absolute_nsec(void);
uint64_t monotime_coarse_absolute_usec(void);
uint64_t monotime_coarse_absolute_msec(void);
#else /* !(defined(MONOTIME_COARSE_FN_IS_DIFFERENT)) */
#define monotime_coarse_get monotime_get
#define monotime_coarse_absolute_nsec monotime_absolute_nsec
#define monotime_coarse_absolute_usec monotime_absolute_usec
#define monotime_coarse_absolute_msec monotime_absolute_msec
#endif /* defined(MONOTIME_COARSE_FN_IS_DIFFERENT) */

/**
 * Return a "timestamp" approximation for a coarse monotonic timer.
 * This timestamp is meant to be fast to calculate and easy to
 * compare, and have a unit of something roughly around 1 msec.
 *
 * It will wrap over from time to time.
 *
 * It has no defined zero point.
 */
uint32_t monotime_coarse_to_stamp(const monotime_coarse_t *t);
/**
 * Convert a difference, expressed in the units of monotime_coarse_to_stamp,
 * into an approximate number of milliseconds.
 */
uint64_t monotime_coarse_stamp_units_to_approx_msec(uint64_t units);
uint64_t monotime_msec_to_approx_coarse_stamp_units(uint64_t msec);
uint32_t monotime_coarse_get_stamp(void);

#if defined(MONOTIME_COARSE_TYPE_IS_DIFFERENT)
int64_t monotime_coarse_diff_nsec(const monotime_coarse_t *start,
    const monotime_coarse_t *end);
int64_t monotime_coarse_diff_usec(const monotime_coarse_t *start,
    const monotime_coarse_t *end);
int64_t monotime_coarse_diff_msec(const monotime_coarse_t *start,
    const monotime_coarse_t *end);
void monotime_coarse_zero(monotime_coarse_t *out);
int monotime_coarse_is_zero(const monotime_coarse_t *val);
void monotime_coarse_add_msec(monotime_coarse_t *out,
                              const monotime_coarse_t *val, uint32_t msec);
#else /* !(defined(MONOTIME_COARSE_TYPE_IS_DIFFERENT)) */
#define monotime_coarse_diff_nsec monotime_diff_nsec
#define monotime_coarse_diff_usec monotime_diff_usec
#define monotime_coarse_diff_msec monotime_diff_msec
#define monotime_coarse_zero monotime_zero
#define monotime_coarse_is_zero monotime_is_zero
#define monotime_coarse_add_msec monotime_add_msec
#endif /* defined(MONOTIME_COARSE_TYPE_IS_DIFFERENT) */

/**
 * As monotime_coarse_diff_msec, but avoid 64-bit division.
 *
 * Requires that the difference fit into an int32_t; not for use with
 * large time differences.
 */
int32_t monotime_coarse_diff_msec32_(const monotime_coarse_t *start,
                                     const monotime_coarse_t *end);

/**
 * As monotime_coarse_diff_msec, but avoid 64-bit division if it is expensive.
 *
 * Requires that the difference fit into an int32_t; not for use with
 * large time differences.
 */
static inline int32_t
monotime_coarse_diff_msec32(const monotime_coarse_t *start,
                            const monotime_coarse_t *end)
{
#if SIZEOF_VOID_P == 8
  // on a 64-bit platform, let's assume 64/64 division is cheap.
  return (int32_t) monotime_coarse_diff_msec(start, end);
#else
#define USING_32BIT_MSEC_HACK
  return monotime_coarse_diff_msec32_(start, end);
#endif
}

#ifdef TOR_UNIT_TESTS
void tor_sleep_msec(int msec);

void monotime_enable_test_mocking(void);
void monotime_disable_test_mocking(void);
void monotime_set_mock_time_nsec(int64_t);
#if defined(MONOTIME_COARSE_FN_IS_DIFFERENT)
void monotime_coarse_set_mock_time_nsec(int64_t);
#else
#define monotime_coarse_set_mock_time_nsec monotime_set_mock_time_nsec
#endif
#endif /* defined(TOR_UNIT_TESTS) */

#ifdef COMPAT_TIME_PRIVATE
#if defined(_WIN32) || defined(TOR_UNIT_TESTS)
STATIC int64_t ratchet_performance_counter(int64_t count_raw);
STATIC int64_t ratchet_coarse_performance_counter(int64_t count_raw);
#endif
#if defined(MONOTIME_USING_GETTIMEOFDAY) || defined(TOR_UNIT_TESTS)
STATIC void ratchet_timeval(const struct timeval *timeval_raw,
                            struct timeval *out);
#endif
#ifdef TOR_UNIT_TESTS
void monotime_reset_ratchets_for_testing(void);
#endif
#endif /* defined(COMPAT_TIME_PRIVATE) */

#endif /* !defined(TOR_COMPAT_TIME_H) */
