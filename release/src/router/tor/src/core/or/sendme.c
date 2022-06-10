/* Copyright (c) 2019-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file sendme.c
 * \brief Code that is related to SENDME cells both in terms of
 *        creating/parsing cells and handling the content.
 */

#define SENDME_PRIVATE

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/crypto/relay_crypto.h"
#include "core/mainloop/connection.h"
#include "core/or/cell_st.h"
#include "core/or/crypt_path.h"
#include "core/or/circuitlist.h"
#include "core/or/circuituse.h"
#include "core/or/or_circuit_st.h"
#include "core/or/relay.h"
#include "core/or/sendme.h"
#include "core/or/congestion_control_common.h"
#include "core/or/congestion_control_flow.h"
#include "feature/nodelist/networkstatus.h"
#include "lib/ctime/di_ops.h"
#include "trunnel/sendme_cell.h"

/* Return the minimum version given by the consensus (if any) that should be
 * used when emitting a SENDME cell. */
STATIC int
get_emit_min_version(void)
{
  return networkstatus_get_param(NULL, "sendme_emit_min_version",
                                 SENDME_EMIT_MIN_VERSION_DEFAULT,
                                 SENDME_EMIT_MIN_VERSION_MIN,
                                 SENDME_EMIT_MIN_VERSION_MAX);
}

/* Return the minimum version given by the consensus (if any) that should be
 * accepted when receiving a SENDME cell. */
STATIC int
get_accept_min_version(void)
{
  return networkstatus_get_param(NULL, "sendme_accept_min_version",
                                 SENDME_ACCEPT_MIN_VERSION_DEFAULT,
                                 SENDME_ACCEPT_MIN_VERSION_MIN,
                                 SENDME_ACCEPT_MIN_VERSION_MAX);
}

/* Pop the first cell digset on the given circuit from the SENDME last digests
 * list. NULL is returned if the list is uninitialized or empty.
 *
 * The caller gets ownership of the returned digest thus is responsible for
 * freeing the memory. */
static uint8_t *
pop_first_cell_digest(const circuit_t *circ)
{
  uint8_t *circ_digest;

  tor_assert(circ);

  if (circ->sendme_last_digests == NULL ||
      smartlist_len(circ->sendme_last_digests) == 0) {
    return NULL;
  }

  circ_digest = smartlist_get(circ->sendme_last_digests, 0);
  smartlist_del_keeporder(circ->sendme_last_digests, 0);
  return circ_digest;
}

/* Return true iff the given cell digest matches the first digest in the
 * circuit sendme list. */
static bool
v1_digest_matches(const uint8_t *circ_digest, const uint8_t *cell_digest)
{
  tor_assert(circ_digest);
  tor_assert(cell_digest);

  /* Compare the digest with the one in the SENDME. This cell is invalid
   * without a perfect match. */
  if (tor_memneq(circ_digest, cell_digest, TRUNNEL_SENDME_V1_DIGEST_LEN)) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "SENDME v1 cell digest do not match.");
    return false;
  }

  /* Digests matches! */
  return true;
}

/* Return true iff the given decoded SENDME version 1 cell is valid and
 * matches the expected digest on the circuit.
 *
 * Validation is done by comparing the digest in the cell from the previous
 * cell we saw which tells us that the other side has in fact seen that cell.
 * See proposal 289 for more details. */
static bool
cell_v1_is_valid(const sendme_cell_t *cell, const uint8_t *circ_digest)
{
  tor_assert(cell);
  tor_assert(circ_digest);

  const uint8_t *cell_digest = sendme_cell_getconstarray_data_v1_digest(cell);
  return v1_digest_matches(circ_digest, cell_digest);
}

/* Return true iff the given cell version can be handled or if the minimum
 * accepted version from the consensus is known to us. */
