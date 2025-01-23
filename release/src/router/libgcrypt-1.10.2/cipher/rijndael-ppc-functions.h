/* Rijndael (AES) for GnuPG - PowerPC Vector Crypto AES implementation
 * Copyright (C) 2019 Shawn Landden <shawn@git.icu>
 * Copyright (C) 2019-2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 *
 * Alternatively, this code may be used in OpenSSL from The OpenSSL Project,
 * and Cryptogams by Andy Polyakov, and if made part of a release of either
 * or both projects, is thereafter dual-licensed under the license said project
 * is released under.
 */

unsigned int ENCRYPT_BLOCK_FUNC (const RIJNDAEL_context *ctx,
				 unsigned char *out,
				 const unsigned char *in)
{
  const block bige_const = asm_load_be_const();
  const u128_t *rk = (u128_t *)&ctx->keyschenc;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES;
  block b;

  b = VEC_LOAD_BE (in, 0, bige_const);

  PRELOAD_ROUND_KEYS (rounds);

  AES_ENCRYPT (b, rounds);
  VEC_STORE_BE (out, 0, b, bige_const);

  return 0; /* does not use stack */
}


unsigned int DECRYPT_BLOCK_FUNC (const RIJNDAEL_context *ctx,
				 unsigned char *out,
				 const unsigned char *in)
{
  const block bige_const = asm_load_be_const();
  const u128_t *rk = (u128_t *)&ctx->keyschdec;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES;
  block b;

  b = VEC_LOAD_BE (in, 0, bige_const);

  PRELOAD_ROUND_KEYS (rounds);

  AES_DECRYPT (b, rounds);
  VEC_STORE_BE (out, 0, b, bige_const);

  return 0; /* does not use stack */
}


void CFB_ENC_FUNC (void *context, unsigned char *iv_arg,
		   void *outbuf_arg, const void *inbuf_arg,
		   size_t nblocks)
{
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = context;
  const u128_t *rk = (u128_t *)&ctx->keyschenc;
  const u128_t *in = (const u128_t *)inbuf_arg;
  u128_t *out = (u128_t *)outbuf_arg;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES_ALL;
  block rkeylast_orig;
  block iv;

  iv = VEC_LOAD_BE (iv_arg, 0, bige_const);

  PRELOAD_ROUND_KEYS_ALL (rounds);
  rkeylast_orig = rkeylast;

  for (; nblocks >= 2; nblocks -= 2)
    {
      block in2, iv1;

      rkeylast = rkeylast_orig ^ VEC_LOAD_BE (in, 0, bige_const);
      in2 = VEC_LOAD_BE (in + 1, 0, bige_const);
      in += 2;

      AES_ENCRYPT_ALL (iv, rounds);

      iv1 = iv;
      rkeylast = rkeylast_orig ^ in2;

      AES_ENCRYPT_ALL (iv, rounds);

      VEC_STORE_BE (out++, 0, iv1, bige_const);
      VEC_STORE_BE (out++, 0, iv, bige_const);
    }

  for (; nblocks; nblocks--)
    {
      rkeylast = rkeylast_orig ^ VEC_LOAD_BE (in++, 0, bige_const);

      AES_ENCRYPT_ALL (iv, rounds);

      VEC_STORE_BE (out++, 0, iv, bige_const);
    }

  VEC_STORE_BE (iv_arg, 0, iv, bige_const);
}

void CFB_DEC_FUNC (void *context, unsigned char *iv_arg,
		   void *outbuf_arg, const void *inbuf_arg,
		   size_t nblocks)
{
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = context;
  const u128_t *rk = (u128_t *)&ctx->keyschenc;
  const u128_t *in = (const u128_t *)inbuf_arg;
  u128_t *out = (u128_t *)outbuf_arg;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES;
  block rkeylast_orig;
  block iv, b, bin;
  block in0, in1, in2, in3, in4, in5, in6, in7;
  block b0, b1, b2, b3, b4, b5, b6, b7;
  block rkey;

  iv = VEC_LOAD_BE (iv_arg, 0, bige_const);

  PRELOAD_ROUND_KEYS (rounds);
  rkeylast_orig = rkeylast;

  for (; nblocks >= 8; nblocks -= 8)
    {
      in0 = iv;
      in1 = VEC_LOAD_BE_NOSWAP (in, 0);
      in2 = VEC_LOAD_BE_NOSWAP (in, 1);
      in3 = VEC_LOAD_BE_NOSWAP (in, 2);
      in4 = VEC_LOAD_BE_NOSWAP (in, 3);
      in1 = VEC_BE_SWAP (in1, bige_const);
      in2 = VEC_BE_SWAP (in2, bige_const);
      in5 = VEC_LOAD_BE_NOSWAP (in, 4);
      in6 = VEC_LOAD_BE_NOSWAP (in, 5);
      in3 = VEC_BE_SWAP (in3, bige_const);
      in4 = VEC_BE_SWAP (in4, bige_const);
      in7 = VEC_LOAD_BE_NOSWAP (in, 6);
      iv = VEC_LOAD_BE_NOSWAP (in, 7);
      in += 8;
      in5 = VEC_BE_SWAP (in5, bige_const);
      in6 = VEC_BE_SWAP (in6, bige_const);
      b0 = asm_xor (rkey0, in0);
      b1 = asm_xor (rkey0, in1);
      in7 = VEC_BE_SWAP (in7, bige_const);
      iv = VEC_BE_SWAP (iv, bige_const);
      b2 = asm_xor (rkey0, in2);
      b3 = asm_xor (rkey0, in3);
      b4 = asm_xor (rkey0, in4);
      b5 = asm_xor (rkey0, in5);
      b6 = asm_xor (rkey0, in6);
      b7 = asm_xor (rkey0, in7);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey); \
	      b4 = asm_cipher_be (b4, rkey); \
	      b5 = asm_cipher_be (b5, rkey); \
	      b6 = asm_cipher_be (b6, rkey); \
	      b7 = asm_cipher_be (b7, rkey);

      DO_ROUND(1);
      DO_ROUND(2);
      DO_ROUND(3);
      DO_ROUND(4);
      DO_ROUND(5);
      DO_ROUND(6);
      DO_ROUND(7);
      DO_ROUND(8);
      DO_ROUND(9);
      if (rounds >= 12)
	{
	  DO_ROUND(10);
	  DO_ROUND(11);
	  if (rounds > 12)
	    {
	      DO_ROUND(12);
	      DO_ROUND(13);
	    }
	}

#undef DO_ROUND

      in1 = asm_xor (rkeylast, in1);
      in2 = asm_xor (rkeylast, in2);
      in3 = asm_xor (rkeylast, in3);
      in4 = asm_xor (rkeylast, in4);
      b0 = asm_cipherlast_be (b0, in1);
      b1 = asm_cipherlast_be (b1, in2);
      in5 = asm_xor (rkeylast, in5);
      in6 = asm_xor (rkeylast, in6);
      b2 = asm_cipherlast_be (b2, in3);
      b3 = asm_cipherlast_be (b3, in4);
      in7 = asm_xor (rkeylast, in7);
      in0 = asm_xor (rkeylast, iv);
      b0 = VEC_BE_SWAP (b0, bige_const);
      b1 = VEC_BE_SWAP (b1, bige_const);
      b4 = asm_cipherlast_be (b4, in5);
      b5 = asm_cipherlast_be (b5, in6);
      b2 = VEC_BE_SWAP (b2, bige_const);
      b3 = VEC_BE_SWAP (b3, bige_const);
      b6 = asm_cipherlast_be (b6, in7);
      b7 = asm_cipherlast_be (b7, in0);
      b4 = VEC_BE_SWAP (b4, bige_const);
      b5 = VEC_BE_SWAP (b5, bige_const);
      b6 = VEC_BE_SWAP (b6, bige_const);
      b7 = VEC_BE_SWAP (b7, bige_const);
      VEC_STORE_BE_NOSWAP (out, 0, b0);
      VEC_STORE_BE_NOSWAP (out, 1, b1);
      VEC_STORE_BE_NOSWAP (out, 2, b2);
      VEC_STORE_BE_NOSWAP (out, 3, b3);
      VEC_STORE_BE_NOSWAP (out, 4, b4);
      VEC_STORE_BE_NOSWAP (out, 5, b5);
      VEC_STORE_BE_NOSWAP (out, 6, b6);
      VEC_STORE_BE_NOSWAP (out, 7, b7);
      out += 8;
    }

  if (nblocks >= 4)
    {
      in0 = iv;
      in1 = VEC_LOAD_BE (in, 0, bige_const);
      in2 = VEC_LOAD_BE (in, 1, bige_const);
      in3 = VEC_LOAD_BE (in, 2, bige_const);
      iv = VEC_LOAD_BE (in, 3, bige_const);

      b0 = asm_xor (rkey0, in0);
      b1 = asm_xor (rkey0, in1);
      b2 = asm_xor (rkey0, in2);
      b3 = asm_xor (rkey0, in3);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey);

      DO_ROUND(1);
      DO_ROUND(2);
      DO_ROUND(3);
      DO_ROUND(4);
      DO_ROUND(5);
      DO_ROUND(6);
      DO_ROUND(7);
      DO_ROUND(8);
      DO_ROUND(9);
      if (rounds >= 12)
	{
	  DO_ROUND(10);
	  DO_ROUND(11);
	  if (rounds > 12)
	    {
	      DO_ROUND(12);
	      DO_ROUND(13);
	    }
	}

