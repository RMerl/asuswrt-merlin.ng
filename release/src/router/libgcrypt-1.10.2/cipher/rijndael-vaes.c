/* VAES/AVX2 accelerated AES for Libgcrypt
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"  /* for byte and u32 typedefs */
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "cipher-selftest.h"
#include "rijndael-internal.h"
#include "./cipher-internal.h"


#ifdef USE_VAES


# ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
#  define ASM_FUNC_ABI __attribute__((sysv_abi))
# else
#  define ASM_FUNC_ABI
# endif


extern void _gcry_aes_aesni_prepare_decryption(RIJNDAEL_context *ctx);


extern void _gcry_vaes_avx2_cbc_dec_amd64 (const void *keysched,
					   unsigned char *iv,
					   void *outbuf_arg,
					   const void *inbuf_arg,
					   size_t nblocks,
					   unsigned int nrounds) ASM_FUNC_ABI;

extern void _gcry_vaes_avx2_cfb_dec_amd64 (const void *keysched,
					   unsigned char *iv,
					   void *outbuf_arg,
					   const void *inbuf_arg,
					   size_t nblocks,
					   unsigned int nrounds) ASM_FUNC_ABI;

extern void _gcry_vaes_avx2_ctr_enc_amd64 (const void *keysched,
					   unsigned char *ctr,
					   void *outbuf_arg,
					   const void *inbuf_arg,
					   size_t nblocks,
					   unsigned int nrounds) ASM_FUNC_ABI;

extern void _gcry_vaes_avx2_ctr32le_enc_amd64 (const void *keysched,
					       unsigned char *ctr,
					       void *outbuf_arg,
					       const void *inbuf_arg,
					       size_t nblocks,
					       unsigned int nrounds)
						ASM_FUNC_ABI;

extern void _gcry_vaes_avx2_ocb_crypt_amd64 (const void *keysched,
					     unsigned int blkn,
					     void *outbuf_arg,
					     const void *inbuf_arg,
					     size_t nblocks,
					     unsigned int nrounds,
					     unsigned char *offset,
					     unsigned char *checksum,
					     unsigned char *L_table,
					     int encrypt) ASM_FUNC_ABI;

extern void _gcry_vaes_avx2_xts_crypt_amd64 (const void *keysched,
					     unsigned char *tweak,
					     void *outbuf_arg,
					     const void *inbuf_arg,
					     size_t nblocks,
					     unsigned int nrounds,
					     int encrypt) ASM_FUNC_ABI;


void
_gcry_aes_vaes_cbc_dec (void *context, unsigned char *iv,
			void *outbuf, const void *inbuf,
			size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  const void *keysched = ctx->keyschdec32;
  unsigned int nrounds = ctx->rounds;

  if (!ctx->decryption_prepared)
    {
      _gcry_aes_aesni_prepare_decryption (ctx);
      ctx->decryption_prepared = 1;
    }

  _gcry_vaes_avx2_cbc_dec_amd64 (keysched, iv, outbuf, inbuf, nblocks, nrounds);
}

void
_gcry_aes_vaes_cfb_dec (void *context, unsigned char *iv,
			void *outbuf, const void *inbuf,
			size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_vaes_avx2_cfb_dec_amd64 (keysched, iv, outbuf, inbuf, nblocks, nrounds);
}

void
_gcry_aes_vaes_ctr_enc (void *context, unsigned char *iv,
			void *outbuf, const void *inbuf,
			size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_vaes_avx2_ctr_enc_amd64 (keysched, iv, outbuf, inbuf, nblocks, nrounds);
}

void
_gcry_aes_vaes_ctr32le_enc (void *context, unsigned char *iv,
			    void *outbuf, const void *inbuf,
			    size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  const void *keysched = ctx->keyschenc32;
  unsigned int nrounds = ctx->rounds;

  _gcry_vaes_avx2_ctr32le_enc_amd64 (keysched, iv, outbuf, inbuf, nblocks,
				     nrounds);
}

size_t
_gcry_aes_vaes_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
			  const void *inbuf_arg, size_t nblocks,
			  int encrypt)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const void *keysched = encrypt ? ctx->keyschenc32 : ctx->keyschdec32;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int nrounds = ctx->rounds;
  u64 blkn = c->u_mode.ocb.data_nblocks;

  if (!encrypt && !ctx->decryption_prepared)
    {
      _gcry_aes_aesni_prepare_decryption (ctx);
      ctx->decryption_prepared = 1;
    }

  c->u_mode.ocb.data_nblocks = blkn + nblocks;

  _gcry_vaes_avx2_ocb_crypt_amd64 (keysched, (unsigned int)blkn, outbuf, inbuf,
				   nblocks, nrounds, c->u_iv.iv, c->u_ctr.ctr,
				   c->u_mode.ocb.L[0], encrypt);

  return 0;
}

void
_gcry_aes_vaes_xts_crypt (void *context, unsigned char *tweak,
			  void *outbuf, const void *inbuf,
			  size_t nblocks, int encrypt)
{
  RIJNDAEL_context *ctx = context;
  const void *keysched = encrypt ? ctx->keyschenc32 : ctx->keyschdec32;
  unsigned int nrounds = ctx->rounds;

  if (!encrypt && !ctx->decryption_prepared)
    {
      _gcry_aes_aesni_prepare_decryption (ctx);
      ctx->decryption_prepared = 1;
    }

  _gcry_vaes_avx2_xts_crypt_amd64 (keysched, tweak, outbuf, inbuf, nblocks,
				   nrounds, encrypt);
}

#endif /* USE_VAES */
