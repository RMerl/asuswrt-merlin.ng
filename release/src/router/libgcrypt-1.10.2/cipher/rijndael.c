/* Rijndael (AES) for GnuPG
 * Copyright (C) 2000, 2001, 2002, 2003, 2007,
 *               2008, 2011, 2012 Free Software Foundation, Inc.
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
 *******************************************************************
 * The code here is based on the optimized implementation taken from
 * http://www.esat.kuleuven.ac.be/~rijmen/rijndael/ on Oct 2, 2000,
 * which carries this notice:
 *------------------------------------------
 * rijndael-alg-fst.c   v2.3   April '2000
 *
 * Optimised ANSI C code
 *
 * authors: v1.0: Antoon Bosselaers
 *          v2.0: Vincent Rijmen
 *          v2.3: Paulo Barreto
 *
 * This code is placed in the public domain.
 *------------------------------------------
 *
 * The SP800-38a document is available at:
 *   http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memcmp() */

#include "types.h"  /* for byte and u32 typedefs */
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "cipher-selftest.h"
#include "rijndael-internal.h"
#include "./cipher-internal.h"


#ifdef USE_AMD64_ASM
/* AMD64 assembly implementations of AES */
extern unsigned int _gcry_aes_amd64_encrypt_block(const void *keysched_enc,
                                                  unsigned char *out,
                                                  const unsigned char *in,
                                                  int rounds,
                                                  const void *encT);

extern unsigned int _gcry_aes_amd64_decrypt_block(const void *keysched_dec,
                                                  unsigned char *out,
                                                  const unsigned char *in,
                                                  int rounds,
                                                  const void *decT);
#endif /*USE_AMD64_ASM*/

#ifdef USE_AESNI
/* AES-NI (AMD64 & i386) accelerated implementations of AES */
extern void _gcry_aes_aesni_do_setkey(RIJNDAEL_context *ctx, const byte *key);
extern void _gcry_aes_aesni_prepare_decryption(RIJNDAEL_context *ctx);

extern unsigned int _gcry_aes_aesni_encrypt (const RIJNDAEL_context *ctx,
                                             unsigned char *dst,
                                             const unsigned char *src);
extern unsigned int _gcry_aes_aesni_decrypt (const RIJNDAEL_context *ctx,
                                             unsigned char *dst,
                                             const unsigned char *src);
