/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file connection_or.c
 * \brief Functions to handle OR connections, TLS handshaking, and
 * cells on the network.
 *
 * An or_connection_t is a subtype of connection_t (as implemented in
 * connection.c) that uses a TLS connection to send and receive cells on the
 * Tor network. (By sending and receiving cells connection_or.c, it cooperates
 * with channeltls.c to implement a the channel interface of channel.c.)
 *
 * Every OR connection has an underlying tortls_t object (as implemented in
 * tortls.c) which it uses as its TLS stream.  It is responsible for
 * sending and receiving cells over that TLS.
 *
 * This module also implements the client side of the v3 (and greater) Tor
 * link handshake.
 **/
#include "core/or/or.h"
#include "feature/client/bridges.h"
#include "lib/buf/buffers.h"
/*
 * Define this so we get channel internal functions, since we're implementing
 * part of a subclass (channel_tls_t).
 */
#define CHANNEL_OBJECT_PRIVATE
#define CONNECTION_OR_PRIVATE
#define ORCONN_EVENT_PRIVATE
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitstats.h"
#include "core/or/command.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_or.h"
#include "feature/relay/relay_handshake.h"
#include "feature/control/control_events.h"
#include "lib/crypt_ops/crypto_util.h"
#include "feature/dirauth/reachability.h"
#include "feature/client/entrynodes.h"
#include "lib/geoip/geoip.h"
#include "core/mainloop/mainloop.h"
#include "trunnel/netinfo.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodelist.h"
#include "core/proto/proto_cell.h"
#include "core/or/reasons.h"
#include "core/or/relay.h"
#include "feature/rend/rendcommon.h"
#include "feature/stats/rephist.h"
#include "feature/relay/router.h"
#include "feature/relay/routerkeys.h"
#include "feature/relay/routermode.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/relay/ext_orport.h"
#include "core/or/scheduler.h"
#include "feature/nodelist/torcert.h"
#include "core/or/channelpadding.h"
#include "feature/dirauth/authmode.h"

#include "core/or/cell_st.h"
#include "core/or/cell_queue_st.h"
#include "core/or/or_connection_st.h"
#include "core/or/or_handshake_certs_st.h"
#include "core/or/or_handshake_state_st.h"
#include "app/config/or_state_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "core/or/var_cell_st.h"
#include "lib/crypt_ops/crypto_format.h"

#include "lib/tls/tortls.h"

#include "core/or/orconn_event.h"

static int connection_tls_finish_handshake(or_connection_t *conn);
static int connection_or_launch_v3_or_handshake(or_connection_t *conn);
static int connection_or_process_cells_from_inbuf(or_connection_t *conn);
static int connection_or_check_valid_tls_handshake(or_connection_t *conn,
                                                   int started_here,
                                                   char *digest_rcvd_out);

static void connection_or_tls_renegotiated_cb(tor_tls_t *tls, void *_conn);

static unsigned int
connection_or_is_bad_for_new_circs(or_connection_t *or_conn);
static void connection_or_mark_bad_for_new_circs(or_connection_t *or_conn);

static void connection_or_check_canonicity(or_connection_t *conn,
                                           int started_here);

/**************************************************************/

/**
 * Cast a `connection_t *` to an `or_connection_t *`.
 *
 * Exit with an assertion failure if the input is not an `or_connnection_t`.
 **/
or_connection_t *
TO_OR_CONN(connection_t *c)
{
  tor_assert(c->magic == OR_CONNECTION_MAGIC);
  return DOWNCAST(or_connection_t, c);
}

/**
 * Cast a `const connection_t *` to a `const or_connection_t *`.
 *
 * Exit with an assertion failure if the input is not an `or_connnection_t`.
 **/
const or_connection_t *
CONST_TO_OR_CONN(const connection_t *c)
{
  return TO_OR_CONN((connection_t *)c);
}

/** Clear clear conn->identity_digest and update other data
 * structures as appropriate.*/
void
connection_or_clear_identity(or_connection_t *conn)
{
  tor_assert(conn);
  memset(conn->identity_digest, 0, DIGEST_LEN);
}

/** Clear all identities in OR conns.*/
void
connection_or_clear_identity_map(void)
{
  smartlist_t *conns = get_connection_array();
  SMARTLIST_FOREACH(conns, connection_t *, conn,
  {
    if (conn->type == CONN_TYPE_OR) {
      connection_or_clear_identity(TO_OR_CONN(conn));
    }
  });
}

/** Change conn->identity_digest to digest, and add conn into
 * the appropriate digest maps.
 *
 * NOTE that this function only allows two kinds of transitions: from
 * unset identity to set identity, and from idempotent re-settings
 * of the same identity.  It's not allowed to clear an identity or to
 * change an identity.  Return 0 on success, and -1 if the transition
 * is not allowed.
 **/
static void
connection_or_set_identity_digest(or_connection_t *conn,
                                  const char *rsa_digest,
                                  const ed25519_public_key_t *ed_id)
{
  channel_t *chan = NULL;
  tor_assert(conn);
  tor_assert(rsa_digest);

  if (conn->chan)
    chan = TLS_CHAN_TO_BASE(conn->chan);

  log_info(LD_HANDSHAKE, "Set identity digest for %s at %p: %s %s.",
           connection_describe(TO_CONN(conn)),
           conn,
           hex_str(rsa_digest, DIGEST_LEN),
           ed25519_fmt(ed_id));
  log_info(LD_HANDSHAKE, "   (Previously: %s %s)",
           hex_str(conn->identity_digest, DIGEST_LEN),
           chan ? ed25519_fmt(&chan->ed25519_identity) : "<null>");

  const int rsa_id_was_set = ! tor_digest_is_zero(conn->identity_digest);
  const int ed_id_was_set =
    chan && !ed25519_public_key_is_zero(&chan->ed25519_identity);
  const int rsa_changed =
    tor_memneq(conn->identity_digest, rsa_digest, DIGEST_LEN);
  const int ed_changed = ed_id_was_set &&
    (!ed_id || !ed25519_pubkey_eq(ed_id, &chan->ed25519_identity));

  tor_assert(!rsa_changed || !rsa_id_was_set);
  tor_assert(!ed_changed || !ed_id_was_set);

  if (!rsa_changed && !ed_changed)
    return;

  /* If the identity was set previously, remove the old mapping. */
  if (rsa_id_was_set) {
    connection_or_clear_identity(conn);
    if (chan)
      channel_clear_identity_digest(chan);
  }

  memcpy(conn->identity_digest, rsa_digest, DIGEST_LEN);

  /* If we're initializing the IDs to zero, don't add a mapping yet. */
  if (tor_digest_is_zero(rsa_digest) &&
      (!ed_id || ed25519_public_key_is_zero(ed_id)))
    return;

  /* Deal with channels */
  if (chan)
    channel_set_identity_digest(chan, rsa_digest, ed_id);
}

/**
 * Return the Ed25519 identity of the peer for this connection (if any).
 *
 * Note that this ID may not be the _actual_ identity for the peer if
 * authentication is not complete.
 **/
const struct ed25519_public_key_t *
connection_or_get_alleged_ed25519_id(const or_connection_t *conn)
{
  if (conn && conn->chan) {
    const channel_t *chan = NULL;
    chan = TLS_CHAN_TO_BASE(conn->chan);
    if (!ed25519_public_key_is_zero(&chan->ed25519_identity)) {
      return &chan->ed25519_identity;
    }
  }

  return NULL;
}

/**************************************************************/

/** Map from a string describing what a non-open OR connection was doing when
 * failed, to an intptr_t describing the count of connections that failed that
 * way.  Note that the count is stored _as_ the pointer.
 */
static strmap_t *broken_connection_counts;

/** If true, do not record information in <b>broken_connection_counts</b>. */
static int disable_broken_connection_counts = 0;

/** Record that an OR connection failed in <b>state</b>. */
static void
note_broken_connection(const char *state)
{
  void *ptr;
  intptr_t val;
  if (disable_broken_connection_counts)
    return;

  if (!broken_connection_counts)
    broken_connection_counts = strmap_new();

  ptr = strmap_get(broken_connection_counts, state);
  val = (intptr_t)ptr;
  val++;
  ptr = (void*)val;
  strmap_set(broken_connection_counts, state, ptr);
}

/** Forget all recorded states for failed connections.  If
 * <b>stop_recording</b> is true, don't record any more. */
void
clear_broken_connection_map(int stop_recording)
{
  if (broken_connection_counts)
    strmap_free(broken_connection_counts, NULL);
  broken_connection_counts = NULL;
  if (stop_recording)
    disable_broken_connection_counts = 1;
}

/** Write a detailed description the state of <b>orconn</b> into the
 * <b>buflen</b>-byte buffer at <b>buf</b>.  This description includes not
 * only the OR-conn level state but also the TLS state.  It's useful for
 * diagnosing broken handshakes. */
static void
connection_or_get_state_description(or_connection_t *orconn,
                                    char *buf, size_t buflen)
{
  connection_t *conn = TO_CONN(orconn);
  const char *conn_state;
  char tls_state[256];

  tor_assert(conn->type == CONN_TYPE_OR || conn->type == CONN_TYPE_EXT_OR);

  conn_state = conn_state_to_string(conn->type, conn->state);
  tor_tls_get_state_description(orconn->tls, tls_state, sizeof(tls_state));

  tor_snprintf(buf, buflen, "%s with SSL state %s", conn_state, tls_state);
}

/** Record the current state of <b>orconn</b> as the state of a broken
 * connection. */
static void
connection_or_note_state_when_broken(or_connection_t *orconn)
{
  char buf[256];
  if (disable_broken_connection_counts)
    return;
  connection_or_get_state_description(orconn, buf, sizeof(buf));
  log_info(LD_HANDSHAKE,"Connection died in state '%s'", buf);
  note_broken_connection(buf);
}

/** Helper type used to sort connection states and find the most frequent. */
typedef struct broken_state_count_t {
  intptr_t count;
  const char *state;
} broken_state_count_t;

/** Helper function used to sort broken_state_count_t by frequency. */
static int
broken_state_count_compare(const void **a_ptr, const void **b_ptr)
{
  const broken_state_count_t *a = *a_ptr, *b = *b_ptr;
  if (b->count < a->count)
    return -1;
  else if (b->count == a->count)
    return 0;
  else
    return 1;
}

/** Upper limit on the number of different states to report for connection
 * failure. */
#define MAX_REASONS_TO_REPORT 10

/** Report a list of the top states for failed OR connections at log level
 * <b>severity</b>, in log domain <b>domain</b>. */
void
connection_or_report_broken_states(int severity, int domain)
{
  int total = 0;
  smartlist_t *items;

  if (!broken_connection_counts || disable_broken_connection_counts)
    return;

  items = smartlist_new();
  STRMAP_FOREACH(broken_connection_counts, state, void *, countptr) {
    broken_state_count_t *c = tor_malloc(sizeof(broken_state_count_t));
    c->count = (intptr_t)countptr;
    total += (int)c->count;
    c->state = state;
    smartlist_add(items, c);
  } STRMAP_FOREACH_END;

  smartlist_sort(items, broken_state_count_compare);

  tor_log(severity, domain, "%d connections have failed%s", total,
      smartlist_len(items) > MAX_REASONS_TO_REPORT ? ". Top reasons:" : ":");

  SMARTLIST_FOREACH_BEGIN(items, const broken_state_count_t *, c) {
    if (c_sl_idx > MAX_REASONS_TO_REPORT)
      break;
    tor_log(severity, domain,
        " %d connections died in state %s", (int)c->count, c->state);
  } SMARTLIST_FOREACH_END(c);

  SMARTLIST_FOREACH(items, broken_state_count_t *, c, tor_free(c));
  smartlist_free(items);
}

