/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_ident.c
 * \brief Contains circuit and connection identifier code for the whole HS
 *        subsystem.
 **/

#include "lib/crypt_ops/crypto_util.h"
#include "feature/hs/hs_ident.h"

/** Return a newly allocated circuit identifier. The given public key is copied
 * identity_pk into the identifier. */
hs_ident_circuit_t *
hs_ident_circuit_new(const ed25519_public_key_t *identity_pk)
{
  hs_ident_circuit_t *ident = tor_malloc_zero(sizeof(*ident));
  ed25519_pubkey_copy(&ident->identity_pk, identity_pk);
  return ident;
}

/** Free the given circuit identifier. */
void
hs_ident_circuit_free_(hs_ident_circuit_t *ident)
{
  if (ident == NULL) {
    return;
  }
  memwipe(ident, 0, sizeof(hs_ident_circuit_t));
  tor_free(ident);
}

/** For a given circuit identifier src, return a newly allocated copy of it.
 * This can't fail. */
hs_ident_circuit_t *
hs_ident_circuit_dup(const hs_ident_circuit_t *src)
{
  hs_ident_circuit_t *ident = tor_malloc_zero(sizeof(*ident));
  memcpy(ident, src, sizeof(*ident));
  return ident;
}

/** For a given directory connection identifier src, return a newly allocated
 * copy of it. This can't fail. */
hs_ident_dir_conn_t *
hs_ident_dir_conn_dup(const hs_ident_dir_conn_t *src)
{
  hs_ident_dir_conn_t *ident = tor_malloc_zero(sizeof(*ident));
  memcpy(ident, src, sizeof(*ident));
  return ident;
}

/** Free the given directory connection identifier. */
void
hs_ident_dir_conn_free_(hs_ident_dir_conn_t *ident)
{
  if (ident == NULL) {
    return;
  }
  memwipe(ident, 0, sizeof(hs_ident_dir_conn_t));
  tor_free(ident);
}

/** Initialized the allocated ident object with identity_pk and blinded_pk.
 * None of them can be NULL since a valid directory connection identifier must
 * have all fields set. */
void
hs_ident_dir_conn_init(const ed25519_public_key_t *identity_pk,
                       const ed25519_public_key_t *blinded_pk,
                       hs_ident_dir_conn_t *ident)
{
  tor_assert(identity_pk);
  tor_assert(blinded_pk);
  tor_assert(ident);

  ed25519_pubkey_copy(&ident->identity_pk, identity_pk);
  ed25519_pubkey_copy(&ident->blinded_pk, blinded_pk);
}

/** Return a newly allocated edge connection identifier. The given public key
 * identity_pk is copied into the identifier. */
hs_ident_edge_conn_t *
hs_ident_edge_conn_new(const ed25519_public_key_t *identity_pk)
{
  hs_ident_edge_conn_t *ident = tor_malloc_zero(sizeof(*ident));
  ed25519_pubkey_copy(&ident->identity_pk, identity_pk);
  return ident;
}

/** Free the given edge connection identifier. */
void
hs_ident_edge_conn_free_(hs_ident_edge_conn_t *ident)
{
  if (ident == NULL) {
    return;
  }
  memwipe(ident, 0, sizeof(hs_ident_edge_conn_t));
  tor_free(ident);
}

/** Return true if the given ident is valid for an introduction circuit. */
int
hs_ident_intro_circ_is_valid(const hs_ident_circuit_t *ident)
{
  if (ident == NULL) {
    goto invalid;
  }

  if (ed25519_public_key_is_zero(&ident->identity_pk)) {
    goto invalid;
  }

  if (ed25519_public_key_is_zero(&ident->intro_auth_pk)) {
    goto invalid;
  }

  /* Valid. */
  return 1;
 invalid:
  return 0;
}
