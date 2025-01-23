/* hmac.c -  HMAC regression tests
 *	Copyright (C) 2005 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define PGM "hmac"
#include "t-common.h"


static void
check_one_mac (int algo,
               const void *key, size_t keylen,
               const void *data, size_t datalen,
               const char *expect)
{
  gcry_md_hd_t hd;
  unsigned char *p;
  int mdlen;
  int i;
  gcry_error_t err = 0;

  err = gcry_md_open (&hd, algo, GCRY_MD_FLAG_HMAC);
  if (err)
    {
      fail ("algo %d, gcry_md_open failed: %s\n", algo, gpg_strerror (err));
      return;
    }

  mdlen = gcry_md_get_algo_dlen (algo);
  if (mdlen < 1 || mdlen > 500)
    {
      fail ("algo %d, gcry_md_get_algo_dlen failed: %d\n", algo, mdlen);
      return;
    }

  err = gcry_md_setkey (hd, key, keylen);
  if (err)
    {
      fail ("algo %d, gcry_md_setkey failed: %s\n", algo, gpg_strerror (err));
      return;
    }

  gcry_md_write (hd, data, datalen);

  p = gcry_md_read (hd, 0);

  if (memcmp (p, expect, mdlen))
    {
      printf ("computed: ");
      for (i = 0; i < mdlen; i++)
	printf ("%02x ", p[i] & 0xFF);
      printf ("\nexpected: ");
      for (i = 0; i < mdlen; i++)
	printf ("%02x ", expect[i] & 0xFF);
      printf ("\n");

      fail ("algo %d, MAC does not match\n", algo);
    }

  gcry_md_close (hd);
}

static void
check_hmac (void)
{
  unsigned char key[128];
  int i, j;

  if (verbose)
    fprintf (stderr, "checking FIPS-198a, A.1\n");
  for (i=0; i < 64; i++)
    key[i] = i;
  check_one_mac (GCRY_MD_SHA1, key, 64, "Sample #1", 9,
                 "\x4f\x4c\xa3\xd5\xd6\x8b\xa7\xcc\x0a\x12"
                 "\x08\xc9\xc6\x1e\x9c\x5d\xa0\x40\x3c\x0a");

  if (verbose)
    fprintf (stderr, "checking FIPS-198a, A.2\n");
  for (i=0, j=0x30; i < 20; i++)
    key[i] = j++;
  check_one_mac (GCRY_MD_SHA1, key, 20, "Sample #2", 9,
                 "\x09\x22\xd3\x40\x5f\xaa\x3d\x19\x4f\x82"
                 "\xa4\x58\x30\x73\x7d\x5c\xc6\xc7\x5d\x24");

  if (verbose)
    fprintf (stderr, "checking FIPS-198a, A.3\n");
  for (i=0, j=0x50; i < 100; i++)
    key[i] = j++;
  check_one_mac (GCRY_MD_SHA1, key, 100, "Sample #3", 9,
                 "\xbc\xf4\x1e\xab\x8b\xb2\xd8\x02\xf3\xd0"
                 "\x5c\xaf\x7c\xb0\x92\xec\xf8\xd1\xa3\xaa");

  if (verbose)
    fprintf (stderr, "checking FIPS-198a, A.4\n");
  for (i=0, j=0x70; i < 49; i++)
    key[i] = j++;
  check_one_mac (GCRY_MD_SHA1, key, 49, "Sample #4", 9,
                 "\x9e\xa8\x86\xef\xe2\x68\xdb\xec\xce\x42"
                 "\x0c\x75\x24\xdf\x32\xe0\x75\x1a\x2a\x26");

}


static void
check_hmac_multi (void)
{
  gpg_error_t err;
  unsigned char key[128];
  const char msg[] = "Sample #1";
  const char mac[] = ("\x4f\x4c\xa3\xd5\xd6\x8b\xa7\xcc\x0a\x12"
                      "\x08\xc9\xc6\x1e\x9c\x5d\xa0\x40\x3c\x0a");
  gcry_buffer_t iov[4];
  char digest[64];
  int i;
  int algo;
  int maclen;

  if (verbose)
    fprintf (stderr, "checking HMAC using multiple buffers\n");
  for (i=0; i < 64; i++)
    key[i] = i;

  memset (iov, 0, sizeof iov);
  iov[0].data = key;
  iov[0].len = 64;
  iov[1].data = (void*)msg;
  iov[1].off = 0;
  iov[1].len = 3;
  iov[2].data = (void*)msg;
  iov[2].off = 3;
  iov[2].len = 1;
  iov[3].data = (void*)msg;
  iov[3].off = 4;
  iov[3].len = 5;

  algo = GCRY_MD_SHA1;
  maclen = gcry_md_get_algo_dlen (algo);
  err = gcry_md_hash_buffers (algo, GCRY_MD_FLAG_HMAC, digest, iov, 4);
  if (err)
    {
      fail ("gcry_md_hash_buffers failed for algo %d: %s\n",
            algo, gpg_strerror (err));
      return;
    }

  if (memcmp (digest, mac, maclen))
    {
      printf ("computed: ");
      for (i = 0; i < maclen; i++)
	printf ("%02x ", digest[i] & 0xFF);
      printf ("\nexpected: ");
      for (i = 0; i < maclen; i++)
	printf ("%02x ", mac[i] & 0xFF);
      printf ("\n");

      fail ("gcry_md_hash_buffers, algo %d, MAC does not match\n", algo);
    }
}


int
main (int argc, char **argv)
{
  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));
  check_hmac ();
  check_hmac_multi ();

  return error_count ? 1 : 0;
}
