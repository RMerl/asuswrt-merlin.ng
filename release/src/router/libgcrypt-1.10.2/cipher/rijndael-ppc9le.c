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

#ifdef USE_PPC_CRYPTO_WITH_PPC9LE

#include "rijndael-ppc-common.h"


static ASM_FUNC_ATTR_INLINE block
asm_load_be_const(void)
{
  static const block vec_dummy = { 0 };
  return vec_dummy;
}

static ASM_FUNC_ATTR_INLINE block
asm_be_swap(block vec, block be_bswap_const)
{
  (void)be_bswap_const;
  return vec;
}

static ASM_FUNC_ATTR_INLINE block
asm_load_be_noswap(unsigned long offset, const void *ptr)
{
  block vec;
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("lxvb16x %x0,0,%1\n\t"
		      : "=wa" (vec)
		      : "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("lxvb16x %x0,%1,%2\n\t"
		      : "=wa" (vec)
		      : "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
  return vec;
}

static ASM_FUNC_ATTR_INLINE void
asm_store_be_noswap(block vec, unsigned long offset, void *ptr)
{
#if __GNUC__ >= 4
  if (__builtin_constant_p (offset) && offset == 0)
    __asm__ volatile ("stxvb16x %x0,0,%1\n\t"
		      :
		      : "wa" (vec), "r" ((uintptr_t)ptr)
		      : "memory");
  else
#endif
    __asm__ volatile ("stxvb16x %x0,%1,%2\n\t"
		      :
		      : "wa" (vec), "r" (offset), "r" ((uintptr_t)ptr)
		      : "memory", "r0");
}


#define GCRY_AES_PPC9LE 1
#define ENCRYPT_BLOCK_FUNC	_gcry_aes_ppc9le_encrypt
#define DECRYPT_BLOCK_FUNC	_gcry_aes_ppc9le_decrypt
#define CFB_ENC_FUNC		_gcry_aes_ppc9le_cfb_enc
#define CFB_DEC_FUNC		_gcry_aes_ppc9le_cfb_dec
#define CBC_ENC_FUNC		_gcry_aes_ppc9le_cbc_enc
#define CBC_DEC_FUNC		_gcry_aes_ppc9le_cbc_dec
#define CTR_ENC_FUNC		_gcry_aes_ppc9le_ctr_enc
#define OCB_CRYPT_FUNC		_gcry_aes_ppc9le_ocb_crypt
#define OCB_AUTH_FUNC		_gcry_aes_ppc9le_ocb_auth
#define XTS_CRYPT_FUNC		_gcry_aes_ppc9le_xts_crypt

#include <rijndael-ppc-functions.h>

#endif /* USE_PPC_CRYPTO */
