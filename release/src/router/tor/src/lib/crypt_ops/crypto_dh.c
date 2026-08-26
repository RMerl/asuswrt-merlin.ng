/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_dh.c
 * \brief Block of functions related with DH utilities and operations.
 *    over Z_p.  We aren't using this for any new crypto -- EC is more
 *    efficient.
 **/

#include "lib/crypt_ops/compat_openssl.h"
#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_digest.h"
#include "lib/crypt_ops/crypto_hkdf.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

/** Our DH 'g' parameter */
const unsigned DH_GENERATOR = 2;
/** This is ffdhe2048 from RFC 7919.
 */
const char TLS_DH_PRIME[] =
  "FFFFFFFFFFFFFFFFADF85458A2BB4A9AAFDC5620273D3CF1"
  "D8B9C583CE2D3695A9E13641146433FBCC939DCE249B3EF9"
  "7D2FE363630C75D8F681B202AEC4617AD3DF1ED5D5FD6561"
  "2433F51F5F066ED0856365553DED1AF3B557135E7F57C935"
  "984F0C70E0E68B77E2A689DAF3EFE8721DF158A136ADE735"
  "30ACCA4F483A797ABC0AB182B324FB61D108A94BB2C8E3FB"
  "B96ADAB760D7F4681D4F42A3DE394DF4AE56EDE76372BB19"
  "0B07A7C8EE0A6D709E02FCE1CDF7E2ECC03404CD28342F61"
  "9172FE9CE98583FF8E4F1232EEF28183C3FE3B1B4C6FAD73"
  "3BB5FCBC2EC22005C58EF1837D1683B2C6F34A26C1B2EFFA"
  "886B423861285C97FFFFFFFFFFFFFFFF";
/**
 * This is from rfc2409, section 6.2.  It's a safe prime, and
 * supposedly it equals:
 * 2^1024 - 2^960 - 1 + 2^64 * { [2^894 pi] + 129093 }.
 */
const char OAKLEY_PRIME_2[] =
  "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E08"
  "8A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B"
  "302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9"
  "A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE6"
  "49286651ECE65381FFFFFFFFFFFFFFFF";

void
crypto_dh_init(void)
{
#ifdef ENABLE_OPENSSL
  crypto_dh_init_openssl();
#endif
#ifdef ENABLE_NSS
  crypto_dh_init_nss();
#endif
}

void
crypto_dh_free_all(void)
{
#ifdef ENABLE_OPENSSL
  crypto_dh_free_all_openssl();
#endif
#ifdef ENABLE_NSS
  crypto_dh_free_all_nss();
#endif
}

/** Given a DH key exchange object, and our peer's value of g^y (as a
 * <b>pubkey_len</b>-byte value in <b>pubkey</b>) generate
 * <b>secret_bytes_out</b> bytes of shared key material and write them
 * to <b>secret_out</b>.  Return the number of bytes generated on success,
 * or -1 on failure.
 *
 * (We generate key material by computing
 *         SHA1( g^xy || "\x00" ) || SHA1( g^xy || "\x01" ) || ...
 * where || is concatenation.)
 */
ssize_t
crypto_dh_compute_secret(int severity, crypto_dh_t *dh,
                         const char *pubkey, size_t pubkey_len,
                         char *secret_out, size_t secret_bytes_out)
{
  tor_assert(secret_bytes_out/DIGEST_LEN <= 255);

  unsigned char *secret_tmp = NULL;
  size_t secret_len=0, secret_tmp_len=0;
  secret_tmp_len = crypto_dh_get_bytes(dh);
  secret_tmp = tor_malloc(secret_tmp_len);

  ssize_t result = crypto_dh_handshake(severity, dh, pubkey, pubkey_len,
                                   secret_tmp, secret_tmp_len);
  if (result < 0)
    goto error;

  secret_len = result;
  if (crypto_expand_key_material_TAP(secret_tmp, secret_len,
                                     (uint8_t*)secret_out, secret_bytes_out)<0)
    goto error;
  secret_len = secret_bytes_out;

  goto done;
 error:
  result = -1;
 done:
  if (secret_tmp) {
    memwipe(secret_tmp, 0, secret_tmp_len);
    tor_free(secret_tmp);
  }
  if (result < 0)
    return result;
  else
    return secret_len;
}
