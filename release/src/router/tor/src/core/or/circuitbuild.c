/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitbuild.c
 *
 * \brief Implements the details of building circuits (by choosing paths,
 * constructing/sending create/extend cells, and so on).
 *
 * On the client side, this module handles launching circuits. Circuit
 * launches are started from circuit_establish_circuit(), called from
 * circuit_launch_by_extend_info()).  To choose the path the circuit will
 * take, onion_extend_cpath() calls into a maze of node selection functions.
 *
 * Once the circuit is ready to be launched, the first hop is treated as a
 * special case with circuit_handle_first_hop(), since it might need to open a
 * channel.  As the channel opens, and later as CREATED and RELAY_EXTENDED
 * cells arrive, the client will invoke circuit_send_next_onion_skin() to send
 * CREATE or RELAY_EXTEND cells.
 *
 * The server side is handled in feature/relay/circuitbuild_relay.c.
 **/

#define CIRCUITBUILD_PRIVATE
#define OCIRC_EVENT_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/confmgt/confmgt.h"
#include "core/crypto/hs_ntor.h"
#include "core/crypto/onion_crypto.h"
#include "core/crypto/onion_fast.h"
#include "core/crypto/onion_tap.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/or/channel.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitstats.h"
#include "core/or/circuituse.h"
#include "core/or/circuitpadding.h"
#include "core/or/command.h"
#include "core/or/connection_edge.h"
#include "core/or/connection_or.h"
#include "core/or/conflux_pool.h"
#include "core/or/extendinfo.h"
#include "core/or/onion.h"
#include "core/or/ocirc_event.h"
#include "core/or/policies.h"
#include "core/or/relay.h"
#include "core/or/trace_probes_circuit.h"
#include "core/or/crypt_path.h"
#include "feature/client/bridges.h"
#include "feature/client/circpathbias.h"
#include "feature/client/entrynodes.h"
#include "feature/client/transports.h"
#include "feature/control/control_events.h"
#include "feature/dircommon/directory.h"
#include "feature/nodelist/describe.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nickname.h"
#include "feature/nodelist/node_select.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"
#include "feature/stats/predict_ports.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/trace/events.h"
#include "core/or/congestion_control_common.h"

#include "core/or/cell_st.h"
#include "core/or/cpath_build_state_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

#include "trunnel/extension.h"
#include "trunnel/congestion_control.h"

static int circuit_send_first_onion_skin(origin_circuit_t *circ);
static int circuit_build_no_more_hops(origin_circuit_t *circ);
static int circuit_send_intermediate_onion_skin(origin_circuit_t *circ,
                                                crypt_path_t *hop);
static const node_t *choose_good_middle_server(const origin_circuit_t *,
                          uint8_t purpose,
                          cpath_build_state_t *state,
                          crypt_path_t *head,
                          int cur_len);

/** This function tries to get a channel to the specified endpoint,
 * and then calls command_setup_channel() to give it the right
 * callbacks.
 */
MOCK_IMPL(channel_t *,
channel_connect_for_circuit,(const extend_info_t *ei))
{
  channel_t *chan;

  const tor_addr_port_t *orport = extend_info_pick_orport(ei);
  if (!orport)
    return NULL;
  const char *id_digest = ei->identity_digest;
  const ed25519_public_key_t *ed_id = &ei->ed_identity;

  chan = channel_connect(&orport->addr, orport->port, id_digest, ed_id);
  if (chan) command_setup_channel(chan);

  return chan;
}

/** Search for a value for circ_id that we can use on <b>chan</b> for an
 * outbound circuit, until we get a circ_id that is not in use by any other
 * circuit on that conn.
 *
 * Return it, or 0 if can't get a unique circ_id.
 */
STATIC circid_t
get_unique_circ_id_by_chan(channel_t *chan)
{
/* This number is chosen somewhat arbitrarily; see comment below for more
 * info.  When the space is 80% full, it gives a one-in-a-million failure
 * chance; when the space is 90% full, it gives a one-in-850 chance; and when
 * the space is 95% full, it gives a one-in-26 failure chance.  That seems
 * okay, though you could make a case IMO for anything between N=32 and
 * N=256. */
#define MAX_CIRCID_ATTEMPTS 64
  int in_use;
  unsigned n_with_circ = 0, n_pending_destroy = 0, n_weird_pending_destroy = 0;
  circid_t test_circ_id;
  circid_t attempts=0;
  circid_t high_bit, max_range, mask;
  int64_t pending_destroy_time_total = 0;
  int64_t pending_destroy_time_max = 0;

  tor_assert(chan);

  if (chan->circ_id_type == CIRC_ID_TYPE_NEITHER) {
    log_warn(LD_BUG,
             "Trying to pick a circuit ID for a connection from "
             "a client with no identity.");
    return 0;
  }
  max_range = (chan->wide_circ_ids) ? (1u<<31) : (1u<<15);
  mask = max_range - 1;
  high_bit = (chan->circ_id_type == CIRC_ID_TYPE_HIGHER) ? max_range : 0;
  do {
    if (++attempts > MAX_CIRCID_ATTEMPTS) {
      /* Make sure we don't loop forever because all circuit IDs are used.
       *
       * Once, we would try until we had tried every possible circuit ID.  But
       * that's quite expensive.  Instead, we try MAX_CIRCID_ATTEMPTS random
       * circuit IDs, and then give up.
       *
       * This potentially causes us to give up early if our circuit ID space
       * is nearly full.  If we have N circuit IDs in use, then we will reject
       * a new circuit with probability (N / max_range) ^ MAX_CIRCID_ATTEMPTS.
       * This means that in practice, a few percent of our circuit ID capacity
       * will go unused.
       *
       * The alternative here, though, is to do a linear search over the
       * whole circuit ID space every time we extend a circuit, which is
       * not so great either.
       */
      int64_t queued_destroys;
      char *m = rate_limit_log(&chan->last_warned_circ_ids_exhausted,
                               approx_time());
      if (m == NULL)
        return 0; /* This message has been rate-limited away. */
      if (n_pending_destroy)
        pending_destroy_time_total /= n_pending_destroy;
      log_warn(LD_CIRC,"No unused circIDs found on channel %s wide "
                 "circID support, with %u inbound and %u outbound circuits. "
                 "Found %u circuit IDs in use by circuits, and %u with "
                 "pending destroy cells. (%u of those were marked bogusly.) "
                 "The ones with pending destroy cells "
                 "have been marked unusable for an average of %ld seconds "
                 "and a maximum of %ld seconds. This channel is %ld seconds "
                 "old. Failing a circuit.%s",
                 chan->wide_circ_ids ? "with" : "without",
                 chan->num_p_circuits, chan->num_n_circuits,
                 n_with_circ, n_pending_destroy, n_weird_pending_destroy,
                 (long)pending_destroy_time_total,
                 (long)pending_destroy_time_max,
                 (long)(approx_time() - chan->timestamp_created),
                 m);
      tor_free(m);

      if (!chan->cmux) {
        /* This warning should be impossible. */
        log_warn(LD_BUG, "  This channel somehow has no cmux on it!");
        return 0;
      }

      /* analysis so far on 12184 suggests that we're running out of circuit
         IDs because it looks like we have too many pending destroy
         cells. Let's see how many we really have pending.
      */
      queued_destroys = circuitmux_count_queued_destroy_cells(chan,
                                                              chan->cmux);

      log_warn(LD_CIRC, "  Circuitmux on this channel has %u circuits, "
               "of which %u are active. It says it has %"PRId64
               " destroy cells queued.",
               circuitmux_num_circuits(chan->cmux),
               circuitmux_num_active_circuits(chan->cmux),
               (queued_destroys));

      /* Change this into "if (1)" in order to get more information about
       * possible failure modes here.  You'll need to know how to use gdb with
       * Tor: this will make Tor exit with an assertion failure if the cmux is
       * corrupt. */
      if (0)
        circuitmux_assert_okay(chan->cmux);

      channel_dump_statistics(chan, LOG_WARN);

      return 0;
    }

    do {
      crypto_rand((char*) &test_circ_id, sizeof(test_circ_id));
      test_circ_id &= mask;
    } while (test_circ_id == 0);

    test_circ_id |= high_bit;

    in_use = circuit_id_in_use_on_channel(test_circ_id, chan);
    if (in_use == 1)
      ++n_with_circ;
    else if (in_use == 2) {
      time_t since_when;
      ++n_pending_destroy;
      since_when =
        circuit_id_when_marked_unusable_on_channel(test_circ_id, chan);
      if (since_when) {
        time_t waiting = approx_time() - since_when;
        pending_destroy_time_total += waiting;
        if (waiting > pending_destroy_time_max)
          pending_destroy_time_max = waiting;
      } else {
        ++n_weird_pending_destroy;
      }
    }
  } while (in_use);
  return test_circ_id;
}

/** If <b>verbose</b> is false, allocate and return a comma-separated list of
 * the currently built elements of <b>circ</b>. If <b>verbose</b> is true, also
 * list information about link status in a more verbose format using spaces.
 * If <b>verbose_names</b> is false, give hex digests; if <b>verbose_names</b>
 * is true, use $DIGEST=Name style names.
 */
static char *
circuit_list_path_impl(origin_circuit_t *circ, int verbose, int verbose_names)
{
  crypt_path_t *hop;
  smartlist_t *elements;
  const char *states[] = {"closed", "waiting for keys", "open"};
  char *s;

  elements = smartlist_new();

  if (verbose) {
    const char *nickname = build_state_get_exit_nickname(circ->build_state);
    smartlist_add_asprintf(elements, "%s%s circ (length %d%s%s):",
                 circ->build_state->is_internal ? "internal" : "exit",
                 circ->build_state->need_uptime ? " (high-uptime)" : "",
                 circ->build_state->desired_path_len,
                 circ->base_.state == CIRCUIT_STATE_OPEN ? "" : ", last hop ",
                 circ->base_.state == CIRCUIT_STATE_OPEN ? "" :
                 (nickname?nickname:"*unnamed*"));
  }

  hop = circ->cpath;
  do {
    char *elt;
    const char *id;
    const node_t *node;
    if (!hop)
      break;
    if (!verbose && hop->state != CPATH_STATE_OPEN)
      break;
    if (!hop->extend_info)
      break;
    id = hop->extend_info->identity_digest;
    if (verbose_names) {
      elt = tor_malloc(MAX_VERBOSE_NICKNAME_LEN+1);
      if ((node = node_get_by_id(id))) {
        node_get_verbose_nickname(node, elt);
      } else if (is_legal_nickname(hop->extend_info->nickname)) {
        elt[0] = '$';
        base16_encode(elt+1, HEX_DIGEST_LEN+1, id, DIGEST_LEN);
        elt[HEX_DIGEST_LEN+1]= '~';
        strlcpy(elt+HEX_DIGEST_LEN+2,
                hop->extend_info->nickname, MAX_NICKNAME_LEN+1);
      } else {
        elt[0] = '$';
        base16_encode(elt+1, HEX_DIGEST_LEN+1, id, DIGEST_LEN);
      }
    } else { /* ! verbose_names */
      elt = tor_malloc(HEX_DIGEST_LEN+2);
      elt[0] = '$';
      base16_encode(elt+1, HEX_DIGEST_LEN+1, id, DIGEST_LEN);
    }
    tor_assert(elt);
    if (verbose) {
      tor_assert(hop->state <= 2);
      smartlist_add_asprintf(elements,"%s(%s)",elt,states[hop->state]);
      tor_free(elt);
    } else {
      smartlist_add(elements, elt);
    }
    hop = hop->next;
  } while (hop != circ->cpath);

  s = smartlist_join_strings(elements, verbose?" ":",", 0, NULL);
  SMARTLIST_FOREACH(elements, char*, cp, tor_free(cp));
  smartlist_free(elements);
  return s;
}

/** If <b>verbose</b> is false, allocate and return a comma-separated
 * list of the currently built elements of <b>circ</b>.  If
 * <b>verbose</b> is true, also list information about link status in
 * a more verbose format using spaces.
 */
char *
circuit_list_path(origin_circuit_t *circ, int verbose)
{
  return circuit_list_path_impl(circ, verbose, 0);
}

/** Allocate and return a comma-separated list of the currently built elements
 * of <b>circ</b>, giving each as a verbose nickname.
 */
char *
circuit_list_path_for_controller(origin_circuit_t *circ)
{
  return circuit_list_path_impl(circ, 0, 1);
}

/** Log, at severity <b>severity</b>, the nicknames of each router in
 * <b>circ</b>'s cpath. Also log the length of the cpath, and the intended
 * exit point.
 */
void
circuit_log_path(int severity, unsigned int domain, origin_circuit_t *circ)
{
  char *s = circuit_list_path(circ,1);
  tor_log(severity,domain,"%s",s);
  tor_free(s);
}

