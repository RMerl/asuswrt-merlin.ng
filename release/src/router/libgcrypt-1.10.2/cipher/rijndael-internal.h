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
 */

#ifndef G10_RIJNDAEL_INTERNAL_H
#define G10_RIJNDAEL_INTERNAL_H

#include "types.h"  /* for byte and u32 typedefs */


#define MAXKC                   (256/32)
#define MAXROUNDS               14
#define BLOCKSIZE               (128/8)


/* Helper macro to force alignment to 16 or 64 bytes.  */
#ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
# define ATTR_ALIGNED_16  __attribute__ ((aligned (16)))
# define ATTR_ALIGNED_64  __attribute__ ((aligned (64)))
#else
# define ATTR_ALIGNED_16
# define ATTR_ALIGNED_64
#endif


/* USE_AMD64_ASM indicates whether to use AMD64 assembly code. */
#undef USE_AMD64_ASM
#if defined(__x86_64__) && (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
    defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
# define USE_AMD64_ASM 1
#endif

/* USE_SSSE3 indicates whether to use SSSE3 code. */
#if defined(__x86_64__) && defined(HAVE_GCC_INLINE_ASM_SSSE3) && \
    (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS))
#  define USE_SSSE3 1
#endif

/* USE_ARM_ASM indicates whether to use ARM assembly code. */
#undef USE_ARM_ASM
#if defined(__ARMEL__)
# ifdef HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS
#  define USE_ARM_ASM 1
# endif
#endif
#if defined(__AARCH64EL__)
# ifdef HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS
#  define USE_ARM_ASM 1
# endif
#endif

/* USE_PADLOCK indicates whether to compile the padlock specific
   code.  */
#undef USE_PADLOCK
#ifdef ENABLE_PADLOCK_SUPPORT
# ifdef HAVE_GCC_ATTRIBUTE_ALIGNED
#  if (defined (__i386__) && SIZEOF_UNSIGNED_LONG == 4) || defined(__x86_64__)
#   define USE_PADLOCK 1
#  endif
# endif
#endif /* ENABLE_PADLOCK_SUPPORT */

/* USE_AESNI inidicates whether to compile with Intel AES-NI code.  We
   need the vector-size attribute which seems to be available since
   gcc 3.  However, to be on the safe side we require at least gcc 4.  */
#undef USE_AESNI
#ifdef ENABLE_AESNI_SUPPORT
# if ((defined (__i386__) && SIZEOF_UNSIGNED_LONG == 4) || defined(__x86_64__))
#  if __GNUC__ >= 4
#   define USE_AESNI 1
#  endif
# endif
#endif /* ENABLE_AESNI_SUPPORT */

/* USE_VAES inidicates whether to compile with Intel VAES code.  */
#undef USE_VAES
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
     defined(__x86_64__) && defined(ENABLE_AVX2_SUPPORT) && \
     defined(HAVE_GCC_INLINE_ASM_VAES_VPCLMUL) && \
     defined(USE_AESNI)
# define USE_VAES 1
#endif

/* USE_ARM_CE indicates whether to enable ARMv8 Crypto Extension assembly
 * code. */
#undef USE_ARM_CE
#ifdef ENABLE_ARM_CRYPTO_SUPPORT
# if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) \
     && defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) \
     && defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO)
#  define USE_ARM_CE 1
# elif defined(__AARCH64EL__) \
       && defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) \
       && defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)
#  define USE_ARM_CE 1
# endif
#endif /* ENABLE_ARM_CRYPTO_SUPPORT */

/* USE_PPC_CRYPTO indicates whether to enable PowerPC vector crypto
 * accelerated code.  USE_PPC_CRYPTO_WITH_PPC9LE indicates whether to
 * enable POWER9 optimized variant.  */
