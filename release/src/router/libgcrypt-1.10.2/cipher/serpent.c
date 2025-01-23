/* serpent.c - Implementation of the Serpent encryption algorithm.
 *	Copyright (C) 2003, 2004, 2005 Free Software Foundation, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>

#include <string.h>
#include <stdio.h>

#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher-internal.h"
#include "cipher-selftest.h"


/* USE_SSE2 indicates whether to compile with AMD64 SSE2 code. */
#undef USE_SSE2
#if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_SSE2 1
#endif

/* USE_AVX2 indicates whether to compile with AMD64 AVX2 code. */
#undef USE_AVX2
#if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# if defined(ENABLE_AVX2_SUPPORT)
#  define USE_AVX2 1
# endif
#endif

/* USE_NEON indicates whether to enable ARM NEON assembly code. */
#undef USE_NEON
#ifdef ENABLE_NEON_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_NEON)
#  define USE_NEON 1
# endif
#endif /*ENABLE_NEON_SUPPORT*/

/* Number of rounds per Serpent encrypt/decrypt operation.  */
#define ROUNDS 32

/* Magic number, used during generating of the subkeys.  */
#define PHI 0x9E3779B9

/* Serpent works on 128 bit blocks.  */
typedef u32 serpent_block_t[4];

/* Serpent key, provided by the user.  If the original key is shorter
   than 256 bits, it is padded.  */
typedef u32 serpent_key_t[8];

/* The key schedule consists of 33 128 bit subkeys.  */
typedef u32 serpent_subkeys_t[ROUNDS + 1][4];

/* A Serpent context.  */
typedef struct serpent_context
{
  serpent_subkeys_t keys;	/* Generated subkeys.  */

#ifdef USE_AVX2
  int use_avx2;
#endif
#ifdef USE_NEON
  int use_neon;
#endif
} serpent_context_t;


/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#if defined(USE_SSE2) || defined(USE_AVX2)
# ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
#  define ASM_FUNC_ABI __attribute__((sysv_abi))
# else
#  define ASM_FUNC_ABI
# endif
#endif


#ifdef USE_SSE2
/* Assembler implementations of Serpent using SSE2.  Process 8 block in
   parallel.
 */
extern void _gcry_serpent_sse2_ctr_enc(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *ctr) ASM_FUNC_ABI;

extern void _gcry_serpent_sse2_cbc_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_serpent_sse2_cfb_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_serpent_sse2_ocb_enc(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *offset,
				       unsigned char *checksum,
				       const u64 Ls[8]) ASM_FUNC_ABI;

extern void _gcry_serpent_sse2_ocb_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *offset,
				       unsigned char *checksum,
				       const u64 Ls[8]) ASM_FUNC_ABI;

extern void _gcry_serpent_sse2_ocb_auth(serpent_context_t *ctx,
					const unsigned char *abuf,
					unsigned char *offset,
					unsigned char *checksum,
					const u64 Ls[8]) ASM_FUNC_ABI;
#endif

#ifdef USE_AVX2
/* Assembler implementations of Serpent using AVX2.  Process 16 block in
   parallel.
 */
extern void _gcry_serpent_avx2_ctr_enc(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *ctr) ASM_FUNC_ABI;

extern void _gcry_serpent_avx2_cbc_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_serpent_avx2_cfb_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_serpent_avx2_ocb_enc(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *offset,
				       unsigned char *checksum,
				       const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_serpent_avx2_ocb_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *offset,
				       unsigned char *checksum,
				       const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_serpent_avx2_ocb_auth(serpent_context_t *ctx,
					const unsigned char *abuf,
					unsigned char *offset,
					unsigned char *checksum,
					const u64 Ls[16]) ASM_FUNC_ABI;
#endif

#ifdef USE_NEON
/* Assembler implementations of Serpent using ARM NEON.  Process 8 block in
   parallel.
 */
extern void _gcry_serpent_neon_ctr_enc(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *ctr);

extern void _gcry_serpent_neon_cbc_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *iv);

extern void _gcry_serpent_neon_cfb_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *iv);

extern void _gcry_serpent_neon_ocb_enc(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *offset,
				       unsigned char *checksum,
				       const void *Ls[8]);

extern void _gcry_serpent_neon_ocb_dec(serpent_context_t *ctx,
				       unsigned char *out,
				       const unsigned char *in,
				       unsigned char *offset,
				       unsigned char *checksum,
				       const void *Ls[8]);

extern void _gcry_serpent_neon_ocb_auth(serpent_context_t *ctx,
					const unsigned char *abuf,
					unsigned char *offset,
					unsigned char *checksum,
					const void *Ls[8]);
#endif


/* Prototypes.  */
static const char *serpent_test (void);

static void _gcry_serpent_ctr_enc (void *context, unsigned char *ctr,
				   void *outbuf_arg, const void *inbuf_arg,
				   size_t nblocks);
static void _gcry_serpent_cbc_dec (void *context, unsigned char *iv,
				   void *outbuf_arg, const void *inbuf_arg,
				   size_t nblocks);
static void _gcry_serpent_cfb_dec (void *context, unsigned char *iv,
				   void *outbuf_arg, const void *inbuf_arg,
				   size_t nblocks);
static size_t _gcry_serpent_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
				       const void *inbuf_arg, size_t nblocks,
				       int encrypt);
static size_t _gcry_serpent_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
				      size_t nblocks);


/*
 * These are the S-Boxes of Serpent from following research paper.
 *
 *  D. A. Osvik, “Speeding up Serpent,” in Third AES Candidate Conference,
 *   (New York, New York, USA), p. 317–329, National Institute of Standards and
 *   Technology, 2000.
 *
 * Paper is also available at: http://www.ii.uib.no/~osvik/pub/aes3.pdf
 *
 */

#define SBOX0(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r3 ^= r0; r4 =  r1; \
    r1 &= r3; r4 ^= r2; \
    r1 ^= r0; r0 |= r3; \
    r0 ^= r4; r4 ^= r3; \
    r3 ^= r2; r2 |= r1; \
    r2 ^= r4; r4 = ~r4; \
    r4 |= r1; r1 ^= r3; \
    r1 ^= r4; r3 |= r0; \
    r1 ^= r3; r4 ^= r3; \
    \
    w = r1; x = r4; y = r2; z = r0; \
  }

#define SBOX0_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r2 = ~r2; r4 =  r1; \
    r1 |= r0; r4 = ~r4; \
    r1 ^= r2; r2 |= r4; \
    r1 ^= r3; r0 ^= r4; \
    r2 ^= r0; r0 &= r3; \
    r4 ^= r0; r0 |= r1; \
    r0 ^= r2; r3 ^= r4; \
    r2 ^= r1; r3 ^= r0; \
    r3 ^= r1; \
    r2 &= r3; \
    r4 ^= r2; \
    \
    w = r0; x = r4; y = r1; z = r3; \
  }

#define SBOX1(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r0 = ~r0; r2 = ~r2; \
    r4 =  r0; r0 &= r1; \
    r2 ^= r0; r0 |= r3; \
    r3 ^= r2; r1 ^= r0; \
    r0 ^= r4; r4 |= r1; \
    r1 ^= r3; r2 |= r0; \
    r2 &= r4; r0 ^= r1; \
    r1 &= r2; \
    r1 ^= r0; r0 &= r2; \
    r0 ^= r4; \
    \
    w = r2; x = r0; y = r3; z = r1; \
  }

