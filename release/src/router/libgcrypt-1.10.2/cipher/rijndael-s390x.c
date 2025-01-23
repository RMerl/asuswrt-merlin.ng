/* Rijndael (AES) for GnuPG - s390x/zSeries AES implementation
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

#include "rijndael-internal.h"
#include "cipher-internal.h"
#include "bufhelp.h"

#ifdef USE_S390X_CRYPTO

#include "asm-inline-s390x.h"

#define NO_INLINE __attribute__((noinline))

struct aes_s390x_gcm_params_s
{
  u32 reserved[3];
  u32 counter_value;
  u64 tag[2];
  u64 hash_subkey[2];
  u64 total_aad_length;
  u64 total_cipher_length;
  u32 initial_counter_value[4];
  u64 key[4];
};

#define DECL_QUERY_FUNC(instruction, opcode) \
  static u128_t instruction ##_query(void) \
  { \
    static u128_t function_codes = 0; \
    static int initialized = 0; \
    register unsigned long reg0 asm("0") = 0; \
    register void *reg1 asm("1") = &function_codes; \
    u128_t r1, r2; \
    \
    if (initialized) \
      return function_codes; \
    \
    asm volatile ("0: .insn rre," #opcode " << 16, %[r1], %[r2]\n\t" \
		  "   brc 1,0b\n\t" \
		  : [r1] "=a" (r1), [r2] "=a" (r2) \
		  : [reg0] "r" (reg0), [reg1] "r" (reg1) \
		  : "cc", "memory"); \
    \
    initialized = 1; \
    return function_codes; \
  }

#define DECL_EXECUTE_FUNC(instruction, opcode, param_const) \
  static ALWAYS_INLINE size_t \
  instruction ##_execute(unsigned int func, param_const void *param_block, \
			 void *dst, const void *src, size_t src_len) \
  { \
    register unsigned long reg0 asm("0") = func; \
    register param_const byte *reg1 asm("1") = param_block; \
    u128_t r1 = ((u128_t)(uintptr_t)dst << 64); \
    u128_t r2 = ((u128_t)(uintptr_t)src << 64) | (u64)src_len; \
    \
    asm volatile ("0: .insn rre," #opcode " << 16, %[r1], %[r2]\n\t" \
		  "   brc 1,0b\n\t" \
		  : [r1] "+a" (r1), [r2] "+a" (r2) \
		  : [func] "r" (reg0), [param_ptr] "r" (reg1) \
		  : "cc", "memory"); \
    \
    return (u64)r2; \
  }

DECL_QUERY_FUNC(km, 0xb92e);
DECL_QUERY_FUNC(kmc, 0xb92f);
DECL_QUERY_FUNC(kmac, 0xb91e);
DECL_QUERY_FUNC(kmf, 0xb92a);
DECL_QUERY_FUNC(kmo, 0xb92b);

DECL_EXECUTE_FUNC(km, 0xb92e, const);
DECL_EXECUTE_FUNC(kmc, 0xb92f, );
DECL_EXECUTE_FUNC(kmac, 0xb91e, );
DECL_EXECUTE_FUNC(kmf, 0xb92a, );
DECL_EXECUTE_FUNC(kmo, 0xb92b, );

static u128_t kma_query(void)
{
  static u128_t function_codes = 0;
  static int initialized = 0;
  register unsigned long reg0 asm("0") = 0;
  register void *reg1 asm("1") = &function_codes;
  u128_t r1, r2, r3;

  if (initialized)
    return function_codes;

  asm volatile ("0: .insn rrf,0xb929 << 16, %[r1], %[r2], %[r3], 0\n\t"
		"   brc 1,0b\n\t"
		: [r1] "=a" (r1), [r2] "=a" (r2), [r3] "=a" (r3)
		: [reg0] "r" (reg0), [reg1] "r" (reg1)
		: "cc", "memory");

  initialized = 1;
  return function_codes;
}

static ALWAYS_INLINE void
kma_execute(unsigned int func, void *param_block, byte *dst, const byte *src,
	    size_t src_len, const byte *aad, size_t aad_len)
{
  register unsigned long reg0 asm("0") = func;
  register byte *reg1 asm("1") = param_block;
  u128_t r1 = ((u128_t)(uintptr_t)dst << 64);
  u128_t r2 = ((u128_t)(uintptr_t)src << 64) | (u64)src_len;
  u128_t r3 = ((u128_t)(uintptr_t)aad << 64) | (u64)aad_len;

  asm volatile ("0: .insn rrf,0xb929 << 16, %[r1], %[r2], %[r3], 0\n\t"
		"   brc 1,0b\n\t"
		: [r1] "+a" (r1), [r2] "+a" (r2), [r3] "+a" (r3),
		  [func] "+r" (reg0)
		: [param_ptr] "r" (reg1)
		: "cc", "memory");
}

unsigned int _gcry_aes_s390x_encrypt(const RIJNDAEL_context *ctx,
				     unsigned char *dst,
				     const unsigned char *src)
{
  km_execute (ctx->km_func | KM_ENCRYPT, ctx->keyschenc, dst, src,
	      BLOCKSIZE);
  return 0;
}

unsigned int _gcry_aes_s390x_decrypt(const RIJNDAEL_context *ctx,
				     unsigned char *dst,
				     const unsigned char *src)
{
  km_execute (ctx->km_func | KM_DECRYPT, ctx->keyschenc, dst, src,
	      BLOCKSIZE);
  return 0;
}

static void aes_s390x_cbc_enc(void *context, unsigned char *iv,
			      void *outbuf_arg, const void *inbuf_arg,
			      size_t nblocks, int cbc_mac)
{
  RIJNDAEL_context *ctx = context;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  u128_t params[3];

  /* Prepare parameter block. */
  memcpy (&params[0], iv, BLOCKSIZE);
  memcpy (&params[1], ctx->keyschenc, 32);

  if (cbc_mac)
    {
      kmac_execute (ctx->kmac_func | KM_ENCRYPT, &params, NULL, in,
	            nblocks * BLOCKSIZE);
      memcpy (out, &params[0], BLOCKSIZE);
    }
  else
    {
      kmc_execute (ctx->kmc_func | KM_ENCRYPT, &params, out, in,
	           nblocks * BLOCKSIZE);
    }

  /* Update IV with OCV. */
  memcpy (iv, &params[0], BLOCKSIZE);

  wipememory (&params, sizeof(params));
}

