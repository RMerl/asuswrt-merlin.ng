/* mac.c  -  message authentication code dispatcher
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "mac-internal.h"


/* This is the list of the digest implementations included in
   libgcrypt.  */
static const gcry_mac_spec_t * const mac_list[] = {
#if USE_SHA1
  &_gcry_mac_type_spec_hmac_sha1,
#endif
#if USE_SHA256
  &_gcry_mac_type_spec_hmac_sha256,
  &_gcry_mac_type_spec_hmac_sha224,
#endif
#if USE_SHA512
  &_gcry_mac_type_spec_hmac_sha512,
  &_gcry_mac_type_spec_hmac_sha384,
  &_gcry_mac_type_spec_hmac_sha512_256,
  &_gcry_mac_type_spec_hmac_sha512_224,
#endif
#if USE_SHA3
  &_gcry_mac_type_spec_hmac_sha3_224,
  &_gcry_mac_type_spec_hmac_sha3_256,
  &_gcry_mac_type_spec_hmac_sha3_384,
  &_gcry_mac_type_spec_hmac_sha3_512,
#endif
#if USE_GOST_R_3411_94
  &_gcry_mac_type_spec_hmac_gost3411_94,
  &_gcry_mac_type_spec_hmac_gost3411_cp,
#endif
#if USE_GOST_R_3411_12
  &_gcry_mac_type_spec_hmac_stribog256,
  &_gcry_mac_type_spec_hmac_stribog512,
#endif
#if USE_WHIRLPOOL
  &_gcry_mac_type_spec_hmac_whirlpool,
#endif
#if USE_RMD160
  &_gcry_mac_type_spec_hmac_rmd160,
#endif
#if USE_TIGER
  &_gcry_mac_type_spec_hmac_tiger1,
#endif
#if USE_MD5
  &_gcry_mac_type_spec_hmac_md5,
#endif
#if USE_MD4
  &_gcry_mac_type_spec_hmac_md4,
#endif
#if USE_BLAKE2
  &_gcry_mac_type_spec_hmac_blake2b_512,
  &_gcry_mac_type_spec_hmac_blake2b_384,
  &_gcry_mac_type_spec_hmac_blake2b_256,
  &_gcry_mac_type_spec_hmac_blake2b_160,
  &_gcry_mac_type_spec_hmac_blake2s_256,
  &_gcry_mac_type_spec_hmac_blake2s_224,
  &_gcry_mac_type_spec_hmac_blake2s_160,
  &_gcry_mac_type_spec_hmac_blake2s_128,
#endif
#if USE_SM3
  &_gcry_mac_type_spec_hmac_sm3,
#endif
#if USE_BLOWFISH
  &_gcry_mac_type_spec_cmac_blowfish,
#endif
#if USE_DES
  &_gcry_mac_type_spec_cmac_tripledes,
#endif
#if USE_CAST5
  &_gcry_mac_type_spec_cmac_cast5,
#endif
#if USE_AES
  &_gcry_mac_type_spec_cmac_aes,
  &_gcry_mac_type_spec_gmac_aes,
  &_gcry_mac_type_spec_poly1305mac_aes,
#endif
#if USE_TWOFISH
  &_gcry_mac_type_spec_cmac_twofish,
  &_gcry_mac_type_spec_gmac_twofish,
  &_gcry_mac_type_spec_poly1305mac_twofish,
#endif
#if USE_SERPENT
  &_gcry_mac_type_spec_cmac_serpent,
  &_gcry_mac_type_spec_gmac_serpent,
  &_gcry_mac_type_spec_poly1305mac_serpent,
#endif
#if USE_RFC2268
  &_gcry_mac_type_spec_cmac_rfc2268,
#endif
#if USE_SEED
  &_gcry_mac_type_spec_cmac_seed,
  &_gcry_mac_type_spec_gmac_seed,
  &_gcry_mac_type_spec_poly1305mac_seed,
#endif
#if USE_CAMELLIA
  &_gcry_mac_type_spec_cmac_camellia,
  &_gcry_mac_type_spec_gmac_camellia,
  &_gcry_mac_type_spec_poly1305mac_camellia,
#endif
#if USE_IDEA
  &_gcry_mac_type_spec_cmac_idea,
#endif
#if USE_GOST28147
  &_gcry_mac_type_spec_cmac_gost28147,
  &_gcry_mac_type_spec_gost28147_imit,
#endif
  &_gcry_mac_type_spec_poly1305mac,
#if USE_SM4
  &_gcry_mac_type_spec_cmac_sm4,
#endif
  NULL,
};

