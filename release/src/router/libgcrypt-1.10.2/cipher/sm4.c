/* sm4.c  -  SM4 Cipher Algorithm
 * Copyright (C) 2020 Alibaba Group.
 * Copyright (C) 2020 Tianjia Zhang <tianjia.zhang@linux.alibaba.com>
 * Copyright (C) 2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"  /* for byte and u32 typedefs */
#include "bithelp.h"
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "cipher-internal.h"
#include "cipher-selftest.h"

/* Helper macro to force alignment to 64 bytes.  */
#ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
# define ATTR_ALIGNED_64  __attribute__ ((aligned (64)))
#else
# define ATTR_ALIGNED_64
#endif

/* USE_AESNI_AVX inidicates whether to compile with Intel AES-NI/AVX code. */
#undef USE_AESNI_AVX
#if defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)
# if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
#  define USE_AESNI_AVX 1
# endif
#endif

/* USE_AESNI_AVX inidicates whether to compile with Intel AES-NI/AVX2 code. */
#undef USE_AESNI_AVX2
#if defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX2_SUPPORT)
# if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
#  define USE_AESNI_AVX2 1
# endif
#endif

/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2)
# ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
#  define ASM_FUNC_ABI __attribute__((sysv_abi))
# else
#  define ASM_FUNC_ABI
# endif
#endif

static const char *sm4_selftest (void);

static void _gcry_sm4_ctr_enc (void *context, unsigned char *ctr,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks);
static void _gcry_sm4_cbc_dec (void *context, unsigned char *iv,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks);
static void _gcry_sm4_cfb_dec (void *context, unsigned char *iv,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks);
static size_t _gcry_sm4_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
				   const void *inbuf_arg, size_t nblocks,
				   int encrypt);
static size_t _gcry_sm4_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
				  size_t nblocks);

typedef struct
{
  u32 rkey_enc[32];
  u32 rkey_dec[32];
#ifdef USE_AESNI_AVX
  unsigned int use_aesni_avx:1;
#endif
#ifdef USE_AESNI_AVX2
  unsigned int use_aesni_avx2:1;
#endif
} SM4_context;

static const u32 fk[4] =
{
  0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc
};

static struct
{
  volatile u32 counter_head;
  u32 cacheline_align[64 / 4 - 1];
  byte S[256];
  volatile u32 counter_tail;
} sbox_table ATTR_ALIGNED_64 =
  {
    0,
    { 0, },
    {
      0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7,
      0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
      0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3,
      0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
      0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a,
      0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
      0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95,
      0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
      0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba,
      0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
      0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b,
      0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
      0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2,
      0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
      0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52,
      0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
      0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5,
      0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
      0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55,
      0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
      0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60,
      0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
      0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f,
      0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
      0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f,
      0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
      0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd,
      0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
      0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e,
      0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
      0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20,
      0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
    },
    0
  };

