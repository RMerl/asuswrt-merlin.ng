/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dlstatus.c
 * @brief Track status and retry schedule of a downloadable object.
 **/

#define DLSTATUS_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "feature/client/entrynodes.h"
#include "feature/dirclient/dlstatus.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/relay/routermode.h"
#include "lib/crypt_ops/crypto_rand.h"

#include "feature/dirclient/download_status_st.h"

/** Decide which download schedule we want to use based on descriptor type
 * in <b>dls</b> and <b>options</b>.
 *
 * Then, return the initial delay for that download schedule, in seconds.
 *
 * Helper function for download_status_increment_failure(),
 * download_status_reset(), and download_status_increment_attempt(). */
STATIC int
find_dl_min_delay(const download_status_t *dls, const or_options_t *options)
{
  tor_assert(dls);
  tor_assert(options);

  switch (dls->schedule) {
    case DL_SCHED_GENERIC:
      /* Any other directory document */
      if (dir_server_mode(options)) {
        /* A directory authority or directory mirror */
        return options->TestingServerDownloadInitialDelay;
      } else {
        return options->TestingClientDownloadInitialDelay;
      }
    case DL_SCHED_CONSENSUS:
      if (!networkstatus_consensus_can_use_multiple_directories(options)) {
        /* A public relay */
        return options->TestingServerConsensusDownloadInitialDelay;
      } else {
        /* A client or bridge */
        if (networkstatus_consensus_is_bootstrapping(time(NULL))) {
          /* During bootstrapping */
          if (!networkstatus_consensus_can_use_extra_fallbacks(options)) {
            /* A bootstrapping client without extra fallback directories */
            return options->
              ClientBootstrapConsensusAuthorityOnlyDownloadInitialDelay;
          } else if (dls->want_authority) {
            /* A bootstrapping client with extra fallback directories, but
             * connecting to an authority */
            return
             options->ClientBootstrapConsensusAuthorityDownloadInitialDelay;
          } else {
            /* A bootstrapping client connecting to extra fallback directories
             */
            return
              options->ClientBootstrapConsensusFallbackDownloadInitialDelay;
          }
        } else {
          /* A client with a reasonably live consensus, with or without
           * certificates */
          return options->TestingClientConsensusDownloadInitialDelay;
        }
      }
    case DL_SCHED_BRIDGE:
      /* Be conservative here: always return the 'during bootstrap' delay
       * value, so we never delay while trying to fetch descriptors
       * for new bridges. Once we do succeed at fetching a descriptor
       * for our bridge, we will adjust its next_attempt_at based on
       * the longer "TestingBridgeDownloadInitialDelay" value. See
       * learned_bridge_descriptor() for details.
       */
      return options->TestingBridgeBootstrapDownloadInitialDelay;
    default:
      tor_assert(0);
  }

  /* Impossible, but gcc will fail with -Werror without a `return`. */
  return 0;
}

/** As next_random_exponential_delay() below, but does not compute a random
 * value. Instead, compute the range of values that
 * next_random_exponential_delay() should use when computing its random value.
 * Store the low bound into *<b>low_bound_out</b>, and the high bound into
 * *<b>high_bound_out</b>.  Guarantees that the low bound is strictly less
 * than the high bound. */
STATIC void
next_random_exponential_delay_range(int *low_bound_out,
                                    int *high_bound_out,
                                    int delay,
                                    int base_delay)
{
  // This is the "decorrelated jitter" approach, from
  //    https://www.awsarchitectureblog.com/2015/03/backoff.html
  // The formula is
  //    sleep = min(cap, random_between(base, sleep * 3))

  const int delay_times_3 = delay < INT_MAX/3 ? delay * 3 : INT_MAX;
  *low_bound_out = base_delay;
  if (delay_times_3 > base_delay) {
    *high_bound_out = delay_times_3;
  } else {
    *high_bound_out = base_delay+1;
  }
}

/** Advance one delay step.  The algorithm will generate a random delay,
 * such that each failure is possibly (random) longer than the ones before.
 *
 * We then clamp that value to be no larger than max_delay, and return it.
 *
 * The <b>base_delay</b> parameter is lowest possible delay time (can't be
 * zero); the <b>backoff_position</b> parameter is the number of times we've
 * generated a delay; and the <b>delay</b> argument is the most recently used
 * delay.
 */
STATIC int
next_random_exponential_delay(int delay,
                              int base_delay)
{
  /* Check preconditions */
  if (BUG(delay < 0))
    delay = 0;

  if (base_delay < 1)
    base_delay = 1;

  int low_bound=0, high_bound=INT_MAX;

  next_random_exponential_delay_range(&low_bound, &high_bound,
                                      delay, base_delay);

  return crypto_rand_int_range(low_bound, high_bound);
}

/** Find the current delay for dls based on min_delay.
 *
 * This function sets dls->next_attempt_at based on now, and returns the delay.
 * Helper for download_status_increment_failure and
 * download_status_increment_attempt. */
