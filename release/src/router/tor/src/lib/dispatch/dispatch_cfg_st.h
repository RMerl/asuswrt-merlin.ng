/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2018, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_DISPATCH_CFG_ST_H
#define TOR_DISPATCH_CFG_ST_H

struct smartlist_t;

/* Information needed to create a dispatcher, but in a less efficient, more
 * mutable format. */
struct dispatch_cfg_t {
  /** A list of msg_type_id_t (cast to void*), indexed by msg_t. */
  struct smartlist_t *type_by_msg;
  /** A list of channel_id_t (cast to void*), indexed by msg_t. */
  struct smartlist_t *chan_by_msg;
  /** A list of dispatch_rcv_t, indexed by msg_type_id_t. */
  struct smartlist_t *fns_by_type;
  /** A list of dispatch_typefns_t, indexed by msg_t. */
  struct smartlist_t *recv_by_msg;
};

#endif /* !defined(TOR_DISPATCH_CFG_ST_H) */
