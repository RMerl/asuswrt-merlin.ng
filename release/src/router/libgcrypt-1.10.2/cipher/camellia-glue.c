/* camellia-glue.c - Glue for the Camellia cipher
 * Copyright (C) 2007 Free Software Foundation, Inc.
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/* I put all the libgcrypt-specific stuff in this file to keep the
   camellia.c/camellia.h files exactly as provided by NTT.  If they
   update their code, this should make it easier to bring the changes
   in. - dshaw

   There is one small change which needs to be done: Include the
   following code at the top of camellia.h: */
#if 0

/* To use Camellia with libraries it is often useful to keep the name
 * space of the library clean.  The following macro is thus useful:
 *
 *     #define CAMELLIA_EXT_SYM_PREFIX foo_
 *
 * This prefixes all external symbols with "foo_".
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef CAMELLIA_EXT_SYM_PREFIX
#define CAMELLIA_PREFIX1(x,y) x ## y
#define CAMELLIA_PREFIX2(x,y) CAMELLIA_PREFIX1(x,y)
#define CAMELLIA_PREFIX(x)    CAMELLIA_PREFIX2(CAMELLIA_EXT_SYM_PREFIX,x)
#define Camellia_Ekeygen      CAMELLIA_PREFIX(Camellia_Ekeygen)
#define Camellia_EncryptBlock CAMELLIA_PREFIX(Camellia_EncryptBlock)
#define Camellia_DecryptBlock CAMELLIA_PREFIX(Camellia_DecryptBlock)
#define camellia_decrypt128   CAMELLIA_PREFIX(camellia_decrypt128)
#define camellia_decrypt256   CAMELLIA_PREFIX(camellia_decrypt256)
#define camellia_encrypt128   CAMELLIA_PREFIX(camellia_encrypt128)
#define camellia_encrypt256   CAMELLIA_PREFIX(camellia_encrypt256)
#define camellia_setup128     CAMELLIA_PREFIX(camellia_setup128)
#define camellia_setup192     CAMELLIA_PREFIX(camellia_setup192)
#define camellia_setup256     CAMELLIA_PREFIX(camellia_setup256)
#endif /*CAMELLIA_EXT_SYM_PREFIX*/

#endif /* Code sample. */


#include <config.h>
#include "types.h"
#include "g10lib.h"
#include "cipher.h"
#include "camellia.h"
#include "bufhelp.h"
#include "cipher-internal.h"
#include "cipher-selftest.h"

/* Helper macro to force alignment to 16 bytes.  */
#ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
# define ATTR_ALIGNED_16  __attribute__ ((aligned (16)))
#else
# define ATTR_ALIGNED_16
#endif

/* USE_AESNI inidicates whether to compile with Intel AES-NI/AVX code. */
#undef USE_AESNI_AVX
#if defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)
# if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
#  define USE_AESNI_AVX 1
# endif
#endif

/* USE_AESNI_AVX2 inidicates whether to compile with Intel AES-NI/AVX2 code. */
#undef USE_AESNI_AVX2
#if defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX2_SUPPORT)
# if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
#  define USE_AESNI_AVX2 1
# endif
#endif

/* USE_VAES_AVX2 inidicates whether to compile with Intel VAES/AVX2 code. */
#undef USE_VAES_AVX2
#if defined(USE_AESNI_AVX2) && defined(HAVE_GCC_INLINE_ASM_VAES_VPCLMUL)
# define USE_VAES_AVX2 1
#endif

typedef struct
{
  KEY_TABLE_TYPE keytable;
  int keybitlength;
#ifdef USE_AESNI_AVX
  unsigned int use_aesni_avx:1;	/* AES-NI/AVX implementation shall be used.  */
#endif /*USE_AESNI_AVX*/
#ifdef USE_AESNI_AVX2
  unsigned int use_aesni_avx2:1;/* AES-NI/AVX2 implementation shall be used.  */
  unsigned int use_vaes_avx2:1; /* VAES/AVX2 implementation shall be used.  */
#endif /*USE_AESNI_AVX2*/
} CAMELLIA_context;

/* Assembly implementations use SystemV ABI, ABI conversion and additional
 * stack to store XMM6-XMM15 needed on Win64. */
#undef ASM_FUNC_ABI
#undef ASM_EXTRA_STACK
#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2)
# ifdef HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS
#  define ASM_FUNC_ABI __attribute__((sysv_abi))
#  define ASM_EXTRA_STACK (10 * 16)
# else
#  define ASM_FUNC_ABI
#  define ASM_EXTRA_STACK 0
# endif
#endif

#ifdef USE_AESNI_AVX
/* Assembler implementations of Camellia using AES-NI and AVX.  Process data
   in 16 block same time.
 */
