/* mac-hmac.c  -  HMAC glue for MAC API
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
#include "./mac-internal.h"
#include "bufhelp.h"
#include "cipher.h"


static int
map_mac_algo_to_md (int mac_algo)
{
  switch (mac_algo)
    {
    default:
      return GCRY_MD_NONE;
    case GCRY_MAC_HMAC_MD2:
      return GCRY_MD_MD2;
    case GCRY_MAC_HMAC_MD4:
      return GCRY_MD_MD4;
    case GCRY_MAC_HMAC_MD5:
      return GCRY_MD_MD5;
    case GCRY_MAC_HMAC_SHA1:
      return GCRY_MD_SHA1;
    case GCRY_MAC_HMAC_SHA224:
      return GCRY_MD_SHA224;
    case GCRY_MAC_HMAC_SHA256:
      return GCRY_MD_SHA256;
    case GCRY_MAC_HMAC_SHA384:
      return GCRY_MD_SHA384;
    case GCRY_MAC_HMAC_SHA512:
      return GCRY_MD_SHA512;
    case GCRY_MAC_HMAC_SHA512_256:
      return GCRY_MD_SHA512_256;
    case GCRY_MAC_HMAC_SHA512_224:
      return GCRY_MD_SHA512_224;
    case GCRY_MAC_HMAC_SHA3_224:
      return GCRY_MD_SHA3_224;
    case GCRY_MAC_HMAC_SHA3_256:
      return GCRY_MD_SHA3_256;
    case GCRY_MAC_HMAC_SHA3_384:
      return GCRY_MD_SHA3_384;
    case GCRY_MAC_HMAC_SHA3_512:
      return GCRY_MD_SHA3_512;
    case GCRY_MAC_HMAC_RMD160:
      return GCRY_MD_RMD160;
    case GCRY_MAC_HMAC_TIGER1:
      return GCRY_MD_TIGER1;
    case GCRY_MAC_HMAC_WHIRLPOOL:
      return GCRY_MD_WHIRLPOOL;
    case GCRY_MAC_HMAC_GOSTR3411_94:
      return GCRY_MD_GOSTR3411_94;
    case GCRY_MAC_HMAC_GOSTR3411_CP:
      return GCRY_MD_GOSTR3411_CP;
    case GCRY_MAC_HMAC_STRIBOG256:
      return GCRY_MD_STRIBOG256;
    case GCRY_MAC_HMAC_STRIBOG512:
      return GCRY_MD_STRIBOG512;
    case GCRY_MAC_HMAC_BLAKE2B_512:
      return GCRY_MD_BLAKE2B_512;
    case GCRY_MAC_HMAC_BLAKE2B_384:
      return GCRY_MD_BLAKE2B_384;
    case GCRY_MAC_HMAC_BLAKE2B_256:
      return GCRY_MD_BLAKE2B_256;
    case GCRY_MAC_HMAC_BLAKE2B_160:
      return GCRY_MD_BLAKE2B_160;
    case GCRY_MAC_HMAC_BLAKE2S_256:
      return GCRY_MD_BLAKE2S_256;
    case GCRY_MAC_HMAC_BLAKE2S_224:
      return GCRY_MD_BLAKE2S_224;
    case GCRY_MAC_HMAC_BLAKE2S_160:
      return GCRY_MD_BLAKE2S_160;
    case GCRY_MAC_HMAC_BLAKE2S_128:
      return GCRY_MD_BLAKE2S_128;
    case GCRY_MAC_HMAC_SM3:
      return GCRY_MD_SM3;
    }
}


static gcry_err_code_t
hmac_open (gcry_mac_hd_t h)
{
  gcry_err_code_t err;
  gcry_md_hd_t hd;
  int secure = (h->magic == CTX_MAC_MAGIC_SECURE);
  unsigned int flags;
  int md_algo;

  md_algo = map_mac_algo_to_md (h->spec->algo);

  flags = GCRY_MD_FLAG_HMAC;
  flags |= (secure ? GCRY_MD_FLAG_SECURE : 0);

  err = _gcry_md_open (&hd, md_algo, flags);
  if (err)
    return err;

  h->u.hmac.md_algo = md_algo;
  h->u.hmac.md_ctx = hd;
  return 0;
}


static void
hmac_close (gcry_mac_hd_t h)
{
  _gcry_md_close (h->u.hmac.md_ctx);
  h->u.hmac.md_ctx = NULL;
}


static gcry_err_code_t
hmac_setkey (gcry_mac_hd_t h, const unsigned char *key, size_t keylen)
{
  return _gcry_md_setkey (h->u.hmac.md_ctx, key, keylen);
}


static gcry_err_code_t
hmac_reset (gcry_mac_hd_t h)
{
  _gcry_md_reset (h->u.hmac.md_ctx);
  return 0;
}


static gcry_err_code_t
hmac_write (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  _gcry_md_write (h->u.hmac.md_ctx, buf, buflen);
  return 0;
}


static gcry_err_code_t
hmac_read (gcry_mac_hd_t h, unsigned char *outbuf, size_t * outlen)
{
  unsigned int dlen;
  const unsigned char *digest;

  dlen = _gcry_md_get_algo_dlen (h->u.hmac.md_algo);
  digest = _gcry_md_read (h->u.hmac.md_ctx, h->u.hmac.md_algo);

  if (*outlen <= dlen)
    buf_cpy (outbuf, digest, *outlen);
  else
    {
      buf_cpy (outbuf, digest, dlen);
      *outlen = dlen;
    }

  return 0;
}


static gcry_err_code_t
hmac_verify (gcry_mac_hd_t h, const unsigned char *buf, size_t buflen)
{
  unsigned int dlen;
  const unsigned char *digest;

  dlen = _gcry_md_get_algo_dlen (h->u.hmac.md_algo);
  digest = _gcry_md_read (h->u.hmac.md_ctx, h->u.hmac.md_algo);

  if (buflen > dlen)
    return GPG_ERR_INV_LENGTH;

  return buf_eq_const (buf, digest, buflen) ? 0 : GPG_ERR_CHECKSUM;
}


static unsigned int
hmac_get_maclen (int algo)
{
  return _gcry_md_get_algo_dlen (map_mac_algo_to_md (algo));
}


static unsigned int
hmac_get_keylen (int algo)
{
  /* Return blocksize for default key length. */
  switch (algo)
    {
    case GCRY_MD_SHA3_224:
      return 1152 / 8;
    case GCRY_MD_SHA3_256:
      return 1088 / 8;
    case GCRY_MD_SHA3_384:
      return 832 / 8;
    case GCRY_MD_SHA3_512:
      return 576 / 8;
    case GCRY_MAC_HMAC_SHA384:
    case GCRY_MAC_HMAC_SHA512:
      return 128;
    case GCRY_MAC_HMAC_GOSTR3411_94:
      return 32;
    default:
      return 64;
    }
}


/* Check one HMAC with digest ALGO using the regualr HAMC
 * API. (DATA,DATALEN) is the data to be MACed, (KEY,KEYLEN) the key
 * and (EXPECT,EXPECTLEN) the expected result.  If TRUNC is set, the
 * EXPECTLEN may be less than the digest length.  Returns NULL on
 * success or a string describing the failure.  */