extern void _gcry_aes_aesni_cfb_enc (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern void _gcry_aes_aesni_cbc_enc (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks, int cbc_mac);
extern void _gcry_aes_aesni_ctr_enc (void *context, unsigned char *ctr,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern void _gcry_aes_aesni_ctr32le_enc (void *context, unsigned char *ctr,
					 void *outbuf_arg,
					 const void *inbuf_arg, size_t nblocks);
extern void _gcry_aes_aesni_cfb_dec (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern void _gcry_aes_aesni_cbc_dec (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern size_t _gcry_aes_aesni_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
                                         const void *inbuf_arg, size_t nblocks,
                                         int encrypt);
extern size_t _gcry_aes_aesni_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
                                        size_t nblocks);
extern void _gcry_aes_aesni_xts_crypt (void *context, unsigned char *tweak,
                                       void *outbuf_arg, const void *inbuf_arg,
                                       size_t nblocks, int encrypt);
#endif

#ifdef USE_VAES
/* VAES (AMD64) accelerated implementation of AES */

extern void _gcry_aes_vaes_cfb_dec (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
extern void _gcry_aes_vaes_cbc_dec (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
extern void _gcry_aes_vaes_ctr_enc (void *context, unsigned char *ctr,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
extern void _gcry_aes_vaes_ctr32le_enc (void *context, unsigned char *ctr,
					void *outbuf_arg, const void *inbuf_arg,
					size_t nblocks);
extern size_t _gcry_aes_vaes_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
					const void *inbuf_arg, size_t nblocks,
					int encrypt);
extern void _gcry_aes_vaes_xts_crypt (void *context, unsigned char *tweak,
				      void *outbuf_arg, const void *inbuf_arg,
				      size_t nblocks, int encrypt);
#endif

#ifdef USE_SSSE3
/* SSSE3 (AMD64) vector permutation implementation of AES */
extern void _gcry_aes_ssse3_do_setkey(RIJNDAEL_context *ctx, const byte *key);
extern void _gcry_aes_ssse3_prepare_decryption(RIJNDAEL_context *ctx);

extern unsigned int _gcry_aes_ssse3_encrypt (const RIJNDAEL_context *ctx,
                                             unsigned char *dst,
                                             const unsigned char *src);
extern unsigned int _gcry_aes_ssse3_decrypt (const RIJNDAEL_context *ctx,
                                             unsigned char *dst,
                                             const unsigned char *src);
extern void _gcry_aes_ssse3_cfb_enc (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern void _gcry_aes_ssse3_cbc_enc (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks,
                                     int cbc_mac);
extern void _gcry_aes_ssse3_ctr_enc (void *context, unsigned char *ctr,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern void _gcry_aes_ssse3_cfb_dec (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern void _gcry_aes_ssse3_cbc_dec (void *context, unsigned char *iv,
                                     void *outbuf_arg, const void *inbuf_arg,
                                     size_t nblocks);
extern size_t _gcry_aes_ssse3_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
                                         const void *inbuf_arg, size_t nblocks,
                                         int encrypt);
extern size_t _gcry_aes_ssse3_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
                                        size_t nblocks);
#endif

#ifdef USE_PADLOCK
extern unsigned int _gcry_aes_padlock_encrypt (const RIJNDAEL_context *ctx,
                                               unsigned char *bx,
                                               const unsigned char *ax);
extern unsigned int _gcry_aes_padlock_decrypt (const RIJNDAEL_context *ctx,
                                               unsigned char *bx,
                                               const unsigned char *ax);
extern void _gcry_aes_padlock_prepare_decryption (RIJNDAEL_context *ctx);
#endif

#ifdef USE_ARM_ASM
/* ARM assembly implementations of AES */
extern unsigned int _gcry_aes_arm_encrypt_block(const void *keysched_enc,
                                                unsigned char *out,
                                                const unsigned char *in,
                                                int rounds,
                                                const void *encT);

extern unsigned int _gcry_aes_arm_decrypt_block(const void *keysched_dec,
                                                unsigned char *out,
                                                const unsigned char *in,
                                                int rounds,
                                                const void *decT);
#endif /*USE_ARM_ASM*/

#ifdef USE_ARM_CE
/* ARMv8 Crypto Extension implementations of AES */
extern void _gcry_aes_armv8_ce_setkey(RIJNDAEL_context *ctx, const byte *key);
extern void _gcry_aes_armv8_ce_prepare_decryption(RIJNDAEL_context *ctx);

extern unsigned int _gcry_aes_armv8_ce_encrypt(const RIJNDAEL_context *ctx,
                                               unsigned char *dst,
                                               const unsigned char *src);
extern unsigned int _gcry_aes_armv8_ce_decrypt(const RIJNDAEL_context *ctx,
                                               unsigned char *dst,
                                               const unsigned char *src);

extern void _gcry_aes_armv8_ce_cfb_enc (void *context, unsigned char *iv,
                                        void *outbuf_arg, const void *inbuf_arg,
                                        size_t nblocks);
extern void _gcry_aes_armv8_ce_cbc_enc (void *context, unsigned char *iv,
                                        void *outbuf_arg, const void *inbuf_arg,
                                        size_t nblocks,
                                        int cbc_mac);
extern void _gcry_aes_armv8_ce_ctr_enc (void *context, unsigned char *ctr,
                                        void *outbuf_arg, const void *inbuf_arg,
                                        size_t nblocks);
extern void _gcry_aes_armv8_ce_ctr32le_enc (void *context, unsigned char *ctr,
                                            void *outbuf_arg,
                                            const void *inbuf_arg,
                                            size_t nblocks);
extern void _gcry_aes_armv8_ce_cfb_dec (void *context, unsigned char *iv,
                                        void *outbuf_arg, const void *inbuf_arg,
                                        size_t nblocks);
extern void _gcry_aes_armv8_ce_cbc_dec (void *context, unsigned char *iv,
                                        void *outbuf_arg, const void *inbuf_arg,
                                        size_t nblocks);
extern size_t _gcry_aes_armv8_ce_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
                                            const void *inbuf_arg, size_t nblocks,
                                            int encrypt);
extern size_t _gcry_aes_armv8_ce_ocb_auth (gcry_cipher_hd_t c,
                                           const void *abuf_arg, size_t nblocks);
extern void _gcry_aes_armv8_ce_xts_crypt (void *context, unsigned char *tweak,
                                          void *outbuf_arg,
                                          const void *inbuf_arg,
                                          size_t nblocks, int encrypt);
#endif /*USE_ARM_ASM*/

#ifdef USE_PPC_CRYPTO
/* PowerPC Crypto implementations of AES */
extern void _gcry_aes_ppc8_setkey(RIJNDAEL_context *ctx, const byte *key);
extern void _gcry_aes_ppc8_prepare_decryption(RIJNDAEL_context *ctx);

extern unsigned int _gcry_aes_ppc8_encrypt(const RIJNDAEL_context *ctx,
					   unsigned char *dst,
					   const unsigned char *src);
extern unsigned int _gcry_aes_ppc8_decrypt(const RIJNDAEL_context *ctx,
					   unsigned char *dst,
					   const unsigned char *src);

extern void _gcry_aes_ppc8_cfb_enc (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
extern void _gcry_aes_ppc8_cbc_enc (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks, int cbc_mac);
extern void _gcry_aes_ppc8_ctr_enc (void *context, unsigned char *ctr,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
extern void _gcry_aes_ppc8_cfb_dec (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);
extern void _gcry_aes_ppc8_cbc_dec (void *context, unsigned char *iv,
				    void *outbuf_arg, const void *inbuf_arg,
				    size_t nblocks);

extern size_t _gcry_aes_ppc8_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
					const void *inbuf_arg, size_t nblocks,
					int encrypt);
extern size_t _gcry_aes_ppc8_ocb_auth (gcry_cipher_hd_t c,
				       const void *abuf_arg, size_t nblocks);

extern void _gcry_aes_ppc8_xts_crypt (void *context, unsigned char *tweak,
				      void *outbuf_arg,
				      const void *inbuf_arg,
				      size_t nblocks, int encrypt);
#endif /*USE_PPC_CRYPTO*/

#ifdef USE_PPC_CRYPTO_WITH_PPC9LE
/* Power9 little-endian crypto implementations of AES */
extern unsigned int _gcry_aes_ppc9le_encrypt(const RIJNDAEL_context *ctx,
					    unsigned char *dst,
					    const unsigned char *src);
extern unsigned int _gcry_aes_ppc9le_decrypt(const RIJNDAEL_context *ctx,
					    unsigned char *dst,
					    const unsigned char *src);

extern void _gcry_aes_ppc9le_cfb_enc (void *context, unsigned char *iv,
				      void *outbuf_arg, const void *inbuf_arg,
				      size_t nblocks);
extern void _gcry_aes_ppc9le_cbc_enc (void *context, unsigned char *iv,
				      void *outbuf_arg, const void *inbuf_arg,
				      size_t nblocks, int cbc_mac);
extern void _gcry_aes_ppc9le_ctr_enc (void *context, unsigned char *ctr,
				      void *outbuf_arg, const void *inbuf_arg,
				      size_t nblocks);
extern void _gcry_aes_ppc9le_cfb_dec (void *context, unsigned char *iv,
				      void *outbuf_arg, const void *inbuf_arg,
				      size_t nblocks);
extern void _gcry_aes_ppc9le_cbc_dec (void *context, unsigned char *iv,
				      void *outbuf_arg, const void *inbuf_arg,
				      size_t nblocks);

extern size_t _gcry_aes_ppc9le_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
					  const void *inbuf_arg, size_t nblocks,
					  int encrypt);
extern size_t _gcry_aes_ppc9le_ocb_auth (gcry_cipher_hd_t c,
					const void *abuf_arg, size_t nblocks);

extern void _gcry_aes_ppc9le_xts_crypt (void *context, unsigned char *tweak,
					void *outbuf_arg,
					const void *inbuf_arg,
					size_t nblocks, int encrypt);

extern size_t _gcry_aes_p10le_gcm_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
					 const void *inbuf_arg,
					 size_t nblocks, int encrypt);
#endif /*USE_PPC_CRYPTO_WITH_PPC9LE*/

#ifdef USE_S390X_CRYPTO
/* zSeries crypto implementations of AES */
extern int _gcry_aes_s390x_setup_acceleration(RIJNDAEL_context *ctx,
					      unsigned int keylen,
					      unsigned int hwfeatures,
					      cipher_bulk_ops_t *bulk_ops);
extern void _gcry_aes_s390x_setkey(RIJNDAEL_context *ctx, const byte *key);
extern void _gcry_aes_s390x_prepare_decryption(RIJNDAEL_context *ctx);

extern unsigned int _gcry_aes_s390x_encrypt(const RIJNDAEL_context *ctx,
					    unsigned char *dst,
					    const unsigned char *src);
extern unsigned int _gcry_aes_s390x_decrypt(const RIJNDAEL_context *ctx,
					    unsigned char *dst,
					    const unsigned char *src);

#endif /*USE_S390X_CRYPTO*/

static unsigned int do_encrypt (const RIJNDAEL_context *ctx, unsigned char *bx,
                                const unsigned char *ax);
static unsigned int do_decrypt (const RIJNDAEL_context *ctx, unsigned char *bx,
                                const unsigned char *ax);

static void _gcry_aes_cfb_enc (void *context, unsigned char *iv,
			       void *outbuf, const void *inbuf,
			       size_t nblocks);
static void _gcry_aes_cfb_dec (void *context, unsigned char *iv,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks);
static void _gcry_aes_cbc_enc (void *context, unsigned char *iv,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks, int cbc_mac);
static void _gcry_aes_cbc_dec (void *context, unsigned char *iv,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks);
static void _gcry_aes_ctr_enc (void *context, unsigned char *ctr,
			       void *outbuf_arg, const void *inbuf_arg,
			       size_t nblocks);
static size_t _gcry_aes_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
				   const void *inbuf_arg, size_t nblocks,
				   int encrypt);
static size_t _gcry_aes_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg,
				  size_t nblocks);
static void _gcry_aes_xts_crypt (void *context, unsigned char *tweak,
				 void *outbuf_arg, const void *inbuf_arg,
				 size_t nblocks, int encrypt);


/* All the numbers.  */
#include "rijndael-tables.h"




/* Function prototypes.  */
static const char *selftest(void);
static void prepare_decryption(RIJNDAEL_context *ctx);



/* Prefetching for encryption/decryption tables. */
static inline void prefetch_table(const volatile byte *tab, size_t len)
{
  size_t i;

  for (i = 0; len - i >= 8 * 32; i += 8 * 32)
    {
      (void)tab[i + 0 * 32];
      (void)tab[i + 1 * 32];
      (void)tab[i + 2 * 32];
      (void)tab[i + 3 * 32];
      (void)tab[i + 4 * 32];
      (void)tab[i + 5 * 32];
      (void)tab[i + 6 * 32];
      (void)tab[i + 7 * 32];
    }
  for (; i < len; i += 32)
    {
      (void)tab[i];
    }

  (void)tab[len - 1];
}

static void prefetch_enc(void)
{
  /* Modify counters to trigger copy-on-write and unsharing if physical pages
   * of look-up table are shared between processes.  Modifying counters also
   * causes checksums for pages to change and hint same-page merging algorithm
   * that these pages are frequently changing.  */
  enc_tables.counter_head++;
  enc_tables.counter_tail++;

  /* Prefetch look-up tables to cache.  */
  prefetch_table((const void *)&enc_tables, sizeof(enc_tables));
}

static void prefetch_dec(void)
{
  /* Modify counters to trigger copy-on-write and unsharing if physical pages
   * of look-up table are shared between processes.  Modifying counters also
   * causes checksums for pages to change and hint same-page merging algorithm
   * that these pages are frequently changing.  */
  dec_tables.counter_head++;
  dec_tables.counter_tail++;

  /* Prefetch look-up tables to cache.  */
  prefetch_table((const void *)&dec_tables, sizeof(dec_tables));
}



/* Perform the key setup.  */
static gcry_err_code_t
do_setkey (RIJNDAEL_context *ctx, const byte *key, const unsigned keylen,
           cipher_bulk_ops_t *bulk_ops)
{
  static int initialized = 0;
  static const char *selftest_failed = 0;
  void (*hw_setkey)(RIJNDAEL_context *ctx, const byte *key) = NULL;
  int rounds;
  int i,j, r, t, rconpointer = 0;
  int KC;
  unsigned int hwfeatures;

  /* The on-the-fly self tests are only run in non-fips mode. In fips
     mode explicit self-tests are required.  Actually the on-the-fly
     self-tests are not fully thread-safe and it might happen that a
     failed self-test won't get noticed in another thread.

     FIXME: We might want to have a central registry of succeeded
     self-tests. */
  if (!fips_mode () && !initialized)
    {
      initialized = 1;
      selftest_failed = selftest ();
      if (selftest_failed)
        log_error ("%s\n", selftest_failed );
    }
  if (selftest_failed)
    return GPG_ERR_SELFTEST_FAILED;

  if( keylen == 128/8 )
    {
      rounds = 10;
      KC = 4;
    }
  else if ( keylen == 192/8 )
    {
      rounds = 12;
      KC = 6;
    }
  else if ( keylen == 256/8 )
    {
      rounds = 14;
      KC = 8;
    }
  else
    return GPG_ERR_INV_KEYLEN;

  ctx->rounds = rounds;
  hwfeatures = _gcry_get_hw_features ();

  ctx->decryption_prepared = 0;

  /* Setup default bulk encryption routines.  */
  memset (bulk_ops, 0, sizeof(*bulk_ops));
  bulk_ops->cfb_enc = _gcry_aes_cfb_enc;
  bulk_ops->cfb_dec = _gcry_aes_cfb_dec;
  bulk_ops->cbc_enc = _gcry_aes_cbc_enc;
  bulk_ops->cbc_dec = _gcry_aes_cbc_dec;
  bulk_ops->ctr_enc = _gcry_aes_ctr_enc;
  bulk_ops->ocb_crypt = _gcry_aes_ocb_crypt;
  bulk_ops->ocb_auth  = _gcry_aes_ocb_auth;
  bulk_ops->xts_crypt = _gcry_aes_xts_crypt;

  (void)hwfeatures;

  if (0)
    {
      ;
    }
#ifdef USE_AESNI
  else if (hwfeatures & HWF_INTEL_AESNI)
    {
      hw_setkey = _gcry_aes_aesni_do_setkey;
      ctx->encrypt_fn = _gcry_aes_aesni_encrypt;
      ctx->decrypt_fn = _gcry_aes_aesni_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_aesni_prepare_decryption;
      ctx->use_avx = !!(hwfeatures & HWF_INTEL_AVX);
      ctx->use_avx2 = !!(hwfeatures & HWF_INTEL_AVX2);

      /* Setup AES-NI bulk encryption routines.  */
      bulk_ops->cfb_enc = _gcry_aes_aesni_cfb_enc;
      bulk_ops->cfb_dec = _gcry_aes_aesni_cfb_dec;
      bulk_ops->cbc_enc = _gcry_aes_aesni_cbc_enc;
      bulk_ops->cbc_dec = _gcry_aes_aesni_cbc_dec;
      bulk_ops->ctr_enc = _gcry_aes_aesni_ctr_enc;
      bulk_ops->ctr32le_enc = _gcry_aes_aesni_ctr32le_enc;
      bulk_ops->ocb_crypt = _gcry_aes_aesni_ocb_crypt;
      bulk_ops->ocb_auth = _gcry_aes_aesni_ocb_auth;
      bulk_ops->xts_crypt = _gcry_aes_aesni_xts_crypt;

#ifdef USE_VAES
      if ((hwfeatures & HWF_INTEL_VAES_VPCLMUL) &&
	  (hwfeatures & HWF_INTEL_AVX2))
	{
	  /* Setup VAES bulk encryption routines.  */
	  bulk_ops->cfb_dec = _gcry_aes_vaes_cfb_dec;
	  bulk_ops->cbc_dec = _gcry_aes_vaes_cbc_dec;
	  bulk_ops->ctr_enc = _gcry_aes_vaes_ctr_enc;
	  bulk_ops->ctr32le_enc = _gcry_aes_vaes_ctr32le_enc;
	  bulk_ops->ocb_crypt = _gcry_aes_vaes_ocb_crypt;
	  bulk_ops->xts_crypt = _gcry_aes_vaes_xts_crypt;
	}
#endif
    }
#endif
#ifdef USE_PADLOCK
  else if ((hwfeatures & HWF_PADLOCK_AES) && keylen == 128/8)
    {
      ctx->encrypt_fn = _gcry_aes_padlock_encrypt;
      ctx->decrypt_fn = _gcry_aes_padlock_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_padlock_prepare_decryption;
      memcpy (ctx->padlockkey, key, keylen);
    }
#endif
#ifdef USE_SSSE3
  else if (hwfeatures & HWF_INTEL_SSSE3)
    {
      hw_setkey = _gcry_aes_ssse3_do_setkey;
      ctx->encrypt_fn = _gcry_aes_ssse3_encrypt;
      ctx->decrypt_fn = _gcry_aes_ssse3_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_ssse3_prepare_decryption;

      /* Setup SSSE3 bulk encryption routines.  */
      bulk_ops->cfb_enc = _gcry_aes_ssse3_cfb_enc;
      bulk_ops->cfb_dec = _gcry_aes_ssse3_cfb_dec;
      bulk_ops->cbc_enc = _gcry_aes_ssse3_cbc_enc;
      bulk_ops->cbc_dec = _gcry_aes_ssse3_cbc_dec;
      bulk_ops->ctr_enc = _gcry_aes_ssse3_ctr_enc;
      bulk_ops->ocb_crypt = _gcry_aes_ssse3_ocb_crypt;
      bulk_ops->ocb_auth = _gcry_aes_ssse3_ocb_auth;
    }
#endif
#ifdef USE_ARM_CE
  else if (hwfeatures & HWF_ARM_AES)
    {
      hw_setkey = _gcry_aes_armv8_ce_setkey;
      ctx->encrypt_fn = _gcry_aes_armv8_ce_encrypt;
      ctx->decrypt_fn = _gcry_aes_armv8_ce_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_armv8_ce_prepare_decryption;

      /* Setup ARM-CE bulk encryption routines.  */
      bulk_ops->cfb_enc = _gcry_aes_armv8_ce_cfb_enc;
      bulk_ops->cfb_dec = _gcry_aes_armv8_ce_cfb_dec;
      bulk_ops->cbc_enc = _gcry_aes_armv8_ce_cbc_enc;
      bulk_ops->cbc_dec = _gcry_aes_armv8_ce_cbc_dec;
      bulk_ops->ctr_enc = _gcry_aes_armv8_ce_ctr_enc;
      bulk_ops->ctr32le_enc = _gcry_aes_armv8_ce_ctr32le_enc;
      bulk_ops->ocb_crypt = _gcry_aes_armv8_ce_ocb_crypt;
      bulk_ops->ocb_auth = _gcry_aes_armv8_ce_ocb_auth;
      bulk_ops->xts_crypt = _gcry_aes_armv8_ce_xts_crypt;
    }
#endif
#ifdef USE_PPC_CRYPTO_WITH_PPC9LE
  else if ((hwfeatures & HWF_PPC_VCRYPTO) && (hwfeatures & HWF_PPC_ARCH_3_00))
    {
      hw_setkey = _gcry_aes_ppc8_setkey;
      ctx->encrypt_fn = _gcry_aes_ppc9le_encrypt;
      ctx->decrypt_fn = _gcry_aes_ppc9le_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_ppc8_prepare_decryption;

      /* Setup PPC9LE bulk encryption routines.  */
      bulk_ops->cfb_enc = _gcry_aes_ppc9le_cfb_enc;
      bulk_ops->cfb_dec = _gcry_aes_ppc9le_cfb_dec;
      bulk_ops->cbc_enc = _gcry_aes_ppc9le_cbc_enc;
      bulk_ops->cbc_dec = _gcry_aes_ppc9le_cbc_dec;
      bulk_ops->ctr_enc = _gcry_aes_ppc9le_ctr_enc;
      bulk_ops->ocb_crypt = _gcry_aes_ppc9le_ocb_crypt;
      bulk_ops->ocb_auth = _gcry_aes_ppc9le_ocb_auth;
      bulk_ops->xts_crypt = _gcry_aes_ppc9le_xts_crypt;
      if (hwfeatures & HWF_PPC_ARCH_3_10)  /* for P10 */
        bulk_ops->gcm_crypt = _gcry_aes_p10le_gcm_crypt;
    }
#endif
#ifdef USE_PPC_CRYPTO
  else if (hwfeatures & HWF_PPC_VCRYPTO)
    {
      hw_setkey = _gcry_aes_ppc8_setkey;
      ctx->encrypt_fn = _gcry_aes_ppc8_encrypt;
      ctx->decrypt_fn = _gcry_aes_ppc8_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_ppc8_prepare_decryption;

      /* Setup PPC8 bulk encryption routines.  */
      bulk_ops->cfb_enc = _gcry_aes_ppc8_cfb_enc;
      bulk_ops->cfb_dec = _gcry_aes_ppc8_cfb_dec;
      bulk_ops->cbc_enc = _gcry_aes_ppc8_cbc_enc;
      bulk_ops->cbc_dec = _gcry_aes_ppc8_cbc_dec;
      bulk_ops->ctr_enc = _gcry_aes_ppc8_ctr_enc;
      bulk_ops->ocb_crypt = _gcry_aes_ppc8_ocb_crypt;
      bulk_ops->ocb_auth = _gcry_aes_ppc8_ocb_auth;
      bulk_ops->xts_crypt = _gcry_aes_ppc8_xts_crypt;
    }
#endif
#ifdef USE_S390X_CRYPTO
  else if (_gcry_aes_s390x_setup_acceleration (ctx, keylen, hwfeatures,
					       bulk_ops))
  {
      hw_setkey = _gcry_aes_s390x_setkey;
      ctx->encrypt_fn = _gcry_aes_s390x_encrypt;
      ctx->decrypt_fn = _gcry_aes_s390x_decrypt;
      ctx->prefetch_enc_fn = NULL;
      ctx->prefetch_dec_fn = NULL;
      ctx->prepare_decryption = _gcry_aes_s390x_prepare_decryption;
    }
#endif
  else
    {
      ctx->encrypt_fn = do_encrypt;
      ctx->decrypt_fn = do_decrypt;
      ctx->prefetch_enc_fn = prefetch_enc;
      ctx->prefetch_dec_fn = prefetch_dec;
      ctx->prepare_decryption = prepare_decryption;
    }

  /* NB: We don't yet support Padlock hardware key generation.  */

  if (hw_setkey)
    {
      hw_setkey (ctx, key);
    }
  else
    {
      const byte *sbox = ((const byte *)encT) + 1;
      union
        {
          PROPERLY_ALIGNED_TYPE dummy;
          byte data[MAXKC][4];
          u32 data32[MAXKC];
        } tkk[2];
#define k      tkk[0].data
#define k_u32  tkk[0].data32
#define tk     tkk[1].data
#define tk_u32 tkk[1].data32
#define W      (ctx->keyschenc)
#define W_u32  (ctx->keyschenc32)

      prefetch_enc();

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
          /* While not enough round key material calculated calculate
             new values.  */
          tk[0][0] ^= sbox[tk[KC-1][1] * 4];
          tk[0][1] ^= sbox[tk[KC-1][2] * 4];
          tk[0][2] ^= sbox[tk[KC-1][3] * 4];
          tk[0][3] ^= sbox[tk[KC-1][0] * 4];
          tk[0][0] ^= rcon[rconpointer++];

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
              tk[KC/2][0] ^= sbox[tk[KC/2 - 1][0] * 4];
              tk[KC/2][1] ^= sbox[tk[KC/2 - 1][1] * 4];
              tk[KC/2][2] ^= sbox[tk[KC/2 - 1][2] * 4];
              tk[KC/2][3] ^= sbox[tk[KC/2 - 1][3] * 4];
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
        }
#undef W
#undef tk
#undef k
#undef W_u32
#undef tk_u32
#undef k_u32
      wipememory(&tkk, sizeof(tkk));
    }

  return 0;
}