static void aes_s390x_cbc_dec(void *context, unsigned char *iv,
			      void *outbuf_arg, const void *inbuf_arg,
			      size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  u128_t params[3];

  /* Prepare parameter block (ICV & key). */
  memcpy (&params[0], iv, BLOCKSIZE);
  memcpy (&params[1], ctx->keyschenc, 32);

  kmc_execute (ctx->kmc_func | KM_DECRYPT, &params, out, in,
	       nblocks * BLOCKSIZE);

  /* Update IV with OCV. */
  memcpy (iv, &params[0], BLOCKSIZE);

  wipememory (&params, sizeof(params));
}

static void aes_s390x_cfb128_enc(void *context, unsigned char *iv,
				 void *outbuf_arg, const void *inbuf_arg,
				 size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  unsigned int function;
  u128_t params[3];

  /* Prepare parameter block. */
  memcpy (&params[0], iv, BLOCKSIZE);
  memcpy (&params[1], ctx->keyschenc, 32);

  function = ctx->kmf_func | KM_ENCRYPT | KMF_LCFB_16;
  kmf_execute (function, &params, out, in, nblocks * BLOCKSIZE);

  /* Update IV with OCV. */
  memcpy (iv, &params[0], BLOCKSIZE);

  wipememory (&params, sizeof(params));
}