#undef DO_ROUND

      in1 = asm_xor (rkeylast, in1);
      in2 = asm_xor (rkeylast, in2);
      in3 = asm_xor (rkeylast, in3);
      in0 = asm_xor (rkeylast, iv);
      b0 = asm_cipherlast_be (b0, in1);
      b1 = asm_cipherlast_be (b1, in2);
      b2 = asm_cipherlast_be (b2, in3);
      b3 = asm_cipherlast_be (b3, in0);
      VEC_STORE_BE (out, 0, b0, bige_const);
      VEC_STORE_BE (out, 1, b1, bige_const);
      VEC_STORE_BE (out, 2, b2, bige_const);
      VEC_STORE_BE (out, 3, b3, bige_const);

      in += 4;
      out += 4;
      nblocks -= 4;
    }

  for (; nblocks; nblocks--)
    {
      bin = VEC_LOAD_BE (in, 0, bige_const);
      rkeylast = rkeylast_orig ^ bin;
      b = iv;
      iv = bin;

      AES_ENCRYPT (b, rounds);

      VEC_STORE_BE (out, 0, b, bige_const);

      out++;
      in++;
    }

  VEC_STORE_BE (iv_arg, 0, iv, bige_const);
}


void CBC_ENC_FUNC (void *context, unsigned char *iv_arg,
		   void *outbuf_arg, const void *inbuf_arg,
		   size_t nblocks, int cbc_mac)
{
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = context;
  const u128_t *rk = (u128_t *)&ctx->keyschenc;
  const u128_t *in = (const u128_t *)inbuf_arg;
  byte *out = (byte *)outbuf_arg;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES_ALL;
  block lastiv, b;
  unsigned int outadd = -(!cbc_mac) & 16;

  lastiv = VEC_LOAD_BE (iv_arg, 0, bige_const);

  PRELOAD_ROUND_KEYS_ALL (rounds);

  for (; nblocks >= 2; nblocks -= 2)
    {
      block in2, lastiv1;

      b = lastiv ^ VEC_LOAD_BE (in, 0, bige_const);
      in2 = VEC_LOAD_BE (in + 1, 0, bige_const);
      in += 2;

      AES_ENCRYPT_ALL (b, rounds);

      lastiv1 = b;
      b = lastiv1 ^ in2;

      AES_ENCRYPT_ALL (b, rounds);

      lastiv = b;
      VEC_STORE_BE ((u128_t *)out, 0, lastiv1, bige_const);
      out += outadd;
      VEC_STORE_BE ((u128_t *)out, 0, lastiv, bige_const);
      out += outadd;
    }

  for (; nblocks; nblocks--)
    {
      b = lastiv ^ VEC_LOAD_BE (in++, 0, bige_const);

      AES_ENCRYPT_ALL (b, rounds);

      lastiv = b;
      VEC_STORE_BE ((u128_t *)out, 0, b, bige_const);
      out += outadd;
    }

  VEC_STORE_BE (iv_arg, 0, lastiv, bige_const);
}

void CBC_DEC_FUNC (void *context, unsigned char *iv_arg,
		   void *outbuf_arg, const void *inbuf_arg,
		   size_t nblocks)
{
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = context;
  const u128_t *rk = (u128_t *)&ctx->keyschdec;
  const u128_t *in = (const u128_t *)inbuf_arg;
  u128_t *out = (u128_t *)outbuf_arg;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES;
  block rkeylast_orig;
  block in0, in1, in2, in3, in4, in5, in6, in7;
  block b0, b1, b2, b3, b4, b5, b6, b7;
  block rkey;
  block iv, b;

  if (!ctx->decryption_prepared)
    {
      internal_aes_ppc_prepare_decryption (ctx);
      ctx->decryption_prepared = 1;
    }

  iv = VEC_LOAD_BE (iv_arg, 0, bige_const);

  PRELOAD_ROUND_KEYS (rounds);
  rkeylast_orig = rkeylast;

  for (; nblocks >= 8; nblocks -= 8)
    {
      in0 = VEC_LOAD_BE_NOSWAP (in, 0);
      in1 = VEC_LOAD_BE_NOSWAP (in, 1);
      in2 = VEC_LOAD_BE_NOSWAP (in, 2);
      in3 = VEC_LOAD_BE_NOSWAP (in, 3);
      in0 = VEC_BE_SWAP (in0, bige_const);
      in1 = VEC_BE_SWAP (in1, bige_const);
      in4 = VEC_LOAD_BE_NOSWAP (in, 4);
      in5 = VEC_LOAD_BE_NOSWAP (in, 5);
      in2 = VEC_BE_SWAP (in2, bige_const);
      in3 = VEC_BE_SWAP (in3, bige_const);
      in6 = VEC_LOAD_BE_NOSWAP (in, 6);
      in7 = VEC_LOAD_BE_NOSWAP (in, 7);
      in += 8;
      b0 = asm_xor (rkey0, in0);
      b1 = asm_xor (rkey0, in1);
      in4 = VEC_BE_SWAP (in4, bige_const);
      in5 = VEC_BE_SWAP (in5, bige_const);
      b2 = asm_xor (rkey0, in2);
      b3 = asm_xor (rkey0, in3);
      in6 = VEC_BE_SWAP (in6, bige_const);
      in7 = VEC_BE_SWAP (in7, bige_const);
      b4 = asm_xor (rkey0, in4);
      b5 = asm_xor (rkey0, in5);
      b6 = asm_xor (rkey0, in6);
      b7 = asm_xor (rkey0, in7);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_ncipher_be (b0, rkey); \
	      b1 = asm_ncipher_be (b1, rkey); \
	      b2 = asm_ncipher_be (b2, rkey); \
	      b3 = asm_ncipher_be (b3, rkey); \
	      b4 = asm_ncipher_be (b4, rkey); \
	      b5 = asm_ncipher_be (b5, rkey); \
	      b6 = asm_ncipher_be (b6, rkey); \
	      b7 = asm_ncipher_be (b7, rkey);

      DO_ROUND(1);
      DO_ROUND(2);
      DO_ROUND(3);
      DO_ROUND(4);
      DO_ROUND(5);
      DO_ROUND(6);
      DO_ROUND(7);
      DO_ROUND(8);
      DO_ROUND(9);
      if (rounds >= 12)
	{
	  DO_ROUND(10);
	  DO_ROUND(11);
	  if (rounds > 12)
	    {
	      DO_ROUND(12);
	      DO_ROUND(13);
	    }
	}

#undef DO_ROUND

      iv = asm_xor (rkeylast, iv);
      in0 = asm_xor (rkeylast, in0);
      in1 = asm_xor (rkeylast, in1);
      in2 = asm_xor (rkeylast, in2);
      b0 = asm_ncipherlast_be (b0, iv);
      iv = in7;
      b1 = asm_ncipherlast_be (b1, in0);
      in3 = asm_xor (rkeylast, in3);
      in4 = asm_xor (rkeylast, in4);
      b2 = asm_ncipherlast_be (b2, in1);
      b3 = asm_ncipherlast_be (b3, in2);
      in5 = asm_xor (rkeylast, in5);
      in6 = asm_xor (rkeylast, in6);
      b0 = VEC_BE_SWAP (b0, bige_const);
      b1 = VEC_BE_SWAP (b1, bige_const);
      b4 = asm_ncipherlast_be (b4, in3);
      b5 = asm_ncipherlast_be (b5, in4);
      b2 = VEC_BE_SWAP (b2, bige_const);
      b3 = VEC_BE_SWAP (b3, bige_const);
      b6 = asm_ncipherlast_be (b6, in5);
      b7 = asm_ncipherlast_be (b7, in6);
      b4 = VEC_BE_SWAP (b4, bige_const);
      b5 = VEC_BE_SWAP (b5, bige_const);
      b6 = VEC_BE_SWAP (b6, bige_const);
      b7 = VEC_BE_SWAP (b7, bige_const);
      VEC_STORE_BE_NOSWAP (out, 0, b0);
      VEC_STORE_BE_NOSWAP (out, 1, b1);
      VEC_STORE_BE_NOSWAP (out, 2, b2);
      VEC_STORE_BE_NOSWAP (out, 3, b3);
      VEC_STORE_BE_NOSWAP (out, 4, b4);
      VEC_STORE_BE_NOSWAP (out, 5, b5);
      VEC_STORE_BE_NOSWAP (out, 6, b6);
      VEC_STORE_BE_NOSWAP (out, 7, b7);
      out += 8;
    }

  if (nblocks >= 4)
    {
      in0 = VEC_LOAD_BE (in, 0, bige_const);
      in1 = VEC_LOAD_BE (in, 1, bige_const);
      in2 = VEC_LOAD_BE (in, 2, bige_const);
      in3 = VEC_LOAD_BE (in, 3, bige_const);

      b0 = asm_xor (rkey0, in0);
      b1 = asm_xor (rkey0, in1);
      b2 = asm_xor (rkey0, in2);
      b3 = asm_xor (rkey0, in3);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_ncipher_be (b0, rkey); \
	      b1 = asm_ncipher_be (b1, rkey); \
	      b2 = asm_ncipher_be (b2, rkey); \
	      b3 = asm_ncipher_be (b3, rkey);

      DO_ROUND(1);
      DO_ROUND(2);
      DO_ROUND(3);
      DO_ROUND(4);
      DO_ROUND(5);
      DO_ROUND(6);
      DO_ROUND(7);
      DO_ROUND(8);
      DO_ROUND(9);
      if (rounds >= 12)
	{
	  DO_ROUND(10);
	  DO_ROUND(11);
	  if (rounds > 12)
	    {
	      DO_ROUND(12);
	      DO_ROUND(13);
	    }
	}

#undef DO_ROUND

      iv = asm_xor (rkeylast, iv);
      in0 = asm_xor (rkeylast, in0);
      in1 = asm_xor (rkeylast, in1);
      in2 = asm_xor (rkeylast, in2);

      b0 = asm_ncipherlast_be (b0, iv);
      iv = in3;
      b1 = asm_ncipherlast_be (b1, in0);
      b2 = asm_ncipherlast_be (b2, in1);
      b3 = asm_ncipherlast_be (b3, in2);

      VEC_STORE_BE (out, 0, b0, bige_const);
      VEC_STORE_BE (out, 1, b1, bige_const);
      VEC_STORE_BE (out, 2, b2, bige_const);
      VEC_STORE_BE (out, 3, b3, bige_const);

      in += 4;
      out += 4;
      nblocks -= 4;
    }

  for (; nblocks; nblocks--)
    {
      rkeylast = rkeylast_orig ^ iv;

      iv = VEC_LOAD_BE (in, 0, bige_const);
      b = iv;
      AES_DECRYPT (b, rounds);

      VEC_STORE_BE (out, 0, b, bige_const);

      in++;
      out++;
    }

  VEC_STORE_BE (iv_arg, 0, iv, bige_const);
}


