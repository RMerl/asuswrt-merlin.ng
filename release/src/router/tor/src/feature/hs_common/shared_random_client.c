/* Copyright (c) 2018-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file shared_random_client.c
 * \brief This file contains functions that are from the shared random
 *        subsystem but used by many part of tor. The full feature is built
 *        as part of the dirauth module.
 **/

#define SHARED_RANDOM_CLIENT_PRIVATE
#include "feature/hs_common/shared_random_client.h"

#include "app/config/config.h"
#include "feature/dircommon/voting_schedule.h"
#include "feature/nodelist/networkstatus.h"
#include "lib/encoding/binascii.h"

#include "feature/nodelist/networkstatus_st.h"

/* Convert a given srv object to a string for the control port. This doesn't
 * fail and the srv object MUST be valid. */
static char *
srv_to_control_string(const sr_srv_t *srv)
{
  char *srv_str;
  char srv_hash_encoded[SR_SRV_VALUE_BASE64_LEN + 1];
  tor_assert(srv);

  sr_srv_encode(srv_hash_encoded, sizeof(srv_hash_encoded), srv);
  tor_asprintf(&srv_str, "%s", srv_hash_encoded);
  return srv_str;
}

/* Return the voting interval of the tor vote subsystem. */
int
get_voting_interval(void)
{
  int interval;
  networkstatus_t *consensus = networkstatus_get_live_consensus(time(NULL));

  if (consensus) {
    interval = (int)(consensus->fresh_until - consensus->valid_after);
  } else {
    /* Same for both a testing and real network. We voluntarily ignore the
     * InitialVotingInterval since it complexifies things and it doesn't
     * affect the SR protocol. */
    interval = get_options()->V3AuthVotingInterval;
  }
  tor_assert(interval > 0);
  return interval;
}

/* Given the current consensus, return the start time of the current round of
 * the SR protocol. For example, if it's 23:47:08, the current round thus
 * started at 23:47:00 for a voting interval of 10 seconds.
 *
 * This function uses the consensus voting schedule to derive its results,
 * instead of the actual consensus we are currently using, so it should be used
 * for voting purposes. */
time_t
get_start_time_of_current_round(void)
{
  const or_options_t *options = get_options();
  int voting_interval = get_voting_interval();
  /* First, get the start time of the next round */
  time_t next_start = voting_schedule_get_next_valid_after_time();
  /* Now roll back next_start by a voting interval to find the start time of
     the current round. */
  time_t curr_start = voting_schedule_get_start_of_next_interval(
                                     next_start - voting_interval - 1,
                                     voting_interval,
                                     options->TestingV3AuthVotingStartOffset);
  return curr_start;
}

/*
 * Public API
 */

/* Encode the given shared random value and put it in dst. Destination
 * buffer must be at least SR_SRV_VALUE_BASE64_LEN plus the NULL byte. */
void
sr_srv_encode(char *dst, size_t dst_len, const sr_srv_t *srv)
{
  int ret;
  /* Extra byte for the NULL terminated char. */
  char buf[SR_SRV_VALUE_BASE64_LEN + 1];

  tor_assert(dst);
  tor_assert(srv);
  tor_assert(dst_len >= sizeof(buf));

  ret = base64_encode(buf, sizeof(buf), (const char *) srv->value,
                      sizeof(srv->value), 0);
  /* Always expect the full length without the NULL byte. */
  tor_assert(ret == (sizeof(buf) - 1));
  tor_assert(ret <= (int) dst_len);
  strlcpy(dst, buf, dst_len);
}

/* Return the current SRV string representation for the control port. Return a
 * newly allocated string on success containing the value else "" if not found
 * or if we don't have a valid consensus yet. */
char *
sr_get_current_for_control(void)
{
  char *srv_str;
  const networkstatus_t *c = networkstatus_get_latest_consensus();
  if (c && c->sr_info.current_srv) {
    srv_str = srv_to_control_string(c->sr_info.current_srv);
  } else {
    srv_str = tor_strdup("");
  }
  return srv_str;
}

/* Return the previous SRV string representation for the control port. Return
 * a newly allocated string on success containing the value else "" if not
 * found or if we don't have a valid consensus yet. */
char *
sr_get_previous_for_control(void)
{
  char *srv_str;
  const networkstatus_t *c = networkstatus_get_latest_consensus();
  if (c && c->sr_info.previous_srv) {
    srv_str = srv_to_control_string(c->sr_info.previous_srv);
  } else {
    srv_str = tor_strdup("");
  }
  return srv_str;
}

/* Return current shared random value from the latest consensus. Caller can
 * NOT keep a reference to the returned pointer. Return NULL if none. */
const sr_srv_t *
sr_get_current(const networkstatus_t *ns)
{
  const networkstatus_t *consensus;

  /* Use provided ns else get a live one */
  if (ns) {
    consensus = ns;
  } else {
    consensus = networkstatus_get_live_consensus(approx_time());
  }
  /* Ideally we would never be asked for an SRV without a live consensus. Make
   * sure this assumption is correct. */
  tor_assert_nonfatal(consensus);

  if (consensus) {
    return consensus->sr_info.current_srv;
  }
  return NULL;
}