STATIC bool
cell_version_can_be_handled(uint8_t cell_version)
{
  int accept_version = get_accept_min_version();

  /* We will first check if the consensus minimum accepted version can be
   * handled by us and if not, regardless of the cell version we got, we can't
   * continue. */
  if (accept_version > SENDME_MAX_SUPPORTED_VERSION) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Unable to accept SENDME version %u (from consensus). "
           "We only support <= %u. Probably your tor is too old?",
           accept_version, SENDME_MAX_SUPPORTED_VERSION);
    goto invalid;
  }

  /* Then, is this version below the accepted version from the consensus? If
   * yes, we must not handle it. */
  if (cell_version < accept_version) {
    log_info(LD_PROTOCOL, "Unacceptable SENDME version %u. Only "
                          "accepting %u (from consensus). Closing circuit.",
             cell_version, accept_version);
    goto invalid;
  }

  /* Is this cell version supported by us? */
  if (cell_version > SENDME_MAX_SUPPORTED_VERSION) {
    log_info(LD_PROTOCOL, "SENDME cell version %u is not supported by us. "
                          "We only support <= %u",
             cell_version, SENDME_MAX_SUPPORTED_VERSION);
    goto invalid;
  }

  return true;
 invalid:
  return false;
}

/* Return true iff the encoded SENDME cell in cell_payload of length
 * cell_payload_len is valid. For each version:
 *
 *  0: No validation
 *  1: Authenticated with last cell digest.
 *
 * This is the main critical function to make sure we can continue to
 * send/recv cells on a circuit. If the SENDME is invalid, the circuit should
 * be marked for close by the caller. */
STATIC bool
sendme_is_valid(const circuit_t *circ, const uint8_t *cell_payload,
                size_t cell_payload_len)
{
  uint8_t cell_version;
  uint8_t *circ_digest = NULL;
  sendme_cell_t *cell = NULL;

  tor_assert(circ);
  tor_assert(cell_payload);

  /* An empty payload means version 0 so skip trunnel parsing. We won't be
   * able to parse a 0 length buffer into a valid SENDME cell. */
  if (cell_payload_len == 0) {
    cell_version = 0;
  } else {
    /* First we'll decode the cell so we can get the version. */
    if (sendme_cell_parse(&cell, cell_payload, cell_payload_len) < 0) {
      log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
             "Unparseable SENDME cell received. Closing circuit.");
      goto invalid;
    }
    cell_version = sendme_cell_get_version(cell);
  }

  /* Validate that we can handle this cell version. */
  if (!cell_version_can_be_handled(cell_version)) {
    goto invalid;
  }

  /* Pop the first element that was added (FIFO). We do that regardless of the
   * version so we don't accumulate on the circuit if v0 is used by the other
   * end point. */
  circ_digest = pop_first_cell_digest(circ);
  if (circ_digest == NULL) {
    /* We shouldn't have received a SENDME if we have no digests. Log at
     * protocol warning because it can be tricked by sending many SENDMEs
     * without prior data cell. */
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "We received a SENDME but we have no cell digests to match. "
           "Closing circuit.");
    goto invalid;
  }

  /* Validate depending on the version now. */
  switch (cell_version) {
  case 0x01:
    if (!cell_v1_is_valid(cell, circ_digest)) {
      goto invalid;
    }
    break;
  case 0x00:
    /* Version 0, there is no work to be done on the payload so it is
     * necessarily valid if we pass the version validation. */
    break;
  default:
    log_warn(LD_PROTOCOL, "Unknown SENDME cell version %d received.",
             cell_version);
    tor_assert_nonfatal_unreached();
    break;
  }

  /* Valid cell. */
  sendme_cell_free(cell);
  tor_free(circ_digest);
  return true;
 invalid:
  sendme_cell_free(cell);
  tor_free(circ_digest);
  return false;
}

/* Build and encode a version 1 SENDME cell into payload, which must be at
 * least of RELAY_PAYLOAD_SIZE bytes, using the digest for the cell data.
 *
 * Return the size in bytes of the encoded cell in payload. A negative value
 * is returned on encoding failure. */
