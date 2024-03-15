/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file conflux_cell.c
 * \brief XXX: Write a brief introduction to this module.
 **/

#define CONFLUX_CELL_PRIVATE

#include "app/config/config.h"

#include "core/or/conflux.h"
#include "core/or/conflux_cell.h"
#include "core/or/relay.h"
#include "core/or/circuitlist.h"

#include "lib/crypt_ops/crypto_rand.h"

#include "trunnel/conflux.h"

#include "core/or/crypt_path_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

STATIC ssize_t
build_link_cell(const conflux_cell_link_t *link, uint8_t *cell_out)
{
  ssize_t cell_len = -1;
  trn_cell_conflux_link_t *cell = NULL;
  trn_cell_conflux_link_payload_v1_t *payload = NULL;

  tor_assert(cell_out);

  cell = trn_cell_conflux_link_new();
  trn_cell_conflux_link_set_version(cell, 0x01);

  payload = trn_cell_conflux_link_payload_v1_new();

  /* Set the nonce. */
  size_t nonce_len = trn_cell_conflux_link_payload_v1_getlen_nonce(payload);
  tor_assert(nonce_len == sizeof(link->nonce));
  memcpy(trn_cell_conflux_link_payload_v1_getarray_nonce(payload),
         link->nonce, nonce_len);

  /* Set the sequence number. */
  trn_cell_conflux_link_payload_v1_set_last_seqno_recv(payload,
                                                       link->last_seqno_recv);
  trn_cell_conflux_link_payload_v1_set_last_seqno_sent(payload,
                                                       link->last_seqno_sent);

  /* Set the algorithm */
  trn_cell_conflux_link_payload_v1_set_desired_ux(payload, link->desired_ux);

  /* Encode payload. */
  ssize_t pay_len = trn_cell_conflux_link_payload_v1_encoded_len(payload);
  tor_assert(pay_len >= 0);

  trn_cell_conflux_link_setlen_payload(cell, pay_len);

  trn_cell_conflux_link_payload_v1_encode(
      trn_cell_conflux_link_getarray_payload(cell),
      trn_cell_conflux_link_getlen_payload(cell), payload);

  /* Encode cell. */
  cell_len = trn_cell_conflux_link_encode(cell_out, RELAY_PAYLOAD_SIZE, cell);

  trn_cell_conflux_link_payload_v1_free(payload);
  trn_cell_conflux_link_free(cell);
  return cell_len;
}

static ssize_t
build_linked_cell(const conflux_cell_link_t *link, uint8_t *cell_out)
{
  /* Same payload. This might not be true in the future but for now, we don't
   * need to duplicate the code as it is really the same. */
  return build_link_cell(link, cell_out);
}

static ssize_t
build_linked_ack_cell(uint8_t *cell_out)
{
  ssize_t cell_len = -1;
  trn_cell_conflux_linked_ack_t *cell = NULL;

  tor_assert(cell_out);

  cell = trn_cell_conflux_linked_ack_new();
  cell_len = trn_cell_conflux_linked_ack_encode(cell_out, RELAY_PAYLOAD_SIZE,
                                                cell);

  trn_cell_conflux_linked_ack_free(cell);
  return cell_len;
}

bool
conflux_cell_send_link(const conflux_cell_link_t *link, origin_circuit_t *circ)
{
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};
  ssize_t cell_len;

  tor_assert(link);
  tor_assert(circ);

  log_info(LD_CIRC, "Sending CONFLUX_LINK cell onto origin circuit");

  /* Build the CONFLUX_LINK cell. */
  cell_len = build_link_cell(link, payload);
  if (BUG(cell_len < 0)) {
    log_info(LD_CIRC, "Unable to build CONFLUX_LINK cell.");
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_INTERNAL);
    goto err;
  }

  /* Send the cell to the endpoint of the circuit. */
  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_CONFLUX_LINK,
                                   (char *) payload, cell_len,
                                   circ->cpath->prev) < 0) {
    log_info(LD_CIRC, "Unable to send CONFLUX_LINK cell.");
    goto err;
  }

  return true;

 err:
  return false;
}

bool
conflux_cell_send_linked(const conflux_cell_link_t *link, or_circuit_t *circ)
{
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};
  ssize_t cell_len;

  tor_assert(link);
  tor_assert(circ);

  log_info(LD_CIRC, "Sending CONFLUX_LINKED cell onto OR circuit");

  /* Build the CONFLUX_LINK cell. */
  cell_len = build_linked_cell(link, payload);
  if (BUG(cell_len < 0)) {
    log_info(LD_CIRC, "Unable to build CONFLUX_LINKED cell.");
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_INTERNAL);
    goto err;
  }

  /* Send back the LINKED cell. */
  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_CONFLUX_LINKED,
                                   (char *) payload, cell_len, NULL) < 0) {
    log_info(LD_CIRC, "Unable to send CONFLUX_LINKED cell.");
    goto err;
  }

  return true;

 err:
  return false;
}

bool
conflux_cell_send_linked_ack(origin_circuit_t *circ)
{
  uint8_t payload[RELAY_PAYLOAD_SIZE] = {0};
  ssize_t cell_len;

  tor_assert(circ);

  log_info(LD_CIRC, "Sending CONFLUX_LINKED_ACK cell onto origin circuit");

  /* Build the CONFLUX_LINKED_ACK cell. */
  cell_len = build_linked_ack_cell(payload);
  if (BUG(cell_len < 0)) {
    log_info(LD_CIRC, "Unable to build CONFLUX_LINKED_ACK cell.");
    circuit_mark_for_close(TO_CIRCUIT(circ), END_CIRC_REASON_INTERNAL);
    goto err;
  }

  /* Send the cell to the endpoint of the circuit. */
  if (relay_send_command_from_edge(CONTROL_CELL_ID, TO_CIRCUIT(circ),
                                   RELAY_COMMAND_CONFLUX_LINKED_ACK,
                                   (char *) payload, cell_len,
                                   circ->cpath->prev) < 0) {
    log_info(LD_CIRC, "Unable to send CONFLUX_LINKED_ACK cell.");
    goto err;
  }

  return true;

 err:
  return false;
}

