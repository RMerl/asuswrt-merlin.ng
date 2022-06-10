/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_dh_nss.c
 *
 * \brief NSS implementation of Diffie-Hellman over Z_p.
 **/

#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_nss_mgt.h"

#include "lib/encoding/binascii.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

DISABLE_GCC_WARNING("-Wstrict-prototypes")
#include <cryptohi.h>
#include <keyhi.h>
#include <pk11pub.h>
ENABLE_GCC_WARNING("-Wstrict-prototypes")

static int dh_initialized = 0;
static SECKEYDHParams tls_dh_param, circuit_dh_param;
static unsigned char tls_dh_prime_data[DH1024_KEY_LEN];
static unsigned char circuit_dh_prime_data[DH1024_KEY_LEN];
static unsigned char dh_generator_data[1];

void
crypto_dh_init_nss(void)
{
  if (dh_initialized)
    return;

  int r;
  r = base16_decode((char*)tls_dh_prime_data,
                    sizeof(tls_dh_prime_data),
                    TLS_DH_PRIME, strlen(TLS_DH_PRIME));
  tor_assert(r == DH1024_KEY_LEN);
  r = base16_decode((char*)circuit_dh_prime_data,
                    sizeof(circuit_dh_prime_data),
                    OAKLEY_PRIME_2, strlen(OAKLEY_PRIME_2));
  tor_assert(r == DH1024_KEY_LEN);
  dh_generator_data[0] = DH_GENERATOR;

  tls_dh_param.prime.data = tls_dh_prime_data;
  tls_dh_param.prime.len = DH1024_KEY_LEN;
  tls_dh_param.base.data = dh_generator_data;
  tls_dh_param.base.len = 1;

  circuit_dh_param.prime.data = circuit_dh_prime_data;
  circuit_dh_param.prime.len = DH1024_KEY_LEN;
  circuit_dh_param.base.data = dh_generator_data;
  circuit_dh_param.base.len = 1;

  dh_initialized = 1;
}

void
crypto_dh_free_all_nss(void)
{
  dh_initialized = 0;
}

struct crypto_dh_t {
  int dh_type; // XXXX let's remove this later on.
  SECKEYPrivateKey *seckey;
  SECKEYPublicKey *pubkey;
};

crypto_dh_t *
crypto_dh_new(int dh_type)
{
  crypto_dh_t *r = tor_malloc_zero(sizeof(crypto_dh_t));
  r->dh_type = dh_type;
  return r;
}

crypto_dh_t *
crypto_dh_dup(const crypto_dh_t *dh)
{
  tor_assert(dh);
  crypto_dh_t *r = crypto_dh_new(dh->dh_type);
  if (dh->seckey)
    r->seckey = SECKEY_CopyPrivateKey(dh->seckey);
  if (dh->pubkey)
    r->pubkey = SECKEY_CopyPublicKey(dh->pubkey);
  return r;
}

int
crypto_dh_get_bytes(crypto_dh_t *dh)
{
  (void)dh;
  return DH1024_KEY_LEN;
}

int
crypto_dh_generate_public(crypto_dh_t *dh)
{
  tor_assert(dh);
  SECKEYDHParams *p;
  if (dh->dh_type == DH_TYPE_TLS)
    p = &tls_dh_param;
  else
    p = &circuit_dh_param;

  dh->seckey = SECKEY_CreateDHPrivateKey(p, &dh->pubkey, NULL);
  if (!dh->seckey || !dh->pubkey)
    return -1;
  else
    return 0;
}
int
crypto_dh_get_public(crypto_dh_t *dh, char *pubkey_out,
                     size_t pubkey_out_len)
{
  tor_assert(dh);
  tor_assert(pubkey_out);
  if (!dh->pubkey) {
    if (crypto_dh_generate_public(dh) < 0)
      return -1;
  }

  const SECItem *item = &dh->pubkey->u.dh.publicValue;

  if (item->len > pubkey_out_len)
    return -1;

  /* Left-pad the result with 0s. */
  memset(pubkey_out, 0, pubkey_out_len);
  memcpy(pubkey_out + pubkey_out_len - item->len,
         item->data,
         item->len);

  return 0;
}

void
crypto_dh_free_(crypto_dh_t *dh)
{
  if (!dh)
    return;
  if (dh->seckey)
    SECKEY_DestroyPrivateKey(dh->seckey);
  if (dh->pubkey)
    SECKEY_DestroyPublicKey(dh->pubkey);
  tor_free(dh);
}

ssize_t
crypto_dh_handshake(int severity, crypto_dh_t *dh,
                    const char *pubkey, size_t pubkey_len,
                    unsigned char *secret_out,
                    size_t secret_bytes_out)
{
  tor_assert(dh);
  if (pubkey_len > DH1024_KEY_LEN)
    return -1;
  if (!dh->pubkey || !dh->seckey)
    return -1;
  if (secret_bytes_out < DH1024_KEY_LEN)
    return -1;

  SECKEYPublicKey peer_key;
  memset(&peer_key, 0, sizeof(peer_key));
  peer_key.keyType = dhKey;
  peer_key.pkcs11ID = CK_INVALID_HANDLE;

  if (dh->dh_type == DH_TYPE_TLS)
    peer_key.u.dh.prime.data = tls_dh_prime_data; // should never use this code
  else
    peer_key.u.dh.prime.data = circuit_dh_prime_data;
  peer_key.u.dh.prime.len = DH1024_KEY_LEN;
  peer_key.u.dh.base.data = dh_generator_data;
  peer_key.u.dh.base.len = 1;
  peer_key.u.dh.publicValue.data = (unsigned char *)pubkey;
  peer_key.u.dh.publicValue.len = (int) pubkey_len;

  PK11SymKey *sym = PK11_PubDerive(dh->seckey, &peer_key,
                       PR_FALSE, NULL, NULL, CKM_DH_PKCS_DERIVE,
                       CKM_GENERIC_SECRET_KEY_GEN /* ??? */,
                       CKA_DERIVE, 0, NULL);
  if (! sym) {
    crypto_nss_log_errors(severity, "deriving a DH shared secret");
    return -1;
  }

  SECStatus s = PK11_ExtractKeyValue(sym);
  if (s != SECSuccess) {
    crypto_nss_log_errors(severity, "extracting a DH shared secret");
    PK11_FreeSymKey(sym);
    return -1;
  }

  SECItem *result = PK11_GetKeyData(sym);
  tor_assert(result); // This cannot fail.
  if (BUG(result->len > secret_bytes_out)) {
    PK11_FreeSymKey(sym);
    return -1;
  }

  ssize_t len = result->len;
  memcpy(secret_out, result->data, len);
  PK11_FreeSymKey(sym);

  return len;
}
