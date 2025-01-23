/* cipher-internal.h  - Internal defs for cipher.c
 * Copyright (C) 2011 Free Software Foundation, Inc.
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
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef G10_CIPHER_INTERNAL_H
#define G10_CIPHER_INTERNAL_H

#include "./poly1305-internal.h"


/* The maximum supported size of a block in bytes.  */
#define MAX_BLOCKSIZE 16

/* The length for an OCB block.  Although OCB supports any block
   length it does not make sense to use a 64 bit blocklen (and cipher)
   because this reduces the security margin to an unacceptable state.
   Thus we require a cipher with 128 bit blocklength.  */
#define OCB_BLOCK_LEN  (128/8)

/* The size of the pre-computed L table for OCB.  This takes the same
   size as the table used for GCM and thus we don't save anything by
   not using such a table.  */
#define OCB_L_TABLE_SIZE 16


/* Check the above constants.  */
#if OCB_BLOCK_LEN > MAX_BLOCKSIZE
# error OCB_BLOCKLEN > MAX_BLOCKSIZE
#endif



/* Magic values for the context structure.  */
#define CTX_MAGIC_NORMAL 0x24091964
#define CTX_MAGIC_SECURE 0x46919042

/* Try to use 16 byte aligned cipher context for better performance.
   We use the aligned attribute, thus it is only possible to implement
   this with gcc.  */
#undef NEED_16BYTE_ALIGNED_CONTEXT
#ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
# define NEED_16BYTE_ALIGNED_CONTEXT 1
#endif

/* Undef this symbol to trade GCM speed for 256 bytes of memory per context */
#define GCM_USE_TABLES 1


/* GCM_USE_INTEL_PCLMUL indicates whether to compile GCM with Intel PCLMUL
   code.  */
#undef GCM_USE_INTEL_PCLMUL
#if defined(ENABLE_PCLMUL_SUPPORT) && defined(GCM_USE_TABLES)
# if ((defined(__i386__) && SIZEOF_UNSIGNED_LONG == 4) || defined(__x86_64__))
#  if __GNUC__ >= 4
#   define GCM_USE_INTEL_PCLMUL 1
#  endif
# endif
#endif /* GCM_USE_INTEL_PCLMUL */

/* GCM_USE_ARM_PMULL indicates whether to compile GCM with ARMv8 PMULL code. */
#undef GCM_USE_ARM_PMULL
#if defined(ENABLE_ARM_CRYPTO_SUPPORT) && defined(GCM_USE_TABLES)
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO)
#  define GCM_USE_ARM_PMULL 1
# elif defined(__AARCH64EL__) && \
    defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)
#  define GCM_USE_ARM_PMULL 1
# endif
#endif /* GCM_USE_ARM_PMULL */

/* GCM_USE_ARM_NEON indicates whether to compile GCM with ARMv7 NEON code. */
#undef GCM_USE_ARM_NEON
#if defined(GCM_USE_TABLES)
#if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) && \
    defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_NEON)
#  define GCM_USE_ARM_NEON 1
#endif
#endif /* GCM_USE_ARM_NEON */

/* GCM_USE_S390X_CRYPTO indicates whether to enable zSeries code. */
#undef GCM_USE_S390X_CRYPTO
#if defined(HAVE_GCC_INLINE_ASM_S390X)
# define GCM_USE_S390X_CRYPTO 1
#endif /* GCM_USE_S390X_CRYPTO */

/* GCM_USE_PPC_VPMSUM indicates whether to compile GCM with PPC Power 8
 * polynomial multiplication instruction. */
#undef GCM_USE_PPC_VPMSUM
#if defined(GCM_USE_TABLES)
#if defined(ENABLE_PPC_CRYPTO_SUPPORT) && defined(__powerpc64__) && \
    defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
    defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC) && __GNUC__ >= 4
#  define GCM_USE_PPC_VPMSUM 1
#  define NEED_16BYTE_ALIGNED_CONTEXT 1 /* this also aligns gcm_table */
#endif
#endif /* GCM_USE_PPC_VPMSUM */

typedef unsigned int (*ghash_fn_t) (gcry_cipher_hd_t c, byte *result,
                                    const byte *buf, size_t nblocks);


