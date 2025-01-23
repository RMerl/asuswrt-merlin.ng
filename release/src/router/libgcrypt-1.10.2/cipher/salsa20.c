/* salsa20.c  -  Bernstein's Salsa20 cipher
 * Copyright (C) 2012 Simon Josefsson, Niels Möller
 * Copyright (C) 2013 g10 Code GmbH
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
 *
 * For a description of the algorithm, see:
 *   http://cr.yp.to/snuffle/spec.pdf
 *   http://cr.yp.to/snuffle/design.pdf
 */

/* The code is based on the code in Nettle
   (git commit id 9d2d8ddaee35b91a4e1a32ae77cba04bea3480e7)
   which in turn is based on
   salsa20-ref.c version 20051118
   D. J. Bernstein
   Public domain.
*/


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "cipher-internal.h"


/* USE_AMD64 indicates whether to compile with AMD64 code. */
#undef USE_AMD64
#if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AMD64 1
#endif

/* USE_ARM_NEON_ASM indicates whether to enable ARM NEON assembly code. */
#undef USE_ARM_NEON_ASM
#ifdef ENABLE_NEON_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_NEON)
#  define USE_ARM_NEON_ASM 1
# endif
#endif /*ENABLE_NEON_SUPPORT*/


#define SALSA20_MIN_KEY_SIZE 16  /* Bytes.  */
#define SALSA20_MAX_KEY_SIZE 32  /* Bytes.  */
#define SALSA20_BLOCK_SIZE   64  /* Bytes.  */
#define SALSA20_IV_SIZE       8  /* Bytes.  */
#define SALSA20_INPUT_LENGTH 16  /* Bytes.  */

/* Number of rounds.  The standard uses 20 rounds.  In any case the
   number of rounds must be even.  */
#define SALSA20_ROUNDS       20
#define SALSA20R12_ROUNDS    12


struct SALSA20_context_s;

typedef unsigned int (*salsa20_core_t) (u32 *dst, struct SALSA20_context_s *ctx,
                                        unsigned int rounds);
typedef void (* salsa20_keysetup_t)(struct SALSA20_context_s *ctx,
                                    const byte *key, int keylen);
typedef void (* salsa20_ivsetup_t)(struct SALSA20_context_s *ctx,
                                   const byte *iv);

typedef struct SALSA20_context_s
{
  /* Indices 1-4 and 11-14 holds the key (two identical copies for the
     shorter key size), indices 0, 5, 10, 15 are constant, indices 6, 7
     are the IV, and indices 8, 9 are the block counter:

     C K K K
     K C I I
     B B C K
     K K K C
  */
  u32 input[SALSA20_INPUT_LENGTH];
  u32 pad[SALSA20_INPUT_LENGTH];
  unsigned int unused; /* bytes in the pad.  */
#ifdef USE_ARM_NEON_ASM
  int use_neon;
#endif
  salsa20_keysetup_t keysetup;
  salsa20_ivsetup_t ivsetup;
  salsa20_core_t core;
} SALSA20_context_t;


/* The masking of the right shift is needed to allow n == 0 (using
   just 32 - n and 64 - n results in undefined behaviour). Most uses
   of these macros use a constant and non-zero rotation count. */
#define ROTL32(n,x) (((x)<<(n)) | ((x)>>((-(n)&31))))


#define LE_SWAP32(v) le_bswap32(v)

#define LE_READ_UINT32(p) buf_get_le32(p)


static void salsa20_setiv (void *context, const byte *iv, size_t ivlen);
static const char *selftest (void);


#ifdef USE_AMD64

/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
# define ASM_FUNC_ABI __attribute__((sysv_abi))
# define ASM_EXTRA_STACK (10 * 16)
#else
# define ASM_FUNC_ABI
# define ASM_EXTRA_STACK 0
#endif

/* AMD64 assembly implementations of Salsa20. */
void _gcry_salsa20_amd64_keysetup(u32 *ctxinput, const void *key, int keybits)
                                 ASM_FUNC_ABI;
void _gcry_salsa20_amd64_ivsetup(u32 *ctxinput, const void *iv)
                                ASM_FUNC_ABI;
unsigned int
_gcry_salsa20_amd64_encrypt_blocks(u32 *ctxinput, const void *src, void *dst,
                                   size_t len, int rounds) ASM_FUNC_ABI;