/**
 * Helper function to publish an OR connection status event
 *
 * Publishes a messages to subscribers of ORCONN messages, and sends
 * the control event.
 **/
void
connection_or_event_status(or_connection_t *conn, or_conn_status_event_t tp,
                           int reason)
{
  orconn_status_msg_t *msg = tor_malloc(sizeof(*msg));

  msg->gid = conn->base_.global_identifier;
  msg->status = tp;
  msg->reason = reason;
  orconn_status_publish(msg);
  control_event_or_conn_status(conn, tp, reason);
}

/**
 * Helper function to publish a state change message
 *
 * connection_or_change_state() calls this to notify subscribers about
 * a change of an OR connection state.
 **/
static void
connection_or_state_publish(const or_connection_t *conn, uint8_t state)
{
  orconn_state_msg_t *msg = tor_malloc(sizeof(*msg));

  msg->gid = conn->base_.global_identifier;
  if (conn->is_pt) {
    /* Do extra decoding because conn->proxy_type indicates the proxy
     * protocol that tor uses to talk with the transport plugin,
     * instead of PROXY_PLUGGABLE. */
    tor_assert_nonfatal(conn->proxy_type != PROXY_NONE);
    msg->proxy_type = PROXY_PLUGGABLE;
  } else {
    msg->proxy_type = conn->proxy_type;
  }
  msg->state = state;
  if (conn->chan) {
    msg->chan = TLS_CHAN_TO_BASE(conn->chan)->global_identifier;
  } else {
    msg->chan = 0;
  }
  orconn_state_publish(msg);
}

/** Call this to change or_connection_t states, so the owning channel_tls_t can
 * be notified.
 */

MOCK_IMPL(void,
connection_or_change_state,(or_connection_t *conn, uint8_t state))
{
  tor_assert(conn);

  conn->base_.state = state;

  connection_or_state_publish(conn, state);
  if (conn->chan)
    channel_tls_handle_state_change_on_orconn(conn->chan, conn, state);
}

/** Return the number of circuits using an or_connection_t; this used to
 * be an or_connection_t field, but it got moved to channel_t and we
 * shouldn't maintain two copies. */

MOCK_IMPL(int,
connection_or_get_num_circuits, (or_connection_t *conn))
{
  tor_assert(conn);

  if (conn->chan) {
    return channel_num_circuits(TLS_CHAN_TO_BASE(conn->chan));
  } else return 0;
}

/**************************************************************/

/** Pack the cell_t host-order structure <b>src</b> into network-order
 * in the buffer <b>dest</b>. See tor-spec.txt for details about the
 * wire format.
 *
 * Note that this function doesn't touch <b>dst</b>-\>next: the caller
 * should set it or clear it as appropriate.
 */
void
cell_pack(packed_cell_t *dst, const cell_t *src, int wide_circ_ids)
{
  char *dest = dst->body;
  if (wide_circ_ids) {
    set_uint32(dest, htonl(src->circ_id));
    dest += 4;
  } else {
    /* Clear the last two bytes of dest, in case we can accidentally
     * send them to the network somehow. */
    memset(dest+CELL_MAX_NETWORK_SIZE-2, 0, 2);
    set_uint16(dest, htons(src->circ_id));
    dest += 2;
  }
  set_uint8(dest, src->command);
  memcpy(dest+1, src->payload, CELL_PAYLOAD_SIZE);
}

/** Unpack the network-order buffer <b>src</b> into a host-order
 * cell_t structure <b>dest</b>.
 */
static void
cell_unpack(cell_t *dest, const char *src, int wide_circ_ids)
{
  if (wide_circ_ids) {
    dest->circ_id = ntohl(get_uint32(src));
    src += 4;
  } else {
    dest->circ_id = ntohs(get_uint16(src));
    src += 2;
  }
  dest->command = get_uint8(src);
  memcpy(dest->payload, src+1, CELL_PAYLOAD_SIZE);
}

/** Write the header of <b>cell</b> into the first VAR_CELL_MAX_HEADER_SIZE
 * bytes of <b>hdr_out</b>. Returns number of bytes used. */
int
var_cell_pack_header(const var_cell_t *cell, char *hdr_out, int wide_circ_ids)
{
  int r;
  if (wide_circ_ids) {
    set_uint32(hdr_out, htonl(cell->circ_id));
    hdr_out += 4;
    r = VAR_CELL_MAX_HEADER_SIZE;
  } else {
    set_uint16(hdr_out, htons(cell->circ_id));
    hdr_out += 2;
    r = VAR_CELL_MAX_HEADER_SIZE - 2;
  }
  set_uint8(hdr_out, cell->command);
  set_uint16(hdr_out+1, htons(cell->payload_len));
  return r;
}

/** Allocate and return a new var_cell_t with <b>payload_len</b> bytes of
 * payload space. */
var_cell_t *
var_cell_new(uint16_t payload_len)
{
  size_t size = offsetof(var_cell_t, payload) + payload_len;
  var_cell_t *cell = tor_malloc_zero(size);
  cell->payload_len = payload_len;
  cell->command = 0;
  cell->circ_id = 0;
  return cell;
}

/**
 * Copy a var_cell_t
 */

var_cell_t *
var_cell_copy(const var_cell_t *src)
{
  var_cell_t *copy = NULL;
  size_t size = 0;

  if (src != NULL) {
    size = offsetof(var_cell_t, payload) + src->payload_len;
    copy = tor_malloc_zero(size);
    copy->payload_len = src->payload_len;
    copy->command = src->command;
    copy->circ_id = src->circ_id;
    memcpy(copy->payload, src->payload, copy->payload_len);
  }

  return copy;
}

/** Release all space held by <b>cell</b>. */
void
var_cell_free_(var_cell_t *cell)
{
  tor_free(cell);
}

/** We've received an EOF from <b>conn</b>. Mark it for close and return. */
int
connection_or_reached_eof(or_connection_t *conn)
{
  tor_assert(conn);

  log_info(LD_OR,"OR connection reached EOF. Closing.");
  connection_or_close_normally(conn, 1);

  return 0;
}

/** Handle any new bytes that have come in on connection <b>conn</b>.
 * If conn is in 'open' state, hand it to
 * connection_or_process_cells_from_inbuf()
 * (else do nothing).
 */
int
connection_or_process_inbuf(or_connection_t *conn)
{
  int ret = 0;
  tor_assert(conn);

  switch (conn->base_.state) {
    case OR_CONN_STATE_PROXY_HANDSHAKING:
      ret = connection_read_proxy_handshake(TO_CONN(conn));

      /* start TLS after handshake completion, or deal with error */
      if (ret == 1) {
        tor_assert(TO_CONN(conn)->proxy_state == PROXY_CONNECTED);
        if (buf_datalen(conn->base_.inbuf) != 0) {
          log_fn(LOG_PROTOCOL_WARN, LD_NET, "Found leftover (%d bytes) "
                 "when transitioning from PROXY_HANDSHAKING state on %s: "
                 "closing.",
                 (int)buf_datalen(conn->base_.inbuf),
                 connection_describe(TO_CONN(conn)));
          connection_or_close_for_error(conn, 0);
          return -1;
        }
        if (connection_tls_start_handshake(conn, 0) < 0)
          ret = -1;
        /* Touch the channel's active timestamp if there is one */
        if (conn->chan)
          channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));
      }
      if (ret < 0) {
        connection_or_close_for_error(conn, 0);
      }

      return ret;
    case OR_CONN_STATE_TLS_SERVER_RENEGOTIATING:
    case OR_CONN_STATE_OPEN:
    case OR_CONN_STATE_OR_HANDSHAKING_V2:
    case OR_CONN_STATE_OR_HANDSHAKING_V3:
      return connection_or_process_cells_from_inbuf(conn);
    default:
      break; /* don't do anything */
  }

  /* This check makes sure that we don't have any data on the inbuf if we're
   * doing our TLS handshake: if we did, they were probably put there by a
   * SOCKS proxy trying to trick us into accepting unauthenticated data.
   */
  if (buf_datalen(conn->base_.inbuf) != 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_NET, "Accumulated data (%d bytes) "
           "on non-open %s; closing.",
           (int)buf_datalen(conn->base_.inbuf),
           connection_describe(TO_CONN(conn)));
    connection_or_close_for_error(conn, 0);
    ret = -1;
  }

  return ret;
}

/** Called whenever we have flushed some data on an or_conn: add more data
 * from active circuits. */
int
connection_or_flushed_some(or_connection_t *conn)
{
  size_t datalen;

  /* Update the channel's active timestamp if there is one */
  if (conn->chan)
    channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));

  /* If we're under the low water mark, add cells until we're just over the
   * high water mark. */
  datalen = connection_get_outbuf_len(TO_CONN(conn));
  if (datalen < OR_CONN_LOWWATER) {
    /* Let the scheduler know */
    scheduler_channel_wants_writes(TLS_CHAN_TO_BASE(conn->chan));
  }

  return 0;
}

/** This is for channeltls.c to ask how many cells we could accept if
 * they were available. */
ssize_t
connection_or_num_cells_writeable(or_connection_t *conn)
{
  size_t datalen, cell_network_size;
  ssize_t n = 0;

  tor_assert(conn);

  /*
   * If we're under the high water mark, we're potentially
   * writeable; note this is different from the calculation above
   * used to trigger when to start writing after we've stopped.
   */
  datalen = connection_get_outbuf_len(TO_CONN(conn));
  if (datalen < OR_CONN_HIGHWATER) {
    cell_network_size = get_cell_network_size(conn->wide_circ_ids);
    n = CEIL_DIV(OR_CONN_HIGHWATER - datalen, cell_network_size);
  }

  return n;
}

/** Connection <b>conn</b> has finished writing and has no bytes left on
 * its outbuf.
 *
 * Otherwise it's in state "open": stop writing and return.
 *
 * If <b>conn</b> is broken, mark it for close and return -1, else
 * return 0.
 */
int
connection_or_finished_flushing(or_connection_t *conn)
{
  tor_assert(conn);
  assert_connection_ok(TO_CONN(conn),0);

  switch (conn->base_.state) {
    case OR_CONN_STATE_PROXY_HANDSHAKING:
      /* PROXY_HAPROXY gets connected by receiving an ack. */
      if (conn->proxy_type == PROXY_HAPROXY) {
        tor_assert(TO_CONN(conn)->proxy_state == PROXY_HAPROXY_WAIT_FOR_FLUSH);
        TO_CONN(conn)->proxy_state = PROXY_CONNECTED;

        if (connection_tls_start_handshake(conn, 0) < 0) {
          /* TLS handshaking error of some kind. */
          connection_or_close_for_error(conn, 0);
          return -1;
        }
        break;
      }
      break;
    case OR_CONN_STATE_OPEN:
    case OR_CONN_STATE_OR_HANDSHAKING_V2:
    case OR_CONN_STATE_OR_HANDSHAKING_V3:
      break;
    default:
      log_err(LD_BUG,"Called in unexpected state %d.", conn->base_.state);
      tor_fragile_assert();
      return -1;
  }

  /* Update the channel's active timestamp if there is one */
  if (conn->chan)
    channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));

  return 0;
}

/** Connected handler for OR connections: begin the TLS handshake.
 */
int
connection_or_finished_connecting(or_connection_t *or_conn)
{
  const int proxy_type = or_conn->proxy_type;
  connection_t *conn;

  tor_assert(or_conn);
  conn = TO_CONN(or_conn);
  tor_assert(conn->state == OR_CONN_STATE_CONNECTING);

  log_debug(LD_HANDSHAKE,"connect finished for %s",
            connection_describe(conn));

  if (proxy_type != PROXY_NONE) {
    /* start proxy handshake */
    if (connection_proxy_connect(conn, proxy_type) < 0) {
      connection_or_close_for_error(or_conn, 0);
      return -1;
    }

    connection_or_change_state(or_conn, OR_CONN_STATE_PROXY_HANDSHAKING);
    connection_start_reading(conn);

    return 0;
  }

  if (connection_tls_start_handshake(or_conn, 0) < 0) {
    /* TLS handshaking error of some kind. */
    connection_or_close_for_error(or_conn, 0);
    return -1;
  }
  return 0;
}