static const u32 ck[] =
{
  0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
  0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
  0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
  0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
  0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
  0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
  0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
  0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

#ifdef USE_AESNI_AVX
extern void _gcry_sm4_aesni_avx_expand_key(const byte *key, u32 *rk_enc,
					   u32 *rk_dec, const u32 *fk,
					   const u32 *ck) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx_ctr_enc(const u32 *rk_enc, byte *out,
					const byte *in, byte *ctr) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx_cbc_dec(const u32 *rk_dec, byte *out,
					const byte *in, byte *iv) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx_cfb_dec(const u32 *rk_enc, byte *out,
					const byte *in, byte *iv) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx_ocb_enc(const u32 *rk_enc,
					unsigned char *out,
					const unsigned char *in,
					unsigned char *offset,
					unsigned char *checksum,
					const u64 Ls[8]) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx_ocb_dec(const u32 *rk_dec,
					unsigned char *out,
					const unsigned char *in,
					unsigned char *offset,
					unsigned char *checksum,
					const u64 Ls[8]) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx_ocb_auth(const u32 *rk_enc,
					 const unsigned char *abuf,
					 unsigned char *offset,
					 unsigned char *checksum,
					 const u64 Ls[8]) ASM_FUNC_ABI;

extern unsigned int
_gcry_sm4_aesni_avx_crypt_blk1_8(const u32 *rk, byte *out, const byte *in,
				 unsigned int num_blks) ASM_FUNC_ABI;

static inline unsigned int
sm4_aesni_avx_crypt_blk1_8(const u32 *rk, byte *out, const byte *in,
			   unsigned int num_blks)
{
  return _gcry_sm4_aesni_avx_crypt_blk1_8(rk, out, in, num_blks);
}

#endif /* USE_AESNI_AVX */

#ifdef USE_AESNI_AVX2
extern void _gcry_sm4_aesni_avx2_ctr_enc(const u32 *rk_enc, byte *out,
					 const byte *in,
					 byte *ctr) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx2_cbc_dec(const u32 *rk_dec, byte *out,
					 const byte *in,
					 byte *iv) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx2_cfb_dec(const u32 *rk_enc, byte *out,
					 const byte *in,
					 byte *iv) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx2_ocb_enc(const u32 *rk_enc,
					 unsigned char *out,
					 const unsigned char *in,
					 unsigned char *offset,
					 unsigned char *checksum,
					 const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx2_ocb_dec(const u32 *rk_dec,
					 unsigned char *out,
					 const unsigned char *in,
					 unsigned char *offset,
					 unsigned char *checksum,
					 const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_sm4_aesni_avx2_ocb_auth(const u32 *rk_enc,
					  const unsigned char *abuf,
					  unsigned char *offset,
					  unsigned char *checksum,
					  const u64 Ls[16]) ASM_FUNC_ABI;
#endif /* USE_AESNI_AVX2 */

static inline void prefetch_sbox_table(void)
{
  const volatile byte *vtab = (void *)&sbox_table;

  /* Modify counters to trigger copy-on-write and unsharing if physical pages
   * of look-up table are shared between processes.  Modifying counters also
   * causes checksums for pages to change and hint same-page merging algorithm
   * that these pages are frequently changing.  */
  sbox_table.counter_head++;
  sbox_table.counter_tail++;

  /* Prefetch look-up table to cache.  */
  (void)vtab[0 * 32];
  (void)vtab[1 * 32];
  (void)vtab[2 * 32];
  (void)vtab[3 * 32];
  (void)vtab[4 * 32];
  (void)vtab[5 * 32];
  (void)vtab[6 * 32];
  (void)vtab[7 * 32];
  (void)vtab[8 * 32 - 1];
}

static inline u32 sm4_t_non_lin_sub(u32 x)
{
  u32 out;

  out  = (u32)sbox_table.S[(x >> 0) & 0xff] << 0;
  out |= (u32)sbox_table.S[(x >> 8) & 0xff] << 8;
  out |= (u32)sbox_table.S[(x >> 16) & 0xff] << 16;
  out |= (u32)sbox_table.S[(x >> 24) & 0xff] << 24;

  return out;
}

static inline u32 sm4_key_lin_sub(u32 x)
{
  return x ^ rol(x, 13) ^ rol(x, 23);
}

static inline u32 sm4_enc_lin_sub(u32 x)
{
  u32 xrol2 = rol(x, 2);
  return x ^ xrol2 ^ rol(xrol2, 8) ^ rol(xrol2, 16) ^ rol(x, 24);
}

static inline u32 sm4_key_sub(u32 x)
{
  return sm4_key_lin_sub(sm4_t_non_lin_sub(x));
}

static inline u32 sm4_enc_sub(u32 x)
{
  return sm4_enc_lin_sub(sm4_t_non_lin_sub(x));
}

static inline u32
sm4_round(const u32 x0, const u32 x1, const u32 x2, const u32 x3, const u32 rk)
{
  return x0 ^ sm4_enc_sub(x1 ^ x2 ^ x3 ^ rk);
}

static void
sm4_expand_key (SM4_context *ctx, const byte *key)
{
  u32 rk[4];
  int i;

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      _gcry_sm4_aesni_avx_expand_key (key, ctx->rkey_enc, ctx->rkey_dec,
				      fk, ck);
      return;
    }
#endif

  rk[0] = buf_get_be32(key + 4 * 0) ^ fk[0];
  rk[1] = buf_get_be32(key + 4 * 1) ^ fk[1];
  rk[2] = buf_get_be32(key + 4 * 2) ^ fk[2];
  rk[3] = buf_get_be32(key + 4 * 3) ^ fk[3];

  for (i = 0; i < 32; i += 4)
    {
      rk[0] = rk[0] ^ sm4_key_sub(rk[1] ^ rk[2] ^ rk[3] ^ ck[i + 0]);
      rk[1] = rk[1] ^ sm4_key_sub(rk[2] ^ rk[3] ^ rk[0] ^ ck[i + 1]);
      rk[2] = rk[2] ^ sm4_key_sub(rk[3] ^ rk[0] ^ rk[1] ^ ck[i + 2]);
      rk[3] = rk[3] ^ sm4_key_sub(rk[0] ^ rk[1] ^ rk[2] ^ ck[i + 3]);
      ctx->rkey_enc[i + 0] = rk[0];
      ctx->rkey_enc[i + 1] = rk[1];
      ctx->rkey_enc[i + 2] = rk[2];
      ctx->rkey_enc[i + 3] = rk[3];
      ctx->rkey_dec[31 - i - 0] = rk[0];
      ctx->rkey_dec[31 - i - 1] = rk[1];
      ctx->rkey_dec[31 - i - 2] = rk[2];
      ctx->rkey_dec[31 - i - 3] = rk[3];
    }

  wipememory (rk, sizeof(rk));
}

static gcry_err_code_t
sm4_setkey (void *context, const byte *key, const unsigned keylen,
            cipher_bulk_ops_t *bulk_ops)
{
  SM4_context *ctx = context;
  static int init = 0;
  static const char *selftest_failed = NULL;
  unsigned int hwf = _gcry_get_hw_features ();

  (void)hwf;

  if (!init)
    {
      init = 1;
      selftest_failed = sm4_selftest();
      if (selftest_failed)
	log_error("%s\n", selftest_failed);
    }
  if (selftest_failed)
    return GPG_ERR_SELFTEST_FAILED;

  if (keylen != 16)
    return GPG_ERR_INV_KEYLEN;

#ifdef USE_AESNI_AVX
  ctx->use_aesni_avx = (hwf & HWF_INTEL_AESNI) && (hwf & HWF_INTEL_AVX);
#endif
#ifdef USE_AESNI_AVX2
  ctx->use_aesni_avx2 = (hwf & HWF_INTEL_AESNI) && (hwf & HWF_INTEL_AVX2);
#endif

  /* Setup bulk encryption routines.  */
  memset (bulk_ops, 0, sizeof(*bulk_ops));
  bulk_ops->cbc_dec = _gcry_sm4_cbc_dec;
  bulk_ops->cfb_dec = _gcry_sm4_cfb_dec;
  bulk_ops->ctr_enc = _gcry_sm4_ctr_enc;
  bulk_ops->ocb_crypt = _gcry_sm4_ocb_crypt;
  bulk_ops->ocb_auth  = _gcry_sm4_ocb_auth;

  sm4_expand_key (ctx, key);
  return 0;
}

static unsigned int
sm4_do_crypt (const u32 *rk, byte *out, const byte *in)
{
  u32 x[4];
  int i;

  x[0] = buf_get_be32(in + 0 * 4);
  x[1] = buf_get_be32(in + 1 * 4);
  x[2] = buf_get_be32(in + 2 * 4);
  x[3] = buf_get_be32(in + 3 * 4);

  for (i = 0; i < 32; i += 4)
    {
      x[0] = sm4_round(x[0], x[1], x[2], x[3], rk[i + 0]);
      x[1] = sm4_round(x[1], x[2], x[3], x[0], rk[i + 1]);
      x[2] = sm4_round(x[2], x[3], x[0], x[1], rk[i + 2]);
      x[3] = sm4_round(x[3], x[0], x[1], x[2], rk[i + 3]);
    }

  buf_put_be32(out + 0 * 4, x[3 - 0]);
  buf_put_be32(out + 1 * 4, x[3 - 1]);
  buf_put_be32(out + 2 * 4, x[3 - 2]);
  buf_put_be32(out + 3 * 4, x[3 - 3]);

  return /*burn_stack*/ 4*6+sizeof(void*)*4;
}

static unsigned int
sm4_encrypt (void *context, byte *outbuf, const byte *inbuf)
{
  SM4_context *ctx = context;

  prefetch_sbox_table ();

  return sm4_do_crypt (ctx->rkey_enc, outbuf, inbuf);
}

static unsigned int
sm4_decrypt (void *context, byte *outbuf, const byte *inbuf)
{
  SM4_context *ctx = context;

  prefetch_sbox_table ();

  return sm4_do_crypt (ctx->rkey_dec, outbuf, inbuf);
}

static unsigned int
sm4_do_crypt_blks2 (const u32 *rk, byte *out, const byte *in)
{
  u32 x[4];
  u32 y[4];
  u32 k;
  int i;

  /* Encrypts/Decrypts two blocks for higher instruction level
   * parallelism. */

  x[0] = buf_get_be32(in + 0 * 4);
  x[1] = buf_get_be32(in + 1 * 4);
  x[2] = buf_get_be32(in + 2 * 4);
  x[3] = buf_get_be32(in + 3 * 4);
  y[0] = buf_get_be32(in + 4 * 4);
  y[1] = buf_get_be32(in + 5 * 4);
  y[2] = buf_get_be32(in + 6 * 4);
  y[3] = buf_get_be32(in + 7 * 4);

  for (i = 0; i < 32; i += 4)
    {
      k = rk[i + 0];
      x[0] = sm4_round(x[0], x[1], x[2], x[3], k);
      y[0] = sm4_round(y[0], y[1], y[2], y[3], k);
      k = rk[i + 1];
      x[1] = sm4_round(x[1], x[2], x[3], x[0], k);
      y[1] = sm4_round(y[1], y[2], y[3], y[0], k);
      k = rk[i + 2];
      x[2] = sm4_round(x[2], x[3], x[0], x[1], k);
      y[2] = sm4_round(y[2], y[3], y[0], y[1], k);
      k = rk[i + 3];
      x[3] = sm4_round(x[3], x[0], x[1], x[2], k);
      y[3] = sm4_round(y[3], y[0], y[1], y[2], k);
    }

  buf_put_be32(out + 0 * 4, x[3 - 0]);
  buf_put_be32(out + 1 * 4, x[3 - 1]);
  buf_put_be32(out + 2 * 4, x[3 - 2]);
  buf_put_be32(out + 3 * 4, x[3 - 3]);
  buf_put_be32(out + 4 * 4, y[3 - 0]);
  buf_put_be32(out + 5 * 4, y[3 - 1]);
  buf_put_be32(out + 6 * 4, y[3 - 2]);
  buf_put_be32(out + 7 * 4, y[3 - 3]);

  return /*burn_stack*/ 4*10+sizeof(void*)*4;
}

static unsigned int
sm4_crypt_blocks (const u32 *rk, byte *out, const byte *in,
		  unsigned int num_blks)
{
  unsigned int burn_depth = 0;
  unsigned int nburn;

  while (num_blks >= 2)
    {
      nburn = sm4_do_crypt_blks2 (rk, out, in);
      burn_depth = nburn > burn_depth ? nburn : burn_depth;
      out += 2 * 16;
      in += 2 * 16;
      num_blks -= 2;
    }

  while (num_blks)
    {
      nburn = sm4_do_crypt (rk, out, in);
      burn_depth = nburn > burn_depth ? nburn : burn_depth;
      out += 16;
      in += 16;
      num_blks--;
    }

  if (burn_depth)
    burn_depth += sizeof(void *) * 5;
  return burn_depth;
}

/* Bulk encryption of complete blocks in CTR mode.  This function is only
   intended for the bulk encryption feature of cipher.c.  CTR is expected to be
   of size 16. */
static void
_gcry_sm4_ctr_enc(void *context, unsigned char *ctr,
                  void *outbuf_arg, const void *inbuf_arg,
                  size_t nblocks)
{
  SM4_context *ctx = context;
  byte *outbuf = outbuf_arg;
  const byte *inbuf = inbuf_arg;
  int burn_stack_depth = 0;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_sm4_aesni_avx2_ctr_enc(ctx->rkey_enc, outbuf, inbuf, ctr);

          nblocks -= 16;
          outbuf += 16 * 16;
          inbuf += 16 * 16;
        }
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      /* Process data in 8 block chunks. */
      while (nblocks >= 8)
        {
          _gcry_sm4_aesni_avx_ctr_enc(ctx->rkey_enc, outbuf, inbuf, ctr);

          nblocks -= 8;
          outbuf += 8 * 16;
          inbuf += 8 * 16;
        }
    }
#endif

  /* Process remaining blocks. */
  if (nblocks)
    {
      unsigned int (*crypt_blk1_8)(const u32 *rk, byte *out, const byte *in,
				   unsigned int num_blks);
      byte tmpbuf[16 * 8];
      unsigned int tmp_used = 16;

      if (0)
	;
#ifdef USE_AESNI_AVX
      else if (ctx->use_aesni_avx)
	{
	  crypt_blk1_8 = sm4_aesni_avx_crypt_blk1_8;
	}
#endif
      else
	{
	  prefetch_sbox_table ();
	  crypt_blk1_8 = sm4_crypt_blocks;
	}

      /* Process remaining blocks. */
      while (nblocks)
	{
	  size_t curr_blks = nblocks > 8 ? 8 : nblocks;
	  size_t i;

	  if (curr_blks * 16 > tmp_used)
	    tmp_used = curr_blks * 16;

	  cipher_block_cpy (tmpbuf + 0 * 16, ctr, 16);
	  for (i = 1; i < curr_blks; i++)
	    {
	      cipher_block_cpy (&tmpbuf[i * 16], ctr, 16);
	      cipher_block_add (&tmpbuf[i * 16], i, 16);
	    }
	  cipher_block_add (ctr, curr_blks, 16);

	  burn_stack_depth = crypt_blk1_8 (ctx->rkey_enc, tmpbuf, tmpbuf,
					   curr_blks);

	  for (i = 0; i < curr_blks; i++)
	    {
	      cipher_block_xor (outbuf, &tmpbuf[i * 16], inbuf, 16);
	      outbuf += 16;
	      inbuf += 16;
	    }

	  nblocks -= curr_blks;
	}

      wipememory(tmpbuf, tmp_used);
    }

  if (burn_stack_depth)
    _gcry_burn_stack(burn_stack_depth);
}