STATIC int
download_status_schedule_get_delay(download_status_t *dls,
                                   int min_delay,
                                   time_t now)
{
  tor_assert(dls);
  /* If we're using random exponential backoff, we do need min/max delay */
  tor_assert(min_delay >= 0);

  int delay = INT_MAX;
  uint8_t dls_schedule_position = (dls->increment_on
                                   == DL_SCHED_INCREMENT_ATTEMPT
                                   ? dls->n_download_attempts
                                   : dls->n_download_failures);

  /* Check if we missed a reset somehow */
  IF_BUG_ONCE(dls->last_backoff_position > dls_schedule_position) {
    dls->last_backoff_position = 0;
    dls->last_delay_used = 0;
  }

  if (dls_schedule_position > 0) {
    delay = dls->last_delay_used;

    while (dls->last_backoff_position < dls_schedule_position) {
      /* Do one increment step */
      delay = next_random_exponential_delay(delay, min_delay);
      /* Update our position */
      ++(dls->last_backoff_position);
    }
  } else {
    /* If we're just starting out, use the minimum delay */
    delay = min_delay;
  }

  /* Clamp it within min/max if we have them */
  if (min_delay >= 0 && delay < min_delay) delay = min_delay;

  /* Store it for next time */
  dls->last_backoff_position = dls_schedule_position;
  dls->last_delay_used = delay;

  /* A negative delay makes no sense. Knowing that delay is
   * non-negative allows us to safely do the wrapping check below. */
  tor_assert(delay >= 0);

  /* Avoid now+delay overflowing TIME_MAX, by comparing with a subtraction
   * that won't overflow (since delay is non-negative). */
  if (delay < INT_MAX && now <= TIME_MAX - delay) {
    dls->next_attempt_at = now+delay;
  } else {
    dls->next_attempt_at = TIME_MAX;
  }

  return delay;
}

/* Log a debug message about item, which increments on increment_action, has
 * incremented dls_n_download_increments times. The message varies based on
 * was_schedule_incremented (if not, not_incremented_response is logged), and
 * the values of increment, dls_next_attempt_at, and now.
 * Helper for download_status_increment_failure and
 * download_status_increment_attempt. */
static void
download_status_log_helper(const char *item, int was_schedule_incremented,
                           const char *increment_action,
                           const char *not_incremented_response,
                           uint8_t dls_n_download_increments, int increment,
                           time_t dls_next_attempt_at, time_t now)
{
  if (item) {
    if (!was_schedule_incremented)
      log_debug(LD_DIR, "%s %s %d time(s); I'll try again %s.",
                item, increment_action, (int)dls_n_download_increments,
                not_incremented_response);
    else if (increment == 0)
      log_debug(LD_DIR, "%s %s %d time(s); I'll try again immediately.",
                item, increment_action, (int)dls_n_download_increments);
    else if (dls_next_attempt_at < TIME_MAX)
      log_debug(LD_DIR, "%s %s %d time(s); I'll try again in %d seconds.",
                item, increment_action, (int)dls_n_download_increments,
                (int)(dls_next_attempt_at-now));
    else
      log_debug(LD_DIR, "%s %s %d time(s); Giving up for a while.",
                item, increment_action, (int)dls_n_download_increments);
  }
}

/** Determine when a failed download attempt should be retried.
 * Called when an attempt to download <b>dls</b> has failed with HTTP status
 * <b>status_code</b>.  Increment the failure count (if the code indicates a
 * real failure, or if we're a server) and set <b>dls</b>-\>next_attempt_at to
 * an appropriate time in the future and return it.
 * If <b>dls->increment_on</b> is DL_SCHED_INCREMENT_ATTEMPT, increment the
 * failure count, and return a time in the far future for the next attempt (to
 * avoid an immediate retry). */
time_t
download_status_increment_failure(download_status_t *dls, int status_code,
                                  const char *item, int server, time_t now)
{
  (void) status_code; // XXXX no longer used.
  (void) server; // XXXX no longer used.
  int increment = -1;
  int min_delay = 0;

  tor_assert(dls);

  /* dls wasn't reset before it was used */
  if (dls->next_attempt_at == 0) {
    download_status_reset(dls);
  }

  /* count the failure */
  if (dls->n_download_failures < IMPOSSIBLE_TO_DOWNLOAD-1) {
    ++dls->n_download_failures;
  }

  if (dls->increment_on == DL_SCHED_INCREMENT_FAILURE) {
    /* We don't find out that a failure-based schedule has attempted a
     * connection until that connection fails.
     * We'll never find out about successful connections, but this doesn't
     * matter, because schedules are reset after a successful download.
     */
    if (dls->n_download_attempts < IMPOSSIBLE_TO_DOWNLOAD-1)
      ++dls->n_download_attempts;

    /* only return a failure retry time if this schedule increments on failures
     */
    min_delay = find_dl_min_delay(dls, get_options());
    increment = download_status_schedule_get_delay(dls, min_delay, now);
  }

  download_status_log_helper(item, !dls->increment_on, "failed",
                             "concurrently", dls->n_download_failures,
                             increment,
                             download_status_get_next_attempt_at(dls),
                             now);

  if (dls->increment_on == DL_SCHED_INCREMENT_ATTEMPT) {
    /* stop this schedule retrying on failure, it will launch concurrent
     * connections instead */
    return TIME_MAX;
  } else {
    return download_status_get_next_attempt_at(dls);
  }
}

