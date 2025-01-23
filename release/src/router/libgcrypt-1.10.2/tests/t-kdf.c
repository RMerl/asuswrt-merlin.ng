/* t-kdf.c -  KDF regression tests
 * Copyright (C) 2011 Free Software Foundation, Inc.
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
#include <assert.h>

#include "stopwatch.h"
#define PGM "t-kdf"
#include "t-common.h"

static int in_fips_mode;


static void
dummy_consumer (volatile char *buffer, size_t buflen)
{
  (void)buffer;
  (void)buflen;
}


static void
bench_s2k (unsigned long s2kcount)
{
  gpg_error_t err;
  const char passphrase[] = "123456789abcdef0";
  char keybuf[128/8];
  unsigned int repetitions = 10;
  unsigned int count;
  const char *elapsed;
  int pass = 0;

 again:
  start_timer ();
  for (count = 0; count < repetitions; count++)
    {
      err = gcry_kdf_derive (passphrase, strlen (passphrase),
                             GCRY_KDF_ITERSALTED_S2K,
                             GCRY_MD_SHA1, "saltsalt", 8, s2kcount,
                             sizeof keybuf, keybuf);
      if (err)
        die ("gcry_kdf_derive failed: %s\n", gpg_strerror (err));
      dummy_consumer (keybuf, sizeof keybuf);
    }
  stop_timer ();

  elapsed = elapsed_time (repetitions);
  if (!pass++)
    {
      if (!atoi (elapsed))
        {
          repetitions = 10000;
          goto again;
        }
      else if (atoi (elapsed) < 10)
        {
          repetitions = 100;
          goto again;
        }
    }

  printf ("%s\n", elapsed);
}


static void
check_openpgp (void)
{
  /* Test vectors manually created with gpg 1.4 derived code: In
     passphrase.c:hash_passpharse, add this code to the end of the
     function:

       ===8<===
       printf ("{\n"
               "  \"");
       for (i=0; i < pwlen; i++)
         {
           if (i && !(i%16))
             printf ("\"\n  \"");
           printf ("\\x%02x", ((const unsigned char *)pw)[i]);
         }
       printf ("\", %d,\n", pwlen);

       printf ("  %s, %s,\n",
               s2k->mode == 0? "GCRY_KDF_SIMPLE_S2K":
               s2k->mode == 1? "GCRY_KDF_SALTED_S2K":
               s2k->mode == 3? "GCRY_KDF_ITERSALTED_S2K":"?",
               s2k->hash_algo == DIGEST_ALGO_MD5   ? "GCRY_MD_MD5" :
               s2k->hash_algo == DIGEST_ALGO_SHA1  ? "GCRY_MD_SHA1" :
               s2k->hash_algo == DIGEST_ALGO_RMD160? "GCRY_MD_RMD160" :
               s2k->hash_algo == DIGEST_ALGO_SHA256? "GCRY_MD_SHA256" :
               s2k->hash_algo == DIGEST_ALGO_SHA384? "GCRY_MD_SHA384" :
               s2k->hash_algo == DIGEST_ALGO_SHA512? "GCRY_MD_SHA512" :
               s2k->hash_algo == DIGEST_ALGO_SHA224? "GCRY_MD_SHA224" : "?");

       if (s2k->mode == 0)
         printf ("  NULL, 0,\n");
       else
         {
           printf ("  \"");
           for (i=0; i < 8; i++)
             printf ("\\x%02x", (unsigned int)s2k->salt[i]);
           printf ("\", %d,\n", 8);
         }

       if (s2k->mode == 3)
         printf ("  %lu,\n", (unsigned long)S2K_DECODE_COUNT(s2k->count));
       else
         printf ("  0,\n");

       printf ("  %d,\n", (int)dek->keylen);

       printf ("  \"");
       for (i=0; i < dek->keylen; i++)
         {
           if (i && !(i%16))
             printf ("\"\n  \"");
           printf ("\\x%02x", ((unsigned char *)dek->key)[i]);
         }
       printf ("\"\n},\n");
       ===>8===

     Then prepare a file x.inp with utf8 encoding:

       ===8<===
       0 aes    md5 1024 a
       0 aes    md5 1024 ab
       0 aes    md5 1024 abc
       0 aes    md5 1024 abcd
       0 aes    md5 1024 abcde
       0 aes    md5 1024 abcdef
       0 aes    md5 1024 abcdefg
       0 aes    md5 1024 abcdefgh
       0 aes    md5 1024 abcdefghi
       0 aes    md5 1024 abcdefghijklmno
       0 aes    md5 1024 abcdefghijklmnop
       0 aes    md5 1024 abcdefghijklmnopq
       0 aes    md5 1024 Long_sentence_used_as_passphrase
       0 aes    md5 1024 With_utf8_umlauts:äüÖß
       0 aes    sha1 1024 a
       0 aes    sha1 1024 ab
       0 aes    sha1 1024 abc
       0 aes    sha1 1024 abcd
       0 aes    sha1 1024 abcde
       0 aes    sha1 1024 abcdef
       0 aes    sha1 1024 abcdefg
       0 aes    sha1 1024 abcdefgh
       0 aes    sha1 1024 abcdefghi
       0 aes    sha1 1024 abcdefghijklmno
       0 aes    sha1 1024 abcdefghijklmnop
       0 aes    sha1 1024 abcdefghijklmnopq
       0 aes    sha1 1024 abcdefghijklmnopqr
       0 aes    sha1 1024 abcdefghijklmnopqrs
       0 aes    sha1 1024 abcdefghijklmnopqrst
       0 aes    sha1 1024 abcdefghijklmnopqrstu
       0 aes    sha1 1024 Long_sentence_used_as_passphrase
       0 aes256 sha1 1024 Long_sentence_used_as_passphrase
       0 aes    sha1 1024 With_utf8_umlauts:äüÖß
       3 aes    sha1 1024 a
       3 aes    sha1 1024 ab
       3 aes    sha1 1024 abc
       3 aes    sha1 1024 abcd
       3 aes    sha1 1024 abcde
       3 aes    sha1 1024 abcdef
       3 aes    sha1 1024 abcdefg
       3 aes    sha1 1024 abcdefgh
       3 aes    sha1 1024 abcdefghi
       3 aes    sha1 1024 abcdefghijklmno
       3 aes    sha1 1024 abcdefghijklmnop
       3 aes    sha1 1024 abcdefghijklmnopq
       3 aes    sha1 1024 abcdefghijklmnopqr
       3 aes    sha1 1024 abcdefghijklmnopqrs
       3 aes    sha1 1024 abcdefghijklmnopqrst
       3 aes    sha1 1024 abcdefghijklmnopqrstu
       3 aes    sha1 1024 With_utf8_umlauts:äüÖß
       3 aes    sha1 1024 Long_sentence_used_as_passphrase
       3 aes    sha1 10240 Long_sentence_used_as_passphrase
       3 aes    sha1 102400 Long_sentence_used_as_passphrase
       3 aes192 sha1 1024 a
       3 aes192 sha1 1024 abcdefg
       3 aes192 sha1 1024 abcdefghi
       3 aes192 sha1 1024 abcdefghi
       3 aes192 sha1 1024 Long_sentence_used_as_passphrase
       3 aes256 sha1 1024 a
       3 aes256 sha1 1024 abcdefg
       3 aes256 sha1 1024 abcdefghi
       3 aes256 sha1 1024 abcdefghi
       3 aes256 sha1 1024 Long_sentence_used_as_passphrase
       0 aes    sha256 1024 Long_sentence_used_as_passphrase
       1 aes    sha256 1024 Long_sentence_used_as_passphrase
       3 aes    sha256 1024 Long_sentence_used_as_passphrase
       3 aes    sha256 10240 Long_sentence_used_as_passphrase
       3 aes    sha384 1024 Long_sentence_used_as_passphrase
       3 aes    sha512 1024 Long_sentence_used_as_passphrase
       3 aes256 sha512 1024 Long_sentence_used_as_passphrase
       3 3des   sha512 1024 Long_sentence_used_as_passphrase
       ===>8===

    and finally using a proper utf-8 enabled shell, run:

       cat x.inp | while read mode cipher digest count pass dummy; do \
         ./gpg </dev/null -o /dev/null -c  --passphrase "$pass" \
           --s2k-mode $mode --s2k-digest $digest --s2k-count $count \
           --cipher-algo $cipher ; done >x.out
  */
  static struct {
    const char *p;   /* Passphrase.  */
    size_t plen;     /* Length of P. */
    int algo;
    int hashalgo;
    const char *salt;
    size_t saltlen;
    unsigned long c; /* Iterations.  */
    int dklen;       /* Requested key length.  */
    const char *dk;  /* Derived key.  */
    int disabled;
  } tv[] = {
    {
      "\x61", 1,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x0c\xc1\x75\xb9\xc0\xf1\xb6\xa8\x31\xc3\x99\xe2\x69\x77\x26\x61"
    },
    {
      "\x61\x62", 2,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x18\x7e\xf4\x43\x61\x22\xd1\xcc\x2f\x40\xdc\x2b\x92\xf0\xeb\xa0"
    },
    {
      "\x61\x62\x63", 3,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72"
    },
    {
      "\x61\x62\x63\x64", 4,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\xe2\xfc\x71\x4c\x47\x27\xee\x93\x95\xf3\x24\xcd\x2e\x7f\x33\x1f"
    },
    {
      "\x61\x62\x63\x64\x65", 5,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\xab\x56\xb4\xd9\x2b\x40\x71\x3a\xcc\x5a\xf8\x99\x85\xd4\xb7\x86"
    },
    {
      "\x61\x62\x63\x64\x65\x66", 6,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\xe8\x0b\x50\x17\x09\x89\x50\xfc\x58\xaa\xd8\x3c\x8c\x14\x97\x8e"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67", 7,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x7a\xc6\x6c\x0f\x14\x8d\xe9\x51\x9b\x8b\xd2\x64\x31\x2c\x4d\x64"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68", 8,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\xe8\xdc\x40\x81\xb1\x34\x34\xb4\x51\x89\xa7\x20\xb7\x7b\x68\x18"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x8a\xa9\x9b\x1f\x43\x9f\xf7\x12\x93\xe9\x53\x57\xba\xc6\xfd\x94"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f", 15,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x8a\x73\x19\xdb\xf6\x54\x4a\x74\x22\xc9\xe2\x54\x52\x58\x0e\xa5"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70", 16,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x1d\x64\xdc\xe2\x39\xc4\x43\x7b\x77\x36\x04\x1d\xb0\x89\xe1\xb9"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71", 17,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x9a\x8d\x98\x45\xa6\xb4\xd8\x2d\xfc\xb2\xc2\xe3\x51\x62\xc8\x30"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x35\x2a\xf0\xfc\xdf\xe9\xbb\x62\x16\xfc\x99\x9d\x8d\x58\x05\xcb"
    },
    {
      "\x57\x69\x74\x68\x5f\x75\x74\x66\x38\x5f\x75\x6d\x6c\x61\x75\x74"
      "\x73\x3a\xc3\xa4\xc3\xbc\xc3\x96\xc3\x9f", 26,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_MD5,
      NULL, 0,
      0,
      16,
      "\x21\xa4\xeb\xd8\xfd\xf0\x59\x25\xd1\x32\x31\xdb\xe7\xf2\x13\x5d"
    },
    {
      "\x61", 1,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x86\xf7\xe4\x37\xfa\xa5\xa7\xfc\xe1\x5d\x1d\xdc\xb9\xea\xea\xea"
    },
    {
      "\x61\x62", 2,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xda\x23\x61\x4e\x02\x46\x9a\x0d\x7c\x7b\xd1\xbd\xab\x5c\x9c\x47"
    },
    {
      "\x61\x62\x63", 3,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xa9\x99\x3e\x36\x47\x06\x81\x6a\xba\x3e\x25\x71\x78\x50\xc2\x6c"
    },
    {
      "\x61\x62\x63\x64", 4,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x81\xfe\x8b\xfe\x87\x57\x6c\x3e\xcb\x22\x42\x6f\x8e\x57\x84\x73"
    },
    {
      "\x61\x62\x63\x64\x65", 5,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x03\xde\x6c\x57\x0b\xfe\x24\xbf\xc3\x28\xcc\xd7\xca\x46\xb7\x6e"
    },
    {
      "\x61\x62\x63\x64\x65\x66", 6,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x1f\x8a\xc1\x0f\x23\xc5\xb5\xbc\x11\x67\xbd\xa8\x4b\x83\x3e\x5c"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67", 7,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x2f\xb5\xe1\x34\x19\xfc\x89\x24\x68\x65\xe7\xa3\x24\xf4\x76\xec"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68", 8,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x42\x5a\xf1\x2a\x07\x43\x50\x2b\x32\x2e\x93\xa0\x15\xbc\xf8\x68"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xc6\x3b\x19\xf1\xe4\xc8\xb5\xf7\x6b\x25\xc4\x9b\x8b\x87\xf5\x7d"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f", 15,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x29\x38\xdc\xc2\xe3\xaa\x77\x98\x7c\x7e\x5d\x4a\x0f\x26\x96\x67"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70", 16,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x14\xf3\x99\x52\x88\xac\xd1\x89\xe6\xe5\x0a\x7a\xf4\x7e\xe7\x09"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71", 17,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xd8\x3d\x62\x1f\xcd\x2d\x4d\x29\x85\x54\x70\x43\xa7\xa5\xfd\x4d"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72", 18,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xe3\x81\xfe\x42\xc5\x7e\x48\xa0\x82\x17\x86\x41\xef\xfd\x1c\xb9"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72\x73", 19,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x89\x3e\x69\xff\x01\x09\xf3\x45\x9c\x42\x43\x01\x3b\x3d\xe8\xb1"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72\x73\x74", 20,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x14\xa2\x3a\xd7\x0f\x2a\x5d\xd7\x25\x57\x5d\xe6\xc4\x3e\x1c\xdd"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72\x73\x74\x75", 21,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xec\xa9\x86\xb9\x5d\x58\x7f\x34\xd7\x1c\xa7\x75\x2a\x4e\x00\x10"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\x3e\x1b\x9a\x50\x7d\x6e\x9a\xd8\x93\x64\x96\x7a\x3f\xcb\x27\x3f"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      32,
      "\x3e\x1b\x9a\x50\x7d\x6e\x9a\xd8\x93\x64\x96\x7a\x3f\xcb\x27\x3f"
      "\xc3\x7b\x3a\xb2\xef\x4d\x68\xaa\x9c\xd7\xe4\x88\xee\xd1\x5e\x70"
    },
    {
      "\x57\x69\x74\x68\x5f\x75\x74\x66\x38\x5f\x75\x6d\x6c\x61\x75\x74"
      "\x73\x3a\xc3\xa4\xc3\xbc\xc3\x96\xc3\x9f", 26,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA1,
      NULL, 0,
      0,
      16,
      "\xe0\x4e\x1e\xe3\xad\x0b\x49\x7c\x7a\x5f\x37\x3b\x4d\x90\x3c\x2e"
    },
    {
      "\x61", 1,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x6d\x47\xe3\x68\x5d\x2c\x36\x16", 8,
      1024,
      16,
      "\x41\x9f\x48\x6e\xbf\xe6\xdd\x05\x9a\x72\x23\x17\x44\xd8\xd3\xf3"
    },
    {
      "\x61\x62", 2,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x7c\x34\x78\xfb\x28\x2d\x25\xc7", 8,
      1024,
      16,
      "\x0a\x9d\x09\x06\x43\x3d\x4f\xf9\x87\xd6\xf7\x48\x90\xde\xd1\x1c"
    },
    {
      "\x61\x62\x63", 3,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xc3\x16\x37\x2e\x27\xf6\x9f\x6f", 8,
      1024,
      16,
      "\xf8\x27\xa0\x07\xc6\xcb\xdd\xf1\xfe\x5c\x88\x3a\xfc\xcd\x84\x4d"
    },
    {
      "\x61\x62\x63\x64", 4,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xf0\x0c\x73\x38\xb7\xc3\xd5\x14", 8,
      1024,
      16,
      "\x9b\x5f\x26\xba\x52\x3b\xcd\xd9\xa5\x2a\xef\x3c\x03\x4d\xd1\x52"
    },
    {
      "\x61\x62\x63\x64\x65", 5,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xe1\x7d\xa2\x36\x09\x59\xee\xc5", 8,
      1024,
      16,
      "\x94\x9d\x5b\x1a\x5a\x66\x8c\xfa\x8f\x6f\x22\xaf\x8b\x60\x9f\xaf"
    },
    {
      "\x61\x62\x63\x64\x65\x66", 6,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xaf\xa7\x0c\x68\xdf\x7e\xaa\x27", 8,
      1024,
      16,
      "\xe5\x38\xf4\x39\x62\x27\xcd\xcc\x91\x37\x7f\x1b\xdc\x58\x64\x27"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67", 7,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x40\x57\xb2\x9d\x5f\xbb\x11\x4f", 8,
      1024,
      16,
      "\xad\xa2\x33\xd9\xdd\xe0\xfb\x94\x8e\xcc\xec\xcc\xb3\xa8\x3a\x9e"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68", 8,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x38\xf5\x65\xc5\x0f\x8c\x19\x61", 8,
      1024,
      16,
      "\xa0\xb0\x3e\x29\x76\xe6\x8f\xa0\xd8\x34\x8f\xa4\x2d\xfd\x65\xee"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xc3\xb7\x99\xcc\xda\x2d\x05\x7b", 8,
      1024,
      16,
      "\x27\x21\xc8\x99\x5f\xcf\x20\xeb\xf2\xd9\xff\x6a\x69\xff\xad\xe8"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f", 15,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x7d\xd8\x68\x8a\x1c\xc5\x47\x22", 8,
      1024,
      16,
      "\x0f\x96\x7a\x12\x23\x54\xf6\x92\x61\x67\x07\xb4\x68\x17\xb8\xaa"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70", 16,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x8a\x95\xd4\x88\x0b\xb8\xe9\x9d", 8,
      1024,
      16,
      "\xcc\xe4\xc8\x82\x53\x32\xf1\x93\x5a\x00\xd4\x7f\xd4\x46\xfa\x07"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71", 17,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xb5\x22\x48\xa6\xc4\xad\x74\x67", 8,
      1024,
      16,
      "\x0c\xe3\xe0\xee\x3d\x8f\x35\xd2\x35\x14\x14\x29\x0c\xf1\xe3\x34"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72", 18,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xac\x9f\x04\x63\x83\x0e\x3c\x95", 8,
      1024,
      16,
      "\x49\x0a\x04\x68\xa8\x2a\x43\x6f\xb9\x73\x94\xb4\x85\x9a\xaa\x0e"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72\x73", 19,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x03\x6f\x60\x30\x3a\x19\x61\x0d", 8,
      1024,
      16,
      "\x15\xe5\x9b\xbf\x1c\xf0\xbe\x74\x95\x1a\xb2\xc4\xda\x09\xcd\x99"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72\x73\x74", 20,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x51\x40\xa5\x57\xf5\x28\xfd\x03", 8,
      1024,
      16,
      "\xa6\xf2\x7e\x6b\x30\x4d\x8d\x67\xd4\xa2\x7f\xa2\x57\x27\xab\x96"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70"
      "\x71\x72\x73\x74\x75", 21,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x4c\xf1\x10\x11\x04\x70\xd3\x6e", 8,
      1024,
      16,
      "\x2c\x50\x79\x8d\x83\x23\xac\xd6\x22\x29\x37\xaf\x15\x0d\xdd\x8f"
    },
    {
      "\x57\x69\x74\x68\x5f\x75\x74\x66\x38\x5f\x75\x6d\x6c\x61\x75\x74"
      "\x73\x3a\xc3\xa4\xc3\xbc\xc3\x96\xc3\x9f", 26,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xfe\x3a\x25\xcb\x78\xef\xe1\x21", 8,
      1024,
      16,
      "\x2a\xb0\x53\x08\xf3\x2f\xd4\x6e\xeb\x01\x49\x5d\x87\xf6\x27\xf6"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x04\x97\xd0\x02\x6a\x44\x2d\xde", 8,
      1024,
      16,
      "\x57\xf5\x70\x41\xa0\x9b\x8c\x09\xca\x74\xa9\x22\xa5\x82\x2d\x17"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xdd\xf3\x31\x7c\xce\xf4\x81\x26", 8,
      10240,
      16,
      "\xc3\xdd\x01\x6d\xaf\xf6\x58\xc8\xd7\x79\xb4\x40\x00\xb5\xe8\x0b"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x95\xd6\x72\x4e\xfb\xe1\xc3\x1a", 8,
      102400,
      16,
      "\xf2\x3f\x36\x7f\xb4\x6a\xd0\x3a\x31\x9e\x65\x11\x8e\x2b\x99\x9b"
    },
    {
      "\x61", 1,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x6d\x69\x15\x18\xe4\x13\x42\x82", 8,
      1024,
      24,
      "\x28\x0c\x7e\xf2\x31\xf6\x1c\x6b\x5c\xef\x6a\xd5\x22\x64\x97\x91"
      "\xe3\x5e\x37\xfd\x50\xe2\xfc\x6c"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67", 7,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x9b\x76\x5e\x81\xde\x13\xdf\x15", 8,
      1024,
      24,
      "\x91\x1b\xa1\xc1\x7b\x4f\xc3\xb1\x80\x61\x26\x08\xbe\x53\xe6\x50"
      "\x40\x6f\x28\xed\xc6\xe6\x67\x55"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x7a\xac\xcc\x6e\x15\x56\xbd\xa1", 8,
      1024,
      24,
      "\xfa\x7e\x20\x07\xb6\x47\xb0\x09\x46\xb8\x38\xfb\xa1\xaf\xf7\x75"
      "\x2a\xfa\x77\x14\x06\x54\xcb\x34"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x1c\x68\xf8\xfb\x98\xf7\x8c\x39", 8,
      1024,
      24,
      "\xcb\x1e\x86\xf5\xe0\xe4\xfb\xbf\x71\x34\x99\x24\xf4\x39\x8c\xc2"
      "\x8e\x25\x1c\x4c\x96\x47\x22\xe8"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x10\xa9\x4e\xc1\xa5\xec\x17\x52", 8,
      1024,
      24,
      "\x0f\x83\xa2\x77\x92\xbb\xe4\x58\x68\xc5\xf2\x14\x6e\x6e\x2e\x6b"
      "\x98\x17\x70\x92\x07\x44\xe0\x51"
    },
    {
      "\x61", 1,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xef\x8f\x37\x61\x8f\xab\xae\x4f", 8,
      1024,
      32,
      "\x6d\x65\xae\x86\x23\x91\x39\x98\xec\x1c\x23\x44\xb6\x0d\xad\x32"
      "\x54\x46\xc7\x23\x26\xbb\xdf\x4b\x54\x6e\xd4\xc2\xfa\xc6\x17\x17"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67", 7,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xaa\xfb\xd9\x06\x7d\x7c\x40\xaf", 8,
      1024,
      32,
      "\x7d\x10\x54\x13\x3c\x43\x7a\xb3\x54\x1f\x38\xd4\x8f\x70\x0a\x09"
      "\xe2\xfa\xab\x97\x9a\x70\x16\xef\x66\x68\xca\x34\x2e\xce\xfa\x1f"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x58\x03\x4f\x56\x8b\x97\xd4\x98", 8,
      1024,
      32,
      "\xf7\x40\xb1\x25\x86\x0d\x35\x8f\x9f\x91\x2d\xce\x04\xee\x5a\x04"
      "\x9d\xbd\x44\x23\x4c\xa6\xbb\xab\xb0\xd0\x56\x82\xa9\xda\x47\x16"
    },
    {
      "\x61\x62\x63\x64\x65\x66\x67\x68\x69", 9,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\x5d\x41\x3d\xa3\xa7\xfc\x5d\x0c", 8,
      1024,
      32,
      "\x4c\x7a\x86\xed\x81\x8a\x94\x99\x7d\x4a\xc4\xf7\x1c\xf8\x08\xdb"
      "\x09\x35\xd9\xa3\x2d\x22\xde\x32\x2d\x74\x38\xe5\xc8\xf2\x50\x6e"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA1,
      "\xca\xa7\xdc\x59\xce\x31\xe7\x49", 8,
      1024,
      32,
      "\x67\xe9\xd6\x29\x49\x1c\xb6\xa0\x85\xe8\xf9\x8b\x85\x47\x3a\x7e"
      "\xa7\xee\x89\x52\x6f\x19\x00\x53\x93\x07\x0a\x8b\xb9\xa8\x86\x94"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_SIMPLE_S2K, GCRY_MD_SHA256,
      NULL, 0,
      0,
      16,
      "\x88\x36\x78\x6b\xd9\x5a\x62\xff\x47\xd3\xfb\x79\xc9\x08\x70\x56"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_SALTED_S2K, GCRY_MD_SHA256,
      "\x05\x8b\xfe\x31\xaa\xf3\x29\x11", 8,
      0,
      16,
      "\xb2\x42\xfe\x5e\x09\x02\xd9\x62\xb9\x35\xf3\xa8\x43\x80\x9f\xb1"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA256,
      "\xd3\x4a\xea\xc9\x97\x1b\xcc\x83", 8,
      1024,
      16,
      "\x35\x37\x99\x62\x07\x26\x68\x23\x05\x47\xb2\xa0\x0b\x2b\x2b\x8d"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA256,
      "\x5e\x71\xbd\x00\x5f\x96\xc4\x23", 8,
      10240,
      16,
      "\xa1\x6a\xee\xba\xde\x73\x25\x25\xd1\xab\xa0\xc5\x7e\xc6\x39\xa7"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA384,
      "\xc3\x08\xeb\x17\x62\x08\x89\xef", 8,
      1024,
      16,
      "\x9b\x7f\x0c\x81\x6f\x71\x59\x9b\xd5\xf6\xbf\x3a\x86\x20\x16\x33"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA512,
      "\xe6\x7d\x13\x6b\x39\xe3\x44\x05", 8,
      1024,
      16,
      "\xc8\xcd\x4b\xa4\xf3\xf1\xd5\xb0\x59\x06\xf0\xbb\x89\x34\x6a\xad"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA512,
      "\xed\x7d\x30\x47\xe4\xc3\xf8\xb6", 8,
      1024,
      32,
      "\x89\x7a\xef\x70\x97\xe7\x10\xdb\x75\xcc\x20\x22\xab\x7b\xf3\x05"
      "\x4b\xb6\x2e\x17\x11\x9f\xd6\xeb\xbf\xdf\x4d\x70\x59\xf0\xf9\xe5"
    },
    {
      "\x4c\x6f\x6e\x67\x5f\x73\x65\x6e\x74\x65\x6e\x63\x65\x5f\x75\x73"
      "\x65\x64\x5f\x61\x73\x5f\x70\x61\x73\x73\x70\x68\x72\x61\x73\x65", 32,
      GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA512,
      "\xbb\x1a\x45\x30\x68\x62\x6d\x63", 8,
      1024,
      24,
      "\xde\x5c\xb8\xd5\x75\xf6\xad\x69\x5b\xc9\xf6\x2f\xba\xeb\xfb\x36"
      "\x34\xf2\xb8\xee\x3b\x37\x21\xb7"
    }
  };
  int tvidx;
  gpg_error_t err;
  unsigned char outbuf[32];
  int i;

  for (tvidx=0; tvidx < DIM(tv); tvidx++)
    {
      if (tv[tvidx].disabled)
        continue;
      /* MD5 isn't supported in fips mode */
      if (in_fips_mode && tv[tvidx].hashalgo == GCRY_MD_MD5)
        continue;
      if (verbose)
        fprintf (stderr, "checking S2K test vector %d\n", tvidx);
      assert (tv[tvidx].dklen <= sizeof outbuf);
      err = gcry_kdf_derive (tv[tvidx].p, tv[tvidx].plen,
                             tv[tvidx].algo, tv[tvidx].hashalgo,
                             tv[tvidx].salt, tv[tvidx].saltlen,
                             tv[tvidx].c, tv[tvidx].dklen, outbuf);
      if (err)
        fail ("s2k test %d failed: %s\n", tvidx, gpg_strerror (err));
      else if (memcmp (outbuf, tv[tvidx].dk, tv[tvidx].dklen))
        {
          fail ("s2k test %d failed: mismatch\n", tvidx);
          fputs ("got:", stderr);
          for (i=0; i < tv[tvidx].dklen; i++)
            fprintf (stderr, " %02x", outbuf[i]);
          putc ('\n', stderr);
        }
    }
}


