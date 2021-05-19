/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file onion_crypto.h
 * \brief Header file for onion_crypto.c.
 **/

#ifndef TOR_ONION_CRYPTO_H
#define TOR_ONION_CRYPTO_H

typedef struct server_onion_keys_t {
  uint8_t my_identity[DIGEST_LEN];
  crypto_pk_t *onion_key;
  crypto_pk_t *last_onion_key;
  struct di_digest256_map_t *curve25519_key_map;
  struct curve25519_keypair_t *junk_keypair;
} server_onion_keys_t;

void onion_handshake_state_release(onion_handshake_state_t *state);

int onion_skin_create(int type,
                      const extend_info_t *node,
                      onion_handshake_state_t *state_out,
                      uint8_t *onion_skin_out);
int onion_skin_server_handshake(int type,
                      const uint8_t *onion_skin, size_t onionskin_len,
                      const server_onion_keys_t *keys,
                      uint8_t *reply_out,
                      uint8_t *keys_out, size_t key_out_len,
                      uint8_t *rend_nonce_out);
int onion_skin_client_handshake(int type,
                      const onion_handshake_state_t *handshake_state,
                      const uint8_t *reply, size_t reply_len,
                      uint8_t *keys_out, size_t key_out_len,
                      uint8_t *rend_authenticator_out,
                      const char **msg_out);

server_onion_keys_t *server_onion_keys_new(void);
void server_onion_keys_free_(server_onion_keys_t *keys);
#define server_onion_keys_free(keys) \
  FREE_AND_NULL(server_onion_keys_t, server_onion_keys_free_, (keys))

#endif /* !defined(TOR_ONION_CRYPTO_H) */