#define SBOX1_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r1; r1 ^= r3; \
    r3 &= r1; r4 ^= r2; \
    r3 ^= r0; r0 |= r1; \
    r2 ^= r3; r0 ^= r4; \
    r0 |= r2; r1 ^= r3; \
    r0 ^= r1; r1 |= r3; \
    r1 ^= r0; r4 = ~r4; \
    r4 ^= r1; r1 |= r0; \
    r1 ^= r0; \
    r1 |= r4; \
    r3 ^= r1; \
    \
    w = r4; x = r0; y = r3; z = r2; \
  }

#define SBOX2(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r0; r0 &= r2; \
    r0 ^= r3; r2 ^= r1; \
    r2 ^= r0; r3 |= r4; \
    r3 ^= r1; r4 ^= r2; \
    r1 =  r3; r3 |= r4; \
    r3 ^= r0; r0 &= r1; \
    r4 ^= r0; r1 ^= r3; \
    r1 ^= r4; r4 = ~r4; \
    \
    w = r2; x = r3; y = r1; z = r4; \
  }

#define SBOX2_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r2 ^= r3; r3 ^= r0; \
    r4 =  r3; r3 &= r2; \
    r3 ^= r1; r1 |= r2; \
    r1 ^= r4; r4 &= r3; \
    r2 ^= r3; r4 &= r0; \
    r4 ^= r2; r2 &= r1; \
    r2 |= r0; r3 = ~r3; \
    r2 ^= r3; r0 ^= r3; \
    r0 &= r1; r3 ^= r4; \
    r3 ^= r0; \
    \
    w = r1; x = r4; y = r2; z = r3; \
  }

#define SBOX3(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r0; r0 |= r3; \
    r3 ^= r1; r1 &= r4; \
    r4 ^= r2; r2 ^= r3; \
    r3 &= r0; r4 |= r1; \
    r3 ^= r4; r0 ^= r1; \
    r4 &= r0; r1 ^= r3; \
    r4 ^= r2; r1 |= r0; \
    r1 ^= r2; r0 ^= r3; \
    r2 =  r1; r1 |= r3; \
    r1 ^= r0; \
    \
    w = r1; x = r2; y = r3; z = r4; \
  }

#define SBOX3_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r2; r2 ^= r1; \
    r0 ^= r2; r4 &= r2; \
    r4 ^= r0; r0 &= r1; \
    r1 ^= r3; r3 |= r4; \
    r2 ^= r3; r0 ^= r3; \
    r1 ^= r4; r3 &= r2; \
    r3 ^= r1; r1 ^= r0; \
    r1 |= r2; r0 ^= r3; \
    r1 ^= r4; \
    r0 ^= r1; \
    \
    w = r2; x = r1; y = r3; z = r0; \
  }

#define SBOX4(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r1 ^= r3; r3 = ~r3; \
    r2 ^= r3; r3 ^= r0; \
    r4 =  r1; r1 &= r3; \
    r1 ^= r2; r4 ^= r3; \
    r0 ^= r4; r2 &= r4; \
    r2 ^= r0; r0 &= r1; \
    r3 ^= r0; r4 |= r1; \
    r4 ^= r0; r0 |= r3; \
    r0 ^= r2; r2 &= r3; \
    r0 = ~r0; r4 ^= r2; \
    \
    w = r1; x = r4; y = r0; z = r3; \
  }

#define SBOX4_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r2; r2 &= r3; \
    r2 ^= r1; r1 |= r3; \
    r1 &= r0; r4 ^= r2; \
    r4 ^= r1; r1 &= r2; \
    r0 = ~r0; r3 ^= r4; \
    r1 ^= r3; r3 &= r0; \
    r3 ^= r2; r0 ^= r1; \
    r2 &= r0; r3 ^= r0; \
    r2 ^= r4; \
    r2 |= r3; r3 ^= r0; \
    r2 ^= r1; \
    \
    w = r0; x = r3; y = r2; z = r4; \
  }

#define SBOX5(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r0 ^= r1; r1 ^= r3; \
    r3 = ~r3; r4 =  r1; \
    r1 &= r0; r2 ^= r3; \
    r1 ^= r2; r2 |= r4; \
    r4 ^= r3; r3 &= r1; \
    r3 ^= r0; r4 ^= r1; \
    r4 ^= r2; r2 ^= r0; \
    r0 &= r3; r2 = ~r2; \
    r0 ^= r4; r4 |= r3; \
    r2 ^= r4; \
    \
    w = r1; x = r3; y = r0; z = r2; \
  }

#define SBOX5_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r1 = ~r1; r4 =  r3; \
    r2 ^= r1; r3 |= r0; \
    r3 ^= r2; r2 |= r1; \
    r2 &= r0; r4 ^= r3; \
    r2 ^= r4; r4 |= r0; \
    r4 ^= r1; r1 &= r2; \
    r1 ^= r3; r4 ^= r2; \
    r3 &= r4; r4 ^= r1; \
    r3 ^= r4; r4 = ~r4; \
    r3 ^= r0; \
    \
    w = r1; x = r4; y = r3; z = r2; \
  }

#define SBOX6(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r2 = ~r2; r4 =  r3; \
    r3 &= r0; r0 ^= r4; \
    r3 ^= r2; r2 |= r4; \
    r1 ^= r3; r2 ^= r0; \
    r0 |= r1; r2 ^= r1; \
    r4 ^= r0; r0 |= r3; \
    r0 ^= r2; r4 ^= r3; \
    r4 ^= r0; r3 = ~r3; \
    r2 &= r4; \
    r2 ^= r3; \
    \
    w = r0; x = r1; y = r4; z = r2; \
  }

#define SBOX6_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r0 ^= r2; r4 =  r2; \
    r2 &= r0; r4 ^= r3; \
    r2 = ~r2; r3 ^= r1; \
    r2 ^= r3; r4 |= r0; \
    r0 ^= r2; r3 ^= r4; \
    r4 ^= r1; r1 &= r3; \
    r1 ^= r0; r0 ^= r3; \
    r0 |= r2; r3 ^= r1; \
    r4 ^= r0; \
    \
    w = r1; x = r2; y = r4; z = r3; \
  }

#define SBOX7(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r1; r1 |= r2; \
    r1 ^= r3; r4 ^= r2; \
    r2 ^= r1; r3 |= r4; \
    r3 &= r0; r4 ^= r2; \
    r3 ^= r1; r1 |= r4; \
    r1 ^= r0; r0 |= r4; \
    r0 ^= r2; r1 ^= r4; \
    r2 ^= r1; r1 &= r0; \
    r1 ^= r4; r2 = ~r2; \
    r2 |= r0; \
    r4 ^= r2; \
    \
    w = r4; x = r3; y = r1; z = r0; \
  }

#define SBOX7_INVERSE(r0, r1, r2, r3, w, x, y, z) \
  { \
    u32 r4; \
    \
    r4 =  r2; r2 ^= r0; \
    r0 &= r3; r4 |= r3; \
    r2 = ~r2; r3 ^= r1; \
    r1 |= r0; r0 ^= r2; \
    r2 &= r4; r3 &= r4; \
    r1 ^= r2; r2 ^= r0; \
    r0 |= r2; r4 ^= r1; \
    r0 ^= r3; r3 ^= r4; \
    r4 |= r0; r3 ^= r2; \
    r4 ^= r2; \
    \
    w = r3; x = r0; y = r1; z = r4; \
  }