/** Return 1 iff every node in circ's cpath definitely supports ntor. */
static int
circuit_cpath_supports_ntor(const origin_circuit_t *circ)
{
  crypt_path_t *head, *cpath;

  cpath = head = circ->cpath;
  do {
    /* if the extend_info is missing, we can't tell if it supports ntor */
    if (!cpath->extend_info) {
      return 0;
    }

    /* if the key is blank, it definitely doesn't support ntor */
    if (!extend_info_supports_ntor(cpath->extend_info)) {
      return 0;
    }
    cpath = cpath->next;
  } while (cpath != head);

  return 1;
}

/** Pick all the entries in our cpath. Stop and return 0 when we're
 * happy, or return -1 if an error occurs. */
static int
onion_populate_cpath(origin_circuit_t *circ)
{
  int r = 0;

  /* onion_extend_cpath assumes these are non-NULL */
  tor_assert(circ);
  tor_assert(circ->build_state);

  while (r == 0) {
    r = onion_extend_cpath(circ);
    if (r < 0) {
      log_info(LD_CIRC,"Generating cpath hop failed.");
      return -1;
    }
  }

  /* The path is complete */
  tor_assert(r == 1);

  /* Does every node in this path support ntor? */
  int path_supports_ntor = circuit_cpath_supports_ntor(circ);

  /* We would like every path to support ntor, but we have to allow for some
   * edge cases. */
  tor_assert(circuit_get_cpath_len(circ));
  if (circuit_can_use_tap(circ)) {
    /* Circuits from clients to intro points, and hidden services to rend
     * points do not support ntor, because the hidden service protocol does
     * not include ntor onion keys. This is also true for Single Onion
     * Services. */
    return 0;
  }

  if (circuit_get_cpath_len(circ) == 1) {
    /* Allow for bootstrapping: when we're fetching directly from a fallback,
     * authority, or bridge, we have no way of knowing its ntor onion key
     * before we connect to it. So instead, we try connecting, and end up using
     * CREATE_FAST. */
    tor_assert(circ->cpath);
    tor_assert(circ->cpath->extend_info);
    const node_t *node = node_get_by_id(
                                    circ->cpath->extend_info->identity_digest);
    /* If we don't know the node and its descriptor, we must be bootstrapping.
     */
    if (!node || !node_has_preferred_descriptor(node, 1)) {
      return 0;
    }
  }

  if (BUG(!path_supports_ntor)) {
    /* If we're building a multi-hop path, and it's not one of the HS or
     * bootstrapping exceptions, and it doesn't support ntor, something has
     * gone wrong. */
    return -1;
  }

  return 0;
}

/** Create and return a new origin circuit. Initialize its purpose and
 * build-state based on our arguments.  The <b>flags</b> argument is a
 * bitfield of CIRCLAUNCH_* flags, see circuit_launch_by_extend_info() for
 * more details. */
origin_circuit_t *
origin_circuit_init(uint8_t purpose, int flags)
{
  /* sets circ->p_circ_id and circ->p_chan */
  origin_circuit_t *circ = origin_circuit_new();
  circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_CHAN_WAIT);
  circ->build_state = tor_malloc_zero(sizeof(cpath_build_state_t));
  circ->build_state->onehop_tunnel =
    ((flags & CIRCLAUNCH_ONEHOP_TUNNEL) ? 1 : 0);
  circ->build_state->need_uptime =
    ((flags & CIRCLAUNCH_NEED_UPTIME) ? 1 : 0);
  circ->build_state->need_capacity =
    ((flags & CIRCLAUNCH_NEED_CAPACITY) ? 1 : 0);
  circ->build_state->is_internal =
    ((flags & CIRCLAUNCH_IS_INTERNAL) ? 1 : 0);
  circ->build_state->is_ipv6_selftest =
    ((flags & CIRCLAUNCH_IS_IPV6_SELFTEST) ? 1 : 0);
  circ->build_state->need_conflux =
    ((flags & CIRCLAUNCH_NEED_CONFLUX) ? 1 : 0);
  circ->base_.purpose = purpose;
  return circ;
}

/** Build a new circuit for <b>purpose</b>. If <b>exit</b> is defined, then use
 * that as your exit router, else choose a suitable exit node. The <b>flags</b>
 * argument is a bitfield of CIRCLAUNCH_* flags, see
 * circuit_launch_by_extend_info() for more details.
 *
 * Also launch a connection to the first OR in the chosen path, if
 * it's not open already.
 */
origin_circuit_t *
circuit_establish_circuit(uint8_t purpose, extend_info_t *exit_ei, int flags)
{
  origin_circuit_t *circ;
  int err_reason = 0;
  int is_hs_v3_rp_circuit = 0;

  if (flags & CIRCLAUNCH_IS_V3_RP) {
    is_hs_v3_rp_circuit = 1;
  }

  circ = origin_circuit_init(purpose, flags);

  if (onion_pick_cpath_exit(circ, exit_ei, is_hs_v3_rp_circuit) < 0 ||
      onion_populate_cpath(circ) < 0) {
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_NOPATH);
    return NULL;
  }

  circuit_event_status(circ, CIRC_EVENT_LAUNCHED, 0);

  if ((err_reason = circuit_handle_first_hop(circ)) < 0) {
    circuit_mark_for_close(TO_CIRCUIT(circ), -err_reason);
    return NULL;
  }

  tor_trace(TR_SUBSYS(circuit), TR_EV(establish), circ);
  return circ;
}

/**
 * Build a new conflux circuit for <b>purpose</b>. If <b>exit</b> is defined,
 * then use that as your exit router, else choose a suitable exit node.
 * The <b>flags</b> argument is a bitfield of CIRCLAUNCH_* flags, see
 * circuit_launch_by_extend_info() for more details.
 *
 * Also launch a connection to the first OR in the chosen path, if
 * it's not open already.
 */
MOCK_IMPL(origin_circuit_t *,
circuit_establish_circuit_conflux,(const uint8_t *conflux_nonce,
                                   uint8_t purpose, extend_info_t *exit_ei,
                                   int flags))
{
  origin_circuit_t *circ;
  int err_reason = 0;

  /* Right now, only conflux client circuits use this function */
  tor_assert(purpose == CIRCUIT_PURPOSE_CONFLUX_UNLINKED);

  circ = origin_circuit_init(purpose, flags);
  TO_CIRCUIT(circ)->conflux_pending_nonce =
    tor_memdup(conflux_nonce, DIGEST256_LEN);

  if (onion_pick_cpath_exit(circ, exit_ei, 0) < 0 ||
      onion_populate_cpath(circ) < 0) {
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_NOPATH);
    return NULL;
  }

  circuit_event_status(circ, CIRC_EVENT_LAUNCHED, 0);

  if ((err_reason = circuit_handle_first_hop(circ)) < 0) {
    circuit_mark_for_close(TO_CIRCUIT(circ), -err_reason);
    return NULL;
  }

  tor_trace(TR_SUBSYS(circuit), TR_EV(establish), circ);
  return circ;
}

/** Return the guard state associated with <b>circ</b>, which may be NULL. */
circuit_guard_state_t *
origin_circuit_get_guard_state(origin_circuit_t *circ)
{
  return circ->guard_state;
}

/**
 * Helper function to publish a channel association message
 *
 * circuit_handle_first_hop() calls this to notify subscribers about a
 * channel launch event, which associates a circuit with a channel.
 * This doesn't always correspond to an assignment of the circuit's
 * n_chan field, because that seems to be only for fully-open
 * channels.
 **/
static void
circuit_chan_publish(const origin_circuit_t *circ, const channel_t *chan)
{
  ocirc_chan_msg_t *msg = tor_malloc(sizeof(*msg));

  msg->gid = circ->global_identifier;
  msg->chan = chan->global_identifier;
  msg->onehop = circ->build_state->onehop_tunnel;

  ocirc_chan_publish(msg);
}

/** Start establishing the first hop of our circuit. Figure out what
 * OR we should connect to, and if necessary start the connection to
 * it. If we're already connected, then send the 'create' cell.
 * Return 0 for ok, -reason if circ should be marked-for-close. */
int
circuit_handle_first_hop(origin_circuit_t *circ)
{
  crypt_path_t *firsthop;
  channel_t *n_chan;
  int err_reason = 0;
  const char *msg = NULL;
  int should_launch = 0;
  const or_options_t *options = get_options();

  firsthop = cpath_get_next_non_open_hop(circ->cpath);
  tor_assert(firsthop);
  tor_assert(firsthop->extend_info);

  /* Some bridges are on private addresses. Others pass a dummy private
   * address to the pluggable transport, which ignores it.
   * Deny the connection if:
   * - the address is internal, and
   * - we're not connecting to a configured bridge, and
   * - we're not configured to allow extends to private addresses. */
  if (extend_info_any_orport_addr_is_internal(firsthop->extend_info) &&
      !extend_info_is_a_configured_bridge(firsthop->extend_info) &&
      !options->ExtendAllowPrivateAddresses) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Client asked me to connect directly to a private address");
    return -END_CIRC_REASON_TORPROTOCOL;
  }

  /* now see if we're already connected to the first OR in 'route' */
  const tor_addr_port_t *orport4 =
    extend_info_get_orport(firsthop->extend_info, AF_INET);
  const tor_addr_port_t *orport6 =
    extend_info_get_orport(firsthop->extend_info, AF_INET6);
  n_chan = channel_get_for_extend(
                          firsthop->extend_info->identity_digest,
                          &firsthop->extend_info->ed_identity,
                          orport4 ? &orport4->addr : NULL,
                          orport6 ? &orport6->addr : NULL,
                          true,
                          &msg,
                          &should_launch);

  if (!n_chan) {
    /* not currently connected in a useful way. */
    log_info(LD_CIRC, "Next router is %s: %s",
             safe_str_client(extend_info_describe(firsthop->extend_info)),
             msg?msg:"???");
    circ->base_.n_hop = extend_info_dup(firsthop->extend_info);

    if (should_launch) {
      n_chan = channel_connect_for_circuit(firsthop->extend_info);
      if (!n_chan) { /* connect failed, forget the whole thing */
        log_info(LD_CIRC,"connect to firsthop failed. Closing.");
        return -END_CIRC_REASON_CONNECTFAILED;
      }
      /* We didn't find a channel, but we're launching one for an origin
       * circuit.  (If we decided not to launch a channel, then we found at
       * least one once good in-progress channel use for this circuit, and
       * marked it in channel_get_for_extend().) */
      channel_mark_as_used_for_origin_circuit(n_chan);
      circuit_chan_publish(circ, n_chan);
    }

    log_debug(LD_CIRC,"connecting in progress (or finished). Good.");
    /* return success. The onion/circuit/etc will be taken care of
     * automatically (may already have been) whenever n_chan reaches
     * OR_CONN_STATE_OPEN.
     */
    return 0;
  } else { /* it's already open. use it. */
    tor_assert(!circ->base_.n_hop);
    circ->base_.n_chan = n_chan;
    /* We found a channel, and we're using it for an origin circuit. */
    channel_mark_as_used_for_origin_circuit(n_chan);
    circuit_chan_publish(circ, n_chan);
    log_debug(LD_CIRC,"Conn open for %s. Delivering first onion skin.",
              safe_str_client(extend_info_describe(firsthop->extend_info)));
    if ((err_reason = circuit_send_next_onion_skin(circ)) < 0) {
      log_info(LD_CIRC,"circuit_send_next_onion_skin failed.");
      circ->base_.n_chan = NULL;
      return err_reason;
    }
  }
  return 0;
}

/** Find any circuits that are waiting on <b>or_conn</b> to become
 * open and get them to send their create cells forward.
 *
 * Status is 1 if connect succeeded, or 0 if connect failed.
 *
 * Close_origin_circuits is 1 if we should close all the origin circuits
 * through this channel, or 0 otherwise.  (This happens when we want to retry
 * an older guard.)
 */