STATIC ssize_t
build_cell_payload_v1(const uint8_t *cell_digest, uint8_t *payload)
{
  ssize_t len = -1;
  sendme_cell_t *cell = NULL;

  tor_assert(cell_digest);
  tor_assert(payload);

  cell = sendme_cell_new();

  /* Building a payload for version 1. */
  sendme_cell_set_version(cell, 0x01);
  /* Set the data length field for v1. */
  sendme_cell_set_data_len(cell, TRUNNEL_SENDME_V1_DIGEST_LEN);

  /* Copy the digest into the data payload. */
  memcpy(sendme_cell_getarray_data_v1_digest(cell), cell_digest,
         sendme_cell_get_data_len(cell));

  /* Finally, encode the cell into the payload. */
  len = sendme_cell_encode(payload, RELAY_PAYLOAD_SIZE, cell);

  sendme_cell_free(cell);
  return len;
}

/* Send a circuit-level SENDME on the given circuit using the layer_hint if
 * not NULL. The digest is only used for version 1.
 *
 * Return 0 on success else a negative value and the circuit will be closed
 * because we failed to send the cell on it. */
static int
send_circuit_level_sendme(circuit_t *circ, crypt_path_t *layer_hint,
                          const uint8_t *cell_digest)
{
  uint8_t emit_version;
  uint8_t payload[RELAY_PAYLOAD_SIZE];
  ssize_t payload_len;

  tor_assert(circ);
  tor_assert(cell_digest);

  emit_version = get_emit_min_version();
  switch (emit_version) {
  case 0x01:
    payload_len = build_cell_payload_v1(cell_digest, payload);
    if (BUG(payload_len < 0)) {
      /* Unable to encode the cell, abort. We can recover from this by closing
       * the circuit but in theory it should never happen. */
      return -1;
    }
    log_debug(LD_PROTOCOL, "Emitting SENDME version 1 cell.");
    break;
  case 0x00:
    FALLTHROUGH;
  default:
    /* Unknown version, fallback to version 0 meaning no payload. */
    payload_len = 0;
    log_debug(LD_PROTOCOL, "Emitting SENDME version 0 cell. "
                           "Consensus emit version is %d", emit_version);
    break;
  }

  if (relay_send_command_from_edge(0, circ, RELAY_COMMAND_SENDME,
                                   (char *) payload, payload_len,
                                   layer_hint) < 0) {
    log_warn(LD_CIRC,
             "SENDME relay_send_command_from_edge failed. Circuit's closed.");
    return -1; /* the circuit's closed, don't continue */
  }
  return 0;
}

/* Record the cell digest only if the next cell is expected to be a SENDME. */
static void
record_cell_digest_on_circ(circuit_t *circ, const uint8_t *sendme_digest)
{
  tor_assert(circ);
  tor_assert(sendme_digest);

  /* Add the digest to the last seen list in the circuit. */
  if (circ->sendme_last_digests == NULL) {
    circ->sendme_last_digests = smartlist_new();
  }
  smartlist_add(circ->sendme_last_digests,
                tor_memdup(sendme_digest, DIGEST_LEN));
}

/*
 * Public API
 */

/** Return true iff the next cell for the given cell window is expected to be
 * a SENDME.
 *
 * We are able to know that because the package or inflight window value minus
 * one cell (the possible SENDME cell) should be a multiple of the
 * cells-per-sendme increment value (set via consensus parameter, negotiated
 * for the circuit, and passed in as sendme_inc).
 *
 * This function is used when recording a cell digest and this is done quite
 * low in the stack when decrypting or encrypting a cell. The window is only
 * updated once the cell is actually put in the outbuf.
 */