static void aes_s390x_cfb128_dec(void *context, unsigned char *iv,
				 void *outbuf_arg, const void *inbuf_arg,
				 size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  u128_t blocks[64];
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  size_t max_blocks_used = 0;

  /* AES128-CFB128 decryption speed using KMF was observed to be the same as
   * the KMF encryption, ~1.03 cpb. Expection was to see similar performance
   * as for AES128-CBC decryption as decryption for both modes should be
   * parallalizeble (CBC shows ~0.22 cpb). Therefore there is quite a bit
   * of room for improvement and implementation below using KM instruction
   * shows ~0.70 cpb speed, ~30% improvement over KMF instruction.
   */

  while (nblocks >= 64)
    {
      /* Copy IV to encrypt buffer, copy (nblocks - 1) input blocks to
       * encrypt buffer and update IV. */
      asm volatile ("mvc 0(16, %[blocks]), 0(%[iv])\n\t"
		    "mvc  16(240, %[blocks]),   0(%[in])\n\t"
		    "mvc 256(256, %[blocks]), 240(%[in])\n\t"
		    "mvc 512(256, %[blocks]), 496(%[in])\n\t"
		    "mvc 768(256, %[blocks]), 752(%[in])\n\t"
		    "mvc 0(16, %[iv]), 1008(%[in])\n\t"
		    :
		    : [in] "a" (in), [out] "a" (out), [blocks] "a" (blocks),
		      [iv] "a" (iv)
		    : "memory");

      /* Perform encryption of temporary buffer. */
      km_execute (ctx->km_func | KM_ENCRYPT, ctx->keyschenc, blocks, blocks,
		  64 * BLOCKSIZE);

      /* Xor encrypt buffer with input blocks and store to output blocks. */
      asm volatile ("xc   0(256, %[blocks]),   0(%[in])\n\t"
		    "xc 256(256, %[blocks]), 256(%[in])\n\t"
		    "xc 512(256, %[blocks]), 512(%[in])\n\t"
		    "xc 768(256, %[blocks]), 768(%[in])\n\t"
		    "mvc   0(256, %[out]),   0(%[blocks])\n\t"
		    "mvc 256(256, %[out]), 256(%[blocks])\n\t"
		    "mvc 512(256, %[out]), 512(%[blocks])\n\t"
		    "mvc 768(256, %[out]), 768(%[blocks])\n\t"
		    :
		    : [in] "a" (in), [out] "a" (out), [blocks] "a" (blocks)
		    : "memory");

      max_blocks_used = 64;
      in += 64 * BLOCKSIZE;
      out += 64 * BLOCKSIZE;
      nblocks -= 64;
    }

  if (nblocks)
    {
      unsigned int pos = 0;
      size_t in_nblocks = nblocks;
      size_t num_in = 0;

      max_blocks_used = max_blocks_used < nblocks ? nblocks : max_blocks_used;

      /* Copy IV to encrypt buffer. */
      asm volatile ("mvc 0(16, %[blocks]), 0(%[iv])\n\t"
		    :
		    : [blocks] "a" (blocks), [iv] "a" (iv)
		    : "memory");
      pos += 1;

#define CFB_MOVE_BLOCKS(block_oper, move_nbytes) \
      block_oper (in_nblocks - 1 >= move_nbytes / BLOCKSIZE) \
	{ \
	  unsigned int move_nblocks = move_nbytes / BLOCKSIZE; \
	  asm volatile ("mvc 0(" #move_nbytes ", %[blocks_x]), 0(%[in])\n\t" \
			: \
			: [blocks_x] "a" (&blocks[pos]), [in] "a" (in) \
			: "memory"); \
	  num_in += move_nblocks; \
	  in += move_nblocks * BLOCKSIZE; \
	  pos += move_nblocks; \
          in_nblocks -= move_nblocks; \
	}

      /* Copy (nblocks - 1) input blocks to encrypt buffer. */
      CFB_MOVE_BLOCKS(while, 256);
      CFB_MOVE_BLOCKS(if, 128);
      CFB_MOVE_BLOCKS(if, 64);
      CFB_MOVE_BLOCKS(if, 32);
      CFB_MOVE_BLOCKS(if, 16);

#undef CFB_MOVE_BLOCKS

      /* Update IV. */
      asm volatile ("mvc 0(16, %[iv]), 0(%[in])\n\t"
		    :
		    : [iv] "a" (iv), [in] "a" (in)
		    : "memory");
      num_in += 1;
      in += BLOCKSIZE;

      /* Perform encryption of temporary buffer. */
      km_execute (ctx->km_func | KM_ENCRYPT, ctx->keyschenc, blocks, blocks,
		  nblocks * BLOCKSIZE);

      /* Xor encrypt buffer with input blocks and store to output blocks. */
      pos = 0;
      in -= nblocks * BLOCKSIZE;

#define CFB_XOR_BLOCKS(block_oper, xor_nbytes) \
      block_oper (nblocks >= xor_nbytes / BLOCKSIZE) \
	{ \
	  unsigned int xor_nblocks = xor_nbytes / BLOCKSIZE; \
	  asm volatile ("xc 0(" #xor_nbytes ", %[blocks_x]), 0(%[in])\n\t" \
			"mvc 0(" #xor_nbytes ", %[out]), 0(%[blocks_x])\n\t" \
			: \
			: [blocks_x] "a" (&blocks[pos]), [out] "a" (out), \
			  [in] "a" (in) \
			: "memory"); \
	  out += xor_nblocks * BLOCKSIZE; \
	  in += xor_nblocks * BLOCKSIZE; \
	  nblocks -= xor_nblocks; \
	  pos += xor_nblocks; \
	}

      CFB_XOR_BLOCKS(while, 256);
      CFB_XOR_BLOCKS(if, 128);
      CFB_XOR_BLOCKS(if, 64);
      CFB_XOR_BLOCKS(if, 32);
      CFB_XOR_BLOCKS(if, 16);

#undef CFB_XOR_BLOCKS
    }

  if (max_blocks_used)
    wipememory (&blocks, max_blocks_used * BLOCKSIZE);
}

static void aes_s390x_ofb_enc(void *context, unsigned char *iv,
			      void *outbuf_arg, const void *inbuf_arg,
			      size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  unsigned int function;
  u128_t params[3];

  /* Prepare parameter block. */
  memcpy (&params[0], iv, BLOCKSIZE);
  memcpy (&params[1], ctx->keyschenc, 32);

  function = ctx->kmo_func | KM_ENCRYPT;
  kmo_execute (function, &params, out, in, nblocks * BLOCKSIZE);

  /* Update IV with OCV. */
  memcpy (iv, &params[0], BLOCKSIZE);

  wipememory (&params, sizeof(params));
}

static void aes_s390x_ctr128_enc(void *context, unsigned char *ctr,
				 void *outbuf_arg, const void *inbuf_arg,
				 size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  unsigned int function;
  struct aes_s390x_gcm_params_s params;

  memset (&params.hash_subkey, 0, sizeof(params.hash_subkey));
  memcpy (&params.key, ctx->keyschenc, 32);

  function = ctx->kma_func | KM_DECRYPT | KMA_HS | KMA_LAAD;

  while (nblocks)
    {
      u64 to_overflow = (u64)0xFFFFFFFFU + 1 - buf_get_be32 (ctr + 12);
      u64 ncurr = nblocks > to_overflow ? to_overflow : nblocks;

      /* Prepare parameter block. */
      memset (&params.reserved, 0, sizeof(params.reserved));
      buf_put_be32 (&params.counter_value, buf_get_be32(ctr + 12) - 1);
      memcpy (&params.initial_counter_value, ctr, 16);
      params.initial_counter_value[3] = params.counter_value;
      memset (&params.tag, 0, sizeof(params.tag));
      params.total_aad_length = 0;
      params.total_cipher_length = 0;

      /* Update counter. */
      cipher_block_add (ctr, ncurr, BLOCKSIZE);
      if (ncurr == (u64)0xFFFFFFFFU + 1)
	cipher_block_add (ctr, 1, BLOCKSIZE);

      /* Perform CTR using KMA-GCM. */
      kma_execute (function, &params, out, in, ncurr * BLOCKSIZE, NULL, 0);

      out += ncurr * BLOCKSIZE;
      in += ncurr * BLOCKSIZE;
      nblocks -= ncurr;
    }

  wipememory (&params, sizeof(params));
}

