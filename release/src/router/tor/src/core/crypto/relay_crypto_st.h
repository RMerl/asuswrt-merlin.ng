/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file relay_crypto_st.h
 * @brief Relay-cell encryption state structure.
 **/

#ifndef RELAY_CRYPTO_ST_H
#define RELAY_CRYPTO_ST_H

#include "core/crypto/tor1_crypt_st.h"
#include "core/crypto/relay_crypto_cgo.h"

typedef enum relay_crypto_kind_t {
  RCK_TOR1,
  RCK_CGO,
} relay_crypto_kind_t;

typedef struct cgo_pair_t {
  // NOTE: Using pointers here is a bit awkward; we may want to refactor
  // eventually.
  cgo_crypt_t *fwd;
  cgo_crypt_t *back;
  /* The last tag that we got when originating or recognizing a message */
  uint8_t last_tag[SENDME_TAG_LEN_CGO];
} cgo_pair_t;

struct relay_crypto_t {
  relay_crypto_kind_t kind;
  union {
    struct tor1_crypt_t tor1;
    cgo_pair_t cgo;
  } c;
};

#endif /* !defined(RELAY_CRYPTO_ST_H) */