/* XOR BLOCK1 into BLOCK0.  */
#define BLOCK_XOR(block0, block1) \
  {                               \
    block0[0] ^= block1[0];       \
    block0[1] ^= block1[1];       \
    block0[2] ^= block1[2];       \
    block0[3] ^= block1[3];       \
  }

/* Copy BLOCK_SRC to BLOCK_DST.  */
#define BLOCK_COPY(block_dst, block_src) \
  {                                      \
    block_dst[0] = block_src[0];         \
    block_dst[1] = block_src[1];         \
    block_dst[2] = block_src[2];         \
    block_dst[3] = block_src[3];         \
  }

/* Apply SBOX number WHICH to to the block found in ARRAY0, writing
   the output to the block found in ARRAY1.  */
#define SBOX(which, array0, array1)                         \
  SBOX##which (array0[0], array0[1], array0[2], array0[3],  \
               array1[0], array1[1], array1[2], array1[3]);

/* Apply inverse SBOX number WHICH to to the block found in ARRAY0, writing
   the output to the block found in ARRAY1.  */
#define SBOX_INVERSE(which, array0, array1)                           \
  SBOX##which##_INVERSE (array0[0], array0[1], array0[2], array0[3],  \
                         array1[0], array1[1], array1[2], array1[3]);

/* Apply the linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION(block)                  \
  {                                                   \
    block[0] = rol (block[0], 13);                    \
    block[2] = rol (block[2], 3);                     \
    block[1] = block[1] ^ block[0] ^ block[2];        \
    block[3] = block[3] ^ block[2] ^ (block[0] << 3); \
    block[1] = rol (block[1], 1);                     \
    block[3] = rol (block[3], 7);                     \
    block[0] = block[0] ^ block[1] ^ block[3];        \
    block[2] = block[2] ^ block[3] ^ (block[1] << 7); \
    block[0] = rol (block[0], 5);                     \
    block[2] = rol (block[2], 22);                    \
  }

/* Apply the inverse linear transformation to BLOCK.  */
#define LINEAR_TRANSFORMATION_INVERSE(block)          \
  {                                                   \
    block[2] = ror (block[2], 22);                    \
    block[0] = ror (block[0] , 5);                    \
    block[2] = block[2] ^ block[3] ^ (block[1] << 7); \
    block[0] = block[0] ^ block[1] ^ block[3];        \
    block[3] = ror (block[3], 7);                     \
    block[1] = ror (block[1], 1);                     \
    block[3] = block[3] ^ block[2] ^ (block[0] << 3); \
    block[1] = block[1] ^ block[0] ^ block[2];        \
    block[2] = ror (block[2], 3);                     \
    block[0] = ror (block[0], 13);                    \
  }

/* Apply a Serpent round to BLOCK, using the SBOX number WHICH and the
   subkeys contained in SUBKEYS.  Use BLOCK_TMP as temporary storage.
   This macro increments `round'.  */
#define ROUND(which, subkeys, block, block_tmp) \
  {                                             \
    BLOCK_XOR (block, subkeys[round]);          \
    round++;                                    \
    SBOX (which, block, block_tmp);             \
    LINEAR_TRANSFORMATION (block_tmp);          \
    BLOCK_COPY (block, block_tmp);              \
  }

/* Apply the last Serpent round to BLOCK, using the SBOX number WHICH
   and the subkeys contained in SUBKEYS.  Use BLOCK_TMP as temporary
   storage.  The result will be stored in BLOCK_TMP.  This macro
   increments `round'.  */
#define ROUND_LAST(which, subkeys, block, block_tmp) \
  {                                                  \
    BLOCK_XOR (block, subkeys[round]);               \
    round++;                                         \
    SBOX (which, block, block_tmp);                  \
    BLOCK_XOR (block_tmp, subkeys[round]);           \
    round++;                                         \
  }

/* Apply an inverse Serpent round to BLOCK, using the SBOX number
   WHICH and the subkeys contained in SUBKEYS.  Use BLOCK_TMP as
   temporary storage.  This macro increments `round'.  */
#define ROUND_INVERSE(which, subkey, block, block_tmp) \
  {                                                    \
    LINEAR_TRANSFORMATION_INVERSE (block);             \
    SBOX_INVERSE (which, block, block_tmp);            \
    BLOCK_XOR (block_tmp, subkey[round]);              \
    round--;                                           \
    BLOCK_COPY (block, block_tmp);                     \
  }

/* Apply the first Serpent round to BLOCK, using the SBOX number WHICH
   and the subkeys contained in SUBKEYS.  Use BLOCK_TMP as temporary
   storage.  The result will be stored in BLOCK_TMP.  This macro
   increments `round'.  */
#define ROUND_FIRST_INVERSE(which, subkeys, block, block_tmp) \
  {                                                           \
    BLOCK_XOR (block, subkeys[round]);                        \
    round--;                                                  \
    SBOX_INVERSE (which, block, block_tmp);                   \
    BLOCK_XOR (block_tmp, subkeys[round]);                    \
    round--;                                                  \
  }

/* Convert the user provided key KEY of KEY_LENGTH bytes into the
   internally used format.  */
static void
serpent_key_prepare (const byte *key, unsigned int key_length,
		     serpent_key_t key_prepared)
{
  int i;

  /* Copy key.  */
  key_length /= 4;
  for (i = 0; i < key_length; i++)
    key_prepared[i] = buf_get_le32 (key + i * 4);

  if (i < 8)
    {
      /* Key must be padded according to the Serpent
	 specification.  */
      key_prepared[i] = 0x00000001;

      for (i++; i < 8; i++)
	key_prepared[i] = 0;
    }
}