/* Bulk decryption of complete blocks in CBC mode.  This function is only
   intended for the bulk encryption feature of cipher.c. */
static void
_gcry_sm4_cbc_dec(void *context, unsigned char *iv,
                  void *outbuf_arg, const void *inbuf_arg,
                  size_t nblocks)
{
  SM4_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  int burn_stack_depth = 0;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_sm4_aesni_avx2_cbc_dec(ctx->rkey_dec, outbuf, inbuf, iv);

          nblocks -= 16;
          outbuf += 16 * 16;
          inbuf += 16 * 16;
        }
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      /* Process data in 8 block chunks. */
      while (nblocks >= 8)
        {
          _gcry_sm4_aesni_avx_cbc_dec(ctx->rkey_dec, outbuf, inbuf, iv);

          nblocks -= 8;
          outbuf += 8 * 16;
          inbuf += 8 * 16;
        }
    }
#endif

  /* Process remaining blocks. */
  if (nblocks)
    {
      unsigned int (*crypt_blk1_8)(const u32 *rk, byte *out, const byte *in,
				   unsigned int num_blks);
      unsigned char savebuf[16 * 8];
      unsigned int tmp_used = 16;

      if (0)
	;
#ifdef USE_AESNI_AVX
      else if (ctx->use_aesni_avx)
	{
	  crypt_blk1_8 = sm4_aesni_avx_crypt_blk1_8;
	}
#endif
      else
	{
	  prefetch_sbox_table ();
	  crypt_blk1_8 = sm4_crypt_blocks;
	}

      /* Process remaining blocks. */
      while (nblocks)
	{
	  size_t curr_blks = nblocks > 8 ? 8 : nblocks;
	  size_t i;

	  if (curr_blks * 16 > tmp_used)
	    tmp_used = curr_blks * 16;

	  burn_stack_depth = crypt_blk1_8 (ctx->rkey_dec, savebuf, inbuf,
					   curr_blks);

	  for (i = 0; i < curr_blks; i++)
	    {
	      cipher_block_xor_n_copy_2(outbuf, &savebuf[i * 16], iv, inbuf,
					16);
	      outbuf += 16;
	      inbuf += 16;
	    }

	  nblocks -= curr_blks;
	}

      wipememory(savebuf, tmp_used);
    }

  if (burn_stack_depth)
    _gcry_burn_stack(burn_stack_depth);
}