static const char *
check_one (int algo,
           const void *data, size_t datalen,
           const void *key, size_t keylen,
           const void *expect, size_t expectlen, int trunc)
{
  gcry_md_hd_t hd;
  const unsigned char *digest;

/*   printf ("HMAC algo %d\n", algo); */

  /* Skip test with shoter key in FIPS mode.  */
  if (fips_mode () && keylen < 14)
    return NULL;

  if (trunc)
    {
      if (_gcry_md_get_algo_dlen (algo) < expectlen)
        return "invalid tests data";
    }
  else
    {
      if (_gcry_md_get_algo_dlen (algo) != expectlen)
        return "invalid tests data";
    }
  if (_gcry_md_open (&hd, algo, GCRY_MD_FLAG_HMAC))
    return "gcry_md_open failed";
  if (_gcry_md_setkey (hd, key, keylen))
    {
      _gcry_md_close (hd);
      return "gcry_md_setkey failed";
    }
  _gcry_md_write (hd, data, datalen);
  digest = _gcry_md_read (hd, algo);
  if (!digest)
    {
      _gcry_md_close (hd);
      return "gcry_md_read failed";
    }
  if (memcmp (digest, expect, expectlen))
    {
/*       int i; */

/*       fputs ("        {", stdout); */
/*       for (i=0; i < expectlen-1; i++) */
/*         { */
/*           if (i && !(i % 8)) */
/*             fputs ("\n         ", stdout); */
/*           printf (" 0x%02x,", digest[i]); */
/*         } */
/*       printf (" 0x%02x } },\n", digest[i]); */

      _gcry_md_close (hd);
      return "does not match";
    }
  _gcry_md_close (hd);
  return NULL;
}


static gpg_err_code_t
selftests_sha1 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;
  unsigned char key[128];
  int i, j;

  what = "FIPS-198a, A.1";
  for (i=0; i < 64; i++)
    key[i] = i;
  errtxt = check_one (GCRY_MD_SHA1,
                      "Sample #1", 9,
                      key, 64,
                      "\x4f\x4c\xa3\xd5\xd6\x8b\xa7\xcc\x0a\x12"
                      "\x08\xc9\xc6\x1e\x9c\x5d\xa0\x40\x3c\x0a", 20, 0);
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "FIPS-198a, A.2";
      for (i=0, j=0x30; i < 20; i++)
        key[i] = j++;
      errtxt = check_one (GCRY_MD_SHA1,
                          "Sample #2", 9,
                          key, 20,
                          "\x09\x22\xd3\x40\x5f\xaa\x3d\x19\x4f\x82"
                          "\xa4\x58\x30\x73\x7d\x5c\xc6\xc7\x5d\x24", 20, 0);
      if (errtxt)
        goto failed;

      what = "FIPS-198a, A.3";
      for (i=0, j=0x50; i < 100; i++)
        key[i] = j++;
      errtxt = check_one (GCRY_MD_SHA1,
                          "Sample #3", 9,
                          key, 100,
                          "\xbc\xf4\x1e\xab\x8b\xb2\xd8\x02\xf3\xd0"
                          "\x5c\xaf\x7c\xb0\x92\xec\xf8\xd1\xa3\xaa", 20, 0);
      if (errtxt)
        goto failed;

      what = "FIPS-198a, A.4";
      for (i=0, j=0x70; i < 49; i++)
        key[i] = j++;
      errtxt = check_one (GCRY_MD_SHA1,
                          "Sample #4", 9,
                          key, 49,
                          "\x9e\xa8\x86\xef\xe2\x68\xdb\xec\xce\x42"
                          "\x0c\x75\x24\xdf\x32\xe0\x75\x1a\x2a\x26", 20, 0);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("hmac", GCRY_MD_SHA1, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}