static conflux_cell_link_t *
conflux_cell_parse_link_v1(const trn_cell_conflux_link_t *trn_link)
{
  conflux_cell_link_t *link = NULL;
  trn_cell_conflux_link_payload_v1_t *payload = NULL;

  if (trn_cell_conflux_link_payload_v1_parse(&payload,
               trn_cell_conflux_link_getconstarray_payload(trn_link),
               trn_cell_conflux_link_getlen_payload(trn_link)) < 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Unable to parse CONFLUX_LINK v1 payload.");
    goto end;
  }

  link = tor_malloc_zero(sizeof(*link));
  link->version = trn_cell_conflux_link_get_version(trn_link);
  link->desired_ux =
    trn_cell_conflux_link_payload_v1_get_desired_ux(payload);
  link->last_seqno_recv =
    trn_cell_conflux_link_payload_v1_get_last_seqno_recv(payload);
  link->last_seqno_sent =
    trn_cell_conflux_link_payload_v1_get_last_seqno_sent(payload);
  memcpy(link->nonce,
         trn_cell_conflux_link_payload_v1_getconstarray_nonce(payload),
         trn_cell_conflux_link_payload_v1_getlen_nonce(payload));

 end:
  trn_cell_conflux_link_payload_v1_free(payload);
  return link;
}

conflux_cell_link_t *
conflux_cell_parse_link(const cell_t *cell, const uint16_t cell_len)
{
  conflux_cell_link_t *link = NULL;
  trn_cell_conflux_link_t *trn_cell = NULL;

  tor_assert(cell);

  if (trn_cell_conflux_link_parse(&trn_cell,
                                  cell->payload + RELAY_HEADER_SIZE,
                                  cell_len) < 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Unable to parse CONFLUX_LINK cell.");
    goto end;
  }

  uint8_t version = trn_cell_conflux_link_get_version(trn_cell);
  switch (version) {
  case 0x01:
    link = conflux_cell_parse_link_v1(trn_cell);
    break;
  default:
    log_fn(LOG_PROTOCOL_WARN, LD_CIRC,
           "Unsupported version %d in CONFLUX_LINK cell", version);
    goto end;
  }

 end:
  trn_cell_conflux_link_free(trn_cell);
  return link;
}

conflux_cell_link_t *
conflux_cell_parse_linked(const cell_t *cell, const uint16_t cell_len)
{
  /* At the moment, same exact payload so avoid code duplication. */
  return conflux_cell_parse_link(cell, cell_len);
}

conflux_cell_link_t *
conflux_cell_new_link(const uint8_t *nonce, uint64_t last_seqno_sent,
                      uint64_t last_seqno_recv, uint8_t ux)
{
  conflux_cell_link_t *link = tor_malloc_zero(sizeof(*link));

  link->version = 0x01;
  link->desired_ux = ux;

  link->last_seqno_sent = last_seqno_sent;
  link->last_seqno_recv = last_seqno_recv;
  memcpy(link->nonce, nonce, sizeof(link->nonce));

  return link;
}

/**
 * Extracts the sequence number from a switch cell.
 */
uint32_t
conflux_cell_parse_switch(const cell_t *cell, uint16_t rh_len)
{
  uint32_t seq = 0;
  trn_cell_conflux_switch_t *switch_cell = NULL;
  tor_assert(cell);

  if (trn_cell_conflux_switch_parse(&switch_cell,
                                    cell->payload + RELAY_HEADER_SIZE,
                                    rh_len) < 0) {
    log_warn(LD_BUG, "Failed to parse switch cell");
    // Zero counts as a failure to the validation, since legs should
    // not switch after 0 cells.
    return 0;
  }

  seq = trn_cell_conflux_switch_get_seqnum(switch_cell);

  trn_cell_conflux_switch_free(switch_cell);

  return seq;
}

/** Send a RELAY_COMMAND_CONFLUX_SWITCH cell on the circuit. */
bool
conflux_send_switch_command(circuit_t *send_circ, uint64_t relative_seq)
{
  trn_cell_conflux_switch_t *switch_cell = trn_cell_conflux_switch_new();
  cell_t cell;
  bool ret = true;

  tor_assert(send_circ);
  tor_assert(relative_seq < UINT32_MAX);

  memset(&cell, 0, sizeof(cell));

  trn_cell_conflux_switch_set_seqnum(switch_cell, (uint32_t)relative_seq);

  if (trn_cell_conflux_switch_encode(cell.payload, RELAY_PAYLOAD_SIZE,
                                     switch_cell) < 0) {
    log_warn(LD_BUG, "Failed to encode conflux switch cell");
    ret = false;
    goto end;
  }

  /* Send the switch command to the new hop */
  if (CIRCUIT_IS_ORIGIN(send_circ)) {
    relay_send_command_from_edge(0, send_circ,
                               RELAY_COMMAND_CONFLUX_SWITCH,
                               (const char*)cell.payload,
                               RELAY_PAYLOAD_SIZE,
                               TO_ORIGIN_CIRCUIT(send_circ)->cpath->prev);
  } else {
    relay_send_command_from_edge(0, send_circ,
                               RELAY_COMMAND_CONFLUX_SWITCH,
                               (const char*)cell.payload,
                               RELAY_PAYLOAD_SIZE, NULL);
  }

end:
  trn_cell_conflux_switch_free(switch_cell);
  return ret;
}