/* Bulk decryption of complete blocks in CFB mode.  This function is only
   intended for the bulk encryption feature of cipher.c. */
static void
_gcry_sm4_cfb_dec(void *context, unsigned char *iv,
                  void *outbuf_arg, const void *inbuf_arg,
                  size_t nblocks)
{
  SM4_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  int burn_stack_depth = 0;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_sm4_aesni_avx2_cfb_dec(ctx->rkey_enc, outbuf, inbuf, iv);

          nblocks -= 16;
          outbuf += 16 * 16;
          inbuf += 16 * 16;
        }
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      /* Process data in 8 block chunks. */
      while (nblocks >= 8)
        {
          _gcry_sm4_aesni_avx_cfb_dec(ctx->rkey_enc, outbuf, inbuf, iv);

          nblocks -= 8;
          outbuf += 8 * 16;
          inbuf += 8 * 16;
        }
    }
#endif

  /* Process remaining blocks. */
  if (nblocks)
    {
      unsigned int (*crypt_blk1_8)(const u32 *rk, byte *out, const byte *in,
				   unsigned int num_blks);
      unsigned char ivbuf[16 * 8];
      unsigned int tmp_used = 16;

      if (0)
	;
#ifdef USE_AESNI_AVX
      else if (ctx->use_aesni_avx)
	{
	  crypt_blk1_8 = sm4_aesni_avx_crypt_blk1_8;
	}
#endif
      else
	{
	  prefetch_sbox_table ();
	  crypt_blk1_8 = sm4_crypt_blocks;
	}

      /* Process remaining blocks. */
      while (nblocks)
	{
	  size_t curr_blks = nblocks > 8 ? 8 : nblocks;
	  size_t i;

	  if (curr_blks * 16 > tmp_used)
	    tmp_used = curr_blks * 16;

	  cipher_block_cpy (&ivbuf[0 * 16], iv, 16);
	  for (i = 1; i < curr_blks; i++)
	    cipher_block_cpy (&ivbuf[i * 16], &inbuf[(i - 1) * 16], 16);
	  cipher_block_cpy (iv, &inbuf[(i - 1) * 16], 16);

	  burn_stack_depth = crypt_blk1_8 (ctx->rkey_enc, ivbuf, ivbuf,
					   curr_blks);

	  for (i = 0; i < curr_blks; i++)
	    {
	      cipher_block_xor (outbuf, inbuf, &ivbuf[i * 16], 16);
	      outbuf += 16;
	      inbuf += 16;
	    }

	  nblocks -= curr_blks;
	}

      wipememory(ivbuf, tmp_used);
    }

  if (burn_stack_depth)
    _gcry_burn_stack(burn_stack_depth);
}