static size_t aes_s390x_gcm_crypt(gcry_cipher_hd_t c, void *outbuf_arg,
				  const void *inbuf_arg, size_t nblocks,
				  int encrypt)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  byte *ctr = c->u_ctr.ctr;
  unsigned int function;
  struct aes_s390x_gcm_params_s params;

  function = ctx->kma_func | (encrypt ? KM_ENCRYPT : KM_DECRYPT)
	      | KMA_HS | KMA_LAAD;

  /* Prepare parameter block. */
  memset (&params.reserved, 0, sizeof(params.reserved));
  buf_put_be32 (&params.counter_value, buf_get_be32(ctr + 12) - 1);
  memcpy (&params.tag, c->u_mode.gcm.u_tag.tag, 16);
  memcpy (&params.hash_subkey, c->u_mode.gcm.u_ghash_key.key, 16);
  params.total_aad_length = 0;
  params.total_cipher_length = 0;
  memcpy (&params.initial_counter_value, ctr, 12);
  params.initial_counter_value[3] = params.counter_value;
  memcpy (&params.key, ctx->keyschenc, 32);

  /* Update counter (CTR32). */
  buf_put_be32(ctr + 12, buf_get_be32(ctr + 12) + nblocks);

  /* Perform KMA-GCM. */
  kma_execute (function, &params, out, in, nblocks * BLOCKSIZE, NULL, 0);

  /* Update tag. */
  memcpy (c->u_mode.gcm.u_tag.tag, &params.tag, 16);

  wipememory (&params, sizeof(params));

  return 0;
}

static void aes_s390x_xts_crypt(void *context, unsigned char *tweak,
				void *outbuf_arg, const void *inbuf_arg,
				size_t nblocks, int encrypt)
{
  RIJNDAEL_context *ctx = context;
  byte *out = outbuf_arg;
  const byte *in = inbuf_arg;
  unsigned int function;
  u128_t params[3];
  u128_t *params_tweak;

  if (ctx->rounds < 12)
    {
      memcpy (&params[0], ctx->keyschenc, 16);
      params_tweak = &params[1];
      memcpy (params_tweak, tweak, BLOCKSIZE);
    }
  else if (ctx->rounds == 12)
    {
      BUG(); /* KM-XTS-AES-192 not defined. */
    }
  else
    {
      memcpy (&params[0], ctx->keyschenc, 32);
      params_tweak = &params[2];
      memcpy (params_tweak, tweak, BLOCKSIZE);
    }

  function = ctx->km_func_xts | (encrypt ? KM_ENCRYPT : KM_DECRYPT);
  km_execute (function, &params, out, in, nblocks * BLOCKSIZE);

  /* Update tweak with XTSP. */
  memcpy (tweak, params_tweak, BLOCKSIZE);

  wipememory (&params, sizeof(params));
}

static NO_INLINE void
aes_s390x_ocb_prepare_Ls (gcry_cipher_hd_t c, u64 blkn, const void *Ls[64],
			  const void ***pl)
{
  unsigned int n = 64 - (blkn % 64);
  int i;

  /* Prepare L pointers. */
  *pl = &Ls[(63 + n) % 64];
  for (i = 0; i < 64; i += 8, n = (n + 8) % 64)
    {
      static const int lastL[8] = { 3, 4, 3, 5, 3, 4, 3, 0 };

      Ls[(0 + n) % 64] = c->u_mode.ocb.L[0];
      Ls[(1 + n) % 64] = c->u_mode.ocb.L[1];
      Ls[(2 + n) % 64] = c->u_mode.ocb.L[0];
      Ls[(3 + n) % 64] = c->u_mode.ocb.L[2];
      Ls[(4 + n) % 64] = c->u_mode.ocb.L[0];
      Ls[(5 + n) % 64] = c->u_mode.ocb.L[1];
      Ls[(6 + n) % 64] = c->u_mode.ocb.L[0];
      Ls[(7 + n) % 64] = c->u_mode.ocb.L[lastL[i / 8]];
    }
}

static ALWAYS_INLINE const unsigned char *
aes_s390x_ocb_get_l (gcry_cipher_hd_t c, u64 n)
{
  unsigned long ntz = _gcry_ctz (n);
  if (ntz >= OCB_L_TABLE_SIZE)
    {
      return NULL; /* Not accessed. */
    }
  return c->u_mode.ocb.L[ntz];
}