static gcry_err_code_t
rijndael_setkey (void *context, const byte *key, const unsigned keylen,
                 cipher_bulk_ops_t *bulk_ops)
{
  RIJNDAEL_context *ctx = context;
  return do_setkey (ctx, key, keylen, bulk_ops);
}


/* Make a decryption key from an encryption key. */
static void
prepare_decryption( RIJNDAEL_context *ctx )
{
  const byte *sbox = ((const byte *)encT) + 1;
  int r;

  prefetch_enc();
  prefetch_dec();

  ctx->keyschdec32[0][0] = ctx->keyschenc32[0][0];
  ctx->keyschdec32[0][1] = ctx->keyschenc32[0][1];
  ctx->keyschdec32[0][2] = ctx->keyschenc32[0][2];
  ctx->keyschdec32[0][3] = ctx->keyschenc32[0][3];

  for (r = 1; r < ctx->rounds; r++)
    {
      u32 *wi = ctx->keyschenc32[r];
      u32 *wo = ctx->keyschdec32[r];
      u32 wt;

      wt = wi[0];
      wo[0] = rol(decT[sbox[(byte)(wt >> 0) * 4]], 8 * 0)
	      ^ rol(decT[sbox[(byte)(wt >> 8) * 4]], 8 * 1)
	      ^ rol(decT[sbox[(byte)(wt >> 16) * 4]], 8 * 2)
	      ^ rol(decT[sbox[(byte)(wt >> 24) * 4]], 8 * 3);

      wt = wi[1];
      wo[1] = rol(decT[sbox[(byte)(wt >> 0) * 4]], 8 * 0)
	      ^ rol(decT[sbox[(byte)(wt >> 8) * 4]], 8 * 1)
	      ^ rol(decT[sbox[(byte)(wt >> 16) * 4]], 8 * 2)
	      ^ rol(decT[sbox[(byte)(wt >> 24) * 4]], 8 * 3);

      wt = wi[2];
      wo[2] = rol(decT[sbox[(byte)(wt >> 0) * 4]], 8 * 0)
	      ^ rol(decT[sbox[(byte)(wt >> 8) * 4]], 8 * 1)
	      ^ rol(decT[sbox[(byte)(wt >> 16) * 4]], 8 * 2)
	      ^ rol(decT[sbox[(byte)(wt >> 24) * 4]], 8 * 3);

      wt = wi[3];
      wo[3] = rol(decT[sbox[(byte)(wt >> 0) * 4]], 8 * 0)
	      ^ rol(decT[sbox[(byte)(wt >> 8) * 4]], 8 * 1)
	      ^ rol(decT[sbox[(byte)(wt >> 16) * 4]], 8 * 2)
	      ^ rol(decT[sbox[(byte)(wt >> 24) * 4]], 8 * 3);
    }

  ctx->keyschdec32[r][0] = ctx->keyschenc32[r][0];
  ctx->keyschdec32[r][1] = ctx->keyschenc32[r][1];
  ctx->keyschdec32[r][2] = ctx->keyschenc32[r][2];
  ctx->keyschdec32[r][3] = ctx->keyschenc32[r][3];
}