STATIC bool
circuit_sendme_cell_is_next(int deliver_window, int sendme_inc)
{
  /* Are we at the limit of the increment and if not, we don't expect next
   * cell is a SENDME.
   *
   * We test against the window minus 1 because when we are looking if the
   * next cell is a SENDME, the window (either package or deliver) hasn't been
   * decremented just yet so when this is called, we are currently processing
   * the "window - 1" cell.
   *
   * Because deliver_window starts at CIRCWINDOW_START and counts down,
   * to get the actual number of received cells for this check, we must
   * first convert to receieved cells, or the modulus operator will fail.
   */
  tor_assert(deliver_window <= CIRCWINDOW_START);
  if (((CIRCWINDOW_START - (deliver_window - 1)) % sendme_inc) != 0) {
    return false;
  }

  /* Next cell is expected to be a SENDME. */
  return true;
}

/** Called when we've just received a relay data cell, when we've just
 * finished flushing all bytes to stream <b>conn</b>, or when we've flushed
 * *some* bytes to the stream <b>conn</b>.
 *
 * If conn->outbuf is not too full, and our deliver window is low, send back a
 * suitable number of stream-level sendme cells.
 */
void
sendme_connection_edge_consider_sending(edge_connection_t *conn)
{
  tor_assert(conn);

  int log_domain = TO_CONN(conn)->type == CONN_TYPE_AP ? LD_APP : LD_EXIT;

  /* If we use flow control, we do not send stream sendmes */
  if (edge_uses_flow_control(conn))
    goto end;

  /* Don't send it if we still have data to deliver. */
  if (connection_outbuf_too_full(TO_CONN(conn))) {
    goto end;
  }

  if (circuit_get_by_edge_conn(conn) == NULL) {
    /* This can legitimately happen if the destroy has already arrived and
     * torn down the circuit. */
    log_info(log_domain, "No circuit associated with edge connection. "
                         "Skipping sending SENDME.");
    goto end;
  }

  while (conn->deliver_window <=
         (STREAMWINDOW_START - STREAMWINDOW_INCREMENT)) {
    log_debug(log_domain, "Outbuf %" TOR_PRIuSZ ", queuing stream SENDME.",
              buf_datalen(TO_CONN(conn)->outbuf));
    conn->deliver_window += STREAMWINDOW_INCREMENT;
    if (connection_edge_send_command(conn, RELAY_COMMAND_SENDME,
                                     NULL, 0) < 0) {
      log_debug(LD_CIRC, "connection_edge_send_command failed while sending "
                         "a SENDME. Circuit probably closed, skipping.");
      goto end; /* The circuit's closed, don't continue */
    }
  }

 end:
  return;
}

/** Check if the deliver_window for circuit <b>circ</b> (at hop
 * <b>layer_hint</b> if it's defined) is low enough that we should
 * send a circuit-level sendme back down the circuit. If so, send
 * enough sendmes that the window would be overfull if we sent any
 * more.
 */
void
sendme_circuit_consider_sending(circuit_t *circ, crypt_path_t *layer_hint)
{
  bool sent_one_sendme = false;
  const uint8_t *digest;
  int sendme_inc = sendme_get_inc_count(circ, layer_hint);

  while ((layer_hint ? layer_hint->deliver_window : circ->deliver_window) <=
          CIRCWINDOW_START - sendme_inc) {
    log_debug(LD_CIRC,"Queuing circuit sendme.");
    if (layer_hint) {
      layer_hint->deliver_window += sendme_inc;
      digest = cpath_get_sendme_digest(layer_hint);
    } else {
      circ->deliver_window += sendme_inc;
      digest = relay_crypto_get_sendme_digest(&TO_OR_CIRCUIT(circ)->crypto);
    }
    if (send_circuit_level_sendme(circ, layer_hint, digest) < 0) {
      return; /* The circuit's closed, don't continue */
    }
    /* Current implementation is not suppose to send multiple SENDME at once
     * because this means we would use the same relay crypto digest for each
     * SENDME leading to a mismatch on the other side and the circuit to
     * collapse. Scream loudly if it ever happens so we can address it. */
    tor_assert_nonfatal(!sent_one_sendme);
    sent_one_sendme = true;
  }
}