static NO_INLINE void
aes_s390x_ocb_checksum (unsigned char *checksum, const void *plainbuf_arg,
			size_t nblks)
{
  const char *plainbuf = plainbuf_arg;
  u64 tmp0[2];
  u64 tmp1[2] = { 0, 0 };
  u64 tmp2[2] = { 0, 0 };
  u64 tmp3[2] = { 0, 0 };

  cipher_block_cpy (tmp0, checksum, BLOCKSIZE);

  if (nblks >= 4)
    {
      while (nblks >= 4)
	{
	  /* Checksum_i = Checksum_{i-1} xor P_i  */
	  cipher_block_xor_1 (tmp0, plainbuf + 0 * BLOCKSIZE, BLOCKSIZE);
	  cipher_block_xor_1 (tmp1, plainbuf + 1 * BLOCKSIZE, BLOCKSIZE);
	  cipher_block_xor_1 (tmp2, plainbuf + 2 * BLOCKSIZE, BLOCKSIZE);
	  cipher_block_xor_1 (tmp3, plainbuf + 3 * BLOCKSIZE, BLOCKSIZE);

	  plainbuf += 4 * BLOCKSIZE;
	  nblks -= 4;
	}

      cipher_block_xor_1 (tmp0, tmp1, BLOCKSIZE);
      cipher_block_xor_1 (tmp2, tmp3, BLOCKSIZE);
      cipher_block_xor_1 (tmp0, tmp2, BLOCKSIZE);

      wipememory (tmp1, sizeof(tmp1));
      wipememory (tmp2, sizeof(tmp2));
      wipememory (tmp3, sizeof(tmp3));
    }

  while (nblks > 0)
    {
      /* Checksum_i = Checksum_{i-1} xor P_i  */
      cipher_block_xor_1 (tmp0, plainbuf, BLOCKSIZE);

      plainbuf += BLOCKSIZE;
      nblks--;
    }

  cipher_block_cpy (checksum, tmp0, BLOCKSIZE);

  wipememory (tmp0, sizeof(tmp0));
}

static NO_INLINE size_t
aes_s390x_ocb_enc (gcry_cipher_hd_t c, void *outbuf_arg,
		   const void *inbuf_arg, size_t nblocks_arg)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  size_t nblocks = nblocks_arg;
  u128_t blocks[64];
  u128_t offset;
  size_t max_blocks_used = 0;
  u64 blkn = c->u_mode.ocb.data_nblocks;
  unsigned int function = ctx->km_func | KM_ENCRYPT;
  const void *Ls[64];
  const void **pl;

  aes_s390x_ocb_prepare_Ls (c, blkn, Ls, &pl);

  /* Checksumming could be done inline in OCB_INPUT macros, but register
   * pressure becomes too heavy and performance would end up being worse.
   * For decryption, checksumming is part of OCB_OUTPUT macros as
   * output handling is less demanding and can handle the additional
   * computation. */
  aes_s390x_ocb_checksum (c->u_ctr.ctr, inbuf_arg, nblocks_arg);

  cipher_block_cpy (&offset, &c->u_iv.iv, BLOCKSIZE);

#define OCB_INPUT(n) \
      cipher_block_xor_2dst (&blocks[n], &offset, Ls[n], BLOCKSIZE); \
      cipher_block_xor (outbuf + (n) * BLOCKSIZE, inbuf + (n) * BLOCKSIZE, \
			&offset, BLOCKSIZE)

#define OCB_INPUT_4(n) \
      OCB_INPUT((n) + 0); OCB_INPUT((n) + 1); OCB_INPUT((n) + 2); \
      OCB_INPUT((n) + 3)

#define OCB_INPUT_16(n) \
      OCB_INPUT_4((n) + 0); OCB_INPUT_4((n) + 4); OCB_INPUT_4((n) + 8); \
      OCB_INPUT_4((n) + 12);

#define OCB_OUTPUT(n) \
      cipher_block_xor_1 (outbuf + (n) * BLOCKSIZE, &blocks[n], BLOCKSIZE)

#define OCB_OUTPUT_4(n) \
      OCB_OUTPUT((n) + 0); OCB_OUTPUT((n) + 1); OCB_OUTPUT((n) + 2); \
      OCB_OUTPUT((n) + 3)