#if !defined(USE_ARM_ASM) && !defined(USE_AMD64_ASM)
/* Encrypt one block. A and B may be the same. */
static unsigned int
do_encrypt_fn (const RIJNDAEL_context *ctx, unsigned char *b,
               const unsigned char *a)
{
#define rk (ctx->keyschenc32)
  const byte *sbox = ((const byte *)encT) + 1;
  int rounds = ctx->rounds;
  int r;
  u32 sa[4];
  u32 sb[4];

  sb[0] = buf_get_le32(a + 0);
  sb[1] = buf_get_le32(a + 4);
  sb[2] = buf_get_le32(a + 8);
  sb[3] = buf_get_le32(a + 12);

  sa[0] = sb[0] ^ rk[0][0];
  sa[1] = sb[1] ^ rk[0][1];
  sa[2] = sb[2] ^ rk[0][2];
  sa[3] = sb[3] ^ rk[0][3];

  sb[0] = rol(encT[(byte)(sa[0] >> (0 * 8))], (0 * 8));
  sb[3] = rol(encT[(byte)(sa[0] >> (1 * 8))], (1 * 8));
  sb[2] = rol(encT[(byte)(sa[0] >> (2 * 8))], (2 * 8));
  sb[1] = rol(encT[(byte)(sa[0] >> (3 * 8))], (3 * 8));
  sa[0] = rk[1][0] ^ sb[0];

  sb[1] ^= rol(encT[(byte)(sa[1] >> (0 * 8))], (0 * 8));
  sa[0] ^= rol(encT[(byte)(sa[1] >> (1 * 8))], (1 * 8));
  sb[3] ^= rol(encT[(byte)(sa[1] >> (2 * 8))], (2 * 8));
  sb[2] ^= rol(encT[(byte)(sa[1] >> (3 * 8))], (3 * 8));
  sa[1] = rk[1][1] ^ sb[1];

  sb[2] ^= rol(encT[(byte)(sa[2] >> (0 * 8))], (0 * 8));
  sa[1] ^= rol(encT[(byte)(sa[2] >> (1 * 8))], (1 * 8));
  sa[0] ^= rol(encT[(byte)(sa[2] >> (2 * 8))], (2 * 8));
  sb[3] ^= rol(encT[(byte)(sa[2] >> (3 * 8))], (3 * 8));
  sa[2] = rk[1][2] ^ sb[2];

  sb[3] ^= rol(encT[(byte)(sa[3] >> (0 * 8))], (0 * 8));
  sa[2] ^= rol(encT[(byte)(sa[3] >> (1 * 8))], (1 * 8));
  sa[1] ^= rol(encT[(byte)(sa[3] >> (2 * 8))], (2 * 8));
  sa[0] ^= rol(encT[(byte)(sa[3] >> (3 * 8))], (3 * 8));
  sa[3] = rk[1][3] ^ sb[3];

  for (r = 2; r < rounds; r++)
    {
      sb[0] = rol(encT[(byte)(sa[0] >> (0 * 8))], (0 * 8));
      sb[3] = rol(encT[(byte)(sa[0] >> (1 * 8))], (1 * 8));
      sb[2] = rol(encT[(byte)(sa[0] >> (2 * 8))], (2 * 8));
      sb[1] = rol(encT[(byte)(sa[0] >> (3 * 8))], (3 * 8));
      sa[0] = rk[r][0] ^ sb[0];

      sb[1] ^= rol(encT[(byte)(sa[1] >> (0 * 8))], (0 * 8));
      sa[0] ^= rol(encT[(byte)(sa[1] >> (1 * 8))], (1 * 8));
      sb[3] ^= rol(encT[(byte)(sa[1] >> (2 * 8))], (2 * 8));
      sb[2] ^= rol(encT[(byte)(sa[1] >> (3 * 8))], (3 * 8));
      sa[1] = rk[r][1] ^ sb[1];

      sb[2] ^= rol(encT[(byte)(sa[2] >> (0 * 8))], (0 * 8));
      sa[1] ^= rol(encT[(byte)(sa[2] >> (1 * 8))], (1 * 8));
      sa[0] ^= rol(encT[(byte)(sa[2] >> (2 * 8))], (2 * 8));
      sb[3] ^= rol(encT[(byte)(sa[2] >> (3 * 8))], (3 * 8));
      sa[2] = rk[r][2] ^ sb[2];

      sb[3] ^= rol(encT[(byte)(sa[3] >> (0 * 8))], (0 * 8));
      sa[2] ^= rol(encT[(byte)(sa[3] >> (1 * 8))], (1 * 8));
      sa[1] ^= rol(encT[(byte)(sa[3] >> (2 * 8))], (2 * 8));
      sa[0] ^= rol(encT[(byte)(sa[3] >> (3 * 8))], (3 * 8));
      sa[3] = rk[r][3] ^ sb[3];

      r++;

      sb[0] = rol(encT[(byte)(sa[0] >> (0 * 8))], (0 * 8));
      sb[3] = rol(encT[(byte)(sa[0] >> (1 * 8))], (1 * 8));
      sb[2] = rol(encT[(byte)(sa[0] >> (2 * 8))], (2 * 8));
      sb[1] = rol(encT[(byte)(sa[0] >> (3 * 8))], (3 * 8));
      sa[0] = rk[r][0] ^ sb[0];

      sb[1] ^= rol(encT[(byte)(sa[1] >> (0 * 8))], (0 * 8));
      sa[0] ^= rol(encT[(byte)(sa[1] >> (1 * 8))], (1 * 8));
      sb[3] ^= rol(encT[(byte)(sa[1] >> (2 * 8))], (2 * 8));
      sb[2] ^= rol(encT[(byte)(sa[1] >> (3 * 8))], (3 * 8));
      sa[1] = rk[r][1] ^ sb[1];

      sb[2] ^= rol(encT[(byte)(sa[2] >> (0 * 8))], (0 * 8));
      sa[1] ^= rol(encT[(byte)(sa[2] >> (1 * 8))], (1 * 8));
      sa[0] ^= rol(encT[(byte)(sa[2] >> (2 * 8))], (2 * 8));
      sb[3] ^= rol(encT[(byte)(sa[2] >> (3 * 8))], (3 * 8));
      sa[2] = rk[r][2] ^ sb[2];

      sb[3] ^= rol(encT[(byte)(sa[3] >> (0 * 8))], (0 * 8));
      sa[2] ^= rol(encT[(byte)(sa[3] >> (1 * 8))], (1 * 8));
      sa[1] ^= rol(encT[(byte)(sa[3] >> (2 * 8))], (2 * 8));
      sa[0] ^= rol(encT[(byte)(sa[3] >> (3 * 8))], (3 * 8));
      sa[3] = rk[r][3] ^ sb[3];
    }

  /* Last round is special. */

  sb[0] = ((u32)sbox[(byte)(sa[0] >> (0 * 8)) * 4]) << (0 * 8);
  sb[3] = ((u32)sbox[(byte)(sa[0] >> (1 * 8)) * 4]) << (1 * 8);
  sb[2] = ((u32)sbox[(byte)(sa[0] >> (2 * 8)) * 4]) << (2 * 8);
  sb[1] = ((u32)sbox[(byte)(sa[0] >> (3 * 8)) * 4]) << (3 * 8);
  sa[0] = rk[r][0] ^ sb[0];

  sb[1] ^= ((u32)sbox[(byte)(sa[1] >> (0 * 8)) * 4]) << (0 * 8);
  sa[0] ^= ((u32)sbox[(byte)(sa[1] >> (1 * 8)) * 4]) << (1 * 8);
  sb[3] ^= ((u32)sbox[(byte)(sa[1] >> (2 * 8)) * 4]) << (2 * 8);
  sb[2] ^= ((u32)sbox[(byte)(sa[1] >> (3 * 8)) * 4]) << (3 * 8);
  sa[1] = rk[r][1] ^ sb[1];

  sb[2] ^= ((u32)sbox[(byte)(sa[2] >> (0 * 8)) * 4]) << (0 * 8);
  sa[1] ^= ((u32)sbox[(byte)(sa[2] >> (1 * 8)) * 4]) << (1 * 8);
  sa[0] ^= ((u32)sbox[(byte)(sa[2] >> (2 * 8)) * 4]) << (2 * 8);
  sb[3] ^= ((u32)sbox[(byte)(sa[2] >> (3 * 8)) * 4]) << (3 * 8);
  sa[2] = rk[r][2] ^ sb[2];

  sb[3] ^= ((u32)sbox[(byte)(sa[3] >> (0 * 8)) * 4]) << (0 * 8);
  sa[2] ^= ((u32)sbox[(byte)(sa[3] >> (1 * 8)) * 4]) << (1 * 8);
  sa[1] ^= ((u32)sbox[(byte)(sa[3] >> (2 * 8)) * 4]) << (2 * 8);
  sa[0] ^= ((u32)sbox[(byte)(sa[3] >> (3 * 8)) * 4]) << (3 * 8);
  sa[3] = rk[r][3] ^ sb[3];

  buf_put_le32(b + 0, sa[0]);
  buf_put_le32(b + 4, sa[1]);
  buf_put_le32(b + 8, sa[2]);
  buf_put_le32(b + 12, sa[3]);
#undef rk

  return (56 + 2*sizeof(int));
}
#endif /*!USE_ARM_ASM && !USE_AMD64_ASM*/


static unsigned int
do_encrypt (const RIJNDAEL_context *ctx,
            unsigned char *bx, const unsigned char *ax)
{
#ifdef USE_AMD64_ASM
  return _gcry_aes_amd64_encrypt_block(ctx->keyschenc, bx, ax, ctx->rounds,
				       enc_tables.T);
#elif defined(USE_ARM_ASM)
  return _gcry_aes_arm_encrypt_block(ctx->keyschenc, bx, ax, ctx->rounds,
				     enc_tables.T);
#else
  return do_encrypt_fn (ctx, bx, ax);
#endif /* !USE_ARM_ASM && !USE_AMD64_ASM*/
}


static unsigned int
rijndael_encrypt (void *context, byte *b, const byte *a)
{
  RIJNDAEL_context *ctx = context;

  if (ctx->prefetch_enc_fn)
    ctx->prefetch_enc_fn();

  return ctx->encrypt_fn (ctx, b, a);
}


/* Bulk encryption of complete blocks in CFB mode.  Caller needs to
   make sure that IV is aligned on an unsigned long boundary.  This
   function is only intended for the bulk encryption feature of
   cipher.c. */