void
circuit_n_chan_done(channel_t *chan, int status, int close_origin_circuits)
{
  smartlist_t *pending_circs;
  int err_reason = 0;

  tor_assert(chan);

  log_debug(LD_CIRC,"chan to %s, status=%d",
            channel_describe_peer(chan), status);

  pending_circs = smartlist_new();
  circuit_get_all_pending_on_channel(pending_circs, chan);

  SMARTLIST_FOREACH_BEGIN(pending_circs, circuit_t *, circ)
    {
      /* These checks are redundant wrt get_all_pending_on_or_conn, but I'm
       * leaving them in in case it's possible for the status of a circuit to
       * change as we're going down the list. */
      if (circ->marked_for_close || circ->n_chan || !circ->n_hop ||
          circ->state != CIRCUIT_STATE_CHAN_WAIT)
        continue;

      const char *rsa_ident = NULL;
      const ed25519_public_key_t *ed_ident = NULL;
      if (! tor_digest_is_zero(circ->n_hop->identity_digest)) {
        rsa_ident = circ->n_hop->identity_digest;
      }
      if (! ed25519_public_key_is_zero(&circ->n_hop->ed_identity)) {
        ed_ident = &circ->n_hop->ed_identity;
      }

      if (rsa_ident == NULL && ed_ident == NULL) {
        /* Look at addr/port. This is an unkeyed connection. */
        if (!channel_matches_extend_info(chan, circ->n_hop))
          continue;
      } else {
        /* We expected a key or keys. See if they matched. */
        if (!channel_remote_identity_matches(chan, rsa_ident, ed_ident))
          continue;

        /* If the channel is canonical, great.  If not, it needs to match
         * the requested address exactly. */
        if (! chan->is_canonical &&
            ! channel_matches_extend_info(chan, circ->n_hop)) {
          continue;
        }
      }
      if (!status) { /* chan failed; close circ */
        log_info(LD_CIRC,"Channel failed; closing circ.");
        circuit_mark_for_close(circ, END_CIRC_REASON_CHANNEL_CLOSED);
        continue;
      }

      if (close_origin_circuits && CIRCUIT_IS_ORIGIN(circ)) {
        log_info(LD_CIRC,"Channel deprecated for origin circs; closing circ.");
        circuit_mark_for_close(circ, END_CIRC_REASON_CHANNEL_CLOSED);
        continue;
      }
      log_debug(LD_CIRC, "Found circ, sending create cell.");
      /* circuit_deliver_create_cell will set n_circ_id and add us to
       * chan_circuid_circuit_map, so we don't need to call
       * set_circid_chan here. */
      circ->n_chan = chan;
      extend_info_free(circ->n_hop);
      circ->n_hop = NULL;

      if (CIRCUIT_IS_ORIGIN(circ)) {
        if ((err_reason =
             circuit_send_next_onion_skin(TO_ORIGIN_CIRCUIT(circ))) < 0) {
          log_info(LD_CIRC,
                   "send_next_onion_skin failed; circuit marked for closing.");
          circuit_mark_for_close(circ, -err_reason);
          continue;
          /* XXX could this be bad, eg if next_onion_skin failed because conn
           *     died? */
        }
      } else {
        /* pull the create cell out of circ->n_chan_create_cell, and send it */
        tor_assert(circ->n_chan_create_cell);
        if (circuit_deliver_create_cell(circ, circ->n_chan_create_cell, 1)<0) {
          circuit_mark_for_close(circ, END_CIRC_REASON_RESOURCELIMIT);
          continue;
        }
        tor_free(circ->n_chan_create_cell);
        circuit_set_state(circ, CIRCUIT_STATE_OPEN);
      }
    }
  SMARTLIST_FOREACH_END(circ);

  smartlist_free(pending_circs);
}

/** Find a new circid that isn't currently in use on the circ->n_chan
 * for the outgoing
 * circuit <b>circ</b>, and deliver the cell <b>create_cell</b> to this
 * circuit.  If <b>relayed</b> is true, this is a create cell somebody
 * gave us via an EXTEND cell, so we shouldn't worry if we don't understand
 * it. Return -1 if we failed to find a suitable circid, else return 0.
 */
MOCK_IMPL(int,
circuit_deliver_create_cell,(circuit_t *circ,
                             const struct create_cell_t *create_cell,
                             int relayed))
{
  cell_t cell;
  circid_t id;
  int r;

  tor_assert(circ);
  tor_assert(circ->n_chan);
  tor_assert(create_cell);
  tor_assert(create_cell->cell_type == CELL_CREATE ||
             create_cell->cell_type == CELL_CREATE_FAST ||
             create_cell->cell_type == CELL_CREATE2);

  id = get_unique_circ_id_by_chan(circ->n_chan);
  if (!id) {
    static ratelim_t circid_warning_limit = RATELIM_INIT(9600);
    log_fn_ratelim(&circid_warning_limit, LOG_WARN, LD_CIRC,
                   "failed to get unique circID.");
    goto error;
  }

  tor_assert_nonfatal_once(circ->n_chan->is_canonical);

  memset(&cell, 0, sizeof(cell_t));
  r = relayed ? create_cell_format_relayed(&cell, create_cell)
              : create_cell_format(&cell, create_cell);
  if (r < 0) {
    log_warn(LD_CIRC,"Couldn't format create cell");
    goto error;
  }
  log_debug(LD_CIRC,"Chosen circID %u.", (unsigned)id);
  circuit_set_n_circid_chan(circ, id, circ->n_chan);
  cell.circ_id = circ->n_circ_id;

  append_cell_to_circuit_queue(circ, circ->n_chan, &cell,
                               CELL_DIRECTION_OUT, 0);

  if (CIRCUIT_IS_ORIGIN(circ)) {
    /* Update began timestamp for circuits starting their first hop */
    if (TO_ORIGIN_CIRCUIT(circ)->cpath->state == CPATH_STATE_CLOSED) {
      if (!CHANNEL_IS_OPEN(circ->n_chan)) {
        log_warn(LD_CIRC,
                 "Got first hop for a circuit without an opened channel. "
                 "State: %s.", channel_state_to_string(circ->n_chan->state));
        tor_fragile_assert();
      }

      tor_gettimeofday(&circ->timestamp_began);
    }

    /* mark it so it gets better rate limiting treatment. */
    channel_timestamp_client(circ->n_chan);
  }

  return 0;
 error:
  circ->n_chan = NULL;
  return -1;
}

/** Return true iff we should send a create_fast cell to start building a
 * given circuit */
static inline bool
should_use_create_fast_for_circuit(origin_circuit_t *circ)
{
  tor_assert(circ->cpath);
  tor_assert(circ->cpath->extend_info);

  return ! circuit_has_usable_onion_key(circ);
}

/**
 * Return true if <b>circ</b> is the type of circuit we want to count
 * timeouts from.
 *
 * In particular, we want to consider any circuit that plans to build
 * at least 3 hops (but maybe more), but has 3 or fewer hops built
 * so far.
 *
 * We still want to consider circuits before 3 hops, because we need
 * to decide if we should convert them to a measurement circuit in
 * circuit_build_times_handle_completed_hop(), rather than letting
 * slow circuits get killed right away.
 */
int
circuit_timeout_want_to_count_circ(const origin_circuit_t *circ)
{
  return !circ->has_opened
          && circ->build_state->desired_path_len >= DEFAULT_ROUTE_LEN
          && circuit_get_cpath_opened_len(circ) <= DEFAULT_ROUTE_LEN;
}

/** Decide whether to use a TAP or ntor handshake for connecting to <b>ei</b>
 * directly, and set *<b>cell_type_out</b> and *<b>handshake_type_out</b>
 * accordingly.
 * Note that TAP handshakes in CREATE cells are only used for direct
 * connections:
 *  - from Single Onions to rend points not in the service's consensus.
 * This is checked in onion_populate_cpath. */
static void
circuit_pick_create_handshake(uint8_t *cell_type_out,
                              uint16_t *handshake_type_out,
                              const extend_info_t *ei)
{
  /* torspec says: In general, clients SHOULD use CREATE whenever they are
   * using the TAP handshake, and CREATE2 otherwise. */
  if (extend_info_supports_ntor(ei)) {
    *cell_type_out = CELL_CREATE2;
    /* Only use ntor v3 with exits that support congestion control,
     * and only when it is enabled. */
    if (ei->exit_supports_congestion_control &&
        congestion_control_enabled())
      *handshake_type_out = ONION_HANDSHAKE_TYPE_NTOR_V3;
    else
      *handshake_type_out = ONION_HANDSHAKE_TYPE_NTOR;
  } else {
    /* XXXX030 Remove support for deciding to use TAP and EXTEND. */
    *cell_type_out = CELL_CREATE;
    *handshake_type_out = ONION_HANDSHAKE_TYPE_TAP;
  }
}

/** Decide whether to use a TAP or ntor handshake for extending to <b>ei</b>
 * and set *<b>handshake_type_out</b> accordingly. Decide whether we should
 * use an EXTEND2 or an EXTEND cell to do so, and set *<b>cell_type_out</b>
 * and *<b>create_cell_type_out</b> accordingly.
 * Note that TAP handshakes in EXTEND cells are only used:
 *  - from clients to intro points, and
 *  - from hidden services to rend points.
 * This is checked in onion_populate_cpath.
 */
static void
circuit_pick_extend_handshake(uint8_t *cell_type_out,
                              uint8_t *create_cell_type_out,
                              uint16_t *handshake_type_out,
                              const extend_info_t *ei)
{
  uint8_t t;
  circuit_pick_create_handshake(&t, handshake_type_out, ei);

  /* torspec says: Clients SHOULD use the EXTEND format whenever sending a TAP
   * handshake... In other cases, clients SHOULD use EXTEND2. */
  if (*handshake_type_out != ONION_HANDSHAKE_TYPE_TAP) {
    *cell_type_out = RELAY_COMMAND_EXTEND2;
    *create_cell_type_out = CELL_CREATE2;
  } else {
    /* XXXX030 Remove support for deciding to use TAP and EXTEND. */
    *cell_type_out = RELAY_COMMAND_EXTEND;
    *create_cell_type_out = CELL_CREATE;
  }
}

/**
 * Return true iff <b>circ</b> is allowed
 * to have no guard configured, even if the circuit is multihop
 * and guards are enabled.
 */
static int
circuit_may_omit_guard(const origin_circuit_t *circ)
{
  if (BUG(!circ))
    return 0;

  if (circ->first_hop_from_controller) {
    /* The controller picked the first hop: that bypasses the guard system. */
    return 1;
  }

  switch (circ->base_.purpose) {
    case CIRCUIT_PURPOSE_TESTING:
    case CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT:
      /* Testing circuits may omit guards because they're measuring
       * liveness or performance, and don't want guards to interfere. */
      return 1;
    default:
      /* All other multihop circuits should use guards if guards are
       * enabled. */
      return 0;
  }
}

/** This is the backbone function for building circuits.
 *
 * If circ's first hop is closed, then we need to build a create
 * cell and send it forward.
 *
 * Otherwise, if circ's cpath still has any non-open hops, we need to
 * build a relay extend cell and send it forward to the next non-open hop.
 *
 * If all hops on the cpath are open, we're done building the circuit
 * and we should do housekeeping for the newly opened circuit.
 *
 * Return -reason if we want to tear down circ, else return 0.
 */
int
circuit_send_next_onion_skin(origin_circuit_t *circ)
{
  tor_assert(circ);

  if (circ->cpath->state == CPATH_STATE_CLOSED) {
    /* Case one: we're on the first hop. */
    return circuit_send_first_onion_skin(circ);
  }

  tor_assert(circ->cpath->state == CPATH_STATE_OPEN);
  tor_assert(circ->base_.state == CIRCUIT_STATE_BUILDING);

  crypt_path_t *hop = cpath_get_next_non_open_hop(circ->cpath);
  circuit_build_times_handle_completed_hop(circ);

  circpad_machine_event_circ_added_hop(circ);

  if (hop) {
    /* Case two: we're on a hop after the first. */
    return circuit_send_intermediate_onion_skin(circ, hop);
  }

  /* Case three: the circuit is finished. Do housekeeping tasks on it. */
  circpad_machine_event_circ_built(circ);
  return circuit_build_no_more_hops(circ);
}

/**
 * Called from circuit_send_next_onion_skin() when we find ourselves connected
 * to the first hop in <b>circ</b>: Send a CREATE or CREATE2 or CREATE_FAST
 * cell to that hop.  Return 0 on success; -reason on failure (if the circuit
 * should be torn down).
 */