void CTR_ENC_FUNC (void *context, unsigned char *ctr_arg,
		   void *outbuf_arg, const void *inbuf_arg,
		   size_t nblocks)
{
  static const unsigned char vec_one_const[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = context;
  const u128_t *rk = (u128_t *)&ctx->keyschenc;
  const u128_t *in = (const u128_t *)inbuf_arg;
  u128_t *out = (u128_t *)outbuf_arg;
  int rounds = ctx->rounds;
  ROUND_KEY_VARIABLES;
  block rkeylast_orig;
  block ctr, b, one;

  ctr = VEC_LOAD_BE (ctr_arg, 0, bige_const);
  one = VEC_LOAD_BE (&vec_one_const, 0, bige_const);

  PRELOAD_ROUND_KEYS (rounds);
  rkeylast_orig = rkeylast;

  if (nblocks >= 4)
    {
      block in0, in1, in2, in3, in4, in5, in6, in7;
      block b0, b1, b2, b3, b4, b5, b6, b7;
      block two, three, four;
      block rkey;

      two   = asm_add_uint128 (one, one);
      three = asm_add_uint128 (two, one);
      four  = asm_add_uint128 (two, two);

      for (; nblocks >= 8; nblocks -= 8)
	{
	  b1 = asm_add_uint128 (ctr, one);
	  b2 = asm_add_uint128 (ctr, two);
	  b3 = asm_add_uint128 (ctr, three);
	  b4 = asm_add_uint128 (ctr, four);
	  b5 = asm_add_uint128 (b1, four);
	  b6 = asm_add_uint128 (b2, four);
	  b7 = asm_add_uint128 (b3, four);
	  b0 = asm_xor (rkey0, ctr);
	  rkey = ALIGNED_LOAD (rk, 1);
	  ctr = asm_add_uint128 (b4, four);
	  b1 = asm_xor (rkey0, b1);
	  b2 = asm_xor (rkey0, b2);
	  b3 = asm_xor (rkey0, b3);
	  b0 = asm_cipher_be (b0, rkey);
	  b1 = asm_cipher_be (b1, rkey);
	  b2 = asm_cipher_be (b2, rkey);
	  b3 = asm_cipher_be (b3, rkey);
	  b4 = asm_xor (rkey0, b4);
	  b5 = asm_xor (rkey0, b5);
	  b6 = asm_xor (rkey0, b6);
	  b7 = asm_xor (rkey0, b7);
	  b4 = asm_cipher_be (b4, rkey);
	  b5 = asm_cipher_be (b5, rkey);
	  b6 = asm_cipher_be (b6, rkey);
	  b7 = asm_cipher_be (b7, rkey);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey); \
	      b4 = asm_cipher_be (b4, rkey); \
	      b5 = asm_cipher_be (b5, rkey); \
	      b6 = asm_cipher_be (b6, rkey); \
	      b7 = asm_cipher_be (b7, rkey);

	  in0 = VEC_LOAD_BE_NOSWAP (in, 0);
	  DO_ROUND(2);
	  in1 = VEC_LOAD_BE_NOSWAP (in, 1);
	  DO_ROUND(3);
	  in2 = VEC_LOAD_BE_NOSWAP (in, 2);
	  DO_ROUND(4);
	  in3 = VEC_LOAD_BE_NOSWAP (in, 3);
	  DO_ROUND(5);
	  in4 = VEC_LOAD_BE_NOSWAP (in, 4);
	  DO_ROUND(6);
	  in5 = VEC_LOAD_BE_NOSWAP (in, 5);
	  DO_ROUND(7);
	  in6 = VEC_LOAD_BE_NOSWAP (in, 6);
	  DO_ROUND(8);
	  in7 = VEC_LOAD_BE_NOSWAP (in, 7);
	  in += 8;
	  DO_ROUND(9);

	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  in0 = VEC_BE_SWAP (in0, bige_const);
	  in1 = VEC_BE_SWAP (in1, bige_const);
	  in2 = VEC_BE_SWAP (in2, bige_const);
	  in3 = VEC_BE_SWAP (in3, bige_const);
	  in4 = VEC_BE_SWAP (in4, bige_const);
	  in5 = VEC_BE_SWAP (in5, bige_const);
	  in6 = VEC_BE_SWAP (in6, bige_const);
	  in7 = VEC_BE_SWAP (in7, bige_const);

	  in0 = asm_xor (rkeylast, in0);
	  in1 = asm_xor (rkeylast, in1);
	  in2 = asm_xor (rkeylast, in2);
	  in3 = asm_xor (rkeylast, in3);
	  b0 = asm_cipherlast_be (b0, in0);
	  b1 = asm_cipherlast_be (b1, in1);
	  in4 = asm_xor (rkeylast, in4);
	  in5 = asm_xor (rkeylast, in5);
	  b2 = asm_cipherlast_be (b2, in2);
	  b3 = asm_cipherlast_be (b3, in3);
	  in6 = asm_xor (rkeylast, in6);
	  in7 = asm_xor (rkeylast, in7);
	  b4 = asm_cipherlast_be (b4, in4);
	  b5 = asm_cipherlast_be (b5, in5);
	  b6 = asm_cipherlast_be (b6, in6);
	  b7 = asm_cipherlast_be (b7, in7);

	  b0 = VEC_BE_SWAP (b0, bige_const);
	  b1 = VEC_BE_SWAP (b1, bige_const);
	  b2 = VEC_BE_SWAP (b2, bige_const);
	  b3 = VEC_BE_SWAP (b3, bige_const);
	  b4 = VEC_BE_SWAP (b4, bige_const);
	  b5 = VEC_BE_SWAP (b5, bige_const);
	  b6 = VEC_BE_SWAP (b6, bige_const);
	  b7 = VEC_BE_SWAP (b7, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 0, b0);
	  VEC_STORE_BE_NOSWAP (out, 1, b1);
	  VEC_STORE_BE_NOSWAP (out, 2, b2);
	  VEC_STORE_BE_NOSWAP (out, 3, b3);
	  VEC_STORE_BE_NOSWAP (out, 4, b4);
	  VEC_STORE_BE_NOSWAP (out, 5, b5);
	  VEC_STORE_BE_NOSWAP (out, 6, b6);
	  VEC_STORE_BE_NOSWAP (out, 7, b7);
	  out += 8;
	}

      if (nblocks >= 4)
	{
	  b1 = asm_add_uint128 (ctr, one);
	  b2 = asm_add_uint128 (ctr, two);
	  b3 = asm_add_uint128 (ctr, three);
	  b0 = asm_xor (rkey0, ctr);
	  ctr = asm_add_uint128 (ctr, four);
	  b1 = asm_xor (rkey0, b1);
	  b2 = asm_xor (rkey0, b2);
	  b3 = asm_xor (rkey0, b3);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);
	  DO_ROUND(8);

	  in0 = VEC_LOAD_BE (in, 0, bige_const);
	  in1 = VEC_LOAD_BE (in, 1, bige_const);
	  in2 = VEC_LOAD_BE (in, 2, bige_const);
	  in3 = VEC_LOAD_BE (in, 3, bige_const);

	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  in0 = asm_xor (rkeylast, in0);
	  in1 = asm_xor (rkeylast, in1);
	  in2 = asm_xor (rkeylast, in2);
	  in3 = asm_xor (rkeylast, in3);

	  b0 = asm_cipherlast_be (b0, in0);
	  b1 = asm_cipherlast_be (b1, in1);
	  b2 = asm_cipherlast_be (b2, in2);
	  b3 = asm_cipherlast_be (b3, in3);

	  VEC_STORE_BE (out, 0, b0, bige_const);
	  VEC_STORE_BE (out, 1, b1, bige_const);
	  VEC_STORE_BE (out, 2, b2, bige_const);
	  VEC_STORE_BE (out, 3, b3, bige_const);

	  in += 4;
	  out += 4;
	  nblocks -= 4;
	}
    }

  for (; nblocks; nblocks--)
    {
      b = ctr;
      ctr = asm_add_uint128 (ctr, one);
      rkeylast = rkeylast_orig ^ VEC_LOAD_BE (in, 0, bige_const);

      AES_ENCRYPT (b, rounds);

      VEC_STORE_BE (out, 0, b, bige_const);

      out++;
      in++;
    }

  VEC_STORE_BE (ctr_arg, 0, ctr, bige_const);
}