extern void _gcry_camellia_aesni_avx_ctr_enc(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *ctr) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx_cbc_dec(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx_cfb_dec(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx_ocb_enc(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *offset,
					     unsigned char *checksum,
					     const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx_ocb_dec(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *offset,
					     unsigned char *checksum,
					     const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx_ocb_auth(CAMELLIA_context *ctx,
					     const unsigned char *abuf,
					     unsigned char *offset,
					     unsigned char *checksum,
					     const u64 Ls[16]) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx_keygen(CAMELLIA_context *ctx,
					    const unsigned char *key,
					    unsigned int keylen) ASM_FUNC_ABI;
#endif

#ifdef USE_AESNI_AVX2
/* Assembler implementations of Camellia using AES-NI and AVX2.  Process data
   in 32 block same time.
 */
extern void _gcry_camellia_aesni_avx2_ctr_enc(CAMELLIA_context *ctx,
					      unsigned char *out,
					      const unsigned char *in,
					      unsigned char *ctr) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx2_cbc_dec(CAMELLIA_context *ctx,
					      unsigned char *out,
					      const unsigned char *in,
					      unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx2_cfb_dec(CAMELLIA_context *ctx,
					      unsigned char *out,
					      const unsigned char *in,
					      unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx2_ocb_enc(CAMELLIA_context *ctx,
					      unsigned char *out,
					      const unsigned char *in,
					      unsigned char *offset,
					      unsigned char *checksum,
					      const u64 Ls[32]) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx2_ocb_dec(CAMELLIA_context *ctx,
					      unsigned char *out,
					      const unsigned char *in,
					      unsigned char *offset,
					      unsigned char *checksum,
					      const u64 Ls[32]) ASM_FUNC_ABI;

extern void _gcry_camellia_aesni_avx2_ocb_auth(CAMELLIA_context *ctx,
					       const unsigned char *abuf,
					       unsigned char *offset,
					       unsigned char *checksum,
					       const u64 Ls[32]) ASM_FUNC_ABI;
#endif

#ifdef USE_VAES_AVX2
/* Assembler implementations of Camellia using VAES and AVX2.  Process data
   in 32 block same time.
 */
extern void _gcry_camellia_vaes_avx2_ctr_enc(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *ctr) ASM_FUNC_ABI;

extern void _gcry_camellia_vaes_avx2_cbc_dec(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_camellia_vaes_avx2_cfb_dec(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *iv) ASM_FUNC_ABI;

extern void _gcry_camellia_vaes_avx2_ocb_enc(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *offset,
					     unsigned char *checksum,
					     const u64 Ls[32]) ASM_FUNC_ABI;

extern void _gcry_camellia_vaes_avx2_ocb_dec(CAMELLIA_context *ctx,
					     unsigned char *out,
					     const unsigned char *in,
					     unsigned char *offset,
					     unsigned char *checksum,
					     const u64 Ls[32]) ASM_FUNC_ABI;

extern void _gcry_camellia_vaes_avx2_ocb_auth(CAMELLIA_context *ctx,
					      const unsigned char *abuf,
					      unsigned char *offset,
					      unsigned char *checksum,
					      const u64 Ls[32]) ASM_FUNC_ABI;
#endif

static const char *selftest(void);

static void _gcry_camellia_ctr_enc (void *context, unsigned char *ctr,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
static void _gcry_camellia_cbc_dec (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
static void _gcry_camellia_cfb_dec (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
static size_t _gcry_camellia_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
					const void *inbuf_arg, size_t nblocks,
					int encrypt);
static size_t _gcry_camellia_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
				       size_t nblocks);

static gcry_err_code_t
camellia_setkey(void *c, const byte *key, unsigned keylen,
                cipher_bulk_ops_t *bulk_ops)
{
  CAMELLIA_context *ctx=c;
  static int initialized=0;
  static const char *selftest_failed=NULL;
#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2) || defined(USE_VAES_AVX2)
  unsigned int hwf = _gcry_get_hw_features ();
#endif

  if(keylen!=16 && keylen!=24 && keylen!=32)
    return GPG_ERR_INV_KEYLEN;

  if(!initialized)
    {
      initialized=1;
      selftest_failed=selftest();
      if(selftest_failed)
	log_error("%s\n",selftest_failed);
    }

  if(selftest_failed)
    return GPG_ERR_SELFTEST_FAILED;

#ifdef USE_AESNI_AVX
  ctx->use_aesni_avx = (hwf & HWF_INTEL_AESNI) && (hwf & HWF_INTEL_AVX);
#endif
#ifdef USE_AESNI_AVX2
  ctx->use_aesni_avx2 = (hwf & HWF_INTEL_AESNI) && (hwf & HWF_INTEL_AVX2);
  ctx->use_vaes_avx2 = 0;
#endif
#ifdef USE_VAES_AVX2
  ctx->use_vaes_avx2 = (hwf & HWF_INTEL_VAES_VPCLMUL) && (hwf & HWF_INTEL_AVX2);
#endif

  ctx->keybitlength=keylen*8;

  /* Setup bulk encryption routines.  */
  memset (bulk_ops, 0, sizeof(*bulk_ops));
  bulk_ops->cbc_dec = _gcry_camellia_cbc_dec;
  bulk_ops->cfb_dec = _gcry_camellia_cfb_dec;
  bulk_ops->ctr_enc = _gcry_camellia_ctr_enc;
  bulk_ops->ocb_crypt = _gcry_camellia_ocb_crypt;
  bulk_ops->ocb_auth  = _gcry_camellia_ocb_auth;

  if (0)
    { }
#ifdef USE_AESNI_AVX
  else if (ctx->use_aesni_avx)
    _gcry_camellia_aesni_avx_keygen(ctx, key, keylen);
  else
#endif
    {
      Camellia_Ekeygen(ctx->keybitlength,key,ctx->keytable);
      _gcry_burn_stack
        ((19+34+34)*sizeof(u32)+2*sizeof(void*) /* camellia_setup256 */
         +(4+32)*sizeof(u32)+2*sizeof(void*)    /* camellia_setup192 */
         +0+sizeof(int)+2*sizeof(void*)         /* Camellia_Ekeygen */
         +3*2*sizeof(void*)                     /* Function calls.  */
         );
    }

  return 0;
}

#ifdef USE_ARM_ASM

/* Assembly implementations of Camellia. */
extern void _gcry_camellia_arm_encrypt_block(const KEY_TABLE_TYPE keyTable,
					       byte *outbuf, const byte *inbuf,
					       const int keybits);

extern void _gcry_camellia_arm_decrypt_block(const KEY_TABLE_TYPE keyTable,
					       byte *outbuf, const byte *inbuf,
					       const int keybits);

static void Camellia_EncryptBlock(const int keyBitLength,
				  const unsigned char *plaintext,
				  const KEY_TABLE_TYPE keyTable,
				  unsigned char *cipherText)
{
  _gcry_camellia_arm_encrypt_block(keyTable, cipherText, plaintext,
				     keyBitLength);
}

static void Camellia_DecryptBlock(const int keyBitLength,
				  const unsigned char *cipherText,
				  const KEY_TABLE_TYPE keyTable,
				  unsigned char *plaintext)
{
  _gcry_camellia_arm_decrypt_block(keyTable, plaintext, cipherText,
				     keyBitLength);
}

#ifdef __aarch64__
#  define CAMELLIA_encrypt_stack_burn_size (0)
#  define CAMELLIA_decrypt_stack_burn_size (0)
#else
#  define CAMELLIA_encrypt_stack_burn_size (15*4)
#  define CAMELLIA_decrypt_stack_burn_size (15*4)
#endif

static unsigned int
camellia_encrypt(void *c, byte *outbuf, const byte *inbuf)
{
  CAMELLIA_context *ctx = c;
  Camellia_EncryptBlock(ctx->keybitlength,inbuf,ctx->keytable,outbuf);
  return /*burn_stack*/ (CAMELLIA_encrypt_stack_burn_size);
}

static unsigned int
camellia_decrypt(void *c, byte *outbuf, const byte *inbuf)
{
  CAMELLIA_context *ctx=c;
  Camellia_DecryptBlock(ctx->keybitlength,inbuf,ctx->keytable,outbuf);
  return /*burn_stack*/ (CAMELLIA_decrypt_stack_burn_size);
}

#else /*USE_ARM_ASM*/

static unsigned int
camellia_encrypt(void *c, byte *outbuf, const byte *inbuf)
{
  CAMELLIA_context *ctx=c;

  Camellia_EncryptBlock(ctx->keybitlength,inbuf,ctx->keytable,outbuf);

#define CAMELLIA_encrypt_stack_burn_size \
  (sizeof(int)+2*sizeof(unsigned char *)+sizeof(void*/*KEY_TABLE_TYPE*/) \
     +4*sizeof(u32)+4*sizeof(u32) \
     +2*sizeof(u32*)+4*sizeof(u32) \
     +2*2*sizeof(void*) /* Function calls.  */ \
    )

  return /*burn_stack*/ (CAMELLIA_encrypt_stack_burn_size);
}

static unsigned int
camellia_decrypt(void *c, byte *outbuf, const byte *inbuf)
{
  CAMELLIA_context *ctx=c;

  Camellia_DecryptBlock(ctx->keybitlength,inbuf,ctx->keytable,outbuf);

#define CAMELLIA_decrypt_stack_burn_size \
    (sizeof(int)+2*sizeof(unsigned char *)+sizeof(void*/*KEY_TABLE_TYPE*/) \
     +4*sizeof(u32)+4*sizeof(u32) \
     +2*sizeof(u32*)+4*sizeof(u32) \
     +2*2*sizeof(void*) /* Function calls.  */ \
    )

  return /*burn_stack*/ (CAMELLIA_decrypt_stack_burn_size);
}

#endif /*!USE_ARM_ASM*/

/* Bulk encryption of complete blocks in CTR mode.  This function is only
   intended for the bulk encryption feature of cipher.c.  CTR is expected to be
   of size CAMELLIA_BLOCK_SIZE. */
static void
_gcry_camellia_ctr_enc(void *context, unsigned char *ctr,
                       void *outbuf_arg, const void *inbuf_arg,
                       size_t nblocks)
{
  CAMELLIA_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned char tmpbuf[CAMELLIA_BLOCK_SIZE];
  int burn_stack_depth = CAMELLIA_encrypt_stack_burn_size;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      int did_use_aesni_avx2 = 0;
#ifdef USE_VAES_AVX2
      int use_vaes = ctx->use_vaes_avx2;
#endif

      /* Process data in 32 block chunks. */
      while (nblocks >= 32)
        {
#ifdef USE_VAES_AVX2
          if (use_vaes)
            _gcry_camellia_vaes_avx2_ctr_enc(ctx, outbuf, inbuf, ctr);
          else
#endif
            _gcry_camellia_aesni_avx2_ctr_enc(ctx, outbuf, inbuf, ctr);

          nblocks -= 32;
          outbuf += 32 * CAMELLIA_BLOCK_SIZE;
          inbuf  += 32 * CAMELLIA_BLOCK_SIZE;
          did_use_aesni_avx2 = 1;
        }

      if (did_use_aesni_avx2)
        {
          int avx2_burn_stack_depth = 32 * CAMELLIA_BLOCK_SIZE + 16 +
                                        2 * sizeof(void *) + ASM_EXTRA_STACK;

          if (burn_stack_depth < avx2_burn_stack_depth)
            burn_stack_depth = avx2_burn_stack_depth;
        }

      /* Use generic code to handle smaller chunks... */
      /* TODO: use caching instead? */
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      int did_use_aesni_avx = 0;

      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_camellia_aesni_avx_ctr_enc(ctx, outbuf, inbuf, ctr);

          nblocks -= 16;
          outbuf += 16 * CAMELLIA_BLOCK_SIZE;
          inbuf  += 16 * CAMELLIA_BLOCK_SIZE;
          did_use_aesni_avx = 1;
        }

      if (did_use_aesni_avx)
        {
          int avx_burn_stack_depth = 16 * CAMELLIA_BLOCK_SIZE +
                                       2 * sizeof(void *) + ASM_EXTRA_STACK;

          if (burn_stack_depth < avx_burn_stack_depth)
            burn_stack_depth = avx_burn_stack_depth;
        }

      /* Use generic code to handle smaller chunks... */
      /* TODO: use caching instead? */
    }
#endif

  for ( ;nblocks; nblocks-- )
    {
      /* Encrypt the counter. */
      Camellia_EncryptBlock(ctx->keybitlength, ctr, ctx->keytable, tmpbuf);
      /* XOR the input with the encrypted counter and store in output.  */
      cipher_block_xor(outbuf, tmpbuf, inbuf, CAMELLIA_BLOCK_SIZE);
      outbuf += CAMELLIA_BLOCK_SIZE;
      inbuf  += CAMELLIA_BLOCK_SIZE;
      /* Increment the counter.  */
      cipher_block_add(ctr, 1, CAMELLIA_BLOCK_SIZE);
    }

  wipememory(tmpbuf, sizeof(tmpbuf));
  _gcry_burn_stack(burn_stack_depth);
}

/* Bulk decryption of complete blocks in CBC mode.  This function is only
   intended for the bulk encryption feature of cipher.c. */
static void
_gcry_camellia_cbc_dec(void *context, unsigned char *iv,
                       void *outbuf_arg, const void *inbuf_arg,
                       size_t nblocks)
{
  CAMELLIA_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned char savebuf[CAMELLIA_BLOCK_SIZE];
  int burn_stack_depth = CAMELLIA_decrypt_stack_burn_size;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      int did_use_aesni_avx2 = 0;
#ifdef USE_VAES_AVX2
      int use_vaes = ctx->use_vaes_avx2;
#endif

      /* Process data in 32 block chunks. */
      while (nblocks >= 32)
        {
#ifdef USE_VAES_AVX2
          if (use_vaes)
            _gcry_camellia_vaes_avx2_cbc_dec(ctx, outbuf, inbuf, iv);
          else
#endif
            _gcry_camellia_aesni_avx2_cbc_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 32;
          outbuf += 32 * CAMELLIA_BLOCK_SIZE;
          inbuf  += 32 * CAMELLIA_BLOCK_SIZE;
          did_use_aesni_avx2 = 1;
        }

      if (did_use_aesni_avx2)
        {
          int avx2_burn_stack_depth = 32 * CAMELLIA_BLOCK_SIZE + 16 +
                                        2 * sizeof(void *) + ASM_EXTRA_STACK;;

          if (burn_stack_depth < avx2_burn_stack_depth)
            burn_stack_depth = avx2_burn_stack_depth;
        }

      /* Use generic code to handle smaller chunks... */
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      int did_use_aesni_avx = 0;

      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_camellia_aesni_avx_cbc_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 16;
          outbuf += 16 * CAMELLIA_BLOCK_SIZE;
          inbuf  += 16 * CAMELLIA_BLOCK_SIZE;
          did_use_aesni_avx = 1;
        }

      if (did_use_aesni_avx)
        {
          int avx_burn_stack_depth = 16 * CAMELLIA_BLOCK_SIZE +
                                       2 * sizeof(void *) + ASM_EXTRA_STACK;

          if (burn_stack_depth < avx_burn_stack_depth)
            burn_stack_depth = avx_burn_stack_depth;
        }

      /* Use generic code to handle smaller chunks... */
    }
#endif

  for ( ;nblocks; nblocks-- )
    {
      /* INBUF is needed later and it may be identical to OUTBUF, so store
         the intermediate result to SAVEBUF.  */
      Camellia_DecryptBlock(ctx->keybitlength, inbuf, ctx->keytable, savebuf);

      cipher_block_xor_n_copy_2(outbuf, savebuf, iv, inbuf,
                                CAMELLIA_BLOCK_SIZE);
      inbuf += CAMELLIA_BLOCK_SIZE;
      outbuf += CAMELLIA_BLOCK_SIZE;
    }

  wipememory(savebuf, sizeof(savebuf));
  _gcry_burn_stack(burn_stack_depth);
}

/* Bulk decryption of complete blocks in CFB mode.  This function is only
   intended for the bulk encryption feature of cipher.c. */
static void
_gcry_camellia_cfb_dec(void *context, unsigned char *iv,
                       void *outbuf_arg, const void *inbuf_arg,
                       size_t nblocks)
{
  CAMELLIA_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  int burn_stack_depth = CAMELLIA_decrypt_stack_burn_size;

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      int did_use_aesni_avx2 = 0;
#ifdef USE_VAES_AVX2
      int use_vaes = ctx->use_vaes_avx2;
#endif

      /* Process data in 32 block chunks. */
      while (nblocks >= 32)
        {
#ifdef USE_VAES_AVX2
          if (use_vaes)
            _gcry_camellia_vaes_avx2_cfb_dec(ctx, outbuf, inbuf, iv);
          else
#endif
            _gcry_camellia_aesni_avx2_cfb_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 32;
          outbuf += 32 * CAMELLIA_BLOCK_SIZE;
          inbuf  += 32 * CAMELLIA_BLOCK_SIZE;
          did_use_aesni_avx2 = 1;
        }

      if (did_use_aesni_avx2)
        {
          int avx2_burn_stack_depth = 32 * CAMELLIA_BLOCK_SIZE + 16 +
                                        2 * sizeof(void *) + ASM_EXTRA_STACK;

          if (burn_stack_depth < avx2_burn_stack_depth)
            burn_stack_depth = avx2_burn_stack_depth;
        }

      /* Use generic code to handle smaller chunks... */
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      int did_use_aesni_avx = 0;

      /* Process data in 16 block chunks. */
      while (nblocks >= 16)
        {
          _gcry_camellia_aesni_avx_cfb_dec(ctx, outbuf, inbuf, iv);

          nblocks -= 16;
          outbuf += 16 * CAMELLIA_BLOCK_SIZE;
          inbuf  += 16 * CAMELLIA_BLOCK_SIZE;
          did_use_aesni_avx = 1;
        }

      if (did_use_aesni_avx)
        {
          int avx_burn_stack_depth = 16 * CAMELLIA_BLOCK_SIZE +
                                       2 * sizeof(void *) + ASM_EXTRA_STACK;

          if (burn_stack_depth < avx_burn_stack_depth)
            burn_stack_depth = avx_burn_stack_depth;
        }

      /* Use generic code to handle smaller chunks... */
    }
#endif

  for ( ;nblocks; nblocks-- )
    {
      Camellia_EncryptBlock(ctx->keybitlength, iv, ctx->keytable, iv);
      cipher_block_xor_n_copy(outbuf, iv, inbuf, CAMELLIA_BLOCK_SIZE);
      outbuf += CAMELLIA_BLOCK_SIZE;
      inbuf  += CAMELLIA_BLOCK_SIZE;
    }

  _gcry_burn_stack(burn_stack_depth);
}

/* Bulk encryption/decryption of complete blocks in OCB mode. */
static size_t
_gcry_camellia_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
			  const void *inbuf_arg, size_t nblocks, int encrypt)
{
#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2)
  CAMELLIA_context *ctx = (void *)&c->context.c;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  int burn_stack_depth;
  u64 blkn = c->u_mode.ocb.data_nblocks;

  burn_stack_depth = encrypt ? CAMELLIA_encrypt_stack_burn_size :
			      CAMELLIA_decrypt_stack_burn_size;
#else
  (void)c;
  (void)outbuf_arg;
  (void)inbuf_arg;
  (void)encrypt;
#endif

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      int did_use_aesni_avx2 = 0;
#ifdef USE_VAES_AVX2
      int encrypt_use_vaes = encrypt && ctx->use_vaes_avx2;
      int decrypt_use_vaes = !encrypt && ctx->use_vaes_avx2;
#endif
      u64 Ls[32];
      unsigned int n = 32 - (blkn % 32);
      u64 *l;
      int i;

      if (nblocks >= 32)
	{
	  for (i = 0; i < 32; i += 8)
	    {
	      /* Use u64 to store pointers for x32 support (assembly function
	       * assumes 64-bit pointers). */
	      Ls[(i + 0 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 1 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 2 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 3 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[2];
	      Ls[(i + 4 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 5 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 6 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	    }

	  Ls[(7 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  Ls[(15 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[4];
	  Ls[(23 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  l = &Ls[(31 + n) % 32];

	  /* Process data in 32 block chunks. */
	  while (nblocks >= 32)
	    {
	      blkn += 32;
	      *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 32);

	      if (0) {}
#ifdef USE_VAES_AVX2
	      else if (encrypt_use_vaes)
		_gcry_camellia_vaes_avx2_ocb_enc(ctx, outbuf, inbuf, c->u_iv.iv,
                                                 c->u_ctr.ctr, Ls);
	      else if (decrypt_use_vaes)
		_gcry_camellia_vaes_avx2_ocb_dec(ctx, outbuf, inbuf, c->u_iv.iv,
                                                 c->u_ctr.ctr, Ls);
#endif
	      else if (encrypt)
		_gcry_camellia_aesni_avx2_ocb_enc(ctx, outbuf, inbuf, c->u_iv.iv,
						  c->u_ctr.ctr, Ls);
	      else
		_gcry_camellia_aesni_avx2_ocb_dec(ctx, outbuf, inbuf, c->u_iv.iv,
						  c->u_ctr.ctr, Ls);

	      nblocks -= 32;
	      outbuf += 32 * CAMELLIA_BLOCK_SIZE;
	      inbuf  += 32 * CAMELLIA_BLOCK_SIZE;
	      did_use_aesni_avx2 = 1;
	    }
	}

      if (did_use_aesni_avx2)
	{
	  int avx2_burn_stack_depth = 32 * CAMELLIA_BLOCK_SIZE +
				      2 * sizeof(void *) + ASM_EXTRA_STACK;

	  if (burn_stack_depth < avx2_burn_stack_depth)
	    burn_stack_depth = avx2_burn_stack_depth;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      int did_use_aesni_avx = 0;
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
		_gcry_camellia_aesni_avx_ocb_enc(ctx, outbuf, inbuf, c->u_iv.iv,
						c->u_ctr.ctr, Ls);
	      else
		_gcry_camellia_aesni_avx_ocb_dec(ctx, outbuf, inbuf, c->u_iv.iv,
						c->u_ctr.ctr, Ls);

	      nblocks -= 16;
	      outbuf += 16 * CAMELLIA_BLOCK_SIZE;
	      inbuf  += 16 * CAMELLIA_BLOCK_SIZE;
	      did_use_aesni_avx = 1;
	    }
	}

      if (did_use_aesni_avx)
	{
	  int avx_burn_stack_depth = 16 * CAMELLIA_BLOCK_SIZE +
				      2 * sizeof(void *) + ASM_EXTRA_STACK;

	  if (burn_stack_depth < avx_burn_stack_depth)
	    burn_stack_depth = avx_burn_stack_depth;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2)
  c->u_mode.ocb.data_nblocks = blkn;

  if (burn_stack_depth)
    _gcry_burn_stack (burn_stack_depth + 4 * sizeof(void *));
#endif

  return nblocks;
}

/* Bulk authentication of complete blocks in OCB mode. */
static size_t
_gcry_camellia_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
			 size_t nblocks)
{
#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2)
  CAMELLIA_context *ctx = (void *)&c->context.c;
  const unsigned char *abuf = abuf_arg;
  int burn_stack_depth;
  u64 blkn = c->u_mode.ocb.aad_nblocks;

  burn_stack_depth = CAMELLIA_encrypt_stack_burn_size;
#else
  (void)c;
  (void)abuf_arg;
#endif

#ifdef USE_AESNI_AVX2
  if (ctx->use_aesni_avx2)
    {
      int did_use_aesni_avx2 = 0;
#ifdef USE_VAES_AVX2
      int use_vaes = ctx->use_vaes_avx2;
#endif
      u64 Ls[32];
      unsigned int n = 32 - (blkn % 32);
      u64 *l;
      int i;

      if (nblocks >= 32)
	{
	  for (i = 0; i < 32; i += 8)
	    {
	      /* Use u64 to store pointers for x32 support (assembly function
	       * assumes 64-bit pointers). */
	      Ls[(i + 0 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 1 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 2 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 3 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[2];
	      Ls[(i + 4 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	      Ls[(i + 5 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[1];
	      Ls[(i + 6 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[0];
	    }

	  Ls[(7 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  Ls[(15 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[4];
	  Ls[(23 + n) % 32] = (uintptr_t)(void *)c->u_mode.ocb.L[3];
	  l = &Ls[(31 + n) % 32];

	  /* Process data in 32 block chunks. */
	  while (nblocks >= 32)
	    {
	      blkn += 32;
	      *l = (uintptr_t)(void *)ocb_get_l(c, blkn - blkn % 32);

#ifdef USE_VAES_AVX2
              if (use_vaes)
                _gcry_camellia_vaes_avx2_ocb_auth(ctx, abuf,
                                                  c->u_mode.ocb.aad_offset,
                                                  c->u_mode.ocb.aad_sum, Ls);
              else
#endif
                _gcry_camellia_aesni_avx2_ocb_auth(ctx, abuf,
                                                   c->u_mode.ocb.aad_offset,
                                                   c->u_mode.ocb.aad_sum, Ls);

	      nblocks -= 32;
	      abuf += 32 * CAMELLIA_BLOCK_SIZE;
	      did_use_aesni_avx2 = 1;
	    }
	}

      if (did_use_aesni_avx2)
	{
	  int avx2_burn_stack_depth = 32 * CAMELLIA_BLOCK_SIZE +
				      2 * sizeof(void *) + ASM_EXTRA_STACK;

	  if (burn_stack_depth < avx2_burn_stack_depth)
	    burn_stack_depth = avx2_burn_stack_depth;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#ifdef USE_AESNI_AVX
  if (ctx->use_aesni_avx)
    {
      int did_use_aesni_avx = 0;
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

	      _gcry_camellia_aesni_avx_ocb_auth(ctx, abuf,
						c->u_mode.ocb.aad_offset,
						c->u_mode.ocb.aad_sum, Ls);

	      nblocks -= 16;
	      abuf += 16 * CAMELLIA_BLOCK_SIZE;
	      did_use_aesni_avx = 1;
	    }
	}

      if (did_use_aesni_avx)
	{
	  int avx_burn_stack_depth = 16 * CAMELLIA_BLOCK_SIZE +
				      2 * sizeof(void *) + ASM_EXTRA_STACK;

	  if (burn_stack_depth < avx_burn_stack_depth)
	    burn_stack_depth = avx_burn_stack_depth;
	}

      /* Use generic code to handle smaller chunks... */
    }
#endif

#if defined(USE_AESNI_AVX) || defined(USE_AESNI_AVX2)
  c->u_mode.ocb.aad_nblocks = blkn;

  if (burn_stack_depth)
    _gcry_burn_stack (burn_stack_depth + 4 * sizeof(void *));
#endif

  return nblocks;
}

/* Run the self-tests for CAMELLIA-CTR-128, tests IV increment of bulk CTR
   encryption.  Returns NULL on success. */
static const char*
selftest_ctr_128 (void)
{
  const int nblocks = 32+16+1;
  const int blocksize = CAMELLIA_BLOCK_SIZE;
  const int context_size = sizeof(CAMELLIA_context);

  return _gcry_selftest_helper_ctr("CAMELLIA", &camellia_setkey,
           &camellia_encrypt, nblocks, blocksize, context_size);
}

/* Run the self-tests for CAMELLIA-CBC-128, tests bulk CBC decryption.
   Returns NULL on success. */
static const char*
selftest_cbc_128 (void)
{
  const int nblocks = 32+16+2;
  const int blocksize = CAMELLIA_BLOCK_SIZE;
  const int context_size = sizeof(CAMELLIA_context);

  return _gcry_selftest_helper_cbc("CAMELLIA", &camellia_setkey,
           &camellia_encrypt, nblocks, blocksize, context_size);
}

/* Run the self-tests for CAMELLIA-CFB-128, tests bulk CFB decryption.
   Returns NULL on success. */
static const char*
selftest_cfb_128 (void)
{
  const int nblocks = 32+16+2;
  const int blocksize = CAMELLIA_BLOCK_SIZE;
  const int context_size = sizeof(CAMELLIA_context);

  return _gcry_selftest_helper_cfb("CAMELLIA", &camellia_setkey,
           &camellia_encrypt, nblocks, blocksize, context_size);
}

static const char *
selftest(void)
{
  CAMELLIA_context ctx;
  byte scratch[16];
  cipher_bulk_ops_t bulk_ops;
  const char *r;

  /* These test vectors are from RFC-3713 */
  static const byte plaintext[]=
    {
      0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
      0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10
    };
  static const byte key_128[]=
    {
      0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
      0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10
    };
  static const byte ciphertext_128[]=
    {
      0x67,0x67,0x31,0x38,0x54,0x96,0x69,0x73,
      0x08,0x57,0x06,0x56,0x48,0xea,0xbe,0x43
    };
  static const byte key_192[]=
    {
      0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,
      0x76,0x54,0x32,0x10,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77
    };
  static const byte ciphertext_192[]=
    {
      0xb4,0x99,0x34,0x01,0xb3,0xe9,0x96,0xf8,
      0x4e,0xe5,0xce,0xe7,0xd7,0x9b,0x09,0xb9
    };
  static const byte key_256[]=
    {
      0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,
      0x98,0x76,0x54,0x32,0x10,0x00,0x11,0x22,0x33,0x44,0x55,
      0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };
  static const byte ciphertext_256[]=
    {
      0x9a,0xcc,0x23,0x7d,0xff,0x16,0xd7,0x6c,
      0x20,0xef,0x7c,0x91,0x9e,0x3a,0x75,0x09
    };

  camellia_setkey(&ctx,key_128,sizeof(key_128),&bulk_ops);
  camellia_encrypt(&ctx,scratch,plaintext);
  if(memcmp(scratch,ciphertext_128,sizeof(ciphertext_128))!=0)
    return "CAMELLIA-128 test encryption failed.";
  camellia_decrypt(&ctx,scratch,scratch);
  if(memcmp(scratch,plaintext,sizeof(plaintext))!=0)
    return "CAMELLIA-128 test decryption failed.";

  camellia_setkey(&ctx,key_192,sizeof(key_192),&bulk_ops);
  camellia_encrypt(&ctx,scratch,plaintext);
  if(memcmp(scratch,ciphertext_192,sizeof(ciphertext_192))!=0)
    return "CAMELLIA-192 test encryption failed.";
  camellia_decrypt(&ctx,scratch,scratch);
  if(memcmp(scratch,plaintext,sizeof(plaintext))!=0)
    return "CAMELLIA-192 test decryption failed.";

  camellia_setkey(&ctx,key_256,sizeof(key_256),&bulk_ops);
  camellia_encrypt(&ctx,scratch,plaintext);
  if(memcmp(scratch,ciphertext_256,sizeof(ciphertext_256))!=0)
    return "CAMELLIA-256 test encryption failed.";
  camellia_decrypt(&ctx,scratch,scratch);
  if(memcmp(scratch,plaintext,sizeof(plaintext))!=0)
    return "CAMELLIA-256 test decryption failed.";

  if ( (r = selftest_ctr_128 ()) )
    return r;

  if ( (r = selftest_cbc_128 ()) )
    return r;

  if ( (r = selftest_cfb_128 ()) )
    return r;

  return NULL;
}

/* These oids are from
   <http://info.isl.ntt.co.jp/crypt/eng/camellia/specifications_oid.html>,
   retrieved May 1, 2007. */

static const gcry_cipher_oid_spec_t camellia128_oids[] =
  {
    {"1.2.392.200011.61.1.1.1.2", GCRY_CIPHER_MODE_CBC},
    {"0.3.4401.5.3.1.9.1", GCRY_CIPHER_MODE_ECB},
    {"0.3.4401.5.3.1.9.3", GCRY_CIPHER_MODE_OFB},
    {"0.3.4401.5.3.1.9.4", GCRY_CIPHER_MODE_CFB},
    { NULL }
  };

static const gcry_cipher_oid_spec_t camellia192_oids[] =
  {
    {"1.2.392.200011.61.1.1.1.3", GCRY_CIPHER_MODE_CBC},
    {"0.3.4401.5.3.1.9.21", GCRY_CIPHER_MODE_ECB},
    {"0.3.4401.5.3.1.9.23", GCRY_CIPHER_MODE_OFB},
    {"0.3.4401.5.3.1.9.24", GCRY_CIPHER_MODE_CFB},
    { NULL }
  };

static const gcry_cipher_oid_spec_t camellia256_oids[] =
  {
    {"1.2.392.200011.61.1.1.1.4", GCRY_CIPHER_MODE_CBC},
    {"0.3.4401.5.3.1.9.41", GCRY_CIPHER_MODE_ECB},
    {"0.3.4401.5.3.1.9.43", GCRY_CIPHER_MODE_OFB},
    {"0.3.4401.5.3.1.9.44", GCRY_CIPHER_MODE_CFB},
    { NULL }
  };

gcry_cipher_spec_t _gcry_cipher_spec_camellia128 =
  {
    GCRY_CIPHER_CAMELLIA128, {0, 0},
    "CAMELLIA128",NULL,camellia128_oids,CAMELLIA_BLOCK_SIZE,128,
    sizeof(CAMELLIA_context),camellia_setkey,camellia_encrypt,camellia_decrypt
  };

gcry_cipher_spec_t _gcry_cipher_spec_camellia192 =
  {
    GCRY_CIPHER_CAMELLIA192, {0, 0},
    "CAMELLIA192",NULL,camellia192_oids,CAMELLIA_BLOCK_SIZE,192,
    sizeof(CAMELLIA_context),camellia_setkey,camellia_encrypt,camellia_decrypt
  };

gcry_cipher_spec_t _gcry_cipher_spec_camellia256 =
  {
    GCRY_CIPHER_CAMELLIA256, {0, 0},
    "CAMELLIA256",NULL,camellia256_oids,CAMELLIA_BLOCK_SIZE,256,
    sizeof(CAMELLIA_context),camellia_setkey,camellia_encrypt,camellia_decrypt
  };