static void
check_pbkdf2 (void)
{
  /* Test vectors are from RFC-6070.  */
  static struct {
    const char *p;   /* Passphrase.  */
    size_t plen;     /* Length of P. */
    const char *salt;
    size_t saltlen;
    int hashalgo;
    unsigned long c; /* Iterations.  */
    int dklen;       /* Requested key length.  */
    const char *dk;  /* Derived key.  */
    int disabled;
  } tv[] = {
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_SHA1,
      1,
      20,
      "\x0c\x60\xc8\x0f\x96\x1f\x0e\x71\xf3\xa9"
      "\xb5\x24\xaf\x60\x12\x06\x2f\xe0\x37\xa6"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_SHA1,
      1,
      10, /* too short dklen for FIPS */
      "\x0c\x60\xc8\x0f\x96\x1f\x0e\x71\xf3\xa9"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_SHA1,
      2,
      20,
      "\xea\x6c\x01\x4d\xc7\x2d\x6f\x8c\xcd\x1e"
      "\xd9\x2a\xce\x1d\x41\xf0\xd8\xde\x89\x57"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_SHA1,
      4096,
      20,
      "\x4b\x00\x79\x01\xb7\x65\x48\x9a\xbe\xad"
      "\x49\xd9\x26\xf7\x21\xd0\x65\xa4\x29\xc1"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_SHA1,
      16777216,
      20,
      "\xee\xfe\x3d\x61\xcd\x4d\xa4\xe4\xe9\x94"
      "\x5b\x3d\x6b\xa2\x15\x8c\x26\x34\xe9\x84",
      1 /* This test takes too long.  */
    },
    {
      "passwordPASSWORDpassword", 24,
      "saltSALTsaltSALTsaltSALTsaltSALTsalt", 36,
      GCRY_MD_SHA1,
      4096,
      25,
      "\x3d\x2e\xec\x4f\xe4\x1c\x84\x9b\x80\xc8"
      "\xd8\x36\x62\xc0\xe4\x4a\x8b\x29\x1a\x96"
      "\x4c\xf2\xf0\x70\x38"
    },
    {
      "pass\0word", 9,
      "sa\0lt", 5,
      GCRY_MD_SHA1,
      4096,
      16,
      "\x56\xfa\x6a\xa7\x55\x48\x09\x9d\xcc\x37"
      "\xd7\xf0\x34\x25\xe0\xc3"
    },
    { /* empty password test, not in RFC-6070 */
      "", 0,
      "salt", 4,
      GCRY_MD_SHA1,
      2,
      20,
      "\x13\x3a\x4c\xe8\x37\xb4\xd2\x52\x1e\xe2"
      "\xbf\x03\xe1\x1c\x71\xca\x79\x4e\x07\x97"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_GOSTR3411_CP,
      1,
      32,
      "\x73\x14\xe7\xc0\x4f\xb2\xe6\x62\xc5\x43\x67\x42\x53\xf6\x8b\xd0"
      "\xb7\x34\x45\xd0\x7f\x24\x1b\xed\x87\x28\x82\xda\x21\x66\x2d\x58"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_GOSTR3411_CP,
      2,
      32,
      "\x99\x0d\xfa\x2b\xd9\x65\x63\x9b\xa4\x8b\x07\xb7\x92\x77\x5d\xf7"
      "\x9f\x2d\xb3\x4f\xef\x25\xf2\x74\x37\x88\x72\xfe\xd7\xed\x1b\xb3"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_GOSTR3411_CP,
      4096,
      32,
      "\x1f\x18\x29\xa9\x4b\xdf\xf5\xbe\x10\xd0\xae\xb3\x6a\xf4\x98\xe7"
      "\xa9\x74\x67\xf3\xb3\x11\x16\xa5\xa7\xc1\xaf\xff\x9d\xea\xda\xfe"
    },
    /* { -- takes too long (4-5 min) to calculate
      "password", 8,
      "salt", 4,
      GCRY_MD_GOSTR3411_CP,
      16777216,
      32,
      "\xa5\x7a\xe5\xa6\x08\x83\x96\xd1\x20\x85\x0c\x5c\x09\xde\x0a\x52"
      "\x51\x00\x93\x8a\x59\xb1\xb5\xc3\xf7\x81\x09\x10\xd0\x5f\xcd\x97"
    }, */
    {
      "passwordPASSWORDpassword", 24,
      "saltSALTsaltSALTsaltSALTsaltSALTsalt", 36,
      GCRY_MD_GOSTR3411_CP,
      4096,
      40,
      "\x78\x83\x58\xc6\x9c\xb2\xdb\xe2\x51\xa7\xbb\x17\xd5\xf4\x24\x1f"
      "\x26\x5a\x79\x2a\x35\xbe\xcd\xe8\xd5\x6f\x32\x6b\x49\xc8\x50\x47"
      "\xb7\x63\x8a\xcb\x47\x64\xb1\xfd"
    },
    {
      "pass\0word", 9,
      "sa\0lt", 5,
      GCRY_MD_GOSTR3411_CP,
      4096,
      20,
      "\x43\xe0\x6c\x55\x90\xb0\x8c\x02\x25\x24"
      "\x23\x73\x12\x7e\xdf\x9c\x8e\x9c\x32\x91"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_STRIBOG512,
      1,
      64,
      "\x64\x77\x0a\xf7\xf7\x48\xc3\xb1\xc9\xac\x83\x1d\xbc\xfd\x85\xc2"
      "\x61\x11\xb3\x0a\x8a\x65\x7d\xdc\x30\x56\xb8\x0c\xa7\x3e\x04\x0d"
      "\x28\x54\xfd\x36\x81\x1f\x6d\x82\x5c\xc4\xab\x66\xec\x0a\x68\xa4"
      "\x90\xa9\xe5\xcf\x51\x56\xb3\xa2\xb7\xee\xcd\xdb\xf9\xa1\x6b\x47"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_STRIBOG512,
      2,
      64,
      "\x5a\x58\x5b\xaf\xdf\xbb\x6e\x88\x30\xd6\xd6\x8a\xa3\xb4\x3a\xc0"
      "\x0d\x2e\x4a\xeb\xce\x01\xc9\xb3\x1c\x2c\xae\xd5\x6f\x02\x36\xd4"
      "\xd3\x4b\x2b\x8f\xbd\x2c\x4e\x89\xd5\x4d\x46\xf5\x0e\x47\xd4\x5b"
      "\xba\xc3\x01\x57\x17\x43\x11\x9e\x8d\x3c\x42\xba\x66\xd3\x48\xde"
    },
    {
      "password", 8,
      "salt", 4,
      GCRY_MD_STRIBOG512,
      4096,
      64,
      "\xe5\x2d\xeb\x9a\x2d\x2a\xaf\xf4\xe2\xac\x9d\x47\xa4\x1f\x34\xc2"
      "\x03\x76\x59\x1c\x67\x80\x7f\x04\x77\xe3\x25\x49\xdc\x34\x1b\xc7"
      "\x86\x7c\x09\x84\x1b\x6d\x58\xe2\x9d\x03\x47\xc9\x96\x30\x1d\x55"
      "\xdf\x0d\x34\xe4\x7c\xf6\x8f\x4e\x3c\x2c\xda\xf1\xd9\xab\x86\xc3"
    },
    /* { -- takes toooo long
      "password", 8,
      "salt", 4,
      GCRY_MD_STRIBOG512,
      16777216,
      64,
      "\x49\xe4\x84\x3b\xba\x76\xe3\x00\xaf\xe2\x4c\x4d\x23\xdc\x73\x92"
      "\xde\xf1\x2f\x2c\x0e\x24\x41\x72\x36\x7c\xd7\x0a\x89\x82\xac\x36"
      "\x1a\xdb\x60\x1c\x7e\x2a\x31\x4e\x8c\xb7\xb1\xe9\xdf\x84\x0e\x36"
      "\xab\x56\x15\xbe\x5d\x74\x2b\x6c\xf2\x03\xfb\x55\xfd\xc4\x80\x71"
    }, */
    {
      "passwordPASSWORDpassword", 24,
      "saltSALTsaltSALTsaltSALTsaltSALTsalt", 36,
      GCRY_MD_STRIBOG512,
      4096,
      100,
      "\xb2\xd8\xf1\x24\x5f\xc4\xd2\x92\x74\x80\x20\x57\xe4\xb5\x4e\x0a"
      "\x07\x53\xaa\x22\xfc\x53\x76\x0b\x30\x1c\xf0\x08\x67\x9e\x58\xfe"
      "\x4b\xee\x9a\xdd\xca\xe9\x9b\xa2\xb0\xb2\x0f\x43\x1a\x9c\x5e\x50"
      "\xf3\x95\xc8\x93\x87\xd0\x94\x5a\xed\xec\xa6\xeb\x40\x15\xdf\xc2"
      "\xbd\x24\x21\xee\x9b\xb7\x11\x83\xba\x88\x2c\xee\xbf\xef\x25\x9f"
      "\x33\xf9\xe2\x7d\xc6\x17\x8c\xb8\x9d\xc3\x74\x28\xcf\x9c\xc5\x2a"
      "\x2b\xaa\x2d\x3a"
    },
    {
      "pass\0word", 9,
      "sa\0lt", 5,
      GCRY_MD_STRIBOG512,
      4096,
      64,
      "\x50\xdf\x06\x28\x85\xb6\x98\x01\xa3\xc1\x02\x48\xeb\x0a\x27\xab"
      "\x6e\x52\x2f\xfe\xb2\x0c\x99\x1c\x66\x0f\x00\x14\x75\xd7\x3a\x4e"
      "\x16\x7f\x78\x2c\x18\xe9\x7e\x92\x97\x6d\x9c\x1d\x97\x08\x31\xea"
      "\x78\xcc\xb8\x79\xf6\x70\x68\xcd\xac\x19\x10\x74\x08\x44\xe8\x30"
    }
  };
  int tvidx;
  gpg_error_t err;
  unsigned char outbuf[100];
  int i;

  for (tvidx=0; tvidx < DIM(tv); tvidx++)
    {
      if (tv[tvidx].disabled)
        continue;
      if (verbose)
        fprintf (stderr, "checking PBKDF2 test vector %d algo %d\n", tvidx,
                 tv[tvidx].hashalgo);
      assert (tv[tvidx].dklen <= sizeof outbuf);
      err = gcry_kdf_derive (tv[tvidx].p, tv[tvidx].plen,
                             GCRY_KDF_PBKDF2, tv[tvidx].hashalgo,
                             tv[tvidx].salt, tv[tvidx].saltlen,
                             tv[tvidx].c, tv[tvidx].dklen, outbuf);
      if (in_fips_mode && tvidx > 7)
        {
          if (!err)
            fail ("pbkdf2 test %d unexpectedly passed in FIPS mode: %s\n",
                  tvidx, gpg_strerror (err));
          continue;
        }
      if (err)
        {
          if (in_fips_mode && (tv[tvidx].plen < 14 || tv[tvidx].dklen < 14))
            {
              if (verbose)
                fprintf (stderr,
                         "  shorter key (%u) rejected correctly in fips mode\n",
                         (unsigned int)tv[tvidx].plen);
            }
          else
            fail ("pbkdf2 test %d failed: %s\n", tvidx, gpg_strerror (err));
        }
      else if (memcmp (outbuf, tv[tvidx].dk, tv[tvidx].dklen))
        {
          fail ("pbkdf2 test %d failed: mismatch\n", tvidx);
          fputs ("got:", stderr);
          for (i=0; i < tv[tvidx].dklen; i++)
            fprintf (stderr, " %02x", outbuf[i]);
          putc ('\n', stderr);
        }
    }
}