/* A structure with function pointers for mode operations. */
typedef struct cipher_mode_ops
{
  gcry_err_code_t (*encrypt)(gcry_cipher_hd_t c, unsigned char *outbuf,
			     size_t outbuflen, const unsigned char *inbuf,
			     size_t inbuflen);
  gcry_err_code_t (*decrypt)(gcry_cipher_hd_t c, unsigned char *outbuf,
			     size_t outbuflen, const unsigned char *inbuf,
			     size_t inbuflen);
  gcry_err_code_t (*setiv)(gcry_cipher_hd_t c, const unsigned char *iv,
			   size_t ivlen);

  gcry_err_code_t (*authenticate)(gcry_cipher_hd_t c,
				  const unsigned char *abuf, size_t abuflen);
  gcry_err_code_t (*get_tag)(gcry_cipher_hd_t c, unsigned char *outtag,
			     size_t taglen);
  gcry_err_code_t (*check_tag)(gcry_cipher_hd_t c, const unsigned char *intag,
			       size_t taglen);
} cipher_mode_ops_t;


/* A structure with function pointers for bulk operations.  The cipher
   algorithm setkey function initializes them when bulk operations are
   available and the actual encryption routines use them if they are
   not NULL.  */
typedef struct cipher_bulk_ops
{
  void (*cfb_enc)(void *context, unsigned char *iv, void *outbuf_arg,
		  const void *inbuf_arg, size_t nblocks);
  void (*cfb_dec)(void *context, unsigned char *iv, void *outbuf_arg,
		  const void *inbuf_arg, size_t nblocks);
  void (*cbc_enc)(void *context, unsigned char *iv, void *outbuf_arg,
		  const void *inbuf_arg, size_t nblocks, int cbc_mac);
  void (*cbc_dec)(void *context, unsigned char *iv, void *outbuf_arg,
		  const void *inbuf_arg, size_t nblocks);
  void (*ofb_enc)(void *context, unsigned char *iv, void *outbuf_arg,
		  const void *inbuf_arg, size_t nblocks);
  void (*ctr_enc)(void *context, unsigned char *iv, void *outbuf_arg,
		  const void *inbuf_arg, size_t nblocks);
  void (*ctr32le_enc)(void *context, unsigned char *iv, void *outbuf_arg,
		      const void *inbuf_arg, size_t nblocks);
  size_t (*ocb_crypt)(gcry_cipher_hd_t c, void *outbuf_arg,
		      const void *inbuf_arg, size_t nblocks, int encrypt);
  size_t (*ocb_auth)(gcry_cipher_hd_t c, const void *abuf_arg, size_t nblocks);
  void (*xts_crypt)(void *context, unsigned char *tweak, void *outbuf_arg,
		    const void *inbuf_arg, size_t nblocks, int encrypt);
  size_t (*gcm_crypt)(gcry_cipher_hd_t c, void *outbuf_arg,
		      const void *inbuf_arg, size_t nblocks, int encrypt);
} cipher_bulk_ops_t;


/* A VIA processor with the Padlock engine as well as the Intel AES_NI
   instructions require an alignment of most data on a 16 byte
   boundary.  Because we trick out the compiler while allocating the
   context, the align attribute as used in rijndael.c does not work on
   its own.  Thus we need to make sure that the entire context
   structure is a aligned on that boundary.  We achieve this by
   defining a new type and use that instead of our usual alignment
   type.  */
typedef union
{
  PROPERLY_ALIGNED_TYPE foo;
#ifdef NEED_16BYTE_ALIGNED_CONTEXT
  char bar[16] __attribute__ ((aligned (16)));
#endif
  char c[1];
} cipher_context_alignment_t;


/* Storage structure for CMAC, for CMAC and EAX modes. */
typedef struct {
  /* The initialization vector. Also contains tag after finalization. */
  union {
    cipher_context_alignment_t iv_align;
    unsigned char iv[MAX_BLOCKSIZE];
  } u_iv;

  /* Subkeys for tag creation, not cleared by gcry_cipher_reset. */
  unsigned char subkeys[2][MAX_BLOCKSIZE];

  /* Space to save partial input lengths for MAC. */
  unsigned char macbuf[MAX_BLOCKSIZE];

  int mac_unused;  /* Number of unprocessed bytes in MACBUF. */
  unsigned int tag:1; /* Set to 1 if tag has been finalized.  */
} gcry_cmac_context_t;