/** Called when we're about to finally unlink and free an OR connection:
 * perform necessary accounting and cleanup */
void
connection_or_about_to_close(or_connection_t *or_conn)
{
  connection_t *conn = TO_CONN(or_conn);

  /* Tell the controlling channel we're closed */
  if (or_conn->chan) {
    channel_closed(TLS_CHAN_TO_BASE(or_conn->chan));
    /*
     * NULL this out because the channel might hang around a little
     * longer before channel_run_cleanup() gets it.
     */
    or_conn->chan->conn = NULL;
    or_conn->chan = NULL;
  }

  /* Remember why we're closing this connection. */
  if (conn->state != OR_CONN_STATE_OPEN) {
    /* now mark things down as needed */
    if (connection_or_nonopen_was_started_here(or_conn)) {
      const or_options_t *options = get_options();
      connection_or_note_state_when_broken(or_conn);
      /* Tell the new guard API about the channel failure */
      entry_guard_chan_failed(TLS_CHAN_TO_BASE(or_conn->chan));
      if (conn->state >= OR_CONN_STATE_TLS_HANDSHAKING) {
        int reason = tls_error_to_orconn_end_reason(or_conn->tls_error);
        connection_or_event_status(or_conn, OR_CONN_EVENT_FAILED,
                                   reason);
        if (!authdir_mode_tests_reachability(options)) {
          const char *warning = NULL;
          if (reason == END_OR_CONN_REASON_TLS_ERROR && or_conn->tls) {
            warning = tor_tls_get_last_error_msg(or_conn->tls);
          }
          if (warning == NULL) {
            warning = orconn_end_reason_to_control_string(reason);
          }
          control_event_bootstrap_prob_or(warning, reason, or_conn);
        }
      }
    }
  } else if (conn->hold_open_until_flushed) {
    /* We only set hold_open_until_flushed when we're intentionally
     * closing a connection. */
    connection_or_event_status(or_conn, OR_CONN_EVENT_CLOSED,
                tls_error_to_orconn_end_reason(or_conn->tls_error));
  } else if (!tor_digest_is_zero(or_conn->identity_digest)) {
    connection_or_event_status(or_conn, OR_CONN_EVENT_CLOSED,
                tls_error_to_orconn_end_reason(or_conn->tls_error));
  }
}

/** Return 1 if identity digest <b>id_digest</b> is known to be a
 * currently or recently running relay. Otherwise return 0. */
int
connection_or_digest_is_known_relay(const char *id_digest)
{
  if (router_get_consensus_status_by_id(id_digest))
    return 1; /* It's in the consensus: "yes" */
  if (router_get_by_id_digest(id_digest))
    return 1; /* Not in the consensus, but we have a descriptor for
               * it. Probably it was in a recent consensus. "Yes". */
  return 0;
}

/** Set the per-conn read and write limits for <b>conn</b>. If it's a known
 * relay, we will rely on the global read and write buckets, so give it
 * per-conn limits that are big enough they'll never matter. But if it's
 * not a known relay, first check if we set PerConnBwRate/Burst, then
 * check if the consensus sets them, else default to 'big enough'.
 *
 * If <b>reset</b> is true, set the bucket to be full.  Otherwise, just
 * clip the bucket if it happens to be <em>too</em> full.
 */
static void
connection_or_update_token_buckets_helper(or_connection_t *conn, int reset,
                                          const or_options_t *options)
{
  int rate, burst; /* per-connection rate limiting params */
  if (connection_or_digest_is_known_relay(conn->identity_digest)) {
    /* It's in the consensus, or we have a descriptor for it meaning it
     * was probably in a recent consensus. It's a recognized relay:
     * give it full bandwidth. */
    rate = (int)options->BandwidthRate;
    burst = (int)options->BandwidthBurst;
  } else {
    /* Not a recognized relay. Squeeze it down based on the suggested
     * bandwidth parameters in the consensus, but allow local config
     * options to override. */
    rate = options->PerConnBWRate ? (int)options->PerConnBWRate :
        networkstatus_get_param(NULL, "perconnbwrate",
                                (int)options->BandwidthRate, 1, INT32_MAX);
    burst = options->PerConnBWBurst ? (int)options->PerConnBWBurst :
        networkstatus_get_param(NULL, "perconnbwburst",
                                (int)options->BandwidthBurst, 1, INT32_MAX);
  }

  token_bucket_rw_adjust(&conn->bucket, rate, burst);
  if (reset) {
    token_bucket_rw_reset(&conn->bucket, monotime_coarse_get_stamp());
  }
}

/** Either our set of relays or our per-conn rate limits have changed.
 * Go through all the OR connections and update their token buckets to make
 * sure they don't exceed their maximum values. */
void
connection_or_update_token_buckets(smartlist_t *conns,
                                   const or_options_t *options)
{
  SMARTLIST_FOREACH(conns, connection_t *, conn,
  {
    if (connection_speaks_cells(conn))
      connection_or_update_token_buckets_helper(TO_OR_CONN(conn), 0, options);
  });
}

/* Mark <b>or_conn</b> as canonical if <b>is_canonical</b> is set, and
 * non-canonical otherwise. Adjust idle_timeout accordingly.
 */
void
connection_or_set_canonical(or_connection_t *or_conn,
                            int is_canonical)
{
  if (bool_eq(is_canonical, or_conn->is_canonical) &&
      or_conn->idle_timeout != 0) {
    /* Don't recalculate an existing idle_timeout unless the canonical
     * status changed. */
    return;
  }

  or_conn->is_canonical = !! is_canonical; /* force to a 1-bit boolean */
  or_conn->idle_timeout = channelpadding_get_channel_idle_timeout(
          TLS_CHAN_TO_BASE(or_conn->chan), is_canonical);

  log_info(LD_CIRC,
          "Channel %"PRIu64 " chose an idle timeout of %d.",
          or_conn->chan ?
          (TLS_CHAN_TO_BASE(or_conn->chan)->global_identifier):0,
          or_conn->idle_timeout);
}

/** If we don't necessarily know the router we're connecting to, but we
 * have an addr/port/id_digest, then fill in as much as we can. Start
 * by checking to see if this describes a router we know.
 * <b>started_here</b> is 1 if we are the initiator of <b>conn</b> and
 * 0 if it's an incoming connection.  */
void
connection_or_init_conn_from_address(or_connection_t *conn,
                                     const tor_addr_t *addr, uint16_t port,
                                     const char *id_digest,
                                     const ed25519_public_key_t *ed_id,
                                     int started_here)
{
  log_debug(LD_HANDSHAKE, "init conn from address %s: %s, %s (%d)",
            fmt_addr(addr),
            hex_str((const char*)id_digest, DIGEST_LEN),
            ed25519_fmt(ed_id),
            started_here);

  connection_or_set_identity_digest(conn, id_digest, ed_id);
  connection_or_update_token_buckets_helper(conn, 1, get_options());

  conn->base_.port = port;
  tor_addr_copy(&conn->base_.addr, addr);
  if (! conn->base_.address) {
    conn->base_.address = tor_strdup(fmt_addr(addr));
  }

  connection_or_check_canonicity(conn, started_here);
}

/** Check whether the identity of <b>conn</b> matches a known node.  If it
 * does, check whether the address of conn matches the expected address, and
 * update the connection's is_canonical flag, nickname, and address fields as
 * appropriate. */
static void
connection_or_check_canonicity(or_connection_t *conn, int started_here)
{
  (void) started_here;

  const char *id_digest = conn->identity_digest;
  const ed25519_public_key_t *ed_id = NULL;
  if (conn->chan)
    ed_id = & TLS_CHAN_TO_BASE(conn->chan)->ed25519_identity;

  const node_t *r = node_get_by_id(id_digest);
  if (r &&
      node_supports_ed25519_link_authentication(r, 1) &&
      ! node_ed25519_id_matches(r, ed_id)) {
    /* If this node is capable of proving an ed25519 ID,
     * we can't call this a canonical connection unless both IDs match. */
     r = NULL;
  }

  if (r) {
    tor_addr_port_t node_ipv4_ap;
    tor_addr_port_t node_ipv6_ap;
    node_get_prim_orport(r, &node_ipv4_ap);
    node_get_pref_ipv6_orport(r, &node_ipv6_ap);
    if (tor_addr_eq(&conn->base_.addr, &node_ipv4_ap.addr) ||
        tor_addr_eq(&conn->base_.addr, &node_ipv6_ap.addr)) {
      connection_or_set_canonical(conn, 1);
    }
    /* Choose the correct canonical address and port. */
    tor_addr_port_t *node_ap;
    if (tor_addr_family(&conn->base_.addr) == AF_INET) {
      node_ap = &node_ipv4_ap;
    } else {
      node_ap = &node_ipv6_ap;
    }
    /* Remember the canonical addr/port so our log messages will make
       sense. */
    tor_addr_port_copy(&conn->canonical_orport, node_ap);
    tor_free(conn->nickname);
    conn->nickname = tor_strdup(node_get_nickname(r));
  } else {
    tor_free(conn->nickname);
    conn->nickname = tor_malloc(HEX_DIGEST_LEN+2);
    conn->nickname[0] = '$';
    base16_encode(conn->nickname+1, HEX_DIGEST_LEN+1,
                  conn->identity_digest, DIGEST_LEN);
  }

  /*
   * We have to tell channeltls.c to update the channel marks (local, in
   * particular), since we may have changed the address.
   */

  if (conn->chan) {
    channel_tls_update_marks(conn);
  }
}

/** These just pass all the is_bad_for_new_circs manipulation on to
 * channel_t */

static unsigned int
connection_or_is_bad_for_new_circs(or_connection_t *or_conn)
{
  tor_assert(or_conn);

  if (or_conn->chan)
    return channel_is_bad_for_new_circs(TLS_CHAN_TO_BASE(or_conn->chan));
  else return 0;
}

static void
connection_or_mark_bad_for_new_circs(or_connection_t *or_conn)
{
  tor_assert(or_conn);

  if (or_conn->chan)
    channel_mark_bad_for_new_circs(TLS_CHAN_TO_BASE(or_conn->chan));
}

/** How old do we let a connection to an OR get before deciding it's
 * too old for new circuits? */
#define TIME_BEFORE_OR_CONN_IS_TOO_OLD (60*60*24*7)

/** Expire an or_connection if it is too old. Helper for
 * connection_or_group_set_badness_ and fast path for
 * channel_rsa_id_group_set_badness.
 *
 * Returns 1 if the connection was already expired, else 0.
 */
int
connection_or_single_set_badness_(time_t now,
                                  or_connection_t *or_conn,
                                  int force)
{
  /* XXXX this function should also be about channels? */
  if (or_conn->base_.marked_for_close ||
      connection_or_is_bad_for_new_circs(or_conn))
    return 1;

  if (force ||
      or_conn->base_.timestamp_created + TIME_BEFORE_OR_CONN_IS_TOO_OLD
        < now) {
    log_info(LD_OR,
             "Marking %s as too old for new circuits "
             "(fd "TOR_SOCKET_T_FORMAT", %d secs old).",
             connection_describe(TO_CONN(or_conn)),
             or_conn->base_.s,
             (int)(now - or_conn->base_.timestamp_created));
    connection_or_mark_bad_for_new_circs(or_conn);
  }

  return 0;
}