static void
salsa20_keysetup(SALSA20_context_t *ctx, const byte *key, int keylen)
{
  _gcry_salsa20_amd64_keysetup(ctx->input, key, keylen * 8);
}

static void
salsa20_ivsetup(SALSA20_context_t *ctx, const byte *iv)
{
  _gcry_salsa20_amd64_ivsetup(ctx->input, iv);
}

static unsigned int
salsa20_core (u32 *dst, SALSA20_context_t *ctx, unsigned int rounds)
{
  memset(dst, 0, SALSA20_BLOCK_SIZE);
  return _gcry_salsa20_amd64_encrypt_blocks(ctx->input, dst, dst, 1, rounds)
         + ASM_EXTRA_STACK;
}

#else /* USE_AMD64 */



#if 0
# define SALSA20_CORE_DEBUG(i) do {		\
    unsigned debug_j;				\
    for (debug_j = 0; debug_j < 16; debug_j++)	\
      {						\
	if (debug_j == 0)			\
	  fprintf(stderr, "%2d:", (i));		\
	else if (debug_j % 4 == 0)		\
	  fprintf(stderr, "\n   ");		\
	fprintf(stderr, " %8x", pad[debug_j]);	\
      }						\
    fprintf(stderr, "\n");			\
  } while (0)
#else
# define SALSA20_CORE_DEBUG(i)
#endif

#define QROUND(x0, x1, x2, x3)      \
  do {                              \
    x1 ^= ROTL32 ( 7, x0 + x3);	    \
    x2 ^= ROTL32 ( 9, x1 + x0);	    \
    x3 ^= ROTL32 (13, x2 + x1);	    \
    x0 ^= ROTL32 (18, x3 + x2);	    \
  } while(0)

static unsigned int
salsa20_core (u32 *dst, SALSA20_context_t *ctx, unsigned rounds)
{
  u32 pad[SALSA20_INPUT_LENGTH], *src = ctx->input;
  unsigned int i;

  memcpy (pad, src, sizeof(pad));
  for (i = 0; i < rounds; i += 2)
    {
      SALSA20_CORE_DEBUG (i);
      QROUND (pad[0],  pad[4],  pad[8],  pad[12]);
      QROUND (pad[5],  pad[9],  pad[13], pad[1] );
      QROUND (pad[10], pad[14], pad[2],  pad[6] );
      QROUND (pad[15], pad[3],  pad[7],  pad[11]);

      SALSA20_CORE_DEBUG (i+1);
      QROUND (pad[0],  pad[1],  pad[2],  pad[3] );
      QROUND (pad[5],  pad[6],  pad[7],  pad[4] );
      QROUND (pad[10], pad[11], pad[8],  pad[9] );
      QROUND (pad[15], pad[12], pad[13], pad[14]);
    }
  SALSA20_CORE_DEBUG (i);

  for (i = 0; i < SALSA20_INPUT_LENGTH; i++)
    {
      u32 t = pad[i] + src[i];
      dst[i] = LE_SWAP32 (t);
    }

  /* Update counter. */
  if (!++src[8])
    src[9]++;

  /* burn_stack */
  return ( 3*sizeof (void*) \
         + 2*sizeof (void*) \
         + 64 \
         + sizeof (unsigned int) \
         + sizeof (u32) );
}
#undef QROUND
#undef SALSA20_CORE_DEBUG