/* Process a circuit-level SENDME cell that we just received. The layer_hint,
 * if not NULL, is the Exit hop of the connection which means that we are a
 * client. In that case, circ must be an origin circuit. The cell_body_len is
 * the length of the SENDME cell payload (excluding the header). The
 * cell_payload is the payload.
 *
 * This function validates the SENDME's digest, and then dispatches to
 * the appropriate congestion control algorithm in use on the circuit.
 *
 * Return 0 on success (the SENDME is valid and the package window has
 * been updated properly).
 *
 * On error, a negative value is returned, which indicates that the
 * circuit must be closed using the value as the reason for it. */
int
sendme_process_circuit_level(crypt_path_t *layer_hint,
                             circuit_t *circ, const uint8_t *cell_payload,
                             uint16_t cell_payload_len)
{
  tor_assert(circ);
  tor_assert(cell_payload);
  congestion_control_t *cc;

  /* Validate the SENDME cell. Depending on the version, different validation
   * can be done. An invalid SENDME requires us to close the circuit. */
  if (!sendme_is_valid(circ, cell_payload, cell_payload_len)) {
    return -END_CIRC_REASON_TORPROTOCOL;
  }

  /* origin circuits need to count valid sendmes as valid protocol data */
  if (CIRCUIT_IS_ORIGIN(circ)) {
    circuit_read_valid_data(TO_ORIGIN_CIRCUIT(circ), cell_payload_len);
  }

  // Get CC
  if (layer_hint) {
    cc = layer_hint->ccontrol;
  } else {
    cc = circ->ccontrol;
  }

  /* If there is no CC object, assume fixed alg */
  if (!cc) {
    return sendme_process_circuit_level_impl(layer_hint, circ);
  }

  return congestion_control_dispatch_cc_alg(cc, circ, layer_hint);
}

/**
 * Process a SENDME for Tor's original fixed window circuit-level flow control.
 * Updates the package_window and ensures that it does not exceed the max.
 *
 * Returns -END_CIRC_REASON_TORPROTOCOL if the max is exceeded, otherwise
 * returns 0.
 */
int
sendme_process_circuit_level_impl(crypt_path_t *layer_hint, circuit_t *circ)
{
  /* If we are the origin of the circuit, we are the Client so we use the
   * layer hint (the Exit hop) for the package window tracking. */
  if (CIRCUIT_IS_ORIGIN(circ)) {
    /* If we are the origin of the circuit, it is impossible to not have a
     * cpath. Just in case, bug on it and close the circuit. */
    if (BUG(layer_hint == NULL)) {
      return -END_CIRC_REASON_TORPROTOCOL;
    }
    if ((layer_hint->package_window + CIRCWINDOW_INCREMENT) >
        CIRCWINDOW_START_MAX) {
      static struct ratelim_t exit_warn_ratelim = RATELIM_INIT(600);
      log_fn_ratelim(&exit_warn_ratelim, LOG_WARN, LD_PROTOCOL,
                     "Unexpected sendme cell from exit relay. "
                     "Closing circ.");
      return -END_CIRC_REASON_TORPROTOCOL;
    }
    layer_hint->package_window += CIRCWINDOW_INCREMENT;
    log_debug(LD_APP, "circ-level sendme at origin, packagewindow %d.",
              layer_hint->package_window);
  } else {
    /* We aren't the origin of this circuit so we are the Exit and thus we
     * track the package window with the circuit object. */
    if ((circ->package_window + CIRCWINDOW_INCREMENT) >
        CIRCWINDOW_START_MAX) {
      static struct ratelim_t client_warn_ratelim = RATELIM_INIT(600);
      log_fn_ratelim(&client_warn_ratelim, LOG_PROTOCOL_WARN, LD_PROTOCOL,
                     "Unexpected sendme cell from client. "
                     "Closing circ (window %d).", circ->package_window);
      return -END_CIRC_REASON_TORPROTOCOL;
    }
    circ->package_window += CIRCWINDOW_INCREMENT;
    log_debug(LD_EXIT, "circ-level sendme at non-origin, packagewindow %d.",
              circ->package_window);
  }

  return 0;
}