/* HMAC implementations start with index 101 (enum gcry_mac_algos) */
static const gcry_mac_spec_t * const mac_list_algo101[] =
  {
#if USE_SHA256
    &_gcry_mac_type_spec_hmac_sha256,
    &_gcry_mac_type_spec_hmac_sha224,
#else
    NULL,
    NULL,
#endif
#if USE_SHA512
    &_gcry_mac_type_spec_hmac_sha512,
    &_gcry_mac_type_spec_hmac_sha384,
#else
    NULL,
    NULL,
#endif
#if USE_SHA1
    &_gcry_mac_type_spec_hmac_sha1,
#else
    NULL,
#endif
#if USE_MD5
    &_gcry_mac_type_spec_hmac_md5,
#else
    NULL,
#endif
#if USE_MD4
    &_gcry_mac_type_spec_hmac_md4,
#else
    NULL,
#endif
#if USE_RMD160
    &_gcry_mac_type_spec_hmac_rmd160,
#else
    NULL,
#endif
#if USE_TIGER
    &_gcry_mac_type_spec_hmac_tiger1,
#else
    NULL,
#endif
#if USE_WHIRLPOOL
    &_gcry_mac_type_spec_hmac_whirlpool,
#else
    NULL,
#endif
#if USE_GOST_R_3411_94
    &_gcry_mac_type_spec_hmac_gost3411_94,
#else
    NULL,
#endif
#if USE_GOST_R_3411_12
    &_gcry_mac_type_spec_hmac_stribog256,
    &_gcry_mac_type_spec_hmac_stribog512,
#else
    NULL,
    NULL,
#endif
#if USE_MD2
    &_gcry_mac_type_spec_hmac_md2,
#else
    NULL,
#endif
#if USE_SHA3
    &_gcry_mac_type_spec_hmac_sha3_224,
    &_gcry_mac_type_spec_hmac_sha3_256,
    &_gcry_mac_type_spec_hmac_sha3_384,
    &_gcry_mac_type_spec_hmac_sha3_512,
#else
    NULL,
    NULL,
    NULL,
    NULL,
#endif
#if USE_GOST_R_3411_94
    &_gcry_mac_type_spec_hmac_gost3411_cp,
#else
    NULL,
#endif
#if USE_BLAKE2
    &_gcry_mac_type_spec_hmac_blake2b_512,
    &_gcry_mac_type_spec_hmac_blake2b_384,
    &_gcry_mac_type_spec_hmac_blake2b_256,
    &_gcry_mac_type_spec_hmac_blake2b_160,
    &_gcry_mac_type_spec_hmac_blake2s_256,
    &_gcry_mac_type_spec_hmac_blake2s_224,
    &_gcry_mac_type_spec_hmac_blake2s_160,
    &_gcry_mac_type_spec_hmac_blake2s_128,
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif
#if USE_SM3
    &_gcry_mac_type_spec_hmac_sm3,
#else
    NULL,
#endif
#if USE_SHA512
    &_gcry_mac_type_spec_hmac_sha512_256,
    &_gcry_mac_type_spec_hmac_sha512_224,
#else
    NULL,
    NULL,
#endif
  };

/* CMAC implementations start with index 201 (enum gcry_mac_algos) */
static const gcry_mac_spec_t * const mac_list_algo201[] =
  {
#if USE_AES
    &_gcry_mac_type_spec_cmac_aes,
#else
    NULL,
#endif
#if USE_DES
    &_gcry_mac_type_spec_cmac_tripledes,
#else
    NULL,
#endif
#if USE_CAMELLIA
    &_gcry_mac_type_spec_cmac_camellia,
#else
    NULL,
#endif
#if USE_CAST5
    &_gcry_mac_type_spec_cmac_cast5,
#else
    NULL,
#endif
#if USE_BLOWFISH
    &_gcry_mac_type_spec_cmac_blowfish,
#else
    NULL,
#endif
#if USE_TWOFISH
    &_gcry_mac_type_spec_cmac_twofish,
#else
    NULL,
#endif
#if USE_SERPENT
    &_gcry_mac_type_spec_cmac_serpent,
#else
    NULL,
#endif
#if USE_SEED
    &_gcry_mac_type_spec_cmac_seed,
#else
    NULL,
#endif
#if USE_RFC2268
    &_gcry_mac_type_spec_cmac_rfc2268,
#else
    NULL,
#endif
#if USE_IDEA
    &_gcry_mac_type_spec_cmac_idea,
#else
    NULL,
#endif
#if USE_GOST28147
    &_gcry_mac_type_spec_cmac_gost28147,
#else
    NULL,
#endif
#if USE_SM4
    &_gcry_mac_type_spec_cmac_sm4
#else
    NULL
#endif
  };

