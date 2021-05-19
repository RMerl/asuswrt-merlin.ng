/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file proto_control0.c
 * @brief Code to detect the obsolete v0 control protocol.
 **/

#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "core/proto/proto_control0.h"

/** Return 1 iff buf looks more like it has an (obsolete) v0 controller
 * command on it than any valid v1 controller command. */
int
peek_buf_has_control0_command(buf_t *buf)
{
  if (buf_datalen(buf) >= 4) {
    char header[4];
    uint16_t cmd;
    buf_peek(buf, header, sizeof(header));
    cmd = ntohs(get_uint16(header+2));
    if (cmd <= 0x14)
      return 1; /* This is definitely not a v1 control command. */
  }
  return 0;
}