/* Derive the 33 subkeys from KEY and store them in SUBKEYS.  */
static void
serpent_subkeys_generate (serpent_key_t key, serpent_subkeys_t subkeys)
{
  u32 w[8];		/* The `prekey'.  */
  u32 ws[4];
  u32 wt[4];

  /* Initialize with key values.  */
  w[0] = key[0];
  w[1] = key[1];
  w[2] = key[2];
  w[3] = key[3];
  w[4] = key[4];
  w[5] = key[5];
  w[6] = key[6];
  w[7] = key[7];

  /* Expand to intermediate key using the affine recurrence.  */
#define EXPAND_KEY4(wo, r)                                                     \
  wo[0] = w[(r+0)%8] =                                                         \
    rol (w[(r+0)%8] ^ w[(r+3)%8] ^ w[(r+5)%8] ^ w[(r+7)%8] ^ PHI ^ (r+0), 11); \
  wo[1] = w[(r+1)%8] =                                                         \
    rol (w[(r+1)%8] ^ w[(r+4)%8] ^ w[(r+6)%8] ^ w[(r+0)%8] ^ PHI ^ (r+1), 11); \
  wo[2] = w[(r+2)%8] =                                                         \
    rol (w[(r+2)%8] ^ w[(r+5)%8] ^ w[(r+7)%8] ^ w[(r+1)%8] ^ PHI ^ (r+2), 11); \
  wo[3] = w[(r+3)%8] =                                                         \
    rol (w[(r+3)%8] ^ w[(r+6)%8] ^ w[(r+0)%8] ^ w[(r+2)%8] ^ PHI ^ (r+3), 11);

#define EXPAND_KEY(r)       \
  EXPAND_KEY4(ws, (r));     \
  EXPAND_KEY4(wt, (r + 4));

  /* Calculate subkeys via S-Boxes, in bitslice mode.  */
  EXPAND_KEY (0); SBOX (3, ws, subkeys[0]); SBOX (2, wt, subkeys[1]);
  EXPAND_KEY (8); SBOX (1, ws, subkeys[2]); SBOX (0, wt, subkeys[3]);
  EXPAND_KEY (16); SBOX (7, ws, subkeys[4]); SBOX (6, wt, subkeys[5]);
  EXPAND_KEY (24); SBOX (5, ws, subkeys[6]); SBOX (4, wt, subkeys[7]);
  EXPAND_KEY (32); SBOX (3, ws, subkeys[8]); SBOX (2, wt, subkeys[9]);
  EXPAND_KEY (40); SBOX (1, ws, subkeys[10]); SBOX (0, wt, subkeys[11]);
  EXPAND_KEY (48); SBOX (7, ws, subkeys[12]); SBOX (6, wt, subkeys[13]);
  EXPAND_KEY (56); SBOX (5, ws, subkeys[14]); SBOX (4, wt, subkeys[15]);
  EXPAND_KEY (64); SBOX (3, ws, subkeys[16]); SBOX (2, wt, subkeys[17]);
  EXPAND_KEY (72); SBOX (1, ws, subkeys[18]); SBOX (0, wt, subkeys[19]);
  EXPAND_KEY (80); SBOX (7, ws, subkeys[20]); SBOX (6, wt, subkeys[21]);
  EXPAND_KEY (88); SBOX (5, ws, subkeys[22]); SBOX (4, wt, subkeys[23]);
  EXPAND_KEY (96); SBOX (3, ws, subkeys[24]); SBOX (2, wt, subkeys[25]);
  EXPAND_KEY (104); SBOX (1, ws, subkeys[26]); SBOX (0, wt, subkeys[27]);
  EXPAND_KEY (112); SBOX (7, ws, subkeys[28]); SBOX (6, wt, subkeys[29]);
  EXPAND_KEY (120); SBOX (5, ws, subkeys[30]); SBOX (4, wt, subkeys[31]);
  EXPAND_KEY4 (ws, 128); SBOX (3, ws, subkeys[32]);

  wipememory (ws, sizeof (ws));
  wipememory (wt, sizeof (wt));
  wipememory (w, sizeof (w));
}

/* Initialize CONTEXT with the key KEY of KEY_LENGTH bits.  */
static gcry_err_code_t
serpent_setkey_internal (serpent_context_t *context,
			 const byte *key, unsigned int key_length)
{
  serpent_key_t key_prepared;

  if (key_length > 32)
    return GPG_ERR_INV_KEYLEN;

  serpent_key_prepare (key, key_length, key_prepared);
  serpent_subkeys_generate (key_prepared, context->keys);

#ifdef USE_AVX2
  context->use_avx2 = 0;
  if ((_gcry_get_hw_features () & HWF_INTEL_AVX2))
    {
      context->use_avx2 = 1;
    }
#endif

#ifdef USE_NEON
  context->use_neon = 0;
  if ((_gcry_get_hw_features () & HWF_ARM_NEON))
    {
      context->use_neon = 1;
    }
#endif

  wipememory (key_prepared, sizeof(key_prepared));
  return 0;
}

/* Initialize CTX with the key KEY of KEY_LENGTH bytes.  */
static gcry_err_code_t
serpent_setkey (void *ctx,
		const byte *key, unsigned int key_length,
                cipher_bulk_ops_t *bulk_ops)
{
  serpent_context_t *context = ctx;
  static const char *serpent_test_ret;
  static int serpent_init_done;
  gcry_err_code_t ret = GPG_ERR_NO_ERROR;

  if (! serpent_init_done)
    {
      /* Execute a self-test the first time, Serpent is used.  */
      serpent_init_done = 1;
      serpent_test_ret = serpent_test ();
      if (serpent_test_ret)
	log_error ("Serpent test failure: %s\n", serpent_test_ret);
    }

  /* Setup bulk encryption routines.  */
  memset (bulk_ops, 0, sizeof(*bulk_ops));
  bulk_ops->cbc_dec = _gcry_serpent_cbc_dec;
  bulk_ops->cfb_dec = _gcry_serpent_cfb_dec;
  bulk_ops->ctr_enc = _gcry_serpent_ctr_enc;
  bulk_ops->ocb_crypt = _gcry_serpent_ocb_crypt;
  bulk_ops->ocb_auth  = _gcry_serpent_ocb_auth;

  if (serpent_test_ret)
    ret = GPG_ERR_SELFTEST_FAILED;
  else
    ret = serpent_setkey_internal (context, key, key_length);

  return ret;
}

static void
serpent_encrypt_internal (serpent_context_t *context,
			  const byte *input, byte *output)
{
  serpent_block_t b, b_next;
  int round = 0;

  b[0] = buf_get_le32 (input + 0);
  b[1] = buf_get_le32 (input + 4);
  b[2] = buf_get_le32 (input + 8);
  b[3] = buf_get_le32 (input + 12);

  ROUND (0, context->keys, b, b_next);
  ROUND (1, context->keys, b, b_next);
  ROUND (2, context->keys, b, b_next);
  ROUND (3, context->keys, b, b_next);
  ROUND (4, context->keys, b, b_next);
  ROUND (5, context->keys, b, b_next);
  ROUND (6, context->keys, b, b_next);
  ROUND (7, context->keys, b, b_next);
  ROUND (0, context->keys, b, b_next);
  ROUND (1, context->keys, b, b_next);
  ROUND (2, context->keys, b, b_next);
  ROUND (3, context->keys, b, b_next);
  ROUND (4, context->keys, b, b_next);
  ROUND (5, context->keys, b, b_next);
  ROUND (6, context->keys, b, b_next);
  ROUND (7, context->keys, b, b_next);
  ROUND (0, context->keys, b, b_next);
  ROUND (1, context->keys, b, b_next);
  ROUND (2, context->keys, b, b_next);
  ROUND (3, context->keys, b, b_next);
  ROUND (4, context->keys, b, b_next);
  ROUND (5, context->keys, b, b_next);
  ROUND (6, context->keys, b, b_next);
  ROUND (7, context->keys, b, b_next);
  ROUND (0, context->keys, b, b_next);
  ROUND (1, context->keys, b, b_next);
  ROUND (2, context->keys, b, b_next);
  ROUND (3, context->keys, b, b_next);
  ROUND (4, context->keys, b, b_next);
  ROUND (5, context->keys, b, b_next);
  ROUND (6, context->keys, b, b_next);

  ROUND_LAST (7, context->keys, b, b_next);

  buf_put_le32 (output + 0, b_next[0]);
  buf_put_le32 (output + 4, b_next[1]);
  buf_put_le32 (output + 8, b_next[2]);
  buf_put_le32 (output + 12, b_next[3]);
}