/* Bulk encryption/decryption of complete blocks in OCB mode. */
static size_t
_gcry_sm4_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
		     const void *inbuf_arg, size_t nblocks, int encrypt)
{
  SM4_context *ctx = (void *)&c->context.c;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  u64 blkn = c->u_mode.ocb.data_nblocks;
  int burn_stack_depth = 0;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
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
		_gcry_sm4_aesni_avx2_ocb_enc(ctx->rkey_enc, outbuf, inbuf,
					     c->u_iv.iv, c->u_ctr.ctr, Ls);
	      else
		_gcry_sm4_aesni_avx2_ocb_dec(ctx->rkey_dec, outbuf, inbuf,
					     c->u_iv.iv, c->u_ctr.ctr, Ls);

	      nblocks -= 16;
	      outbuf += 16 * 16;
	      inbuf += 16 * 16;
	    }
	}
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
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
	  Ls[(7 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  l = &Ls[(7 + n) % 8];

	  /* Process data in 8 block chunks. */
	  while (nblocks >= 8)
	    {
	      blkn += 8;
	      *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 8);

	      if (encrypt)
		_gcry_sm4_aesni_avx_ocb_enc(ctx->rkey_enc, outbuf, inbuf,
					    c->u_iv.iv, c->u_ctr.ctr, Ls);
	      else
		_gcry_sm4_aesni_avx_ocb_dec(ctx->rkey_dec, outbuf, inbuf,
					    c->u_iv.iv, c->u_ctr.ctr, Ls);

	      nblocks -= 8;
	      outbuf += 8 * 16;
	      inbuf += 8 * 16;
	    }
	}
    }
