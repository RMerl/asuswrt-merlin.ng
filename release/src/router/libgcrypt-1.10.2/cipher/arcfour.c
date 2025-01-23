/* arcfour.c  -  The arcfour stream cipher
 *	Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * For a description of the algorithm, see:
 *   Bruce Schneier: Applied Cryptography. John Wiley & Sons, 1996.
 *   ISBN 0-471-11709-9. Pages 397 ff.
 */


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "cipher-internal.h"

/* USE_AMD64_ASM indicates whether to use AMD64 assembly code. */
#undef USE_AMD64_ASM
#if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AMD64_ASM 1
#endif

static const char *selftest(void);

#ifdef USE_AMD64_ASM

typedef struct {
    u32 sbox[256];
    u32 idx_i, idx_j;
} ARCFOUR_context;

void _gcry_arcfour_amd64(void *key, size_t len, const byte *indata,
			 byte *outdata);

static void
encrypt_stream (void *context,
                byte *outbuf, const byte *inbuf, size_t length)
{
  _gcry_arcfour_amd64 (context, length, inbuf, outbuf );
}

#else /*!USE_AMD64_ASM*/

typedef struct {
    byte sbox[256];
    int idx_i, idx_j;
} ARCFOUR_context;

static void
do_encrypt_stream( ARCFOUR_context *ctx,
		   byte *outbuf, const byte *inbuf, size_t length )
{
#ifndef __i386__
  register unsigned int i = ctx->idx_i;
  register byte j = ctx->idx_j;
  register byte *sbox = ctx->sbox;
  register byte t, u;

  while ( length-- )
    {
      i++;
      t = sbox[(byte)i];
      j += t;
      u = sbox[j];
      sbox[(byte)i] = u;
      u += t;
      sbox[j] = t;
      *outbuf++ = sbox[u] ^ *inbuf++;
    }

  ctx->idx_i = (byte)i;
  ctx->idx_j = (byte)j;
#else /*__i386__*/
  /* Old implementation of arcfour is faster on i386 than the version above.
   * This is because version above increases register pressure which on i386
   * would push some of the variables to memory/stack.  Therefore keep this
   * version for i386 to avoid regressing performance.  */
  register int i = ctx->idx_i;
  register int j = ctx->idx_j;
  register byte *sbox = ctx->sbox;
  register int t;

  while ( length-- )
    {
      i++;
      i = i & 255; /* The and-op seems to be faster than the mod-op. */
      j += sbox[i];
      j &= 255;
      t = sbox[i]; sbox[i] = sbox[j]; sbox[j] = t;
      *outbuf++ = *inbuf++ ^ sbox[(sbox[i] + sbox[j]) & 255];
    }

  ctx->idx_i = i;
  ctx->idx_j = j;
#endif
}

static void
encrypt_stream (void *context,
                byte *outbuf, const byte *inbuf, size_t length)
{
  ARCFOUR_context *ctx = (ARCFOUR_context *) context;
  do_encrypt_stream (ctx, outbuf, inbuf, length );
  _gcry_burn_stack (64);
}

#endif /*!USE_AMD64_ASM*/


static gcry_err_code_t
do_arcfour_setkey (void *context, const byte *key, unsigned int keylen)
{
  static int initialized;
  static const char* selftest_failed;
  int i, j;
  byte karr[256];
  ARCFOUR_context *ctx = (ARCFOUR_context *) context;

  if (!initialized )
    {
      initialized = 1;
      selftest_failed = selftest();
      if( selftest_failed )
        log_error ("ARCFOUR selftest failed (%s)\n", selftest_failed );
    }
  if( selftest_failed )
    return GPG_ERR_SELFTEST_FAILED;

  if( keylen < 40/8 ) /* we want at least 40 bits */
    return GPG_ERR_INV_KEYLEN;

  ctx->idx_i = ctx->idx_j = 0;
  for (i=0; i < 256; i++ )
    ctx->sbox[i] = i;
  for (i=j=0; i < 256; i++,j++ )
    {
      if (j >= keylen)
        j = 0;
      karr[i] = key[j];
    }
  for (i=j=0; i < 256; i++ )
    {
      int t;
      j = (j + ctx->sbox[i] + karr[i]) & 255;
      t = ctx->sbox[i];
      ctx->sbox[i] = ctx->sbox[j];
      ctx->sbox[j] = t;
    }
  wipememory( karr, sizeof(karr) );

  return GPG_ERR_NO_ERROR;
}

static gcry_err_code_t
arcfour_setkey ( void *context, const byte *key, unsigned int keylen,
                 cipher_bulk_ops_t *bulk_ops )
{
  ARCFOUR_context *ctx = (ARCFOUR_context *) context;
  gcry_err_code_t rc = do_arcfour_setkey (ctx, key, keylen );
  (void)bulk_ops;
  return rc;
}


static const char*
selftest(void)
{
  ARCFOUR_context ctx;
  byte scratch[16];

  /* Test vector from Cryptlib labeled there: "from the
     State/Commerce Department". */
  static const byte key_1[] =
    { 0x61, 0x8A, 0x63, 0xD2, 0xFB };
  static const byte plaintext_1[] =
    { 0xDC, 0xEE, 0x4C, 0xF9, 0x2C };
  static const byte ciphertext_1[] =
    { 0xF1, 0x38, 0x29, 0xC9, 0xDE };

  arcfour_setkey( &ctx, key_1, sizeof(key_1), NULL);
  encrypt_stream( &ctx, scratch, plaintext_1, sizeof(plaintext_1));
  if ( memcmp (scratch, ciphertext_1, sizeof (ciphertext_1)))
    return "Arcfour encryption test 1 failed.";
  arcfour_setkey( &ctx, key_1, sizeof(key_1), NULL);
  encrypt_stream(&ctx, scratch, scratch, sizeof(plaintext_1)); /* decrypt */
  if ( memcmp (scratch, plaintext_1, sizeof (plaintext_1)))
    return "Arcfour decryption test 1 failed.";
  return NULL;
}


gcry_cipher_spec_t _gcry_cipher_spec_arcfour =
  {
    GCRY_CIPHER_ARCFOUR, {0, 0},
    "ARCFOUR", NULL, NULL, 1, 128, sizeof (ARCFOUR_context),
    arcfour_setkey, NULL, NULL, encrypt_stream, encrypt_stream,
  };