static void
serpent_decrypt_internal (serpent_context_t *context,
			  const byte *input, byte *output)
{
  serpent_block_t b, b_next;
  int round = ROUNDS;

  b_next[0] = buf_get_le32 (input + 0);
  b_next[1] = buf_get_le32 (input + 4);
  b_next[2] = buf_get_le32 (input + 8);
  b_next[3] = buf_get_le32 (input + 12);

  ROUND_FIRST_INVERSE (7, context->keys, b_next, b);

  ROUND_INVERSE (6, context->keys, b, b_next);
  ROUND_INVERSE (5, context->keys, b, b_next);
  ROUND_INVERSE (4, context->keys, b, b_next);
  ROUND_INVERSE (3, context->keys, b, b_next);
  ROUND_INVERSE (2, context->keys, b, b_next);
  ROUND_INVERSE (1, context->keys, b, b_next);
  ROUND_INVERSE (0, context->keys, b, b_next);
  ROUND_INVERSE (7, context->keys, b, b_next);
  ROUND_INVERSE (6, context->keys, b, b_next);
  ROUND_INVERSE (5, context->keys, b, b_next);
  ROUND_INVERSE (4, context->keys, b, b_next);
  ROUND_INVERSE (3, context->keys, b, b_next);
  ROUND_INVERSE (2, context->keys, b, b_next);
  ROUND_INVERSE (1, context->keys, b, b_next);
  ROUND_INVERSE (0, context->keys, b, b_next);
  ROUND_INVERSE (7, context->keys, b, b_next);
  ROUND_INVERSE (6, context->keys, b, b_next);
  ROUND_INVERSE (5, context->keys, b, b_next);
  ROUND_INVERSE (4, context->keys, b, b_next);
  ROUND_INVERSE (3, context->keys, b, b_next);
  ROUND_INVERSE (2, context->keys, b, b_next);
  ROUND_INVERSE (1, context->keys, b, b_next);
  ROUND_INVERSE (0, context->keys, b, b_next);
  ROUND_INVERSE (7, context->keys, b, b_next);
  ROUND_INVERSE (6, context->keys, b, b_next);
  ROUND_INVERSE (5, context->keys, b, b_next);
  ROUND_INVERSE (4, context->keys, b, b_next);
  ROUND_INVERSE (3, context->keys, b, b_next);
  ROUND_INVERSE (2, context->keys, b, b_next);
  ROUND_INVERSE (1, context->keys, b, b_next);
  ROUND_INVERSE (0, context->keys, b, b_next);

  buf_put_le32 (output + 0, b_next[0]);
  buf_put_le32 (output + 4, b_next[1]);
  buf_put_le32 (output + 8, b_next[2]);
  buf_put_le32 (output + 12, b_next[3]);
}

static unsigned int
serpent_encrypt (void *ctx, byte *buffer_out, const byte *buffer_in)
{
  serpent_context_t *context = ctx;

  serpent_encrypt_internal (context, buffer_in, buffer_out);
  return /*burn_stack*/ (2 * sizeof (serpent_block_t));
}

static unsigned int
serpent_decrypt (void *ctx, byte *buffer_out, const byte *buffer_in)
{
  serpent_context_t *context = ctx;

  serpent_decrypt_internal (context, buffer_in, buffer_out);
  return /*burn_stack*/ (2 * sizeof (serpent_block_t));
}



/* Bulk encryption of complete blocks in CTR mode.  This function is only
   intended for the bulk encryption feature of cipher.c.  CTR is expected to be
   of size sizeof(serpent_block_t). */
static void
_gcry_serpent_ctr_enc(void *context, unsigned char *ctr,
                      void *outbuf_arg, const void *inbuf_arg,
                      size_t nblocks)
{
  serpent_context_t *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned char tmpbuf[sizeof(serpent_block_t)];
  int burn_stack_depth = 2 * sizeof (serpent_block_t);

#ifdef USE_AVX2
  if (ctx->use_avx2)
    {
      int did_use_avx2 = 0;

      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_serpent_avx2_ctr_enc(ctx, outbuf, inbuf, ctr);

          nblocks -= 16;
          outbuf += 16 * sizeof(serpent_block_t);
          inbuf  += 16 * sizeof(serpent_block_t);
          did_use_avx2 = 1;
        }

      if (did_use_avx2)
        {
          /* serpent-avx2 assembly code does not use stack */
          if (nblocks == 0)
            burn_stack_depth = 0;
        }

      /* Use generic/sse2 code to handle smaller chunks... */
      /* TODO: use caching instead? */
    }
#endif

#ifdef USE_SSE2
  {
    int did_use_sse2 = 0;

    /* Process data in 8 block chunks. */
    while (nblocks >= 8)
      {
        _gcry_serpent_sse2_ctr_enc(ctx, outbuf, inbuf, ctr);

        nblocks -= 8;
        outbuf += 8 * sizeof(serpent_block_t);
        inbuf  += 8 * sizeof(serpent_block_t);
        did_use_sse2 = 1;
      }

    if (did_use_sse2)
      {
        /* serpent-sse2 assembly code does not use stack */
        if (nblocks == 0)
          burn_stack_depth = 0;
      }

    /* Use generic code to handle smaller chunks... */
    /* TODO: use caching instead? */
  }
#endif

#ifdef USE_NEON
  if (ctx->use_neon)
    {
      int did_use_neon = 0;

      /* Process data in 8 block chunks. */
      while (nblocks >= 8)
        {
          _gcry_serpent_neon_ctr_enc(ctx, outbuf, inbuf, ctr);

          nblocks -= 8;
          outbuf += 8 * sizeof(serpent_block_t);
          inbuf  += 8 * sizeof(serpent_block_t);
          did_use_neon = 1;
        }

      if (did_use_neon)
        {
          /* serpent-neon assembly code does not use stack */
          if (nblocks == 0)
            burn_stack_depth = 0;
        }

      /* Use generic code to handle smaller chunks... */
      /* TODO: use caching instead? */
    }
#endif

  for ( ;nblocks; nblocks-- )
    {
      /* Encrypt the counter. */
      serpent_encrypt_internal(ctx, ctr, tmpbuf);
      /* XOR the input with the encrypted counter and store in output.  */
      cipher_block_xor(outbuf, tmpbuf, inbuf, sizeof(serpent_block_t));
      outbuf += sizeof(serpent_block_t);
      inbuf  += sizeof(serpent_block_t);
      /* Increment the counter.  */
      cipher_block_add(ctr, 1, sizeof(serpent_block_t));
    }

  wipememory(tmpbuf, sizeof(tmpbuf));
  _gcry_burn_stack(burn_stack_depth);
}

/* Bulk decryption of complete blocks in CBC mode.  This function is only
   intended for the bulk encryption feature of cipher.c. */
static void
_gcry_serpent_cbc_dec(void *context, unsigned char *iv,
                      void *outbuf_arg, const void *inbuf_arg,
                      size_t nblocks)
{
  serpent_context_t *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned char savebuf[sizeof(serpent_block_t)];
  int burn_stack_depth = 2 * sizeof (serpent_block_t);

#ifdef USE_AVX2
  if (ctx->use_avx2)
    {
      int did_use_avx2 = 0;

      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_serpent_avx2_cbc_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 16;
          outbuf += 16 * sizeof(serpent_block_t);
          inbuf  += 16 * sizeof(serpent_block_t);
          did_use_avx2 = 1;
        }

      if (did_use_avx2)
        {
          /* serpent-avx2 assembly code does not use stack */
          if (nblocks == 0)
            burn_stack_depth = 0;
        }

      /* Use generic/sse2 code to handle smaller chunks... */
    }
#endif