#define OCB_OUTPUT_16(n) \
      OCB_OUTPUT_4((n) + 0); OCB_OUTPUT_4((n) + 4); OCB_OUTPUT_4((n) + 8); \
      OCB_OUTPUT_4((n) + 12);

  while (nblocks >= 64)
    {
      blkn += 64;
      *pl = aes_s390x_ocb_get_l(c, blkn - blkn % 64);

      OCB_INPUT_16(0);
      OCB_INPUT_16(16);
      OCB_INPUT_16(32);
      OCB_INPUT_16(48);

      km_execute (function, ctx->keyschenc, outbuf, outbuf, 64 * BLOCKSIZE);

      asm volatile ("xc   0(256, %[out]),   0(%[blocks])\n\t"
		    "xc 256(256, %[out]), 256(%[blocks])\n\t"
		    "xc 512(256, %[out]), 512(%[blocks])\n\t"
		    "xc 768(256, %[out]), 768(%[blocks])\n\t"
		    :
		    : [out] "a" (outbuf), [blocks] "a" (blocks)
		    : "memory");

      max_blocks_used = 64;
      inbuf += 64 * BLOCKSIZE;
      outbuf += 64 * BLOCKSIZE;
      nblocks -= 64;
    }

  if (nblocks)
    {
      unsigned int pos = 0;

      max_blocks_used = max_blocks_used < nblocks ? nblocks : max_blocks_used;

      blkn += nblocks;
      *pl = aes_s390x_ocb_get_l(c, blkn - blkn % 64);

      while (nblocks >= 16)
	{
	  OCB_INPUT_16(pos + 0);
	  pos += 16;
	  nblocks -= 16;
	}
      while (nblocks >= 4)
	{
	  OCB_INPUT_4(pos + 0);
	  pos += 4;
	  nblocks -= 4;
	}
      if (nblocks >= 2)
	{
	  OCB_INPUT(pos + 0);
	  OCB_INPUT(pos + 1);
	  pos += 2;
	  nblocks -= 2;
	}
      if (nblocks >= 1)
	{
	  OCB_INPUT(pos + 0);
	  pos += 1;
	  nblocks -= 1;
	}

      nblocks = pos;
      pos = 0;
      km_execute (function, ctx->keyschenc, outbuf, outbuf,
		  nblocks * BLOCKSIZE);

      while (nblocks >= 16)
	{
	  OCB_OUTPUT_16(pos + 0);
	  pos += 16;
	  nblocks -= 16;
	}
      while (nblocks >= 4)
	{
	  OCB_OUTPUT_4(pos + 0);
	  pos += 4;
	  nblocks -= 4;
	}
      if (nblocks >= 2)
	{
	  OCB_OUTPUT(pos + 0);
	  OCB_OUTPUT(pos + 1);
	  pos += 2;
	  nblocks -= 2;
	}
      if (nblocks >= 1)
	{
	  OCB_OUTPUT(pos + 0);
	  pos += 1;
	  nblocks -= 1;
	}
    }

#undef OCB_INPUT
#undef OCB_INPUT_4
#undef OCB_INPUT_16
#undef OCB_OUTPUT
#undef OCB_OUTPUT_4
#undef OCB_OUTPUT_16

  c->u_mode.ocb.data_nblocks = blkn;
  cipher_block_cpy (&c->u_iv.iv, &offset, BLOCKSIZE);

  if (max_blocks_used)
    wipememory (&blocks, max_blocks_used * BLOCKSIZE);

  return 0;
}

static NO_INLINE size_t
aes_s390x_ocb_dec (gcry_cipher_hd_t c, void *outbuf_arg,
		   const void *inbuf_arg, size_t nblocks_arg)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  size_t nblocks = nblocks_arg;
  u128_t blocks[64];
  u128_t offset;
  size_t max_blocks_used = 0;
  u64 blkn = c->u_mode.ocb.data_nblocks;
  unsigned int function = ctx->km_func | KM_DECRYPT;
  const void *Ls[64];
  const void **pl;

  aes_s390x_ocb_prepare_Ls (c, blkn, Ls, &pl);

  cipher_block_cpy (&offset, &c->u_iv.iv, BLOCKSIZE);

#define OCB_INPUT(n) \
      cipher_block_xor_2dst (&blocks[n], &offset, Ls[n], BLOCKSIZE); \
      cipher_block_xor (outbuf + (n) * BLOCKSIZE, inbuf + (n) * BLOCKSIZE, \
			&offset, BLOCKSIZE)

#define OCB_INPUT_4(n) \
      OCB_INPUT((n) + 0); OCB_INPUT((n) + 1); OCB_INPUT((n) + 2); \
      OCB_INPUT((n) + 3)

#define OCB_INPUT_16(n) \
      OCB_INPUT_4((n) + 0); OCB_INPUT_4((n) + 4); OCB_INPUT_4((n) + 8); \
      OCB_INPUT_4((n) + 12);

#define OCB_OUTPUT(n) \
      cipher_block_xor_1 (outbuf + (n) * BLOCKSIZE, &blocks[n], BLOCKSIZE);

#define OCB_OUTPUT_4(n) \
      OCB_OUTPUT((n) + 0); OCB_OUTPUT((n) + 1); OCB_OUTPUT((n) + 2); \
      OCB_OUTPUT((n) + 3)