static void
_gcry_aes_cfb_enc (void *context, unsigned char *iv,
                   void *outbuf_arg, const void *inbuf_arg,
                   size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int burn_depth = 0;
  rijndael_cryptfn_t encrypt_fn = ctx->encrypt_fn;

  if (ctx->prefetch_enc_fn)
    ctx->prefetch_enc_fn();

  for ( ;nblocks; nblocks-- )
    {
      /* Encrypt the IV. */
      burn_depth = encrypt_fn (ctx, iv, iv);
      /* XOR the input with the IV and store input into IV.  */
      cipher_block_xor_2dst(outbuf, iv, inbuf, BLOCKSIZE);
      outbuf += BLOCKSIZE;
      inbuf  += BLOCKSIZE;
    }

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));
}


/* Bulk encryption of complete blocks in CBC mode.  Caller needs to
   make sure that IV is aligned on an unsigned long boundary.  This
   function is only intended for the bulk encryption feature of
   cipher.c. */
static void
_gcry_aes_cbc_enc (void *context, unsigned char *iv,
                   void *outbuf_arg, const void *inbuf_arg,
                   size_t nblocks, int cbc_mac)
{
  RIJNDAEL_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned char *last_iv;
  unsigned int burn_depth = 0;
  rijndael_cryptfn_t encrypt_fn = ctx->encrypt_fn;

  if (ctx->prefetch_enc_fn)
    ctx->prefetch_enc_fn();

  last_iv = iv;

  for ( ;nblocks; nblocks-- )
    {
      cipher_block_xor(outbuf, inbuf, last_iv, BLOCKSIZE);

      burn_depth = encrypt_fn (ctx, outbuf, outbuf);

      last_iv = outbuf;
      inbuf += BLOCKSIZE;
      if (!cbc_mac)
	outbuf += BLOCKSIZE;
    }

  if (last_iv != iv)
    cipher_block_cpy (iv, last_iv, BLOCKSIZE);

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));
}


/* Bulk encryption of complete blocks in CTR mode.  Caller needs to
   make sure that CTR is aligned on a 16 byte boundary if AESNI; the
   minimum alignment is for an u32.  This function is only intended
   for the bulk encryption feature of cipher.c.  CTR is expected to be
   of size BLOCKSIZE. */
static void
_gcry_aes_ctr_enc (void *context, unsigned char *ctr,
                   void *outbuf_arg, const void *inbuf_arg,
                   size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int burn_depth = 0;
  union { unsigned char x1[16] ATTR_ALIGNED_16; u32 x32[4]; } tmp;
  rijndael_cryptfn_t encrypt_fn = ctx->encrypt_fn;

  if (ctx->prefetch_enc_fn)
    ctx->prefetch_enc_fn();

  for ( ;nblocks; nblocks-- )
    {
      /* Encrypt the counter. */
      burn_depth = encrypt_fn (ctx, tmp.x1, ctr);
      /* XOR the input with the encrypted counter and store in output.  */
      cipher_block_xor(outbuf, tmp.x1, inbuf, BLOCKSIZE);
      outbuf += BLOCKSIZE;
      inbuf  += BLOCKSIZE;
      /* Increment the counter.  */
      cipher_block_add(ctr, 1, BLOCKSIZE);
    }

  wipememory(&tmp, sizeof(tmp));

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));
}



#if !defined(USE_ARM_ASM) && !defined(USE_AMD64_ASM)
/* Decrypt one block.  A and B may be the same. */
static unsigned int
do_decrypt_fn (const RIJNDAEL_context *ctx, unsigned char *b,
               const unsigned char *a)
{
#define rk (ctx->keyschdec32)
  int rounds = ctx->rounds;
  int r;
  u32 sa[4];
  u32 sb[4];

  sb[0] = buf_get_le32(a + 0);
  sb[1] = buf_get_le32(a + 4);
  sb[2] = buf_get_le32(a + 8);
  sb[3] = buf_get_le32(a + 12);

  sa[0] = sb[0] ^ rk[rounds][0];
  sa[1] = sb[1] ^ rk[rounds][1];
  sa[2] = sb[2] ^ rk[rounds][2];
  sa[3] = sb[3] ^ rk[rounds][3];

  for (r = rounds - 1; r > 1; r--)
    {
      sb[0] = rol(decT[(byte)(sa[0] >> (0 * 8))], (0 * 8));
      sb[1] = rol(decT[(byte)(sa[0] >> (1 * 8))], (1 * 8));
      sb[2] = rol(decT[(byte)(sa[0] >> (2 * 8))], (2 * 8));
      sb[3] = rol(decT[(byte)(sa[0] >> (3 * 8))], (3 * 8));
      sa[0] = rk[r][0] ^ sb[0];

      sb[1] ^= rol(decT[(byte)(sa[1] >> (0 * 8))], (0 * 8));
      sb[2] ^= rol(decT[(byte)(sa[1] >> (1 * 8))], (1 * 8));
      sb[3] ^= rol(decT[(byte)(sa[1] >> (2 * 8))], (2 * 8));
      sa[0] ^= rol(decT[(byte)(sa[1] >> (3 * 8))], (3 * 8));
      sa[1] = rk[r][1] ^ sb[1];

      sb[2] ^= rol(decT[(byte)(sa[2] >> (0 * 8))], (0 * 8));
      sb[3] ^= rol(decT[(byte)(sa[2] >> (1 * 8))], (1 * 8));
      sa[0] ^= rol(decT[(byte)(sa[2] >> (2 * 8))], (2 * 8));
      sa[1] ^= rol(decT[(byte)(sa[2] >> (3 * 8))], (3 * 8));
      sa[2] = rk[r][2] ^ sb[2];

      sb[3] ^= rol(decT[(byte)(sa[3] >> (0 * 8))], (0 * 8));
      sa[0] ^= rol(decT[(byte)(sa[3] >> (1 * 8))], (1 * 8));
      sa[1] ^= rol(decT[(byte)(sa[3] >> (2 * 8))], (2 * 8));
      sa[2] ^= rol(decT[(byte)(sa[3] >> (3 * 8))], (3 * 8));
      sa[3] = rk[r][3] ^ sb[3];

      r--;

      sb[0] = rol(decT[(byte)(sa[0] >> (0 * 8))], (0 * 8));
      sb[1] = rol(decT[(byte)(sa[0] >> (1 * 8))], (1 * 8));
      sb[2] = rol(decT[(byte)(sa[0] >> (2 * 8))], (2 * 8));
      sb[3] = rol(decT[(byte)(sa[0] >> (3 * 8))], (3 * 8));
      sa[0] = rk[r][0] ^ sb[0];

      sb[1] ^= rol(decT[(byte)(sa[1] >> (0 * 8))], (0 * 8));
      sb[2] ^= rol(decT[(byte)(sa[1] >> (1 * 8))], (1 * 8));
      sb[3] ^= rol(decT[(byte)(sa[1] >> (2 * 8))], (2 * 8));
      sa[0] ^= rol(decT[(byte)(sa[1] >> (3 * 8))], (3 * 8));
      sa[1] = rk[r][1] ^ sb[1];

      sb[2] ^= rol(decT[(byte)(sa[2] >> (0 * 8))], (0 * 8));
      sb[3] ^= rol(decT[(byte)(sa[2] >> (1 * 8))], (1 * 8));
      sa[0] ^= rol(decT[(byte)(sa[2] >> (2 * 8))], (2 * 8));
      sa[1] ^= rol(decT[(byte)(sa[2] >> (3 * 8))], (3 * 8));
      sa[2] = rk[r][2] ^ sb[2];

      sb[3] ^= rol(decT[(byte)(sa[3] >> (0 * 8))], (0 * 8));
      sa[0] ^= rol(decT[(byte)(sa[3] >> (1 * 8))], (1 * 8));
      sa[1] ^= rol(decT[(byte)(sa[3] >> (2 * 8))], (2 * 8));
      sa[2] ^= rol(decT[(byte)(sa[3] >> (3 * 8))], (3 * 8));
      sa[3] = rk[r][3] ^ sb[3];
    }

  sb[0] = rol(decT[(byte)(sa[0] >> (0 * 8))], (0 * 8));
  sb[1] = rol(decT[(byte)(sa[0] >> (1 * 8))], (1 * 8));
  sb[2] = rol(decT[(byte)(sa[0] >> (2 * 8))], (2 * 8));
  sb[3] = rol(decT[(byte)(sa[0] >> (3 * 8))], (3 * 8));
  sa[0] = rk[1][0] ^ sb[0];

  sb[1] ^= rol(decT[(byte)(sa[1] >> (0 * 8))], (0 * 8));
  sb[2] ^= rol(decT[(byte)(sa[1] >> (1 * 8))], (1 * 8));
  sb[3] ^= rol(decT[(byte)(sa[1] >> (2 * 8))], (2 * 8));
  sa[0] ^= rol(decT[(byte)(sa[1] >> (3 * 8))], (3 * 8));
  sa[1] = rk[1][1] ^ sb[1];

  sb[2] ^= rol(decT[(byte)(sa[2] >> (0 * 8))], (0 * 8));
  sb[3] ^= rol(decT[(byte)(sa[2] >> (1 * 8))], (1 * 8));
  sa[0] ^= rol(decT[(byte)(sa[2] >> (2 * 8))], (2 * 8));
  sa[1] ^= rol(decT[(byte)(sa[2] >> (3 * 8))], (3 * 8));
  sa[2] = rk[1][2] ^ sb[2];

  sb[3] ^= rol(decT[(byte)(sa[3] >> (0 * 8))], (0 * 8));
  sa[0] ^= rol(decT[(byte)(sa[3] >> (1 * 8))], (1 * 8));
  sa[1] ^= rol(decT[(byte)(sa[3] >> (2 * 8))], (2 * 8));
  sa[2] ^= rol(decT[(byte)(sa[3] >> (3 * 8))], (3 * 8));
  sa[3] = rk[1][3] ^ sb[3];

  /* Last round is special. */
  sb[0] = (u32)inv_sbox[(byte)(sa[0] >> (0 * 8))] << (0 * 8);
  sb[1] = (u32)inv_sbox[(byte)(sa[0] >> (1 * 8))] << (1 * 8);
  sb[2] = (u32)inv_sbox[(byte)(sa[0] >> (2 * 8))] << (2 * 8);
  sb[3] = (u32)inv_sbox[(byte)(sa[0] >> (3 * 8))] << (3 * 8);
  sa[0] = sb[0] ^ rk[0][0];

  sb[1] ^= (u32)inv_sbox[(byte)(sa[1] >> (0 * 8))] << (0 * 8);
  sb[2] ^= (u32)inv_sbox[(byte)(sa[1] >> (1 * 8))] << (1 * 8);
  sb[3] ^= (u32)inv_sbox[(byte)(sa[1] >> (2 * 8))] << (2 * 8);
  sa[0] ^= (u32)inv_sbox[(byte)(sa[1] >> (3 * 8))] << (3 * 8);
  sa[1] = sb[1] ^ rk[0][1];

  sb[2] ^= (u32)inv_sbox[(byte)(sa[2] >> (0 * 8))] << (0 * 8);
  sb[3] ^= (u32)inv_sbox[(byte)(sa[2] >> (1 * 8))] << (1 * 8);
  sa[0] ^= (u32)inv_sbox[(byte)(sa[2] >> (2 * 8))] << (2 * 8);
  sa[1] ^= (u32)inv_sbox[(byte)(sa[2] >> (3 * 8))] << (3 * 8);
  sa[2] = sb[2] ^ rk[0][2];

  sb[3] ^= (u32)inv_sbox[(byte)(sa[3] >> (0 * 8))] << (0 * 8);
  sa[0] ^= (u32)inv_sbox[(byte)(sa[3] >> (1 * 8))] << (1 * 8);
  sa[1] ^= (u32)inv_sbox[(byte)(sa[3] >> (2 * 8))] << (2 * 8);
  sa[2] ^= (u32)inv_sbox[(byte)(sa[3] >> (3 * 8))] << (3 * 8);
  sa[3] = sb[3] ^ rk[0][3];

  buf_put_le32(b + 0, sa[0]);
  buf_put_le32(b + 4, sa[1]);
  buf_put_le32(b + 8, sa[2]);
  buf_put_le32(b + 12, sa[3]);
#undef rk

  return (56+2*sizeof(int));
}
#endif /*!USE_ARM_ASM && !USE_AMD64_ASM*/