/* The handle structure.  */
struct gcry_cipher_handle
{
  int magic;
  size_t actual_handle_size;     /* Allocated size of this handle. */
  size_t handle_offset;          /* Offset to the malloced block.  */
  gcry_cipher_spec_t *spec;

  /* The algorithm id.  This is a hack required because the module
     interface does not easily allow to retrieve this value. */
  int algo;

  /* A structure with function pointers for mode operations. */
  cipher_mode_ops_t mode_ops;

  /* A structure with function pointers for bulk operations.  Due to
     limitations of the module system (we don't want to change the
     API) we need to keep these function pointers here.  */
  cipher_bulk_ops_t bulk;

  int mode;
  unsigned int flags;

  struct {
    unsigned int key:1; /* Set to 1 if a key has been set.  */
    unsigned int iv:1;  /* Set to 1 if a IV has been set.  */
    unsigned int tag:1; /* Set to 1 if a tag is finalized. */
    unsigned int finalize:1; /* Next encrypt/decrypt has the final data.  */
    unsigned int allow_weak_key:1; /* Set to 1 if weak keys are allowed. */
  } marks;

  /* The initialization vector.  For best performance we make sure
     that it is properly aligned.  In particular some implementations
     of bulk operations expect an 16 byte aligned IV.  IV is also used
     to store CBC-MAC in CCM mode; counter IV is stored in U_CTR.  For
     OCB mode it is used for the offset value.  */
  union {
    cipher_context_alignment_t iv_align;
    unsigned char iv[MAX_BLOCKSIZE];
  } u_iv;

  /* The counter for CTR mode.  This field is also used by AESWRAP and
     thus we can't use the U_IV union.  For OCB mode it is used for
     the checksum.  */
  union {
    cipher_context_alignment_t iv_align;
    unsigned char ctr[MAX_BLOCKSIZE];
  } u_ctr;

  /* Space to save an IV or CTR for chaining operations.  */
  unsigned char lastiv[MAX_BLOCKSIZE];
  int unused;  /* Number of unused bytes in LASTIV. */

  union {
    /* Mode specific storage for CCM mode. */
    struct {
      u64 encryptlen;
      u64 aadlen;
      unsigned int authlen;

      /* Space to save partial input lengths for MAC. */
      unsigned char macbuf[GCRY_CCM_BLOCK_LEN];
      int mac_unused;  /* Number of unprocessed bytes in MACBUF. */

      unsigned char s0[GCRY_CCM_BLOCK_LEN];

      unsigned int nonce:1; /* Set to 1 if nonce has been set.  */
      unsigned int lengths:1; /* Set to 1 if CCM length parameters has been
                                 processed.  */
    } ccm;

    /* Mode specific storage for Poly1305 mode. */
    struct {
      /* byte counter for AAD. */
      u32 aadcount[2];

      /* byte counter for data. */
      u32 datacount[2];

      unsigned int aad_finalized:1;
      unsigned int bytecount_over_limits:1;

      poly1305_context_t ctx;
    } poly1305;

    /* Mode specific storage for CMAC mode. */
    gcry_cmac_context_t cmac;

    /* Mode specific storage for EAX mode. */
    struct {
      /* CMAC for header (AAD). */
      gcry_cmac_context_t cmac_header;

      /* CMAC for ciphertext. */
      gcry_cmac_context_t cmac_ciphertext;
    } eax;

    /* Mode specific storage for GCM mode and GCM-SIV mode. */
    struct {
      /* The interim tag for GCM mode.  */
      union {
        cipher_context_alignment_t iv_align;
        unsigned char tag[MAX_BLOCKSIZE];
      } u_tag;

      /* Space to save partial input lengths for MAC. */
      unsigned char macbuf[GCRY_CCM_BLOCK_LEN];
      int mac_unused;  /* Number of unprocessed bytes in MACBUF. */