static void
salsa20_keysetup(SALSA20_context_t *ctx, const byte *key, int keylen)
{
  /* These constants are the little endian encoding of the string
     "expand 32-byte k".  For the 128 bit variant, the "32" in that
     string will be fixed up to "16".  */
  ctx->input[0]  = 0x61707865; /* "apxe"  */
  ctx->input[5]  = 0x3320646e; /* "3 dn"  */
  ctx->input[10] = 0x79622d32; /* "yb-2"  */
  ctx->input[15] = 0x6b206574; /* "k et"  */

  ctx->input[1] = LE_READ_UINT32(key + 0);
  ctx->input[2] = LE_READ_UINT32(key + 4);
  ctx->input[3] = LE_READ_UINT32(key + 8);
  ctx->input[4] = LE_READ_UINT32(key + 12);
  if (keylen == SALSA20_MAX_KEY_SIZE) /* 256 bits */
    {
      ctx->input[11] = LE_READ_UINT32(key + 16);
      ctx->input[12] = LE_READ_UINT32(key + 20);
      ctx->input[13] = LE_READ_UINT32(key + 24);
      ctx->input[14] = LE_READ_UINT32(key + 28);
    }
  else  /* 128 bits */
    {
      ctx->input[11] = ctx->input[1];
      ctx->input[12] = ctx->input[2];
      ctx->input[13] = ctx->input[3];
      ctx->input[14] = ctx->input[4];

      ctx->input[5]  -= 0x02000000; /* Change to "1 dn".  */
      ctx->input[10] += 0x00000004; /* Change to "yb-6".  */
    }
}

static void salsa20_ivsetup(SALSA20_context_t *ctx, const byte *iv)
{
  ctx->input[6] = LE_READ_UINT32(iv + 0);
  ctx->input[7] = LE_READ_UINT32(iv + 4);
  /* Reset the block counter.  */
  ctx->input[8] = 0;
  ctx->input[9] = 0;
}

#endif /*!USE_AMD64*/

#ifdef USE_ARM_NEON_ASM

/* ARM NEON implementation of Salsa20. */
unsigned int
_gcry_arm_neon_salsa20_encrypt(void *c, const void *m, unsigned int nblks,
                               void *k, unsigned int rounds);

static unsigned int
salsa20_core_neon (u32 *dst, SALSA20_context_t *ctx, unsigned int rounds)
{
  return _gcry_arm_neon_salsa20_encrypt(dst, NULL, 1, ctx->input, rounds);
}

static void salsa20_ivsetup_neon(SALSA20_context_t *ctx, const byte *iv)
{
  memcpy(ctx->input + 8, iv, 8);
  /* Reset the block counter.  */
  memset(ctx->input + 10, 0, 8);
}

static void
salsa20_keysetup_neon(SALSA20_context_t *ctx, const byte *key, int klen)
{
  static const unsigned char sigma32[16] = "expand 32-byte k";
  static const unsigned char sigma16[16] = "expand 16-byte k";

  if (klen == 16)
    {
      memcpy (ctx->input, key, 16);
      memcpy (ctx->input + 4, key, 16); /* Duplicate 128-bit key. */
      memcpy (ctx->input + 12, sigma16, 16);
    }
  else
    {
      /* 32-byte key */
      memcpy (ctx->input, key, 32);
      memcpy (ctx->input + 12, sigma32, 16);
    }
}

#endif /*USE_ARM_NEON_ASM*/


static gcry_err_code_t
salsa20_do_setkey (SALSA20_context_t *ctx,
                   const byte *key, unsigned int keylen)
{
  static int initialized;
  static const char *selftest_failed;

  if (!initialized )
    {
      initialized = 1;
      selftest_failed = selftest ();
      if (selftest_failed)
        log_error ("SALSA20 selftest failed (%s)\n", selftest_failed );
    }
  if (selftest_failed)
    return GPG_ERR_SELFTEST_FAILED;

  if (keylen != SALSA20_MIN_KEY_SIZE
      && keylen != SALSA20_MAX_KEY_SIZE)
    return GPG_ERR_INV_KEYLEN;

  /* Default ops. */
  ctx->keysetup = salsa20_keysetup;
  ctx->ivsetup = salsa20_ivsetup;
  ctx->core = salsa20_core;

#ifdef USE_ARM_NEON_ASM
  ctx->use_neon = (_gcry_get_hw_features () & HWF_ARM_NEON) != 0;
  if (ctx->use_neon)
    {
      /* Use ARM NEON ops instead. */
      ctx->keysetup = salsa20_keysetup_neon;
      ctx->ivsetup = salsa20_ivsetup_neon;
      ctx->core = salsa20_core_neon;
    }
#endif

  ctx->keysetup (ctx, key, keylen);

  /* We default to a zero nonce.  */
  salsa20_setiv (ctx, NULL, 0);

  return 0;
}