#endif

  if (nblocks)
    {
      unsigned int (*crypt_blk1_8)(const u32 *rk, byte *out, const byte *in,
				   unsigned int num_blks);
      const u32 *rk = encrypt ? ctx->rkey_enc : ctx->rkey_dec;
      unsigned char tmpbuf[16 * 8];
      unsigned int tmp_used = 16;

      if (0)
	;
#ifdef USE_AESNI_AVX
      else if (ctx->use_aesni_avx)
	{
	  crypt_blk1_8 = sm4_aesni_avx_crypt_blk1_8;
	}
#endif
      else
	{
	  prefetch_sbox_table ();
	  crypt_blk1_8 = sm4_crypt_blocks;
	}

      while (nblocks)
	{
	  size_t curr_blks = nblocks > 8 ? 8 : nblocks;
	  size_t i;

	  if (curr_blks * 16 > tmp_used)
	    tmp_used = curr_blks * 16;

	  for (i = 0; i < curr_blks; i++)
	    {
	      const unsigned char *l = ocb_get_l(c, ++blkn);

	      /* Checksum_i = Checksum_{i-1} xor P_i  */
	      if (encrypt)
		cipher_block_xor_1(c->u_ctr.ctr, &inbuf[i * 16], 16);

	      /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	      cipher_block_xor_2dst (&tmpbuf[i * 16], c->u_iv.iv, l, 16);
	      cipher_block_xor (&outbuf[i * 16], &inbuf[i * 16],
				c->u_iv.iv, 16);
	    }

	  /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
	  crypt_blk1_8 (rk, outbuf, outbuf, curr_blks);

	  for (i = 0; i < curr_blks; i++)
	    {
	      cipher_block_xor_1 (&outbuf[i * 16], &tmpbuf[i * 16], 16);

	      /* Checksum_i = Checksum_{i-1} xor P_i  */
	      if (!encrypt)
		  cipher_block_xor_1(c->u_ctr.ctr, &outbuf[i * 16], 16);
	    }

	  outbuf += curr_blks * 16;
	  inbuf  += curr_blks * 16;
	  nblocks -= curr_blks;
	}

      wipememory(tmpbuf, tmp_used);
    }

  c->u_mode.ocb.data_nblocks = blkn;

  if (burn_stack_depth)
    _gcry_burn_stack(burn_stack_depth);

  return 0;
}

