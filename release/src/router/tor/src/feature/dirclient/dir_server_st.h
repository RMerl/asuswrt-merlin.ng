/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef DIR_SERVER_ST_H
#define DIR_SERVER_ST_H

#include "lib/cc/torint.h"
#include "core/or/or.h"
#include "feature/nodelist/routerstatus_st.h"

/** Represents information about a single trusted or fallback directory
 * server. */
struct dir_server_t {
  char *description;
  char *nickname;
  char *address; /**< Hostname. */
  /* XX/teor - why do we duplicate the address and port fields here and in
   *           fake_status? Surely we could just use fake_status (#17867). */
  tor_addr_t ipv6_addr; /**< IPv6 address if present; AF_UNSPEC if not */
  uint32_t addr; /**< IPv4 address. */
  uint16_t dir_port; /**< Directory port. */
  uint16_t or_port; /**< OR port: Used for tunneling connections. */
  uint16_t ipv6_orport; /**< OR port corresponding to ipv6_addr. */
  double weight; /** Weight used when selecting this node at random */
  char digest[DIGEST_LEN]; /**< Digest of identity key. */
  char v3_identity_digest[DIGEST_LEN]; /**< Digest of v3 (authority only,
                                        * high-security) identity key. */

  unsigned int is_running:1; /**< True iff we think this server is running. */
  unsigned int is_authority:1; /**< True iff this is a directory authority
                                * of some kind. */

  /** True iff this server has accepted the most recent server descriptor
   * we tried to upload to it. */
  unsigned int has_accepted_serverdesc:1;

  /** What kind of authority is this? (Bitfield.) */
  dirinfo_type_t type;

  time_t addr_current_at; /**< When was the document that we derived the
                           * address information from published? */

  routerstatus_t fake_status; /**< Used when we need to pass this trusted
                               * dir_server_t to
                               * directory_request_set_routerstatus.
                               * as a routerstatus_t.  Not updated by the
                               * router-status management code!
                               **/
};

#endif /* !defined(DIR_SERVER_ST_H) */
