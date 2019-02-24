/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_PROTO_EXT_OR_H
#define TOR_PROTO_EXT_OR_H

struct buf_t;

/** A parsed Extended ORPort message. */
struct ext_or_cmd_t {
  uint16_t cmd; /** Command type */
  uint16_t len; /** Body length */
  char body[FLEXIBLE_ARRAY_MEMBER]; /** Message body */
};

int fetch_ext_or_command_from_buf(struct buf_t *buf,
                                  struct ext_or_cmd_t **out);

#endif /* !defined(TOR_PROTO_EXT_OR_H) */