static int
circuit_send_first_onion_skin(origin_circuit_t *circ)
{
  int fast;
  int len;
  const node_t *node;
  create_cell_t cc;
  memset(&cc, 0, sizeof(cc));

  log_debug(LD_CIRC,"First skin; sending create cell.");

  if (circ->build_state->onehop_tunnel) {
    control_event_bootstrap(BOOTSTRAP_STATUS_ONEHOP_CREATE, 0);
  } else {
    control_event_bootstrap(BOOTSTRAP_STATUS_CIRCUIT_CREATE, 0);

    /* If this is not a one-hop tunnel, the channel is being used
     * for traffic that wants anonymity and protection from traffic
     * analysis (such as netflow record retention). That means we want
     * to pad it.
     */
    if (circ->base_.n_chan->channel_usage < CHANNEL_USED_FOR_FULL_CIRCS)
      circ->base_.n_chan->channel_usage = CHANNEL_USED_FOR_FULL_CIRCS;
  }

  node = node_get_by_id(circ->base_.n_chan->identity_digest);
  fast = should_use_create_fast_for_circuit(circ);
  if (!fast) {
    /* We know the right onion key: we should send a create cell. */
    circuit_pick_create_handshake(&cc.cell_type, &cc.handshake_type,
                                  circ->cpath->extend_info);
  } else {
    /* We don't know an onion key, so we need to fall back to CREATE_FAST. */
    cc.cell_type = CELL_CREATE_FAST;
    cc.handshake_type = ONION_HANDSHAKE_TYPE_FAST;
  }

  len = onion_skin_create(cc.handshake_type,
                          circ->cpath->extend_info,
                          &circ->cpath->handshake_state,
                          cc.onionskin,
                          sizeof(cc.onionskin));
  if (len < 0) {
    log_warn(LD_CIRC,"onion_skin_create (first hop) failed.");
    return - END_CIRC_REASON_INTERNAL;
  }
  cc.handshake_len = len;

  if (circuit_deliver_create_cell(TO_CIRCUIT(circ), &cc, 0) < 0)
    return - END_CIRC_REASON_RESOURCELIMIT;
  tor_trace(TR_SUBSYS(circuit), TR_EV(first_onion_skin), circ, circ->cpath);

  circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
  circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_BUILDING);
  log_info(LD_CIRC,"First hop: finished sending %s cell to '%s'",
           fast ? "CREATE_FAST" : "CREATE",
           node ? node_describe(node) : "<unnamed>");
  return 0;
}

/**
 * Called from circuit_send_next_onion_skin() when we find that we have no
 * more hops: mark the circuit as finished, and perform the necessary
 * bookkeeping.  Return 0 on success; -reason on failure (if the circuit
 * should be torn down).
 */
static int
circuit_build_no_more_hops(origin_circuit_t *circ)
{
  guard_usable_t r;
  if (! circ->guard_state) {
    if (circuit_get_cpath_len(circ) != 1 &&
        ! circuit_may_omit_guard(circ) &&
        get_options()->UseEntryGuards) {
      log_warn(LD_BUG, "%d-hop circuit %p with purpose %d has no "
               "guard state",
               circuit_get_cpath_len(circ), circ, circ->base_.purpose);
    }
    r = GUARD_USABLE_NOW;
  } else {
    r = entry_guard_succeeded(&circ->guard_state);
  }
  const int is_usable_for_streams = (r == GUARD_USABLE_NOW);
  if (r == GUARD_USABLE_NOW) {
    circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_OPEN);
  } else if (r == GUARD_MAYBE_USABLE_LATER) {
    // Wait till either a better guard succeeds, or till
    // all better guards fail.
    circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_GUARD_WAIT);
  } else {
    tor_assert_nonfatal(r == GUARD_USABLE_NEVER);
    return - END_CIRC_REASON_INTERNAL;
  }

  /* XXXX #21422 -- the rest of this branch needs careful thought!
   * Some of the things here need to happen when a circuit becomes
   * mechanically open; some need to happen when it is actually usable.
   * I think I got them right, but more checking would be wise. -NM
   */

  log_info(LD_CIRC,"circuit built!");
  circuit_reset_failure_count(0);

  if (circ->build_state->onehop_tunnel || circ->has_opened) {
    control_event_bootstrap(BOOTSTRAP_STATUS_REQUESTING_STATUS, 0);
  }

  pathbias_count_build_success(circ);
  if (is_usable_for_streams)
    circuit_has_opened(circ); /* do other actions as necessary */

  if (!have_completed_a_circuit() && !circ->build_state->onehop_tunnel) {
    const or_options_t *options = get_options();
    note_that_we_completed_a_circuit();
    /* FFFF Log a count of known routers here */
    log_info(LD_GENERAL,
             "Tor has successfully opened a circuit. "
             "Looks like client functionality is working.");
    control_event_bootstrap(BOOTSTRAP_STATUS_DONE, 0);
    control_event_client_status(LOG_NOTICE, "CIRCUIT_ESTABLISHED");
    clear_broken_connection_map(1);
    if (server_mode(options) &&
        !router_all_orports_seem_reachable(options)) {
      router_do_reachability_checks();
    }
  }

  /* We're done with measurement circuits here. Just close them */
  if (circ->base_.purpose == CIRCUIT_PURPOSE_C_MEASURE_TIMEOUT) {
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_FINISHED);
  }
  return 0;
}

/**
 * Called from circuit_send_next_onion_skin() when we find that we have a hop
 * other than the first that we need to extend to: use <b>hop</b>'s
 * information to extend the circuit another step. Return 0 on success;
 * -reason on failure (if the circuit should be torn down).
 */
static int
circuit_send_intermediate_onion_skin(origin_circuit_t *circ,
                                     crypt_path_t *hop)
{
  int len;
  extend_cell_t ec;
  /* Relays and bridges can send IPv6 extends. But for clients, it's an
   * obvious version distinguisher. */
  const bool include_ipv6 = server_mode(get_options());
  memset(&ec, 0, sizeof(ec));
  tor_addr_make_unspec(&ec.orport_ipv4.addr);
  tor_addr_make_unspec(&ec.orport_ipv6.addr);

  log_debug(LD_CIRC,"starting to send subsequent skin.");

  circuit_pick_extend_handshake(&ec.cell_type,
                                &ec.create_cell.cell_type,
                                &ec.create_cell.handshake_type,
                                hop->extend_info);

  const tor_addr_port_t *orport4 =
    extend_info_get_orport(hop->extend_info, AF_INET);
  const tor_addr_port_t *orport6 =
    extend_info_get_orport(hop->extend_info, AF_INET6);
  int n_addrs_set = 0;
  if (orport4) {
    tor_addr_copy(&ec.orport_ipv4.addr, &orport4->addr);
    ec.orport_ipv4.port = orport4->port;
    ++n_addrs_set;
  }
  if (orport6 && include_ipv6) {
    tor_addr_copy(&ec.orport_ipv6.addr, &orport6->addr);
    ec.orport_ipv6.port = orport6->port;
    ++n_addrs_set;
  }

  if (n_addrs_set == 0) {
    log_warn(LD_BUG, "No supported address family found in extend_info.");
    return - END_CIRC_REASON_INTERNAL;
  }
  memcpy(ec.node_id, hop->extend_info->identity_digest, DIGEST_LEN);
  /* Set the ED25519 identity too -- it will only get included
   * in the extend2 cell if we're configured to use it, though. */
  ed25519_pubkey_copy(&ec.ed_pubkey, &hop->extend_info->ed_identity);

  len = onion_skin_create(ec.create_cell.handshake_type,
                          hop->extend_info,
                          &hop->handshake_state,
                          ec.create_cell.onionskin,
                          sizeof(ec.create_cell.onionskin));
  if (len < 0) {
    log_warn(LD_CIRC,"onion_skin_create failed.");
    return - END_CIRC_REASON_INTERNAL;
  }
  ec.create_cell.handshake_len = len;

  log_info(LD_CIRC,"Sending extend relay cell.");
  {
    uint8_t command = 0;
    uint16_t payload_len=0;
    uint8_t payload[RELAY_PAYLOAD_SIZE];
    if (extend_cell_format(&command, &payload_len, payload, &ec)<0) {
      log_warn(LD_CIRC,"Couldn't format extend cell");
      return -END_CIRC_REASON_INTERNAL;
    }

    /* send it to hop->prev, because that relay will transfer
     * it to a create cell and then send to hop */
    if (relay_send_command_from_edge(0, TO_CIRCUIT(circ),
                                     command,
                                     (char*)payload, payload_len,
                                     hop->prev) < 0)
      return 0; /* circuit is closed */
  }
  hop->state = CPATH_STATE_AWAITING_KEYS;
  tor_trace(TR_SUBSYS(circuit), TR_EV(intermediate_onion_skin), circ, hop);
  return 0;
}

/** Our clock just jumped by <b>seconds_elapsed</b>. If <b>was_idle</b> is
 * true, then the monotonic time matches; otherwise it doesn't. Assume
 * something has also gone wrong with our network: notify the user, and
 * abandon all not-yet-used circuits. */
void
circuit_note_clock_jumped(int64_t seconds_elapsed, bool was_idle)
{
  int severity = server_mode(get_options()) ? LOG_WARN : LOG_NOTICE;
  if (was_idle) {
    tor_log(severity, LD_GENERAL, "Tor has been idle for %"PRId64
            " seconds; assuming established circuits no longer work.",
            (seconds_elapsed));
  } else {
    tor_log(severity, LD_GENERAL,
            "Your system clock just jumped %"PRId64" seconds %s; "
            "assuming established circuits no longer work.",
            (
                 seconds_elapsed >=0 ? seconds_elapsed : -seconds_elapsed),
            seconds_elapsed >=0 ? "forward" : "backward");
  }
  control_event_general_status(LOG_WARN, "CLOCK_JUMPED TIME=%"PRId64
                               " IDLE=%d",
                               (seconds_elapsed), was_idle?1:0);
  /* so we log when it works again */
  note_that_we_maybe_cant_complete_circuits();
  control_event_client_status(severity, "CIRCUIT_NOT_ESTABLISHED REASON=%s",
                              "CLOCK_JUMPED");
  circuit_mark_all_unused_circs();
  circuit_mark_all_dirty_circs_as_unusable();
  if (seconds_elapsed < 0) {
    /* Restart all the timers in case we jumped a long way into the past. */
    reset_all_main_loop_timers();
  }
}

/** A "created" cell <b>reply</b> came back to us on circuit <b>circ</b>.
 * (The body of <b>reply</b> varies depending on what sort of handshake
 * this is.)
 *
 * Calculate the appropriate keys and digests, make sure KH is
 * correct, and initialize this hop of the cpath.
 *
 * Return - reason if we want to mark circ for close, else return 0.
 */
int
circuit_finish_handshake(origin_circuit_t *circ,
                         const created_cell_t *reply)
{
  char keys[CPATH_KEY_MATERIAL_LEN];
  crypt_path_t *hop;
  int rv;

  if ((rv = pathbias_count_build_attempt(circ)) < 0) {
    log_warn(LD_CIRC, "pathbias_count_build_attempt failed: %d", rv);
    return rv;
  }

  if (circ->cpath->state == CPATH_STATE_AWAITING_KEYS) {
    hop = circ->cpath;
  } else {
    hop = cpath_get_next_non_open_hop(circ->cpath);
    if (!hop) { /* got an extended when we're all done? */
      log_warn(LD_PROTOCOL,"got extended when circ already built? Closing.");
      return - END_CIRC_REASON_TORPROTOCOL;
    }
  }
  tor_assert(hop->state == CPATH_STATE_AWAITING_KEYS);

  circuit_params_t params;
  {
    const char *msg = NULL;
    if (onion_skin_client_handshake(hop->handshake_state.tag,
                                    &hop->handshake_state,
                                    reply->reply, reply->handshake_len,
                                    (uint8_t*)keys, sizeof(keys),
                                    (uint8_t*)hop->rend_circ_nonce,
                                    &params,
                                    &msg) < 0) {
      if (msg)
        log_warn(LD_CIRC,"onion_skin_client_handshake failed: %s", msg);
      return -END_CIRC_REASON_TORPROTOCOL;
    }
  }

  onion_handshake_state_release(&hop->handshake_state);

  if (cpath_init_circuit_crypto(hop, keys, sizeof(keys), 0, 0)<0) {
    return -END_CIRC_REASON_TORPROTOCOL;
  }

  if (params.cc_enabled) {
    int circ_len = circuit_get_cpath_len(circ);

    if (circ_len == DEFAULT_ROUTE_LEN &&
        circuit_get_cpath_hop(circ, DEFAULT_ROUTE_LEN) == hop) {
      hop->ccontrol = congestion_control_new(&params, CC_PATH_EXIT);
    } else if (circ_len == SBWS_ROUTE_LEN &&
               circuit_get_cpath_hop(circ, SBWS_ROUTE_LEN) == hop) {
      hop->ccontrol = congestion_control_new(&params, CC_PATH_SBWS);
    } else {
      if (circ_len > DEFAULT_ROUTE_LEN) {
        /* This can happen for unknown reasons; cannibalization codepaths
         * don't seem able to do it, so there is some magic way that hops can
         * still get added. Perhaps some cases of circuit pre-build that change
         * purpose? */
        log_info(LD_CIRC,
                   "Unexpected path length %d for exit circuit %d, purpose %d",
                    circ_len, circ->global_identifier,
                    TO_CIRCUIT(circ)->purpose);
        hop->ccontrol = congestion_control_new(&params, CC_PATH_EXIT);
      } else {
        /* This is likely directory requests, which should block on orconn
         * before congestion control, but lets give them the lower sbws
         * param set anyway just in case. */
        log_info(LD_CIRC,
                 "Unexpected path length %d for exit circuit %d, purpose %d",
                 circ_len, circ->global_identifier,
                 TO_CIRCUIT(circ)->purpose);

        hop->ccontrol = congestion_control_new(&params, CC_PATH_SBWS);
      }
    }
  }

  hop->state = CPATH_STATE_OPEN;
  log_info(LD_CIRC,"Finished building circuit hop:");
  circuit_log_path(LOG_INFO,LD_CIRC,circ);
  circuit_event_status(circ, CIRC_EVENT_EXTENDED, 0);

  return 0;
}