static gpg_err_code_t
selftests_sha224 (int extended, selftest_report_func_t report)
{
  static struct
  {
    const char * const desc;
    const char * const data;
    const char * const key;
    const char expect[28];
  } tv[] =
    {
      { "data-28 key-4",
        "what do ya want for nothing?",
        "Jefe",
        { 0xa3, 0x0e, 0x01, 0x09, 0x8b, 0xc6, 0xdb, 0xbf,
          0x45, 0x69, 0x0f, 0x3a, 0x7e, 0x9e, 0x6d, 0x0f,
          0x8b, 0xbe, 0xa2, 0xa3, 0x9e, 0x61, 0x48, 0x00,
          0x8f, 0xd0, 0x5e, 0x44 } },

      { "data-9 key-20",
        "Hi There",
	"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
        "\x0b\x0b\x0b\x0b",
        { 0x89, 0x6f, 0xb1, 0x12, 0x8a, 0xbb, 0xdf, 0x19,
          0x68, 0x32, 0x10, 0x7c, 0xd4, 0x9d, 0xf3, 0x3f,
          0x47, 0xb4, 0xb1, 0x16, 0x99, 0x12, 0xba, 0x4f,
          0x53, 0x68, 0x4b, 0x22 } },

      { "data-50 key-20",
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa",
        { 0x7f, 0xb3, 0xcb, 0x35, 0x88, 0xc6, 0xc1, 0xf6,
          0xff, 0xa9, 0x69, 0x4d, 0x7d, 0x6a, 0xd2, 0x64,
          0x93, 0x65, 0xb0, 0xc1, 0xf6, 0x5d, 0x69, 0xd1,
          0xec, 0x83, 0x33, 0xea } },

      { "data-50 key-26",
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd",
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
        "\x11\x12\x13\x14\x15\x16\x17\x18\x19",
        { 0x6c, 0x11, 0x50, 0x68, 0x74, 0x01, 0x3c, 0xac,
          0x6a, 0x2a, 0xbc, 0x1b, 0xb3, 0x82, 0x62, 0x7c,
          0xec, 0x6a, 0x90, 0xd8, 0x6e, 0xfc, 0x01, 0x2d,
          0xe7, 0xaf, 0xec, 0x5a } },

      { "data-54 key-131",
        "Test Using Larger Than Block-Size Key - Hash Key First",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
        { 0x95, 0xe9, 0xa0, 0xdb, 0x96, 0x20, 0x95, 0xad,
          0xae, 0xbe, 0x9b, 0x2d, 0x6f, 0x0d, 0xbc, 0xe2,
          0xd4, 0x99, 0xf1, 0x12, 0xf2, 0xd2, 0xb7, 0x27,
          0x3f, 0xa6, 0x87, 0x0e } },

      { "data-152 key-131",
        "This is a test using a larger than block-size key and a larger "
        "than block-size data. The key needs to be hashed before being "
        "used by the HMAC algorithm.",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
        { 0x3a, 0x85, 0x41, 0x66, 0xac, 0x5d, 0x9f, 0x02,
          0x3f, 0x54, 0xd5, 0x17, 0xd0, 0xb3, 0x9d, 0xbd,
          0x94, 0x67, 0x70, 0xdb, 0x9c, 0x2b, 0x95, 0xc9,
          0xf6, 0xf5, 0x65, 0xd1 } },

      { NULL }
    };
  const char *what;
  const char *errtxt;
  int tvidx;

  for (tvidx=0; tv[tvidx].desc; tvidx++)
    {
      what = tv[tvidx].desc;
      errtxt = check_one (GCRY_MD_SHA224,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          tv[tvidx].expect, DIM (tv[tvidx].expect), 0);
      if (errtxt)
        goto failed;
      if (!extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("hmac", GCRY_MD_SHA224, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


static gpg_err_code_t
selftests_sha256 (int extended, selftest_report_func_t report)
{
  static struct
  {
    const char * const desc;
    const char * const data;
    const char * const key;
    const char expect[32];
  } tv[] =
    {
      { "data-28 key-4",
        "what do ya want for nothing?",
        "Jefe",
	{ 0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
          0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
          0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83,
          0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43 } },

      { "data-9 key-20",
        "Hi There",
	"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
        "\x0b\x0b\x0b\x0b",
        { 0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
          0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
          0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
          0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7 } },

      { "data-50 key-20",
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa",
        { 0x77, 0x3e, 0xa9, 0x1e, 0x36, 0x80, 0x0e, 0x46,
          0x85, 0x4d, 0xb8, 0xeb, 0xd0, 0x91, 0x81, 0xa7,
          0x29, 0x59, 0x09, 0x8b, 0x3e, 0xf8, 0xc1, 0x22,
          0xd9, 0x63, 0x55, 0x14, 0xce, 0xd5, 0x65, 0xfe } },

      { "data-50 key-26",
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd",
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
        "\x11\x12\x13\x14\x15\x16\x17\x18\x19",
	{ 0x82, 0x55, 0x8a, 0x38, 0x9a, 0x44, 0x3c, 0x0e,
          0xa4, 0xcc, 0x81, 0x98, 0x99, 0xf2, 0x08, 0x3a,
          0x85, 0xf0, 0xfa, 0xa3, 0xe5, 0x78, 0xf8, 0x07,
          0x7a, 0x2e, 0x3f, 0xf4, 0x67, 0x29, 0x66, 0x5b } },

      { "data-54 key-131",
        "Test Using Larger Than Block-Size Key - Hash Key First",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
	{ 0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f,
          0x0d, 0x8a, 0x26, 0xaa, 0xcb, 0xf5, 0xb7, 0x7f,
          0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28, 0xc5, 0x14,
          0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54 } },

      { "data-152 key-131",
        "This is a test using a larger than block-size key and a larger "
        "than block-size data. The key needs to be hashed before being "
        "used by the HMAC algorithm.",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
	{ 0x9b, 0x09, 0xff, 0xa7, 0x1b, 0x94, 0x2f, 0xcb,
          0x27, 0x63, 0x5f, 0xbc, 0xd5, 0xb0, 0xe9, 0x44,
          0xbf, 0xdc, 0x63, 0x64, 0x4f, 0x07, 0x13, 0x93,
          0x8a, 0x7f, 0x51, 0x53, 0x5c, 0x3a, 0x35, 0xe2 } },

      { NULL }
    };
  const char *what;
  const char *errtxt;
  int tvidx;

  for (tvidx=0; tv[tvidx].desc; tvidx++)
    {
      what = tv[tvidx].desc;
      errtxt = check_one (GCRY_MD_SHA256,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          tv[tvidx].expect, DIM (tv[tvidx].expect), 0);
      if (errtxt)
        goto failed;
      if (!extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("hmac", GCRY_MD_SHA256, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


static gpg_err_code_t
selftests_sha384 (int extended, selftest_report_func_t report)
{
  static struct
  {
    const char * const desc;
    const char * const data;
    const char * const key;
    const char expect[48];
  } tv[] =
    {
      { "data-28 key-4",
        "what do ya want for nothing?",
        "Jefe",
        { 0xaf, 0x45, 0xd2, 0xe3, 0x76, 0x48, 0x40, 0x31,
          0x61, 0x7f, 0x78, 0xd2, 0xb5, 0x8a, 0x6b, 0x1b,
          0x9c, 0x7e, 0xf4, 0x64, 0xf5, 0xa0, 0x1b, 0x47,
          0xe4, 0x2e, 0xc3, 0x73, 0x63, 0x22, 0x44, 0x5e,
          0x8e, 0x22, 0x40, 0xca, 0x5e, 0x69, 0xe2, 0xc7,
          0x8b, 0x32, 0x39, 0xec, 0xfa, 0xb2, 0x16, 0x49 } },

      { "data-9 key-20",
        "Hi There",
	"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
        "\x0b\x0b\x0b\x0b",
        { 0xaf, 0xd0, 0x39, 0x44, 0xd8, 0x48, 0x95, 0x62,
          0x6b, 0x08, 0x25, 0xf4, 0xab, 0x46, 0x90, 0x7f,
          0x15, 0xf9, 0xda, 0xdb, 0xe4, 0x10, 0x1e, 0xc6,
          0x82, 0xaa, 0x03, 0x4c, 0x7c, 0xeb, 0xc5, 0x9c,
          0xfa, 0xea, 0x9e, 0xa9, 0x07, 0x6e, 0xde, 0x7f,
          0x4a, 0xf1, 0x52, 0xe8, 0xb2, 0xfa, 0x9c, 0xb6 } },

      { "data-50 key-20",
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa",
        { 0x88, 0x06, 0x26, 0x08, 0xd3, 0xe6, 0xad, 0x8a,
          0x0a, 0xa2, 0xac, 0xe0, 0x14, 0xc8, 0xa8, 0x6f,
          0x0a, 0xa6, 0x35, 0xd9, 0x47, 0xac, 0x9f, 0xeb,
          0xe8, 0x3e, 0xf4, 0xe5, 0x59, 0x66, 0x14, 0x4b,
          0x2a, 0x5a, 0xb3, 0x9d, 0xc1, 0x38, 0x14, 0xb9,
          0x4e, 0x3a, 0xb6, 0xe1, 0x01, 0xa3, 0x4f, 0x27 } },

      { "data-50 key-26",
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd",
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
        "\x11\x12\x13\x14\x15\x16\x17\x18\x19",
        { 0x3e, 0x8a, 0x69, 0xb7, 0x78, 0x3c, 0x25, 0x85,
          0x19, 0x33, 0xab, 0x62, 0x90, 0xaf, 0x6c, 0xa7,
          0x7a, 0x99, 0x81, 0x48, 0x08, 0x50, 0x00, 0x9c,
          0xc5, 0x57, 0x7c, 0x6e, 0x1f, 0x57, 0x3b, 0x4e,
          0x68, 0x01, 0xdd, 0x23, 0xc4, 0xa7, 0xd6, 0x79,
          0xcc, 0xf8, 0xa3, 0x86, 0xc6, 0x74, 0xcf, 0xfb } },

      { "data-54 key-131",
        "Test Using Larger Than Block-Size Key - Hash Key First",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
        { 0x4e, 0xce, 0x08, 0x44, 0x85, 0x81, 0x3e, 0x90,
          0x88, 0xd2, 0xc6, 0x3a, 0x04, 0x1b, 0xc5, 0xb4,
          0x4f, 0x9e, 0xf1, 0x01, 0x2a, 0x2b, 0x58, 0x8f,
          0x3c, 0xd1, 0x1f, 0x05, 0x03, 0x3a, 0xc4, 0xc6,
          0x0c, 0x2e, 0xf6, 0xab, 0x40, 0x30, 0xfe, 0x82,
          0x96, 0x24, 0x8d, 0xf1, 0x63, 0xf4, 0x49, 0x52 } },

      { "data-152 key-131",
        "This is a test using a larger than block-size key and a larger "
        "than block-size data. The key needs to be hashed before being "
        "used by the HMAC algorithm.",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
        { 0x66, 0x17, 0x17, 0x8e, 0x94, 0x1f, 0x02, 0x0d,
          0x35, 0x1e, 0x2f, 0x25, 0x4e, 0x8f, 0xd3, 0x2c,
          0x60, 0x24, 0x20, 0xfe, 0xb0, 0xb8, 0xfb, 0x9a,
          0xdc, 0xce, 0xbb, 0x82, 0x46, 0x1e, 0x99, 0xc5,
          0xa6, 0x78, 0xcc, 0x31, 0xe7, 0x99, 0x17, 0x6d,
          0x38, 0x60, 0xe6, 0x11, 0x0c, 0x46, 0x52, 0x3e } },

      { NULL }
    };
  const char *what;
  const char *errtxt;
  int tvidx;

  for (tvidx=0; tv[tvidx].desc; tvidx++)
    {
      what = tv[tvidx].desc;
      errtxt = check_one (GCRY_MD_SHA384,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          tv[tvidx].expect, DIM (tv[tvidx].expect), 0);
      if (errtxt)
        goto failed;
      if (!extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("hmac", GCRY_MD_SHA384, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


static gpg_err_code_t
selftests_sha512 (int extended, selftest_report_func_t report)
{
  static struct
  {
    const char * const desc;
    const char * const data;
    const char * const key;
    const char expect[64];
  } tv[] =
    {
      { "data-28 key-4",
        "what do ya want for nothing?",
        "Jefe",
        { 0x16, 0x4b, 0x7a, 0x7b, 0xfc, 0xf8, 0x19, 0xe2,
          0xe3, 0x95, 0xfb, 0xe7, 0x3b, 0x56, 0xe0, 0xa3,
          0x87, 0xbd, 0x64, 0x22, 0x2e, 0x83, 0x1f, 0xd6,
          0x10, 0x27, 0x0c, 0xd7, 0xea, 0x25, 0x05, 0x54,
          0x97, 0x58, 0xbf, 0x75, 0xc0, 0x5a, 0x99, 0x4a,
          0x6d, 0x03, 0x4f, 0x65, 0xf8, 0xf0, 0xe6, 0xfd,
          0xca, 0xea, 0xb1, 0xa3, 0x4d, 0x4a, 0x6b, 0x4b,
          0x63, 0x6e, 0x07, 0x0a, 0x38, 0xbc, 0xe7, 0x37 } },

      { "data-9 key-20",
        "Hi There",
	"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
        "\x0b\x0b\x0b\x0b",
        { 0x87, 0xaa, 0x7c, 0xde, 0xa5, 0xef, 0x61, 0x9d,
          0x4f, 0xf0, 0xb4, 0x24, 0x1a, 0x1d, 0x6c, 0xb0,
          0x23, 0x79, 0xf4, 0xe2, 0xce, 0x4e, 0xc2, 0x78,
          0x7a, 0xd0, 0xb3, 0x05, 0x45, 0xe1, 0x7c, 0xde,
          0xda, 0xa8, 0x33, 0xb7, 0xd6, 0xb8, 0xa7, 0x02,
          0x03, 0x8b, 0x27, 0x4e, 0xae, 0xa3, 0xf4, 0xe4,
          0xbe, 0x9d, 0x91, 0x4e, 0xeb, 0x61, 0xf1, 0x70,
          0x2e, 0x69, 0x6c, 0x20, 0x3a, 0x12, 0x68, 0x54 } },

      { "data-50 key-20",
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa",
        { 0xfa, 0x73, 0xb0, 0x08, 0x9d, 0x56, 0xa2, 0x84,
          0xef, 0xb0, 0xf0, 0x75, 0x6c, 0x89, 0x0b, 0xe9,
          0xb1, 0xb5, 0xdb, 0xdd, 0x8e, 0xe8, 0x1a, 0x36,
          0x55, 0xf8, 0x3e, 0x33, 0xb2, 0x27, 0x9d, 0x39,
          0xbf, 0x3e, 0x84, 0x82, 0x79, 0xa7, 0x22, 0xc8,
          0x06, 0xb4, 0x85, 0xa4, 0x7e, 0x67, 0xc8, 0x07,
          0xb9, 0x46, 0xa3, 0x37, 0xbe, 0xe8, 0x94, 0x26,
          0x74, 0x27, 0x88, 0x59, 0xe1, 0x32, 0x92, 0xfb } },

      { "data-50 key-26",
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd",
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
        "\x11\x12\x13\x14\x15\x16\x17\x18\x19",
        { 0xb0, 0xba, 0x46, 0x56, 0x37, 0x45, 0x8c, 0x69,
          0x90, 0xe5, 0xa8, 0xc5, 0xf6, 0x1d, 0x4a, 0xf7,
          0xe5, 0x76, 0xd9, 0x7f, 0xf9, 0x4b, 0x87, 0x2d,
          0xe7, 0x6f, 0x80, 0x50, 0x36, 0x1e, 0xe3, 0xdb,
          0xa9, 0x1c, 0xa5, 0xc1, 0x1a, 0xa2, 0x5e, 0xb4,
          0xd6, 0x79, 0x27, 0x5c, 0xc5, 0x78, 0x80, 0x63,
          0xa5, 0xf1, 0x97, 0x41, 0x12, 0x0c, 0x4f, 0x2d,
          0xe2, 0xad, 0xeb, 0xeb, 0x10, 0xa2, 0x98, 0xdd } },

      { "data-54 key-131",
        "Test Using Larger Than Block-Size Key - Hash Key First",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
        { 0x80, 0xb2, 0x42, 0x63, 0xc7, 0xc1, 0xa3, 0xeb,
          0xb7, 0x14, 0x93, 0xc1, 0xdd, 0x7b, 0xe8, 0xb4,
          0x9b, 0x46, 0xd1, 0xf4, 0x1b, 0x4a, 0xee, 0xc1,
          0x12, 0x1b, 0x01, 0x37, 0x83, 0xf8, 0xf3, 0x52,
          0x6b, 0x56, 0xd0, 0x37, 0xe0, 0x5f, 0x25, 0x98,
          0xbd, 0x0f, 0xd2, 0x21, 0x5d, 0x6a, 0x1e, 0x52,
          0x95, 0xe6, 0x4f, 0x73, 0xf6, 0x3f, 0x0a, 0xec,
          0x8b, 0x91, 0x5a, 0x98, 0x5d, 0x78, 0x65, 0x98 } },

      { "data-152 key-131",
        "This is a test using a larger than block-size key and a larger "
        "than block-size data. The key needs to be hashed before being "
        "used by the HMAC algorithm.",
	"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",
        { 0xe3, 0x7b, 0x6a, 0x77, 0x5d, 0xc8, 0x7d, 0xba,
          0xa4, 0xdf, 0xa9, 0xf9, 0x6e, 0x5e, 0x3f, 0xfd,
          0xde, 0xbd, 0x71, 0xf8, 0x86, 0x72, 0x89, 0x86,
          0x5d, 0xf5, 0xa3, 0x2d, 0x20, 0xcd, 0xc9, 0x44,
          0xb6, 0x02, 0x2c, 0xac, 0x3c, 0x49, 0x82, 0xb1,
          0x0d, 0x5e, 0xeb, 0x55, 0xc3, 0xe4, 0xde, 0x15,
          0x13, 0x46, 0x76, 0xfb, 0x6d, 0xe0, 0x44, 0x60,
          0x65, 0xc9, 0x74, 0x40, 0xfa, 0x8c, 0x6a, 0x58 } },

      { NULL }
    };
  const char *what;
  const char *errtxt;
  int tvidx;

  for (tvidx=0; tv[tvidx].desc; tvidx++)
    {
      what = tv[tvidx].desc;
      errtxt = check_one (GCRY_MD_SHA512,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          tv[tvidx].expect, DIM (tv[tvidx].expect), 0);
      if (errtxt)
        goto failed;
      if (!extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("hmac", GCRY_MD_SHA512, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}



/* Test for the SHA3 algorithms.  Vectors taken on 2017-07-18 from
 * http://www.wolfgang-ehrhardt.de/hmac-sha3-testvectors.html  */
static gpg_err_code_t
selftests_sha3 (int hashalgo, int extended, selftest_report_func_t report)
{
  static struct
  {
    const char * const desc;
    const char * const data;
    const char * const key;
    const char expect_224[28];
    const char expect_256[32];
    const char expect_384[48];
    const char expect_512[64];
    unsigned char trunc;
  } tv[] =
    {
      { "data-9 key-20", /* Test 1 */
        "Hi There",
	"\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b"
        "\x0b\x0b\x0b\x0b",

        { 0x3b, 0x16, 0x54, 0x6b, 0xbc, 0x7b, 0xe2, 0x70,
          0x6a, 0x03, 0x1d, 0xca, 0xfd, 0x56, 0x37, 0x3d,
          0x98, 0x84, 0x36, 0x76, 0x41, 0xd8, 0xc5, 0x9a,
          0xf3, 0xc8, 0x60, 0xf7                          },
        { 0xba, 0x85, 0x19, 0x23, 0x10, 0xdf, 0xfa, 0x96,
          0xe2, 0xa3, 0xa4, 0x0e, 0x69, 0x77, 0x43, 0x51,
          0x14, 0x0b, 0xb7, 0x18, 0x5e, 0x12, 0x02, 0xcd,
          0xcc, 0x91, 0x75, 0x89, 0xf9, 0x5e, 0x16, 0xbb  },
        { 0x68, 0xd2, 0xdc, 0xf7, 0xfd, 0x4d, 0xdd, 0x0a,
          0x22, 0x40, 0xc8, 0xa4, 0x37, 0x30, 0x5f, 0x61,
          0xfb, 0x73, 0x34, 0xcf, 0xb5, 0xd0, 0x22, 0x6e,
          0x1b, 0xc2, 0x7d, 0xc1, 0x0a, 0x2e, 0x72, 0x3a,
          0x20, 0xd3, 0x70, 0xb4, 0x77, 0x43, 0x13, 0x0e,
          0x26, 0xac, 0x7e, 0x3d, 0x53, 0x28, 0x86, 0xbd  },
        { 0xeb, 0x3f, 0xbd, 0x4b, 0x2e, 0xaa, 0xb8, 0xf5,
          0xc5, 0x04, 0xbd, 0x3a, 0x41, 0x46, 0x5a, 0xac,
          0xec, 0x15, 0x77, 0x0a, 0x7c, 0xab, 0xac, 0x53,
          0x1e, 0x48, 0x2f, 0x86, 0x0b, 0x5e, 0xc7, 0xba,
          0x47, 0xcc, 0xb2, 0xc6, 0xf2, 0xaf, 0xce, 0x8f,
          0x88, 0xd2, 0x2b, 0x6d, 0xc6, 0x13, 0x80, 0xf2,
          0x3a, 0x66, 0x8f, 0xd3, 0x88, 0x8b, 0xb8, 0x05,
          0x37, 0xc0, 0xa0, 0xb8, 0x64, 0x07, 0x68, 0x9e  }
      },

      { "data-28 key-4",  /* Test 2  */
        /* Test with a key shorter than the length of the HMAC output. */
        "what do ya want for nothing?",
        "Jefe",

        { 0x7f, 0xdb, 0x8d, 0xd8, 0x8b, 0xd2, 0xf6, 0x0d,
          0x1b, 0x79, 0x86, 0x34, 0xad, 0x38, 0x68, 0x11,
          0xc2, 0xcf, 0xc8, 0x5b, 0xfa, 0xf5, 0xd5, 0x2b,
          0xba, 0xce, 0x5e, 0x66                          },
        { 0xc7, 0xd4, 0x07, 0x2e, 0x78, 0x88, 0x77, 0xae,
          0x35, 0x96, 0xbb, 0xb0, 0xda, 0x73, 0xb8, 0x87,
          0xc9, 0x17, 0x1f, 0x93, 0x09, 0x5b, 0x29, 0x4a,
          0xe8, 0x57, 0xfb, 0xe2, 0x64, 0x5e, 0x1b, 0xa5  },
        { 0xf1, 0x10, 0x1f, 0x8c, 0xbf, 0x97, 0x66, 0xfd,
          0x67, 0x64, 0xd2, 0xed, 0x61, 0x90, 0x3f, 0x21,
          0xca, 0x9b, 0x18, 0xf5, 0x7c, 0xf3, 0xe1, 0xa2,
          0x3c, 0xa1, 0x35, 0x08, 0xa9, 0x32, 0x43, 0xce,
          0x48, 0xc0, 0x45, 0xdc, 0x00, 0x7f, 0x26, 0xa2,
          0x1b, 0x3f, 0x5e, 0x0e, 0x9d, 0xf4, 0xc2, 0x0a  },
        { 0x5a, 0x4b, 0xfe, 0xab, 0x61, 0x66, 0x42, 0x7c,
          0x7a, 0x36, 0x47, 0xb7, 0x47, 0x29, 0x2b, 0x83,
          0x84, 0x53, 0x7c, 0xdb, 0x89, 0xaf, 0xb3, 0xbf,
          0x56, 0x65, 0xe4, 0xc5, 0xe7, 0x09, 0x35, 0x0b,
          0x28, 0x7b, 0xae, 0xc9, 0x21, 0xfd, 0x7c, 0xa0,
          0xee, 0x7a, 0x0c, 0x31, 0xd0, 0x22, 0xa9, 0x5e,
          0x1f, 0xc9, 0x2b, 0xa9, 0xd7, 0x7d, 0xf8, 0x83,
          0x96, 0x02, 0x75, 0xbe, 0xb4, 0xe6, 0x20, 0x24  }
      },

      { "data-50 key-20",  /* Test 3 */
        /* Test with a combined length of key and data that is larger
         * than 64 bytes (= block-size of SHA-224 and SHA-256).  */
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd"
        "\xdd\xdd",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa",

        { 0x67, 0x6c, 0xfc, 0x7d, 0x16, 0x15, 0x36, 0x38,
          0x78, 0x03, 0x90, 0x69, 0x2b, 0xe1, 0x42, 0xd2,
          0xdf, 0x7c, 0xe9, 0x24, 0xb9, 0x09, 0xc0, 0xc0,
          0x8d, 0xbf, 0xdc, 0x1a  },
        { 0x84, 0xec, 0x79, 0x12, 0x4a, 0x27, 0x10, 0x78,
          0x65, 0xce, 0xdd, 0x8b, 0xd8, 0x2d, 0xa9, 0x96,
          0x5e, 0x5e, 0xd8, 0xc3, 0x7b, 0x0a, 0xc9, 0x80,
          0x05, 0xa7, 0xf3, 0x9e, 0xd5, 0x8a, 0x42, 0x07  },
        { 0x27, 0x5c, 0xd0, 0xe6, 0x61, 0xbb, 0x8b, 0x15,
          0x1c, 0x64, 0xd2, 0x88, 0xf1, 0xf7, 0x82, 0xfb,
          0x91, 0xa8, 0xab, 0xd5, 0x68, 0x58, 0xd7, 0x2b,
          0xab, 0xb2, 0xd4, 0x76, 0xf0, 0x45, 0x83, 0x73,
          0xb4, 0x1b, 0x6a, 0xb5, 0xbf, 0x17, 0x4b, 0xec,
          0x42, 0x2e, 0x53, 0xfc, 0x31, 0x35, 0xac, 0x6e  },
        { 0x30, 0x9e, 0x99, 0xf9, 0xec, 0x07, 0x5e, 0xc6,
          0xc6, 0xd4, 0x75, 0xed, 0xa1, 0x18, 0x06, 0x87,
          0xfc, 0xf1, 0x53, 0x11, 0x95, 0x80, 0x2a, 0x99,
          0xb5, 0x67, 0x74, 0x49, 0xa8, 0x62, 0x51, 0x82,
          0x85, 0x1c, 0xb3, 0x32, 0xaf, 0xb6, 0xa8, 0x9c,
          0x41, 0x13, 0x25, 0xfb, 0xcb, 0xcd, 0x42, 0xaf,
          0xcb, 0x7b, 0x6e, 0x5a, 0xab, 0x7e, 0xa4, 0x2c,
          0x66, 0x0f, 0x97, 0xfd, 0x85, 0x84, 0xbf, 0x03  }
      },

      { "data-50 key-25",  /* Test 4 */
        /* Test with a combined length of key and data that is larger
         * than 64 bytes (= block-size of SHA-224 and SHA-256).  */
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd"
        "\xcd\xcd",
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
        "\x11\x12\x13\x14\x15\x16\x17\x18\x19",

        { 0xa9, 0xd7, 0x68, 0x5a, 0x19, 0xc4, 0xe0, 0xdb,
          0xd9, 0xdf, 0x25, 0x56, 0xcc, 0x8a, 0x7d, 0x2a,
          0x77, 0x33, 0xb6, 0x76, 0x25, 0xce, 0x59, 0x4c,
          0x78, 0x27, 0x0e, 0xeb   },
        { 0x57, 0x36, 0x6a, 0x45, 0xe2, 0x30, 0x53, 0x21,
          0xa4, 0xbc, 0x5a, 0xa5, 0xfe, 0x2e, 0xf8, 0xa9,
          0x21, 0xf6, 0xaf, 0x82, 0x73, 0xd7, 0xfe, 0x7b,
          0xe6, 0xcf, 0xed, 0xb3, 0xf0, 0xae, 0xa6, 0xd7  },
        { 0x3a, 0x5d, 0x7a, 0x87, 0x97, 0x02, 0xc0, 0x86,
          0xbc, 0x96, 0xd1, 0xdd, 0x8a, 0xa1, 0x5d, 0x9c,
          0x46, 0x44, 0x6b, 0x95, 0x52, 0x13, 0x11, 0xc6,
          0x06, 0xfd, 0xc4, 0xe3, 0x08, 0xf4, 0xb9, 0x84,
          0xda, 0x2d, 0x0f, 0x94, 0x49, 0xb3, 0xba, 0x84,
          0x25, 0xec, 0x7f, 0xb8, 0xc3, 0x1b, 0xc1, 0x36  },
        { 0xb2, 0x7e, 0xab, 0x1d, 0x6e, 0x8d, 0x87, 0x46,
          0x1c, 0x29, 0xf7, 0xf5, 0x73, 0x9d, 0xd5, 0x8e,
          0x98, 0xaa, 0x35, 0xf8, 0xe8, 0x23, 0xad, 0x38,
          0xc5, 0x49, 0x2a, 0x20, 0x88, 0xfa, 0x02, 0x81,
          0x99, 0x3b, 0xbf, 0xff, 0x9a, 0x0e, 0x9c, 0x6b,
          0xf1, 0x21, 0xae, 0x9e, 0xc9, 0xbb, 0x09, 0xd8,
          0x4a, 0x5e, 0xba, 0xc8, 0x17, 0x18, 0x2e, 0xa9,
          0x74, 0x67, 0x3f, 0xb1, 0x33, 0xca, 0x0d, 0x1d  }
      },

      { "data-20 key-20 trunc",  /* Test 5 */
        /* Test with a truncation of output to 128 bits.  */
        "Test With Truncation",
        "\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c"
        "\x0c\x0c\x0c\x0c",

        { 0x49, 0xfd, 0xd3, 0xab, 0xd0, 0x05, 0xeb, 0xb8,
          0xae, 0x63, 0xfe, 0xa9, 0x46, 0xd1, 0x88, 0x3c  },
        { 0x6e, 0x02, 0xc6, 0x45, 0x37, 0xfb, 0x11, 0x80,
          0x57, 0xab, 0xb7, 0xfb, 0x66, 0xa2, 0x3b, 0x3c  },
        { 0x47, 0xc5, 0x1a, 0xce, 0x1f, 0xfa, 0xcf, 0xfd,
          0x74, 0x94, 0x72, 0x46, 0x82, 0x61, 0x57, 0x83  },
        { 0x0f, 0xa7, 0x47, 0x59, 0x48, 0xf4, 0x3f, 0x48,
          0xca, 0x05, 0x16, 0x67, 0x1e, 0x18, 0x97, 0x8c  },
        16
      },

      { "data-54 key-131",  /* Test 6 */
        /* Test with a key larger than 128 bytes (= block-size of
         * SHA-384 and SHA-512).  */
        "Test Using Larger Than Block-Size Key - Hash Key First",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",

        { 0xb4, 0xa1, 0xf0, 0x4c, 0x00, 0x28, 0x7a, 0x9b,
          0x7f, 0x60, 0x75, 0xb3, 0x13, 0xd2, 0x79, 0xb8,
          0x33, 0xbc, 0x8f, 0x75, 0x12, 0x43, 0x52, 0xd0,
          0x5f, 0xb9, 0x99, 0x5f  },
        { 0xed, 0x73, 0xa3, 0x74, 0xb9, 0x6c, 0x00, 0x52,
          0x35, 0xf9, 0x48, 0x03, 0x2f, 0x09, 0x67, 0x4a,
          0x58, 0xc0, 0xce, 0x55, 0x5c, 0xfc, 0x1f, 0x22,
          0x3b, 0x02, 0x35, 0x65, 0x60, 0x31, 0x2c, 0x3b  },
        { 0x0f, 0xc1, 0x95, 0x13, 0xbf, 0x6b, 0xd8, 0x78,
          0x03, 0x70, 0x16, 0x70, 0x6a, 0x0e, 0x57, 0xbc,
          0x52, 0x81, 0x39, 0x83, 0x6b, 0x9a, 0x42, 0xc3,
          0xd4, 0x19, 0xe4, 0x98, 0xe0, 0xe1, 0xfb, 0x96,
          0x16, 0xfd, 0x66, 0x91, 0x38, 0xd3, 0x3a, 0x11,
          0x05, 0xe0, 0x7c, 0x72, 0xb6, 0x95, 0x3b, 0xcc  },
        { 0x00, 0xf7, 0x51, 0xa9, 0xe5, 0x06, 0x95, 0xb0,
          0x90, 0xed, 0x69, 0x11, 0xa4, 0xb6, 0x55, 0x24,
          0x95, 0x1c, 0xdc, 0x15, 0xa7, 0x3a, 0x5d, 0x58,
          0xbb, 0x55, 0x21, 0x5e, 0xa2, 0xcd, 0x83, 0x9a,
          0xc7, 0x9d, 0x2b, 0x44, 0xa3, 0x9b, 0xaf, 0xab,
          0x27, 0xe8, 0x3f, 0xde, 0x9e, 0x11, 0xf6, 0x34,
          0x0b, 0x11, 0xd9, 0x91, 0xb1, 0xb9, 0x1b, 0xf2,
          0xee, 0xe7, 0xfc, 0x87, 0x24, 0x26, 0xc3, 0xa4  }
      },

      { "data-54 key-147",  /* Test 6a */
        /* Test with a key larger than 144 bytes (= block-size of
         * SHA3-224).  */
        "Test Using Larger Than Block-Size Key - Hash Key First",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",

        { 0xb9, 0x6d, 0x73, 0x0c, 0x14, 0x8c, 0x2d, 0xaa,
          0xd8, 0x64, 0x9d, 0x83, 0xde, 0xfa, 0xa3, 0x71,
          0x97, 0x38, 0xd3, 0x47, 0x75, 0x39, 0x7b, 0x75,
          0x71, 0xc3, 0x85, 0x15  },
        { 0xa6, 0x07, 0x2f, 0x86, 0xde, 0x52, 0xb3, 0x8b,
          0xb3, 0x49, 0xfe, 0x84, 0xcd, 0x6d, 0x97, 0xfb,
          0x6a, 0x37, 0xc4, 0xc0, 0xf6, 0x2a, 0xae, 0x93,
          0x98, 0x11, 0x93, 0xa7, 0x22, 0x9d, 0x34, 0x67  },
        { 0x71, 0x3d, 0xff, 0x03, 0x02, 0xc8, 0x50, 0x86,
          0xec, 0x5a, 0xd0, 0x76, 0x8d, 0xd6, 0x5a, 0x13,
          0xdd, 0xd7, 0x90, 0x68, 0xd8, 0xd4, 0xc6, 0x21,
          0x2b, 0x71, 0x2e, 0x41, 0x64, 0x94, 0x49, 0x11,
          0x14, 0x80, 0x23, 0x00, 0x44, 0x18, 0x5a, 0x99,
          0x10, 0x3e, 0xd8, 0x20, 0x04, 0xdd, 0xbf, 0xcc  },
        { 0xb1, 0x48, 0x35, 0xc8, 0x19, 0xa2, 0x90, 0xef,
          0xb0, 0x10, 0xac, 0xe6, 0xd8, 0x56, 0x8d, 0xc6,
          0xb8, 0x4d, 0xe6, 0x0b, 0xc4, 0x9b, 0x00, 0x4c,
          0x3b, 0x13, 0xed, 0xa7, 0x63, 0x58, 0x94, 0x51,
          0xe5, 0xdd, 0x74, 0x29, 0x28, 0x84, 0xd1, 0xbd,
          0xce, 0x64, 0xe6, 0xb9, 0x19, 0xdd, 0x61, 0xdc,
          0x9c, 0x56, 0xa2, 0x82, 0xa8, 0x1c, 0x0b, 0xd1,
          0x4f, 0x1f, 0x36, 0x5b, 0x49, 0xb8, 0x3a, 0x5b  }
      },

      { "data-152 key-131",  /* Test 7  */
        /* Test with a key and data that is larger than 128 bytes (=
         * block-size of SHA-384 and SHA-512).  */
        "This is a test using a larger than block-size key and a larger "
        "than block-size data. The key needs to be hashed before being "
        "used by the HMAC algorithm.",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",

        { 0x05, 0xd8, 0xcd, 0x6d, 0x00, 0xfa, 0xea, 0x8d,
          0x1e, 0xb6, 0x8a, 0xde, 0x28, 0x73, 0x0b, 0xbd,
          0x3c, 0xba, 0xb6, 0x92, 0x9f, 0x0a, 0x08, 0x6b,
          0x29, 0xcd, 0x62, 0xa0  },
        { 0x65, 0xc5, 0xb0, 0x6d, 0x4c, 0x3d, 0xe3, 0x2a,
          0x7a, 0xef, 0x87, 0x63, 0x26, 0x1e, 0x49, 0xad,
          0xb6, 0xe2, 0x29, 0x3e, 0xc8, 0xe7, 0xc6, 0x1e,
          0x8d, 0xe6, 0x17, 0x01, 0xfc, 0x63, 0xe1, 0x23  },
        { 0x02, 0x6f, 0xdf, 0x6b, 0x50, 0x74, 0x1e, 0x37,
          0x38, 0x99, 0xc9, 0xf7, 0xd5, 0x40, 0x6d, 0x4e,
          0xb0, 0x9f, 0xc6, 0x66, 0x56, 0x36, 0xfc, 0x1a,
          0x53, 0x00, 0x29, 0xdd, 0xf5, 0xcf, 0x3c, 0xa5,
          0xa9, 0x00, 0xed, 0xce, 0x01, 0xf5, 0xf6, 0x1e,
          0x2f, 0x40, 0x8c, 0xdf, 0x2f, 0xd3, 0xe7, 0xe8  },
        { 0x38, 0xa4, 0x56, 0xa0, 0x04, 0xbd, 0x10, 0xd3,
          0x2c, 0x9a, 0xb8, 0x33, 0x66, 0x84, 0x11, 0x28,
          0x62, 0xc3, 0xdb, 0x61, 0xad, 0xcc, 0xa3, 0x18,
          0x29, 0x35, 0x5e, 0xaf, 0x46, 0xfd, 0x5c, 0x73,
          0xd0, 0x6a, 0x1f, 0x0d, 0x13, 0xfe, 0xc9, 0xa6,
          0x52, 0xfb, 0x38, 0x11, 0xb5, 0x77, 0xb1, 0xb1,
          0xd1, 0xb9, 0x78, 0x9f, 0x97, 0xae, 0x5b, 0x83,
          0xc6, 0xf4, 0x4d, 0xfc, 0xf1, 0xd6, 0x7e, 0xba  }
      },

      { "data-152 key-147",  /* Test 7a  */
        /* Test with a key larger than 144 bytes (= block-size of
         * SHA3-224). */
        "This is a test using a larger than block-size key and a larger "
        "than block-size data. The key needs to be hashed before being "
        "used by the HMAC algorithm.",
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa",

        { 0xc7, 0x9c, 0x9b, 0x09, 0x34, 0x24, 0xe5, 0x88,
          0xa9, 0x87, 0x8b, 0xbc, 0xb0, 0x89, 0xe0, 0x18,
          0x27, 0x00, 0x96, 0xe9, 0xb4, 0xb1, 0xa9, 0xe8,
          0x22, 0x0c, 0x86, 0x6a  },
        { 0xe6, 0xa3, 0x6d, 0x9b, 0x91, 0x5f, 0x86, 0xa0,
          0x93, 0xca, 0xc7, 0xd1, 0x10, 0xe9, 0xe0, 0x4c,
          0xf1, 0xd6, 0x10, 0x0d, 0x30, 0x47, 0x55, 0x09,
          0xc2, 0x47, 0x5f, 0x57, 0x1b, 0x75, 0x8b, 0x5a  },
        { 0xca, 0xd1, 0x8a, 0x8f, 0xf6, 0xc4, 0xcc, 0x3a,
          0xd4, 0x87, 0xb9, 0x5f, 0x97, 0x69, 0xe9, 0xb6,
          0x1c, 0x06, 0x2a, 0xef, 0xd6, 0x95, 0x25, 0x69,
          0xe6, 0xe6, 0x42, 0x18, 0x97, 0x05, 0x4c, 0xfc,
          0x70, 0xb5, 0xfd, 0xc6, 0x60, 0x5c, 0x18, 0x45,
          0x71, 0x12, 0xfc, 0x6a, 0xaa, 0xd4, 0x55, 0x85  },
        { 0xdc, 0x03, 0x0e, 0xe7, 0x88, 0x70, 0x34, 0xf3,
          0x2c, 0xf4, 0x02, 0xdf, 0x34, 0x62, 0x2f, 0x31,
          0x1f, 0x3e, 0x6c, 0xf0, 0x48, 0x60, 0xc6, 0xbb,
          0xd7, 0xfa, 0x48, 0x86, 0x74, 0x78, 0x2b, 0x46,
          0x59, 0xfd, 0xbd, 0xf3, 0xfd, 0x87, 0x78, 0x52,
          0x88, 0x5c, 0xfe, 0x6e, 0x22, 0x18, 0x5f, 0xe7,
          0xb2, 0xee, 0x95, 0x20, 0x43, 0x62, 0x9b, 0xc9,
          0xd5, 0xf3, 0x29, 0x8a, 0x41, 0xd0, 0x2c, 0x66  }
      }/*,*/

      /* Our API does not allow to specify a bit count and thus we
       * can't use the following test.  */
      /* { "data-5bit key-4",  /\* Test 8  *\/ */
      /*   /\* Test with data bit size no multiple of 8, the data bits are */
      /*    * '11001' from the NIST example using SHA-3 order (= 5 bits */
      /*    * from LSB hex byte 13 or 5 bits from MSB hex byte c8).  *\/ */
      /*   "\xc8", */
      /*   "Jefe", */

      /*   { 0x5f, 0x8c, 0x0e, 0xa7, 0xfa, 0xfe, 0xcd, 0x0c, */
      /*     0x34, 0x63, 0xaa, 0xd0, 0x97, 0x42, 0xce, 0xce, */
      /*     0xb1, 0x42, 0xfe, 0x0a, 0xb6, 0xf4, 0x53, 0x94, */
      /*     0x38, 0xc5, 0x9d, 0xe8  }, */
      /*   { 0xec, 0x82, 0x22, 0x77, 0x3f, 0xac, 0x68, 0xb3, */
      /*     0xd3, 0xdc, 0xb1, 0x82, 0xae, 0xc8, 0xb0, 0x50, */
      /*     0x7a, 0xce, 0x44, 0x48, 0xd2, 0x0a, 0x11, 0x47, */
      /*     0xe6, 0x82, 0x11, 0x8d, 0xa4, 0xe3, 0xf4, 0x4c  }, */
      /*   { 0x21, 0xfb, 0xd3, 0xbf, 0x3e, 0xbb, 0xa3, 0xcf, */
      /*     0xc9, 0xef, 0x64, 0xc0, 0x59, 0x1c, 0x92, 0xc5, */
      /*     0xac, 0xb2, 0x65, 0xe9, 0x2d, 0x87, 0x61, 0xd1, */
      /*     0xf9, 0x1a, 0x52, 0xa1, 0x03, 0xa6, 0xc7, 0x96, */
      /*     0x94, 0xcf, 0xd6, 0x7a, 0x9a, 0x2a, 0xc1, 0x32, */
      /*     0x4f, 0x02, 0xfe, 0xa6, 0x3b, 0x81, 0xef, 0xfc  }, */
      /*   { 0x27, 0xf9, 0x38, 0x8c, 0x15, 0x67, 0xef, 0x4e, */
      /*     0xf2, 0x00, 0x60, 0x2a, 0x6c, 0xf8, 0x71, 0xd6, */
      /*     0x8a, 0x6f, 0xb0, 0x48, 0xd4, 0x73, 0x7a, 0xc4, */
      /*     0x41, 0x8a, 0x2f, 0x02, 0x12, 0x89, 0xd1, 0x3d, */
      /*     0x1f, 0xd1, 0x12, 0x0f, 0xec, 0xb9, 0xcf, 0x96, */
      /*     0x4c, 0x5b, 0x11, 0x7a, 0xb5, 0xb1, 0x1c, 0x61, */
      /*     0x4b, 0x2d, 0xa3, 0x9d, 0xad, 0xd5, 0x1f, 0x2f, */
      /*     0x5e, 0x22, 0xaa, 0xcc, 0xec, 0x7d, 0x57, 0x6e  } */
      /* } */

    };
  const char *what;
  const char *errtxt;
  int tvidx;
  const char *expect;
  int nexpect;

  for (tvidx=0; tvidx < DIM(tv); tvidx++)
    {
      what = tv[tvidx].desc;
      if (hashalgo == GCRY_MD_SHA3_224)
        {
          expect = tv[tvidx].expect_224;
          nexpect = DIM (tv[tvidx].expect_224);
        }
      else if (hashalgo == GCRY_MD_SHA3_256)
        {
          expect = tv[tvidx].expect_256;
          nexpect = DIM (tv[tvidx].expect_256);
        }
      else if (hashalgo == GCRY_MD_SHA3_384)
        {
          expect = tv[tvidx].expect_384;
          nexpect = DIM (tv[tvidx].expect_384);
        }
      else if (hashalgo == GCRY_MD_SHA3_512)
        {
          expect = tv[tvidx].expect_512;
          nexpect = DIM (tv[tvidx].expect_512);
        }
      else
        BUG();

      if (tv[tvidx].trunc && tv[tvidx].trunc < nexpect)
        nexpect = tv[tvidx].trunc;

      errtxt = check_one (hashalgo,
                          tv[tvidx].data, strlen (tv[tvidx].data),
                          tv[tvidx].key, strlen (tv[tvidx].key),
                          expect, nexpect, !!tv[tvidx].trunc);
      if (errtxt)
        goto failed;
      if (!extended)
        break;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("hmac", hashalgo, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


static gpg_err_code_t
hmac_selftest (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_MAC_HMAC_SHA1:
      ec = selftests_sha1 (extended, report);
      break;
    case GCRY_MAC_HMAC_SHA224:
      ec = selftests_sha224 (extended, report);
      break;
    case GCRY_MAC_HMAC_SHA256:
      ec = selftests_sha256 (extended, report);
      break;
    case GCRY_MAC_HMAC_SHA384:
      ec = selftests_sha384 (extended, report);
      break;
    case GCRY_MAC_HMAC_SHA512:
      ec = selftests_sha512 (extended, report);
      break;

    case GCRY_MAC_HMAC_SHA3_224:
    case GCRY_MAC_HMAC_SHA3_256:
    case GCRY_MAC_HMAC_SHA3_384:
    case GCRY_MAC_HMAC_SHA3_512:
      {
        int md_algo = map_mac_algo_to_md (algo);
        ec = selftests_sha3 (md_algo, extended, report);
      }
      break;

    default:
      ec = GPG_ERR_MAC_ALGO;
      break;
    }

  return ec;
}


static const gcry_mac_spec_ops_t hmac_ops = {
  hmac_open,
  hmac_close,
  hmac_setkey,
  NULL,
  hmac_reset,
  hmac_write,
  hmac_read,
  hmac_verify,
  hmac_get_maclen,
  hmac_get_keylen,
  NULL,
  hmac_selftest
};


#if USE_SHA1
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha1 = {
  GCRY_MAC_HMAC_SHA1, {0, 1}, "HMAC_SHA1",
  &hmac_ops
};
#endif
#if USE_SHA256
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha256 = {
  GCRY_MAC_HMAC_SHA256, {0, 1}, "HMAC_SHA256",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha224 = {
  GCRY_MAC_HMAC_SHA224, {0, 1}, "HMAC_SHA224",
  &hmac_ops
};
#endif
#if USE_SHA512
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha512 = {
  GCRY_MAC_HMAC_SHA512, {0, 1}, "HMAC_SHA512",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha384 = {
  GCRY_MAC_HMAC_SHA384, {0, 1}, "HMAC_SHA384",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha512_256 = {
  GCRY_MAC_HMAC_SHA512_256, {0, 1}, "HMAC_SHA512_256",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha512_224 = {
  GCRY_MAC_HMAC_SHA512_224, {0, 1}, "HMAC_SHA512_224",
  &hmac_ops
};

#endif
#if USE_SHA3
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_224 = {
  GCRY_MAC_HMAC_SHA3_224, {0, 1}, "HMAC_SHA3_224",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_256 = {
  GCRY_MAC_HMAC_SHA3_256, {0, 1}, "HMAC_SHA3_256",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_384 = {
  GCRY_MAC_HMAC_SHA3_384, {0, 1}, "HMAC_SHA3_384",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_512 = {
  GCRY_MAC_HMAC_SHA3_512, {0, 1}, "HMAC_SHA3_512",
  &hmac_ops
};
#endif
#if USE_GOST_R_3411_94
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_gost3411_94 = {
  GCRY_MAC_HMAC_GOSTR3411_94, {0, 0}, "HMAC_GOSTR3411_94",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_gost3411_cp = {
  GCRY_MAC_HMAC_GOSTR3411_CP, {0, 0}, "HMAC_GOSTR3411_CP",
  &hmac_ops
};
#endif
#if USE_GOST_R_3411_12
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_stribog256 = {
  GCRY_MAC_HMAC_STRIBOG256, {0, 0}, "HMAC_STRIBOG256",
  &hmac_ops
};

const gcry_mac_spec_t _gcry_mac_type_spec_hmac_stribog512 = {
  GCRY_MAC_HMAC_STRIBOG512, {0, 0}, "HMAC_STRIBOG512",
  &hmac_ops
};
#endif
#if USE_WHIRLPOOL
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_whirlpool = {
  GCRY_MAC_HMAC_WHIRLPOOL, {0, 0}, "HMAC_WHIRLPOOL",
  &hmac_ops
};
#endif
#if USE_RMD160
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_rmd160 = {
  GCRY_MAC_HMAC_RMD160, {0, 0}, "HMAC_RIPEMD160",
  &hmac_ops
};
#endif
#if USE_TIGER
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_tiger1 = {
  GCRY_MAC_HMAC_TIGER1, {0, 0}, "HMAC_TIGER",
  &hmac_ops
};
#endif
#if USE_MD5
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_md5 = {
  GCRY_MAC_HMAC_MD5, {0, 1}, "HMAC_MD5",
  &hmac_ops
};
#endif
#if USE_MD4
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_md4 = {
  GCRY_MAC_HMAC_MD4, {0, 0}, "HMAC_MD4",
  &hmac_ops
};
#endif
#if USE_MD2
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_md2 = {
  GCRY_MAC_HMAC_MD2, {0, 0}, "HMAC_MD2",
  &hmac_ops
};
#endif
#if USE_BLAKE2
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_512 = {
  GCRY_MAC_HMAC_BLAKE2B_512, {0, 0}, "HMAC_BLAKE2B_512",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_384 = {
  GCRY_MAC_HMAC_BLAKE2B_384, {0, 0}, "HMAC_BLAKE2B_384",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_256 = {
  GCRY_MAC_HMAC_BLAKE2B_256, {0, 0}, "HMAC_BLAKE2B_256",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_160 = {
  GCRY_MAC_HMAC_BLAKE2B_160, {0, 0}, "HMAC_BLAKE2B_160",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_256 = {
  GCRY_MAC_HMAC_BLAKE2S_256, {0, 0}, "HMAC_BLAKE2S_256",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_224 = {
  GCRY_MAC_HMAC_BLAKE2S_224, {0, 0}, "HMAC_BLAKE2S_224",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_160 = {
  GCRY_MAC_HMAC_BLAKE2S_160, {0, 0}, "HMAC_BLAKE2S_160",
  &hmac_ops
};
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_128 = {
  GCRY_MAC_HMAC_BLAKE2S_128, {0, 0}, "HMAC_BLAKE2S_128",
  &hmac_ops
};
#endif
#if USE_SM3
const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sm3 = {
  GCRY_MAC_HMAC_SM3, {0, 0}, "HMAC_SM3",
  &hmac_ops
};
#endif