/* Process a stream-level SENDME cell that we just received. The conn is the
 * edge connection (stream) that the circuit circ is associated with. The
 * cell_body_len is the length of the payload (excluding the header).
 *
 * Return 0 on success (the SENDME is valid and the package window has
 * been updated properly).
 *
 * On error, a negative value is returned, which indicates that the
 * circuit must be closed using the value as the reason for it. */
int
sendme_process_stream_level(edge_connection_t *conn, circuit_t *circ,
                            uint16_t cell_body_len)
{
  tor_assert(conn);
  tor_assert(circ);

  if (edge_uses_flow_control(conn)) {
    log_fn(LOG_PROTOCOL_WARN, LD_EDGE,
           "Congestion control got stream sendme");
    return -END_CIRC_REASON_TORPROTOCOL;
  }

  /* Don't allow the other endpoint to request more than our maximum (i.e.
   * initial) stream SENDME window worth of data. Well-behaved stock clients
   * will not request more than this max (as per the check in the while loop
   * of sendme_connection_edge_consider_sending()). */
  if ((conn->package_window + STREAMWINDOW_INCREMENT) >
      STREAMWINDOW_START_MAX) {
    static struct ratelim_t stream_warn_ratelim = RATELIM_INIT(600);
    log_fn_ratelim(&stream_warn_ratelim, LOG_PROTOCOL_WARN, LD_PROTOCOL,
                   "Unexpected stream sendme cell. Closing circ (window %d).",
                   conn->package_window);
    return -END_CIRC_REASON_TORPROTOCOL;
  }
  /* At this point, the stream sendme is valid */
  conn->package_window += STREAMWINDOW_INCREMENT;

  /* We count circuit-level sendme's as valid delivered data because they are
   * rate limited. */
  if (CIRCUIT_IS_ORIGIN(circ)) {
    circuit_read_valid_data(TO_ORIGIN_CIRCUIT(circ), cell_body_len);
  }

  log_debug(CIRCUIT_IS_ORIGIN(circ) ? LD_APP : LD_EXIT,
            "stream-level sendme, package_window now %d.",
            conn->package_window);
  return 0;
}

/* Called when a relay DATA cell is received on the given circuit. If
 * layer_hint is NULL, this means we are the Exit end point else we are the
 * Client. Update the deliver window and return its new value. */
int
sendme_circuit_data_received(circuit_t *circ, crypt_path_t *layer_hint)
{
  int deliver_window, domain;

  if (CIRCUIT_IS_ORIGIN(circ)) {
    tor_assert(layer_hint);
    --layer_hint->deliver_window;
    deliver_window = layer_hint->deliver_window;
    domain = LD_APP;
  } else {
    tor_assert(!layer_hint);
    --circ->deliver_window;
    deliver_window = circ->deliver_window;
    domain = LD_EXIT;
  }

  log_debug(domain, "Circuit deliver_window now %d.", deliver_window);
  return deliver_window;
}

/* Called when a relay DATA cell is received for the given edge connection
 * conn. Update the deliver window and return its new value. */
int
sendme_stream_data_received(edge_connection_t *conn)
{
  tor_assert(conn);

  if (edge_uses_flow_control(conn)) {
    return flow_control_decide_xoff(conn);
  } else {
    return --conn->deliver_window;
  }
}

/* Called when a relay DATA cell is packaged on the given circuit. If
 * layer_hint is NULL, this means we are the Exit end point else we are the
 * Client. Update the package window and return its new value. */
