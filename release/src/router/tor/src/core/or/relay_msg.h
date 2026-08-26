/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file relay_msg.h
 * \brief Header file for relay_msg.c.
 **/

#ifndef TOR_RELAY_MSG_H
#define TOR_RELAY_MSG_H

#include "core/or/or.h"

#include "core/or/relay_msg_st.h"

/* Relay message */
void relay_msg_free_(relay_msg_t *msg);
void relay_msg_clear(relay_msg_t *msg);
relay_msg_t *relay_msg_copy(const relay_msg_t *msg);

int relay_msg_encode_cell(relay_cell_fmt_t format,
                          const relay_msg_t *msg,
                          cell_t *cell_out) ATTR_WUR;
int relay_msg_decode_cell_in_place(relay_cell_fmt_t format,
                                   const cell_t *cell,
                                   relay_msg_t *msg_out) ATTR_WUR;
relay_msg_t *relay_msg_decode_cell(
                          relay_cell_fmt_t format,
                          const cell_t *cell) ATTR_WUR;

#define relay_msg_free(msg) \
  FREE_AND_NULL(relay_msg_t, relay_msg_free_, (msg))

/* Getters */

/*
 * NOTE: The following are inlined for performance reasons. These values are
 * accessed everywhere and so, even if not expensive, we avoid a function call.
 */

/** Return true iff 'cmd' uses a stream ID when using
 * the v1 relay message format. */
static bool
relay_cmd_expects_streamid_in_v1(uint8_t relay_command)
{
  switch (relay_command) {
    case RELAY_COMMAND_BEGIN:
    case RELAY_COMMAND_BEGIN_DIR:
    case RELAY_COMMAND_CONNECTED:
    case RELAY_COMMAND_DATA:
    case RELAY_COMMAND_END:
    case RELAY_COMMAND_RESOLVE:
    case RELAY_COMMAND_RESOLVED:
    case RELAY_COMMAND_XOFF:
    case RELAY_COMMAND_XON:
      return true;
    default:
      return false;
  }
}

/** Return the size of the relay cell payload for the given relay
 * cell format. */
static inline size_t
relay_cell_max_payload_size(relay_cell_fmt_t format,
                            uint8_t relay_command)
{
  switch (format) {
    case RELAY_CELL_FORMAT_V0:
      return CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V0;
    case RELAY_CELL_FORMAT_V1: {
      if (relay_cmd_expects_streamid_in_v1(relay_command)) {
        return CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_WITH_STREAM_ID;
      } else {
        return CELL_PAYLOAD_SIZE - RELAY_HEADER_SIZE_V1_NO_STREAM_ID;
      }
    }
    default:
      tor_fragile_assert();
      return 0;
  }
}

#ifdef RELAY_MSG_PRIVATE

#endif /* RELAY_MSG_PRIVATE */

#endif /* TOR_RELAY_MSG_H */
