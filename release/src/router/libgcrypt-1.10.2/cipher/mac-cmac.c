/* mac-cmac.c  -  CMAC glue for MAC API
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 * Copyright (C) 2008 Free Software Foundation, Inc.
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
    case GCRY_MAC_CMAC_AES:
      return GCRY_CIPHER_AES;
    case GCRY_MAC_CMAC_3DES:
      return GCRY_CIPHER_3DES;
    case GCRY_MAC_CMAC_CAMELLIA:
      return GCRY_CIPHER_CAMELLIA128;
    case GCRY_MAC_CMAC_IDEA:
      return GCRY_CIPHER_IDEA;
    case GCRY_MAC_CMAC_CAST5:
      return GCRY_CIPHER_CAST5;
    case GCRY_MAC_CMAC_BLOWFISH:
      return GCRY_CIPHER_BLOWFISH;
    case GCRY_MAC_CMAC_TWOFISH:
      return GCRY_CIPHER_TWOFISH;
    case GCRY_MAC_CMAC_SERPENT:
      return GCRY_CIPHER_SERPENT128;
    case GCRY_MAC_CMAC_SEED:
      return GCRY_CIPHER_SEED;
    case GCRY_MAC_CMAC_RFC2268:
      return GCRY_CIPHER_RFC2268_128;
    case GCRY_MAC_CMAC_GOST28147:
      return GCRY_CIPHER_GOST28147;
    case GCRY_MAC_CMAC_SM4:
      return GCRY_CIPHER_SM4;
    }
}


static gcry_err_code_t
cmac_open (gcry_mac_hd_t h)
{
  gcry_err_code_t err;
  gcry_cipher_hd_t hd;
  int secure = (h->magic == CTX_MAC_MAGIC_SECURE);
  int cipher_algo;
  unsigned int flags;

  cipher_algo = map_mac_algo_to_cipher (h->spec->algo);
  flags = (secure ? GCRY_CIPHER_SECURE : 0);

  err = _gcry_cipher_open_internal (&hd, cipher_algo, GCRY_CIPHER_MODE_CMAC,
                                    flags);
  if (err)
    return err;

  h->u.cmac.cipher_algo = cipher_algo;
  h->u.cmac.ctx = hd;
  h->u.cmac.blklen = _gcry_cipher_get_algo_blklen (cipher_algo);
  return 0;
}


static void
cmac_close (gcry_mac_hd_t h)
{
  _gcry_cipher_close (h->u.cmac.ctx);
  h->u.cmac.ctx = NULL;
}


static gcry_err_code_t
cmac_setkey (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  return _gcry_cipher_setkey (h->u.cmac.ctx, key, keylen);
}


static gcry_err_code_t
cmac_reset (gcry_mac_hd_t h)
{
  return _gcry_cipher_reset (h->u.cmac.ctx);
}


static gcry_err_code_t
cmac_write (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  return _gcry_cipher_cmac_authenticate (h->u.cmac.ctx, buf, buflen);
}


static gcry_err_code_t
cmac_read (gcry_mac_hd_t h, unsigned char *outbuf, size_t * outlen)
{
  if (*outlen > h->u.cmac.blklen)
    *outlen = h->u.cmac.blklen;
  return _gcry_cipher_cmac_get_tag (h->u.cmac.ctx, outbuf, *outlen);
}


static gcry_err_code_t
cmac_verify (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  return _gcry_cipher_cmac_check_tag (h->u.cmac.ctx, buf, buflen);
}


static unsigned int
cmac_get_maclen (int algo)
{
  return _gcry_cipher_get_algo_blklen (map_mac_algo_to_cipher (algo));
}


static unsigned int
cmac_get_keylen (int algo)
{
  return _gcry_cipher_get_algo_keylen (map_mac_algo_to_cipher (algo));
}


/* Check one CMAC with MAC ALGO using the regular MAC
 * API. (DATA,DATALEN) is the data to be MACed, (KEY,KEYLEN) the key
 * and (EXPECT,EXPECTLEN) the expected result.  Returns NULL on
 * success or a string describing the failure.  */
