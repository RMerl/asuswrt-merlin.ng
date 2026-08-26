/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2025, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file relay_crypto_cgo.h
 * \brief Header file for relay_crypto_cgo.c.
 **/

#ifndef TOR_RELAY_CRYPTO_CGO_H
#define TOR_RELAY_CRYPTO_CGO_H

#include "lib/testsupport/testsupport.h"

/**
 * State to implement forward _or_ reverse crypto between a client and a single
 * hop on a circuit.
 *
 * (There needs to be one of these for each direction.
 */
typedef struct cgo_crypt_t cgo_crypt_t;

typedef enum {
  CGO_MODE_CLIENT_FORWARD,
  CGO_MODE_CLIENT_BACKWARD,
  CGO_MODE_RELAY_FORWARD,
  CGO_MODE_RELAY_BACKWARD,
} cgo_mode_t;

struct cell_t;

size_t cgo_key_material_len(int aesbits);
cgo_crypt_t * cgo_crypt_new(cgo_mode_t mode, int aesbits,
                      const uint8_t *keys, size_t keylen);
void cgo_crypt_free_(cgo_crypt_t *cgo);
#define cgo_crypt_free(cgo) \
  FREE_AND_NULL(cgo_crypt_t, cgo_crypt_free_, (cgo))

void cgo_crypt_relay_forward(cgo_crypt_t *cgo, struct cell_t *cell,
                             const uint8_t **recognized_tag_out);
void cgo_crypt_relay_backward(cgo_crypt_t *cgo, struct cell_t *cell);
void cgo_crypt_relay_originate(cgo_crypt_t *cgo, struct cell_t *cell,
                               const uint8_t **tag_out);
void cgo_crypt_client_forward(cgo_crypt_t *cgo, struct cell_t *cell);
void cgo_crypt_client_originate(cgo_crypt_t *cgo, struct cell_t *cell,
                                const uint8_t **tag_out);
void cgo_crypt_client_backward(cgo_crypt_t *cgo, struct cell_t *cell,
                               const uint8_t **recognized_tag_out);

#ifdef RELAY_CRYPTO_CGO_PRIVATE
/* Internal types and definitions for CGO encryption algorithms.
 *
 * Where reasonable, the identifiers here are chosen to match those
 * in the spec (proposal 359), which in turn were chosen to match
 * those in the paper.
 */

/**
 * Tweakable block cipher, following the LRW2 construction,
 * instantiated with AES.
 *
 * Any given instance can be used for encryption _or_ decryption,
 * not both.
 */
typedef struct cgo_et_t {
  /**
   * AES block cipher instance
   */
  aes_raw_t *kb;
  /**
   * Polyval instance, with expanded key.
   */
  polyvalx_t ku;
} cgo_et_t;
/**
 * Keyed pseudorandom function, based on polyval and AES-CTR.
 */
typedef struct cgo_prf_t {
  /**
   * AES stream cipher: may be 128, 192, or 256 bits.
   */
  aes_cnt_cipher_t *k;
  /**
   * Polyval instance.
   */
  polyval_key_t b;
} cgo_prf_t;
/**
 * Rugged tweakable pseudorandom permutation, using the UIV+ construction.
 *
 * This is, roughly, a wide-block cipher where _encryption_
 * is non-malleable, but where _decryption_ is malleable.
 *
 * UIV+ is the basis of CGO encryption, though it is used in different
 * ways for each of the relay operations.
 */
typedef struct cgo_uiv_t {
  /**
   * Tweakable block cipher instance.
   */
  cgo_et_t j;
  /**
   * PRF instance.
   */
  cgo_prf_t s;
#ifdef TOR_UNIT_TESTS
  /** Testing only: Copy of keys used to instantiate this UIV.
   * We use this in tests so that we can confirm the correctness
   * of cgo_uiv_update().
   */
  uint8_t uiv_keys_[32 * 2 + 16 * 2];
#endif
} cgo_uiv_t;
/**
 * Length of the 'h' component of uiv_tweak_t.
 */
#define ET_TWEAK_LEN_H 16
/**
 * Length of the 'x_r' component of et_tweak_t.
 */
#define ET_TWEAK_LEN_X_R 493

/**
 * Tweak for the UIV+ wide-block cipher.
 */
typedef struct uiv_tweak_t {
  /** H component of the wide-block cipher.
   *
   * This must be ET_TWEAK_LEN_H bytes long.
   **/
  const uint8_t *h;
  /** Additional data component of the wide-block cipher.
   * This value is sent to the cell command (RELAY or RELAY_EARLY)
   * for each relay cell.
   */
  const uint8_t cmd;
} uiv_tweak_t;
/**
 * Tweak for the ET tweakable block cipher.
 */
typedef struct et_tweak_t {
  /** Components from the UIV+ tweak. */
  uiv_tweak_t uiv;
  /**
   * X_R component of the ET tweak.
   *
   * This must be X_R bytes long.
   */
  const uint8_t *x_r;
} et_tweak_t;

/** Length of expected input to the PRF. */
#define PRF_INPUT_LEN 16
/** Output length for cgo_prf_xor_t0(). */
#define PRF_T0_DATA_LEN 493

/** Length of block handled by uiv instantiation. */
#define UIV_BLOCK_LEN 509

STATIC int cgo_et_init(cgo_et_t *et, int aesbits, bool encrypt,
                       const uint8_t *key);
STATIC void cgo_et_set_key(cgo_et_t *et, int aesbits, bool encrypt,
                           const uint8_t *key);
STATIC void cgo_et_encrypt(cgo_et_t *et, const et_tweak_t tweak,
                    uint8_t *block);
STATIC void cgo_et_decrypt(cgo_et_t *et, const et_tweak_t tweak,
                    uint8_t *block);
STATIC void cgo_et_clear(cgo_et_t *et);

STATIC int cgo_prf_init(cgo_prf_t *prf, int aesbits,
                        const uint8_t *key);
STATIC void cgo_prf_set_key(cgo_prf_t *prf, int aesbits,
                           const uint8_t *key);
STATIC void cgo_prf_xor_t0(cgo_prf_t *prf, const uint8_t *input,
                          uint8_t *data);
STATIC void cgo_prf_gen_t1(cgo_prf_t *prf, const uint8_t *input,
                   uint8_t *buf, size_t n);
STATIC void cgo_prf_clear(cgo_prf_t *prf);

STATIC int cgo_uiv_init(cgo_uiv_t *uiv, int aesbits, bool encrypt,
                        const uint8_t *key);
STATIC void cgo_uiv_encrypt(cgo_uiv_t *uiv, const uiv_tweak_t tweak,
                            uint8_t *cell_body);
STATIC void cgo_uiv_decrypt(cgo_uiv_t *uiv, const uiv_tweak_t tweak,
                            uint8_t *cell_body);
STATIC void cgo_uiv_update(cgo_uiv_t *uiv, int aesbits, bool encrypt,
                           uint8_t *nonce);
STATIC void cgo_uiv_clear(cgo_uiv_t *uiv);

struct cgo_crypt_t {
  cgo_uiv_t uiv;
  uint8_t nonce[SENDME_TAG_LEN_CGO];
  uint8_t tprime[SENDME_TAG_LEN_CGO];
  /**
   * Stored version of the last incoming cell tag.
   * Only used for cgo_crypt_relay_fwd, where this information is not
   * otherwise available after encryption.
   */
  uint8_t last_tag_relay_fwd[SENDME_TAG_LEN_CGO];
  uint8_t aes_bytes;
};
#endif

#endif /* !defined(TOR_RELAY_CRYPTO_CGO_H) */