      /* byte counters for GCM */
      u32 aadlen[2];
      u32 datalen[2];

      /* encrypted tag counter */
      unsigned char tagiv[MAX_BLOCKSIZE];

      unsigned int ghash_data_finalized:1;
      unsigned int ghash_aad_finalized:1;

      unsigned int datalen_over_limits:1;
      unsigned int disallow_encryption_because_of_setiv_in_fips_mode:1;

      /* --- Following members are not cleared in gcry_cipher_reset --- */

      /* GHASH multiplier from key.  */
      union {
        cipher_context_alignment_t iv_align;
        unsigned char key[MAX_BLOCKSIZE];
      } u_ghash_key;

      /* Pre-calculated table for GCM. */
#ifdef GCM_USE_TABLES
 #if (SIZEOF_UNSIGNED_LONG == 8 || defined(__x86_64__))
      #define GCM_TABLES_USE_U64 1
      u64 gcm_table[4 * 16];
 #else
      #undef GCM_TABLES_USE_U64
      u32 gcm_table[8 * 16];
 #endif
#endif

      /* GHASH implementation in use. */
      ghash_fn_t ghash_fn;

      /* POLYVAL implementation in use (GCM-SIV). */
      ghash_fn_t polyval_fn;

      /* Key length used for GCM-SIV key generating key. */
      unsigned int siv_keylen;
    } gcm;

    /* Mode specific storage for OCB mode. */
    struct {
      /* --- Following members are not cleared in gcry_cipher_reset --- */

      /* Helper variables and pre-computed table of L values.  */
      unsigned char L_star[OCB_BLOCK_LEN];
      unsigned char L_dollar[OCB_BLOCK_LEN];
      unsigned char L0L1[OCB_BLOCK_LEN];
      unsigned char L[OCB_L_TABLE_SIZE][OCB_BLOCK_LEN];

      /* --- Following members are cleared in gcry_cipher_reset --- */

      /* The tag is valid if marks.tag has been set.  */
      unsigned char tag[OCB_BLOCK_LEN];

      /* A buffer to hold the offset for the AAD processing.  */
      unsigned char aad_offset[OCB_BLOCK_LEN];

      /* A buffer to hold the current sum of AAD processing.  We can't
         use tag here because tag may already hold the preprocessed
         checksum of the data.  */
      unsigned char aad_sum[OCB_BLOCK_LEN];

      /* A buffer to store AAD data not yet processed.  */
      unsigned char aad_leftover[OCB_BLOCK_LEN];

      /* Number of data/aad blocks processed so far.  */
      u64 data_nblocks;
      u64 aad_nblocks;

      /* Number of valid bytes in AAD_LEFTOVER.  */
      unsigned char aad_nleftover;

      /* Length of the tag.  Fixed for now but may eventually be
         specified using a set of gcry_cipher_flags.  */
      unsigned char taglen;

      /* Flags indicating that the final data/aad block has been
         processed.  */
      unsigned int data_finalized:1;
      unsigned int aad_finalized:1;
    } ocb;

    /* Mode specific storage for XTS mode. */
    struct {
      /* Pointer to tweak cipher context, allocated after actual
       * cipher context. */
      char *tweak_context;
    } xts;

    /* Mode specific storage for SIV mode. */
    struct {
      /* Tag used for decryption. */
      unsigned char dec_tag[GCRY_SIV_BLOCK_LEN];

      /* S2V state. */
      unsigned char s2v_d[GCRY_SIV_BLOCK_LEN];

      /* Number of AAD elements processed. */
      unsigned int aad_count:8;

      /* Flags for SIV state. */
      unsigned int dec_tag_set:1;

      /* --- Following members are not cleared in gcry_cipher_reset --- */

      /* S2V CMAC state. */
      gcry_cmac_context_t s2v_cmac;
      unsigned char s2v_zero_block[GCRY_SIV_BLOCK_LEN];

      /* Pointer to CTR cipher context, allocated after actual
       * cipher context. */
      char *ctr_context;
    } siv;

    /* Mode specific storage for WRAP mode. */
    struct {
      unsigned char plen[4];
    } wrap;
  } u_mode;