/** We received a relay truncated cell on circ.
 *
 * Since we don't send truncates currently, getting a truncated
 * means that a connection broke or an extend failed. For now,
 * just give up: force circ to close, and return 0.
 */
int
circuit_truncated(origin_circuit_t *circ, int reason)
{
//  crypt_path_t *victim;
//  connection_t *stream;

  tor_assert(circ);

  /* XXX Since we don't send truncates currently, getting a truncated
   *     means that a connection broke or an extend failed. For now,
   *     just give up.
   */
  circuit_mark_for_close(TO_CIRCUIT(circ),
          END_CIRC_REASON_FLAG_REMOTE|reason);
  return 0;

#if 0
  while (layer->next != circ->cpath) {
    /* we need to clear out layer->next */
    victim = layer->next;
    log_debug(LD_CIRC, "Killing a layer of the cpath.");

    for (stream = circ->p_streams; stream; stream=stream->next_stream) {
      if (stream->cpath_layer == victim) {
        log_info(LD_APP, "Marking stream %d for close because of truncate.",
                 stream->stream_id);
        /* no need to send 'end' relay cells,
         * because the other side's already dead
         */
        connection_mark_unattached_ap(stream, END_STREAM_REASON_DESTROY);
      }
    }

    layer->next = victim->next;
    cpath_free(victim);
  }

  log_info(LD_CIRC, "finished");
  return 0;
#endif /* 0 */
}

/** Helper for new_route_len().  Choose a circuit length for purpose
 * <b>purpose</b>: DEFAULT_ROUTE_LEN (+ 1 if someone else chose the
 * exit).  If someone else chose the exit, they could be colluding
 * with the exit, so add a randomly selected node to preserve
 * anonymity.
 *
 * Here, "exit node" sometimes means an OR acting as an internal
 * endpoint, rather than as a relay to an external endpoint.  This
 * means there need to be at least DEFAULT_ROUTE_LEN routers between
 * us and the internal endpoint to preserve the same anonymity
 * properties that we would get when connecting to an external
 * endpoint.  These internal endpoints can include:
 *
 *   - Connections to a directory of hidden services
 *     (CIRCUIT_PURPOSE_C_GENERAL)
 *
 *   - A client connecting to an introduction point, which the hidden
 *     service picked (CIRCUIT_PURPOSE_C_INTRODUCING, via
 *     circuit_get_open_circ_or_launch() which rewrites it from
 *     CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT)
 *
 *   - A hidden service connecting to a rendezvous point, which the
 *     client picked (CIRCUIT_PURPOSE_S_CONNECT_REND.
 *
 * There are currently two situations where we picked the exit node
 * ourselves, making DEFAULT_ROUTE_LEN a safe circuit length:
 *
 *   - We are a hidden service connecting to an introduction point
 *     (CIRCUIT_PURPOSE_S_ESTABLISH_INTRO).
 *
 *   - We are a router testing its own reachabiity
 *     (CIRCUIT_PURPOSE_TESTING, via router_do_reachability_checks())
 *
 * onion_pick_cpath_exit() bypasses us (by not calling
 * new_route_len()) in the one-hop tunnel case, so we don't need to
 * handle that.
 */
int
route_len_for_purpose(uint8_t purpose, extend_info_t *exit_ei)
{
  int routelen = DEFAULT_ROUTE_LEN;
  int known_purpose = 0;

  /* If we're using L3 vanguards, we need longer paths for onion services */
  if (circuit_purpose_is_hidden_service(purpose) &&
      get_options()->HSLayer3Nodes) {
    /* Clients want an extra hop for rends to avoid linkability.
     * Services want it for intro points to avoid publishing their
     * layer3 guards. They want it for hsdir posts to use
     * their full layer3 guard set for those connections.
     * Ex: C - G - L2 - L3 - R
     *     S - G - L2 - L3 - HSDIR
     *     S - G - L2 - L3 - I
     */
    if (purpose == CIRCUIT_PURPOSE_C_ESTABLISH_REND ||
        purpose == CIRCUIT_PURPOSE_S_HSDIR_POST ||
        purpose == CIRCUIT_PURPOSE_HS_VANGUARDS ||
        purpose == CIRCUIT_PURPOSE_S_ESTABLISH_INTRO)
      return routelen+1;

    /* For connections to hsdirs, clients want two extra hops
     * when using layer3 guards, to avoid linkability.
     * Same goes for intro points. Note that the route len
     * includes the intro point or hsdir, hence the +2.
     * Ex: C - G - L2 - L3 - M - I
     *     C - G - L2 - L3 - M - HSDIR
     *     S - G - L2 - L3 - M - R
     */
    if (purpose == CIRCUIT_PURPOSE_S_CONNECT_REND ||
        purpose == CIRCUIT_PURPOSE_C_HSDIR_GET ||
        purpose == CIRCUIT_PURPOSE_C_INTRODUCING)
      return routelen+2;
  }

  if (!exit_ei)
    return routelen;

  switch (purpose) {
    /* These purposes connect to a router that we chose, so DEFAULT_ROUTE_LEN
     * is safe: */
  case CIRCUIT_PURPOSE_CONFLUX_UNLINKED:
  case CIRCUIT_PURPOSE_TESTING:
    /* router reachability testing */
    known_purpose = 1;
    break;

    /* These purposes connect to a router that someone else
     * might have chosen, so add an extra hop to protect anonymity. */
  case CIRCUIT_PURPOSE_C_GENERAL:
  case CIRCUIT_PURPOSE_C_HSDIR_GET:
  case CIRCUIT_PURPOSE_S_HSDIR_POST:
    /* connecting to hidden service directory */
  case CIRCUIT_PURPOSE_C_INTRODUCING:
    /* client connecting to introduction point */
  case CIRCUIT_PURPOSE_S_CONNECT_REND:
    /* hidden service connecting to rendezvous point */
  case CIRCUIT_PURPOSE_S_ESTABLISH_INTRO:
    /* hidden service connecting to intro point. In this case we want an extra
       hop to avoid linkability attacks by the introduction point. */
    known_purpose = 1;
    routelen++;
    break;

  default:
    /* Got a purpose not listed above along with a chosen exit.
     * Increase the circuit length by one anyway for safety. */
    routelen++;
    break;
  }

  if (BUG(exit_ei && !known_purpose)) {
    log_warn(LD_BUG, "Unhandled purpose %d with a chosen exit; "
             "assuming routelen %d.", purpose, routelen);
  }
  return routelen;
}

/** Choose a length for a circuit of purpose <b>purpose</b> and check
 * if enough routers are available.
 *
 * If the routerlist <b>nodes</b> doesn't have enough routers
 * to handle the desired path length, return -1.
 */
STATIC int
new_route_len(uint8_t purpose, extend_info_t *exit_ei,
              const smartlist_t *nodes)
{
  int routelen;

  tor_assert(nodes);

  routelen = route_len_for_purpose(purpose, exit_ei);

  int num_acceptable_direct = count_acceptable_nodes(nodes, 1);
  int num_acceptable_indirect = count_acceptable_nodes(nodes, 0);

  log_debug(LD_CIRC,"Chosen route length %d (%d direct and %d indirect "
             "routers suitable).", routelen, num_acceptable_direct,
             num_acceptable_indirect);

  if (num_acceptable_direct < 1 || num_acceptable_indirect < routelen - 1) {
    log_info(LD_CIRC,
             "Not enough acceptable routers (%d/%d direct and %d/%d "
             "indirect routers suitable). Discarding this circuit.",
             num_acceptable_direct, routelen,
             num_acceptable_indirect, routelen);
    return -1;
  }

  return routelen;
}

/** Return a newly allocated list of uint16_t * for each predicted port not
 * handled by a current circuit. */
static smartlist_t *
circuit_get_unhandled_ports(time_t now)
{
  smartlist_t *dest = rep_hist_get_predicted_ports(now);
  circuit_remove_handled_ports(dest);
  return dest;
}

/** Return 1 if we already have circuits present or on the way for
 * all anticipated ports. Return 0 if we should make more.
 *
 * If we're returning 0, set need_uptime and need_capacity to
 * indicate any requirements that the unhandled ports have.
 */
MOCK_IMPL(int,
circuit_all_predicted_ports_handled, (time_t now, int *need_uptime,
                                      int *need_capacity))
{
  int i, enough;
  uint16_t *port;
  smartlist_t *sl = circuit_get_unhandled_ports(now);
  smartlist_t *LongLivedServices = get_options()->LongLivedPorts;
  tor_assert(need_uptime);
  tor_assert(need_capacity);
  // Always predict need_capacity
  *need_capacity = 1;
  enough = (smartlist_len(sl) == 0);
  for (i = 0; i < smartlist_len(sl); ++i) {
    port = smartlist_get(sl, i);
    if (smartlist_contains_int_as_string(LongLivedServices, *port))
      *need_uptime = 1;
    tor_free(port);
  }
  smartlist_free(sl);
  return enough;
}

/** Return 1 if <b>node</b> can handle one or more of the ports in
 * <b>needed_ports</b>, else return 0.
 */
static int
node_handles_some_port(const node_t *node, smartlist_t *needed_ports)
{ /* XXXX MOVE */
  int i;
  uint16_t port;

  for (i = 0; i < smartlist_len(needed_ports); ++i) {
    addr_policy_result_t r;
    /* alignment issues aren't a worry for this dereference, since
       needed_ports is explicitly a smartlist of uint16_t's */
    port = *(uint16_t *)smartlist_get(needed_ports, i);
    tor_assert(port);
    if (node)
      r = compare_tor_addr_to_node_policy(NULL, port, node);
    else
      continue;
    if (r != ADDR_POLICY_REJECTED && r != ADDR_POLICY_PROBABLY_REJECTED)
      return 1;
  }
  return 0;
}

/** Return true iff <b>conn</b> needs another general circuit to be
 * built. */
static int
ap_stream_wants_exit_attention(connection_t *conn)
{
  entry_connection_t *entry;
  if (conn->type != CONN_TYPE_AP)
    return 0;
  entry = TO_ENTRY_CONN(conn);

  if (conn->state == AP_CONN_STATE_CIRCUIT_WAIT &&
      !conn->marked_for_close &&
      !(entry->want_onehop) && /* ignore one-hop streams */
      !(entry->use_begindir) && /* ignore targeted dir fetches */
      !(entry->chosen_exit_name) && /* ignore defined streams */
      !connection_edge_is_rendezvous_stream(TO_EDGE_CONN(conn)) &&
      !circuit_stream_is_being_handled(TO_ENTRY_CONN(conn), 0,
                                       MIN_CIRCUITS_HANDLING_STREAM))
    return 1;
  return 0;
}

/** Return a pointer to a suitable router to be the exit node for the
 * general-purpose circuit we're about to build.
 *
 * Look through the connection array, and choose a router that maximizes
 * the number of pending streams that can exit from this router.
 *
 * Return NULL if we can't find any suitable routers.
 */