/* Bulk authentication of complete blocks in OCB mode. */
static size_t
_gcry_sm4_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg, size_t nblocks)
{
  SM4_context *ctx = (void *)&c->context.c;
  const unsigned char *abuf = abuf_arg;
  u64 blkn = c->u_mode.ocb.aad_nblocks;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
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

	      _gcry_sm4_aesni_avx2_ocb_auth(ctx->rkey_enc, abuf,
					    c->u_mode.ocb.aad_offset,
					    c->u_mode.ocb.aad_sum, Ls);

	      nblocks -= 16;
	      abuf += 16 * 16;
	    }
	}
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
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
	  Ls[(7 + n) % 8] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  l = &Ls[(7 + n) % 8];

	  /* Process data in 8 block chunks. */
	  while (nblocks >= 8)
	    {
	      blkn += 8;
	      *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 8);

	      _gcry_sm4_aesni_avx_ocb_auth(ctx->rkey_enc, abuf,
					   c->u_mode.ocb.aad_offset,
					   c->u_mode.ocb.aad_sum, Ls);

	      nblocks -= 8;
	      abuf += 8 * 16;
	    }
	}
    }
#endif

  if (nblocks)
    {
      unsigned int (*crypt_blk1_8)(const u32 *rk, byte *out, const byte *in,
				   unsigned int num_blks);
      unsigned char tmpbuf[16 * 8];
      unsigned int tmp_used = 16;

      if (0)
	;
#ifdef USE_AESNI_AVX
      else if (ctx->use_aesni_avx)
	{
	  crypt_blk1_8 = sm4_aesni_avx_crypt_blk1_8;
	}
#endif
      else
	{
	  prefetch_sbox_table ();
	  crypt_blk1_8 = sm4_crypt_blocks;
	}

      while (nblocks)
	{
	  size_t curr_blks = nblocks > 8 ? 8 : nblocks;
	  size_t i;

	  if (curr_blks * 16 > tmp_used)
	    tmp_used = curr_blks * 16;

	  for (i = 0; i < curr_blks; i++)
	    {
	      const unsigned char *l = ocb_get_l(c, ++blkn);

	      /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	      cipher_block_xor_2dst (&tmpbuf[i * 16],
				     c->u_mode.ocb.aad_offset, l, 16);
	      cipher_block_xor_1 (&tmpbuf[i * 16], &abuf[i * 16], 16);
	    }

	  /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
	  crypt_blk1_8 (ctx->rkey_enc, tmpbuf, tmpbuf, curr_blks);

	  for (i = 0; i < curr_blks; i++)
	    {
	      cipher_block_xor_1 (c->u_mode.ocb.aad_sum, &tmpbuf[i * 16], 16);
	    }

	  abuf += curr_blks * 16;
	  nblocks -= curr_blks;
	}

      wipememory(tmpbuf, tmp_used);
    }

  c->u_mode.ocb.aad_nblocks = blkn;

  return 0;
}

