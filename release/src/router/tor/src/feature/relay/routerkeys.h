/* Copyright (c) 2014-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_ROUTERKEYS_H
#define TOR_ROUTERKEYS_H

#include "lib/crypt_ops/crypto_ed25519.h"

const ed25519_public_key_t *get_master_identity_key(void);
MOCK_DECL(const ed25519_keypair_t *, get_master_signing_keypair,(void));
MOCK_DECL(const struct tor_cert_st *, get_master_signing_key_cert,(void));

const ed25519_keypair_t *get_current_auth_keypair(void);
const struct tor_cert_st *get_current_link_cert_cert(void);
const struct tor_cert_st *get_current_auth_key_cert(void);

void get_master_rsa_crosscert(const uint8_t **cert_out,
                              size_t *size_out);

int router_ed25519_id_is_me(const ed25519_public_key_t *id);

struct tor_cert_st *make_ntor_onion_key_crosscert(
                                  const curve25519_keypair_t *onion_key,
                                  const ed25519_public_key_t *master_id_key,
                                  time_t now, time_t lifetime,
                                  int *sign_out);
uint8_t *make_tap_onion_key_crosscert(const crypto_pk_t *onion_key,
                                  const ed25519_public_key_t *master_id_key,
                                  const crypto_pk_t *rsa_id_key,
                                  int *len_out);

int log_cert_expiration(void);
int load_ed_keys(const or_options_t *options, time_t now);
int should_make_new_ed_keys(const or_options_t *options, const time_t now);

int generate_ed_link_cert(const or_options_t *options, time_t now, int force);

void routerkeys_free_all(void);

#ifdef TOR_UNIT_TESTS
const ed25519_keypair_t *get_master_identity_keypair(void);
void init_mock_ed_keys(const crypto_pk_t *rsa_identity_key);
#endif

#endif /* !defined(TOR_ROUTERKEYS_H) */