size_t OCB_CRYPT_FUNC (gcry_cipher_hd_t c, void *outbuf_arg,
		       const void *inbuf_arg, size_t nblocks,
		       int encrypt)
{
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const u128_t *in = (const u128_t *)inbuf_arg;
  u128_t *out = (u128_t *)outbuf_arg;
  int rounds = ctx->rounds;
  u64 data_nblocks = c->u_mode.ocb.data_nblocks;
  block l0, l1, l2, l;
  block b0, b1, b2, b3, b4, b5, b6, b7, b;
  block iv0, iv1, iv2, iv3, iv4, iv5, iv6, iv7;
  block rkey, rkeylf;
  block ctr, iv;
  ROUND_KEY_VARIABLES;

  iv = VEC_LOAD_BE (c->u_iv.iv, 0, bige_const);
  ctr = VEC_LOAD_BE (c->u_ctr.ctr, 0, bige_const);

  l0 = VEC_LOAD_BE (c->u_mode.ocb.L[0], 0, bige_const);
  l1 = VEC_LOAD_BE (c->u_mode.ocb.L[1], 0, bige_const);
  l2 = VEC_LOAD_BE (c->u_mode.ocb.L[2], 0, bige_const);

  if (encrypt)
    {
      const u128_t *rk = (u128_t *)&ctx->keyschenc;

      PRELOAD_ROUND_KEYS (rounds);

      for (; nblocks >= 8 && data_nblocks % 8; nblocks--)
	{
	  l = VEC_LOAD_BE (ocb_get_l (c, ++data_nblocks), 0, bige_const);
	  b = VEC_LOAD_BE (in, 0, bige_const);

	  /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	  iv ^= l;
	  /* Checksum_i = Checksum_{i-1} xor P_i  */
	  ctr ^= b;
	  /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
	  b ^= iv;
	  AES_ENCRYPT (b, rounds);
	  b ^= iv;

	  VEC_STORE_BE (out, 0, b, bige_const);

	  in += 1;
	  out += 1;
	}

      for (; nblocks >= 8; nblocks -= 8)
	{
	  b0 = VEC_LOAD_BE_NOSWAP (in, 0);
	  b1 = VEC_LOAD_BE_NOSWAP (in, 1);
	  b2 = VEC_LOAD_BE_NOSWAP (in, 2);
	  b3 = VEC_LOAD_BE_NOSWAP (in, 3);
	  b4 = VEC_LOAD_BE_NOSWAP (in, 4);
	  b5 = VEC_LOAD_BE_NOSWAP (in, 5);
	  b6 = VEC_LOAD_BE_NOSWAP (in, 6);
	  b7 = VEC_LOAD_BE_NOSWAP (in, 7);
	  in += 8;
	  l = VEC_LOAD_BE_NOSWAP (ocb_get_l (c, data_nblocks += 8), 0);
	  b0 = VEC_BE_SWAP(b0, bige_const);
	  b1 = VEC_BE_SWAP(b1, bige_const);
	  b2 = VEC_BE_SWAP(b2, bige_const);
	  b3 = VEC_BE_SWAP(b3, bige_const);
	  b4 = VEC_BE_SWAP(b4, bige_const);
	  b5 = VEC_BE_SWAP(b5, bige_const);
	  b6 = VEC_BE_SWAP(b6, bige_const);
	  b7 = VEC_BE_SWAP(b7, bige_const);
	  l = VEC_BE_SWAP(l, bige_const);

	  ctr ^= b0 ^ b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7;

	  iv ^= rkey0;

	  iv0 = iv ^ l0;
	  iv1 = iv ^ l0 ^ l1;
	  iv2 = iv ^ l1;
	  iv3 = iv ^ l1 ^ l2;
	  iv4 = iv ^ l1 ^ l2 ^ l0;
	  iv5 = iv ^ l2 ^ l0;
	  iv6 = iv ^ l2;
	  iv7 = iv ^ l2 ^ l;

	  b0 ^= iv0;
	  b1 ^= iv1;
	  b2 ^= iv2;
	  b3 ^= iv3;
	  b4 ^= iv4;
	  b5 ^= iv5;
	  b6 ^= iv6;
	  b7 ^= iv7;
	  iv = iv7 ^ rkey0;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey); \
	      b4 = asm_cipher_be (b4, rkey); \
	      b5 = asm_cipher_be (b5, rkey); \
	      b6 = asm_cipher_be (b6, rkey); \
	      b7 = asm_cipher_be (b7, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);

	  rkeylf = asm_xor (rkeylast, rkey0);

	  DO_ROUND(8);

	  iv0 = asm_xor (rkeylf, iv0);
	  iv1 = asm_xor (rkeylf, iv1);
	  iv2 = asm_xor (rkeylf, iv2);
	  iv3 = asm_xor (rkeylf, iv3);
	  iv4 = asm_xor (rkeylf, iv4);
	  iv5 = asm_xor (rkeylf, iv5);
	  iv6 = asm_xor (rkeylf, iv6);
	  iv7 = asm_xor (rkeylf, iv7);

	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  b0 = asm_cipherlast_be (b0, iv0);
	  b1 = asm_cipherlast_be (b1, iv1);
	  b2 = asm_cipherlast_be (b2, iv2);
	  b3 = asm_cipherlast_be (b3, iv3);
	  b4 = asm_cipherlast_be (b4, iv4);
	  b5 = asm_cipherlast_be (b5, iv5);
	  b6 = asm_cipherlast_be (b6, iv6);
	  b7 = asm_cipherlast_be (b7, iv7);

	  b0 = VEC_BE_SWAP (b0, bige_const);
	  b1 = VEC_BE_SWAP (b1, bige_const);
	  b2 = VEC_BE_SWAP (b2, bige_const);
	  b3 = VEC_BE_SWAP (b3, bige_const);
	  b4 = VEC_BE_SWAP (b4, bige_const);
	  b5 = VEC_BE_SWAP (b5, bige_const);
	  b6 = VEC_BE_SWAP (b6, bige_const);
	  b7 = VEC_BE_SWAP (b7, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 0, b0);
	  VEC_STORE_BE_NOSWAP (out, 1, b1);
	  VEC_STORE_BE_NOSWAP (out, 2, b2);
	  VEC_STORE_BE_NOSWAP (out, 3, b3);
	  VEC_STORE_BE_NOSWAP (out, 4, b4);
	  VEC_STORE_BE_NOSWAP (out, 5, b5);
	  VEC_STORE_BE_NOSWAP (out, 6, b6);
	  VEC_STORE_BE_NOSWAP (out, 7, b7);
	  out += 8;
	}

      if (nblocks >= 4 && (data_nblocks % 4) == 0)
	{
	  b0 = VEC_LOAD_BE (in, 0, bige_const);
	  b1 = VEC_LOAD_BE (in, 1, bige_const);
	  b2 = VEC_LOAD_BE (in, 2, bige_const);
	  b3 = VEC_LOAD_BE (in, 3, bige_const);

	  l = VEC_LOAD_BE (ocb_get_l (c, data_nblocks += 4), 0, bige_const);

	  ctr ^= b0 ^ b1 ^ b2 ^ b3;

	  iv ^= rkey0;

	  iv0 = iv ^ l0;
	  iv1 = iv ^ l0 ^ l1;
	  iv2 = iv ^ l1;
	  iv3 = iv ^ l1 ^ l;

	  b0 ^= iv0;
	  b1 ^= iv1;
	  b2 ^= iv2;
	  b3 ^= iv3;
	  iv = iv3 ^ rkey0;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);
	  DO_ROUND(8);
	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  rkey = rkeylast ^ rkey0;
	  b0 = asm_cipherlast_be (b0, rkey ^ iv0);
	  b1 = asm_cipherlast_be (b1, rkey ^ iv1);
	  b2 = asm_cipherlast_be (b2, rkey ^ iv2);
	  b3 = asm_cipherlast_be (b3, rkey ^ iv3);

	  VEC_STORE_BE (out, 0, b0, bige_const);
	  VEC_STORE_BE (out, 1, b1, bige_const);
	  VEC_STORE_BE (out, 2, b2, bige_const);
	  VEC_STORE_BE (out, 3, b3, bige_const);

	  in += 4;
	  out += 4;
	  nblocks -= 4;
	}

      for (; nblocks; nblocks--)
	{
	  l = VEC_LOAD_BE (ocb_get_l (c, ++data_nblocks), 0, bige_const);
	  b = VEC_LOAD_BE (in, 0, bige_const);

	  /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	  iv ^= l;
	  /* Checksum_i = Checksum_{i-1} xor P_i  */
	  ctr ^= b;
	  /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
	  b ^= iv;
	  AES_ENCRYPT (b, rounds);
	  b ^= iv;

	  VEC_STORE_BE (out, 0, b, bige_const);

	  in += 1;
	  out += 1;
	}
    }
  else
    {
      const u128_t *rk = (u128_t *)&ctx->keyschdec;

      if (!ctx->decryption_prepared)
	{
	  internal_aes_ppc_prepare_decryption (ctx);
	  ctx->decryption_prepared = 1;
	}

      PRELOAD_ROUND_KEYS (rounds);

      for (; nblocks >= 8 && data_nblocks % 8; nblocks--)
	{
	  l = VEC_LOAD_BE (ocb_get_l (c, ++data_nblocks), 0, bige_const);
	  b = VEC_LOAD_BE (in, 0, bige_const);

	  /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	  iv ^= l;
	  /* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */
	  b ^= iv;
	  AES_DECRYPT (b, rounds);
	  b ^= iv;
	  /* Checksum_i = Checksum_{i-1} xor P_i  */
	  ctr ^= b;

	  VEC_STORE_BE (out, 0, b, bige_const);

	  in += 1;
	  out += 1;
	}

      for (; nblocks >= 8; nblocks -= 8)
	{
	  b0 = VEC_LOAD_BE_NOSWAP (in, 0);
	  b1 = VEC_LOAD_BE_NOSWAP (in, 1);
	  b2 = VEC_LOAD_BE_NOSWAP (in, 2);
	  b3 = VEC_LOAD_BE_NOSWAP (in, 3);
	  b4 = VEC_LOAD_BE_NOSWAP (in, 4);
	  b5 = VEC_LOAD_BE_NOSWAP (in, 5);
	  b6 = VEC_LOAD_BE_NOSWAP (in, 6);
	  b7 = VEC_LOAD_BE_NOSWAP (in, 7);
	  in += 8;
	  l = VEC_LOAD_BE_NOSWAP (ocb_get_l (c, data_nblocks += 8), 0);
	  b0 = VEC_BE_SWAP(b0, bige_const);
	  b1 = VEC_BE_SWAP(b1, bige_const);
	  b2 = VEC_BE_SWAP(b2, bige_const);
	  b3 = VEC_BE_SWAP(b3, bige_const);
	  b4 = VEC_BE_SWAP(b4, bige_const);
	  b5 = VEC_BE_SWAP(b5, bige_const);
	  b6 = VEC_BE_SWAP(b6, bige_const);
	  b7 = VEC_BE_SWAP(b7, bige_const);
	  l = VEC_BE_SWAP(l, bige_const);

	  iv ^= rkey0;

	  iv0 = iv ^ l0;
	  iv1 = iv ^ l0 ^ l1;
	  iv2 = iv ^ l1;
	  iv3 = iv ^ l1 ^ l2;
	  iv4 = iv ^ l1 ^ l2 ^ l0;
	  iv5 = iv ^ l2 ^ l0;
	  iv6 = iv ^ l2;
	  iv7 = iv ^ l2 ^ l;

	  b0 ^= iv0;
	  b1 ^= iv1;
	  b2 ^= iv2;
	  b3 ^= iv3;
	  b4 ^= iv4;
	  b5 ^= iv5;
	  b6 ^= iv6;
	  b7 ^= iv7;
	  iv = iv7 ^ rkey0;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_ncipher_be (b0, rkey); \
	      b1 = asm_ncipher_be (b1, rkey); \
	      b2 = asm_ncipher_be (b2, rkey); \
	      b3 = asm_ncipher_be (b3, rkey); \
	      b4 = asm_ncipher_be (b4, rkey); \
	      b5 = asm_ncipher_be (b5, rkey); \
	      b6 = asm_ncipher_be (b6, rkey); \
	      b7 = asm_ncipher_be (b7, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);

	  rkeylf = asm_xor (rkeylast, rkey0);

	  DO_ROUND(8);

	  iv0 = asm_xor (rkeylf, iv0);
	  iv1 = asm_xor (rkeylf, iv1);
	  iv2 = asm_xor (rkeylf, iv2);
	  iv3 = asm_xor (rkeylf, iv3);
	  iv4 = asm_xor (rkeylf, iv4);
	  iv5 = asm_xor (rkeylf, iv5);
	  iv6 = asm_xor (rkeylf, iv6);
	  iv7 = asm_xor (rkeylf, iv7);

	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  b0 = asm_ncipherlast_be (b0, iv0);
	  b1 = asm_ncipherlast_be (b1, iv1);
	  b2 = asm_ncipherlast_be (b2, iv2);
	  b3 = asm_ncipherlast_be (b3, iv3);
	  b4 = asm_ncipherlast_be (b4, iv4);
	  b5 = asm_ncipherlast_be (b5, iv5);
	  b6 = asm_ncipherlast_be (b6, iv6);
	  b7 = asm_ncipherlast_be (b7, iv7);

	  ctr ^= b0 ^ b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7;

	  b0 = VEC_BE_SWAP (b0, bige_const);
	  b1 = VEC_BE_SWAP (b1, bige_const);
	  b2 = VEC_BE_SWAP (b2, bige_const);
	  b3 = VEC_BE_SWAP (b3, bige_const);
	  b4 = VEC_BE_SWAP (b4, bige_const);
	  b5 = VEC_BE_SWAP (b5, bige_const);
	  b6 = VEC_BE_SWAP (b6, bige_const);
	  b7 = VEC_BE_SWAP (b7, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 0, b0);
	  VEC_STORE_BE_NOSWAP (out, 1, b1);
	  VEC_STORE_BE_NOSWAP (out, 2, b2);
	  VEC_STORE_BE_NOSWAP (out, 3, b3);
	  VEC_STORE_BE_NOSWAP (out, 4, b4);
	  VEC_STORE_BE_NOSWAP (out, 5, b5);
	  VEC_STORE_BE_NOSWAP (out, 6, b6);
	  VEC_STORE_BE_NOSWAP (out, 7, b7);
	  out += 8;
	}

      if (nblocks >= 4 && (data_nblocks % 4) == 0)
	{
	  b0 = VEC_LOAD_BE (in, 0, bige_const);
	  b1 = VEC_LOAD_BE (in, 1, bige_const);
	  b2 = VEC_LOAD_BE (in, 2, bige_const);
	  b3 = VEC_LOAD_BE (in, 3, bige_const);

	  l = VEC_LOAD_BE (ocb_get_l (c, data_nblocks += 4), 0, bige_const);

	  iv ^= rkey0;

	  iv0 = iv ^ l0;
	  iv1 = iv ^ l0 ^ l1;
	  iv2 = iv ^ l1;
	  iv3 = iv ^ l1 ^ l;

	  b0 ^= iv0;
	  b1 ^= iv1;
	  b2 ^= iv2;
	  b3 ^= iv3;
	  iv = iv3 ^ rkey0;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_ncipher_be (b0, rkey); \
	      b1 = asm_ncipher_be (b1, rkey); \
	      b2 = asm_ncipher_be (b2, rkey); \
	      b3 = asm_ncipher_be (b3, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);
	  DO_ROUND(8);
	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  rkey = rkeylast ^ rkey0;
	  b0 = asm_ncipherlast_be (b0, rkey ^ iv0);
	  b1 = asm_ncipherlast_be (b1, rkey ^ iv1);
	  b2 = asm_ncipherlast_be (b2, rkey ^ iv2);
	  b3 = asm_ncipherlast_be (b3, rkey ^ iv3);

	  VEC_STORE_BE (out, 0, b0, bige_const);
	  VEC_STORE_BE (out, 1, b1, bige_const);
	  VEC_STORE_BE (out, 2, b2, bige_const);
	  VEC_STORE_BE (out, 3, b3, bige_const);

	  ctr ^= b0 ^ b1 ^ b2 ^ b3;

	  in += 4;
	  out += 4;
	  nblocks -= 4;
	}

      for (; nblocks; nblocks--)
	{
	  l = VEC_LOAD_BE (ocb_get_l (c, ++data_nblocks), 0, bige_const);
	  b = VEC_LOAD_BE (in, 0, bige_const);

	  /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	  iv ^= l;
	  /* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */
	  b ^= iv;
	  AES_DECRYPT (b, rounds);
	  b ^= iv;
	  /* Checksum_i = Checksum_{i-1} xor P_i  */
	  ctr ^= b;

	  VEC_STORE_BE (out, 0, b, bige_const);

	  in += 1;
	  out += 1;
	}
    }

  VEC_STORE_BE (c->u_iv.iv, 0, iv, bige_const);
  VEC_STORE_BE (c->u_ctr.ctr, 0, ctr, bige_const);
  c->u_mode.ocb.data_nblocks = data_nblocks;

  return 0;
}