#define OCB_OUTPUT_16(n) \
      OCB_OUTPUT_4((n) + 0); OCB_OUTPUT_4((n) + 4); OCB_OUTPUT_4((n) + 8); \
      OCB_OUTPUT_4((n) + 12);

  while (nblocks >= 64)
    {
      blkn += 64;
      *pl = aes_s390x_ocb_get_l(c, blkn - blkn % 64);

      OCB_INPUT_16(0);
      OCB_INPUT_16(16);
      OCB_INPUT_16(32);
      OCB_INPUT_16(48);

      km_execute (function, ctx->keyschenc, outbuf, outbuf, 64 * BLOCKSIZE);

      asm volatile ("xc   0(256, %[out]),   0(%[blocks])\n\t"
		    "xc 256(256, %[out]), 256(%[blocks])\n\t"
		    "xc 512(256, %[out]), 512(%[blocks])\n\t"
		    "xc 768(256, %[out]), 768(%[blocks])\n\t"
		    :
		    : [out] "a" (outbuf), [blocks] "a" (blocks)
		    : "memory");

      max_blocks_used = 64;
      inbuf += 64 * BLOCKSIZE;
      outbuf += 64 * BLOCKSIZE;
      nblocks -= 64;
    }

  if (nblocks)
    {
      unsigned int pos = 0;

      max_blocks_used = max_blocks_used < nblocks ? nblocks : max_blocks_used;

      blkn += nblocks;
      *pl = aes_s390x_ocb_get_l(c, blkn - blkn % 64);

      while (nblocks >= 16)
	{
	  OCB_INPUT_16(pos + 0);
	  pos += 16;
	  nblocks -= 16;
	}
      while (nblocks >= 4)
	{
	  OCB_INPUT_4(pos + 0);
	  pos += 4;
	  nblocks -= 4;
	}
      if (nblocks >= 2)
	{
	  OCB_INPUT(pos + 0);
	  OCB_INPUT(pos + 1);
	  pos += 2;
	  nblocks -= 2;
	}
      if (nblocks >= 1)
	{
	  OCB_INPUT(pos + 0);
	  pos += 1;
	  nblocks -= 1;
	}

      nblocks = pos;
      pos = 0;
      km_execute (function, ctx->keyschenc, outbuf, outbuf,
		  nblocks * BLOCKSIZE);

      while (nblocks >= 16)
	{
	  OCB_OUTPUT_16(pos + 0);
	  pos += 16;
	  nblocks -= 16;
	}
      while (nblocks >= 4)
	{
	  OCB_OUTPUT_4(pos + 0);
	  pos += 4;
	  nblocks -= 4;
	}
      if (nblocks >= 2)
	{
	  OCB_OUTPUT(pos + 0);
	  OCB_OUTPUT(pos + 1);
	  pos += 2;
	  nblocks -= 2;
	}
      if (nblocks >= 1)
	{
	  OCB_OUTPUT(pos + 0);
	  pos += 1;
	  nblocks -= 1;
	}
    }

#undef OCB_INPUT
#undef OCB_INPUT_4
#undef OCB_INPUT_16
#undef OCB_OUTPUT
#undef OCB_OUTPUT_4
#undef OCB_OUTPUT_16

  c->u_mode.ocb.data_nblocks = blkn;
  cipher_block_cpy (&c->u_iv.iv, &offset, BLOCKSIZE);

  if (max_blocks_used)
    wipememory (&blocks, max_blocks_used * BLOCKSIZE);

  aes_s390x_ocb_checksum (c->u_ctr.ctr, outbuf_arg, nblocks_arg);

  return 0;
}

static size_t
aes_s390x_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
		     const void *inbuf_arg, size_t nblocks_arg, int encrypt)
{
  if (encrypt)
    return aes_s390x_ocb_enc (c, outbuf_arg, inbuf_arg, nblocks_arg);
  else
    return aes_s390x_ocb_dec (c, outbuf_arg, inbuf_arg, nblocks_arg);
}

static size_t
aes_s390x_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
		    size_t nblocks_arg)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const unsigned char *abuf = abuf_arg;
  u128_t blocks[64];
  u128_t offset;
  size_t max_blocks_used = 0;
  u64 blkn = c->u_mode.ocb.aad_nblocks;
  unsigned int function = ctx->km_func | KM_ENCRYPT;
  const void *Ls[64];
  const void **pl;

  aes_s390x_ocb_prepare_Ls (c, blkn, Ls, &pl);

  cipher_block_cpy (&offset, c->u_mode.ocb.aad_offset, BLOCKSIZE);

#define OCB_INPUT(n) \
      cipher_block_xor_2dst (&blocks[n], &offset, Ls[n], BLOCKSIZE); \
      cipher_block_xor_1 (&blocks[n], abuf + (n) * BLOCKSIZE, BLOCKSIZE)

#define OCB_INPUT_4(n) \
      OCB_INPUT((n) + 0); OCB_INPUT((n) + 1); OCB_INPUT((n) + 2); \
      OCB_INPUT((n) + 3)

#define OCB_INPUT_16(n) \
      OCB_INPUT_4((n) + 0); OCB_INPUT_4((n) + 4); OCB_INPUT_4((n) + 8); \
      OCB_INPUT_4((n) + 12);

  while (nblocks_arg >= 64)
    {
      blkn += 64;
      *pl = aes_s390x_ocb_get_l(c, blkn - blkn % 64);

      OCB_INPUT_16(0);
      OCB_INPUT_16(16);
      OCB_INPUT_16(32);
      OCB_INPUT_16(48);

      km_execute (function, ctx->keyschenc, blocks, blocks, 64 * BLOCKSIZE);

      aes_s390x_ocb_checksum (c->u_mode.ocb.aad_sum, blocks, 64);

      max_blocks_used = 64;
      abuf += 64 * BLOCKSIZE;
      nblocks_arg -= 64;
    }

  if (nblocks_arg > 0)
    {
      size_t nblocks = nblocks_arg;
      unsigned int pos = 0;

      max_blocks_used = max_blocks_used < nblocks ? nblocks : max_blocks_used;

      blkn += nblocks;
      *pl = aes_s390x_ocb_get_l(c, blkn - blkn % 64);

      while (nblocks >= 16)
	{
	  OCB_INPUT_16(pos + 0);
	  pos += 16;
	  nblocks -= 16;
	}
      while (nblocks >= 4)
	{
	  OCB_INPUT_4(pos + 0);
	  pos += 4;
	  nblocks -= 4;
	}
      if (nblocks >= 2)
	{
	  OCB_INPUT(pos + 0);
	  OCB_INPUT(pos + 1);
	  pos += 2;
	  nblocks -= 2;
	}
      if (nblocks >= 1)
	{
	  OCB_INPUT(pos + 0);
	  pos += 1;
	  nblocks -= 1;
	}

      nblocks = pos;
      nblocks_arg -= pos;
      pos = 0;
      km_execute (function, ctx->keyschenc, blocks, blocks,
		  nblocks * BLOCKSIZE);

      aes_s390x_ocb_checksum (c->u_mode.ocb.aad_sum, blocks, nblocks);
    }