/* Run the self-tests for SM4-CTR, tests IV increment of bulk CTR
   encryption.  Returns NULL on success. */
static const char*
selftest_ctr_128 (void)
{
  const int nblocks = 16 - 1;
  const int blocksize = 16;
  const int context_size = sizeof(SM4_context);

  return _gcry_selftest_helper_ctr("SM4", &sm4_setkey,
           &sm4_encrypt, nblocks, blocksize, context_size);
}

/* Run the self-tests for SM4-CBC, tests bulk CBC decryption.
   Returns NULL on success. */
static const char*
selftest_cbc_128 (void)
{
  const int nblocks = 16 - 1;
  const int blocksize = 16;
  const int context_size = sizeof(SM4_context);

  return _gcry_selftest_helper_cbc("SM4", &sm4_setkey,
           &sm4_encrypt, nblocks, blocksize, context_size);
}

/* Run the self-tests for SM4-CFB, tests bulk CFB decryption.
   Returns NULL on success. */
static const char*
selftest_cfb_128 (void)
{
  const int nblocks = 16 - 1;
  const int blocksize = 16;
  const int context_size = sizeof(SM4_context);

  return _gcry_selftest_helper_cfb("SM4", &sm4_setkey,
           &sm4_encrypt, nblocks, blocksize, context_size);
}

static const char *
sm4_selftest (void)
{
  SM4_context ctx;
  byte scratch[16];
  const char *r;

  static const byte plaintext[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
  };
  static const byte key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
  };
  static const byte ciphertext[16] = {
    0x68, 0x1E, 0xDF, 0x34, 0xD2, 0x06, 0x96, 0x5E,
    0x86, 0xB3, 0xE9, 0x4F, 0x53, 0x6E, 0x42, 0x46
  };

  memset (&ctx, 0, sizeof(ctx));

  sm4_expand_key (&ctx, key);
  sm4_encrypt (&ctx, scratch, plaintext);
  if (memcmp (scratch, ciphertext, sizeof (ciphertext)))
    return "SM4 test encryption failed.";
  sm4_decrypt (&ctx, scratch, scratch);
  if (memcmp (scratch, plaintext, sizeof (plaintext)))
    return "SM4 test decryption failed.";

  if ( (r = selftest_ctr_128 ()) )
    return r;

  if ( (r = selftest_cbc_128 ()) )
    return r;

  if ( (r = selftest_cfb_128 ()) )
    return r;

  return NULL;
}

static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  (void)extended;

  if (algo != GCRY_CIPHER_SM4)
    return GPG_ERR_CIPHER_ALGO;

  what = "selftest";
  errtxt = sm4_selftest ();
  if (errtxt)
    goto failed;

  return 0;

 failed:
  if (report)
    report ("cipher", GCRY_CIPHER_SM4, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


static const gcry_cipher_oid_spec_t sm4_oids[] =
  {
    { "1.2.156.10197.1.104.1", GCRY_CIPHER_MODE_ECB },
    { "1.2.156.10197.1.104.2", GCRY_CIPHER_MODE_CBC },
    { "1.2.156.10197.1.104.3", GCRY_CIPHER_MODE_OFB },
    { "1.2.156.10197.1.104.4", GCRY_CIPHER_MODE_CFB },
    { "1.2.156.10197.1.104.7", GCRY_CIPHER_MODE_CTR },
    { NULL }
  };

gcry_cipher_spec_t _gcry_cipher_spec_sm4 =
  {
    GCRY_CIPHER_SM4, {0, 0},
    "SM4", NULL, sm4_oids, 16, 128,
    sizeof (SM4_context),
    sm4_setkey, sm4_encrypt, sm4_decrypt,
    NULL, NULL,
    run_selftests
  };