size_t OCB_AUTH_FUNC (gcry_cipher_hd_t c, void *abuf_arg, size_t nblocks)
{
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const u128_t *rk = (u128_t *)&ctx->keyschenc;
  const u128_t *abuf = (const u128_t *)abuf_arg;
  int rounds = ctx->rounds;
  u64 data_nblocks = c->u_mode.ocb.aad_nblocks;
  block l0, l1, l2, l;
  block b0, b1, b2, b3, b4, b5, b6, b7, b;
  block iv0, iv1, iv2, iv3, iv4, iv5, iv6, iv7;
  block rkey, frkey;
  block ctr, iv;
  ROUND_KEY_VARIABLES;

  iv = VEC_LOAD_BE (c->u_mode.ocb.aad_offset, 0, bige_const);
  ctr = VEC_LOAD_BE (c->u_mode.ocb.aad_sum, 0, bige_const);

  l0 = VEC_LOAD_BE (c->u_mode.ocb.L[0], 0, bige_const);
  l1 = VEC_LOAD_BE (c->u_mode.ocb.L[1], 0, bige_const);
  l2 = VEC_LOAD_BE (c->u_mode.ocb.L[2], 0, bige_const);

  PRELOAD_ROUND_KEYS (rounds);

  for (; nblocks >= 8 && data_nblocks % 8; nblocks--)
    {
      l = VEC_LOAD_BE (ocb_get_l (c, ++data_nblocks), 0, bige_const);
      b = VEC_LOAD_BE (abuf, 0, bige_const);

      /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
      iv ^= l;
      /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */
      b ^= iv;
      AES_ENCRYPT (b, rounds);
      ctr ^= b;

      abuf += 1;
    }

  for (; nblocks >= 8; nblocks -= 8)
    {
      b0 = VEC_LOAD_BE (abuf, 0, bige_const);
      b1 = VEC_LOAD_BE (abuf, 1, bige_const);
      b2 = VEC_LOAD_BE (abuf, 2, bige_const);
      b3 = VEC_LOAD_BE (abuf, 3, bige_const);
      b4 = VEC_LOAD_BE (abuf, 4, bige_const);
      b5 = VEC_LOAD_BE (abuf, 5, bige_const);
      b6 = VEC_LOAD_BE (abuf, 6, bige_const);
      b7 = VEC_LOAD_BE (abuf, 7, bige_const);

      l = VEC_LOAD_BE (ocb_get_l (c, data_nblocks += 8), 0, bige_const);

      frkey = rkey0;
      iv ^= frkey;

      iv0 = iv ^ l0;
      iv1 = iv ^ l0 ^ l1;
      iv2 = iv ^ l1;
      iv3 = iv ^ l1 ^ l2;
      iv4 = iv ^ l1 ^ l2 ^ l0;
      iv5 = iv ^ l2 ^ l0;
      iv6 = iv ^ l2;
      iv7 = iv ^ l2 ^ l;

      b0 ^= iv0;
      b1 ^= iv1;
      b2 ^= iv2;
      b3 ^= iv3;
      b4 ^= iv4;
      b5 ^= iv5;
      b6 ^= iv6;
      b7 ^= iv7;
      iv = iv7 ^ frkey;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey); \
	      b4 = asm_cipher_be (b4, rkey); \
	      b5 = asm_cipher_be (b5, rkey); \
	      b6 = asm_cipher_be (b6, rkey); \
	      b7 = asm_cipher_be (b7, rkey);

      DO_ROUND(1);
      DO_ROUND(2);
      DO_ROUND(3);
      DO_ROUND(4);
      DO_ROUND(5);
      DO_ROUND(6);
      DO_ROUND(7);
      DO_ROUND(8);
      DO_ROUND(9);
      if (rounds >= 12)
	{
	  DO_ROUND(10);
	  DO_ROUND(11);
	  if (rounds > 12)
	    {
	      DO_ROUND(12);
	      DO_ROUND(13);
	    }
	}

#undef DO_ROUND

      rkey = rkeylast;
      b0 = asm_cipherlast_be (b0, rkey);
      b1 = asm_cipherlast_be (b1, rkey);
      b2 = asm_cipherlast_be (b2, rkey);
      b3 = asm_cipherlast_be (b3, rkey);
      b4 = asm_cipherlast_be (b4, rkey);
      b5 = asm_cipherlast_be (b5, rkey);
      b6 = asm_cipherlast_be (b6, rkey);
      b7 = asm_cipherlast_be (b7, rkey);

      ctr ^= b0 ^ b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7;

      abuf += 8;
    }

  if (nblocks >= 4 && (data_nblocks % 4) == 0)
    {
      b0 = VEC_LOAD_BE (abuf, 0, bige_const);
      b1 = VEC_LOAD_BE (abuf, 1, bige_const);
      b2 = VEC_LOAD_BE (abuf, 2, bige_const);
      b3 = VEC_LOAD_BE (abuf, 3, bige_const);

      l = VEC_LOAD_BE (ocb_get_l (c, data_nblocks += 4), 0, bige_const);

      frkey = rkey0;
      iv ^= frkey;

      iv0 = iv ^ l0;
      iv1 = iv ^ l0 ^ l1;
      iv2 = iv ^ l1;
      iv3 = iv ^ l1 ^ l;

      b0 ^= iv0;
      b1 ^= iv1;
      b2 ^= iv2;
      b3 ^= iv3;
      iv = iv3 ^ frkey;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey);

      DO_ROUND(1);
      DO_ROUND(2);
      DO_ROUND(3);
      DO_ROUND(4);
      DO_ROUND(5);
      DO_ROUND(6);
      DO_ROUND(7);
      DO_ROUND(8);
      DO_ROUND(9);
      if (rounds >= 12)
	{
	  DO_ROUND(10);
	  DO_ROUND(11);
	  if (rounds > 12)
	    {
	      DO_ROUND(12);
	      DO_ROUND(13);
	    }
	}