static const node_t *
choose_good_exit_server_general(router_crn_flags_t flags)
{
  int *n_supported;
  int n_pending_connections = 0;
  smartlist_t *connections;
  int best_support = -1;
  int n_best_support=0;
  const or_options_t *options = get_options();
  const smartlist_t *the_nodes;
  const node_t *selected_node=NULL;
  const int need_uptime = (flags & CRN_NEED_UPTIME) != 0;
  const int need_capacity = (flags & CRN_NEED_CAPACITY) != 0;

  /* We should not require guard flags on exits. */
  IF_BUG_ONCE(flags & CRN_NEED_GUARD)
    return NULL;

  /* We reject single-hop exits for all node positions. */
  IF_BUG_ONCE(flags & CRN_DIRECT_CONN)
    return NULL;

  /* This isn't the function for picking rendezvous nodes. */
  IF_BUG_ONCE(flags & CRN_RENDEZVOUS_V3)
    return NULL;

  /* We only want exits to extend if we cannibalize the circuit.
   * But we don't require IPv6 extends yet. */
  IF_BUG_ONCE(flags & CRN_INITIATE_IPV6_EXTEND)
    return NULL;

  connections = get_connection_array();

  /* Count how many connections are waiting for a circuit to be built.
   * We use this for log messages now, but in the future we may depend on it.
   */
  SMARTLIST_FOREACH(connections, connection_t *, conn,
  {
    if (ap_stream_wants_exit_attention(conn))
      ++n_pending_connections;
  });
//  log_fn(LOG_DEBUG, "Choosing exit node; %d connections are pending",
//         n_pending_connections);
  /* Now we count, for each of the routers in the directory, how many
   * of the pending connections could possibly exit from that
   * router (n_supported[i]). (We can't be sure about cases where we
   * don't know the IP address of the pending connection.)
   *
   * -1 means "Don't use this router at all."
   */
  the_nodes = nodelist_get_list();
  n_supported = tor_calloc(smartlist_len(the_nodes), sizeof(int));
  SMARTLIST_FOREACH_BEGIN(the_nodes, const node_t *, node) {
    const int i = node_sl_idx;
    if (router_digest_is_me(node->identity)) {
      n_supported[i] = -1;
//      log_fn(LOG_DEBUG,"Skipping node %s -- it's me.", router->nickname);
      /* XXX there's probably a reverse predecessor attack here, but
       * it's slow. should we take this out? -RD
       */
      continue;
    }
    if (!router_can_choose_node(node, flags)) {
      n_supported[i] = -1;
      continue;
    }
    if (node->is_bad_exit) {
      n_supported[i] = -1;
      continue; /* skip routers that are known to be down or bad exits */
    }
    if (routerset_contains_node(options->ExcludeExitNodesUnion_, node)) {
      n_supported[i] = -1;
      continue; /* user asked us not to use it, no matter what */
    }
    if (options->ExitNodes &&
        !routerset_contains_node(options->ExitNodes, node)) {
      n_supported[i] = -1;
      continue; /* not one of our chosen exit nodes */
    }
    if (node_exit_policy_rejects_all(node)) {
      n_supported[i] = -1;
//      log_fn(LOG_DEBUG,"Skipping node %s (index %d) -- it rejects all.",
//             router->nickname, i);
      continue; /* skip routers that reject all */
    }
    n_supported[i] = 0;
    /* iterate over connections */
    SMARTLIST_FOREACH_BEGIN(connections, connection_t *, conn) {
      if (!ap_stream_wants_exit_attention(conn))
        continue; /* Skip everything but APs in CIRCUIT_WAIT */
      if (connection_ap_can_use_exit(TO_ENTRY_CONN(conn), node)) {
        ++n_supported[i];
//        log_fn(LOG_DEBUG,"%s is supported. n_supported[%d] now %d.",
//               router->nickname, i, n_supported[i]);
      } else {
//        log_fn(LOG_DEBUG,"%s (index %d) would reject this stream.",
//               router->nickname, i);
      }
    } SMARTLIST_FOREACH_END(conn);
    if (n_pending_connections > 0 && n_supported[i] == 0) {
      /* Leave best_support at -1 if that's where it is, so we can
       * distinguish it later. */
      continue;
    }
    if (n_supported[i] > best_support) {
      /* If this router is better than previous ones, remember its index
       * and goodness, and start counting how many routers are this good. */
      best_support = n_supported[i]; n_best_support=1;
//      log_fn(LOG_DEBUG,"%s is new best supported option so far.",
//             router->nickname);
    } else if (n_supported[i] == best_support) {
      /* If this router is _as good_ as the best one, just increment the
       * count of equally good routers.*/
      ++n_best_support;
    }
  } SMARTLIST_FOREACH_END(node);
  log_info(LD_CIRC,
           "Found %d servers that might support %d/%d pending connections.",
           n_best_support, best_support >= 0 ? best_support : 0,
           n_pending_connections);

  /* If any routers definitely support any pending connections, choose one
   * at random. */
  if (best_support > 0) {
    smartlist_t *supporting = smartlist_new();

    SMARTLIST_FOREACH(the_nodes, const node_t *, node, {
      if (n_supported[node_sl_idx] == best_support)
        smartlist_add(supporting, (void*)node);
    });

    selected_node = node_sl_choose_by_bandwidth(supporting, WEIGHT_FOR_EXIT);
    smartlist_free(supporting);
  } else {
    /* Either there are no pending connections, or no routers even seem to
     * possibly support any of them.  Choose a router at random that satisfies
     * at least one predicted exit port. */

    int attempt;
    smartlist_t *needed_ports, *supporting;

    if (best_support == -1) {
      if (need_uptime || need_capacity) {
        log_info(LD_CIRC,
                 "We couldn't find any live%s%s routers; falling back "
                 "to list of all routers.",
                 need_capacity?", fast":"",
                 need_uptime?", stable":"");
        tor_free(n_supported);
        flags &= ~(CRN_NEED_UPTIME|CRN_NEED_CAPACITY);
        return choose_good_exit_server_general(flags);
      }
      log_notice(LD_CIRC, "All routers are down or won't exit%s -- "
                 "choosing a doomed exit at random.",
                 options->ExcludeExitNodesUnion_ ? " or are Excluded" : "");
    }
    supporting = smartlist_new();
    needed_ports = circuit_get_unhandled_ports(time(NULL));
    for (attempt = 0; attempt < 2; attempt++) {
      /* try once to pick only from routers that satisfy a needed port,
       * then if there are none, pick from any that support exiting. */
      SMARTLIST_FOREACH_BEGIN(the_nodes, const node_t *, node) {
        if (n_supported[node_sl_idx] != -1 &&
            (attempt || node_handles_some_port(node, needed_ports))) {
//          log_fn(LOG_DEBUG,"Try %d: '%s' is a possibility.",
//                 try, router->nickname);
          smartlist_add(supporting, (void*)node);
        }
      } SMARTLIST_FOREACH_END(node);

      selected_node = node_sl_choose_by_bandwidth(supporting, WEIGHT_FOR_EXIT);
      if (selected_node)
        break;
      smartlist_clear(supporting);
      /* If we reach this point, we can't actually support any unhandled
       * predicted ports, so clear all the remaining ones. */
      if (smartlist_len(needed_ports))
        rep_hist_remove_predicted_ports(needed_ports);
    }
    SMARTLIST_FOREACH(needed_ports, uint16_t *, cp, tor_free(cp));
    smartlist_free(needed_ports);
    smartlist_free(supporting);
  }

  tor_free(n_supported);
  if (selected_node) {
    log_info(LD_CIRC, "Chose exit server '%s'", node_describe(selected_node));
    return selected_node;
  }
  if (options->ExitNodes) {
    log_warn(LD_CIRC,
             "No exits in ExitNodes%s seem to be running: "
             "can't choose an exit.",
             options->ExcludeExitNodesUnion_ ?
             ", except possibly those excluded by your configuration, " : "");
  }
  return NULL;
}

/* Pick a Rendezvous Point for our HS circuits according to <b>flags</b>. */
static const node_t *
pick_rendezvous_node(router_crn_flags_t flags)
{
  const or_options_t *options = get_options();
  return router_choose_random_node(NULL, options->ExcludeNodes, flags);
}

/*
 * Helper function to pick a configured restricted middle node
 * (either HSLayer2Nodes or HSLayer3Nodes).
 *
 * Make sure that the node we chose is alive, and not excluded,
 * and return it.
 *
 * The exclude_set is a routerset of nodes that the selected node
 * must not match, and the exclude_list is a simple list of nodes
 * that the selected node must not be in. Either or both may be
 * NULL.
 *
 * Return NULL if no usable nodes could be found. */
static const node_t *
pick_restricted_middle_node(router_crn_flags_t flags,
                            const routerset_t *pick_from,
                            const routerset_t *exclude_set,
                            const smartlist_t *exclude_list,
                            int position_hint)
{
  const node_t *middle_node = NULL;

  smartlist_t *allowlisted_live_middles = smartlist_new();
  smartlist_t *all_live_nodes = smartlist_new();

  tor_assert(pick_from);

  /* Add all running nodes to all_live_nodes */
  router_add_running_nodes_to_smartlist(all_live_nodes, flags);

  /* Filter all_live_nodes to only add live *and* allowlisted middles
   * to the list allowlisted_live_middles. */
  SMARTLIST_FOREACH_BEGIN(all_live_nodes, node_t *, live_node) {
    if (routerset_contains_node(pick_from, live_node)) {
      smartlist_add(allowlisted_live_middles, live_node);
    }
  } SMARTLIST_FOREACH_END(live_node);

  /* Honor ExcludeNodes */
  if (exclude_set) {
    routerset_subtract_nodes(allowlisted_live_middles, exclude_set);
  }

  if (exclude_list) {
    smartlist_subtract(allowlisted_live_middles, exclude_list);
  }

  /**
   * Max number of restricted nodes before we alert the user and try
   * to load balance for them.
   *
   * The most aggressive vanguard design had 16 nodes at layer3.
   * Let's give a small ceiling above that. */
#define MAX_SANE_RESTRICTED_NODES 20
  /* If the user (or associated tor controller) selected only a few nodes,
   * assume they took load balancing into account and don't do it for them.
   *
   * If there are a lot of nodes in here, assume they did not load balance
   * and do it for them, but also warn them that they may be Doing It Wrong.
   */
  if (smartlist_len(allowlisted_live_middles) <=
          MAX_SANE_RESTRICTED_NODES) {
    middle_node = smartlist_choose(allowlisted_live_middles);
  } else {
    static ratelim_t pinned_notice_limit = RATELIM_INIT(24*3600);
    log_fn_ratelim(&pinned_notice_limit, LOG_NOTICE, LD_CIRC,
            "Your _HSLayer%dNodes setting has resulted "
            "in %d total nodes. This is a lot of nodes. "
            "You may want to consider using a Tor controller "
            "to select and update a smaller set of nodes instead.",
            position_hint, smartlist_len(allowlisted_live_middles));

    /* NO_WEIGHTING here just means don't take node flags into account
     * (ie: use consensus measurement only). This is done so that
     * we don't further surprise the user by not using Exits that they
     * specified at all */
    middle_node = node_sl_choose_by_bandwidth(allowlisted_live_middles,
                                              NO_WEIGHTING);
  }

  smartlist_free(allowlisted_live_middles);
  smartlist_free(all_live_nodes);

  return middle_node;
}

/** Return a pointer to a suitable router to be the exit node for the
 * circuit of purpose <b>purpose</b> that we're about to build (or NULL
 * if no router is suitable).
 *
 * For general-purpose circuits, pass it off to
 * choose_good_exit_server_general()
 *
 * For client-side rendezvous circuits, choose a random node, weighted
 * toward the preferences in 'options'.
 */
static const node_t *
choose_good_exit_server(origin_circuit_t *circ,
                        router_crn_flags_t flags, int is_internal)
{
  const or_options_t *options = get_options();
  flags |= CRN_NEED_DESC;

  switch (TO_CIRCUIT(circ)->purpose) {
    case CIRCUIT_PURPOSE_C_HSDIR_GET:
    case CIRCUIT_PURPOSE_S_HSDIR_POST:
    case CIRCUIT_PURPOSE_HS_VANGUARDS:
      /* For these three, we want to pick the exit like a middle hop,
       * since it should be random. */
      tor_assert_nonfatal(is_internal);
      FALLTHROUGH;
    case CIRCUIT_PURPOSE_CONFLUX_UNLINKED:
    case CIRCUIT_PURPOSE_C_GENERAL:
      if (is_internal) /* pick it like a middle hop */
        return router_choose_random_node(NULL, options->ExcludeNodes, flags);
      else
        return choose_good_exit_server_general(flags);
    case CIRCUIT_PURPOSE_C_ESTABLISH_REND:
      {
        /* Pick a new RP */
        const node_t *rendezvous_node = pick_rendezvous_node(flags);
        log_info(LD_REND, "Picked new RP: %s",
                 safe_str_client(node_describe(rendezvous_node)));
        return rendezvous_node;
      }
  }
  log_warn(LD_BUG,"Unhandled purpose %d", TO_CIRCUIT(circ)->purpose);
  tor_fragile_assert();
  return NULL;
}

/** Log a warning if the user specified an exit for the circuit that
 * has been excluded from use by ExcludeNodes or ExcludeExitNodes. */
