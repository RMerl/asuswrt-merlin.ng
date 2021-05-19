/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file proto_ext_or.c
 * @brief Parsing/encoding for the extended OR protocol.
 **/

#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "feature/relay/ext_orport.h"
#include "core/proto/proto_ext_or.h"

/** The size of the header of an Extended ORPort message: 2 bytes for
 *  COMMAND, 2 bytes for BODYLEN */
#define EXT_OR_CMD_HEADER_SIZE 4

/** Read <b>buf</b>, which should contain an Extended ORPort message
 *  from a transport proxy. If well-formed, create and populate
 *  <b>out</b> with the Extended ORport message. Return 0 if the
 *  buffer was incomplete, 1 if it was well-formed and -1 if we
 *  encountered an error while parsing it.  */
int
fetch_ext_or_command_from_buf(buf_t *buf, ext_or_cmd_t **out)
{
  char hdr[EXT_OR_CMD_HEADER_SIZE];
  uint16_t len;

  if (buf_datalen(buf) < EXT_OR_CMD_HEADER_SIZE)
    return 0;
  buf_peek(buf, hdr, sizeof(hdr));
  len = ntohs(get_uint16(hdr+2));
  if (buf_datalen(buf) < (unsigned)len + EXT_OR_CMD_HEADER_SIZE)
    return 0;
  *out = ext_or_cmd_new(len);
  (*out)->cmd = ntohs(get_uint16(hdr));
  (*out)->len = len;
  buf_drain(buf, EXT_OR_CMD_HEADER_SIZE);
  buf_get_bytes(buf, (*out)->body, len);
  return 1;
}