/* Decrypt one block.  AX and BX may be the same. */
static unsigned int
do_decrypt (const RIJNDAEL_context *ctx, unsigned char *bx,
            const unsigned char *ax)
{
#ifdef USE_AMD64_ASM
  return _gcry_aes_amd64_decrypt_block(ctx->keyschdec, bx, ax, ctx->rounds,
				       dec_tables.T);
#elif defined(USE_ARM_ASM)
  return _gcry_aes_arm_decrypt_block(ctx->keyschdec, bx, ax, ctx->rounds,
				     dec_tables.T);
#else
  return do_decrypt_fn (ctx, bx, ax);
#endif /*!USE_ARM_ASM && !USE_AMD64_ASM*/
}


static inline void
check_decryption_preparation (RIJNDAEL_context *ctx)
{
  if ( !ctx->decryption_prepared )
    {
      ctx->prepare_decryption ( ctx );
      ctx->decryption_prepared = 1;
    }
}


static unsigned int
rijndael_decrypt (void *context, byte *b, const byte *a)
{
  RIJNDAEL_context *ctx = context;

  check_decryption_preparation (ctx);

  if (ctx->prefetch_dec_fn)
    ctx->prefetch_dec_fn();

  return ctx->decrypt_fn (ctx, b, a);
}


/* Bulk decryption of complete blocks in CFB mode.  Caller needs to
   make sure that IV is aligned on an unsigned long boundary.  This
   function is only intended for the bulk encryption feature of
   cipher.c. */
static void
_gcry_aes_cfb_dec (void *context, unsigned char *iv,
                   void *outbuf_arg, const void *inbuf_arg,
                   size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int burn_depth = 0;
  rijndael_cryptfn_t encrypt_fn = ctx->encrypt_fn;

  if (ctx->prefetch_enc_fn)
    ctx->prefetch_enc_fn();

  for ( ;nblocks; nblocks-- )
    {
      burn_depth = encrypt_fn (ctx, iv, iv);
      cipher_block_xor_n_copy(outbuf, iv, inbuf, BLOCKSIZE);
      outbuf += BLOCKSIZE;
      inbuf  += BLOCKSIZE;
    }

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));
}


/* Bulk decryption of complete blocks in CBC mode.  Caller needs to
   make sure that IV is aligned on an unsigned long boundary.  This
   function is only intended for the bulk encryption feature of
   cipher.c. */
static void
_gcry_aes_cbc_dec (void *context, unsigned char *iv,
                   void *outbuf_arg, const void *inbuf_arg,
                   size_t nblocks)
{
  RIJNDAEL_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int burn_depth = 0;
  unsigned char savebuf[BLOCKSIZE] ATTR_ALIGNED_16;
  rijndael_cryptfn_t decrypt_fn = ctx->decrypt_fn;

  check_decryption_preparation (ctx);

  if (ctx->prefetch_dec_fn)
    ctx->prefetch_dec_fn();

  for ( ;nblocks; nblocks-- )
    {
      /* INBUF is needed later and it may be identical to OUTBUF, so store
	  the intermediate result to SAVEBUF.  */

      burn_depth = decrypt_fn (ctx, savebuf, inbuf);

      cipher_block_xor_n_copy_2(outbuf, savebuf, iv, inbuf, BLOCKSIZE);
      inbuf += BLOCKSIZE;
      outbuf += BLOCKSIZE;
    }

  wipememory(savebuf, sizeof(savebuf));

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));
}



/* Bulk encryption/decryption of complete blocks in OCB mode. */
static size_t
_gcry_aes_ocb_crypt (gcry_cipher_hd_t c, void *outbuf_arg,
                     const void *inbuf_arg, size_t nblocks, int encrypt)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int burn_depth = 0;

  if (encrypt)
    {
      union { unsigned char x1[16] ATTR_ALIGNED_16; u32 x32[4]; } l_tmp;
      rijndael_cryptfn_t encrypt_fn = ctx->encrypt_fn;

      if (ctx->prefetch_enc_fn)
        ctx->prefetch_enc_fn();

      for ( ;nblocks; nblocks-- )
        {
          u64 i = ++c->u_mode.ocb.data_nblocks;
          const unsigned char *l = ocb_get_l(c, i);

          /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
          cipher_block_xor_1 (c->u_iv.iv, l, BLOCKSIZE);
          cipher_block_cpy (l_tmp.x1, inbuf, BLOCKSIZE);
          /* Checksum_i = Checksum_{i-1} xor P_i  */
          cipher_block_xor_1 (c->u_ctr.ctr, l_tmp.x1, BLOCKSIZE);
          /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
          cipher_block_xor_1 (l_tmp.x1, c->u_iv.iv, BLOCKSIZE);
          burn_depth = encrypt_fn (ctx, l_tmp.x1, l_tmp.x1);
          cipher_block_xor_1 (l_tmp.x1, c->u_iv.iv, BLOCKSIZE);
          cipher_block_cpy (outbuf, l_tmp.x1, BLOCKSIZE);

          inbuf += BLOCKSIZE;
          outbuf += BLOCKSIZE;
        }
    }
  else
    {
      union { unsigned char x1[16] ATTR_ALIGNED_16; u32 x32[4]; } l_tmp;
      rijndael_cryptfn_t decrypt_fn = ctx->decrypt_fn;

      check_decryption_preparation (ctx);

      if (ctx->prefetch_dec_fn)
        ctx->prefetch_dec_fn();

      for ( ;nblocks; nblocks-- )
        {
          u64 i = ++c->u_mode.ocb.data_nblocks;
          const unsigned char *l = ocb_get_l(c, i);

          /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
          cipher_block_xor_1 (c->u_iv.iv, l, BLOCKSIZE);
          cipher_block_cpy (l_tmp.x1, inbuf, BLOCKSIZE);
          /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */
          cipher_block_xor_1 (l_tmp.x1, c->u_iv.iv, BLOCKSIZE);
          burn_depth = decrypt_fn (ctx, l_tmp.x1, l_tmp.x1);
          cipher_block_xor_1 (l_tmp.x1, c->u_iv.iv, BLOCKSIZE);
          /* Checksum_i = Checksum_{i-1} xor P_i  */
          cipher_block_xor_1 (c->u_ctr.ctr, l_tmp.x1, BLOCKSIZE);
          cipher_block_cpy (outbuf, l_tmp.x1, BLOCKSIZE);

          inbuf += BLOCKSIZE;
          outbuf += BLOCKSIZE;
        }
    }

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));

  return 0;
}


/* Bulk authentication of complete blocks in OCB mode. */
static size_t
_gcry_aes_ocb_auth (gcry_cipher_hd_t c, const void *abuf_arg, size_t nblocks)
{
  RIJNDAEL_context *ctx = (void *)&c->context.c;
  const unsigned char *abuf = abuf_arg;
  unsigned int burn_depth = 0;
  union { unsigned char x1[16] ATTR_ALIGNED_16; u32 x32[4]; } l_tmp;
  rijndael_cryptfn_t encrypt_fn = ctx->encrypt_fn;

  if (ctx->prefetch_enc_fn)
    ctx->prefetch_enc_fn();

  for ( ;nblocks; nblocks-- )
    {
      u64 i = ++c->u_mode.ocb.aad_nblocks;
      const unsigned char *l = ocb_get_l(c, i);

      /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
      cipher_block_xor_1 (c->u_mode.ocb.aad_offset, l, BLOCKSIZE);
      /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */
      cipher_block_xor (l_tmp.x1, c->u_mode.ocb.aad_offset, abuf,
			BLOCKSIZE);
      burn_depth = encrypt_fn (ctx, l_tmp.x1, l_tmp.x1);
      cipher_block_xor_1 (c->u_mode.ocb.aad_sum, l_tmp.x1, BLOCKSIZE);

      abuf += BLOCKSIZE;
    }

  wipememory(&l_tmp, sizeof(l_tmp));

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 4 * sizeof(void *));

  return 0;
}