#ifdef USE_SSE2
  {
    int did_use_sse2 = 0;

    /* Process data in 8 block chunks. */
    while (nblocks >= 8)
      {
        _gcry_serpent_sse2_cbc_dec(ctx, outbuf, inbuf, iv);

        nblocks -= 8;
        outbuf += 8 * sizeof(serpent_block_t);
        inbuf  += 8 * sizeof(serpent_block_t);
        did_use_sse2 = 1;
      }

    if (did_use_sse2)
      {
        /* serpent-sse2 assembly code does not use stack */
        if (nblocks == 0)
          burn_stack_depth = 0;
      }

    /* Use generic code to handle smaller chunks... */
  }
#endif

#ifdef USE_NEON
  if (ctx->use_neon)
    {
      int did_use_neon = 0;

      /* Process data in 8 block chunks. */
      while (nblocks >= 8)
        {
          _gcry_serpent_neon_cbc_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 8;
          outbuf += 8 * sizeof(serpent_block_t);
          inbuf  += 8 * sizeof(serpent_block_t);
          did_use_neon = 1;
        }

      if (did_use_neon)
        {
          /* serpent-neon assembly code does not use stack */
          if (nblocks == 0)
            burn_stack_depth = 0;
        }

      /* Use generic code to handle smaller chunks... */
    }
#endif

  for ( ;nblocks; nblocks-- )
    {
      /* INBUF is needed later and it may be identical to OUTBUF, so store
         the intermediate result to SAVEBUF.  */
      serpent_decrypt_internal (ctx, inbuf, savebuf);

      cipher_block_xor_n_copy_2(outbuf, savebuf, iv, inbuf,
                                sizeof(serpent_block_t));
      inbuf += sizeof(serpent_block_t);
      outbuf += sizeof(serpent_block_t);
    }

  wipememory(savebuf, sizeof(savebuf));
  _gcry_burn_stack(burn_stack_depth);
}

/* Bulk decryption of complete blocks in CFB mode.  This function is only
   intended for the bulk encryption feature of cipher.c. */
static void
_gcry_serpent_cfb_dec(void *context, unsigned char *iv,
                      void *outbuf_arg, const void *inbuf_arg,
                      size_t nblocks)
{
  serpent_context_t *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  int burn_stack_depth = 2 * sizeof (serpent_block_t);

#ifdef USE_AVX2
  if (ctx->use_avx2)
    {
      int did_use_avx2 = 0;

      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_serpent_avx2_cfb_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 16;
          outbuf += 16 * sizeof(serpent_block_t);
          inbuf  += 16 * sizeof(serpent_block_t);
          did_use_avx2 = 1;
        }

      if (did_use_avx2)
        {
          /* serpent-avx2 assembly code does not use stack */
          if (nblocks == 0)
            burn_stack_depth = 0;
        }

      /* Use generic/sse2 code to handle smaller chunks... */
    }
#endif

#ifdef USE_SSE2
  {
    int did_use_sse2 = 0;

    /* Process data in 8 block chunks. */
    while (nblocks >= 8)
      {
        _gcry_serpent_sse2_cfb_dec(ctx, outbuf, inbuf, iv);

        nblocks -= 8;
        outbuf += 8 * sizeof(serpent_block_t);
        inbuf  += 8 * sizeof(serpent_block_t);
        did_use_sse2 = 1;
      }

    if (did_use_sse2)
      {
        /* serpent-sse2 assembly code does not use stack */
        if (nblocks == 0)
          burn_stack_depth = 0;
      }

    /* Use generic code to handle smaller chunks... */
  }
#endif

#ifdef USE_NEON
  if (ctx->use_neon)
    {
      int did_use_neon = 0;

      /* Process data in 8 block chunks. */
      while (nblocks >= 8)
        {
          _gcry_serpent_neon_cfb_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 8;
          outbuf += 8 * sizeof(serpent_block_t);
          inbuf  += 8 * sizeof(serpent_block_t);
          did_use_neon = 1;
        }

      if (did_use_neon)
        {
          /* serpent-neon assembly code does not use stack */
          if (nblocks == 0)
            burn_stack_depth = 0;
        }

      /* Use generic code to handle smaller chunks... */
    }
#endif

  for ( ;nblocks; nblocks-- )
    {
      serpent_encrypt_internal(ctx, iv, iv);
      cipher_block_xor_n_copy(outbuf, iv, inbuf, sizeof(serpent_block_t));
      outbuf += sizeof(serpent_block_t);
      inbuf  += sizeof(serpent_block_t);
    }

  _gcry_burn_stack(burn_stack_depth);
}