static gcry_err_code_t
salsa20_setkey (void *context, const byte *key, unsigned int keylen,
                cipher_bulk_ops_t *bulk_ops)
{
  SALSA20_context_t *ctx = (SALSA20_context_t *)context;
  gcry_err_code_t rc = salsa20_do_setkey (ctx, key, keylen);
  (void)bulk_ops;
  _gcry_burn_stack (4 + sizeof (void *) + 4 * sizeof (void *));
  return rc;
}


static void
salsa20_setiv (void *context, const byte *iv, size_t ivlen)
{
  SALSA20_context_t *ctx = (SALSA20_context_t *)context;
  byte tmp[SALSA20_IV_SIZE];

  if (iv && ivlen != SALSA20_IV_SIZE)
    log_info ("WARNING: salsa20_setiv: bad ivlen=%u\n", (u32)ivlen);

  if (!iv || ivlen != SALSA20_IV_SIZE)
    memset (tmp, 0, sizeof(tmp));
  else
    memcpy (tmp, iv, SALSA20_IV_SIZE);

  ctx->ivsetup (ctx, tmp);

  /* Reset the unused pad bytes counter.  */
  ctx->unused = 0;

  wipememory (tmp, sizeof(tmp));
}



/* Note: This function requires LENGTH > 0.  */
static void
salsa20_do_encrypt_stream (SALSA20_context_t *ctx,
                           byte *outbuf, const byte *inbuf,
                           size_t length, unsigned rounds)
{
  unsigned int nburn, burn = 0;

  if (ctx->unused)
    {
      unsigned char *p = (void*)ctx->pad;
      size_t n;

      gcry_assert (ctx->unused < SALSA20_BLOCK_SIZE);

      n = ctx->unused;
      if (n > length)
        n = length;
      buf_xor (outbuf, inbuf, p + SALSA20_BLOCK_SIZE - ctx->unused, n);
      length -= n;
      outbuf += n;
      inbuf  += n;
      ctx->unused -= n;
      if (!length)
        return;
      gcry_assert (!ctx->unused);
    }

#ifdef USE_AMD64
  if (length >= SALSA20_BLOCK_SIZE)
    {
      size_t nblocks = length / SALSA20_BLOCK_SIZE;
      burn = _gcry_salsa20_amd64_encrypt_blocks(ctx->input, inbuf, outbuf,
                                                nblocks, rounds);
      burn += ASM_EXTRA_STACK;
      length -= SALSA20_BLOCK_SIZE * nblocks;
      outbuf += SALSA20_BLOCK_SIZE * nblocks;
      inbuf  += SALSA20_BLOCK_SIZE * nblocks;
    }
#endif

#ifdef USE_ARM_NEON_ASM
  if (ctx->use_neon && length >= SALSA20_BLOCK_SIZE)
    {
      unsigned int nblocks = length / SALSA20_BLOCK_SIZE;
      _gcry_arm_neon_salsa20_encrypt (outbuf, inbuf, nblocks, ctx->input,
                                      rounds);
      length -= SALSA20_BLOCK_SIZE * nblocks;
      outbuf += SALSA20_BLOCK_SIZE * nblocks;
      inbuf  += SALSA20_BLOCK_SIZE * nblocks;
    }
#endif

  while (length > 0)
    {
      /* Create the next pad and bump the block counter.  Note that it
         is the user's duty to change to another nonce not later than
         after 2^70 processed bytes.  */
      nburn = ctx->core (ctx->pad, ctx, rounds);
      burn = nburn > burn ? nburn : burn;

      if (length <= SALSA20_BLOCK_SIZE)
	{
	  buf_xor (outbuf, inbuf, ctx->pad, length);
          ctx->unused = SALSA20_BLOCK_SIZE - length;
	  break;
	}
      buf_xor (outbuf, inbuf, ctx->pad, SALSA20_BLOCK_SIZE);
      length -= SALSA20_BLOCK_SIZE;
      outbuf += SALSA20_BLOCK_SIZE;
      inbuf  += SALSA20_BLOCK_SIZE;
    }

  _gcry_burn_stack (burn);
}


static void
salsa20_encrypt_stream (void *context,
                        byte *outbuf, const byte *inbuf, size_t length)
{
  SALSA20_context_t *ctx = (SALSA20_context_t *)context;

  if (length)
    salsa20_do_encrypt_stream (ctx, outbuf, inbuf, length, SALSA20_ROUNDS);
}