/** Given a list of all the or_connections with a given
 * identity, set elements of that list as is_bad_for_new_circs as
 * appropriate. Helper for connection_or_set_bad_connections().
 *
 * Specifically, we set the is_bad_for_new_circs flag on:
 *    - all connections if <b>force</b> is true.
 *    - all connections that are too old.
 *    - all open non-canonical connections for which a canonical connection
 *      exists to the same router.
 *    - all open canonical connections for which a 'better' canonical
 *      connection exists to the same router.
 *    - all open non-canonical connections for which a 'better' non-canonical
 *      connection exists to the same router at the same address.
 *
 * See channel_is_better() in channel.c for our idea of what makes one OR
 * connection better than another.
 */
void
connection_or_group_set_badness_(smartlist_t *group, int force)
{
  /* XXXX this function should be entirely about channels, not OR
   * XXXX connections. */

  or_connection_t *best = NULL;
  int n_old = 0, n_inprogress = 0, n_canonical = 0, n_other = 0;
  time_t now = time(NULL);

  /* Pass 1: expire everything that's old, and see what the status of
   * everything else is. */
  SMARTLIST_FOREACH_BEGIN(group, or_connection_t *, or_conn) {
    if (connection_or_single_set_badness_(now, or_conn, force))
      continue;

    if (connection_or_is_bad_for_new_circs(or_conn)) {
      ++n_old;
    } else if (or_conn->base_.state != OR_CONN_STATE_OPEN) {
      ++n_inprogress;
    } else if (or_conn->is_canonical) {
      ++n_canonical;
    } else {
      ++n_other;
    }
  } SMARTLIST_FOREACH_END(or_conn);

  /* Pass 2: We know how about how good the best connection is.
   * expire everything that's worse, and find the very best if we can. */
  SMARTLIST_FOREACH_BEGIN(group, or_connection_t *, or_conn) {
    if (or_conn->base_.marked_for_close ||
        connection_or_is_bad_for_new_circs(or_conn))
      continue; /* This one doesn't need to be marked bad. */
    if (or_conn->base_.state != OR_CONN_STATE_OPEN)
      continue; /* Don't mark anything bad until we have seen what happens
                 * when the connection finishes. */
    if (n_canonical && !or_conn->is_canonical) {
      /* We have at least one open canonical connection to this router,
       * and this one is open but not canonical.  Mark it bad. */
      log_info(LD_OR,
               "Marking %s unsuitable for new circuits: "
               "(fd "TOR_SOCKET_T_FORMAT", %d secs old).  It is not "
               "canonical, and we have another connection to that OR that is.",
               connection_describe(TO_CONN(or_conn)),
               or_conn->base_.s,
               (int)(now - or_conn->base_.timestamp_created));
      connection_or_mark_bad_for_new_circs(or_conn);
      continue;
    }

    if (!best ||
        channel_is_better(TLS_CHAN_TO_BASE(or_conn->chan),
                          TLS_CHAN_TO_BASE(best->chan))) {
      best = or_conn;
    }
  } SMARTLIST_FOREACH_END(or_conn);

  if (!best)
    return;

  /* Pass 3: One connection to OR is best.  If it's canonical, mark as bad
   * every other open connection.  If it's non-canonical, mark as bad
   * every other open connection to the same address.
   *
   * XXXX This isn't optimal; if we have connections to an OR at multiple
   *   addresses, we'd like to pick the best _for each address_, and mark as
   *   bad every open connection that isn't best for its address.  But this
   *   can only occur in cases where the other OR is old (so we have no
   *   canonical connection to it), or where all the connections to the OR are
   *   at noncanonical addresses and we have no good direct connection (which
   *   means we aren't at risk of attaching circuits to it anyway).  As
   *   0.1.2.x dies out, the first case will go away, and the second one is
   *   "mostly harmless", so a fix can wait until somebody is bored.
   */
  SMARTLIST_FOREACH_BEGIN(group, or_connection_t *, or_conn) {
    if (or_conn->base_.marked_for_close ||
        connection_or_is_bad_for_new_circs(or_conn) ||
        or_conn->base_.state != OR_CONN_STATE_OPEN)
      continue;
    if (or_conn != best &&
        channel_is_better(TLS_CHAN_TO_BASE(best->chan),
                          TLS_CHAN_TO_BASE(or_conn->chan))) {
      /* This isn't the best conn, _and_ the best conn is better than it */
      if (best->is_canonical) {
        log_info(LD_OR,
                 "Marking %s as unsuitable for new circuits: "
                 "(fd "TOR_SOCKET_T_FORMAT", %d secs old). "
                 "We have a better canonical one "
                 "(fd "TOR_SOCKET_T_FORMAT"; %d secs old).",
                 connection_describe(TO_CONN(or_conn)),
                 or_conn->base_.s,
                 (int)(now - or_conn->base_.timestamp_created),
                 best->base_.s, (int)(now - best->base_.timestamp_created));
        connection_or_mark_bad_for_new_circs(or_conn);
      } else if (tor_addr_eq(&TO_CONN(or_conn)->addr,
                             &TO_CONN(best)->addr)) {
        log_info(LD_OR,
                 "Marking %s unsuitable for new circuits: "
                 "(fd "TOR_SOCKET_T_FORMAT", %d secs old).  We have a better "
                 "one with the "
                 "same address (fd "TOR_SOCKET_T_FORMAT"; %d secs old).",
                 connection_describe(TO_CONN(or_conn)),
                 or_conn->base_.s,
                 (int)(now - or_conn->base_.timestamp_created),
                 best->base_.s, (int)(now - best->base_.timestamp_created));
        connection_or_mark_bad_for_new_circs(or_conn);
      }
    }
  } SMARTLIST_FOREACH_END(or_conn);
}

/* Lifetime of a connection failure. After that, we'll retry. This is in
 * seconds. */
#define OR_CONNECT_FAILURE_LIFETIME 60
/* The interval to use with when to clean up the failure cache. */
#define OR_CONNECT_FAILURE_CLEANUP_INTERVAL 60

/* When is the next time we have to cleanup the failure map. We keep this
 * because we clean it opportunistically. */
static time_t or_connect_failure_map_next_cleanup_ts = 0;

/* OR connection failure entry data structure. It is kept in the connection
 * failure map defined below and indexed by OR identity digest, address and
 * port.
 *
 * We need to identify a connection failure with these three values because we
 * want to avoid to wrongfully block a relay if someone is trying to
 * extend to a known identity digest but with the wrong IP/port. For instance,
 * it can happen if a relay changed its port but the client still has an old
 * descriptor with the old port. We want to stop connecting to that
 * IP/port/identity all together, not only the relay identity. */
typedef struct or_connect_failure_entry_t {
  HT_ENTRY(or_connect_failure_entry_t) node;
  /* Identity digest of the connection where it is connecting to. */
  uint8_t identity_digest[DIGEST_LEN];
  /* This is the connection address from the base connection_t. After the
   * connection is checked for canonicity, the base address should represent
   * what we know instead of where we are connecting to. This is what we need
   * so we can correlate known relays within the consensus. */
  tor_addr_t addr;
  uint16_t port;
  /* Last time we were unable to connect. */
  time_t last_failed_connect_ts;
} or_connect_failure_entry_t;

/* Map where we keep connection failure entries. They are indexed by addr,
 * port and identity digest. */
static HT_HEAD(or_connect_failure_ht, or_connect_failure_entry_t)
       or_connect_failures_map = HT_INITIALIZER();

/* Helper: Hashtable equal function. Return 1 if equal else 0. */
static int
or_connect_failure_ht_eq(const or_connect_failure_entry_t *a,
                         const or_connect_failure_entry_t *b)
{
  return fast_memeq(a->identity_digest, b->identity_digest, DIGEST_LEN) &&
         tor_addr_eq(&a->addr, &b->addr) &&
         a->port == b->port;
}

/* Helper: Return the hash for the hashtable of the given entry. For this
 * table, it is a combination of address, port and identity digest. */
static unsigned int
or_connect_failure_ht_hash(const or_connect_failure_entry_t *entry)
{
  size_t offset = 0, addr_size;
  const void *addr_ptr;
  /* Largest size is IPv6 and IPv4 is smaller so it is fine. */
  uint8_t data[16 + sizeof(uint16_t) + DIGEST_LEN];

  /* Get the right address bytes depending on the family. */
  switch (tor_addr_family(&entry->addr)) {
  case AF_INET:
    addr_size = 4;
    addr_ptr = &entry->addr.addr.in_addr.s_addr;
    break;
  case AF_INET6:
    addr_size = 16;
    addr_ptr = &entry->addr.addr.in6_addr.s6_addr;
    break;
  default:
    tor_assert_nonfatal_unreached();
    return 0;
  }

  memcpy(data, addr_ptr, addr_size);
  offset += addr_size;
  memcpy(data + offset, entry->identity_digest, DIGEST_LEN);
  offset += DIGEST_LEN;
  set_uint16(data + offset, entry->port);
  offset += sizeof(uint16_t);

  return (unsigned int) siphash24g(data, offset);
}

HT_PROTOTYPE(or_connect_failure_ht, or_connect_failure_entry_t, node,
             or_connect_failure_ht_hash, or_connect_failure_ht_eq);

HT_GENERATE2(or_connect_failure_ht, or_connect_failure_entry_t, node,
             or_connect_failure_ht_hash, or_connect_failure_ht_eq,
             0.6, tor_reallocarray_, tor_free_);

/* Initialize a given connect failure entry with the given identity_digest,
 * addr and port. All field are optional except ocf. */
static void
or_connect_failure_init(const char *identity_digest, const tor_addr_t *addr,
                        uint16_t port, or_connect_failure_entry_t *ocf)
{
  tor_assert(ocf);
  if (identity_digest) {
    memcpy(ocf->identity_digest, identity_digest,
           sizeof(ocf->identity_digest));
  }
  if (addr) {
    tor_addr_copy(&ocf->addr, addr);
  }
  ocf->port = port;
}

/* Return a newly allocated connection failure entry. It is initialized with
 * the given or_conn data. This can't fail. */
static or_connect_failure_entry_t *
or_connect_failure_new(const or_connection_t *or_conn)
{
  or_connect_failure_entry_t *ocf = tor_malloc_zero(sizeof(*ocf));
  or_connect_failure_init(or_conn->identity_digest, &TO_CONN(or_conn)->addr,
                          TO_CONN(or_conn)->port, ocf);
  return ocf;
}

/* Return a connection failure entry matching the given or_conn. NULL is
 * returned if not found. */
static or_connect_failure_entry_t *
or_connect_failure_find(const or_connection_t *or_conn)
{
  or_connect_failure_entry_t lookup;
  tor_assert(or_conn);
  or_connect_failure_init(or_conn->identity_digest, &TO_CONN(or_conn)->addr,
                          TO_CONN(or_conn)->port, &lookup);
  return HT_FIND(or_connect_failure_ht, &or_connect_failures_map, &lookup);
}

/* Note down in the connection failure cache that a failure occurred on the
 * given or_conn. */
STATIC void
note_or_connect_failed(const or_connection_t *or_conn)
{
  or_connect_failure_entry_t *ocf = NULL;

  tor_assert(or_conn);

  ocf = or_connect_failure_find(or_conn);
  if (ocf == NULL) {
    ocf = or_connect_failure_new(or_conn);
    HT_INSERT(or_connect_failure_ht, &or_connect_failures_map, ocf);
  }
  ocf->last_failed_connect_ts = approx_time();
}

/* Cleanup the connection failure cache and remove all entries below the
 * given cutoff. */