#undef OCB_INPUT
#undef OCB_INPUT_4
#undef OCB_INPUT_16

  c->u_mode.ocb.aad_nblocks = blkn;
  cipher_block_cpy (c->u_mode.ocb.aad_offset, &offset, BLOCKSIZE);

  if (max_blocks_used)
    wipememory (&blocks, max_blocks_used * BLOCKSIZE);

  return 0;
}

int _gcry_aes_s390x_setup_acceleration(RIJNDAEL_context *ctx,
				       unsigned int keylen,
				       unsigned int hwfeatures,
				       cipher_bulk_ops_t *bulk_ops)
{
  unsigned int func;
  unsigned int func_xts;
  u128_t func_mask;
  u128_t func_xts_mask;

  if (!(hwfeatures & HWF_S390X_MSA))
    return 0;

  switch (keylen)
    {
    default:
    case 16:
      func = KM_FUNCTION_AES_128;
      func_xts = KM_FUNCTION_XTS_AES_128;
      func_mask = km_function_to_mask(KM_FUNCTION_AES_128);
      func_xts_mask = km_function_to_mask(KM_FUNCTION_XTS_AES_128);
      break;
    case 24:
      func = KM_FUNCTION_AES_192;
      func_xts = 0;
      func_mask = km_function_to_mask(KM_FUNCTION_AES_192);
      func_xts_mask = 0; /* XTS-AES192 not available. */
      break;
    case 32:
      func = KM_FUNCTION_AES_256;
      func_xts = KM_FUNCTION_XTS_AES_256;
      func_mask = km_function_to_mask(KM_FUNCTION_AES_256);
      func_xts_mask = km_function_to_mask(KM_FUNCTION_AES_256);
      break;
    }

  /* Query KM for supported algorithms and check if acceleration for
   * requested key-length is available. */
  if (!(km_query () & func_mask))
    return 0;

  ctx->km_func = func;

  /* Query KM for supported XTS algorithms. */
  if (km_query () & func_xts_mask)
    ctx->km_func_xts = func_xts;

  /* Query KMC for supported algorithms. */
  if (kmc_query () & func_mask)
    ctx->kmc_func = func;

  /* Query KMAC for supported algorithms. */
  if (kmac_query () & func_mask)
    ctx->kmac_func = func;

  if (hwfeatures & HWF_S390X_MSA_4)
    {
      /* Query KMF for supported algorithms. */
      if (kmf_query () & func_mask)
	ctx->kmf_func = func;

      /* Query KMO for supported algorithms. */
      if (kmo_query () & func_mask)
	ctx->kmo_func = func;
    }

  if (hwfeatures & HWF_S390X_MSA_8)
    {
      /* Query KMA for supported algorithms. */
      if (kma_query () & func_mask)
	ctx->kma_func = func;
    }

  /* Setup zSeries bulk encryption/decryption routines. */

  if (ctx->km_func)
    {
      bulk_ops->ocb_crypt = aes_s390x_ocb_crypt;
      bulk_ops->ocb_auth = aes_s390x_ocb_auth;

      /* CFB128 decryption uses KM instruction, instead of KMF. */
      bulk_ops->cfb_dec = aes_s390x_cfb128_dec;
    }

  if (ctx->km_func_xts)
    {
      bulk_ops->xts_crypt = aes_s390x_xts_crypt;
    }

  if (ctx->kmc_func)
    {
      if(ctx->kmac_func)
	{
	  /* Either KMC or KMAC used depending on 'cbc_mac' parameter. */
	  bulk_ops->cbc_enc = aes_s390x_cbc_enc;
	}

      bulk_ops->cbc_dec = aes_s390x_cbc_dec;
    }

  if (ctx->kmf_func)
    {
      bulk_ops->cfb_enc = aes_s390x_cfb128_enc;
    }

  if (ctx->kmo_func)
    {
      bulk_ops->ofb_enc = aes_s390x_ofb_enc;
    }

  if (ctx->kma_func)
    {
      bulk_ops->ctr_enc = aes_s390x_ctr128_enc;

      if (kimd_query () & km_function_to_mask (KMID_FUNCTION_GHASH))
	{
	  /* KIMD based GHASH implementation is required with AES-GCM
	   * acceleration. */
	  bulk_ops->gcm_crypt = aes_s390x_gcm_crypt;
	}
    }

  return 1;
}

void _gcry_aes_s390x_setkey(RIJNDAEL_context *ctx, const byte *key)
{
  unsigned int keylen = 16 + (ctx->rounds - 10) * 4;
  memcpy (ctx->keyschenc, key, keylen);
}

void _gcry_aes_s390x_prepare_decryption(RIJNDAEL_context *ctx)
{
  /* Do nothing. */
  (void)ctx;
}

#endif /* USE_S390X_CRYPTO */