/* Bulk encryption/decryption of complete blocks in OCB mode. */
static size_t
_gcry_serpent_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
			const void *inbuf_arg, size_t nblocks, int encrypt)
{
#if defined(USE_AVX2) || defined(USE_SSE2) || defined(USE_NEON)
  serpent_context_t *ctx = (void *)&c->context.c;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  int burn_stack_depth = 2 * sizeof (serpent_block_t);
  u64 blkn = c->u_mode.ocb.data_nblocks;
#else
  (void)c;
  (void)outbuf_arg;
  (void)inbuf_arg;
  (void)encrypt;
#endif

#ifdef USE_AVX2
  if (ctx->use_avx2)
    {
      int did_use_avx2 = 0;
      u64 Ls[16];
      unsigned int n = 16 - (blkn % 16);
      u64 *l;
      int i;

      if (nblocks >= 16)
	{
	  for (i = 0; i < 16; i += 8)
	    {
	      /* Use u64 to store pointers for x32 support (assembly function
	       * assumes 64-bit pointers). */
	      Ls[(i + 0 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 1 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 2 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 3 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[2];
	      Ls[(i + 4 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 5 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 6 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	    }

	  Ls[(7 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  l = &Ls[(15 + n) % 16];

	  /* Process data in 16 block chunks. */
	  while (nblocks >= 16)
	    {
	      blkn += 16;
	      *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 16);

	      if (encrypt)
		_gcry_serpent_avx2_ocb_enc(ctx, outbuf, inbuf, c->u_iv.iv,
					  c->u_ctr.ctr, Ls);
	      else
		_gcry_serpent_avx2_ocb_dec(ctx, outbuf, inbuf, c->u_iv.iv,
					  c->u_ctr.ctr, Ls);

	      nblocks -= 16;
	      outbuf += 16 * sizeof(serpent_block_t);
	      inbuf  += 16 * sizeof(serpent_block_t);
	      did_use_avx2 = 1;
	    }
	}

      if (did_use_avx2)
	{
	  /* serpent-avx2 assembly code does not use stack */
	  if (nblocks == 0)
	    burn_stack_depth = 0;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#ifdef USE_SSE2
  {
    int did_use_sse2 = 0;
    u64 Ls[8];
    unsigned int n = 8 - (blkn % 8);
    u64 *l;

    if (nblocks >= 8)
      {
	/* Use u64 to store pointers for x32 support (assembly function
	  * assumes 64-bit pointers). */
	Ls[(0 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	Ls[(1 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	Ls[(2 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	Ls[(3 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[2];
	Ls[(4 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	Ls[(5 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	Ls[(6 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	l = &Ls[(7 + n) % 8];

	/* Process data in 8 block chunks. */
	while (nblocks >= 8)
	  {
	    blkn += 8;
	    *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 8);

	    if (encrypt)
	      _gcry_serpent_sse2_ocb_enc(ctx, outbuf, inbuf, c->u_iv.iv,
					  c->u_ctr.ctr, Ls);
	    else
	      _gcry_serpent_sse2_ocb_dec(ctx, outbuf, inbuf, c->u_iv.iv,
					  c->u_ctr.ctr, Ls);

	    nblocks -= 8;
	    outbuf += 8 * sizeof(serpent_block_t);
	    inbuf  += 8 * sizeof(serpent_block_t);
	    did_use_sse2 = 1;
	  }
      }

    if (did_use_sse2)
      {
	/* serpent-sse2 assembly code does not use stack */
	if (nblocks == 0)
	  burn_stack_depth = 0;
      }

    /* Use generic code to handle smaller chunks... */
  }
#endif

#ifdef USE_NEON
  if (ctx->use_neon)
    {
      int did_use_neon = 0;
      const void *Ls[8];
      unsigned int n = 8 - (blkn % 8);
      const void **l;

      if (nblocks >= 8)
	{
	  Ls[(0 + n) % 8] = c->u_mode.ocb.L[0];
	  Ls[(1 + n) % 8] = c->u_mode.ocb.L[1];
	  Ls[(2 + n) % 8] = c->u_mode.ocb.L[0];
	  Ls[(3 + n) % 8] = c->u_mode.ocb.L[2];
	  Ls[(4 + n) % 8] = c->u_mode.ocb.L[0];
	  Ls[(5 + n) % 8] = c->u_mode.ocb.L[1];
	  Ls[(6 + n) % 8] = c->u_mode.ocb.L[0];
	  l = &Ls[(7 + n) % 8];

	  /* Process data in 8 block chunks. */
	  while (nblocks >= 8)
	    {
	      blkn += 8;
	      *l = ocb_get_l(c,  blkn - blkn % 8);

	      if (encrypt)
		_gcry_serpent_neon_ocb_enc(ctx, outbuf, inbuf, c->u_iv.iv,
					  c->u_ctr.ctr, Ls);
	      else
		_gcry_serpent_neon_ocb_dec(ctx, outbuf, inbuf, c->u_iv.iv,
					  c->u_ctr.ctr, Ls);

	      nblocks -= 8;
	      outbuf += 8 * sizeof(serpent_block_t);
	      inbuf  += 8 * sizeof(serpent_block_t);
	      did_use_neon = 1;
	    }
	}

      if (did_use_neon)
	{
	  /* serpent-neon assembly code does not use stack */
	  if (nblocks == 0)
	    burn_stack_depth = 0;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#if defined(USE_AVX2) || defined(USE_SSE2) || defined(USE_NEON)
  c->u_mode.ocb.data_nblocks = blkn;

  if (burn_stack_depth)
    _gcry_burn_stack (burn_stack_depth + 4 * sizeof(void *));
#endif

  return nblocks;
}

/* Bulk authentication of complete blocks in OCB mode. */
static size_t
_gcry_serpent_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
			size_t nblocks)
{
#if defined(USE_AVX2) || defined(USE_SSE2) || defined(USE_NEON)
  serpent_context_t *ctx = (void *)&c->context.c;
  const unsigned char *abuf = abuf_arg;
  int burn_stack_depth = 2 * sizeof(serpent_block_t);
  u64 blkn = c->u_mode.ocb.aad_nblocks;
#else
  (void)c;
  (void)abuf_arg;
#endif

#ifdef USE_AVX2
  if (ctx->use_avx2)
    {
      int did_use_avx2 = 0;
      u64 Ls[16];
      unsigned int n = 16 - (blkn % 16);
      u64 *l;
      int i;

      if (nblocks >= 16)
	{
	  for (i = 0; i < 16; i += 8)
	    {
	      /* Use u64 to store pointers for x32 support (assembly function
	       * assumes 64-bit pointers). */
	      Ls[(i + 0 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 1 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 2 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 3 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[2];
	      Ls[(i + 4 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 5 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 6 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	    }

	  Ls[(7 + n) % 16] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  l = &Ls[(15 + n) % 16];

	  /* Process data in 16 block chunks. */
	  while (nblocks >= 16)
	    {
	      blkn += 16;
	      *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 16);

	      _gcry_serpent_avx2_ocb_auth(ctx, abuf, c->u_mode.ocb.aad_offset,
					  c->u_mode.ocb.aad_sum, Ls);

	      nblocks -= 16;
	      abuf += 16 * sizeof(serpent_block_t);
	      did_use_avx2 = 1;
	    }
	}

      if (did_use_avx2)
	{
	  /* serpent-avx2 assembly code does not use stack */
	  if (nblocks == 0)
	    burn_stack_depth = 0;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#ifdef USE_SSE2
  {
    int did_use_sse2 = 0;
    u64 Ls[8];
    unsigned int n = 8 - (blkn % 8);
    u64 *l;

    if (nblocks >= 8)
      {
	/* Use u64 to store pointers for x32 support (assembly function
	* assumes 64-bit pointers). */
	Ls[(0 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	Ls[(1 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	Ls[(2 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	Ls[(3 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[2];
	Ls[(4 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	Ls[(5 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	Ls[(6 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	l = &Ls[(7 + n) % 8];

	/* Process data in 8 block chunks. */
	while (nblocks >= 8)
	  {
	    blkn += 8;
	    *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 8);

	    _gcry_serpent_sse2_ocb_auth(ctx, abuf, c->u_mode.ocb.aad_offset,
					c->u_mode.ocb.aad_sum, Ls);

	    nblocks -= 8;
	    abuf += 8 * sizeof(serpent_block_t);
	    did_use_sse2 = 1;
	  }
      }

    if (did_use_sse2)
      {
	/* serpent-avx2 assembly code does not use stack */
	if (nblocks == 0)
	  burn_stack_depth = 0;
      }

    /* Use generic code to handle smaller chunks... */
  }
#endif

#ifdef USE_NEON
  if (ctx->use_neon)
    {
      int did_use_neon = 0;
      const void *Ls[8];
      unsigned int n = 8 - (blkn % 8);
      const void **l;

      if (nblocks >= 8)
	{
	  Ls[(0 + n) % 8] = c->u_mode.ocb.L[0];
	  Ls[(1 + n) % 8] = c->u_mode.ocb.L[1];
	  Ls[(2 + n) % 8] = c->u_mode.ocb.L[0];
	  Ls[(3 + n) % 8] = c->u_mode.ocb.L[2];
	  Ls[(4 + n) % 8] = c->u_mode.ocb.L[0];
	  Ls[(5 + n) % 8] = c->u_mode.ocb.L[1];
	  Ls[(6 + n) % 8] = c->u_mode.ocb.L[0];
	  l = &Ls[(7 + n) % 8];

	  /* Process data in 8 block chunks. */
	  while (nblocks >= 8)
	    {
	      blkn += 8;
	      *l = ocb_get_l(c, blkn - blkn % 8);

	      _gcry_serpent_neon_ocb_auth(ctx, abuf, c->u_mode.ocb.aad_offset,
					  c->u_mode.ocb.aad_sum, Ls);

	      nblocks -= 8;
	      abuf += 8 * sizeof(serpent_block_t);
	      did_use_neon = 1;
	    }
	}

      if (did_use_neon)
	{
	  /* serpent-neon assembly code does not use stack */
	  if (nblocks == 0)
	    burn_stack_depth = 0;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#if defined(USE_AVX2) || defined(USE_SSE2) || defined(USE_NEON)
  c->u_mode.ocb.aad_nblocks = blkn;

  if (burn_stack_depth)
    _gcry_burn_stack (burn_stack_depth + 4 * sizeof(void *));
#endif

  return nblocks;
}



/* Run the self-tests for SERPENT-CTR-128, tests IV increment of bulk CTR
   encryption.  Returns NULL on success. */
static const char*
selftest_ctr_128 (void)
{
  const int nblocks = 16+8+1;
  const int blocksize = sizeof(serpent_block_t);
  const int context_size = sizeof(serpent_context_t);

  return _gcry_selftest_helper_ctr("SERPENT", &serpent_setkey,
           &serpent_encrypt, nblocks, blocksize, context_size);
}


/* Run the self-tests for SERPENT-CBC-128, tests bulk CBC decryption.
   Returns NULL on success. */
static const char*
selftest_cbc_128 (void)
{
  const int nblocks = 16+8+2;
  const int blocksize = sizeof(serpent_block_t);
  const int context_size = sizeof(serpent_context_t);

  return _gcry_selftest_helper_cbc("SERPENT", &serpent_setkey,
           &serpent_encrypt, nblocks, blocksize, context_size);
}


/* Run the self-tests for SERPENT-CBC-128, tests bulk CBC decryption.
   Returns NULL on success. */
static const char*
selftest_cfb_128 (void)
{
  const int nblocks = 16+8+2;
  const int blocksize = sizeof(serpent_block_t);
  const int context_size = sizeof(serpent_context_t);

  return _gcry_selftest_helper_cfb("SERPENT", &serpent_setkey,
           &serpent_encrypt, nblocks, blocksize, context_size);
}


/* Serpent test.  */

static const char *
serpent_test (void)
{
  serpent_context_t context;
  unsigned char scratch[16];
  unsigned int i;
  const char *r;

  static struct test
  {
    int key_length;
    unsigned char key[32];
    unsigned char text_plain[16];
    unsigned char text_cipher[16];
  } test_data[] =
    {
      {
	16,
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	"\xD2\x9D\x57\x6F\xCE\xA3\xA3\xA7\xED\x90\x99\xF2\x92\x73\xD7\x8E",
	"\xB2\x28\x8B\x96\x8A\xE8\xB0\x86\x48\xD1\xCE\x96\x06\xFD\x99\x2D"
      },
      {
	24,
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00",
	"\xD2\x9D\x57\x6F\xCE\xAB\xA3\xA7\xED\x98\x99\xF2\x92\x7B\xD7\x8E",
	"\x13\x0E\x35\x3E\x10\x37\xC2\x24\x05\xE8\xFA\xEF\xB2\xC3\xC3\xE9"
      },
      {
	32,
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	"\xD0\x95\x57\x6F\xCE\xA3\xE3\xA7\xED\x98\xD9\xF2\x90\x73\xD7\x8E",
	"\xB9\x0E\xE5\x86\x2D\xE6\x91\x68\xF2\xBD\xD5\x12\x5B\x45\x47\x2B"
      },
      {
	32,
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	"\x00\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00",
	"\x20\x61\xA4\x27\x82\xBD\x52\xEC\x69\x1E\xC3\x83\xB0\x3B\xA7\x7C"
      },
      {
	0
      },
    };

  for (i = 0; test_data[i].key_length; i++)
    {
      serpent_setkey_internal (&context, test_data[i].key,
                               test_data[i].key_length);
      serpent_encrypt_internal (&context, test_data[i].text_plain, scratch);

      if (memcmp (scratch, test_data[i].text_cipher, sizeof (serpent_block_t)))
	switch (test_data[i].key_length)
	  {
	  case 16:
	    return "Serpent-128 test encryption failed.";
	  case  24:
	    return "Serpent-192 test encryption failed.";
	  case 32:
	    return "Serpent-256 test encryption failed.";
	  }

    serpent_decrypt_internal (&context, test_data[i].text_cipher, scratch);
    if (memcmp (scratch, test_data[i].text_plain, sizeof (serpent_block_t)))
      switch (test_data[i].key_length)
	{
	case 16:
	  return "Serpent-128 test decryption failed.";
	case  24:
	  return "Serpent-192 test decryption failed.";
	case 32:
	  return "Serpent-256 test decryption failed.";
	}
    }

  if ( (r = selftest_ctr_128 ()) )
    return r;

  if ( (r = selftest_cbc_128 ()) )
    return r;

  if ( (r = selftest_cfb_128 ()) )
    return r;

  return NULL;
}


static const gcry_cipher_oid_spec_t serpent128_oids[] =
  {
    {"1.3.6.1.4.1.11591.13.2.1", GCRY_CIPHER_MODE_ECB },
    {"1.3.6.1.4.1.11591.13.2.2", GCRY_CIPHER_MODE_CBC },
    {"1.3.6.1.4.1.11591.13.2.3", GCRY_CIPHER_MODE_OFB },
    {"1.3.6.1.4.1.11591.13.2.4", GCRY_CIPHER_MODE_CFB },
    { NULL }
  };

static const gcry_cipher_oid_spec_t serpent192_oids[] =
  {
    {"1.3.6.1.4.1.11591.13.2.21", GCRY_CIPHER_MODE_ECB },
    {"1.3.6.1.4.1.11591.13.2.22", GCRY_CIPHER_MODE_CBC },
    {"1.3.6.1.4.1.11591.13.2.23", GCRY_CIPHER_MODE_OFB },
    {"1.3.6.1.4.1.11591.13.2.24", GCRY_CIPHER_MODE_CFB },
    { NULL }
  };

static const gcry_cipher_oid_spec_t serpent256_oids[] =
  {
    {"1.3.6.1.4.1.11591.13.2.41", GCRY_CIPHER_MODE_ECB },
    {"1.3.6.1.4.1.11591.13.2.42", GCRY_CIPHER_MODE_CBC },
    {"1.3.6.1.4.1.11591.13.2.43", GCRY_CIPHER_MODE_OFB },
    {"1.3.6.1.4.1.11591.13.2.44", GCRY_CIPHER_MODE_CFB },
    { NULL }
  };

static const char *serpent128_aliases[] =
  {
    "SERPENT",
    "SERPENT-128",
    NULL
  };
static const char *serpent192_aliases[] =
  {
    "SERPENT-192",
    NULL
  };
static const char *serpent256_aliases[] =
  {
    "SERPENT-256",
    NULL
  };

gcry_cipher_spec_t _gcry_cipher_spec_serpent128 =
  {
    GCRY_CIPHER_SERPENT128, {0, 0},
    "SERPENT128", serpent128_aliases, serpent128_oids, 16, 128,
    sizeof (serpent_context_t),
    serpent_setkey, serpent_encrypt, serpent_decrypt
  };

gcry_cipher_spec_t _gcry_cipher_spec_serpent192 =
  {
    GCRY_CIPHER_SERPENT192, {0, 0},
    "SERPENT192", serpent192_aliases, serpent192_oids, 16, 192,
    sizeof (serpent_context_t),
    serpent_setkey, serpent_encrypt, serpent_decrypt
  };

gcry_cipher_spec_t _gcry_cipher_spec_serpent256 =
  {
    GCRY_CIPHER_SERPENT256, {0, 0},
    "SERPENT256", serpent256_aliases, serpent256_oids, 16, 256,
    sizeof (serpent_context_t),
    serpent_setkey, serpent_encrypt, serpent_decrypt
  };