/* Bulk encryption/decryption of complete blocks in XTS mode. */
static void
_gcry_aes_xts_crypt (void *context, unsigned char *tweak,
		     void *outbuf_arg, const void *inbuf_arg,
		     size_t nblocks, int encrypt)
{
  RIJNDAEL_context *ctx = context;
  unsigned char *outbuf = outbuf_arg;
  const unsigned char *inbuf = inbuf_arg;
  unsigned int burn_depth = 0;
  rijndael_cryptfn_t crypt_fn;
  u64 tweak_lo, tweak_hi, tweak_next_lo, tweak_next_hi, tmp_lo, tmp_hi, carry;

  if (encrypt)
    {
      if (ctx->prefetch_enc_fn)
	ctx->prefetch_enc_fn();

      crypt_fn = ctx->encrypt_fn;
    }
  else
    {
      check_decryption_preparation (ctx);

      if (ctx->prefetch_dec_fn)
	ctx->prefetch_dec_fn();

      crypt_fn = ctx->decrypt_fn;
    }

  tweak_next_lo = buf_get_le64 (tweak + 0);
  tweak_next_hi = buf_get_le64 (tweak + 8);

  while (nblocks)
    {
      tweak_lo = tweak_next_lo;
      tweak_hi = tweak_next_hi;

      /* Xor-Encrypt/Decrypt-Xor block. */
      tmp_lo = buf_get_le64 (inbuf + 0) ^ tweak_lo;
      tmp_hi = buf_get_le64 (inbuf + 8) ^ tweak_hi;

      buf_put_le64 (outbuf + 0, tmp_lo);
      buf_put_le64 (outbuf + 8, tmp_hi);

      /* Generate next tweak. */
      carry = -(tweak_next_hi >> 63) & 0x87;
      tweak_next_hi = (tweak_next_hi << 1) + (tweak_next_lo >> 63);
      tweak_next_lo = (tweak_next_lo << 1) ^ carry;

      burn_depth = crypt_fn (ctx, outbuf, outbuf);

      buf_put_le64 (outbuf + 0, buf_get_le64 (outbuf + 0) ^ tweak_lo);
      buf_put_le64 (outbuf + 8, buf_get_le64 (outbuf + 8) ^ tweak_hi);

      outbuf += GCRY_XTS_BLOCK_LEN;
      inbuf += GCRY_XTS_BLOCK_LEN;
      nblocks--;
    }

  buf_put_le64 (tweak + 0, tweak_next_lo);
  buf_put_le64 (tweak + 8, tweak_next_hi);

  if (burn_depth)
    _gcry_burn_stack (burn_depth + 5 * sizeof(void *));
}


/* Run the self-tests for AES 128.  Returns NULL on success. */
static const char*
selftest_basic_128 (void)
{
  RIJNDAEL_context *ctx;
  unsigned char *ctxmem;
  unsigned char scratch[16];
  cipher_bulk_ops_t bulk_ops;

  /* The test vectors are from the AES supplied ones; more or less
     randomly taken from ecb_tbl.txt (I=42,81,14) */
#if 1
  static const unsigned char plaintext_128[16] =
    {
      0x01,0x4B,0xAF,0x22,0x78,0xA6,0x9D,0x33,
      0x1D,0x51,0x80,0x10,0x36,0x43,0xE9,0x9A
    };
  static const unsigned char key_128[16] =
    {
      0xE8,0xE9,0xEA,0xEB,0xED,0xEE,0xEF,0xF0,
      0xF2,0xF3,0xF4,0xF5,0xF7,0xF8,0xF9,0xFA
    };
  static const unsigned char ciphertext_128[16] =
    {
      0x67,0x43,0xC3,0xD1,0x51,0x9A,0xB4,0xF2,
      0xCD,0x9A,0x78,0xAB,0x09,0xA5,0x11,0xBD
    };
#else
  /* Test vectors from fips-197, appendix C. */
# warning debug test vectors in use
  static const unsigned char plaintext_128[16] =
    {
      0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
      0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };
  static const unsigned char key_128[16] =
    {
      0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
      0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
      /* 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, */
      /* 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c */
    };
  static const unsigned char ciphertext_128[16] =
    {
      0x69,0xc4,0xe0,0xd8,0x6a,0x7b,0x04,0x30,
      0xd8,0xcd,0xb7,0x80,0x70,0xb4,0xc5,0x5a
    };
#endif

  /* Because gcc/ld can only align the CTX struct on 8 bytes on the
     stack, we need to allocate that context on the heap.  */
  ctx = _gcry_cipher_selftest_alloc_ctx (sizeof *ctx, &ctxmem);
  if (!ctx)
    return "failed to allocate memory";

  rijndael_setkey (ctx, key_128, sizeof (key_128), &bulk_ops);
  rijndael_encrypt (ctx, scratch, plaintext_128);
  if (memcmp (scratch, ciphertext_128, sizeof (ciphertext_128)))
    {
      xfree (ctxmem);
      return "AES-128 test encryption failed.";
    }
  rijndael_decrypt (ctx, scratch, scratch);
  xfree (ctxmem);
  if (memcmp (scratch, plaintext_128, sizeof (plaintext_128)))
    return "AES-128 test decryption failed.";

  return NULL;
}

/* Run the self-tests for AES 192.  Returns NULL on success. */
static const char*
selftest_basic_192 (void)
{
  RIJNDAEL_context *ctx;
  unsigned char *ctxmem;
  unsigned char scratch[16];
  cipher_bulk_ops_t bulk_ops;

  static unsigned char plaintext_192[16] =
    {
      0x76,0x77,0x74,0x75,0xF1,0xF2,0xF3,0xF4,
      0xF8,0xF9,0xE6,0xE7,0x77,0x70,0x71,0x72
    };
  static unsigned char key_192[24] =
    {
      0x04,0x05,0x06,0x07,0x09,0x0A,0x0B,0x0C,
      0x0E,0x0F,0x10,0x11,0x13,0x14,0x15,0x16,
      0x18,0x19,0x1A,0x1B,0x1D,0x1E,0x1F,0x20
    };
  static const unsigned char ciphertext_192[16] =
    {
      0x5D,0x1E,0xF2,0x0D,0xCE,0xD6,0xBC,0xBC,
      0x12,0x13,0x1A,0xC7,0xC5,0x47,0x88,0xAA
    };

  ctx = _gcry_cipher_selftest_alloc_ctx (sizeof *ctx, &ctxmem);
  if (!ctx)
    return "failed to allocate memory";
  rijndael_setkey (ctx, key_192, sizeof(key_192), &bulk_ops);
  rijndael_encrypt (ctx, scratch, plaintext_192);
  if (memcmp (scratch, ciphertext_192, sizeof (ciphertext_192)))
    {
      xfree (ctxmem);
      return "AES-192 test encryption failed.";
    }
  rijndael_decrypt (ctx, scratch, scratch);
  xfree (ctxmem);
  if (memcmp (scratch, plaintext_192, sizeof (plaintext_192)))
    return "AES-192 test decryption failed.";

  return NULL;
}


/* Run the self-tests for AES 256.  Returns NULL on success. */
static const char*
selftest_basic_256 (void)
{
  RIJNDAEL_context *ctx;
  unsigned char *ctxmem;
  unsigned char scratch[16];
  cipher_bulk_ops_t bulk_ops;

  static unsigned char plaintext_256[16] =
    {
      0x06,0x9A,0x00,0x7F,0xC7,0x6A,0x45,0x9F,
      0x98,0xBA,0xF9,0x17,0xFE,0xDF,0x95,0x21
    };
  static unsigned char key_256[32] =
    {
      0x08,0x09,0x0A,0x0B,0x0D,0x0E,0x0F,0x10,
      0x12,0x13,0x14,0x15,0x17,0x18,0x19,0x1A,
      0x1C,0x1D,0x1E,0x1F,0x21,0x22,0x23,0x24,
      0x26,0x27,0x28,0x29,0x2B,0x2C,0x2D,0x2E
    };
  static const unsigned char ciphertext_256[16] =
    {
      0x08,0x0E,0x95,0x17,0xEB,0x16,0x77,0x71,
      0x9A,0xCF,0x72,0x80,0x86,0x04,0x0A,0xE3
    };

  ctx = _gcry_cipher_selftest_alloc_ctx (sizeof *ctx, &ctxmem);
  if (!ctx)
    return "failed to allocate memory";
  rijndael_setkey (ctx, key_256, sizeof(key_256), &bulk_ops);
  rijndael_encrypt (ctx, scratch, plaintext_256);
  if (memcmp (scratch, ciphertext_256, sizeof (ciphertext_256)))
    {
      xfree (ctxmem);
      return "AES-256 test encryption failed.";
    }
  rijndael_decrypt (ctx, scratch, scratch);
  xfree (ctxmem);
  if (memcmp (scratch, plaintext_256, sizeof (plaintext_256)))
    return "AES-256 test decryption failed.";

  return NULL;
}


/* Run the self-tests for AES-CTR-128, tests IV increment of bulk CTR
   encryption.  Returns NULL on success. */
static const char*
selftest_ctr_128 (void)
{
#ifdef USE_VAES
  const int nblocks = 16+1;
#else
  const int nblocks = 8+1;
#endif
  const int blocksize = BLOCKSIZE;
  const int context_size = sizeof(RIJNDAEL_context);

  return _gcry_selftest_helper_ctr("AES", &rijndael_setkey,
           &rijndael_encrypt, nblocks, blocksize, context_size);
}


/* Run the self-tests for AES-CBC-128, tests bulk CBC decryption.
   Returns NULL on success. */
static const char*
selftest_cbc_128 (void)
{
#ifdef USE_VAES
  const int nblocks = 16+2;
#else
  const int nblocks = 8+2;
#endif
  const int blocksize = BLOCKSIZE;
  const int context_size = sizeof(RIJNDAEL_context);

  return _gcry_selftest_helper_cbc("AES", &rijndael_setkey,
           &rijndael_encrypt, nblocks, blocksize, context_size);
}


/* Run the self-tests for AES-CFB-128, tests bulk CFB decryption.
   Returns NULL on success. */
static const char*
selftest_cfb_128 (void)
{
#ifdef USE_VAES
  const int nblocks = 16+2;
#else
  const int nblocks = 8+2;
#endif
  const int blocksize = BLOCKSIZE;
  const int context_size = sizeof(RIJNDAEL_context);

  return _gcry_selftest_helper_cfb("AES", &rijndael_setkey,
           &rijndael_encrypt, nblocks, blocksize, context_size);
}


/* Run all the self-tests and return NULL on success.  This function
   is used for the on-the-fly self-tests. */
static const char *
selftest (void)
{
  const char *r;

  if ( (r = selftest_basic_128 ())
       || (r = selftest_basic_192 ())
       || (r = selftest_basic_256 ()) )
    return r;

  if ( (r = selftest_ctr_128 ()) )
    return r;

  if ( (r = selftest_cbc_128 ()) )
    return r;

  if ( (r = selftest_cfb_128 ()) )
    return r;

  return r;
}


