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

#ifndef G10_RIJNDAEL_PPC_COMMON_H
#define G10_RIJNDAEL_PPC_COMMON_H

#include <altivec.h>


typedef vector unsigned char block;

typedef union
{
  u32 data32[4];
} __attribute__((packed, aligned(1), may_alias)) u128_t;


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR          NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE   ASM_FUNC_ATTR ALWAYS_INLINE
#define ASM_FUNC_ATTR_NOINLINE ASM_FUNC_ATTR NO_INLINE


#define ALIGNED_LOAD(in_ptr, offs) \
  (asm_aligned_ld ((offs) * 16, (const void *)(in_ptr)))

#define ALIGNED_STORE(out_ptr, offs, vec) \
  (asm_aligned_st ((vec), (offs) * 16, (void *)(out_ptr)))

#define VEC_BE_SWAP(vec, bige_const) (asm_be_swap ((vec), (bige_const)))

#define VEC_LOAD_BE(in_ptr, offs, bige_const) \
  (asm_be_swap (asm_load_be_noswap ((offs) * 16, (const void *)(in_ptr)), \
		bige_const))

#define VEC_LOAD_BE_NOSWAP(in_ptr, offs) \
  (asm_load_be_noswap ((offs) * 16, (const unsigned char *)(in_ptr)))

#define VEC_STORE_BE(out_ptr, offs, vec, bige_const) \
  (asm_store_be_noswap (asm_be_swap ((vec), (bige_const)), (offs) * 16, \
		        (void *)(out_ptr)))

#define VEC_STORE_BE_NOSWAP(out_ptr, offs, vec) \
  (asm_store_be_noswap ((vec), (offs) * 16, (void *)(out_ptr)))


#define ROUND_KEY_VARIABLES \
  block rkey0, rkeylast

#define PRELOAD_ROUND_KEYS(nrounds) \
  do { \
    rkey0 = ALIGNED_LOAD (rk, 0); \
    rkeylast = ALIGNED_LOAD (rk, nrounds); \
  } while (0)

#define AES_ENCRYPT(blk, nrounds) \
  do { \
    blk ^= rkey0; \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 1)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 2)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 3)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 4)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 5)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 6)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 7)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 8)); \
    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 9)); \
    if (nrounds >= 12) \
      { \
	blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 10)); \
	blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 11)); \
	if (rounds > 12) \
	  { \
	    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 12)); \
	    blk = asm_cipher_be (blk, ALIGNED_LOAD (rk, 13)); \
	  } \
      } \
    blk = asm_cipherlast_be (blk, rkeylast); \
  } while (0)

#define AES_DECRYPT(blk, nrounds) \
  do { \
    blk ^= rkey0; \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 1)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 2)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 3)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 4)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 5)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 6)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 7)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 8)); \
    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 9)); \
    if (nrounds >= 12) \
      { \
	blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 10)); \
	blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 11)); \
	if (rounds > 12) \
	  { \
	    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 12)); \
	    blk = asm_ncipher_be (blk, ALIGNED_LOAD (rk, 13)); \
	  } \
      } \
    blk = asm_ncipherlast_be (blk, rkeylast); \
  } while (0)


#define ROUND_KEY_VARIABLES_ALL \
  block rkey0, rkey1, rkey2, rkey3, rkey4, rkey5, rkey6, rkey7, rkey8, \
        rkey9, rkey10, rkey11, rkey12, rkey13, rkeylast

#define PRELOAD_ROUND_KEYS_ALL(nrounds) \
  do { \
    rkey0 = ALIGNED_LOAD (rk, 0); \
    rkey1 = ALIGNED_LOAD (rk, 1); \
    rkey2 = ALIGNED_LOAD (rk, 2); \
    rkey3 = ALIGNED_LOAD (rk, 3); \
    rkey4 = ALIGNED_LOAD (rk, 4); \
    rkey5 = ALIGNED_LOAD (rk, 5); \
    rkey6 = ALIGNED_LOAD (rk, 6); \
    rkey7 = ALIGNED_LOAD (rk, 7); \
    rkey8 = ALIGNED_LOAD (rk, 8); \
    rkey9 = ALIGNED_LOAD (rk, 9); \
    if (nrounds >= 12) \
      { \
	rkey10 = ALIGNED_LOAD (rk, 10); \
	rkey11 = ALIGNED_LOAD (rk, 11); \
	if (rounds > 12) \
	  { \
	    rkey12 = ALIGNED_LOAD (rk, 12); \
	    rkey13 = ALIGNED_LOAD (rk, 13); \
	  } \
      } \
    rkeylast = ALIGNED_LOAD (rk, nrounds); \
  } while (0)

