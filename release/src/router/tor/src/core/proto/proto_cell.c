/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file proto_cell.c
 * @brief Decodes Tor cells from buffers.
 **/
/* Right now it only handles variable-length cells, but eventually
 * we should refactor other cell-reading code into here. */

#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "core/proto/proto_cell.h"

#include "core/or/connection_or.h"

#include "core/or/var_cell_st.h"

/** True iff the cell command <b>command</b> is one that implies a
 * variable-length cell in Tor link protocol <b>linkproto</b>. */
static inline int
cell_command_is_var_length(uint8_t command, int linkproto)
{
  /* If linkproto is v2 (2), CELL_VERSIONS is the only variable-length cells
   * work as implemented here. If it's 1, there are no variable-length cells.
   * Tor does not support other versions right now, and so can't negotiate
   * them.
   */
  switch (linkproto) {
  case 1:
    /* Link protocol version 1 has no variable-length cells. */
    return 0;
  case 2:
    /* In link protocol version 2, VERSIONS is the only variable-length cell */
    return command == CELL_VERSIONS;
  case 0:
  case 3:
  default:
    /* In link protocol version 3 and later, and in version "unknown",
     * commands 128 and higher indicate variable-length. VERSIONS is
     * grandfathered in. */
    return command == CELL_VERSIONS || command >= 128;
  }
}

/** Check <b>buf</b> for a variable-length cell according to the rules of link
 * protocol version <b>linkproto</b>.  If one is found, pull it off the buffer
 * and assign a newly allocated var_cell_t to *<b>out</b>, and return 1.
 * Return 0 if whatever is on the start of buf_t is not a variable-length
 * cell.  Return 1 and set *<b>out</b> to NULL if there seems to be the start
 * of a variable-length cell on <b>buf</b>, but the whole thing isn't there
 * yet. */
int
fetch_var_cell_from_buf(buf_t *buf, var_cell_t **out, int linkproto)
{
  char hdr[VAR_CELL_MAX_HEADER_SIZE];
  var_cell_t *result;
  uint8_t command;
  uint16_t length;
  const int wide_circ_ids = linkproto >= MIN_LINK_PROTO_FOR_WIDE_CIRC_IDS;
  const int circ_id_len = get_circ_id_size(wide_circ_ids);
  const unsigned header_len = get_var_cell_header_size(wide_circ_ids);
  *out = NULL;
  if (buf_datalen(buf) < header_len)
    return 0;
  buf_peek(buf, hdr, header_len);

  command = get_uint8(hdr + circ_id_len);
  if (!(cell_command_is_var_length(command, linkproto)))
    return 0;

  length = ntohs(get_uint16(hdr + circ_id_len + 1));
  if (buf_datalen(buf) < (size_t)(header_len+length))
    return 1;

  result = var_cell_new(length);
  result->command = command;
  if (wide_circ_ids)
    result->circ_id = ntohl(get_uint32(hdr));
  else
    result->circ_id = ntohs(get_uint16(hdr));

  buf_drain(buf, header_len);
  buf_peek(buf, (char*) result->payload, length);
  buf_drain(buf, length);

  *out = result;
  return 1;
}