/* GMAC implementations start with index 401 (enum gcry_mac_algos) */
static const gcry_mac_spec_t * const mac_list_algo401[] =
  {
#if USE_AES
    &_gcry_mac_type_spec_gmac_aes,
#else
    NULL,
#endif
#if USE_CAMELLIA
    &_gcry_mac_type_spec_gmac_camellia,
#else
    NULL,
#endif
#if USE_TWOFISH
    &_gcry_mac_type_spec_gmac_twofish,
#else
    NULL,
#endif
#if USE_SERPENT
    &_gcry_mac_type_spec_gmac_serpent,
#else
    NULL,
#endif
#if USE_SEED
    &_gcry_mac_type_spec_gmac_seed
#else
    NULL
#endif
  };

/* Poly1305-MAC implementations start with index 501 (enum gcry_mac_algos) */
static const gcry_mac_spec_t * const mac_list_algo501[] =
  {
    &_gcry_mac_type_spec_poly1305mac,
#if USE_AES
    &_gcry_mac_type_spec_poly1305mac_aes,
#else
    NULL,
#endif
#if USE_CAMELLIA
    &_gcry_mac_type_spec_poly1305mac_camellia,
#else
    NULL,
#endif
#if USE_TWOFISH
    &_gcry_mac_type_spec_poly1305mac_twofish,
#else
    NULL,
#endif
#if USE_SERPENT
    &_gcry_mac_type_spec_poly1305mac_serpent,
#else
    NULL,
#endif
#if USE_SEED
    &_gcry_mac_type_spec_poly1305mac_seed
#else
    NULL
#endif
  };




/* Explicitly initialize this module.  */
gcry_err_code_t
_gcry_mac_init (void)
{
  return 0;
}


/* Return the spec structure for the MAC algorithm ALGO.  For an
   unknown algorithm NULL is returned.  */
static const gcry_mac_spec_t *
spec_from_algo (int algo)
{
  const gcry_mac_spec_t *spec = NULL;

  if (algo >= 101 && algo < 101 + DIM(mac_list_algo101))
    spec = mac_list_algo101[algo - 101];
  else if (algo >= 201 && algo < 201 + DIM(mac_list_algo201))
    spec = mac_list_algo201[algo - 201];
  else if (algo >= 401 && algo < 401 + DIM(mac_list_algo401))
    spec = mac_list_algo401[algo - 401];
  else if (algo >= 501 && algo < 501 + DIM(mac_list_algo501))
    spec = mac_list_algo501[algo - 501];
#if USE_GOST28147
  else if (algo == GCRY_MAC_GOST28147_IMIT)
    spec = &_gcry_mac_type_spec_gost28147_imit;
#endif

  if (spec)
    gcry_assert (spec->algo == algo);

  return spec;
}


/* Lookup a mac's spec by its name.  */
static const gcry_mac_spec_t *
spec_from_name (const char *name)
{
  const gcry_mac_spec_t *spec;
  int idx;

  for (idx = 0; (spec = mac_list[idx]); idx++)
    if (!stricmp (name, spec->name))
      return spec;

  return NULL;
}


/****************
 * Map a string to the mac algo
 */
int
_gcry_mac_map_name (const char *string)
{
  const gcry_mac_spec_t *spec;

  if (!string)
    return 0;

  /* Not found, search a matching mac name.  */
  spec = spec_from_name (string);
  if (spec)
    return spec->algo;

  return 0;
}


/****************
 * This function simply returns the name of the algorithm or some constant
 * string when there is no algo.  It will never return NULL.
 * Use the macro gcry_mac_test_algo() to check whether the algorithm
 * is valid.
 */
const char *
_gcry_mac_algo_name (int algorithm)
{
  const gcry_mac_spec_t *spec;

  spec = spec_from_algo (algorithm);
  return spec ? spec->name : "?";
}


static gcry_err_code_t
check_mac_algo (int algorithm)
{
  const gcry_mac_spec_t *spec;

  spec = spec_from_algo (algorithm);
  if (spec && !spec->flags.disabled && (spec->flags.fips || !fips_mode ()))
    return 0;

  return GPG_ERR_MAC_ALGO;
}


/****************
 * Open a message digest handle for use with algorithm ALGO.
 */