#define AES_ENCRYPT_ALL(blk, nrounds) \
  do { \
    blk ^= rkey0; \
    blk = asm_cipher_be (blk, rkey1); \
    blk = asm_cipher_be (blk, rkey2); \
    blk = asm_cipher_be (blk, rkey3); \
    blk = asm_cipher_be (blk, rkey4); \
    blk = asm_cipher_be (blk, rkey5); \
    blk = asm_cipher_be (blk, rkey6); \
    blk = asm_cipher_be (blk, rkey7); \
    blk = asm_cipher_be (blk, rkey8); \
    blk = asm_cipher_be (blk, rkey9); \
    if (nrounds >= 12) \
      { \
	blk = asm_cipher_be (blk, rkey10); \
	blk = asm_cipher_be (blk, rkey11); \
	if (rounds > 12) \
	  { \
	    blk = asm_cipher_be (blk, rkey12); \
	    blk = asm_cipher_be (blk, rkey13); \
	  } \
      } \
    blk = asm_cipherlast_be (blk, rkeylast); \
  } while (0)


static ASM_FUNC_ATTR_INLINE block
asm_aligned_ld(unsigned long offset, const void *ptr)
{
  block vec;
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("lvx %0,0,%1\n\t"
		      : "=v" (vec)
		      : "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("lvx %0,%1,%2\n\t"
		      : "=v" (vec)
		      : "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
  return vec;
}

static ASM_FUNC_ATTR_INLINE void
asm_aligned_st(block vec, unsigned long offset, void *ptr)
{
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("stvx %0,0,%1\n\t"
		      :
		      : "v" (vec), "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("stvx %0,%1,%2\n\t"
		      :
		      : "v" (vec), "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
}

static ASM_FUNC_ATTR_INLINE block
asm_vperm1(block vec, block mask)
{
  block o;
  __asm__ volatile ("vperm %0,%1,%1,%2\n\t"
		    : "=v" (o)
		    : "v" (vec), "v" (mask));
  return o;
}

static ASM_FUNC_ATTR_INLINE block
asm_add_uint128(block a, block b)
{
  block res;
  __asm__ volatile ("vadduqm %0,%1,%2\n\t"
		    : "=v" (res)
		    : "v" (a), "v" (b));
  return res;
}

static ASM_FUNC_ATTR_INLINE block
asm_add_uint64(block a, block b)
{
  block res;
  __asm__ volatile ("vaddudm %0,%1,%2\n\t"
		    : "=v" (res)
		    : "v" (a), "v" (b));
  return res;
}

static ASM_FUNC_ATTR_INLINE block
asm_sra_int64(block a, block b)
{
  block res;
  __asm__ volatile ("vsrad %0,%1,%2\n\t"
		    : "=v" (res)
		    : "v" (a), "v" (b));
  return res;
}

static block
asm_swap_uint64_halfs(block a)
{
  block res;
  __asm__ volatile ("xxswapd %x0, %x1"
		    : "=wa" (res)
		    : "wa" (a));
  return res;
}

static ASM_FUNC_ATTR_INLINE block
asm_xor(block a, block b)
{
  block res;
  __asm__ volatile ("vxor %0,%1,%2\n\t"
		    : "=v" (res)
		    : "v" (a), "v" (b));
  return res;
}

static ASM_FUNC_ATTR_INLINE block
asm_cipher_be(block b, block rk)
{
  block o;
  __asm__ volatile ("vcipher %0, %1, %2\n\t"
		    : "=v" (o)
		    : "v" (b), "v" (rk));
  return o;
}

static ASM_FUNC_ATTR_INLINE block
asm_cipherlast_be(block b, block rk)
{
  block o;
  __asm__ volatile ("vcipherlast %0, %1, %2\n\t"
		    : "=v" (o)
		    : "v" (b), "v" (rk));
  return o;
}

static ASM_FUNC_ATTR_INLINE block
asm_ncipher_be(block b, block rk)
{
  block o;
  __asm__ volatile ("vncipher %0, %1, %2\n\t"
		    : "=v" (o)
		    : "v" (b), "v" (rk));
  return o;
}

static ASM_FUNC_ATTR_INLINE block
asm_ncipherlast_be(block b, block rk)
{
  block o;
  __asm__ volatile ("vncipherlast %0, %1, %2\n\t"
		    : "=v" (o)
		    : "v" (b), "v" (rk));
  return o;
}


/* Make a decryption key from an encryption key. */
static ASM_FUNC_ATTR_INLINE void
internal_aes_ppc_prepare_decryption (RIJNDAEL_context *ctx)
{
  u128_t *ekey = (u128_t *)(void *)ctx->keyschenc;
  u128_t *dkey = (u128_t *)(void *)ctx->keyschdec;
  int rounds = ctx->rounds;
  int rr;
  int r;

  r = 0;
  rr = rounds;
  for (r = 0, rr = rounds; r <= rounds; r++, rr--)
    {
      ALIGNED_STORE (dkey, r, ALIGNED_LOAD (ekey, rr));
    }
}

#endif /* G10_RIJNDAEL_PPC_COMMON_H */