static void
warn_if_last_router_excluded(origin_circuit_t *circ,
                             const extend_info_t *exit_ei)
{
  const or_options_t *options = get_options();
  routerset_t *rs = options->ExcludeNodes;
  const char *description;
  uint8_t purpose = circ->base_.purpose;

  if (circ->build_state->onehop_tunnel)
    return;

  switch (purpose)
    {
    default:
    case CIRCUIT_PURPOSE_OR:
    case CIRCUIT_PURPOSE_INTRO_POINT:
    case CIRCUIT_PURPOSE_REND_POINT_WAITING:
    case CIRCUIT_PURPOSE_REND_ESTABLISHED:
      log_warn(LD_BUG, "Called on non-origin circuit (purpose %d, %s)",
               (int)purpose,
               circuit_purpose_to_string(purpose));
      return;
    case CIRCUIT_PURPOSE_S_HSDIR_POST:
    case CIRCUIT_PURPOSE_C_HSDIR_GET:
    case CIRCUIT_PURPOSE_C_GENERAL:
    case CIRCUIT_PURPOSE_CONFLUX_UNLINKED:
    case CIRCUIT_PURPOSE_CONFLUX_LINKED:
      if (circ->build_state->is_internal)
        return;
      description = "requested exit node";
      rs = options->ExcludeExitNodesUnion_;
      break;
    case CIRCUIT_PURPOSE_C_INTRODUCING:
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACK_WAIT:
    case CIRCUIT_PURPOSE_C_INTRODUCE_ACKED:
    case CIRCUIT_PURPOSE_S_ESTABLISH_INTRO:
    case CIRCUIT_PURPOSE_S_CONNECT_REND:
    case CIRCUIT_PURPOSE_S_REND_JOINED:
    case CIRCUIT_PURPOSE_TESTING:
      return;
    case CIRCUIT_PURPOSE_C_ESTABLISH_REND:
    case CIRCUIT_PURPOSE_C_REND_READY:
    case CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED:
    case CIRCUIT_PURPOSE_C_REND_JOINED:
      description = "chosen rendezvous point";
      break;
    case CIRCUIT_PURPOSE_CONTROLLER:
      rs = options->ExcludeExitNodesUnion_;
      description = "controller-selected circuit target";
      break;
    }

  if (routerset_contains_extendinfo(rs, exit_ei)) {
    /* We should never get here if StrictNodes is set to 1. */
    if (options->StrictNodes) {
      log_warn(LD_BUG, "Using %s '%s' which is listed in ExcludeNodes%s, "
               "even though StrictNodes is set. Please report. "
               "(Circuit purpose: %s)",
               description, extend_info_describe(exit_ei),
               rs==options->ExcludeNodes?"":" or ExcludeExitNodes",
               circuit_purpose_to_string(purpose));
    } else {
      log_warn(LD_CIRC, "Using %s '%s' which is listed in "
               "ExcludeNodes%s, because no better options were available. To "
               "prevent this (and possibly break your Tor functionality), "
               "set the StrictNodes configuration option. "
               "(Circuit purpose: %s)",
               description, extend_info_describe(exit_ei),
               rs==options->ExcludeNodes?"":" or ExcludeExitNodes",
               circuit_purpose_to_string(purpose));
    }
    circuit_log_path(LOG_WARN, LD_CIRC, circ);
  }

  return;
}

/* Return a set of generic CRN_* flags based on <b>state</b>.
 *
 * Called for every position in the circuit. */
STATIC int
cpath_build_state_to_crn_flags(const cpath_build_state_t *state)
{
  router_crn_flags_t flags = 0;
  /* These flags apply to entry, middle, and exit nodes.
   * If a flag only applies to a specific position, it should be checked in
   * that function. */
  if (state->need_uptime)
    flags |= CRN_NEED_UPTIME;
  if (state->need_capacity)
    flags |= CRN_NEED_CAPACITY;
  return flags;
}

/* Return the CRN_INITIATE_IPV6_EXTEND flag, based on <b>state</b> and
 * <b>cur_len</b>.
 *
 * Only called for middle nodes (for now). Must not be called on single-hop
 * circuits. */
STATIC int
cpath_build_state_to_crn_ipv6_extend_flag(const cpath_build_state_t *state,
                                          int cur_len)
{
  IF_BUG_ONCE(state->desired_path_len < 2)
    return 0;

  /* The last node is the relay doing the self-test. So we want to extend over
   * IPv6 from the second-last node. */
  if (state->is_ipv6_selftest && cur_len == state->desired_path_len - 2)
    return CRN_INITIATE_IPV6_EXTEND;
  else
    return 0;
}

/** Decide a suitable length for circ's cpath, and pick an exit
 * router (or use <b>exit_ei</b> if provided). Store these in the
 * cpath.
 *
 * If <b>is_hs_v3_rp_circuit</b> is set, then this exit should be suitable to
 * be used as an HS v3 rendezvous point.
 *
 * Return 0 if ok, -1 if circuit should be closed. */
STATIC int
onion_pick_cpath_exit(origin_circuit_t *circ, extend_info_t *exit_ei,
                      int is_hs_v3_rp_circuit)
{
  cpath_build_state_t *state = circ->build_state;

  if (state->onehop_tunnel) {
    log_debug(LD_CIRC, "Launching a one-hop circuit for dir tunnel%s.",
              (hs_service_allow_non_anonymous_connection(get_options()) ?
               ", or intro or rendezvous connection" : ""));
    state->desired_path_len = 1;
  } else {
    int r = new_route_len(circ->base_.purpose, exit_ei, nodelist_get_list());
    if (r < 1) /* must be at least 1 */
      return -1;
    state->desired_path_len = r;
  }

  if (exit_ei) { /* the circuit-builder pre-requested one */
    warn_if_last_router_excluded(circ, exit_ei);
    log_info(LD_CIRC,"Using requested exit node '%s'",
             extend_info_describe(exit_ei));
    exit_ei = extend_info_dup(exit_ei);
  } else { /* we have to decide one */
    router_crn_flags_t flags = CRN_NEED_DESC;
    flags |= cpath_build_state_to_crn_flags(state);
    /* Some internal exits are one hop, for example directory connections.
     * (Guards are always direct, middles are never direct.) */
    if (state->onehop_tunnel)
      flags |= CRN_DIRECT_CONN;
    if (is_hs_v3_rp_circuit)
      flags |= CRN_RENDEZVOUS_V3;
    if (state->need_conflux)
      flags |= CRN_CONFLUX;
    const node_t *node =
      choose_good_exit_server(circ, flags, state->is_internal);
    if (!node) {
      log_warn(LD_CIRC,"Failed to choose an exit server");
      return -1;
    }
    exit_ei = extend_info_from_node(node, state->onehop_tunnel,
                /* for_exit_use */
                !state->is_internal && (
                  TO_CIRCUIT(circ)->purpose ==
                  CIRCUIT_PURPOSE_C_GENERAL ||
                  TO_CIRCUIT(circ)->purpose ==
                  CIRCUIT_PURPOSE_CONFLUX_UNLINKED));
    if (BUG(exit_ei == NULL))
      return -1;
  }
  state->chosen_exit = exit_ei;
  return 0;
}

/** Give <b>circ</b> a new exit destination to <b>exit_ei</b>, and add a
 * hop to the cpath reflecting this. Don't send the next extend cell --
 * the caller will do this if it wants to.
 */
int
circuit_append_new_exit(origin_circuit_t *circ, extend_info_t *exit_ei)
{
  cpath_build_state_t *state;
  tor_assert(exit_ei);
  tor_assert(circ);

  state = circ->build_state;
  tor_assert(state);
  extend_info_free(state->chosen_exit);
  state->chosen_exit = extend_info_dup(exit_ei);

  ++circ->build_state->desired_path_len;
  cpath_append_hop(&circ->cpath, exit_ei);
  return 0;
}

/** Take an open <b>circ</b>, and add a new hop at the end, based on
 * <b>info</b>. Set its state back to CIRCUIT_STATE_BUILDING, and then
 * send the next extend cell to begin connecting to that hop.
 */
int
circuit_extend_to_new_exit(origin_circuit_t *circ, extend_info_t *exit_ei)
{
  int err_reason = 0;
  warn_if_last_router_excluded(circ, exit_ei);

  tor_gettimeofday(&circ->base_.timestamp_began);

  circuit_append_new_exit(circ, exit_ei);
  circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_BUILDING);
  if ((err_reason = circuit_send_next_onion_skin(circ))<0) {
    log_warn(LD_CIRC, "Couldn't extend circuit to new point %s.",
             extend_info_describe(exit_ei));
    circuit_mark_for_close(TO_CIRCUIT(circ), -err_reason);
    return -1;
  }

  return 0;
}

/** Return the number of routers in <b>nodes</b> that are currently up and
 * available for building circuits through.
 *
 * If <b>direct</b> is true, only count nodes that are suitable for direct
 * connections. Counts nodes regardless of whether their addresses are
 * preferred.
 */
MOCK_IMPL(STATIC int,
count_acceptable_nodes, (const smartlist_t *nodes, int direct))
{
  int num=0;
  int flags = CRN_NEED_DESC;

  if (direct)
    flags |= CRN_DIRECT_CONN;

  SMARTLIST_FOREACH_BEGIN(nodes, const node_t *, node) {
    //    log_debug(LD_CIRC,
    //              "Contemplating whether router %d (%s) is a new option.",
    //              i, r->nickname);
    if (!router_can_choose_node(node, flags))
      continue;
    ++num;
  } SMARTLIST_FOREACH_END(node);

//    log_debug(LD_CIRC,"I like %d. num_acceptable_routers now %d.",i, num);

  return num;
}

/**
 * Build the exclude list for vanguard circuits.
 *
 * For vanguard circuits we exclude all the already chosen nodes (including the
 * exit) from being middle hops to prevent the creation of A - B - A subpaths.
 * We also allow the 4th hop to be the same as the guard node so as to not leak
 * guard information to RP/IP/HSDirs.
 *
 * For vanguard circuits, we don't apply any subnet or family restrictions.
 * This is to avoid impossible-to-build circuit paths, or just situations where
 * our earlier guards prevent us from using most of our later ones.
 *
 * The alternative is building the circuit in reverse. Reverse calls to
 * onion_extend_cpath() (ie: select outer hops first) would then have the
 * property that you don't gain information about inner hops by observing
 * outer ones. See https://bugs.torproject.org/tpo/core/tor/24487
 * for this.
 *
 * (Note further that we still exclude the exit to prevent A - B - A
 * at the end of the path. */
static smartlist_t *
build_vanguard_middle_exclude_list(uint8_t purpose,
                                   cpath_build_state_t *state,
                                   crypt_path_t *head,
                                   int cur_len)
{
  smartlist_t *excluded;
  const node_t *r;
  crypt_path_t *cpath;
  int i;

  (void) purpose;

  excluded = smartlist_new();

  /* Add the exit to the exclude list (note that the exit/last hop is always
   * chosen first in circuit_establish_circuit()). */
  if ((r = build_state_get_exit_node(state))) {
    smartlist_add(excluded, (node_t*)r);
  }

  /* If we are picking the 4th hop, allow that node to be the guard too.
   * This prevents us from avoiding the Guard for those hops, which
   * gives the adversary information about our guard if they control
   * the RP, IP, or HSDIR. We don't do this check based on purpose
   * because we also want to allow HS_VANGUARDS pre-build circuits
   * to use the guard for that last hop.
   */
  if (cur_len == DEFAULT_ROUTE_LEN+1) {
    /* Skip the first hop for the exclude list below */
    head = head->next;
    cur_len--;
  }

  for (i = 0, cpath = head; cpath && i < cur_len; ++i, cpath=cpath->next) {
    if ((r = node_get_by_id(cpath->extend_info->identity_digest))) {
      smartlist_add(excluded, (node_t*)r);
    }
  }

  return excluded;
}

/**
 * Build a list of nodes to exclude from the choice of this middle
 * hop, based on already chosen nodes.
 */
static smartlist_t *
build_middle_exclude_list(const origin_circuit_t *circ,
                          uint8_t purpose,
                          cpath_build_state_t *state,
                          crypt_path_t *head,
                          int cur_len)
{
  smartlist_t *excluded;
  const node_t *r;
  crypt_path_t *cpath;
  int i;

  /** Vanguard circuits have their own path selection rules */
  if (circuit_should_use_vanguards(purpose)) {
    return build_vanguard_middle_exclude_list(purpose, state, head, cur_len);
  }

  excluded = smartlist_new();

  // Exclude other middles on pending and built conflux circs
  conflux_add_middles_to_exclude_list(circ, excluded);

  /* For non-vanguard circuits, add the exit and its family to the exclude list
   * (note that the exit/last hop is always chosen first in
   * circuit_establish_circuit()). */
  if ((r = build_state_get_exit_node(state))) {
    nodelist_add_node_and_family(excluded, r);
  }

  /* also exclude all other already chosen nodes and their family */
  for (i = 0, cpath = head; cpath && i < cur_len; ++i, cpath=cpath->next) {
    if ((r = node_get_by_id(cpath->extend_info->identity_digest))) {
      nodelist_add_node_and_family(excluded, r);
    }
  }

  return excluded;
}