static const char *
check_one (int algo, const char *data, size_t datalen,
           const char *key, size_t keylen,
           const char *expect, size_t expectlen)
{
  gcry_mac_hd_t hd;
  unsigned char mac[512]; /* hardcoded to avoid allocation */
  unsigned int maclen;
  size_t macoutlen;
  int i;
  gcry_error_t err = 0;

  err = _gcry_mac_open (&hd, algo, 0, NULL);
  if (err)
    return "gcry_mac_open failed";

  i = _gcry_mac_get_algo (hd);
  if (i != algo)
    return "gcry_mac_get_algo failed";

  maclen = _gcry_mac_get_algo_maclen (algo);
  if (maclen < 1 || maclen > 500)
    return "gcry_mac_get_algo_maclen failed";

  if (maclen != expectlen)
    return "invalid tests data";

  err = _gcry_mac_setkey (hd, key, keylen);
  if (err)
    {
      _gcry_mac_close (hd);
      return "gcry_mac_setkey failed";
    }

  err = _gcry_mac_write (hd, data, datalen);
  if (err)
    {
      _gcry_mac_close (hd);
      return "gcry_mac_write failed";
    }

  err = _gcry_mac_verify (hd, expect, maclen);
  if (err)
    {
      _gcry_mac_close (hd);
      return "gcry_mac_verify failed";
    }

  macoutlen = maclen;
  err = _gcry_mac_read (hd, mac, &macoutlen);
  _gcry_mac_close (hd);
  if (err)
    return "gcry_mac_read failed";

  if (memcmp (mac, expect, maclen))
    return "does not match";

  return NULL;
}


/*
 * CMAC AES and DES test vectors are from
 * http://web.archive.org/web/20130930212819/http://csrc.nist.gov/publica \
 * tions/nistpubs/800-38B/Updated_CMAC_Examples.pdf
 */