  /* What follows are two contexts of the cipher in use.  The first
     one needs to be aligned well enough for the cipher operation
     whereas the second one is a copy created by cipher_setkey and
     used by cipher_reset.  That second copy has no need for proper
     aligment because it is only accessed by memcpy.  */
  cipher_context_alignment_t context;
};


/*-- cipher-cbc.c --*/
gcry_err_code_t _gcry_cipher_cbc_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_cbc_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_cbc_cts_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_cbc_cts_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);

/*-- cipher-cfb.c --*/
gcry_err_code_t _gcry_cipher_cfb_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_cfb_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_cfb8_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_cfb8_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);


/*-- cipher-ofb.c --*/
gcry_err_code_t _gcry_cipher_ofb_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);

/*-- cipher-ctr.c --*/
gcry_err_code_t _gcry_cipher_ctr_encrypt_ctx
/*           */ (gcry_cipher_hd_t c,
		 unsigned char *outbuf, size_t outbuflen,
		 const unsigned char *inbuf, size_t inbuflen,
		 void *algo_context);
gcry_err_code_t _gcry_cipher_ctr_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);


/*-- cipher-aeswrap.c --*/
gcry_err_code_t _gcry_cipher_keywrap_encrypt
/*           */   (gcry_cipher_hd_t c,
                   byte *outbuf, size_t outbuflen,
                   const byte *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_keywrap_encrypt_padding
/*           */   (gcry_cipher_hd_t c,
                   byte *outbuf, size_t outbuflen,
                   const byte *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_keywrap_decrypt_auto
/*           */   (gcry_cipher_hd_t c,
                   byte *outbuf, size_t outbuflen,
                   const byte *inbuf, size_t inbuflen);


/*-- cipher-ccm.c --*/
gcry_err_code_t _gcry_cipher_ccm_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_ccm_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_ccm_set_nonce
/*           */ (gcry_cipher_hd_t c, const unsigned char *nonce,
                 size_t noncelen);
gcry_err_code_t _gcry_cipher_ccm_authenticate
/*           */ (gcry_cipher_hd_t c, const unsigned char *abuf, size_t abuflen);
gcry_err_code_t _gcry_cipher_ccm_set_lengths
/*           */ (gcry_cipher_hd_t c, u64 encryptedlen, u64 aadlen, u64 taglen);
gcry_err_code_t _gcry_cipher_ccm_get_tag
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_ccm_check_tag
/*           */ (gcry_cipher_hd_t c,
                 const unsigned char *intag, size_t taglen);


/*-- cipher-cmac.c --*/
gcry_err_code_t _gcry_cmac_generate_subkeys
/*           */ (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx);
gcry_err_code_t _gcry_cmac_write
/*           */ (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx,
		 const byte * inbuf, size_t inlen);
gcry_err_code_t _gcry_cmac_final
/*           */ (gcry_cipher_hd_t c, gcry_cmac_context_t *ctx);
void _gcry_cmac_reset (gcry_cmac_context_t *ctx);


/*-- cipher-eax.c --*/
gcry_err_code_t _gcry_cipher_eax_encrypt
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outbuf, size_t outbuflen,
                   const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_eax_decrypt
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outbuf, size_t outbuflen,
                   const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_eax_set_nonce
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *nonce, size_t noncelen);
gcry_err_code_t _gcry_cipher_eax_authenticate
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *aadbuf, size_t aadbuflen);
gcry_err_code_t _gcry_cipher_eax_get_tag
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_eax_check_tag
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *intag, size_t taglen);
gcry_err_code_t _gcry_cipher_eax_setkey
/*           */   (gcry_cipher_hd_t c);


/*-- cipher-gcm.c --*/
gcry_err_code_t _gcry_cipher_gcm_encrypt
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outbuf, size_t outbuflen,
                   const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_gcm_decrypt
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outbuf, size_t outbuflen,
                   const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_gcm_setiv
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *iv, size_t ivlen);
gcry_err_code_t _gcry_cipher_gcm_authenticate
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *aadbuf, size_t aadbuflen);
gcry_err_code_t _gcry_cipher_gcm_get_tag
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_gcm_check_tag
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *intag, size_t taglen);
void _gcry_cipher_gcm_setkey
/*           */   (gcry_cipher_hd_t c);
void _gcry_cipher_gcm_setupM
/*           */   (gcry_cipher_hd_t c);