static void
or_connect_failure_map_cleanup(time_t cutoff)
{
  or_connect_failure_entry_t **ptr, **next, *entry;

  for (ptr = HT_START(or_connect_failure_ht, &or_connect_failures_map);
       ptr != NULL; ptr = next) {
    entry = *ptr;
    if (entry->last_failed_connect_ts <= cutoff) {
      next = HT_NEXT_RMV(or_connect_failure_ht, &or_connect_failures_map, ptr);
      tor_free(entry);
    } else {
      next = HT_NEXT(or_connect_failure_ht, &or_connect_failures_map, ptr);
    }
  }
}

/* Return true iff the given OR connection can connect to its destination that
 * is the triplet identity_digest, address and port.
 *
 * The or_conn MUST have gone through connection_or_check_canonicity() so the
 * base address is properly set to what we know or doesn't know. */
STATIC int
should_connect_to_relay(const or_connection_t *or_conn)
{
  time_t now, cutoff;
  time_t connect_failed_since_ts = 0;
  or_connect_failure_entry_t *ocf;

  tor_assert(or_conn);

  now = approx_time();
  cutoff = now - OR_CONNECT_FAILURE_LIFETIME;

  /* Opportunistically try to cleanup the failure cache. We do that at regular
   * interval so it doesn't grow too big. */
  if (or_connect_failure_map_next_cleanup_ts <= now) {
    or_connect_failure_map_cleanup(cutoff);
    or_connect_failure_map_next_cleanup_ts =
      now + OR_CONNECT_FAILURE_CLEANUP_INTERVAL;
  }

  /* Look if we have failed previously to the same destination as this
   * OR connection. */
  ocf = or_connect_failure_find(or_conn);
  if (ocf) {
    connect_failed_since_ts = ocf->last_failed_connect_ts;
  }
  /* If we do have an unable to connect timestamp and it is below cutoff, we
   * can connect. Or we have never failed before so let it connect. */
  if (connect_failed_since_ts > cutoff) {
    goto no_connect;
  }

  /* Ok we can connect! */
  return 1;
 no_connect:
  return 0;
}

/** <b>conn</b> is in the 'connecting' state, and it failed to complete
 * a TCP connection. Send notifications appropriately.
 *
 * <b>reason</b> specifies the or_conn_end_reason for the failure;
 * <b>msg</b> specifies the strerror-style error message.
 */
void
connection_or_connect_failed(or_connection_t *conn,
                             int reason, const char *msg)
{
  connection_or_event_status(conn, OR_CONN_EVENT_FAILED, reason);
  if (!authdir_mode_tests_reachability(get_options()))
    control_event_bootstrap_prob_or(msg, reason, conn);
  note_or_connect_failed(conn);
}

/** <b>conn</b> got an error in connection_handle_read_impl() or
 * connection_handle_write_impl() and is going to die soon.
 *
 * <b>reason</b> specifies the or_conn_end_reason for the failure;
 * <b>msg</b> specifies the strerror-style error message.
 */
void
connection_or_notify_error(or_connection_t *conn,
                           int reason, const char *msg)
{
  channel_t *chan;

  tor_assert(conn);

  /* If we're connecting, call connect_failed() too */
  if (TO_CONN(conn)->state == OR_CONN_STATE_CONNECTING)
    connection_or_connect_failed(conn, reason, msg);

  /* Tell the controlling channel if we have one */
  if (conn->chan) {
    chan = TLS_CHAN_TO_BASE(conn->chan);
    /* Don't transition if we're already in closing, closed or error */
    if (!CHANNEL_CONDEMNED(chan)) {
      channel_close_for_error(chan);
    }
  }

  /* No need to mark for error because connection.c is about to do that */
}

/** Launch a new OR connection to <b>addr</b>:<b>port</b> and expect to
 * handshake with an OR with identity digest <b>id_digest</b>.  Optionally,
 * pass in a pointer to a channel using this connection.
 *
 * If <b>id_digest</b> is me, do nothing. If we're already connected to it,
 * return that connection. If the connect() is in progress, set the
 * new conn's state to 'connecting' and return it. If connect() succeeds,
 * call connection_tls_start_handshake() on it.
 *
 * This function is called from router_retry_connections(), for
 * ORs connecting to ORs, and circuit_establish_circuit(), for
 * OPs connecting to ORs.
 *
 * Return the launched conn, or NULL if it failed.
 */

MOCK_IMPL(or_connection_t *,
connection_or_connect, (const tor_addr_t *_addr, uint16_t port,
                        const char *id_digest,
                        const ed25519_public_key_t *ed_id,
                        channel_tls_t *chan))
{
  or_connection_t *conn;
  const or_options_t *options = get_options();
  int socket_error = 0;
  tor_addr_t addr;

  int r;
  tor_addr_t proxy_addr;
  uint16_t proxy_port;
  int proxy_type, is_pt = 0;

  tor_assert(_addr);
  tor_assert(id_digest);
  tor_addr_copy(&addr, _addr);

  if (server_mode(options) && router_digest_is_me(id_digest)) {
    log_info(LD_PROTOCOL,"Client asked me to connect to myself. Refusing.");
    return NULL;
  }
  if (server_mode(options) && router_ed25519_id_is_me(ed_id)) {
    log_info(LD_PROTOCOL,"Client asked me to connect to myself by Ed25519 "
             "identity. Refusing.");
    return NULL;
  }

  conn = or_connection_new(CONN_TYPE_OR, tor_addr_family(&addr));

  /*
   * Set up conn so it's got all the data we need to remember for channels
   *
   * This stuff needs to happen before connection_or_init_conn_from_address()
   * so connection_or_set_identity_digest() and such know where to look to
   * keep the channel up to date.
   */
  conn->chan = chan;
  chan->conn = conn;
  connection_or_init_conn_from_address(conn, &addr, port, id_digest, ed_id, 1);

  /* We have a proper OR connection setup, now check if we can connect to it
   * that is we haven't had a failure earlier. This is to avoid to try to
   * constantly connect to relays that we think are not reachable. */
  if (!should_connect_to_relay(conn)) {
    log_info(LD_GENERAL, "Can't connect to %s because we "
                         "failed earlier. Refusing.",
             connection_describe_peer(TO_CONN(conn)));
    connection_free_(TO_CONN(conn));
    return NULL;
  }

  conn->is_outgoing = 1;

  /* If we are using a proxy server, find it and use it. */
  r = get_proxy_addrport(&proxy_addr, &proxy_port, &proxy_type, &is_pt,
                         TO_CONN(conn));
  if (r == 0) {
    conn->proxy_type = proxy_type;
    if (proxy_type != PROXY_NONE) {
      tor_addr_copy(&addr, &proxy_addr);
      port = proxy_port;
      conn->base_.proxy_state = PROXY_INFANT;
      conn->is_pt = is_pt;
    }
    connection_or_change_state(conn, OR_CONN_STATE_CONNECTING);
    connection_or_event_status(conn, OR_CONN_EVENT_LAUNCHED, 0);
  } else {
    /* This duplication of state change calls is necessary in case we
     * run into an error condition below */
    connection_or_change_state(conn, OR_CONN_STATE_CONNECTING);
    connection_or_event_status(conn, OR_CONN_EVENT_LAUNCHED, 0);

    /* get_proxy_addrport() might fail if we have a Bridge line that
       references a transport, but no ClientTransportPlugin lines
       defining its transport proxy. If this is the case, let's try to
       output a useful log message to the user. */
    const char *transport_name =
      find_transport_name_by_bridge_addrport(&TO_CONN(conn)->addr,
                                             TO_CONN(conn)->port);

    if (transport_name) {
      log_warn(LD_GENERAL, "We were supposed to connect to bridge '%s' "
               "using pluggable transport '%s', but we can't find a pluggable "
               "transport proxy supporting '%s'. This can happen if you "
               "haven't provided a ClientTransportPlugin line, or if "
               "your pluggable transport proxy stopped running.",
               connection_describe_peer(TO_CONN(conn)),
               transport_name, transport_name);

      control_event_bootstrap_prob_or(
                                "Can't connect to bridge",
                                END_OR_CONN_REASON_PT_MISSING,
                                conn);

    } else {
      log_warn(LD_GENERAL, "Tried to connect to %s through a proxy, but "
               "the proxy address could not be found.",
               connection_describe_peer(TO_CONN(conn)));
    }

    connection_free_(TO_CONN(conn));
    return NULL;
  }

  switch (connection_connect(TO_CONN(conn), conn->base_.address,
                             &addr, port, &socket_error)) {
    case -1:
      /* We failed to establish a connection probably because of a local
       * error. No need to blame the guard in this case. Notify the networking
       * system of this failure. */
      connection_or_connect_failed(conn,
                                   errno_to_orconn_end_reason(socket_error),
                                   tor_socket_strerror(socket_error));
      connection_free_(TO_CONN(conn));
      return NULL;
    case 0:
      connection_watch_events(TO_CONN(conn), READ_EVENT | WRITE_EVENT);
      /* writable indicates finish, readable indicates broken link,
         error indicates broken link on windows */
      return conn;
    /* case 1: fall through */
  }

  if (connection_or_finished_connecting(conn) < 0) {
    /* already marked for close */
    return NULL;
  }
  return conn;
}

/** Mark orconn for close and transition the associated channel, if any, to
 * the closing state.
 *
 * It's safe to call this and connection_or_close_for_error() any time, and
 * channel layer will treat it as a connection closing for reasons outside
 * its control, like the remote end closing it.  It can also be a local
 * reason that's specific to connection_t/or_connection_t rather than
 * the channel mechanism, such as expiration of old connections in
 * run_connection_housekeeping().  If you want to close a channel_t
 * from somewhere that logically works in terms of generic channels
 * rather than connections, use channel_mark_for_close(); see also
 * the comment on that function in channel.c.
 */

void
connection_or_close_normally(or_connection_t *orconn, int flush)
{
  channel_t *chan = NULL;

  tor_assert(orconn);
  if (flush) connection_mark_and_flush_internal(TO_CONN(orconn));
  else connection_mark_for_close_internal(TO_CONN(orconn));
  if (orconn->chan) {
    chan = TLS_CHAN_TO_BASE(orconn->chan);
    /* Don't transition if we're already in closing, closed or error */
    if (!CHANNEL_CONDEMNED(chan)) {
      channel_close_from_lower_layer(chan);
    }
  }
}

/** Mark orconn for close and transition the associated channel, if any, to
 * the error state.
 */

MOCK_IMPL(void,
connection_or_close_for_error,(or_connection_t *orconn, int flush))
{
  channel_t *chan = NULL;

  tor_assert(orconn);
  if (flush) connection_mark_and_flush_internal(TO_CONN(orconn));
  else connection_mark_for_close_internal(TO_CONN(orconn));
  if (orconn->chan) {
    chan = TLS_CHAN_TO_BASE(orconn->chan);
    /* Don't transition if we're already in closing, closed or error */
    if (!CHANNEL_CONDEMNED(chan)) {
      channel_close_for_error(chan);
    }
  }
}

/** Begin the tls handshake with <b>conn</b>. <b>receiving</b> is 0 if
 * we initiated the connection, else it's 1.
 *
 * Assign a new tls object to conn->tls, begin reading on <b>conn</b>, and
 * pass <b>conn</b> to connection_tls_continue_handshake().
 *
 * Return -1 if <b>conn</b> is broken, else return 0.
 */
