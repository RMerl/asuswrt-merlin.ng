/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file relay_msg.c
 * \brief Encoding relay messages into cells.
 **/

#define RELAY_MSG_PRIVATE

#include "app/config/config.h"

#include "core/or/cell_st.h"
#include "core/or/circuitlist.h"
#include "core/or/relay.h"
#include "core/or/relay_msg.h"
#include "lib/crypt_ops/crypto_rand.h"

#include "core/or/cell_st.h"
#include "core/or/relay_msg_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/or_circuit_st.h"

/*
 * Public API
 */

/** Free the given relay message. */
void
relay_msg_free_(relay_msg_t *msg)
{
  if (!msg) {
    return;
  }
  tor_free(msg);
}

/** Clear a relay message as in free its content and reset all fields to 0.
 * This is useful for stack allocated memory. */
void
relay_msg_clear(relay_msg_t *msg)
{
  tor_assert(msg);
  memset(msg, 0, sizeof(*msg));
}

/* Positions of fields within a v0 message. */
#define V0_CMD_OFFSET 0
#define V0_STREAM_ID_OFFSET 3
#define V0_LEN_OFFSET 9
#define V0_PAYLOAD_OFFSET 11

/* Positions of fields within a v1 message. */
#define V1_CMD_OFFSET 16
#define V1_LEN_OFFSET 17
#define V1_STREAM_ID_OFFSET 19
#define V1_PAYLOAD_OFFSET_NO_STREAM_ID 19
#define V1_PAYLOAD_OFFSET_WITH_STREAM_ID 21

/** Allocate a new relay message and copy the content of the given message.
 *
 * This message allocation _will_ own its body, even if the original did not.
 *
 * Requires that msg is well-formed, and that its length is within
 * allowable bounds.
 **/
relay_msg_t *
relay_msg_copy(const relay_msg_t *msg)
{
  tor_assert(msg->length <= RELAY_PAYLOAD_SIZE_MAX);
  void *alloc = tor_malloc_zero(sizeof(relay_msg_t) + msg->length);
  relay_msg_t *new_msg = alloc;
  uint8_t *body = ((uint8_t*)alloc) + sizeof(relay_msg_t);

  memcpy(new_msg, msg, sizeof(*msg));
  new_msg->body = body;
  memcpy(body, msg->body, msg->length);

  return new_msg;
}

/* Add random bytes to the unused portion of the payload, to foil attacks
 * where the other side can predict all of the bytes in the payload and thus
 * compute the authenticated SENDME cells without seeing the traffic. See
 * proposal 289. */
static void
relay_cell_pad(cell_t *cell, size_t end_of_message)
{
  // We add 4 bytes of zero before padding, for forward-compatibility.
  const size_t skip = 4;

  if (end_of_message + skip >= CELL_PAYLOAD_SIZE) {
    /* nothing to do. */
    return;
  }

  crypto_fast_rng_getbytes(get_thread_fast_rng(),
                           &cell->payload[end_of_message + skip],
                           CELL_PAYLOAD_SIZE - (end_of_message + skip));
}

/** Encode the relay message in 'msg' into cell, according to the
 * v0 rules. */
static int
encode_v0_cell(const relay_msg_t *msg,
               cell_t *cell_out)
{
  size_t maxlen =
    relay_cell_max_payload_size(RELAY_CELL_FORMAT_V0, msg->command);
  IF_BUG_ONCE(msg->length > maxlen) {
    return -1;
  }

  uint8_t *out = cell_out->payload;

  out[V0_CMD_OFFSET] = (uint8_t) msg->command;
  set_uint16(out+V0_STREAM_ID_OFFSET, htons(msg->stream_id));
  set_uint16(out+V0_LEN_OFFSET, htons(msg->length));
  memcpy(out + RELAY_HEADER_SIZE_V0, msg->body, msg->length);
  relay_cell_pad(cell_out, RELAY_HEADER_SIZE_V0 + msg->length);

  return 0;
}

/** Encode the relay message in 'msg' into cell, according to the
 * v0 rules. */
static int
encode_v1_cell(const relay_msg_t *msg,
               cell_t *cell_out)
{
  bool expects_streamid = relay_cmd_expects_streamid_in_v1(msg->command);
  size_t maxlen =
    relay_cell_max_payload_size(RELAY_CELL_FORMAT_V1, msg->command);
  IF_BUG_ONCE(msg->length > maxlen) {
    return -1;
  }

  uint8_t *out = cell_out->payload;
  out[V1_CMD_OFFSET] = msg->command;
  set_uint16(out+V1_LEN_OFFSET, htons(msg->length));
  size_t payload_offset;
  if (expects_streamid) {
    IF_BUG_ONCE(msg->stream_id == 0) {
      return -1;
    }
    set_uint16(out+V1_STREAM_ID_OFFSET, htons(msg->stream_id));
    payload_offset = V1_PAYLOAD_OFFSET_WITH_STREAM_ID;
  } else {
    IF_BUG_ONCE(msg->stream_id != 0) {
      return -1;
    }

    payload_offset = V1_PAYLOAD_OFFSET_NO_STREAM_ID;
  }

  memcpy(out + payload_offset, msg->body, msg->length);
  relay_cell_pad(cell_out, payload_offset + msg->length);
  return 0;
}