#undef DO_ROUND

      rkey = rkeylast;
      b0 = asm_cipherlast_be (b0, rkey);
      b1 = asm_cipherlast_be (b1, rkey);
      b2 = asm_cipherlast_be (b2, rkey);
      b3 = asm_cipherlast_be (b3, rkey);

      ctr ^= b0 ^ b1 ^ b2 ^ b3;

      abuf += 4;
      nblocks -= 4;
    }

  for (; nblocks; nblocks--)
    {
      l = VEC_LOAD_BE (ocb_get_l (c, ++data_nblocks), 0, bige_const);
      b = VEC_LOAD_BE (abuf, 0, bige_const);

      /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
      iv ^= l;
      /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */
      b ^= iv;
      AES_ENCRYPT (b, rounds);
      ctr ^= b;

      abuf += 1;
    }

  VEC_STORE_BE (c->u_mode.ocb.aad_offset, 0, iv, bige_const);
  VEC_STORE_BE (c->u_mode.ocb.aad_sum, 0, ctr, bige_const);
  c->u_mode.ocb.aad_nblocks = data_nblocks;

  return 0;
}


void XTS_CRYPT_FUNC (void *context, unsigned char *tweak_arg,
		     void *outbuf_arg, const void *inbuf_arg,
		     size_t nblocks, int encrypt)
{
#ifdef WORDS_BIGENDIAN
  static const block vec_bswap128_const =
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
#else
  static const block vec_bswap128_const =
    { ~15, ~14, ~13, ~12, ~11, ~10, ~9, ~8, ~7, ~6, ~5, ~4, ~3, ~2, ~1, ~0 };
#endif
  static const unsigned char vec_tweak_const[16] =
    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0x87 };
  static const vector unsigned long long vec_shift63_const =
    { 63, 63 };
  const block bige_const = asm_load_be_const();
  RIJNDAEL_context *ctx = context;
  const u128_t *in = (const u128_t *)inbuf_arg;
  u128_t *out = (u128_t *)outbuf_arg;
  int rounds = ctx->rounds;
  block tweak;
  block b0, b1, b2, b3, b4, b5, b6, b7, b, rkey, rkeylf;
  block tweak0, tweak1, tweak2, tweak3, tweak4, tweak5, tweak6, tweak7;
  block tweak_const, bswap128_const, shift63_const;
  ROUND_KEY_VARIABLES;

  tweak_const = VEC_LOAD_BE (&vec_tweak_const, 0, bige_const);
  bswap128_const = ALIGNED_LOAD (&vec_bswap128_const, 0);
  shift63_const = ALIGNED_LOAD (&vec_shift63_const, 0);

  tweak = VEC_LOAD_BE (tweak_arg, 0, bige_const);
  tweak = asm_vperm1 (tweak, bswap128_const);

