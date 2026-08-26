/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file relay.h
 * \brief Header file for relay_crypto_tor1.c.
 **/

#ifndef TOR_RELAY_CRYPTO_TOR1_H
#define TOR_RELAY_CRYPTO_TOR1_H

typedef struct tor1_crypt_t tor1_crypt_t;

void tor1_crypt_client_originate(tor1_crypt_t *tor1,
                                 cell_t *cell);
void tor1_crypt_relay_originate(tor1_crypt_t *tor1,
                                cell_t *cell);
void tor1_crypt_relay_backward(tor1_crypt_t *tor1, cell_t *cell);
bool tor1_crypt_relay_forward(tor1_crypt_t *tor1, cell_t *cell);
bool tor1_crypt_client_backward(tor1_crypt_t *tor1, cell_t *cell);
void tor1_crypt_client_forward(tor1_crypt_t *tor1, cell_t *cell);

size_t tor1_key_material_len(bool is_hs);
int tor1_crypt_init(tor1_crypt_t *crypto,
                    const char *key_data, size_t key_data_len,
                    int reverse, int is_hs_v3);
void tor1_crypt_assert_ok(const tor1_crypt_t *tor1);
void tor1_crypt_clear(tor1_crypt_t *crypto);

#endif /* !defined(TOR_RELAY_CRYPTO_TOR1_H) */