static gpg_err_code_t
selftests_cmac_3des (int extended, selftest_report_func_t report)
{
  static const struct
  {
    const char *desc;
    const char *data;
    const char *key;
    const char *expect;
  } tv[] =
    {
      { "Basic 3DES",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57",
        "\x8a\xa8\x3b\xf8\xcb\xda\x10\x62\x0b\xc1\xbf\x19\xfb\xb6\xcd\x58"
        "\xbc\x31\x3d\x4a\x37\x1c\xa8\xb5",
        "\x74\x3d\xdb\xe0\xce\x2d\xc2\xed" },
      { "Extended 3DES #1",
        "",
        "\x8a\xa8\x3b\xf8\xcb\xda\x10\x62\x0b\xc1\xbf\x19\xfb\xb6\xcd\x58"
        "\xbc\x31\x3d\x4a\x37\x1c\xa8\xb5",
        "\xb7\xa6\x88\xe1\x22\xff\xaf\x95" },
      { "Extended 3DES #2",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96",
        "\x8a\xa8\x3b\xf8\xcb\xda\x10\x62\x0b\xc1\xbf\x19\xfb\xb6\xcd\x58"
        "\xbc\x31\x3d\x4a\x37\x1c\xa8\xb5",
        "\x8e\x8f\x29\x31\x36\x28\x37\x97" },
      { "Extended 3DES #3",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51",
        "\x8a\xa8\x3b\xf8\xcb\xda\x10\x62\x0b\xc1\xbf\x19\xfb\xb6\xcd\x58"
        "\xbc\x31\x3d\x4a\x37\x1c\xa8\xb5",
        "\x33\xe6\xb1\x09\x24\x00\xea\xe5" },
      { "Extended 3DES #4",
        "",
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5\x8a\x3d\x10\xba\x80\x57\x0d\x38"
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5",
        "\xbd\x2e\xbf\x9a\x3b\xa0\x03\x61" },
      { "Extended 3DES #5",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96",
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5\x8a\x3d\x10\xba\x80\x57\x0d\x38"
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5",
        "\x4f\xf2\xab\x81\x3c\x53\xce\x83" },
      { "Extended 3DES #6",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57",
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5\x8a\x3d\x10\xba\x80\x57\x0d\x38"
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5",
        "\x62\xdd\x1b\x47\x19\x02\xbd\x4e" },
      { "Extended 3DES #7",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51",
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5\x8a\x3d\x10\xba\x80\x57\x0d\x38"
        "\x4c\xf1\x51\x34\xa2\x85\x0d\xd5",
        "\x31\xb1\xe4\x31\xda\xbc\x4e\xb8" },
      { NULL }
    };
  const char *what;
  const char *errtxt;
  int tvidx;

  for (tvidx=0; tv[tvidx].desc; tvidx++)
    {
      what = tv[tvidx].desc;
      errtxt = check_one (GCRY_MAC_CMAC_3DES,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          tv[tvidx].expect, 8);
      if (errtxt)
        goto failed;
      if (!extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("cmac", GCRY_MAC_CMAC_3DES, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}



static gpg_err_code_t
selftests_cmac_aes (int extended, selftest_report_func_t report)
{
  static const struct
  {
    const char *desc;
    const char *data;
    const char *key;
    const char *expect;
  } tv[] =
    {
      { "Basic AES128",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
        "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11",
        "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
        "\xdf\xa6\x67\x47\xde\x9a\xe6\x30\x30\xca\x32\x61\x14\x97\xc8\x27" },
      { "Basic AES192",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
        "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11",
        "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5"
        "\x62\xf8\xea\xd2\x52\x2c\x6b\x7b",
        "\x8a\x1d\xe5\xbe\x2e\xb3\x1a\xad\x08\x9a\x82\xe6\xee\x90\x8b\x0e" },
      { "Basic AES256",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
        "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11",
        "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81"
        "\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4",
        "\xaa\xf3\xd8\xf1\xde\x56\x40\xc2\x32\xf5\xb1\x69\xb9\xc9\x11\xe6" },
      { "Extended AES #1",
        "",
        "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
        "\xbb\x1d\x69\x29\xe9\x59\x37\x28\x7f\xa3\x7d\x12\x9b\x75\x67\x46" },
      { "Extended AES #2",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
        "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5"
        "\x62\xf8\xea\xd2\x52\x2c\x6b\x7b",
        "\x9e\x99\xa7\xbf\x31\xe7\x10\x90\x06\x62\xf6\x5e\x61\x7c\x51\x84" },
      { "Extended AES #3",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
        "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef"
        "\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10",
        "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81"
        "\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4",
        "\xe1\x99\x21\x90\x54\x9f\x6e\xd5\x69\x6a\x2c\x05\x6c\x31\x54\x10" },
      { "Extended AES #4",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
        "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
        "\x07\x0a\x16\xb4\x6b\x4d\x41\x44\xf7\x9b\xdd\x9d\xd0\x4a\x28\x7c" },
      { "Extended AES #5",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
        "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef"
        "\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10",
        "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
        "\x51\xf0\xbe\xbf\x7e\x3b\x9d\x92\xfc\x49\x74\x17\x79\x36\x3c\xfe" },
      { "Extended AES #6",
        "",
        "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5"
        "\x62\xf8\xea\xd2\x52\x2c\x6b\x7b",
        "\xd1\x7d\xdf\x46\xad\xaa\xcd\xe5\x31\xca\xc4\x83\xde\x7a\x93\x67" },
      { "Extended AES #7",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
        "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
        "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef"
        "\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10",
        "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5"
        "\x62\xf8\xea\xd2\x52\x2c\x6b\x7b",
        "\xa1\xd5\xdf\x0e\xed\x79\x0f\x79\x4d\x77\x58\x96\x59\xf3\x9a\x11" },
      { "Extended AES #8",
        "",
        "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81"
        "\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4",
        "\x02\x89\x62\xf6\x1b\x7b\xf8\x9e\xfc\x6b\x55\x1f\x46\x67\xd9\x83" },
      { "Extended AES #9",
        "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
        "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81"
        "\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4",
        "\x28\xa7\x02\x3f\x45\x2e\x8f\x82\xbd\x4b\xf2\x8d\x8c\x37\xc3\x5c" },
      { NULL }
    };
  const char *what;
  const char *errtxt;
  int tvidx;

  for (tvidx=0; tv[tvidx].desc; tvidx++)
    {
      what = tv[tvidx].desc;
      errtxt = check_one (GCRY_MAC_CMAC_AES,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          tv[tvidx].expect, strlen (tv[tvidx].expect));
      if (errtxt)
        goto failed;
      if (tvidx >= 2 && !extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("cmac", GCRY_MAC_CMAC_AES, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}

static gpg_err_code_t
cmac_selftest (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MAC_CMAC_3DES:
      ec = selftests_cmac_3des (extended, report);
      break;
    case GCRY_MAC_CMAC_AES:
      ec = selftests_cmac_aes (extended, report);
      break;

    default:
      ec = GPG_ERR_MAC_ALGO;
      break;
    }

  return ec;
}


static gcry_mac_spec_ops_t cmac_ops = {
  cmac_open,
  cmac_close,
  cmac_setkey,
  NULL,
  cmac_reset,
  cmac_write,
  cmac_read,
  cmac_verify,
  cmac_get_maclen,
  cmac_get_keylen,
  NULL,
  cmac_selftest
};


#if USE_BLOWFISH
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_blowfish = {
  GCRY_MAC_CMAC_BLOWFISH, {0, 0}, "CMAC_BLOWFISH",
  &cmac_ops
};
#endif
#if USE_DES
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_tripledes = {
  GCRY_MAC_CMAC_3DES, {0, 0}, "CMAC_3DES",
  &cmac_ops
};
#endif
#if USE_CAST5
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_cast5 = {
  GCRY_MAC_CMAC_CAST5, {0, 0}, "CMAC_CAST5",
  &cmac_ops
};
#endif
#if USE_AES
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_aes = {
  GCRY_MAC_CMAC_AES, {0, 1}, "CMAC_AES",
  &cmac_ops
};
#endif
#if USE_TWOFISH
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_twofish = {
  GCRY_MAC_CMAC_TWOFISH, {0, 0}, "CMAC_TWOFISH",
  &cmac_ops
};
#endif
#if USE_SERPENT
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_serpent = {
  GCRY_MAC_CMAC_SERPENT, {0, 0}, "CMAC_SERPENT",
  &cmac_ops
};
#endif
#if USE_RFC2268
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_rfc2268 = {
  GCRY_MAC_CMAC_RFC2268, {0, 0}, "CMAC_RFC2268",
  &cmac_ops
};
#endif
#if USE_SEED
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_seed = {
  GCRY_MAC_CMAC_SEED, {0, 0}, "CMAC_SEED",
  &cmac_ops
};
#endif
#if USE_CAMELLIA
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_camellia = {
  GCRY_MAC_CMAC_CAMELLIA, {0, 0}, "CMAC_CAMELLIA",
  &cmac_ops
};
#endif
#if USE_IDEA
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_idea = {
  GCRY_MAC_CMAC_IDEA, {0, 0}, "CMAC_IDEA",
  &cmac_ops
};
#endif
#if USE_GOST28147
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_gost28147 = {
  GCRY_MAC_CMAC_GOST28147, {0, 0}, "CMAC_GOST28147",
  &cmac_ops
};
#endif
#if USE_SM4
const gcry_mac_spec_t _gcry_mac_type_spec_cmac_sm4 = {
  GCRY_MAC_CMAC_SM4, {0, 0}, "CMAC_SM4",
  &cmac_ops
};
#endif