MOCK_IMPL(int,
connection_tls_start_handshake,(or_connection_t *conn, int receiving))
{
  channel_listener_t *chan_listener;
  channel_t *chan;

  /* Incoming connections will need a new channel passed to the
   * channel_tls_listener */
  if (receiving) {
    /* It shouldn't already be set */
    tor_assert(!(conn->chan));
    chan_listener = channel_tls_get_listener();
    if (!chan_listener) {
      chan_listener = channel_tls_start_listener();
      command_setup_listener(chan_listener);
    }
    chan = channel_tls_handle_incoming(conn);
    channel_listener_queue_incoming(chan_listener, chan);
  }

  connection_or_change_state(conn, OR_CONN_STATE_TLS_HANDSHAKING);
  tor_assert(!conn->tls);
  conn->tls = tor_tls_new(conn->base_.s, receiving);
  if (!conn->tls) {
    log_warn(LD_BUG,"tor_tls_new failed. Closing.");
    return -1;
  }
  tor_tls_set_logged_address(conn->tls,
                             connection_describe_peer(TO_CONN(conn)));

  connection_start_reading(TO_CONN(conn));
  log_debug(LD_HANDSHAKE,"starting TLS handshake on fd "TOR_SOCKET_T_FORMAT,
            conn->base_.s);

  if (connection_tls_continue_handshake(conn) < 0)
    return -1;

  return 0;
}

/** Block all future attempts to renegotiate on 'conn' */
void
connection_or_block_renegotiation(or_connection_t *conn)
{
  tor_tls_t *tls = conn->tls;
  if (!tls)
    return;
  tor_tls_set_renegotiate_callback(tls, NULL, NULL);
  tor_tls_block_renegotiation(tls);
}

/** Invoked on the server side from inside tor_tls_read() when the server
 * gets a successful TLS renegotiation from the client. */
static void
connection_or_tls_renegotiated_cb(tor_tls_t *tls, void *_conn)
{
  or_connection_t *conn = _conn;
  (void)tls;

  /* Don't invoke this again. */
  connection_or_block_renegotiation(conn);

  if (connection_tls_finish_handshake(conn) < 0) {
    /* XXXX_TLS double-check that it's ok to do this from inside read. */
    /* XXXX_TLS double-check that this verifies certificates. */
    connection_or_close_for_error(conn, 0);
  }
}

/** Move forward with the tls handshake. If it finishes, hand
 * <b>conn</b> to connection_tls_finish_handshake().
 *
 * Return -1 if <b>conn</b> is broken, else return 0.
 */
int
connection_tls_continue_handshake(or_connection_t *conn)
{
  int result;
  check_no_tls_errors();

  tor_assert(conn->base_.state == OR_CONN_STATE_TLS_HANDSHAKING);
  // log_notice(LD_OR, "Continue handshake with %p", conn->tls);
  result = tor_tls_handshake(conn->tls);
  // log_notice(LD_OR, "Result: %d", result);

  switch (result) {
    CASE_TOR_TLS_ERROR_ANY:
      conn->tls_error = result;
      log_info(LD_OR,"tls error [%s]. breaking connection.",
             tor_tls_err_to_string(result));
      return -1;
    case TOR_TLS_DONE:
      if (! tor_tls_used_v1_handshake(conn->tls)) {
        if (!tor_tls_is_server(conn->tls)) {
          tor_assert(conn->base_.state == OR_CONN_STATE_TLS_HANDSHAKING);
          return connection_or_launch_v3_or_handshake(conn);
        } else {
          /* v2/v3 handshake, but we are not a client. */
          log_debug(LD_OR, "Done with initial SSL handshake (server-side). "
                           "Expecting renegotiation or VERSIONS cell");
          tor_tls_set_renegotiate_callback(conn->tls,
                                           connection_or_tls_renegotiated_cb,
                                           conn);
          connection_or_change_state(conn,
              OR_CONN_STATE_TLS_SERVER_RENEGOTIATING);
          connection_stop_writing(TO_CONN(conn));
          connection_start_reading(TO_CONN(conn));
          return 0;
        }
      }
      tor_assert(tor_tls_is_server(conn->tls));
      return connection_tls_finish_handshake(conn);
    case TOR_TLS_WANTWRITE:
      connection_start_writing(TO_CONN(conn));
      log_debug(LD_OR,"wanted write");
      return 0;
    case TOR_TLS_WANTREAD: /* handshaking conns are *always* reading */
      log_debug(LD_OR,"wanted read");
      return 0;
    case TOR_TLS_CLOSE:
      conn->tls_error = result;
      log_info(LD_OR,"tls closed. breaking connection.");
      return -1;
  }
  return 0;
}

/** Return 1 if we initiated this connection, or 0 if it started
 * out as an incoming connection.
 */
int
connection_or_nonopen_was_started_here(or_connection_t *conn)
{
  tor_assert(conn->base_.type == CONN_TYPE_OR ||
             conn->base_.type == CONN_TYPE_EXT_OR);
  if (!conn->tls)
    return 1; /* it's still in proxy states or something */
  if (conn->handshake_state)
    return conn->handshake_state->started_here;
  return !tor_tls_is_server(conn->tls);
}

/** <b>Conn</b> just completed its handshake. Return 0 if all is well, and
 * return -1 if they are lying, broken, or otherwise something is wrong.
 *
 * If we initiated this connection (<b>started_here</b> is true), make sure
 * the other side sent a correctly formed certificate. If I initiated the
 * connection, make sure it's the right relay by checking the certificate.
 *
 * Otherwise (if we _didn't_ initiate this connection), it's okay for
 * the certificate to be weird or absent.
 *
 * If we return 0, and the certificate is as expected, write a hash of the
 * identity key into <b>digest_rcvd_out</b>, which must have DIGEST_LEN
 * space in it.
 * If the certificate is invalid or missing on an incoming connection,
 * we return 0 and set <b>digest_rcvd_out</b> to DIGEST_LEN NUL bytes.
 * (If we return -1, the contents of this buffer are undefined.)
 *
 * As side effects,
 * 1) Set conn->circ_id_type according to tor-spec.txt.
 * 2) If we're an authdirserver and we initiated the connection: drop all
 *    descriptors that claim to be on that IP/port but that aren't
 *    this relay; and note that this relay is reachable.
 * 3) If this is a bridge and we didn't configure its identity
 *    fingerprint, remember the keyid we just learned.
 */
static int
connection_or_check_valid_tls_handshake(or_connection_t *conn,
                                        int started_here,
                                        char *digest_rcvd_out)
{
  crypto_pk_t *identity_rcvd=NULL;
  const or_options_t *options = get_options();
  int severity = server_mode(options) ? LOG_PROTOCOL_WARN : LOG_WARN;
  const char *conn_type = started_here ? "outgoing" : "incoming";
  int has_cert = 0;

  check_no_tls_errors();
  has_cert = tor_tls_peer_has_cert(conn->tls);
  if (started_here && !has_cert) {
    log_info(LD_HANDSHAKE,"Tried connecting to router at %s, but it didn't "
             "send a cert! Closing.",
             connection_describe_peer(TO_CONN(conn)));
    return -1;
  } else if (!has_cert) {
    log_debug(LD_HANDSHAKE,"Got incoming connection with no certificate. "
              "That's ok.");
  }
  check_no_tls_errors();

  if (has_cert) {
    int v = tor_tls_verify(started_here?severity:LOG_INFO,
                           conn->tls, &identity_rcvd);
    if (started_here && v<0) {
      log_fn(severity,LD_HANDSHAKE,"Tried connecting to router at %s: It"
             " has a cert but it's invalid. Closing.",
             connection_describe_peer(TO_CONN(conn)));
        return -1;
    } else if (v<0) {
      log_info(LD_HANDSHAKE,"Incoming connection gave us an invalid cert "
               "chain; ignoring.");
    } else {
      log_debug(LD_HANDSHAKE,
                "The certificate seems to be valid on %s connection "
                "with %s", conn_type,
                connection_describe_peer(TO_CONN(conn)));
    }
    check_no_tls_errors();
  }

  if (identity_rcvd) {
    if (crypto_pk_get_digest(identity_rcvd, digest_rcvd_out) < 0) {
      crypto_pk_free(identity_rcvd);
      return -1;
    }
  } else {
    memset(digest_rcvd_out, 0, DIGEST_LEN);
  }

  tor_assert(conn->chan);
  channel_set_circid_type(TLS_CHAN_TO_BASE(conn->chan), identity_rcvd, 1);

  crypto_pk_free(identity_rcvd);

  if (started_here) {
    /* A TLS handshake can't teach us an Ed25519 ID, so we set it to NULL
     * here. */
    log_debug(LD_HANDSHAKE, "Calling client_learned_peer_id from "
              "check_valid_tls_handshake");
    return connection_or_client_learned_peer_id(conn,
                                        (const uint8_t*)digest_rcvd_out,
                                        NULL);
  }

  return 0;
}

/** Called when we (as a connection initiator) have definitively,
 * authenticatedly, learned that ID of the Tor instance on the other
 * side of <b>conn</b> is <b>rsa_peer_id</b> and optionally <b>ed_peer_id</b>.
 * For v1 and v2 handshakes,
 * this is right after we get a certificate chain in a TLS handshake
 * or renegotiation.  For v3+ handshakes, this is right after we get a
 * certificate chain in a CERTS cell.
 *
 * If we did not know the ID before, record the one we got.
 *
 * If we wanted an ID, but we didn't get the one we expected, log a message
 * and return -1.
 * On relays:
 *  - log a protocol warning whenever the fingerprints don't match;
 * On clients:
 *  - if a relay's fingerprint doesn't match, log a warning;
 *  - if we don't have updated relay fingerprints from a recent consensus, and
 *    a fallback directory mirror's hard-coded fingerprint has changed, log an
 *    info explaining that we will try another fallback.
 *
 * If we're testing reachability, remember what we learned.
 *
 * Return 0 on success, -1 on failure.
 */