static void
check_scrypt (void)
{
  /* Test vectors are from draft-josefsson-scrypt-kdf-01.  */
  static struct {
    const char *p;        /* Passphrase.  */
    size_t plen;          /* Length of P. */
    const char *salt;
    size_t saltlen;
    int parm_n;           /* CPU/memory cost.  */
    int parm_r;           /* blocksize */
    unsigned long parm_p; /* parallelization. */
    int dklen;            /* Requested key length.  */
    const char *dk;       /* Derived key.  */
    int disabled;
  } tv[] = {
    {
      "", 0,
      "", 0,
      16,
      1,
      1,
      64,
      "\x77\xd6\x57\x62\x38\x65\x7b\x20\x3b\x19\xca\x42\xc1\x8a\x04\x97"
      "\xf1\x6b\x48\x44\xe3\x07\x4a\xe8\xdf\xdf\xfa\x3f\xed\xe2\x14\x42"
      "\xfc\xd0\x06\x9d\xed\x09\x48\xf8\x32\x6a\x75\x3a\x0f\xc8\x1f\x17"
      "\xe8\xd3\xe0\xfb\x2e\x0d\x36\x28\xcf\x35\xe2\x0c\x38\xd1\x89\x06"
    },
    {
      "password", 8,
      "NaCl", 4,
      1024,
      8,
      16,
      64,
      "\xfd\xba\xbe\x1c\x9d\x34\x72\x00\x78\x56\xe7\x19\x0d\x01\xe9\xfe"
      "\x7c\x6a\xd7\xcb\xc8\x23\x78\x30\xe7\x73\x76\x63\x4b\x37\x31\x62"
      "\x2e\xaf\x30\xd9\x2e\x22\xa3\x88\x6f\xf1\x09\x27\x9d\x98\x30\xda"
      "\xc7\x27\xaf\xb9\x4a\x83\xee\x6d\x83\x60\xcb\xdf\xa2\xcc\x06\x40"
    },
    {
      "pleaseletmein", 13,
      "SodiumChloride", 14,
      16384,
      8,
      1,
      64,
      "\x70\x23\xbd\xcb\x3a\xfd\x73\x48\x46\x1c\x06\xcd\x81\xfd\x38\xeb"
      "\xfd\xa8\xfb\xba\x90\x4f\x8e\x3e\xa9\xb5\x43\xf6\x54\x5d\xa1\xf2"
      "\xd5\x43\x29\x55\x61\x3f\x0f\xcf\x62\xd4\x97\x05\x24\x2a\x9a\xf9"
      "\xe6\x1e\x85\xdc\x0d\x65\x1e\x40\xdf\xcf\x01\x7b\x45\x57\x58\x87"
    },
    {
      "pleaseletmein", 13,
      "SodiumChloride", 14,
      1048576,
      8,
      1,
      64,
      "\x21\x01\xcb\x9b\x6a\x51\x1a\xae\xad\xdb\xbe\x09\xcf\x70\xf8\x81"
      "\xec\x56\x8d\x57\x4a\x2f\xfd\x4d\xab\xe5\xee\x98\x20\xad\xaa\x47"
      "\x8e\x56\xfd\x8f\x4b\xa5\xd0\x9f\xfa\x1c\x6d\x92\x7c\x40\xf4\xc3"
      "\x37\x30\x40\x49\xe8\xa9\x52\xfb\xcb\xf4\x5c\x6f\xa7\x7a\x41\xa4",
      2 /* Only in debug mode.  */
    }
  };
  int tvidx;
  gpg_error_t err;
  unsigned char outbuf[64];
  int i;

  for (tvidx=0; tvidx < DIM(tv); tvidx++)
    {
      if (tv[tvidx].disabled && !(tv[tvidx].disabled == 2 && debug))
        continue;
      if (verbose)
        fprintf (stderr, "checking SCRYPT test vector %d\n", tvidx);
      assert (tv[tvidx].dklen <= sizeof outbuf);
      err = gcry_kdf_derive (tv[tvidx].p, tv[tvidx].plen,
                             tv[tvidx].parm_r == 1 ? 41 : GCRY_KDF_SCRYPT,
                             tv[tvidx].parm_n,
                             tv[tvidx].salt, tv[tvidx].saltlen,
                             tv[tvidx].parm_p, tv[tvidx].dklen, outbuf);
      if (err)
        {
          if (in_fips_mode && tv[tvidx].plen < 14)
            {
              if (verbose)
                fprintf (stderr,
                         "  shorter key (%u) rejected correctly in fips mode\n",
                         (unsigned int)tv[tvidx].plen);
            }
          else
            fail ("scrypt test %d failed: %s\n", tvidx, gpg_strerror (err));
        }
      else if (memcmp (outbuf, tv[tvidx].dk, tv[tvidx].dklen))
        {
          fail ("scrypt test %d failed: mismatch\n", tvidx);
          fputs ("got:", stderr);
          for (i=0; i < tv[tvidx].dklen; i++)
            fprintf (stderr, " %02x", outbuf[i]);
          putc ('\n', stderr);
        }
    }
}


