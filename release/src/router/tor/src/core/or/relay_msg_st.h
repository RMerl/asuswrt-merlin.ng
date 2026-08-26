/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_msg_st.h
 * @brief A relay message which contains a relay command and parameters,
 *        if any, that is from a relay cell.
 **/

#ifndef TOR_RELAY_MSG_ST_H
#define TOR_RELAY_MSG_ST_H

#include "core/or/or.h"

/** A relay message object which contains pointers to the header and payload.
 *
 * One acquires a relay message through the use of an iterator. Once you get a
 * reference, the getters MUST be used to access data.
 *
 * This CAN NOT be made opaque so to avoid heap allocation in the fast path. */
typedef struct relay_msg_t {
  /* Relay command of a message. */
  uint8_t command;
  /* Length of the message body.
   *
   * This value MUST always be less than or equal to the lower of:
   * - the number of bytes available in `body`.
   * - relay_cell_max_format(_, command).
   *
   * (These bounds on the length field are guaranteed by all message decoding
   * functions, and enforced by all message encoding functions.)
   */
  uint16_t length;
  /* Optional routing header: stream ID of a message or 0. */
  streamid_t stream_id;
  /* Indicate if this is a message from a relay early cell. */
  bool is_relay_early;
  /* Message body of a relay message.
   *
   * Code MUST NOT access any part of `body` beyond the first `length` bytes.
   *
   * NOTE that this struct does not own the body; instead, this is a pointer
   * into a different object. */
  const uint8_t *body;
} relay_msg_t;

#endif /* !defined(TOR_RELAY_MSG_ST_H) */