/** Try to decode 'cell' into a V0 relay message.
 *
 * Return 0 on success, -1 on error.
 */
static int
decode_v0_cell(const cell_t *cell, relay_msg_t *out)
{
  memset(out, 0, sizeof(relay_msg_t));
  out->is_relay_early = (cell->command == CELL_RELAY_EARLY);

  const uint8_t *body = cell->payload;
  out->command = get_uint8(body + V0_CMD_OFFSET);
  out->stream_id = ntohs(get_uint16(body + V0_STREAM_ID_OFFSET));
  out->length = ntohs(get_uint16(body + V0_LEN_OFFSET));

  if (out->length > CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V0) {
    return -1;
  }
  out->body = body + V0_PAYLOAD_OFFSET;

  return 0;
}

/** Try to decode 'cell' into a V1 relay message.
 *
 * Return 0 on success, -1 on error.=
 */
static int
decode_v1_cell(const cell_t *cell, relay_msg_t *out)
{
  memset(out, 0, sizeof(relay_msg_t));
  out->is_relay_early = (cell->command == CELL_RELAY_EARLY);

  const uint8_t *body = cell->payload;
  out->command = get_uint8(body + V1_CMD_OFFSET);
  if (! is_known_relay_command(out->command))
    return -1;

  out->length = ntohs(get_uint16(body + V1_LEN_OFFSET));
  size_t payload_offset;
  if (relay_cmd_expects_streamid_in_v1(out->command)) {
    out->stream_id = ntohs(get_uint16(body + V1_STREAM_ID_OFFSET));
    payload_offset = V1_PAYLOAD_OFFSET_WITH_STREAM_ID;
  } else {
    payload_offset = V1_PAYLOAD_OFFSET_NO_STREAM_ID;
  }

  if (out->length > CELL_PAYLOAD_SIZE - payload_offset)
    return -1;
  out->body = body + payload_offset;

  return 0;
}
/**
 * Encode 'msg' into 'cell' according to the rules of 'format'.
 *
 * Does not set any "recognized", "digest" or "tag" fields,
 * since those are necessarily part of the crypto logic.
 *
 * Clears the circuit ID on the cell.
 *
 * Return 0 on success, and -1 if 'msg' is not well-formed.
 */
int
relay_msg_encode_cell(relay_cell_fmt_t format,
                      const relay_msg_t *msg,
                      cell_t *cell_out)
{
  memset(cell_out, 0, sizeof(cell_t));
  cell_out->command = msg->is_relay_early ?
    CELL_RELAY_EARLY : CELL_RELAY;

  switch (format) {
    case RELAY_CELL_FORMAT_V0:
      return encode_v0_cell(msg, cell_out);
    case RELAY_CELL_FORMAT_V1:
      return encode_v1_cell(msg, cell_out);
    default:
      tor_fragile_assert();
      return -1;
  }
}

/**
 * Decode 'cell' (which must be RELAY or RELAY_EARLY) into a newly allocated
 * 'relay_msg_t'.
 *
 * Note that the resulting relay_msg_t will have a reference to 'cell'.
 * Do not change 'cell' while the resulting message is still in use!
 *
 * Return -1 on error, and 0 on success.
 */
int
relay_msg_decode_cell_in_place(relay_cell_fmt_t format,
                               const cell_t *cell,
                               relay_msg_t *msg_out)
{
  switch (format) {
    case RELAY_CELL_FORMAT_V0:
      return decode_v0_cell(cell, msg_out);
    case RELAY_CELL_FORMAT_V1:
      return decode_v1_cell(cell, msg_out);
    default:
      tor_fragile_assert();
      return -1;
  }
}

/**
 * As relay_msg_decode_cell_in_place, but allocate a new relay_msg_t
 * on success.
 *
 * Return NULL on error.
 */
relay_msg_t *
relay_msg_decode_cell(relay_cell_fmt_t format,
                      const cell_t *cell)
{
  relay_msg_t *msg = tor_malloc(sizeof(relay_msg_t));
  if (relay_msg_decode_cell_in_place(format, cell, msg) < 0) {
    relay_msg_free(msg);
    return NULL;
  } else {
    return msg;
  }
}