/* Return previous shared random value from the latest consensus. Caller can
 * NOT keep a reference to the returned pointer. Return NULL if none. */
const sr_srv_t *
sr_get_previous(const networkstatus_t *ns)
{
  const networkstatus_t *consensus;

  /* Use provided ns else get a live one */
  if (ns) {
    consensus = ns;
  } else {
    consensus = networkstatus_get_live_consensus(approx_time());
  }
  /* Ideally we would never be asked for an SRV without a live consensus. Make
   * sure this assumption is correct. */
  tor_assert_nonfatal(consensus);

  if (consensus) {
    return consensus->sr_info.previous_srv;
  }
  return NULL;
}

/* Parse a list of arguments from a SRV value either from a vote, consensus
 * or from our disk state and return a newly allocated srv object. NULL is
 * returned on error.
 *
 * The arguments' order:
 *    num_reveals, value
 */
sr_srv_t *
sr_parse_srv(const smartlist_t *args)
{
  char *value;
  int ok, ret;
  uint64_t num_reveals;
  sr_srv_t *srv = NULL;

  tor_assert(args);

  if (smartlist_len(args) < 2) {
    goto end;
  }

  /* First argument is the number of reveal values */
  num_reveals = tor_parse_uint64(smartlist_get(args, 0),
                                 10, 0, UINT64_MAX, &ok, NULL);
  if (!ok) {
    goto end;
  }
  /* Second and last argument is the shared random value it self. */
  value = smartlist_get(args, 1);
  if (strlen(value) != SR_SRV_VALUE_BASE64_LEN) {
    goto end;
  }

  srv = tor_malloc_zero(sizeof(*srv));
  srv->num_reveals = num_reveals;
  /* We subtract one byte from the srclen because the function ignores the
   * '=' character in the given buffer. This is broken but it's a documented
   * behavior of the implementation. */
  ret = base64_decode((char *) srv->value, sizeof(srv->value), value,
                      SR_SRV_VALUE_BASE64_LEN - 1);
  if (ret != sizeof(srv->value)) {
    tor_free(srv);
    srv = NULL;
    goto end;
  }
 end:
  return srv;
}

/** Return the start time of the current SR protocol run using the times from
 *  the current consensus. For example, if the latest consensus valid-after is
 *  23/06/2017 23:00:00 and a full SR protocol run is 24 hours, this function
 *  returns 23/06/2017 00:00:00. */
time_t
sr_state_get_start_time_of_current_protocol_run(void)
{
  int total_rounds = SHARED_RANDOM_N_ROUNDS * SHARED_RANDOM_N_PHASES;
  int voting_interval = get_voting_interval();
  time_t beginning_of_curr_round;

  /* This function is not used for voting purposes, so if we have a live
     consensus, use its valid-after as the beginning of the current round,
     otherwise resort to the voting schedule which should always exist. */
  networkstatus_t *ns = networkstatus_get_live_consensus(approx_time());
  if (ns) {
    beginning_of_curr_round = ns->valid_after;
  } else {
    beginning_of_curr_round = get_start_time_of_current_round();
  }

  /* Get current SR protocol round */
  int curr_round_slot;
  curr_round_slot = (beginning_of_curr_round / voting_interval) % total_rounds;

  /* Get start time by subtracting the time elapsed from the beginning of the
     protocol run */
  time_t time_elapsed_since_start_of_run = curr_round_slot * voting_interval;

  log_debug(LD_GENERAL, "Current SRV proto run: Start of current round: %u. "
            "Time elapsed: %u (%d)", (unsigned) beginning_of_curr_round,
            (unsigned) time_elapsed_since_start_of_run, voting_interval);

  return beginning_of_curr_round - time_elapsed_since_start_of_run;
}

/** Return the start time of the previous SR protocol run. See
 *  sr_state_get_start_time_of_current_protocol_run() for more details.  */
time_t
sr_state_get_start_time_of_previous_protocol_run(void)
{
  time_t start_time_of_current_run =
    sr_state_get_start_time_of_current_protocol_run();

  /* We get the start time of previous protocol run, by getting the start time
   * of current run and the subtracting a full protocol run from that. */
  return start_time_of_current_run - sr_state_get_protocol_run_duration();
}

/** Return the time (in seconds) it takes to complete a full SR protocol phase
 *  (e.g. the commit phase). */
unsigned int
sr_state_get_phase_duration(void)
{
  return SHARED_RANDOM_N_ROUNDS * get_voting_interval();
}

/** Return the time (in seconds) it takes to complete a full SR protocol run */
unsigned int
sr_state_get_protocol_run_duration(void)
{
  int total_protocol_rounds = SHARED_RANDOM_N_ROUNDS * SHARED_RANDOM_N_PHASES;
  return total_protocol_rounds * get_voting_interval();
}