#ifdef HAVE_PTHREAD
#include <pthread.h>

#define MAX_THREADS 8

struct user_defined_threads_ctx
{
  int oldest_thread_idx;
  int next_thread_idx;
  int num_threads_running;
  pthread_attr_t attr;
  pthread_t thread[MAX_THREADS];
  struct job_thread_param
  {
    gcry_kdf_job_fn_t job;
    void *priv;
  } work[MAX_THREADS];
};

static void *
job_thread (void *p)
{
  struct job_thread_param *param = p;
  param->job (param->priv);
  pthread_exit (NULL);
}

static int
wait_all_jobs_completion (void *jobs_context);

static int
pthread_jobs_launch_job (void *jobs_context, gcry_kdf_job_fn_t job,
			 void *job_priv)
{
  struct user_defined_threads_ctx *ctx = jobs_context;
  int ret;

  if (ctx->next_thread_idx == ctx->oldest_thread_idx)
    {
      assert (ctx->num_threads_running == MAX_THREADS);
      /* thread limit reached, join a thread */
      ret = pthread_join (ctx->thread[ctx->oldest_thread_idx], NULL);
      if (ret)
	return -1;
      ctx->oldest_thread_idx = (ctx->oldest_thread_idx + 1) % MAX_THREADS;
      ctx->num_threads_running--;
    }

  ctx->work[ctx->next_thread_idx].job = job;
  ctx->work[ctx->next_thread_idx].priv = job_priv;
  ret = pthread_create (&ctx->thread[ctx->next_thread_idx], &ctx->attr,
			job_thread, &ctx->work[ctx->next_thread_idx]);
  if (ret)
    {
      /* could not create new thread. */
      (void)wait_all_jobs_completion (jobs_context);
      return -1;
    }

  if (ctx->oldest_thread_idx < 0)
    ctx->oldest_thread_idx = ctx->next_thread_idx;
  ctx->next_thread_idx = (ctx->next_thread_idx + 1) % MAX_THREADS;
  ctx->num_threads_running++;
  return 0;
}

