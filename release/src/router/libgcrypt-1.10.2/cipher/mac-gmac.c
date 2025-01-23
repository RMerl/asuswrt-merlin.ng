/* mac-gmac.c  -  GMAC glue for MAC API
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
#include "cipher.h"
#include "./mac-internal.h"


static int
map_mac_algo_to_cipher (int mac_algo)
{
  switch (mac_algo)
    {
    default:
      return GCRY_CIPHER_NONE;
    case GCRY_MAC_GMAC_AES:
      return GCRY_CIPHER_AES;
    case GCRY_MAC_GMAC_CAMELLIA:
      return GCRY_CIPHER_CAMELLIA128;
    case GCRY_MAC_GMAC_TWOFISH:
      return GCRY_CIPHER_TWOFISH;
    case GCRY_MAC_GMAC_SERPENT:
      return GCRY_CIPHER_SERPENT128;
    case GCRY_MAC_GMAC_SEED:
      return GCRY_CIPHER_SEED;
    }
}


static gcry_err_code_t
gmac_open (gcry_mac_hd_t h)
{
  gcry_err_code_t err;
  gcry_cipher_hd_t hd;
  int secure = (h->magic == CTX_MAC_MAGIC_SECURE);
  int cipher_algo;
  unsigned int flags;

  cipher_algo = map_mac_algo_to_cipher (h->spec->algo);
  flags = (secure ? GCRY_CIPHER_SECURE : 0);

  err = _gcry_cipher_open_internal (&hd, cipher_algo, GCRY_CIPHER_MODE_GCM,
                                    flags);
  if (err)
    return err;

  h->u.gmac.cipher_algo = cipher_algo;
  h->u.gmac.ctx = hd;
  return 0;
}


static void
gmac_close (gcry_mac_hd_t h)
{
  _gcry_cipher_close (h->u.gmac.ctx);
  h->u.gmac.ctx = NULL;
}


static gcry_err_code_t
gmac_setkey (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  return _gcry_cipher_setkey (h->u.gmac.ctx, key, keylen);
}


static gcry_err_code_t
gmac_setiv (gcry_mac_hd_t h, const unsigned char *iv, size_t ivlen)
{
  return _gcry_cipher_setiv (h->u.gmac.ctx, iv, ivlen);
}


static gcry_err_code_t
gmac_reset (gcry_mac_hd_t h)
{
  return _gcry_cipher_reset (h->u.gmac.ctx);
}


static gcry_err_code_t
gmac_write (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  return _gcry_cipher_authenticate (h->u.gmac.ctx, buf, buflen);
}


static gcry_err_code_t
gmac_read (gcry_mac_hd_t h, unsigned char *outbuf, size_t * outlen)
{
  if (*outlen > GCRY_GCM_BLOCK_LEN)
    *outlen = GCRY_GCM_BLOCK_LEN;
  return _gcry_cipher_gettag (h->u.gmac.ctx, outbuf, *outlen);
}


static gcry_err_code_t
gmac_verify (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  return _gcry_cipher_checktag (h->u.gmac.ctx, buf, buflen);
}


static unsigned int
gmac_get_maclen (int algo)
{
  (void)algo;
  return GCRY_GCM_BLOCK_LEN;
}


static unsigned int
gmac_get_keylen (int algo)
{
  return _gcry_cipher_get_algo_keylen (map_mac_algo_to_cipher (algo));
}


static gcry_mac_spec_ops_t gmac_ops = {
  gmac_open,
  gmac_close,
  gmac_setkey,
  gmac_setiv,
  gmac_reset,
  gmac_write,
  gmac_read,
  gmac_verify,
  gmac_get_maclen,
  gmac_get_keylen,
  NULL,
  NULL
};


#if USE_AES
const gcry_mac_spec_t _gcry_mac_type_spec_gmac_aes = {
  GCRY_MAC_GMAC_AES, {0, 0}, "GMAC_AES",
  &gmac_ops
};
#endif
#if USE_TWOFISH
const gcry_mac_spec_t _gcry_mac_type_spec_gmac_twofish = {
  GCRY_MAC_GMAC_TWOFISH, {0, 0}, "GMAC_TWOFISH",
  &gmac_ops
};
#endif
#if USE_SERPENT
const gcry_mac_spec_t _gcry_mac_type_spec_gmac_serpent = {
  GCRY_MAC_GMAC_SERPENT, {0, 0}, "GMAC_SERPENT",
  &gmac_ops
};
#endif
#if USE_SEED
const gcry_mac_spec_t _gcry_mac_type_spec_gmac_seed = {
  GCRY_MAC_GMAC_SEED, {0, 0}, "GMAC_SEED",
  &gmac_ops
};
#endif
#if USE_CAMELLIA
const gcry_mac_spec_t _gcry_mac_type_spec_gmac_camellia = {
  GCRY_MAC_GMAC_CAMELLIA, {0, 0}, "GMAC_CAMELLIA",
  &gmac_ops
};
#endif