#define GEN_TWEAK(tout, tin) /* Generate next tweak. */ \
    do { \
      block tmp1, tmp2; \
      tmp1 = asm_swap_uint64_halfs(tin); \
      tmp2 = asm_add_uint64(tin, tin); \
      tmp1 = asm_sra_int64(tmp1, shift63_const) & tweak_const; \
      tout = asm_xor(tmp1, tmp2); \
    } while (0)

  if (encrypt)
    {
      const u128_t *rk = (u128_t *)&ctx->keyschenc;

      PRELOAD_ROUND_KEYS (rounds);

      for (; nblocks >= 8; nblocks -= 8)
	{
	  b0 = VEC_LOAD_BE_NOSWAP (in, 0);
	  b1 = VEC_LOAD_BE_NOSWAP (in, 1);
	  b2 = VEC_LOAD_BE_NOSWAP (in, 2);
	  b3 = VEC_LOAD_BE_NOSWAP (in, 3);
	  tweak0 = tweak;
	  GEN_TWEAK (tweak1, tweak0);
	  tweak0 = asm_vperm1 (tweak0, bswap128_const);
	  b4 = VEC_LOAD_BE_NOSWAP (in, 4);
	  b5 = VEC_LOAD_BE_NOSWAP (in, 5);
	  GEN_TWEAK (tweak2, tweak1);
	  tweak1 = asm_vperm1 (tweak1, bswap128_const);
	  b6 = VEC_LOAD_BE_NOSWAP (in, 6);
	  b7 = VEC_LOAD_BE_NOSWAP (in, 7);
	  in += 8;

	  b0 = VEC_BE_SWAP(b0, bige_const);
	  b1 = VEC_BE_SWAP(b1, bige_const);
	  GEN_TWEAK (tweak3, tweak2);
	  tweak2 = asm_vperm1 (tweak2, bswap128_const);
	  GEN_TWEAK (tweak4, tweak3);
	  tweak3 = asm_vperm1 (tweak3, bswap128_const);
	  b2 = VEC_BE_SWAP(b2, bige_const);
	  b3 = VEC_BE_SWAP(b3, bige_const);
	  GEN_TWEAK (tweak5, tweak4);
	  tweak4 = asm_vperm1 (tweak4, bswap128_const);
	  GEN_TWEAK (tweak6, tweak5);
	  tweak5 = asm_vperm1 (tweak5, bswap128_const);
	  b4 = VEC_BE_SWAP(b4, bige_const);
	  b5 = VEC_BE_SWAP(b5, bige_const);
	  GEN_TWEAK (tweak7, tweak6);
	  tweak6 = asm_vperm1 (tweak6, bswap128_const);
	  GEN_TWEAK (tweak, tweak7);
	  tweak7 = asm_vperm1 (tweak7, bswap128_const);
	  b6 = VEC_BE_SWAP(b6, bige_const);
	  b7 = VEC_BE_SWAP(b7, bige_const);

	  tweak0 = asm_xor (tweak0, rkey0);
	  tweak1 = asm_xor (tweak1, rkey0);
	  tweak2 = asm_xor (tweak2, rkey0);
	  tweak3 = asm_xor (tweak3, rkey0);
	  tweak4 = asm_xor (tweak4, rkey0);
	  tweak5 = asm_xor (tweak5, rkey0);
	  tweak6 = asm_xor (tweak6, rkey0);
	  tweak7 = asm_xor (tweak7, rkey0);

	  b0 = asm_xor (b0, tweak0);
	  b1 = asm_xor (b1, tweak1);
	  b2 = asm_xor (b2, tweak2);
	  b3 = asm_xor (b3, tweak3);
	  b4 = asm_xor (b4, tweak4);
	  b5 = asm_xor (b5, tweak5);
	  b6 = asm_xor (b6, tweak6);
	  b7 = asm_xor (b7, tweak7);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey); \
	      b4 = asm_cipher_be (b4, rkey); \
	      b5 = asm_cipher_be (b5, rkey); \
	      b6 = asm_cipher_be (b6, rkey); \
	      b7 = asm_cipher_be (b7, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);

	  rkeylf = asm_xor (rkeylast, rkey0);

	  DO_ROUND(8);

	  tweak0 = asm_xor (tweak0, rkeylf);
	  tweak1 = asm_xor (tweak1, rkeylf);
	  tweak2 = asm_xor (tweak2, rkeylf);
	  tweak3 = asm_xor (tweak3, rkeylf);
	  tweak4 = asm_xor (tweak4, rkeylf);
	  tweak5 = asm_xor (tweak5, rkeylf);
	  tweak6 = asm_xor (tweak6, rkeylf);
	  tweak7 = asm_xor (tweak7, rkeylf);

	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  b0 = asm_cipherlast_be (b0, tweak0);
	  b1 = asm_cipherlast_be (b1, tweak1);
	  b2 = asm_cipherlast_be (b2, tweak2);
	  b3 = asm_cipherlast_be (b3, tweak3);
	  b0 = VEC_BE_SWAP (b0, bige_const);
	  b1 = VEC_BE_SWAP (b1, bige_const);
	  b4 = asm_cipherlast_be (b4, tweak4);
	  b5 = asm_cipherlast_be (b5, tweak5);
	  b2 = VEC_BE_SWAP (b2, bige_const);
	  b3 = VEC_BE_SWAP (b3, bige_const);
	  b6 = asm_cipherlast_be (b6, tweak6);
	  b7 = asm_cipherlast_be (b7, tweak7);
	  VEC_STORE_BE_NOSWAP (out, 0, b0);
	  VEC_STORE_BE_NOSWAP (out, 1, b1);
	  b4 = VEC_BE_SWAP (b4, bige_const);
	  b5 = VEC_BE_SWAP (b5, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 2, b2);
	  VEC_STORE_BE_NOSWAP (out, 3, b3);
	  b6 = VEC_BE_SWAP (b6, bige_const);
	  b7 = VEC_BE_SWAP (b7, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 4, b4);
	  VEC_STORE_BE_NOSWAP (out, 5, b5);
	  VEC_STORE_BE_NOSWAP (out, 6, b6);
	  VEC_STORE_BE_NOSWAP (out, 7, b7);
	  out += 8;
	}

      if (nblocks >= 4)
	{
	  tweak0 = tweak;
	  GEN_TWEAK (tweak1, tweak0);
	  GEN_TWEAK (tweak2, tweak1);
	  GEN_TWEAK (tweak3, tweak2);
	  GEN_TWEAK (tweak, tweak3);

	  b0 = VEC_LOAD_BE (in, 0, bige_const);
	  b1 = VEC_LOAD_BE (in, 1, bige_const);
	  b2 = VEC_LOAD_BE (in, 2, bige_const);
	  b3 = VEC_LOAD_BE (in, 3, bige_const);

	  tweak0 = asm_vperm1 (tweak0, bswap128_const);
	  tweak1 = asm_vperm1 (tweak1, bswap128_const);
	  tweak2 = asm_vperm1 (tweak2, bswap128_const);
	  tweak3 = asm_vperm1 (tweak3, bswap128_const);

	  b0 ^= tweak0 ^ rkey0;
	  b1 ^= tweak1 ^ rkey0;
	  b2 ^= tweak2 ^ rkey0;
	  b3 ^= tweak3 ^ rkey0;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_cipher_be (b0, rkey); \
	      b1 = asm_cipher_be (b1, rkey); \
	      b2 = asm_cipher_be (b2, rkey); \
	      b3 = asm_cipher_be (b3, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);
	  DO_ROUND(8);
	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  rkey = rkeylast;
	  b0 = asm_cipherlast_be (b0, rkey ^ tweak0);
	  b1 = asm_cipherlast_be (b1, rkey ^ tweak1);
	  b2 = asm_cipherlast_be (b2, rkey ^ tweak2);
	  b3 = asm_cipherlast_be (b3, rkey ^ tweak3);

	  VEC_STORE_BE (out, 0, b0, bige_const);
	  VEC_STORE_BE (out, 1, b1, bige_const);
	  VEC_STORE_BE (out, 2, b2, bige_const);
	  VEC_STORE_BE (out, 3, b3, bige_const);

	  in += 4;
	  out += 4;
	  nblocks -= 4;
	}

      for (; nblocks; nblocks--)
	{
	  tweak0 = asm_vperm1 (tweak, bswap128_const);

	  /* Xor-Encrypt/Decrypt-Xor block. */
	  b = VEC_LOAD_BE (in, 0, bige_const) ^ tweak0;

	  /* Generate next tweak. */
	  GEN_TWEAK (tweak, tweak);

	  AES_ENCRYPT (b, rounds);

	  b ^= tweak0;
	  VEC_STORE_BE (out, 0, b, bige_const);

	  in++;
	  out++;
	}
    }
  else
    {
      const u128_t *rk = (u128_t *)&ctx->keyschdec;

      if (!ctx->decryption_prepared)
	{
	  internal_aes_ppc_prepare_decryption (ctx);
	  ctx->decryption_prepared = 1;
	}

      PRELOAD_ROUND_KEYS (rounds);

      for (; nblocks >= 8; nblocks -= 8)
	{
	  b0 = VEC_LOAD_BE_NOSWAP (in, 0);
	  b1 = VEC_LOAD_BE_NOSWAP (in, 1);
	  b2 = VEC_LOAD_BE_NOSWAP (in, 2);
	  b3 = VEC_LOAD_BE_NOSWAP (in, 3);
	  tweak0 = tweak;
	  GEN_TWEAK (tweak1, tweak0);
	  tweak0 = asm_vperm1 (tweak0, bswap128_const);
	  b4 = VEC_LOAD_BE_NOSWAP (in, 4);
	  b5 = VEC_LOAD_BE_NOSWAP (in, 5);
	  GEN_TWEAK (tweak2, tweak1);
	  tweak1 = asm_vperm1 (tweak1, bswap128_const);
	  b6 = VEC_LOAD_BE_NOSWAP (in, 6);
	  b7 = VEC_LOAD_BE_NOSWAP (in, 7);
	  in += 8;

	  b0 = VEC_BE_SWAP(b0, bige_const);
	  b1 = VEC_BE_SWAP(b1, bige_const);
	  GEN_TWEAK (tweak3, tweak2);
	  tweak2 = asm_vperm1 (tweak2, bswap128_const);
	  GEN_TWEAK (tweak4, tweak3);
	  tweak3 = asm_vperm1 (tweak3, bswap128_const);
	  b2 = VEC_BE_SWAP(b2, bige_const);
	  b3 = VEC_BE_SWAP(b3, bige_const);
	  GEN_TWEAK (tweak5, tweak4);
	  tweak4 = asm_vperm1 (tweak4, bswap128_const);
	  GEN_TWEAK (tweak6, tweak5);
	  tweak5 = asm_vperm1 (tweak5, bswap128_const);
	  b4 = VEC_BE_SWAP(b4, bige_const);
	  b5 = VEC_BE_SWAP(b5, bige_const);
	  GEN_TWEAK (tweak7, tweak6);
	  tweak6 = asm_vperm1 (tweak6, bswap128_const);
	  GEN_TWEAK (tweak, tweak7);
	  tweak7 = asm_vperm1 (tweak7, bswap128_const);
	  b6 = VEC_BE_SWAP(b6, bige_const);
	  b7 = VEC_BE_SWAP(b7, bige_const);

	  tweak0 = asm_xor (tweak0, rkey0);
	  tweak1 = asm_xor (tweak1, rkey0);
	  tweak2 = asm_xor (tweak2, rkey0);
	  tweak3 = asm_xor (tweak3, rkey0);
	  tweak4 = asm_xor (tweak4, rkey0);
	  tweak5 = asm_xor (tweak5, rkey0);
	  tweak6 = asm_xor (tweak6, rkey0);
	  tweak7 = asm_xor (tweak7, rkey0);

	  b0 = asm_xor (b0, tweak0);
	  b1 = asm_xor (b1, tweak1);
	  b2 = asm_xor (b2, tweak2);
	  b3 = asm_xor (b3, tweak3);
	  b4 = asm_xor (b4, tweak4);
	  b5 = asm_xor (b5, tweak5);
	  b6 = asm_xor (b6, tweak6);
	  b7 = asm_xor (b7, tweak7);

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_ncipher_be (b0, rkey); \
	      b1 = asm_ncipher_be (b1, rkey); \
	      b2 = asm_ncipher_be (b2, rkey); \
	      b3 = asm_ncipher_be (b3, rkey); \
	      b4 = asm_ncipher_be (b4, rkey); \
	      b5 = asm_ncipher_be (b5, rkey); \
	      b6 = asm_ncipher_be (b6, rkey); \
	      b7 = asm_ncipher_be (b7, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);

	  rkeylf = asm_xor (rkeylast, rkey0);

	  DO_ROUND(8);

	  tweak0 = asm_xor (tweak0, rkeylf);
	  tweak1 = asm_xor (tweak1, rkeylf);
	  tweak2 = asm_xor (tweak2, rkeylf);
	  tweak3 = asm_xor (tweak3, rkeylf);
	  tweak4 = asm_xor (tweak4, rkeylf);
	  tweak5 = asm_xor (tweak5, rkeylf);
	  tweak6 = asm_xor (tweak6, rkeylf);
	  tweak7 = asm_xor (tweak7, rkeylf);

	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  b0 = asm_ncipherlast_be (b0, tweak0);
	  b1 = asm_ncipherlast_be (b1, tweak1);
	  b2 = asm_ncipherlast_be (b2, tweak2);
	  b3 = asm_ncipherlast_be (b3, tweak3);
	  b0 = VEC_BE_SWAP (b0, bige_const);
	  b1 = VEC_BE_SWAP (b1, bige_const);
	  b4 = asm_ncipherlast_be (b4, tweak4);
	  b5 = asm_ncipherlast_be (b5, tweak5);
	  b2 = VEC_BE_SWAP (b2, bige_const);
	  b3 = VEC_BE_SWAP (b3, bige_const);
	  b6 = asm_ncipherlast_be (b6, tweak6);
	  b7 = asm_ncipherlast_be (b7, tweak7);
	  VEC_STORE_BE_NOSWAP (out, 0, b0);
	  VEC_STORE_BE_NOSWAP (out, 1, b1);
	  b4 = VEC_BE_SWAP (b4, bige_const);
	  b5 = VEC_BE_SWAP (b5, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 2, b2);
	  VEC_STORE_BE_NOSWAP (out, 3, b3);
	  b6 = VEC_BE_SWAP (b6, bige_const);
	  b7 = VEC_BE_SWAP (b7, bige_const);
	  VEC_STORE_BE_NOSWAP (out, 4, b4);
	  VEC_STORE_BE_NOSWAP (out, 5, b5);
	  VEC_STORE_BE_NOSWAP (out, 6, b6);
	  VEC_STORE_BE_NOSWAP (out, 7, b7);
	  out += 8;
	}

      if (nblocks >= 4)
	{
	  tweak0 = tweak;
	  GEN_TWEAK (tweak1, tweak0);
	  GEN_TWEAK (tweak2, tweak1);
	  GEN_TWEAK (tweak3, tweak2);
	  GEN_TWEAK (tweak, tweak3);

	  b0 = VEC_LOAD_BE (in, 0, bige_const);
	  b1 = VEC_LOAD_BE (in, 1, bige_const);
	  b2 = VEC_LOAD_BE (in, 2, bige_const);
	  b3 = VEC_LOAD_BE (in, 3, bige_const);

	  tweak0 = asm_vperm1 (tweak0, bswap128_const);
	  tweak1 = asm_vperm1 (tweak1, bswap128_const);
	  tweak2 = asm_vperm1 (tweak2, bswap128_const);
	  tweak3 = asm_vperm1 (tweak3, bswap128_const);

	  b0 ^= tweak0 ^ rkey0;
	  b1 ^= tweak1 ^ rkey0;
	  b2 ^= tweak2 ^ rkey0;
	  b3 ^= tweak3 ^ rkey0;

#define DO_ROUND(r) \
	      rkey = ALIGNED_LOAD (rk, r); \
	      b0 = asm_ncipher_be (b0, rkey); \
	      b1 = asm_ncipher_be (b1, rkey); \
	      b2 = asm_ncipher_be (b2, rkey); \
	      b3 = asm_ncipher_be (b3, rkey);

	  DO_ROUND(1);
	  DO_ROUND(2);
	  DO_ROUND(3);
	  DO_ROUND(4);
	  DO_ROUND(5);
	  DO_ROUND(6);
	  DO_ROUND(7);
	  DO_ROUND(8);
	  DO_ROUND(9);
	  if (rounds >= 12)
	    {
	      DO_ROUND(10);
	      DO_ROUND(11);
	      if (rounds > 12)
		{
		  DO_ROUND(12);
		  DO_ROUND(13);
		}
	    }

#undef DO_ROUND

	  rkey = rkeylast;
	  b0 = asm_ncipherlast_be (b0, rkey ^ tweak0);
	  b1 = asm_ncipherlast_be (b1, rkey ^ tweak1);
	  b2 = asm_ncipherlast_be (b2, rkey ^ tweak2);
	  b3 = asm_ncipherlast_be (b3, rkey ^ tweak3);

	  VEC_STORE_BE (out, 0, b0, bige_const);
	  VEC_STORE_BE (out, 1, b1, bige_const);
	  VEC_STORE_BE (out, 2, b2, bige_const);
	  VEC_STORE_BE (out, 3, b3, bige_const);

	  in += 4;
	  out += 4;
	  nblocks -= 4;
	}

      for (; nblocks; nblocks--)
	{
	  tweak0 = asm_vperm1 (tweak, bswap128_const);

	  /* Xor-Encrypt/Decrypt-Xor block. */
	  b = VEC_LOAD_BE (in, 0, bige_const) ^ tweak0;

	  /* Generate next tweak. */
	  GEN_TWEAK (tweak, tweak);

	  AES_DECRYPT (b, rounds);

	  b ^= tweak0;
	  VEC_STORE_BE (out, 0, b, bige_const);

	  in++;
	  out++;
	}
    }

  tweak = asm_vperm1 (tweak, bswap128_const);
  VEC_STORE_BE (tweak_arg, 0, tweak, bige_const);

#undef GEN_TWEAK
}