int
connection_or_client_learned_peer_id(or_connection_t *conn,
                                     const uint8_t *rsa_peer_id,
                                     const ed25519_public_key_t *ed_peer_id)
{
  const or_options_t *options = get_options();
  channel_tls_t *chan_tls = conn->chan;
  channel_t *chan = channel_tls_to_base(chan_tls);
  int changed_identity = 0;
  tor_assert(chan);

  const int expected_rsa_key =
    ! tor_digest_is_zero(conn->identity_digest);
  const int expected_ed_key =
    ! ed25519_public_key_is_zero(&chan->ed25519_identity);

  log_info(LD_HANDSHAKE, "learned peer id for %s at %p: %s, %s",
           connection_describe(TO_CONN(conn)),
           conn,
           hex_str((const char*)rsa_peer_id, DIGEST_LEN),
           ed25519_fmt(ed_peer_id));

  if (! expected_rsa_key && ! expected_ed_key) {
    log_info(LD_HANDSHAKE, "(we had no ID in mind when we made this "
             "connection.");
    connection_or_set_identity_digest(conn,
                                      (const char*)rsa_peer_id, ed_peer_id);
    tor_free(conn->nickname);
    conn->nickname = tor_malloc(HEX_DIGEST_LEN+2);
    conn->nickname[0] = '$';
    base16_encode(conn->nickname+1, HEX_DIGEST_LEN+1,
                  conn->identity_digest, DIGEST_LEN);
    log_info(LD_HANDSHAKE, "Connected to router at %s without knowing "
             "its key. Hoping for the best.",
             connection_describe_peer(TO_CONN(conn)));
    /* if it's a bridge and we didn't know its identity fingerprint, now
     * we do -- remember it for future attempts. */
    learned_router_identity(&conn->base_.addr, conn->base_.port,
                            (const char*)rsa_peer_id, ed_peer_id);
    changed_identity = 1;
  }

  const int rsa_mismatch = expected_rsa_key &&
    tor_memneq(rsa_peer_id, conn->identity_digest, DIGEST_LEN);
  /* It only counts as an ed25519 mismatch if we wanted an ed25519 identity
   * and didn't get it. It's okay if we get one that we didn't ask for. */
  const int ed25519_mismatch =
    expected_ed_key &&
    (ed_peer_id == NULL ||
     ! ed25519_pubkey_eq(&chan->ed25519_identity, ed_peer_id));

  if (rsa_mismatch || ed25519_mismatch) {
    /* I was aiming for a particular digest. I didn't get it! */
    char seen_rsa[HEX_DIGEST_LEN+1];
    char expected_rsa[HEX_DIGEST_LEN+1];
    char seen_ed[ED25519_BASE64_LEN+1];
    char expected_ed[ED25519_BASE64_LEN+1];
    base16_encode(seen_rsa, sizeof(seen_rsa),
                  (const char*)rsa_peer_id, DIGEST_LEN);
    base16_encode(expected_rsa, sizeof(expected_rsa), conn->identity_digest,
                  DIGEST_LEN);
    if (ed_peer_id) {
      ed25519_public_to_base64(seen_ed, ed_peer_id);
    } else {
      strlcpy(seen_ed, "no ed25519 key", sizeof(seen_ed));
    }
    if (! ed25519_public_key_is_zero(&chan->ed25519_identity)) {
      ed25519_public_to_base64(expected_ed, &chan->ed25519_identity);
    } else {
      strlcpy(expected_ed, "no ed25519 key", sizeof(expected_ed));
    }
    const int using_hardcoded_fingerprints =
      !networkstatus_get_reasonably_live_consensus(time(NULL),
                                                   usable_consensus_flavor());
    const int is_fallback_fingerprint = router_digest_is_fallback_dir(
                                                   conn->identity_digest);
    const int is_authority_fingerprint = router_digest_is_trusted_dir(
                                                   conn->identity_digest);
    const int non_anonymous_mode = rend_non_anonymous_mode_enabled(options);
    int severity;
    const char *extra_log = "";

    /* Relays and Single Onion Services make direct connections using
     * untrusted authentication keys. */
    if (server_mode(options) || non_anonymous_mode) {
      severity = LOG_PROTOCOL_WARN;
    } else {
      if (using_hardcoded_fingerprints) {
        /* We need to do the checks in this order, because the list of
         * fallbacks includes the list of authorities */
        if (is_authority_fingerprint) {
          severity = LOG_WARN;
        } else if (is_fallback_fingerprint) {
          /* we expect a small number of fallbacks to change from their
           * hard-coded fingerprints over the life of a release */
          severity = LOG_INFO;
          extra_log = " Tor will try a different fallback.";
        } else {
          /* it's a bridge, it's either a misconfiguration, or unexpected */
          severity = LOG_WARN;
        }
      } else {
        /* a relay has changed its fingerprint from the one in the consensus */
        severity = LOG_WARN;
      }
    }

    log_fn(severity, LD_HANDSHAKE,
           "Tried connecting to router at %s, but RSA + ed25519 identity "
           "keys were not as expected: wanted %s + %s but got %s + %s.%s",
           connection_describe_peer(TO_CONN(conn)),
           expected_rsa, expected_ed, seen_rsa, seen_ed, extra_log);

    /* Tell the new guard API about the channel failure */
    entry_guard_chan_failed(TLS_CHAN_TO_BASE(conn->chan));
    connection_or_event_status(conn, OR_CONN_EVENT_FAILED,
                               END_OR_CONN_REASON_OR_IDENTITY);
    if (!authdir_mode_tests_reachability(options))
      control_event_bootstrap_prob_or(
                                "Unexpected identity in router certificate",
                                END_OR_CONN_REASON_OR_IDENTITY,
                                conn);
    return -1;
  }

  if (!expected_ed_key && ed_peer_id) {
    log_info(LD_HANDSHAKE, "(We had no Ed25519 ID in mind when we made this "
             "connection.)");
    connection_or_set_identity_digest(conn,
                                      (const char*)rsa_peer_id, ed_peer_id);
    changed_identity = 1;
  }

  if (changed_identity) {
    /* If we learned an identity for this connection, then we might have
     * just discovered it to be canonical. */
    connection_or_check_canonicity(conn, conn->handshake_state->started_here);
    if (conn->tls)
      tor_tls_set_logged_address(conn->tls,
                                 connection_describe_peer(TO_CONN(conn)));
  }

  if (authdir_mode_tests_reachability(options)) {
    // We don't want to use canonical_orport here -- we want the address
    // that we really used.
    dirserv_orconn_tls_done(&conn->base_.addr, conn->base_.port,
                            (const char*)rsa_peer_id, ed_peer_id);
  }

  return 0;
}

/** Return when we last used this channel for client activity (origin
 * circuits). This is called from connection.c, since client_used is now one
 * of the timestamps in channel_t */

time_t
connection_or_client_used(or_connection_t *conn)
{
  tor_assert(conn);

  if (conn->chan) {
    return channel_when_last_client(TLS_CHAN_TO_BASE(conn->chan));
  } else return 0;
}

/** The v1/v2 TLS handshake is finished.
 *
 * Make sure we are happy with the peer we just handshaked with.
 *
 * If they initiated the connection, make sure they're not already connected,
 * then initialize conn from the information in router.
 *
 * If all is successful, call circuit_n_conn_done() to handle events
 * that have been pending on the <tls handshake completion. Also set the
 * directory to be dirty (only matters if I'm an authdirserver).
 *
 * If this is a v2 TLS handshake, send a versions cell.
 */
static int
connection_tls_finish_handshake(or_connection_t *conn)
{
  char digest_rcvd[DIGEST_LEN];
  int started_here = connection_or_nonopen_was_started_here(conn);

  tor_assert(!started_here);

  log_debug(LD_HANDSHAKE,"%s tls handshake on %s done, using "
            "ciphersuite %s. verifying.",
            started_here?"outgoing":"incoming",
            connection_describe_peer(TO_CONN(conn)),
            tor_tls_get_ciphersuite_name(conn->tls));

  if (connection_or_check_valid_tls_handshake(conn, started_here,
                                              digest_rcvd) < 0)
    return -1;

  circuit_build_times_network_is_live(get_circuit_build_times_mutable());

  if (tor_tls_used_v1_handshake(conn->tls)) {
    conn->link_proto = 1;
    connection_or_init_conn_from_address(conn, &conn->base_.addr,
                                         conn->base_.port, digest_rcvd,
                                         NULL, 0);
    tor_tls_block_renegotiation(conn->tls);
    rep_hist_note_negotiated_link_proto(1, started_here);
    return connection_or_set_state_open(conn);
  } else {
    connection_or_change_state(conn, OR_CONN_STATE_OR_HANDSHAKING_V2);
    if (connection_init_or_handshake_state(conn, started_here) < 0)
      return -1;
    connection_or_init_conn_from_address(conn, &conn->base_.addr,
                                         conn->base_.port, digest_rcvd,
                                         NULL, 0);
    return connection_or_send_versions(conn, 0);
  }
}

/**
 * Called as client when initial TLS handshake is done, and we notice
 * that we got a v3-handshake signalling certificate from the server.
 * Set up structures, do bookkeeping, and send the versions cell.
 * Return 0 on success and -1 on failure.
 */
static int
connection_or_launch_v3_or_handshake(or_connection_t *conn)
{
  tor_assert(connection_or_nonopen_was_started_here(conn));

  circuit_build_times_network_is_live(get_circuit_build_times_mutable());

  connection_or_change_state(conn, OR_CONN_STATE_OR_HANDSHAKING_V3);
  if (connection_init_or_handshake_state(conn, 1) < 0)
    return -1;

  return connection_or_send_versions(conn, 1);
}

/** Allocate a new connection handshake state for the connection
 * <b>conn</b>.  Return 0 on success, -1 on failure. */
int
connection_init_or_handshake_state(or_connection_t *conn, int started_here)
{
  or_handshake_state_t *s;
  if (conn->handshake_state) {
    log_warn(LD_BUG, "Duplicate call to connection_init_or_handshake_state!");
    return 0;
  }
  s = conn->handshake_state = tor_malloc_zero(sizeof(or_handshake_state_t));
  s->started_here = started_here ? 1 : 0;
  s->digest_sent_data = 1;
  s->digest_received_data = 1;
  if (! started_here && get_current_link_cert_cert()) {
    s->own_link_cert = tor_cert_dup(get_current_link_cert_cert());
  }
  s->certs = or_handshake_certs_new();
  s->certs->started_here = s->started_here;
  return 0;
}

/** Free all storage held by <b>state</b>. */
void
or_handshake_state_free_(or_handshake_state_t *state)
{
  if (!state)
    return;
  crypto_digest_free(state->digest_sent);
  crypto_digest_free(state->digest_received);
  or_handshake_certs_free(state->certs);
  tor_cert_free(state->own_link_cert);
  memwipe(state, 0xBE, sizeof(or_handshake_state_t));
  tor_free(state);
}

/**
 * Remember that <b>cell</b> has been transmitted (if <b>incoming</b> is
 * false) or received (if <b>incoming</b> is true) during a V3 handshake using
 * <b>state</b>.
 *
 * (We don't record the cell, but we keep a digest of everything sent or
 * received during the v3 handshake, and the client signs it in an
 * authenticate cell.)
 */
void
or_handshake_state_record_cell(or_connection_t *conn,
                               or_handshake_state_t *state,
                               const cell_t *cell,
                               int incoming)
{
  size_t cell_network_size = get_cell_network_size(conn->wide_circ_ids);
  crypto_digest_t *d, **dptr;
  packed_cell_t packed;
  if (incoming) {
    if (!state->digest_received_data)
      return;
  } else {
    if (!state->digest_sent_data)
      return;
  }
  if (!incoming) {
    log_warn(LD_BUG, "We shouldn't be sending any non-variable-length cells "
             "while making a handshake digest.  But we think we are sending "
             "one with type %d.", (int)cell->command);
  }
  dptr = incoming ? &state->digest_received : &state->digest_sent;
  if (! *dptr)
    *dptr = crypto_digest256_new(DIGEST_SHA256);

  d = *dptr;
  /* Re-packing like this is a little inefficient, but we don't have to do
     this very often at all. */
  cell_pack(&packed, cell, conn->wide_circ_ids);
  crypto_digest_add_bytes(d, packed.body, cell_network_size);
  memwipe(&packed, 0, sizeof(packed));
}

/** Remember that a variable-length <b>cell</b> has been transmitted (if
 * <b>incoming</b> is false) or received (if <b>incoming</b> is true) during a
 * V3 handshake using <b>state</b>.
 *
 * (We don't record the cell, but we keep a digest of everything sent or
 * received during the v3 handshake, and the client signs it in an
 * authenticate cell.)
 */
void
or_handshake_state_record_var_cell(or_connection_t *conn,
                                   or_handshake_state_t *state,
                                   const var_cell_t *cell,
                                   int incoming)
{
  crypto_digest_t *d, **dptr;
  int n;
  char buf[VAR_CELL_MAX_HEADER_SIZE];
  if (incoming) {
    if (!state->digest_received_data)
      return;
  } else {
    if (!state->digest_sent_data)
      return;
  }
  dptr = incoming ? &state->digest_received : &state->digest_sent;
  if (! *dptr)
    *dptr = crypto_digest256_new(DIGEST_SHA256);

  d = *dptr;

  n = var_cell_pack_header(cell, buf, conn->wide_circ_ids);
  crypto_digest_add_bytes(d, buf, n);
  crypto_digest_add_bytes(d, (const char *)cell->payload, cell->payload_len);

  memwipe(buf, 0, sizeof(buf));
}

/** Set <b>conn</b>'s state to OR_CONN_STATE_OPEN, and tell other subsystems
 * as appropriate.  Called when we are done with all TLS and OR handshaking.
 */
int
connection_or_set_state_open(or_connection_t *conn)
{
  connection_or_change_state(conn, OR_CONN_STATE_OPEN);
  connection_or_event_status(conn, OR_CONN_EVENT_CONNECTED, 0);

  /* Link protocol 3 appeared in Tor 0.2.3.6-alpha, so any connection
   * that uses an earlier link protocol should not be treated as a relay. */
  if (conn->link_proto < 3) {
    channel_mark_client(TLS_CHAN_TO_BASE(conn->chan));
  }

  or_handshake_state_free(conn->handshake_state);
  conn->handshake_state = NULL;
  connection_start_reading(TO_CONN(conn));

  return 0;
}

