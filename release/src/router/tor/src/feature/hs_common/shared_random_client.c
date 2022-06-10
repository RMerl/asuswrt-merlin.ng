/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file shared_random_client.c
 * \brief This file contains functions that are from the shared random
 *        subsystem but used by many part of tor. The full feature is built
 *        as part of the dirauth module.
 **/

#include "feature/hs_common/shared_random_client.h"

#include "app/config/config.h"
#include "feature/dirauth/authmode.h"
#include "feature/dirauth/voting_schedule.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "lib/encoding/binascii.h"

#include "feature/nodelist/networkstatus_st.h"

/** Convert a given srv object to a string for the control port. This doesn't
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

/**
 * If we have no consensus and we are not an authority, assume that this is the
 * voting interval. This can be used while bootstrapping as a relay and we are
 * asked to initialize HS stats (see rep_hist_hs_stats_init()) */
#define DEFAULT_NETWORK_VOTING_INTERVAL (3600)
#define TESTING_DEFAULT_NETWORK_VOTING_INTERVAL (20)

/* This is an unpleasing workaround for tests.  Our unit tests assume that we
 * are scheduling all of our shared random stuff as if we were a directory
 * authority, but they do not always set V3AuthoritativeDir.
 */
#ifdef TOR_UNIT_TESTS
#define ASSUME_AUTHORITY_SCHEDULING 1
#else
#define ASSUME_AUTHORITY_SCHEDULING 0
#endif

/** Return the voting interval of the tor vote subsystem. */
int
get_voting_interval(void)
{
  int interval;
  networkstatus_t *consensus =
    networkstatus_get_reasonably_live_consensus(time(NULL),
                                                usable_consensus_flavor());

  if (consensus) {
    /* Ideally we have a live consensus and we can just use that. */
    interval = (int)(consensus->fresh_until - consensus->valid_after);
  } else if (authdir_mode(get_options()) || ASSUME_AUTHORITY_SCHEDULING) {
    /* If we don't have a live consensus and we're an authority,
     * we should believe our own view of what the schedule ought to be. */
    interval = dirauth_sched_get_configured_interval();
  } else if ((consensus = networkstatus_get_latest_consensus())) {
    /* If we're a client, then maybe a latest consensus is good enough?
     * It's better than falling back to the non-consensus case. */
    interval = (int)(consensus->fresh_until - consensus->valid_after);
  } else {
    /* We can reach this as a relay when bootstrapping and we are asked to
     * initialize HS stats (see rep_hist_hs_stats_init()). */
    if (get_options()->TestingTorNetwork) {
      interval = TESTING_DEFAULT_NETWORK_VOTING_INTERVAL;
    } else {
      interval = DEFAULT_NETWORK_VOTING_INTERVAL;
    }
  }
  tor_assert(interval > 0);
  return interval;
}

/*
 * Public API
 */

/** Encode the given shared random value and put it in dst. Destination
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

/** Return the current SRV string representation for the control port. Return a
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

/** Return the previous SRV string representation for the control port. Return
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

/** Return current shared random value from the latest consensus. Caller can
 * NOT keep a reference to the returned pointer. Return NULL if none. */
const sr_srv_t *
sr_get_current(const networkstatus_t *ns)
{
  const networkstatus_t *consensus;

  /* Use provided ns else get a live one */
  if (ns) {
    consensus = ns;
  } else {
    consensus = networkstatus_get_reasonably_live_consensus(approx_time(),
                                                  usable_consensus_flavor());
  }
  /* Ideally we would never be asked for an SRV without a live consensus. Make
   * sure this assumption is correct. */
  tor_assert_nonfatal(consensus);

  if (consensus) {
    return consensus->sr_info.current_srv;
  }
  return NULL;
}

/** Return previous shared random value from the latest consensus. Caller can
 * NOT keep a reference to the returned pointer. Return NULL if none. */
const sr_srv_t *
sr_get_previous(const networkstatus_t *ns)
{
  const networkstatus_t *consensus;

  /* Use provided ns else get a live one */
  if (ns) {
    consensus = ns;
  } else {
    consensus = networkstatus_get_reasonably_live_consensus(approx_time(),
                                                  usable_consensus_flavor());
  }
  /* Ideally we would never be asked for an SRV without a live consensus. Make
   * sure this assumption is correct. */
  tor_assert_nonfatal(consensus);

  if (consensus) {
    return consensus->sr_info.previous_srv;
  }
  return NULL;
}

/** Parse a list of arguments from a SRV value either from a vote, consensus
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

  /* This function is not used for voting purposes, so if we have a reasonably
   * live consensus, use its valid-after as the beginning of the current
   * round. If we have no consensus but we're an authority, use our own
   * schedule. Otherwise, try using our view of the voting interval to figure
   * out when the current round _should_ be starting. */
  networkstatus_t *ns =
    networkstatus_get_reasonably_live_consensus(approx_time(),
                                                usable_consensus_flavor());
  if (ns) {
    beginning_of_curr_round = ns->valid_after;
  } else if (authdir_mode(get_options()) || ASSUME_AUTHORITY_SCHEDULING) {
    beginning_of_curr_round = dirauth_sched_get_cur_valid_after_time();
  } else {
    /* voting_interval comes from get_voting_interval(), so if we're in
     * this case as a client, we already tried to get the voting interval
     * from the latest_consensus and gave a bug warning if we couldn't.
     *
     * We wouldn't want to look at the latest consensus's valid_after time,
     * since that would be out of date. */
    beginning_of_curr_round = voting_sched_get_start_of_interval_after(
                                             approx_time() - voting_interval,
                                             voting_interval,
                                             0);
  }

  /* Get current SR protocol round */
  int curr_round_slot;
  curr_round_slot = (beginning_of_curr_round / voting_interval) % total_rounds;

  /* Get start time by subtracting the time elapsed from the beginning of the
     protocol run */
  time_t time_elapsed_since_start_of_run = curr_round_slot * voting_interval;

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