#undef USE_PPC_CRYPTO
#undef USE_PPC_CRYPTO_WITH_PPC9LE
#ifdef ENABLE_PPC_CRYPTO_SUPPORT
# if defined(HAVE_COMPATIBLE_CC_PPC_ALTIVEC) && \
     defined(HAVE_GCC_INLINE_ASM_PPC_ALTIVEC)
#  if __GNUC__ >= 4
#   define USE_PPC_CRYPTO 1
#   if !defined(WORDS_BIGENDIAN) && defined(HAVE_GCC_INLINE_ASM_PPC_ARCH_3_00)
#    define USE_PPC_CRYPTO_WITH_PPC9LE 1
#   endif
#  endif
# endif
#endif /* ENABLE_PPC_CRYPTO_SUPPORT */

/* USE_S390X_CRYPTO indicates whether to enable zSeries code. */
#undef USE_S390X_CRYPTO
#if defined(HAVE_GCC_INLINE_ASM_S390X)
# define USE_S390X_CRYPTO 1
#endif /* USE_S390X_CRYPTO */

struct RIJNDAEL_context_s;

typedef unsigned int (*rijndael_cryptfn_t)(const struct RIJNDAEL_context_s *ctx,
                                           unsigned char *bx,
                                           const unsigned char *ax);
typedef void (*rijndael_prefetchfn_t)(void);
typedef void (*rijndael_prepare_decfn_t)(struct RIJNDAEL_context_s *ctx);

/* Our context object.  */
typedef struct RIJNDAEL_context_s
{
  /* The first fields are the keyschedule arrays.  This is so that
     they are aligned on a 16 byte boundary if using gcc.  This
     alignment is required for the AES-NI code and a good idea in any
     case.  The alignment is guaranteed due to the way cipher.c
     allocates the space for the context.  The PROPERLY_ALIGNED_TYPE
     hack is used to force a minimal alignment if not using gcc of if
     the alignment requirement is higher that 16 bytes.  */
  union
  {
    PROPERLY_ALIGNED_TYPE dummy;
    byte keyschedule[MAXROUNDS+1][4][4];
    u32 keyschedule32[MAXROUNDS+1][4];
#ifdef USE_PADLOCK
    /* The key as passed to the padlock engine.  It is only used if
       the padlock engine is used (USE_PADLOCK, below).  */
    unsigned char padlock_key[16] __attribute__ ((aligned (16)));
#endif /*USE_PADLOCK*/
  } u1;
  union
  {
    PROPERLY_ALIGNED_TYPE dummy;
    byte keyschedule[MAXROUNDS+1][4][4];
    u32 keyschedule32[MAXROUNDS+1][4];
  } u2;
  int rounds;                         /* Key-length-dependent number of rounds.  */
  unsigned int decryption_prepared:1; /* The decryption key schedule is available.  */
#ifdef USE_AESNI
  unsigned int use_avx:1;             /* AVX shall be used by AES-NI implementation. */
  unsigned int use_avx2:1;            /* AVX2 shall be used by AES-NI implementation. */
#endif /*USE_AESNI*/
#ifdef USE_S390X_CRYPTO
  byte km_func;
  byte km_func_xts;
  byte kmc_func;
  byte kmac_func;
  byte kmf_func;
  byte kmo_func;
  byte kma_func;
#endif /*USE_S390X_CRYPTO*/
  rijndael_cryptfn_t encrypt_fn;
  rijndael_cryptfn_t decrypt_fn;
  rijndael_prefetchfn_t prefetch_enc_fn;
  rijndael_prefetchfn_t prefetch_dec_fn;
  rijndael_prepare_decfn_t prepare_decryption;
} RIJNDAEL_context ATTR_ALIGNED_16;

/* Macros defining alias for the keyschedules.  */
#define keyschenc   u1.keyschedule
#define keyschenc32 u1.keyschedule32
#define keyschdec   u2.keyschedule
#define keyschdec32 u2.keyschedule32
#define padlockkey  u1.padlock_key

#endif /* G10_RIJNDAEL_INTERNAL_H */