/*-- cipher-poly1305.c --*/
gcry_err_code_t _gcry_cipher_poly1305_encrypt
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outbuf, size_t outbuflen,
                   const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_poly1305_decrypt
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outbuf, size_t outbuflen,
                   const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_poly1305_setiv
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *iv, size_t ivlen);
gcry_err_code_t _gcry_cipher_poly1305_authenticate
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *aadbuf, size_t aadbuflen);
gcry_err_code_t _gcry_cipher_poly1305_get_tag
/*           */   (gcry_cipher_hd_t c,
                   unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_poly1305_check_tag
/*           */   (gcry_cipher_hd_t c,
                   const unsigned char *intag, size_t taglen);
void _gcry_cipher_poly1305_setkey
/*           */   (gcry_cipher_hd_t c);


/*-- chacha20.c --*/
gcry_err_code_t _gcry_chacha20_poly1305_encrypt
/*           */   (gcry_cipher_hd_t c, byte *outbuf, const byte *inbuf,
		   size_t length);
gcry_err_code_t _gcry_chacha20_poly1305_decrypt
/*           */   (gcry_cipher_hd_t c, byte *outbuf, const byte *inbuf,
		   size_t length);


/*-- cipher-ocb.c --*/
gcry_err_code_t _gcry_cipher_ocb_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_ocb_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_ocb_set_nonce
/*           */ (gcry_cipher_hd_t c, const unsigned char *nonce,
                 size_t noncelen);
gcry_err_code_t _gcry_cipher_ocb_authenticate
/*           */ (gcry_cipher_hd_t c, const unsigned char *abuf, size_t abuflen);
gcry_err_code_t _gcry_cipher_ocb_get_tag
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_ocb_check_tag
/*           */ (gcry_cipher_hd_t c,
                 const unsigned char *intag, size_t taglen);
void _gcry_cipher_ocb_setkey
/*           */ (gcry_cipher_hd_t c);


/*-- cipher-xts.c --*/
gcry_err_code_t _gcry_cipher_xts_encrypt
/*           */ (gcry_cipher_hd_t c, unsigned char *outbuf, size_t outbuflen,
		 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_xts_decrypt
/*           */ (gcry_cipher_hd_t c, unsigned char *outbuf, size_t outbuflen,
		 const unsigned char *inbuf, size_t inbuflen);


/*-- cipher-siv.c --*/
gcry_err_code_t _gcry_cipher_siv_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_siv_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_siv_set_nonce
/*           */ (gcry_cipher_hd_t c, const unsigned char *nonce,
                 size_t noncelen);
gcry_err_code_t _gcry_cipher_siv_authenticate
/*           */ (gcry_cipher_hd_t c, const unsigned char *abuf, size_t abuflen);
gcry_err_code_t _gcry_cipher_siv_set_decryption_tag
/*           */ (gcry_cipher_hd_t c, const byte *tag, size_t taglen);
gcry_err_code_t _gcry_cipher_siv_get_tag
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_siv_check_tag
/*           */ (gcry_cipher_hd_t c,
                 const unsigned char *intag, size_t taglen);
gcry_err_code_t _gcry_cipher_siv_setkey
/*           */ (gcry_cipher_hd_t c,
                 const unsigned char *ctrkey, size_t ctrkeylen);


/*-- cipher-gcm-siv.c --*/
gcry_err_code_t _gcry_cipher_gcm_siv_encrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_gcm_siv_decrypt
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outbuf, size_t outbuflen,
                 const unsigned char *inbuf, size_t inbuflen);
gcry_err_code_t _gcry_cipher_gcm_siv_set_nonce
/*           */ (gcry_cipher_hd_t c, const unsigned char *nonce,
                 size_t noncelen);
gcry_err_code_t _gcry_cipher_gcm_siv_authenticate
/*           */ (gcry_cipher_hd_t c, const unsigned char *abuf, size_t abuflen);
gcry_err_code_t _gcry_cipher_gcm_siv_set_decryption_tag
/*           */ (gcry_cipher_hd_t c, const byte *tag, size_t taglen);
gcry_err_code_t _gcry_cipher_gcm_siv_get_tag
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_gcm_siv_check_tag
/*           */ (gcry_cipher_hd_t c,
                 const unsigned char *intag, size_t taglen);
gcry_err_code_t _gcry_cipher_gcm_siv_setkey
/*           */ (gcry_cipher_hd_t c, unsigned int keylen);


/* Return the L-value for block N.  Note: 'cipher_ocb.c' ensures that N
 * will never be multiple of 65536 (1 << OCB_L_TABLE_SIZE), thus N can
 * be directly passed to _gcry_ctz() function and resulting index will
 * never overflow the table.  */
static inline const unsigned char *
ocb_get_l (gcry_cipher_hd_t c, u64 n)
{
  unsigned long ntz;

#if ((defined(__i386__) || defined(__x86_64__)) && __GNUC__ >= 4)
  /* Assumes that N != 0. */
  asm ("rep;bsfl %k[low], %k[ntz]\n\t"
        : [ntz] "=r" (ntz)
        : [low] "r" ((unsigned long)n)
        : "cc");
#else
  ntz = _gcry_ctz (n);
#endif

  return c->u_mode.ocb.L[ntz];
}


/* Return bit-shift of blocksize. */
static inline unsigned int _gcry_blocksize_shift(gcry_cipher_hd_t c)
{
  /* Only blocksizes 8 and 16 are used. Return value in such way
   * that compiler can optimize calling functions based on this.  */
  return c->spec->blocksize == 8 ? 3 : 4;
}


/* Optimized function for adding value to cipher block. */
static inline void
cipher_block_add(void *_dstsrc, unsigned int add, size_t blocksize)
{
  byte *dstsrc = _dstsrc;
  u64 s[2];

  if (blocksize == 8)
    {
      buf_put_be64(dstsrc + 0, buf_get_be64(dstsrc + 0) + add);
    }
  else /* blocksize == 16 */
    {
      s[0] = buf_get_be64(dstsrc + 8);
      s[1] = buf_get_be64(dstsrc + 0);
      s[0] += add;
      s[1] += (s[0] < add);
      buf_put_be64(dstsrc + 8, s[0]);
      buf_put_be64(dstsrc + 0, s[1]);
    }
}


/* Optimized function for cipher block copying */
static inline void
cipher_block_cpy(void *_dst, const void *_src, size_t blocksize)
{
  byte *dst = _dst;
  const byte *src = _src;
  u64 s[2];

  if (blocksize == 8)
    {
      buf_put_he64(dst + 0, buf_get_he64(src + 0));
    }
  else /* blocksize == 16 */
    {
      s[0] = buf_get_he64(src + 0);
      s[1] = buf_get_he64(src + 8);
      buf_put_he64(dst + 0, s[0]);
      buf_put_he64(dst + 8, s[1]);
    }
}


/* Optimized function for cipher block xoring */
static inline void
cipher_block_xor(void *_dst, const void *_src1, const void *_src2,
                 size_t blocksize)
{
  byte *dst = _dst;
  const byte *src1 = _src1;
  const byte *src2 = _src2;
  u64 s1[2];
  u64 s2[2];

  if (blocksize == 8)
    {
      buf_put_he64(dst + 0, buf_get_he64(src1 + 0) ^ buf_get_he64(src2 + 0));
    }
  else /* blocksize == 16 */
    {
      s1[0] = buf_get_he64(src1 + 0);
      s1[1] = buf_get_he64(src1 + 8);
      s2[0] = buf_get_he64(src2 + 0);
      s2[1] = buf_get_he64(src2 + 8);
      buf_put_he64(dst + 0, s1[0] ^ s2[0]);
      buf_put_he64(dst + 8, s1[1] ^ s2[1]);
    }
}


/* Optimized function for in-place cipher block xoring */
static inline void
cipher_block_xor_1(void *_dst, const void *_src, size_t blocksize)
{
  cipher_block_xor (_dst, _dst, _src, blocksize);
}


/* Optimized function for cipher block xoring with two destination cipher
   blocks.  Used mainly by CFB mode encryption.  */
static inline void
cipher_block_xor_2dst(void *_dst1, void *_dst2, const void *_src,
                      size_t blocksize)
{
  byte *dst1 = _dst1;
  byte *dst2 = _dst2;
  const byte *src = _src;
  u64 d2[2];
  u64 s[2];

  if (blocksize == 8)
    {
      d2[0] = buf_get_he64(dst2 + 0) ^ buf_get_he64(src + 0);
      buf_put_he64(dst2 + 0, d2[0]);
      buf_put_he64(dst1 + 0, d2[0]);
    }
  else /* blocksize == 16 */
    {
      s[0] = buf_get_he64(src + 0);
      s[1] = buf_get_he64(src + 8);
      d2[0] = buf_get_he64(dst2 + 0);
      d2[1] = buf_get_he64(dst2 + 8);
      d2[0] = d2[0] ^ s[0];
      d2[1] = d2[1] ^ s[1];
      buf_put_he64(dst2 + 0, d2[0]);
      buf_put_he64(dst2 + 8, d2[1]);
      buf_put_he64(dst1 + 0, d2[0]);
      buf_put_he64(dst1 + 8, d2[1]);
    }
}


/* Optimized function for combined cipher block xoring and copying.
   Used by mainly CBC mode decryption.  */
static inline void
cipher_block_xor_n_copy_2(void *_dst_xor, const void *_src_xor,
                          void *_srcdst_cpy, const void *_src_cpy,
                          size_t blocksize)
{
  byte *dst_xor = _dst_xor;
  byte *srcdst_cpy = _srcdst_cpy;
  const byte *src_xor = _src_xor;
  const byte *src_cpy = _src_cpy;
  u64 sc[2];
  u64 sx[2];
  u64 sdc[2];

  if (blocksize == 8)
    {
      sc[0] = buf_get_he64(src_cpy + 0);
      buf_put_he64(dst_xor + 0,
                   buf_get_he64(srcdst_cpy + 0) ^ buf_get_he64(src_xor + 0));
      buf_put_he64(srcdst_cpy + 0, sc[0]);
    }
  else /* blocksize == 16 */
    {
      sc[0] = buf_get_he64(src_cpy + 0);
      sc[1] = buf_get_he64(src_cpy + 8);
      sx[0] = buf_get_he64(src_xor + 0);
      sx[1] = buf_get_he64(src_xor + 8);
      sdc[0] = buf_get_he64(srcdst_cpy + 0);
      sdc[1] = buf_get_he64(srcdst_cpy + 8);
      sx[0] ^= sdc[0];
      sx[1] ^= sdc[1];
      buf_put_he64(dst_xor + 0, sx[0]);
      buf_put_he64(dst_xor + 8, sx[1]);
      buf_put_he64(srcdst_cpy + 0, sc[0]);
      buf_put_he64(srcdst_cpy + 8, sc[1]);
    }
}


/* Optimized function for combined cipher block byte-swapping.  */
static inline void
cipher_block_bswap (void *_dst_bswap, const void *_src_bswap,
                    size_t blocksize)
{
  byte *dst_bswap = _dst_bswap;
  const byte *src_bswap = _src_bswap;
  u64 t[2];

  if (blocksize == 8)
    {
      buf_put_le64(dst_bswap, buf_get_be64(src_bswap));
    }
  else
    {
      t[0] = buf_get_be64(src_bswap + 0);
      t[1] = buf_get_be64(src_bswap + 8);
      buf_put_le64(dst_bswap + 8, t[0]);
      buf_put_le64(dst_bswap + 0, t[1]);
    }
}


/* Optimized function for combined cipher block xoring and copying.
   Used by mainly CFB mode decryption.  */
static inline void
cipher_block_xor_n_copy(void *_dst_xor, void *_srcdst_cpy, const void *_src,
                        size_t blocksize)
{
  cipher_block_xor_n_copy_2(_dst_xor, _src, _srcdst_cpy, _src, blocksize);
}


#endif /*G10_CIPHER_INTERNAL_H*/
