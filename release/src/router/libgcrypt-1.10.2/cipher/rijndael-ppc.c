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

#include <config.h>

#include "rijndael-internal.h"
#include "cipher-internal.h"
#include "bufhelp.h"

#ifdef USE_PPC_CRYPTO

#include "rijndael-ppc-common.h"


#ifdef WORDS_BIGENDIAN
static const block vec_bswap32_const =
  { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12 };
#else
static const block vec_bswap32_const_neg =
  { ~3, ~2, ~1, ~0, ~7, ~6, ~5, ~4, ~11, ~10, ~9, ~8, ~15, ~14, ~13, ~12 };
#endif


static ASM_FUNC_ATTR_INLINE block
asm_load_be_const(void)
{
#ifndef WORDS_BIGENDIAN
  return ALIGNED_LOAD (&vec_bswap32_const_neg, 0);
#else
  static const block vec_dummy = { 0 };
  return vec_dummy;
#endif
}

static ASM_FUNC_ATTR_INLINE block
asm_be_swap(block vec, block be_bswap_const)
{
  (void)be_bswap_const;
#ifndef WORDS_BIGENDIAN
  return asm_vperm1 (vec, be_bswap_const);
#else
  return vec;
#endif
}

static ASM_FUNC_ATTR_INLINE block
asm_load_be_noswap(unsigned long offset, const void *ptr)
{
  block vec;
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("lxvw4x %x0,0,%1\n\t"
		      : "=wa" (vec)
		      : "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("lxvw4x %x0,%1,%2\n\t"
		      : "=wa" (vec)
		      : "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
  /* NOTE: vec needs to be be-swapped using 'asm_be_swap' by caller */
  return vec;
}

static ASM_FUNC_ATTR_INLINE void
asm_store_be_noswap(block vec, unsigned long offset, void *ptr)
{
  /* NOTE: vec be-swapped using 'asm_be_swap' by caller */
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("stxvw4x %x0,0,%1\n\t"
		      :
		      : "wa" (vec), "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("stxvw4x %x0,%1,%2\n\t"
		      :
		      : "wa" (vec), "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
}


static ASM_FUNC_ATTR_INLINE u32
_gcry_aes_sbox4_ppc8(u32 fourbytes)
{
  union
    {
      PROPERLY_ALIGNED_TYPE dummy;
      block data_vec;
      u32 data32[4];
    } u;

  u.data32[0] = fourbytes;
  u.data_vec = vec_sbox_be(u.data_vec);
  return u.data32[0];
}

void
_gcry_aes_ppc8_setkey (RIJNDAEL_context *ctx, const byte *key)
{
  const block bige_const = asm_load_be_const();
  union
    {
      PROPERLY_ALIGNED_TYPE dummy;
      byte data[MAXKC][4];
      u32 data32[MAXKC];
    } tkk[2];
  unsigned int rounds = ctx->rounds;
  int KC = rounds - 6;
  unsigned int keylen = KC * 4;
  u128_t *ekey = (u128_t *)(void *)ctx->keyschenc;
  unsigned int i, r, t;
  byte rcon = 1;
  int j;
#define k      tkk[0].data
#define k_u32  tkk[0].data32
#define tk     tkk[1].data
#define tk_u32 tkk[1].data32
#define W      (ctx->keyschenc)
#define W_u32  (ctx->keyschenc32)

  for (i = 0; i < keylen; i++)
    {
      k[i >> 2][i & 3] = key[i];
    }

  for (j = KC-1; j >= 0; j--)
    {
      tk_u32[j] = k_u32[j];
    }
  r = 0;
  t = 0;
  /* Copy values into round key array.  */
  for (j = 0; (j < KC) && (r < rounds + 1); )
    {
      for (; (j < KC) && (t < 4); j++, t++)
        {
          W_u32[r][t] = le_bswap32(tk_u32[j]);
        }
      if (t == 4)
        {
          r++;
          t = 0;
        }
    }
  while (r < rounds + 1)
    {
      tk_u32[0] ^=
	le_bswap32(
	  _gcry_aes_sbox4_ppc8(rol(le_bswap32(tk_u32[KC - 1]), 24)) ^ rcon);

      if (KC != 8)
        {
          for (j = 1; j < KC; j++)
            {
              tk_u32[j] ^= tk_u32[j-1];
            }
        }
      else
        {
          for (j = 1; j < KC/2; j++)
            {
              tk_u32[j] ^= tk_u32[j-1];
            }

          tk_u32[KC/2] ^=
	    le_bswap32(_gcry_aes_sbox4_ppc8(le_bswap32(tk_u32[KC/2 - 1])));

          for (j = KC/2 + 1; j < KC; j++)
            {
              tk_u32[j] ^= tk_u32[j-1];
            }
        }

      /* Copy values into round key array.  */
      for (j = 0; (j < KC) && (r < rounds + 1); )
        {
          for (; (j < KC) && (t < 4); j++, t++)
            {
              W_u32[r][t] = le_bswap32(tk_u32[j]);
            }
          if (t == 4)
            {
              r++;
              t = 0;
            }
        }

      rcon = (rcon << 1) ^ (-(rcon >> 7) & 0x1b);
    }

  /* Store in big-endian order. */
  for (r = 0; r <= rounds; r++)
    {
#ifndef WORDS_BIGENDIAN
      VEC_STORE_BE(ekey, r, ALIGNED_LOAD (ekey, r), bige_const);
#else
      block rvec = ALIGNED_LOAD (ekey, r);
      ALIGNED_STORE (ekey, r,
                     vec_perm(rvec, rvec, vec_bswap32_const));
      (void)bige_const;
#endif
    }

#undef W
#undef tk
#undef k
#undef W_u32
#undef tk_u32
#undef k_u32
  wipememory(&tkk, sizeof(tkk));
}

void
_gcry_aes_ppc8_prepare_decryption (RIJNDAEL_context *ctx)
{
  internal_aes_ppc_prepare_decryption (ctx);
}


#define GCRY_AES_PPC8 1
#define ENCRYPT_BLOCK_FUNC	_gcry_aes_ppc8_encrypt
#define DECRYPT_BLOCK_FUNC	_gcry_aes_ppc8_decrypt
#define CFB_ENC_FUNC		_gcry_aes_ppc8_cfb_enc
#define CFB_DEC_FUNC		_gcry_aes_ppc8_cfb_dec
#define CBC_ENC_FUNC		_gcry_aes_ppc8_cbc_enc
#define CBC_DEC_FUNC		_gcry_aes_ppc8_cbc_dec
#define CTR_ENC_FUNC		_gcry_aes_ppc8_ctr_enc
#define OCB_CRYPT_FUNC		_gcry_aes_ppc8_ocb_crypt
#define OCB_AUTH_FUNC		_gcry_aes_ppc8_ocb_auth
#define XTS_CRYPT_FUNC		_gcry_aes_ppc8_xts_crypt

#include <rijndael-ppc-functions.h>

#endif /* USE_PPC_CRYPTO */