static gcry_err_code_t
mac_open (gcry_mac_hd_t * hd, int algo, int secure, gcry_ctx_t ctx)
{
  const gcry_mac_spec_t *spec;
  gcry_err_code_t err;
  gcry_mac_hd_t h;

  spec = spec_from_algo (algo);
  if (!spec)
    return GPG_ERR_MAC_ALGO;
  else if (spec->flags.disabled)
    return GPG_ERR_MAC_ALGO;
  else if (!spec->flags.fips && fips_mode ())
    return GPG_ERR_MAC_ALGO;
  else if (!spec->ops)
    return GPG_ERR_MAC_ALGO;
  else if (!spec->ops->open || !spec->ops->write || !spec->ops->setkey ||
           !spec->ops->read || !spec->ops->verify || !spec->ops->reset)
    return GPG_ERR_MAC_ALGO;

  if (secure)
    h = xtrycalloc_secure (1, sizeof (*h));
  else
    h = xtrycalloc (1, sizeof (*h));

  if (!h)
    return gpg_err_code_from_syserror ();

  h->magic = secure ? CTX_MAC_MAGIC_SECURE : CTX_MAC_MAGIC_NORMAL;
  h->spec = spec;
  h->algo = algo;
  h->gcry_ctx = ctx;

  err = h->spec->ops->open (h);
  if (err)
    xfree (h);
  else
    *hd = h;

  return err;
}


static gcry_err_code_t
mac_reset (gcry_mac_hd_t hd)
{
  if (hd->spec->ops->reset)
    return hd->spec->ops->reset (hd);

  return 0;
}


static void
mac_close (gcry_mac_hd_t hd)
{
  if (hd->spec->ops->close)
    hd->spec->ops->close (hd);

  wipememory (hd, sizeof (*hd));

  xfree (hd);
}


static gcry_err_code_t
mac_setkey (gcry_mac_hd_t hd, const void *key, size_t keylen)
{
  if (!hd->spec->ops->setkey)
    return GPG_ERR_INV_ARG;
  if (keylen > 0 && !key)
    return GPG_ERR_INV_ARG;

  return hd->spec->ops->setkey (hd, key, keylen);
}


static gcry_err_code_t
mac_setiv (gcry_mac_hd_t hd, const void *iv, size_t ivlen)
{
  if (!hd->spec->ops->setiv)
    return GPG_ERR_INV_ARG;
  if (ivlen > 0 && !iv)
    return GPG_ERR_INV_ARG;

  return hd->spec->ops->setiv (hd, iv, ivlen);
}


static gcry_err_code_t
mac_write (gcry_mac_hd_t hd, const void *inbuf, size_t inlen)
{
  if (!hd->spec->ops->write)
    return GPG_ERR_INV_ARG;
  if (inlen > 0 && !inbuf)
    return GPG_ERR_INV_ARG;

  return hd->spec->ops->write (hd, inbuf, inlen);
}


static gcry_err_code_t
mac_read (gcry_mac_hd_t hd, void *outbuf, size_t * outlen)
{
  if (!outbuf || !outlen || *outlen == 0 || !hd->spec->ops->read)
    return GPG_ERR_INV_ARG;

  return hd->spec->ops->read (hd, outbuf, outlen);
}


static gcry_err_code_t
mac_verify (gcry_mac_hd_t hd, const void *buf, size_t buflen)
{
  if (!buf || buflen == 0 || !hd->spec->ops->verify)
    return GPG_ERR_INV_ARG;

  return hd->spec->ops->verify (hd, buf, buflen);
}


/* Create a MAC object for algorithm ALGO.  FLAGS may be
   given as an bitwise OR of the gcry_mac_flags values.
   H is guaranteed to be a valid handle or NULL on error.  */
gpg_err_code_t
_gcry_mac_open (gcry_mac_hd_t * h, int algo, unsigned int flags,
                gcry_ctx_t ctx)
{
  gcry_err_code_t rc;
  gcry_mac_hd_t hd = NULL;

  if ((flags & ~GCRY_MAC_FLAG_SECURE))
    rc = GPG_ERR_INV_ARG;
  else
    rc = mac_open (&hd, algo, !!(flags & GCRY_MAC_FLAG_SECURE), ctx);

  *h = rc ? NULL : hd;
  return rc;
}


void
_gcry_mac_close (gcry_mac_hd_t hd)
{
  if (hd)
    mac_close (hd);
}