/* SP800-38a.pdf for AES-128.  */
static const char *
selftest_fips_128_38a (int requested_mode)
{
  static const struct tv
  {
    int mode;
    const unsigned char key[16];
    const unsigned char iv[16];
    struct
    {
      const unsigned char input[16];
      const unsigned char output[16];
    } data[4];
  } tv[2] =
    {
      {
        GCRY_CIPHER_MODE_CFB,  /* F.3.13, CFB128-AES128 */
        { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
          0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
        {
          { { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
              0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a },
            { 0x3b, 0x3f, 0xd9, 0x2e, 0xb7, 0x2d, 0xad, 0x20,
              0x33, 0x34, 0x49, 0xf8, 0xe8, 0x3c, 0xfb, 0x4a } },

          { { 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
              0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51 },
            { 0xc8, 0xa6, 0x45, 0x37, 0xa0, 0xb3, 0xa9, 0x3f,
              0xcd, 0xe3, 0xcd, 0xad, 0x9f, 0x1c, 0xe5, 0x8b } },

          { { 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
              0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef },
            { 0x26, 0x75, 0x1f, 0x67, 0xa3, 0xcb, 0xb1, 0x40,
              0xb1, 0x80, 0x8c, 0xf1, 0x87, 0xa4, 0xf4, 0xdf } },

          { { 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
              0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 },
            { 0xc0, 0x4b, 0x05, 0x35, 0x7c, 0x5d, 0x1c, 0x0e,
              0xea, 0xc4, 0xc6, 0x6f, 0x9f, 0xf7, 0xf2, 0xe6 } }
        }
      },
      {
        GCRY_CIPHER_MODE_OFB,
        { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
          0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
        {
          { { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
              0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a },
            { 0x3b, 0x3f, 0xd9, 0x2e, 0xb7, 0x2d, 0xad, 0x20,
              0x33, 0x34, 0x49, 0xf8, 0xe8, 0x3c, 0xfb, 0x4a } },

          { { 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
              0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51 },
            { 0x77, 0x89, 0x50, 0x8d, 0x16, 0x91, 0x8f, 0x03,
              0xf5, 0x3c, 0x52, 0xda, 0xc5, 0x4e, 0xd8, 0x25 } },

          { { 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
              0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef },
            { 0x97, 0x40, 0x05, 0x1e, 0x9c, 0x5f, 0xec, 0xf6,
              0x43, 0x44, 0xf7, 0xa8, 0x22, 0x60, 0xed, 0xcc } },

          { { 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
              0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 },
            { 0x30, 0x4c, 0x65, 0x28, 0xf6, 0x59, 0xc7, 0x78,
              0x66, 0xa5, 0x10, 0xd9, 0xc1, 0xd6, 0xae, 0x5e } },
        }
      }
    };
  unsigned char scratch[16];
  gpg_error_t err;
  int tvi, idx;
  gcry_cipher_hd_t hdenc = NULL;
  gcry_cipher_hd_t hddec = NULL;

#define Fail(a) do {           \
    _gcry_cipher_close (hdenc);  \
    _gcry_cipher_close (hddec);  \
    return a;                    \
  } while (0)

  gcry_assert (sizeof tv[0].data[0].input == sizeof scratch);
  gcry_assert (sizeof tv[0].data[0].output == sizeof scratch);

  for (tvi=0; tvi < DIM (tv); tvi++)
    if (tv[tvi].mode == requested_mode)
      break;
  if (tvi == DIM (tv))
    Fail ("no test data for this mode");

  err = _gcry_cipher_open (&hdenc, GCRY_CIPHER_AES, tv[tvi].mode, 0);
  if (err)
    Fail ("open");
  err = _gcry_cipher_open (&hddec, GCRY_CIPHER_AES, tv[tvi].mode, 0);
  if (err)
    Fail ("open");
  err = _gcry_cipher_setkey (hdenc, tv[tvi].key,  sizeof tv[tvi].key);
  if (!err)
    err = _gcry_cipher_setkey (hddec, tv[tvi].key, sizeof tv[tvi].key);
  if (err)
    Fail ("set key");
  err = _gcry_cipher_setiv (hdenc, tv[tvi].iv, sizeof tv[tvi].iv);
  if (!err)
    err = _gcry_cipher_setiv (hddec, tv[tvi].iv, sizeof tv[tvi].iv);
  if (err)
    Fail ("set IV");
  for (idx=0; idx < DIM (tv[tvi].data); idx++)
    {
      err = _gcry_cipher_encrypt (hdenc, scratch, sizeof scratch,
                                  tv[tvi].data[idx].input,
                                  sizeof tv[tvi].data[idx].input);
      if (err)
        Fail ("encrypt command");
      if (memcmp (scratch, tv[tvi].data[idx].output, sizeof scratch))
        Fail ("encrypt mismatch");
      err = _gcry_cipher_decrypt (hddec, scratch, sizeof scratch,
                                  tv[tvi].data[idx].output,
                                  sizeof tv[tvi].data[idx].output);
      if (err)
        Fail ("decrypt command");
      if (memcmp (scratch, tv[tvi].data[idx].input, sizeof scratch))
        Fail ("decrypt mismatch");
    }

#undef Fail
  _gcry_cipher_close (hdenc);
  _gcry_cipher_close (hddec);
  return NULL;
}


/* Complete selftest for AES-128 with all modes and driver code.  */
static gpg_err_code_t
selftest_fips_128 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  what = "low-level";
  errtxt = selftest_basic_128 ();
  if (errtxt)
    goto failed;

  if (extended)
    {
      what = "cfb";
      errtxt = selftest_fips_128_38a (GCRY_CIPHER_MODE_CFB);
      if (errtxt)
        goto failed;

      what = "ofb";
      errtxt = selftest_fips_128_38a (GCRY_CIPHER_MODE_OFB);
      if (errtxt)
        goto failed;
    }

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("cipher", GCRY_CIPHER_AES128, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}

/* Complete selftest for AES-192.  */
static gpg_err_code_t
selftest_fips_192 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  (void)extended; /* No extended tests available.  */

  what = "low-level";
  errtxt = selftest_basic_192 ();
  if (errtxt)
    goto failed;


  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("cipher", GCRY_CIPHER_AES192, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}


/* Complete selftest for AES-256.  */
static gpg_err_code_t
selftest_fips_256 (int extended, selftest_report_func_t report)
{
  const char *what;
  const char *errtxt;

  (void)extended; /* No extended tests available.  */

  what = "low-level";
  errtxt = selftest_basic_256 ();
  if (errtxt)
    goto failed;

  return 0; /* Succeeded. */

 failed:
  if (report)
    report ("cipher", GCRY_CIPHER_AES256, what, errtxt);
  return GPG_ERR_SELFTEST_FAILED;
}



/* Run a full self-test for ALGO and return 0 on success.  */
static gpg_err_code_t
run_selftests (int algo, int extended, selftest_report_func_t report)
{
  gpg_err_code_t ec;

  switch (algo)
    {
    case GCRY_CIPHER_AES128:
      ec = selftest_fips_128 (extended, report);
      break;
    case GCRY_CIPHER_AES192:
      ec = selftest_fips_192 (extended, report);
      break;
    case GCRY_CIPHER_AES256:
      ec = selftest_fips_256 (extended, report);
      break;
    default:
      ec = GPG_ERR_CIPHER_ALGO;
      break;

    }
  return ec;
}




static const char *rijndael_names[] =
  {
    "RIJNDAEL",
    "AES128",
    "AES-128",
    NULL
  };

static const gcry_cipher_oid_spec_t rijndael_oids[] =
  {
    { "2.16.840.1.101.3.4.1.1", GCRY_CIPHER_MODE_ECB },
    { "2.16.840.1.101.3.4.1.2", GCRY_CIPHER_MODE_CBC },
    { "2.16.840.1.101.3.4.1.3", GCRY_CIPHER_MODE_OFB },
    { "2.16.840.1.101.3.4.1.4", GCRY_CIPHER_MODE_CFB },
    { "2.16.840.1.101.3.4.1.6", GCRY_CIPHER_MODE_GCM },
    { "2.16.840.1.101.3.4.1.7", GCRY_CIPHER_MODE_CCM },
    { NULL }
  };

gcry_cipher_spec_t _gcry_cipher_spec_aes =
  {
    GCRY_CIPHER_AES, {0, 1},
    "AES", rijndael_names, rijndael_oids, 16, 128,
    sizeof (RIJNDAEL_context),
    rijndael_setkey, rijndael_encrypt, rijndael_decrypt,
    NULL, NULL,
    run_selftests
  };


static const char *rijndael192_names[] =
  {
    "RIJNDAEL192",
    "AES-192",
    NULL
  };

static const gcry_cipher_oid_spec_t rijndael192_oids[] =
  {
    { "2.16.840.1.101.3.4.1.21", GCRY_CIPHER_MODE_ECB },
    { "2.16.840.1.101.3.4.1.22", GCRY_CIPHER_MODE_CBC },
    { "2.16.840.1.101.3.4.1.23", GCRY_CIPHER_MODE_OFB },
    { "2.16.840.1.101.3.4.1.24", GCRY_CIPHER_MODE_CFB },
    { "2.16.840.1.101.3.4.1.26", GCRY_CIPHER_MODE_GCM },
    { "2.16.840.1.101.3.4.1.27", GCRY_CIPHER_MODE_CCM },
    { NULL }
  };

gcry_cipher_spec_t _gcry_cipher_spec_aes192 =
  {
    GCRY_CIPHER_AES192, {0, 1},
    "AES192", rijndael192_names, rijndael192_oids, 16, 192,
    sizeof (RIJNDAEL_context),
    rijndael_setkey, rijndael_encrypt, rijndael_decrypt,
    NULL, NULL,
    run_selftests
  };


static const char *rijndael256_names[] =
  {
    "RIJNDAEL256",
    "AES-256",
    NULL
  };

static const gcry_cipher_oid_spec_t rijndael256_oids[] =
  {
    { "2.16.840.1.101.3.4.1.41", GCRY_CIPHER_MODE_ECB },
    { "2.16.840.1.101.3.4.1.42", GCRY_CIPHER_MODE_CBC },
    { "2.16.840.1.101.3.4.1.43", GCRY_CIPHER_MODE_OFB },
    { "2.16.840.1.101.3.4.1.44", GCRY_CIPHER_MODE_CFB },
    { "2.16.840.1.101.3.4.1.46", GCRY_CIPHER_MODE_GCM },
    { "2.16.840.1.101.3.4.1.47", GCRY_CIPHER_MODE_CCM },
    { NULL }
  };

gcry_cipher_spec_t _gcry_cipher_spec_aes256 =
  {
    GCRY_CIPHER_AES256, {0, 1},
    "AES256", rijndael256_names, rijndael256_oids, 16, 256,
    sizeof (RIJNDAEL_context),
    rijndael_setkey, rijndael_encrypt, rijndael_decrypt,
    NULL, NULL,
    run_selftests
  };
