/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file tor1_crypt_st.h
 * @brief Structures for tor1 relay cell encryption.
 **/

#ifndef TOR1_CRYPT_ST_H
#define TOR1_CRYPT_ST_H

struct aes_cnt_cipher_t;
struct crypto_digest_t;

struct tor1_crypt_t {
  /** Encryption key and counter for cells heading towards the OR at this
   * step. */
  struct aes_cnt_cipher_t *f_crypto;
  /** Encryption key and counter for cells heading back from the OR at this
   * step. */
  struct aes_cnt_cipher_t *b_crypto;

  /** Digest state for cells heading towards the OR at this step. */
  struct crypto_digest_t *f_digest; /* for integrity checking */
  /** Digest state for cells heading away from the OR at this step. */
  struct crypto_digest_t *b_digest;

  /** Digest used for the next SENDME cell if any.
   *
   * This digest is updated every time a cell is _originated_ or _recognized_
   * in either direction.  Any operation with this object may
   * invalidate this digest. */
  uint8_t sendme_digest[DIGEST_LEN];
};

#endif /* !defined(TOR1_CRYPT_ST_H) */