gcry_err_code_t
_gcry_mac_setkey (gcry_mac_hd_t hd, const void *key, size_t keylen)
{
  return mac_setkey (hd, key, keylen);
}


gcry_err_code_t
_gcry_mac_setiv (gcry_mac_hd_t hd, const void *iv, size_t ivlen)
{
  return mac_setiv (hd, iv, ivlen);
}


gcry_err_code_t
_gcry_mac_write (gcry_mac_hd_t hd, const void *inbuf, size_t inlen)
{
  return mac_write (hd, inbuf, inlen);
}


gcry_err_code_t
_gcry_mac_read (gcry_mac_hd_t hd, void *outbuf, size_t * outlen)
{
  return mac_read (hd, outbuf, outlen);
}


gcry_err_code_t
_gcry_mac_verify (gcry_mac_hd_t hd, const void *buf, size_t buflen)
{
  return mac_verify (hd, buf, buflen);
}


int
_gcry_mac_get_algo (gcry_mac_hd_t hd)
{
  return hd->algo;
}


unsigned int
_gcry_mac_get_algo_maclen (int algo)
{
  const gcry_mac_spec_t *spec;

  spec = spec_from_algo (algo);
  if (!spec || !spec->ops || !spec->ops->get_maclen)
    return 0;

  return spec->ops->get_maclen (algo);
}


unsigned int
_gcry_mac_get_algo_keylen (int algo)
{
  const gcry_mac_spec_t *spec;

  spec = spec_from_algo (algo);
  if (!spec || !spec->ops || !spec->ops->get_keylen)
    return 0;

  return spec->ops->get_keylen (algo);
}


gcry_err_code_t
_gcry_mac_ctl (gcry_mac_hd_t hd, int cmd, void *buffer, size_t buflen)
{
  gcry_err_code_t rc;

  /* Currently not used.  */
  (void) hd;
  (void) buffer;
  (void) buflen;

  switch (cmd)
    {
    case GCRYCTL_RESET:
      rc = mac_reset (hd);
      break;
    case GCRYCTL_SET_SBOX:
      if (hd->spec->ops->set_extra_info)
        rc = hd->spec->ops->set_extra_info
          (hd, GCRYCTL_SET_SBOX, buffer, buflen);
      else
        rc = GPG_ERR_NOT_SUPPORTED;
      break;
    default:
      rc = GPG_ERR_INV_OP;
    }
  return rc;
}


/* Return information about the given MAC algorithm ALGO.

    GCRYCTL_TEST_ALGO:
        Returns 0 if the specified algorithm ALGO is available for use.
        BUFFER and NBYTES must be zero.

   Note: Because this function is in most cases used to return an
   integer value, we can make it easier for the caller to just look at
   the return value.  The caller will in all cases consult the value
   and thereby detecting whether a error occurred or not (i.e. while
   checking the block size)
 */
gcry_err_code_t
_gcry_mac_algo_info (int algo, int what, void *buffer, size_t * nbytes)
{
  gcry_err_code_t rc = 0;
  unsigned int ui;

  switch (what)
    {
    case GCRYCTL_GET_KEYLEN:
      if (buffer || (!nbytes))
        rc = GPG_ERR_INV_ARG;
      else
        {
          ui = _gcry_mac_get_algo_keylen (algo);
          if (ui > 0)
            *nbytes = (size_t) ui;
          else
            /* The only reason for an error is an invalid algo.  */
            rc = GPG_ERR_MAC_ALGO;
        }
      break;
    case GCRYCTL_TEST_ALGO:
      if (buffer || nbytes)
        rc = GPG_ERR_INV_ARG;
      else
        rc = check_mac_algo (algo);
      break;

    default:
      rc = GPG_ERR_INV_OP;
    }

  return rc;
}


/* Run the self-tests for the MAC.  */
gpg_error_t
_gcry_mac_selftest (int algo, int extended, selftest_report_func_t report)
{
  gcry_err_code_t ec;
  const gcry_mac_spec_t *spec;

  spec = spec_from_algo (algo);
  if (spec && !spec->flags.disabled
      && (spec->flags.fips || !fips_mode ())
      && spec->ops && spec->ops->selftest)
    ec = spec->ops->selftest (algo, extended, report);
  else
    {
      ec = GPG_ERR_MAC_ALGO;
      if (report)
        report ("mac", algo, "module",
                spec && !spec->flags.disabled
                && (spec->flags.fips || !fips_mode ())?
                "no selftest available" :
                spec? "algorithm disabled" :
                "algorithm not found");
    }

  return gpg_error (ec);
}