static int
wait_all_jobs_completion (void *jobs_context)
{
  struct user_defined_threads_ctx *ctx = jobs_context;
  int i, idx;
  int ret;

  for (i = 0; i < ctx->num_threads_running; i++)
    {
      idx = (ctx->oldest_thread_idx + i) % MAX_THREADS;
      ret = pthread_join (ctx->thread[idx], NULL);
      if (ret)
	return -1;
    }

  /* reset context for next round of parallel work */
  ctx->num_threads_running = 0;
  ctx->oldest_thread_idx = -1;
  ctx->next_thread_idx = 0;

  return 0;
}
#endif

static gcry_error_t
my_kdf_derive (int parallel,
               int algo, int subalgo,
               const unsigned long *params, unsigned int paramslen,
               const unsigned char *pass, size_t passlen,
               const unsigned char *salt, size_t saltlen,
               const unsigned char *key, size_t keylen,
               const unsigned char *ad, size_t adlen,
               size_t outlen, unsigned char *out)
{
  gcry_error_t err;
  gcry_kdf_hd_t hd;

  (void)parallel;

  err = gcry_kdf_open (&hd, algo, subalgo, params, paramslen,
                       pass, passlen, salt, saltlen, key, keylen,
                       ad, adlen);
  if (err)
    return err;

#ifdef HAVE_PTHREAD
  if (parallel)
    {
      struct user_defined_threads_ctx jobs_context;
      const gcry_kdf_thread_ops_t ops =
      {
        &jobs_context,
        pthread_jobs_launch_job,
        wait_all_jobs_completion
      };

      memset (&jobs_context, 0, sizeof (struct user_defined_threads_ctx));
      jobs_context.oldest_thread_idx = -1;

      if (pthread_attr_init (&jobs_context.attr))
	{
          err = gpg_error_from_syserror ();
	  gcry_kdf_close (hd);
	  return err;
	}

      if (pthread_attr_setdetachstate (&jobs_context.attr,
                                       PTHREAD_CREATE_JOINABLE))
	{
          err = gpg_error_from_syserror ();
	  pthread_attr_destroy (&jobs_context.attr);
	  gcry_kdf_close (hd);
	  return err;
	}

      err = gcry_kdf_compute (hd, &ops);

      pthread_attr_destroy (&jobs_context. attr);
    }
  else
#endif
    {
      err = gcry_kdf_compute (hd, NULL);
    }

  if (!err)
    err = gcry_kdf_final (hd, outlen, out);

  gcry_kdf_close (hd);
  return err;
}