/** Return true if we MUST use vanguards for picking this middle node. */
static int
middle_node_must_be_vanguard(const or_options_t *options,
                             uint8_t purpose, int cur_len)
{
  /* If this is not a hidden service circuit, don't use vanguards */
  if (!circuit_purpose_is_hidden_service(purpose)) {
    return 0;
  }

  /* Don't even bother if the feature is disabled */
  if (!vanguards_lite_is_enabled()) {
    return 0;
  }

  /* If we are a hidden service circuit, always use either vanguards-lite
   * or HSLayer2Nodes for 2nd hop. */
  if (cur_len == 1) {
    return 1;
  }

  /* If we have sticky L3 nodes, and this is an L3 pick, use vanguards */
  if (options->HSLayer3Nodes && cur_len == 2) {
    return 1;
  }

  return 0;
}

/** Pick a sticky vanguard middle node or return NULL if not found.
 *  See doc of pick_restricted_middle_node() for argument details. */
static const node_t *
pick_vanguard_middle_node(const or_options_t *options,
                          router_crn_flags_t flags, int cur_len,
                          const smartlist_t *excluded)
{
  const routerset_t *vanguard_routerset = NULL;
  const node_t *node = NULL;

  /* Pick the right routerset based on the current hop */
  if (cur_len == 1) {
    vanguard_routerset = options->HSLayer2Nodes ?
      options->HSLayer2Nodes : get_layer2_guards();
  } else if (cur_len == 2) {
    vanguard_routerset = options->HSLayer3Nodes;
  } else {
    /* guaranteed by middle_node_should_be_vanguard() */
    tor_assert_nonfatal_unreached();
    return NULL;
  }

  if (BUG(!vanguard_routerset)) {
    return NULL;
  }

  node = pick_restricted_middle_node(flags, vanguard_routerset,
                                     options->ExcludeNodes, excluded,
                                     cur_len+1);

  if (!node) {
    static ratelim_t pinned_warning_limit = RATELIM_INIT(300);
    log_fn_ratelim(&pinned_warning_limit, LOG_WARN, LD_CIRC,
            "Could not find a node that matches the configured "
            "_HSLayer%dNodes set", cur_len+1);
  }

  return node;
}

/** A helper function used by onion_extend_cpath(). Use <b>purpose</b>
 * and <b>state</b> and the cpath <b>head</b> (currently populated only
 * to length <b>cur_len</b> to decide a suitable middle hop for a
 * circuit. In particular, make sure we don't pick the exit node or its
 * family, and make sure we don't duplicate any previous nodes or their
 * families. */
static const node_t *
choose_good_middle_server(const origin_circuit_t * circ,
                          uint8_t purpose,
                          cpath_build_state_t *state,
                          crypt_path_t *head,
                          int cur_len)
{
  const node_t *choice;
  smartlist_t *excluded;
  const or_options_t *options = get_options();
  router_crn_flags_t flags = CRN_NEED_DESC;
  tor_assert(CIRCUIT_PURPOSE_MIN_ <= purpose &&
             purpose <= CIRCUIT_PURPOSE_MAX_);

  log_debug(LD_CIRC, "Contemplating intermediate hop #%d: random choice.",
            cur_len+1);

  excluded = build_middle_exclude_list(circ, purpose, state, head, cur_len);

  flags |= cpath_build_state_to_crn_flags(state);
  flags |= cpath_build_state_to_crn_ipv6_extend_flag(state, cur_len);

  /** If a hidden service circuit wants a specific middle node, pin it. */
  if (middle_node_must_be_vanguard(options, purpose, cur_len)) {
    log_debug(LD_GENERAL, "Picking a sticky node (cur_len = %d)", cur_len);
    choice = pick_vanguard_middle_node(options, flags, cur_len, excluded);
    smartlist_free(excluded);
    return choice;
  }

  if (options->MiddleNodes) {
    smartlist_t *sl = smartlist_new();
    routerset_get_all_nodes(sl, options->MiddleNodes,
                            options->ExcludeNodes, 1);

    smartlist_subtract(sl, excluded);

    choice = node_sl_choose_by_bandwidth(sl, WEIGHT_FOR_MID);
    smartlist_free(sl);
    if (choice) {
      log_fn(LOG_INFO, LD_CIRC, "Chose fixed middle node: %s",
          hex_str(choice->identity, DIGEST_LEN));
    } else {
      log_fn(LOG_NOTICE, LD_CIRC, "Restricted middle not available");
    }
  } else {
    choice = router_choose_random_node(excluded, options->ExcludeNodes, flags);
  }
  smartlist_free(excluded);
  return choice;
}

/** Pick a good entry server for the circuit to be built according to
 * <b>state</b>.  Don't reuse a chosen exit (if any), don't use this
 * router (if we're an OR), and respect firewall settings; if we're
 * configured to use entry guards, return one.
 *
 * Set *<b>guard_state_out</b> to information about the guard that
 * we're selecting, which we'll use later to remember whether the
 * guard worked or not.
 */
const node_t *
choose_good_entry_server(const origin_circuit_t *circ,
                         uint8_t purpose, cpath_build_state_t *state,
                         circuit_guard_state_t **guard_state_out)
{
  const node_t *choice;
  smartlist_t *excluded;
  const or_options_t *options = get_options();
  /* If possible, choose an entry server with a preferred address,
   * otherwise, choose one with an allowed address */
  router_crn_flags_t flags = (CRN_NEED_GUARD|CRN_NEED_DESC|CRN_PREF_ADDR|
                              CRN_DIRECT_CONN);
  const node_t *node;

  /* Once we used this function to select a node to be a guard.  We had
   * 'state == NULL' be the signal for that.  But we don't do that any more.
   */
  tor_assert_nonfatal(state);

  if (state && options->UseEntryGuards &&
      (purpose != CIRCUIT_PURPOSE_TESTING || options->BridgeRelay)) {
    /* This request is for an entry server to use for a regular circuit,
     * and we use entry guard nodes.  Just return one of the guard nodes.  */
    tor_assert(guard_state_out);
    return guards_choose_guard(circ, state, purpose, guard_state_out);
  }

  excluded = smartlist_new();

  if (state && (node = build_state_get_exit_node(state))) {
    /* Exclude the exit node from the state, if we have one.  Also exclude its
     * family. */
    nodelist_add_node_and_family(excluded, node);
  }

  if (state) {
    flags |= cpath_build_state_to_crn_flags(state);
  }

  choice = router_choose_random_node(excluded, options->ExcludeNodes, flags);
  smartlist_free(excluded);
  return choice;
}

/** Choose a suitable next hop for the circuit <b>circ</b>.
 * Append the hop info to circ->cpath.
 *
 * Return 1 if the path is complete, 0 if we successfully added a hop,
 * and -1 on error.
 */
STATIC int
onion_extend_cpath(origin_circuit_t *circ)
{
  uint8_t purpose = circ->base_.purpose;
  cpath_build_state_t *state = circ->build_state;
  int cur_len = circuit_get_cpath_len(circ);
  extend_info_t *info = NULL;

  if (cur_len >= state->desired_path_len) {
    log_debug(LD_CIRC, "Path is complete: %d steps long",
              state->desired_path_len);
    return 1;
  }

  log_debug(LD_CIRC, "Path is %d long; we want %d", cur_len,
            state->desired_path_len);

  if (cur_len == state->desired_path_len - 1) { /* Picking last node */
    info = extend_info_dup(state->chosen_exit);
  } else if (cur_len == 0) { /* picking first node */
    const node_t *r = choose_good_entry_server(circ, purpose, state,
                                               &circ->guard_state);
    if (r) {
      /* If we're a client, use the preferred address rather than the
         primary address, for potentially connecting to an IPv6 OR
         port. Servers always want the primary (IPv4) address. */
      int client = (server_mode(get_options()) == 0);
      info = extend_info_from_node(r, client, false);
      /* Clients can fail to find an allowed address */
      tor_assert_nonfatal(info || client);
    }
  } else {
    const node_t *r =
      choose_good_middle_server(circ, purpose, state, circ->cpath, cur_len);
    if (r) {
      info = extend_info_from_node(r, 0, false);
    }
  }

  if (!info) {
    /* This can happen on first startup, possibly due to insufficient relays
     * downloaded to pick vanguards-lite layer2 nodes, or other ephemeral
     * reasons. It only happens briefly, is hard to reproduce, and then goes
     * away for ever. :/ */
    if (!router_have_minimum_dir_info()) {
       log_info(LD_CIRC,
                "Failed to find node for hop #%d of our path. Discarding "
                "this circuit.", cur_len+1);
    } else {
       log_notice(LD_CIRC,
                 "Failed to find node for hop #%d of our path. Discarding "
                 "this circuit.", cur_len+1);
    }
    return -1;
  }

  log_debug(LD_CIRC,"Chose router %s for hop #%d (exit is %s)",
            extend_info_describe(info),
            cur_len+1, build_state_get_exit_nickname(state));

  cpath_append_hop(&circ->cpath, info);
  extend_info_free(info);
  return 0;
}

/** Return the node_t for the chosen exit router in <b>state</b>.
 * If there is no chosen exit, or if we don't know the node_t for
 * the chosen exit, return NULL.
 */
MOCK_IMPL(const node_t *,
build_state_get_exit_node,(cpath_build_state_t *state))
{
  if (!state || !state->chosen_exit)
    return NULL;
  return node_get_by_id(state->chosen_exit->identity_digest);
}

/** Return the RSA ID digest for the chosen exit router in <b>state</b>.
 * If there is no chosen exit, return NULL.
 */
const uint8_t *
build_state_get_exit_rsa_id(cpath_build_state_t *state)
{
  if (!state || !state->chosen_exit)
    return NULL;
  return (const uint8_t *) state->chosen_exit->identity_digest;
}

/** Return the nickname for the chosen exit router in <b>state</b>. If
 * there is no chosen exit, or if we don't know the routerinfo_t for the
 * chosen exit, return NULL.
 */
const char *
build_state_get_exit_nickname(cpath_build_state_t *state)
{
  if (!state || !state->chosen_exit)
    return NULL;
  return state->chosen_exit->nickname;
}

/* Is circuit purpose allowed to use the deprecated TAP encryption protocol?
 * The hidden service protocol still uses TAP for some connections, because
 * ntor onion keys aren't included in HS descriptors or INTRODUCE cells. */
static int
circuit_purpose_can_use_tap_impl(uint8_t purpose)
{
  return (purpose == CIRCUIT_PURPOSE_S_CONNECT_REND ||
          purpose == CIRCUIT_PURPOSE_C_INTRODUCING);
}

/* Is circ allowed to use the deprecated TAP encryption protocol?
 * The hidden service protocol still uses TAP for some connections, because
 * ntor onion keys aren't included in HS descriptors or INTRODUCE cells. */
int
circuit_can_use_tap(const origin_circuit_t *circ)
{
  tor_assert(circ);
  tor_assert(circ->cpath);
  tor_assert(circ->cpath->extend_info);
  return (circuit_purpose_can_use_tap_impl(circ->base_.purpose) &&
          extend_info_supports_tap(circ->cpath->extend_info));
}

/* Does circ have an onion key which it's allowed to use? */
int
circuit_has_usable_onion_key(const origin_circuit_t *circ)
{
  tor_assert(circ);
  tor_assert(circ->cpath);
  tor_assert(circ->cpath->extend_info);
  return (extend_info_supports_ntor(circ->cpath->extend_info) ||
          circuit_can_use_tap(circ));
}

/** Find the circuits that are waiting to find out whether their guards are
 * usable, and if any are ready to become usable, mark them open and try
 * attaching streams as appropriate. */
void
circuit_upgrade_circuits_from_guard_wait(void)
{
  smartlist_t *to_upgrade =
    circuit_find_circuits_to_upgrade_from_guard_wait();

  if (to_upgrade == NULL)
    return;

  log_info(LD_GUARD, "Upgrading %d circuits from 'waiting for better guard' "
           "to 'open'.", smartlist_len(to_upgrade));

  SMARTLIST_FOREACH_BEGIN(to_upgrade, origin_circuit_t *, circ) {
    circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_OPEN);
    circuit_has_opened(circ);
  } SMARTLIST_FOREACH_END(circ);

  smartlist_free(to_upgrade);
}

/**
 * Try to generate a circuit-negotiation message for communication with a
 * given relay.  Assumes we are using ntor v3, or some later version that
 * supports parameter negotiatoin.
 *
 * On success, return 0 and pass back a message in the `out` parameters.
 * Otherwise, return -1.
 **/
int
client_circ_negotiation_message(const extend_info_t *ei,
                                uint8_t **msg_out,
                                size_t *msg_len_out)
{
  tor_assert(ei && msg_out && msg_len_out);

  if (!ei->exit_supports_congestion_control) {
    return -1;
  }

  return congestion_control_build_ext_request(msg_out, msg_len_out);
}