/** Pack <b>cell</b> into wire-format, and write it onto <b>conn</b>'s outbuf.
 * For cells that use or affect a circuit, this should only be called by
 * connection_or_flush_from_first_active_circuit().
 */
void
connection_or_write_cell_to_buf(const cell_t *cell, or_connection_t *conn)
{
  packed_cell_t networkcell;
  size_t cell_network_size = get_cell_network_size(conn->wide_circ_ids);

  tor_assert(cell);
  tor_assert(conn);

  cell_pack(&networkcell, cell, conn->wide_circ_ids);

  /* We need to count padding cells from this non-packed code path
   * since they are sent via chan->write_cell() (which is not packed) */
  rep_hist_padding_count_write(PADDING_TYPE_TOTAL);
  if (cell->command == CELL_PADDING)
    rep_hist_padding_count_write(PADDING_TYPE_CELL);

  connection_buf_add(networkcell.body, cell_network_size, TO_CONN(conn));

  /* Touch the channel's active timestamp if there is one */
  if (conn->chan) {
    channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));

    if (TLS_CHAN_TO_BASE(conn->chan)->padding_enabled) {
      rep_hist_padding_count_write(PADDING_TYPE_ENABLED_TOTAL);
      if (cell->command == CELL_PADDING)
        rep_hist_padding_count_write(PADDING_TYPE_ENABLED_CELL);
    }
  }

  if (conn->base_.state == OR_CONN_STATE_OR_HANDSHAKING_V3)
    or_handshake_state_record_cell(conn, conn->handshake_state, cell, 0);
}

/** Pack a variable-length <b>cell</b> into wire-format, and write it onto
 * <b>conn</b>'s outbuf.  Right now, this <em>DOES NOT</em> support cells that
 * affect a circuit.
 */
MOCK_IMPL(void,
connection_or_write_var_cell_to_buf,(const var_cell_t *cell,
                                     or_connection_t *conn))
{
  int n;
  char hdr[VAR_CELL_MAX_HEADER_SIZE];
  tor_assert(cell);
  tor_assert(conn);
  n = var_cell_pack_header(cell, hdr, conn->wide_circ_ids);
  connection_buf_add(hdr, n, TO_CONN(conn));
  connection_buf_add((char*)cell->payload,
                          cell->payload_len, TO_CONN(conn));
  if (conn->base_.state == OR_CONN_STATE_OR_HANDSHAKING_V3)
    or_handshake_state_record_var_cell(conn, conn->handshake_state, cell, 0);

  rep_hist_padding_count_write(PADDING_TYPE_TOTAL);
  /* Touch the channel's active timestamp if there is one */
  if (conn->chan)
    channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));
}

/** See whether there's a variable-length cell waiting on <b>or_conn</b>'s
 * inbuf.  Return values as for fetch_var_cell_from_buf(). */
static int
connection_fetch_var_cell_from_buf(or_connection_t *or_conn, var_cell_t **out)
{
  connection_t *conn = TO_CONN(or_conn);
  return fetch_var_cell_from_buf(conn->inbuf, out, or_conn->link_proto);
}

/** Process cells from <b>conn</b>'s inbuf.
 *
 * Loop: while inbuf contains a cell, pull it off the inbuf, unpack it,
 * and hand it to command_process_cell().
 *
 * Always return 0.
 */
static int
connection_or_process_cells_from_inbuf(or_connection_t *conn)
{
  var_cell_t *var_cell;

  /*
   * Note on memory management for incoming cells: below the channel layer,
   * we shouldn't need to consider its internal queueing/copying logic.  It
   * is safe to pass cells to it on the stack or on the heap, but in the
   * latter case we must be sure we free them later.
   *
   * The incoming cell queue code in channel.c will (in the common case)
   * decide it can pass them to the upper layer immediately, in which case
   * those functions may run directly on the cell pointers we pass here, or
   * it may decide to queue them, in which case it will allocate its own
   * buffer and copy the cell.
   */

  while (1) {
    log_debug(LD_OR,
              TOR_SOCKET_T_FORMAT": starting, inbuf_datalen %d "
              "(%d pending in tls object).",
              conn->base_.s,(int)connection_get_inbuf_len(TO_CONN(conn)),
              tor_tls_get_pending_bytes(conn->tls));
    if (connection_fetch_var_cell_from_buf(conn, &var_cell)) {
      if (!var_cell)
        return 0; /* not yet. */

      /* Touch the channel's active timestamp if there is one */
      if (conn->chan)
        channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));

      circuit_build_times_network_is_live(get_circuit_build_times_mutable());
      channel_tls_handle_var_cell(var_cell, conn);
      var_cell_free(var_cell);
    } else {
      const int wide_circ_ids = conn->wide_circ_ids;
      size_t cell_network_size = get_cell_network_size(conn->wide_circ_ids);
      char buf[CELL_MAX_NETWORK_SIZE];
      cell_t cell;
      if (connection_get_inbuf_len(TO_CONN(conn))
          < cell_network_size) /* whole response available? */
        return 0; /* not yet */

      /* Touch the channel's active timestamp if there is one */
      if (conn->chan)
        channel_timestamp_active(TLS_CHAN_TO_BASE(conn->chan));

      circuit_build_times_network_is_live(get_circuit_build_times_mutable());
      connection_buf_get_bytes(buf, cell_network_size, TO_CONN(conn));

      /* retrieve cell info from buf (create the host-order struct from the
       * network-order string) */
      cell_unpack(&cell, buf, wide_circ_ids);

      channel_tls_handle_cell(&cell, conn);
    }
  }
}

/** Array of recognized link protocol versions. */
static const uint16_t or_protocol_versions[] = { 1, 2, 3, 4, 5 };
/** Number of versions in <b>or_protocol_versions</b>. */
static const int n_or_protocol_versions =
  (int)( sizeof(or_protocol_versions)/sizeof(uint16_t) );

/** Return true iff <b>v</b> is a link protocol version that this Tor
 * implementation believes it can support. */
int
is_or_protocol_version_known(uint16_t v)
{
  int i;
  for (i = 0; i < n_or_protocol_versions; ++i) {
    if (or_protocol_versions[i] == v)
      return 1;
  }
  return 0;
}

/** Send a VERSIONS cell on <b>conn</b>, telling the other host about the
 * link protocol versions that this Tor can support.
 *
 * If <b>v3_plus</b>, this is part of a V3 protocol handshake, so only
 * allow protocol version v3 or later.  If not <b>v3_plus</b>, this is
 * not part of a v3 protocol handshake, so don't allow protocol v3 or
 * later.
 **/
int
connection_or_send_versions(or_connection_t *conn, int v3_plus)
{
  var_cell_t *cell;
  int i;
  int n_versions = 0;
  const int min_version = v3_plus ? 3 : 0;
  const int max_version = v3_plus ? UINT16_MAX : 2;
  tor_assert(conn->handshake_state &&
             !conn->handshake_state->sent_versions_at);
  cell = var_cell_new(n_or_protocol_versions * 2);
  cell->command = CELL_VERSIONS;
  for (i = 0; i < n_or_protocol_versions; ++i) {
    uint16_t v = or_protocol_versions[i];
    if (v < min_version || v > max_version)
      continue;
    set_uint16(cell->payload+(2*n_versions), htons(v));
    ++n_versions;
  }
  cell->payload_len = n_versions * 2;

  connection_or_write_var_cell_to_buf(cell, conn);
  conn->handshake_state->sent_versions_at = time(NULL);

  var_cell_free(cell);
  return 0;
}

static netinfo_addr_t *
netinfo_addr_from_tor_addr(const tor_addr_t *tor_addr)
{
  sa_family_t addr_family = tor_addr_family(tor_addr);

  if (BUG(addr_family != AF_INET && addr_family != AF_INET6))
    return NULL;

  netinfo_addr_t *netinfo_addr = netinfo_addr_new();

  if (addr_family == AF_INET) {
    netinfo_addr_set_addr_type(netinfo_addr, NETINFO_ADDR_TYPE_IPV4);
    netinfo_addr_set_len(netinfo_addr, 4);
    netinfo_addr_set_addr_ipv4(netinfo_addr, tor_addr_to_ipv4h(tor_addr));
  } else if (addr_family == AF_INET6) {
    netinfo_addr_set_addr_type(netinfo_addr, NETINFO_ADDR_TYPE_IPV6);
    netinfo_addr_set_len(netinfo_addr, 16);
    uint8_t *ipv6_buf = netinfo_addr_getarray_addr_ipv6(netinfo_addr);
    const uint8_t *in6_addr = tor_addr_to_in6_addr8(tor_addr);
    memcpy(ipv6_buf, in6_addr, 16);
  }

  return netinfo_addr;
}

/** Send a NETINFO cell on <b>conn</b>, telling the other server what we know
 * about their address, our address, and the current time. */
MOCK_IMPL(int,
connection_or_send_netinfo,(or_connection_t *conn))
{
  cell_t cell;
  time_t now = time(NULL);
  const routerinfo_t *me;
  int r = -1;

  tor_assert(conn->handshake_state);

  if (conn->handshake_state->sent_netinfo) {
    log_warn(LD_BUG, "Attempted to send an extra netinfo cell on a connection "
             "where we already sent one.");
    return 0;
  }

  memset(&cell, 0, sizeof(cell_t));
  cell.command = CELL_NETINFO;

  netinfo_cell_t *netinfo_cell = netinfo_cell_new();

  /* Timestamp, if we're a relay. */
  if (public_server_mode(get_options()) || ! conn->is_outgoing)
    netinfo_cell_set_timestamp(netinfo_cell, (uint32_t)now);

  /* Their address. */
  const tor_addr_t *remote_tor_addr = &TO_CONN(conn)->addr;
  /* We can safely use TO_CONN(conn)->addr here, since we no longer replace
   * it with a canonical address. */
  netinfo_addr_t *their_addr = netinfo_addr_from_tor_addr(remote_tor_addr);

  netinfo_cell_set_other_addr(netinfo_cell, their_addr);

  /* My address -- only include it if I'm a public relay, or if I'm a
   * bridge and this is an incoming connection. If I'm a bridge and this
   * is an outgoing connection, act like a normal client and omit it. */
  if ((public_server_mode(get_options()) || !conn->is_outgoing) &&
      (me = router_get_my_routerinfo())) {
    uint8_t n_my_addrs = 1 + !tor_addr_is_null(&me->ipv6_addr);
    netinfo_cell_set_n_my_addrs(netinfo_cell, n_my_addrs);

    netinfo_cell_add_my_addrs(netinfo_cell,
                              netinfo_addr_from_tor_addr(&me->ipv4_addr));

    if (!tor_addr_is_null(&me->ipv6_addr)) {
      netinfo_cell_add_my_addrs(netinfo_cell,
                                netinfo_addr_from_tor_addr(&me->ipv6_addr));
    }
  }

  const char *errmsg = NULL;
  if ((errmsg = netinfo_cell_check(netinfo_cell))) {
    log_warn(LD_OR, "Failed to validate NETINFO cell with error: %s",
                    errmsg);
    goto cleanup;
  }

  if (netinfo_cell_encode(cell.payload, CELL_PAYLOAD_SIZE,
                          netinfo_cell) < 0) {
    log_warn(LD_OR, "Failed generating NETINFO cell");
    goto cleanup;
  }

  conn->handshake_state->digest_sent_data = 0;
  conn->handshake_state->sent_netinfo = 1;
  connection_or_write_cell_to_buf(&cell, conn);

  r = 0;
 cleanup:
  netinfo_cell_free(netinfo_cell);

  return r;
}
