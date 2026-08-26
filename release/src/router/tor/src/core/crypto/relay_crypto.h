/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file relay.h
 * \brief Header file for relay.c.
 **/

#ifndef TOR_RELAY_CRYPTO_H
#define TOR_RELAY_CRYPTO_H

/** Enumeration to identify which relay crypto algorithm is in use. */
typedef enum relay_crypto_alg_t {
  /** Tor1 relay crypto, as used for ordinary circuit hops. */
  RELAY_CRYPTO_ALG_TOR1,
  /** Tor1 relay crypto, as used as an onion service client for
   * the shared virtual HS hop created with an INTRODUCE/RENVEZVOUS
   * handshake. */
  RELAY_CRYPTO_ALG_TOR1_HSC,
  /** Tor1 relay crypto, as used as an onion service for
   * the shared virtual HS hop created with an INTRODUCE/RENVEZVOUS
   * handshake. */
  RELAY_CRYPTO_ALG_TOR1_HSS,
  /** CGO crypto, as used at a client */
  RELAY_CRYPTO_ALG_CGO_CLIENT,
  /** CGO crypto, as used at a relay */
  RELAY_CRYPTO_ALG_CGO_RELAY,
} relay_crypto_alg_t;

/** Largest possible return value for relay_crypto_key_material_len. */
/* This is 2x the length needed for a single cgo direction with 256-bit AES
 */
#define MAX_RELAY_KEY_MATERIAL_LEN 224

ssize_t relay_crypto_key_material_len(relay_crypto_alg_t alg);

int relay_crypto_init(relay_crypto_alg_t alg,
                      relay_crypto_t *crypto,
                      const char *key_data, size_t key_data_len);

int relay_decrypt_cell(circuit_t *circ, cell_t *cell,
                       cell_direction_t cell_direction,
                       crypt_path_t **layer_hint, char *recognized);
void relay_encrypt_cell_outbound(cell_t *cell, origin_circuit_t *or_circ,
                            crypt_path_t *layer_hint);
int relay_encrypt_cell_inbound(cell_t *cell, or_circuit_t *or_circ);

void relay_crypto_clear(relay_crypto_t *crypto);

void relay_crypto_assert_ok(const relay_crypto_t *crypto);

const uint8_t *relay_crypto_get_sendme_tag(relay_crypto_t *crypto,
                                           size_t *len_out);
size_t relay_crypto_sendme_tag_len(const relay_crypto_t *crypto);

#endif /* !defined(TOR_RELAY_CRYPTO_H) */