static void
check_argon2 (void)
{
  gcry_error_t err;
  const unsigned long param[4] = { 32, 3, 32, 4 };
  const unsigned char pass[32] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };
  const unsigned char salt[16] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  };
  const unsigned char key[8] = { 3, 3, 3, 3, 3, 3, 3, 3 };
  const unsigned char ad[12] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
  unsigned char out[32];
  unsigned char expected[3][32] = {
    {  /* GCRY_KDF_ARGON2D */
      0x51, 0x2b, 0x39, 0x1b, 0x6f, 0x11, 0x62, 0x97,
      0x53, 0x71, 0xd3, 0x09, 0x19, 0x73, 0x42, 0x94,
      0xf8, 0x68, 0xe3, 0xbe, 0x39, 0x84, 0xf3, 0xc1,
      0xa1, 0x3a, 0x4d, 0xb9, 0xfa, 0xbe, 0x4a, 0xcb
    },
    { /* GCRY_KDF_ARGON2I */
      0xc8, 0x14, 0xd9, 0xd1, 0xdc, 0x7f, 0x37, 0xaa,
      0x13, 0xf0, 0xd7, 0x7f, 0x24, 0x94, 0xbd, 0xa1,
      0xc8, 0xde, 0x6b, 0x01, 0x6d, 0xd3, 0x88, 0xd2,
      0x99, 0x52, 0xa4, 0xc4, 0x67, 0x2b, 0x6c, 0xe8
    },
    { /* GCRY_KDF_ARGON2ID */
      0x0d, 0x64, 0x0d, 0xf5, 0x8d, 0x78, 0x76, 0x6c,
      0x08, 0xc0, 0x37, 0xa3, 0x4a, 0x8b, 0x53, 0xc9,
      0xd0, 0x1e, 0xf0, 0x45, 0x2d, 0x75, 0xb6, 0x5e,
      0xb5, 0x25, 0x20, 0xe9, 0x6b, 0x01, 0xe6, 0x59
    }
  };
  int i;
  int subalgo = GCRY_KDF_ARGON2D;
  int count = 0;

 again:

  if (verbose)
    fprintf (stderr, "checking ARGON2 test vector %d\n", count);

  err = my_kdf_derive (0,
                       GCRY_KDF_ARGON2, subalgo, param, 4,
                       pass, 32, salt, 16, key, 8, ad, 12,
                       32, out);
  if (err)
    fail ("argon2 test %d failed: %s\n", 0, gpg_strerror (err));
  else if (memcmp (out, expected[count], 32))
    {
      fail ("argon2 test %d failed: mismatch\n", 0);
      fputs ("got:", stderr);
      for (i=0; i < 32; i++)
        fprintf (stderr, " %02x", out[i]);
      putc ('\n', stderr);
    }