int
sendme_note_circuit_data_packaged(circuit_t *circ, crypt_path_t *layer_hint)
{
  int package_window, domain;
  congestion_control_t *cc;

  tor_assert(circ);

  if (layer_hint) {
    cc = layer_hint->ccontrol;
    domain = LD_APP;
  } else {
    cc = circ->ccontrol;
    domain = LD_EXIT;
  }

  if (cc) {
    congestion_control_note_cell_sent(cc, circ, layer_hint);
  } else {
    /* Fixed alg uses package_window and must update it */

    if (CIRCUIT_IS_ORIGIN(circ)) {
      /* Client side. */
      tor_assert(layer_hint);
      --layer_hint->package_window;
      package_window = layer_hint->package_window;
    } else {
      /* Exit side. */
      tor_assert(!layer_hint);
      --circ->package_window;
      package_window = circ->package_window;
    }
    log_debug(domain, "Circuit package_window now %d.", package_window);
  }

  /* Return appropriate number designating how many cells can still be sent */
  return congestion_control_get_package_window(circ, layer_hint);
}

/* Called when a relay DATA cell is packaged for the given edge connection
 * conn. Update the package window and return its new value. */
int
sendme_note_stream_data_packaged(edge_connection_t *conn, size_t len)
{
  tor_assert(conn);

  if (edge_uses_flow_control(conn)) {
    flow_control_note_sent_data(conn, len);
    if (conn->xoff_received)
      return -1;
    else
      return 1;
  }

  --conn->package_window;
  log_debug(LD_APP, "Stream package_window now %d.", conn->package_window);
  return conn->package_window;
}

/* Record the cell digest into the circuit sendme digest list depending on
 * which edge we are. The digest is recorded only if we expect the next cell
 * that we will receive is a SENDME so we can match the digest. */
void
sendme_record_cell_digest_on_circ(circuit_t *circ, crypt_path_t *cpath)
{
  uint8_t *sendme_digest;

  tor_assert(circ);

  /* Is this the last cell before a SENDME? The idea is that if the
   * package_window reaches a multiple of the increment, after this cell, we
   * should expect a SENDME. */
  if (!circuit_sent_cell_for_sendme(circ, cpath)) {
    return;
  }

  /* Getting the digest is expensive so we only do it once we are certain to
   * record it on the circuit. */
  if (cpath) {
    sendme_digest = cpath_get_sendme_digest(cpath);
  } else {
    sendme_digest =
      relay_crypto_get_sendme_digest(&TO_OR_CIRCUIT(circ)->crypto);
  }

  record_cell_digest_on_circ(circ, sendme_digest);
}

/* Called once we decrypted a cell and recognized it. Record the cell digest
 * as the next sendme digest only if the next cell we'll send on the circuit
 * is expected to be a SENDME. */
void
sendme_record_received_cell_digest(circuit_t *circ, crypt_path_t *cpath)
{
  tor_assert(circ);

  /* Only record if the next cell is expected to be a SENDME. */
  if (!circuit_sendme_cell_is_next(cpath ? cpath->deliver_window :
                                           circ->deliver_window,
                                   sendme_get_inc_count(circ, cpath))) {
    return;
  }

  if (cpath) {
    /* Record incoming digest. */
    cpath_sendme_record_cell_digest(cpath, false);
  } else {
    /* Record forward digest. */
    relay_crypto_record_sendme_digest(&TO_OR_CIRCUIT(circ)->crypto, true);
  }
}

/* Called once we encrypted a cell. Record the cell digest as the next sendme
 * digest only if the next cell we expect to receive is a SENDME so we can
 * match the digests. */
void
sendme_record_sending_cell_digest(circuit_t *circ, crypt_path_t *cpath)
{
  tor_assert(circ);

  /* Only record if the next cell is expected to be a SENDME. */
  if (!circuit_sent_cell_for_sendme(circ, cpath)) {
    goto end;
  }

  if (cpath) {
    /* Record the forward digest. */
    cpath_sendme_record_cell_digest(cpath, true);
  } else {
    /* Record the incoming digest. */
    relay_crypto_record_sendme_digest(&TO_OR_CIRCUIT(circ)->crypto, false);
  }

 end:
  return;
}
