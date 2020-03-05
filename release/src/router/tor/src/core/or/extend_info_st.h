/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef EXTEND_INFO_ST_H
#define EXTEND_INFO_ST_H

#include "lib/crypt_ops/crypto_curve25519.h"
#include "lib/crypt_ops/crypto_ed25519.h"

/** Information on router used when extending a circuit. We don't need a
 * full routerinfo_t to extend: we only need addr:port:keyid to build an OR
 * connection, and onion_key to create the onionskin. Note that for onehop
 * general-purpose tunnels, the onion_key is NULL. */
struct extend_info_t {
  char nickname[MAX_HEX_NICKNAME_LEN+1]; /**< This router's nickname for
                                          * display. */
  /** Hash of this router's RSA identity key. */
  char identity_digest[DIGEST_LEN];
  /** Ed25519 identity for this router, if any. */
  ed25519_public_key_t ed_identity;
  uint16_t port; /**< OR port. */
  tor_addr_t addr; /**< IP address. */
  crypto_pk_t *onion_key; /**< Current onionskin key. */
  curve25519_public_key_t curve25519_onion_key;
};

#endif /* !defined(EXTEND_INFO_ST_H) */