static void
salsa20r12_encrypt_stream (void *context,
                           byte *outbuf, const byte *inbuf, size_t length)
{
  SALSA20_context_t *ctx = (SALSA20_context_t *)context;

  if (length)
    salsa20_do_encrypt_stream (ctx, outbuf, inbuf, length, SALSA20R12_ROUNDS);
}


static const char*
selftest (void)
{
  byte ctxbuf[sizeof(SALSA20_context_t) + 15];
  SALSA20_context_t *ctx;
  byte scratch[8+1];
  byte buf[256+64+4];
  int i;

  static byte key_1[] =
    { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  static const byte nonce_1[] =
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  static const byte plaintext_1[] =
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  static const byte ciphertext_1[] =
    { 0xE3, 0xBE, 0x8F, 0xDD, 0x8B, 0xEC, 0xA2, 0xE3};

  /* 16-byte alignment required for amd64 implementation. */
  ctx = (SALSA20_context_t *)((uintptr_t)(ctxbuf + 15) & ~(uintptr_t)15);

  salsa20_setkey (ctx, key_1, sizeof key_1, NULL);
  salsa20_setiv  (ctx, nonce_1, sizeof nonce_1);
  scratch[8] = 0;
  salsa20_encrypt_stream (ctx, scratch, plaintext_1, sizeof plaintext_1);
  if (memcmp (scratch, ciphertext_1, sizeof ciphertext_1))
    return "Salsa20 encryption test 1 failed.";
  if (scratch[8])
    return "Salsa20 wrote too much.";
  salsa20_setkey( ctx, key_1, sizeof(key_1), NULL);
  salsa20_setiv  (ctx, nonce_1, sizeof nonce_1);
  salsa20_encrypt_stream (ctx, scratch, scratch, sizeof plaintext_1);
  if (memcmp (scratch, plaintext_1, sizeof plaintext_1))
    return "Salsa20 decryption test 1 failed.";

  for (i = 0; i < sizeof buf; i++)
    buf[i] = i;
  salsa20_setkey (ctx, key_1, sizeof key_1, NULL);
  salsa20_setiv (ctx, nonce_1, sizeof nonce_1);
  /*encrypt*/
  salsa20_encrypt_stream (ctx, buf, buf, sizeof buf);
  /*decrypt*/
  salsa20_setkey (ctx, key_1, sizeof key_1, NULL);
  salsa20_setiv (ctx, nonce_1, sizeof nonce_1);
  salsa20_encrypt_stream (ctx, buf, buf, 1);
  salsa20_encrypt_stream (ctx, buf+1, buf+1, (sizeof buf)-1-1);
  salsa20_encrypt_stream (ctx, buf+(sizeof buf)-1, buf+(sizeof buf)-1, 1);
  for (i = 0; i < sizeof buf; i++)
    if (buf[i] != (byte)i)
      return "Salsa20 encryption test 2 failed.";

  return NULL;
}


gcry_cipher_spec_t _gcry_cipher_spec_salsa20 =
  {
    GCRY_CIPHER_SALSA20,
    {0, 0},     /* flags */
    "SALSA20",  /* name */
    NULL,       /* aliases */
    NULL,       /* oids */
    1,          /* blocksize in bytes. */
    SALSA20_MAX_KEY_SIZE*8,  /* standard key length in bits. */
    sizeof (SALSA20_context_t),
    salsa20_setkey,
    NULL,
    NULL,
    salsa20_encrypt_stream,
    salsa20_encrypt_stream,
    NULL,
    NULL,
    salsa20_setiv
  };

gcry_cipher_spec_t _gcry_cipher_spec_salsa20r12 =
  {
    GCRY_CIPHER_SALSA20R12,
    {0, 0},     /* flags */
    "SALSA20R12",  /* name */
    NULL,       /* aliases */
    NULL,       /* oids */
    1,          /* blocksize in bytes. */
    SALSA20_MAX_KEY_SIZE*8,  /* standard key length in bits. */
    sizeof (SALSA20_context_t),
    salsa20_setkey,
    NULL,
    NULL,
    salsa20r12_encrypt_stream,
    salsa20r12_encrypt_stream,
    NULL,
    NULL,
    salsa20_setiv
  };