/** Determine when the next download attempt should be made when using an
 * attempt-based (potentially concurrent) download schedule.
 * Called when an attempt to download <b>dls</b> is being initiated.
 * Increment the attempt count and set <b>dls</b>-\>next_attempt_at to an
 * appropriate time in the future and return it.
 * If <b>dls->increment_on</b> is DL_SCHED_INCREMENT_FAILURE, don't increment
 * the attempts, and return a time in the far future (to avoid launching a
 * concurrent attempt). */
time_t
download_status_increment_attempt(download_status_t *dls, const char *item,
                                  time_t now)
{
  int delay = -1;
  int min_delay = 0;

  tor_assert(dls);

  /* dls wasn't reset before it was used */
  if (dls->next_attempt_at == 0) {
    download_status_reset(dls);
  }

  if (dls->increment_on == DL_SCHED_INCREMENT_FAILURE) {
    /* this schedule should retry on failure, and not launch any concurrent
     attempts */
    log_warn(LD_BUG, "Tried to launch an attempt-based connection on a "
             "failure-based schedule.");
    return TIME_MAX;
  }

  if (dls->n_download_attempts < IMPOSSIBLE_TO_DOWNLOAD-1)
    ++dls->n_download_attempts;

  min_delay = find_dl_min_delay(dls, get_options());
  delay = download_status_schedule_get_delay(dls, min_delay, now);

  download_status_log_helper(item, dls->increment_on, "attempted",
                             "on failure", dls->n_download_attempts,
                             delay, download_status_get_next_attempt_at(dls),
                             now);

  return download_status_get_next_attempt_at(dls);
}

static time_t
download_status_get_initial_delay_from_now(const download_status_t *dls)
{
  /* We use constant initial delays, even in exponential backoff
   * schedules. */
  return time(NULL) + find_dl_min_delay(dls, get_options());
}

/** Reset <b>dls</b> so that it will be considered downloadable
 * immediately, and/or to show that we don't need it anymore.
 *
 * Must be called to initialise a download schedule, otherwise the zeroth item
 * in the schedule will never be used.
 *
 * (We find the zeroth element of the download schedule, and set
 * next_attempt_at to be the appropriate offset from 'now'. In most
 * cases this means setting it to 'now', so the item will be immediately
 * downloadable; when using authorities with fallbacks, there is a few seconds'
 * delay.) */
void
download_status_reset(download_status_t *dls)
{
  if (dls->n_download_failures == IMPOSSIBLE_TO_DOWNLOAD
      || dls->n_download_attempts == IMPOSSIBLE_TO_DOWNLOAD)
    return; /* Don't reset this. */

  dls->n_download_failures = 0;
  dls->n_download_attempts = 0;
  dls->next_attempt_at = download_status_get_initial_delay_from_now(dls);
  dls->last_backoff_position = 0;
  dls->last_delay_used = 0;
  /* Don't reset dls->want_authority or dls->increment_on */
}

/** Return true iff, as of <b>now</b>, the resource tracked by <b>dls</b> is
 * ready to get its download reattempted. */
int
download_status_is_ready(download_status_t *dls, time_t now)
{
  /* dls wasn't reset before it was used */
  if (dls->next_attempt_at == 0) {
    download_status_reset(dls);
  }

  return download_status_get_next_attempt_at(dls) <= now;
}

/** Mark <b>dl</b> as never downloadable. */
void
download_status_mark_impossible(download_status_t *dl)
{
  dl->n_download_failures = IMPOSSIBLE_TO_DOWNLOAD;
  dl->n_download_attempts = IMPOSSIBLE_TO_DOWNLOAD;
}

/** Return the number of failures on <b>dls</b> since the last success (if
 * any). */
int
download_status_get_n_failures(const download_status_t *dls)
{
  return dls->n_download_failures;
}

/** Return the number of attempts to download <b>dls</b> since the last success
 * (if any). This can differ from download_status_get_n_failures() due to
 * outstanding concurrent attempts. */
int
download_status_get_n_attempts(const download_status_t *dls)
{
  return dls->n_download_attempts;
}

/** Return the next time to attempt to download <b>dls</b>. */
time_t
download_status_get_next_attempt_at(const download_status_t *dls)
{
  /* dls wasn't reset before it was used */
  if (dls->next_attempt_at == 0) {
    /* so give the answer we would have given if it had been */
    return download_status_get_initial_delay_from_now(dls);
  }

  return dls->next_attempt_at;
}