#ifdef HAVE_PTHREAD
  err = my_kdf_derive (1,
                       GCRY_KDF_ARGON2, subalgo, param, 4,
                       pass, 32, salt, 16, key, 8, ad, 12,
                       32, out);
  if (err)
    fail ("argon2 test %d failed: %s\n", 1, gpg_strerror (err));
  else if (memcmp (out, expected[count], 32))
    {
      fail ("argon2 test %d failed: mismatch\n", 1);
      fputs ("got:", stderr);
      for (i=0; i < 32; i++)
        fprintf (stderr, " %02x", out[i]);
      putc ('\n', stderr);
    }
#endif

  /* Next algo */
  if (subalgo == GCRY_KDF_ARGON2D)
    subalgo = GCRY_KDF_ARGON2I;
  else if (subalgo == GCRY_KDF_ARGON2I)
    subalgo = GCRY_KDF_ARGON2ID;

  count++;
  if (count < 3)
    goto again;
}


static void
check_fips_indicators (void)
{
  enum gcry_kdf_algos fips_kdf_algos[] = {
    GCRY_KDF_PBKDF2,
  };
  enum gcry_kdf_algos kdf_algos[] = {
    GCRY_KDF_SIMPLE_S2K,
    GCRY_KDF_SALTED_S2K,
    GCRY_KDF_ITERSALTED_S2K,
    GCRY_KDF_PBKDF1,
    GCRY_KDF_PBKDF2,
    GCRY_KDF_SCRYPT,
    GCRY_KDF_ARGON2
  };
  size_t i, j;

  for (i = 0; i < sizeof(kdf_algos) / sizeof(*kdf_algos); i++)
    {
      int is_fips_kdf_algo = 0;
      gcry_error_t err = gcry_control (GCRYCTL_FIPS_SERVICE_INDICATOR_KDF, kdf_algos[i]);

      if (verbose)
        fprintf (stderr, "checking FIPS indicator for KDF %d: %s\n",
                 kdf_algos[i], gcry_strerror (err));

      for (j = 0; j < sizeof(fips_kdf_algos) / sizeof(*fips_kdf_algos); j++)
        {
          if (kdf_algos[i] == fips_kdf_algos[j])
            {
              is_fips_kdf_algo = 1;
              break;
            }
        }

      switch (err & GPG_ERR_CODE_MASK)
        {
          case GPG_ERR_NO_ERROR:
            if (!is_fips_kdf_algo)
              fail ("KDF algorithm %d is marked as approved by"
                    " GCRYCTL_FIPS_SERVICE_INDICATOR_KDF, but only PBKDF2 should"
                    " be marked as approved.", kdf_algos[i]);
            break;
          case GPG_ERR_NOT_SUPPORTED:
            if (is_fips_kdf_algo)
              fail ("KDF algorithm %d is marked as not approved by"
                    " GCRYCTL_FIPS_SERVICE_INDICATOR_KDF, but it should be"
                    " approved", kdf_algos[i]);
            break;
          default:
            fail ("Unexpected error '%s' (%d) returned by"
                  " GCRYCTL_FIPS_SERVICE_INDICATOR_KDF for KDF algorithm %d",
                  gcry_strerror (err), err, kdf_algos[i]);
        }
    }
}


int
main (int argc, char **argv)
{
  int last_argc = -1;
  unsigned long s2kcount = 0;

  if (argc)
    { argc--; argv++; }

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--help"))
        {
          fputs ("usage: t-kdf [options]"
                 "Options:\n"
                 " --verbose    print timinigs etc.\n"
                 " --debug      flyswatter\n"
                 " --s2k        print the time needed for S2K\n",
                 stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose += 2;
          debug++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--s2k"))
        {
          s2kcount = 1;
          argc--; argv++;
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'\n", *argv);
    }

  if (s2kcount)
    {
      if (argc != 1)
        die ("usage: t-kdf --s2k S2KCOUNT\n");
      s2kcount = strtoul (*argv, NULL, 10);
      if (!s2kcount)
        die ("t-kdf: S2KCOUNT must be positive\n");
    }

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  if (!in_fips_mode)
    xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));

  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));

  if (s2kcount)
    bench_s2k (s2kcount);
  else
    {
      check_openpgp ();
      check_pbkdf2 ();
      check_scrypt ();
      check_argon2 ();
      if (in_fips_mode)
        check_fips_indicators();
    }

  return error_count ? 1 : 0;
}
