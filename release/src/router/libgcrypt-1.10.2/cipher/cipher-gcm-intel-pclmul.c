/* cipher-gcm-intel-pclmul.c  -  Intel PCLMUL accelerated Galois Counter Mode
 *                               implementation
 * Copyright (C) 2013-2014,2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "./cipher-internal.h"


#ifdef GCM_USE_INTEL_PCLMUL


#if _GCRY_GCC_VERSION >= 40400 /* 4.4 */
/* Prevent compiler from issuing SSE instructions between asm blocks. */
#  pragma GCC target("no-sse")
#endif
#if __clang__
#  pragma clang attribute push (__attribute__((target("no-sse"))), apply_to = function)
#endif


#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR        NO_INSTRUMENT_FUNCTION
#define ASM_FUNC_ATTR_INLINE ASM_FUNC_ATTR ALWAYS_INLINE


/*
 Intel PCLMUL ghash based on white paper:
  "Intel® Carry-Less Multiplication Instruction and its Usage for Computing the
   GCM Mode - Rev 2.01"; Shay Gueron, Michael E. Kounavis.
 */
static ASM_FUNC_ATTR_INLINE void reduction(void)
{
  /* input: <xmm1:xmm3> */

  asm volatile (/* first phase of the reduction */
                "movdqa %%xmm3, %%xmm6\n\t"
                "movdqa %%xmm3, %%xmm5\n\t"
                "psllq $1, %%xmm6\n\t"  /* packed right shifting << 63 */
                "pxor %%xmm3, %%xmm6\n\t"
                "psllq $57, %%xmm5\n\t"  /* packed right shifting << 57 */
                "psllq $62, %%xmm6\n\t"  /* packed right shifting << 62 */
                "pxor %%xmm5, %%xmm6\n\t" /* xor the shifted versions */
                "pshufd $0x6a, %%xmm6, %%xmm5\n\t"
                "pshufd $0xae, %%xmm6, %%xmm6\n\t"
                "pxor %%xmm5, %%xmm3\n\t" /* first phase of the reduction
                                             complete */

                /* second phase of the reduction */
                "pxor %%xmm3, %%xmm1\n\t" /* xor the shifted versions */
                "psrlq $1, %%xmm3\n\t"    /* packed left shifting >> 1 */
                "pxor %%xmm3, %%xmm6\n\t"
                "psrlq $1, %%xmm3\n\t"    /* packed left shifting >> 2 */
                "pxor %%xmm3, %%xmm1\n\t"
                "psrlq $5, %%xmm3\n\t"    /* packed left shifting >> 7 */
                "pxor %%xmm3, %%xmm6\n\t"
                "pxor %%xmm6, %%xmm1\n\t" /* the result is in xmm1 */
                ::: "memory" );
}

static ASM_FUNC_ATTR_INLINE void gfmul_pclmul(void)
{
  /* Input: XMM0 and XMM1, Output: XMM1. Input XMM0 stays unmodified.
     Input must be converted to little-endian.
   */
  asm volatile (/* gfmul, xmm0 has operator a and xmm1 has operator b. */
                "pshufd $78, %%xmm0, %%xmm2\n\t"
                "pshufd $78, %%xmm1, %%xmm4\n\t"
                "pxor %%xmm0, %%xmm2\n\t" /* xmm2 holds a0+a1 */
                "pxor %%xmm1, %%xmm4\n\t" /* xmm4 holds b0+b1 */

                "movdqa %%xmm0, %%xmm3\n\t"
                "pclmulqdq $0, %%xmm1, %%xmm3\n\t"  /* xmm3 holds a0*b0 */
                "pclmulqdq $17, %%xmm0, %%xmm1\n\t" /* xmm6 holds a1*b1 */
                "movdqa %%xmm3, %%xmm5\n\t"
                "pclmulqdq $0, %%xmm2, %%xmm4\n\t"  /* xmm4 holds (a0+a1)*(b0+b1) */

                "pxor %%xmm1, %%xmm5\n\t" /* xmm5 holds a0*b0+a1*b1 */
                "pxor %%xmm5, %%xmm4\n\t" /* xmm4 holds a0*b0+a1*b1+(a0+a1)*(b0+b1) */
                "movdqa %%xmm4, %%xmm5\n\t"
                "psrldq $8, %%xmm4\n\t"
                "pslldq $8, %%xmm5\n\t"
                "pxor %%xmm5, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm1\n\t" /* <xmm1:xmm3> holds the result of the
                                             carry-less multiplication of xmm0
                                             by xmm1 */
                ::: "memory" );

  reduction();
}

#define GFMUL_AGGR4_ASM_1(be_to_le)                                            \
    /* perform clmul and merge results... */                                   \
    "movdqu 2*16(%[h_table]), %%xmm2\n\t" /* Load H4 */                        \
    "movdqu 0*16(%[buf]), %%xmm5\n\t"                                          \
    be_to_le("pshufb %[be_mask], %%xmm5\n\t") /* be => le */                   \
    "pxor %%xmm5, %%xmm1\n\t"                                                  \
                                                                               \
    "pshufd $78, %%xmm2, %%xmm5\n\t"                                           \
    "pshufd $78, %%xmm1, %%xmm4\n\t"                                           \
    "pxor %%xmm2, %%xmm5\n\t" /* xmm5 holds 4:a0+a1 */                         \
    "pxor %%xmm1, %%xmm4\n\t" /* xmm4 holds 4:b0+b1 */                         \
    "movdqa %%xmm2, %%xmm3\n\t"                                                \
    "pclmulqdq $0, %%xmm1, %%xmm3\n\t"   /* xmm3 holds 4:a0*b0 */              \
    "pclmulqdq $17, %%xmm2, %%xmm1\n\t"  /* xmm1 holds 4:a1*b1 */              \
    "pclmulqdq $0, %%xmm5, %%xmm4\n\t"   /* xmm4 holds 4:(a0+a1)*(b0+b1) */    \
                                                                               \
    "movdqu 1*16(%[h_table]), %%xmm5\n\t" /* Load H3 */                        \
    "movdqu 1*16(%[buf]), %%xmm2\n\t"                                          \
    be_to_le("pshufb %[be_mask], %%xmm2\n\t") /* be => le */                   \
                                                                               \
    "pshufd $78, %%xmm5, %%xmm0\n\t"                                           \
    "pshufd $78, %%xmm2, %%xmm7\n\t"                                           \
    "pxor %%xmm5, %%xmm0\n\t" /* xmm0 holds 3:a0+a1 */                         \
    "pxor %%xmm2, %%xmm7\n\t" /* xmm7 holds 3:b0+b1 */                         \
    "movdqa %%xmm5, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm2, %%xmm6\n\t"  /* xmm6 holds 3:a0*b0 */               \
    "pclmulqdq $17, %%xmm5, %%xmm2\n\t" /* xmm2 holds 3:a1*b1 */               \
    "pclmulqdq $0, %%xmm0, %%xmm7\n\t" /* xmm7 holds 3:(a0+a1)*(b0+b1) */      \
                                                                               \
    "movdqu 2*16(%[buf]), %%xmm5\n\t"                                          \
    be_to_le("pshufb %[be_mask], %%xmm5\n\t") /* be => le */                   \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 3+4:a0*b0 */                       \
    "pxor %%xmm2, %%xmm1\n\t" /* xmm1 holds 3+4:a1*b1 */                       \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 3+4:(a0+a1)*(b0+b1) */             \
                                                                               \
    "movdqu 0*16(%[h_table]), %%xmm2\n\t" /* Load H2 */                        \
                                                                               \
    "pshufd $78, %%xmm2, %%xmm0\n\t"                                           \
    "pshufd $78, %%xmm5, %%xmm7\n\t"                                           \
    "pxor %%xmm2, %%xmm0\n\t" /* xmm0 holds 2:a0+a1 */                         \
    "pxor %%xmm5, %%xmm7\n\t" /* xmm7 holds 2:b0+b1 */                         \
    "movdqa %%xmm2, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm5, %%xmm6\n\t"  /* xmm6 holds 2:a0*b0 */               \
    "pclmulqdq $17, %%xmm2, %%xmm5\n\t" /* xmm5 holds 2:a1*b1 */               \
    "pclmulqdq $0, %%xmm0, %%xmm7\n\t" /* xmm7 holds 2:(a0+a1)*(b0+b1) */      \
                                                                               \
    "movdqu 3*16(%[buf]), %%xmm2\n\t"                                          \
    be_to_le("pshufb %[be_mask], %%xmm2\n\t") /* be => le */                   \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 2+3+4:a0*b0 */                     \
    "pxor %%xmm5, %%xmm1\n\t" /* xmm1 holds 2+3+4:a1*b1 */                     \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 2+3+4:(a0+a1)*(b0+b1) */

#define GFMUL_AGGR4_ASM_2()                                                    \
    "movdqu %[h_1], %%xmm5\n\t" /* Load H1 */                                  \
                                                                               \
    "pshufd $78, %%xmm5, %%xmm0\n\t"                                           \
    "pshufd $78, %%xmm2, %%xmm7\n\t"                                           \
    "pxor %%xmm5, %%xmm0\n\t" /* xmm0 holds 1:a0+a1 */                         \
    "pxor %%xmm2, %%xmm7\n\t" /* xmm7 holds 1:b0+b1 */                         \
    "movdqa %%xmm5, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm2, %%xmm6\n\t"  /* xmm6 holds 1:a0*b0 */               \
    "pclmulqdq $17, %%xmm5, %%xmm2\n\t" /* xmm2 holds 1:a1*b1 */               \
    "pclmulqdq $0, %%xmm0, %%xmm7\n\t" /* xmm7 holds 1:(a0+a1)*(b0+b1) */      \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 1+2+3+4:a0*b0 */                   \
    "pxor %%xmm2, %%xmm1\n\t" /* xmm1 holds 1+2+3+4:a1*b1 */                   \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 1+2+3+4:(a0+a1)*(b0+b1) */         \
                                                                               \
    /* aggregated reduction... */                                              \
    "movdqa %%xmm3, %%xmm5\n\t"                                                \
    "pxor %%xmm1, %%xmm5\n\t" /* xmm5 holds a0*b0+a1*b1 */                     \
    "pxor %%xmm5, %%xmm4\n\t" /* xmm4 holds a0*b0+a1*b1+(a0+a1)*(b0+b1) */     \
    "movdqa %%xmm4, %%xmm5\n\t"                                                \
    "psrldq $8, %%xmm4\n\t"                                                    \
    "pslldq $8, %%xmm5\n\t"                                                    \
    "pxor %%xmm5, %%xmm3\n\t"                                                  \
    "pxor %%xmm4, %%xmm1\n\t" /* <xmm1:xmm3> holds the result of the           \
                                  carry-less multiplication of xmm0            \
                                  by xmm1 */

#define be_to_le(...) __VA_ARGS__
#define le_to_le(...) /*_*/

static ASM_FUNC_ATTR_INLINE void
gfmul_pclmul_aggr4(const void *buf, const void *h_1, const void *h_table,
		   const unsigned char *be_mask)
{
  /* Input:
      Hash: XMM1
     Output:
      Hash: XMM1
   */
  asm volatile (GFMUL_AGGR4_ASM_1(be_to_le)
                :
                : [buf] "r" (buf),
                  [h_table] "r" (h_table),
                  [be_mask] "m" (*be_mask)
                : "memory" );

  asm volatile (GFMUL_AGGR4_ASM_2()
                :
                : [h_1] "m" (*(const unsigned char *)h_1)
                : "memory" );

  reduction();
}

static ASM_FUNC_ATTR_INLINE void
gfmul_pclmul_aggr4_le(const void *buf, const void *h_1, const void *h_table)
{
  /* Input:
      Hash: XMM1
     Output:
      Hash: XMM1
   */
  asm volatile (GFMUL_AGGR4_ASM_1(le_to_le)
                :
                : [buf] "r" (buf),
                  [h_table] "r" (h_table)
                : "memory" );

  asm volatile (GFMUL_AGGR4_ASM_2()
                :
                : [h_1] "m" (*(const unsigned char *)h_1)
                : "memory" );

  reduction();
}

#ifdef __x86_64__

#define GFMUL_AGGR8_ASM(be_to_le)                                              \
    /* Load H6, H7, H8. */                                                     \
    "movdqu 6*16(%[h_table]), %%xmm10\n\t"                                     \
    "movdqu 5*16(%[h_table]), %%xmm9\n\t"                                      \
    "movdqu 4*16(%[h_table]), %%xmm8\n\t"                                      \
                                                                               \
    /* perform clmul and merge results... */                                   \
    "movdqu 0*16(%[buf]), %%xmm5\n\t"                                          \
    "movdqu 1*16(%[buf]), %%xmm2\n\t"                                          \
    be_to_le("pshufb %%xmm15, %%xmm5\n\t") /* be => le */                      \
    be_to_le("pshufb %%xmm15, %%xmm2\n\t") /* be => le */                      \
    "pxor %%xmm5, %%xmm1\n\t"                                                  \
                                                                               \
    "pshufd $78, %%xmm10, %%xmm5\n\t"                                          \
    "pshufd $78, %%xmm1, %%xmm4\n\t"                                           \
    "pxor %%xmm10, %%xmm5\n\t" /* xmm5 holds 8:a0+a1 */                        \
    "pxor %%xmm1, %%xmm4\n\t"  /* xmm4 holds 8:b0+b1 */                        \
    "movdqa %%xmm10, %%xmm3\n\t"                                               \
    "pclmulqdq $0, %%xmm1, %%xmm3\n\t"   /* xmm3 holds 8:a0*b0 */              \
    "pclmulqdq $17, %%xmm10, %%xmm1\n\t" /* xmm1 holds 8:a1*b1 */              \
    "pclmulqdq $0, %%xmm5, %%xmm4\n\t"   /* xmm4 holds 8:(a0+a1)*(b0+b1) */    \
                                                                               \
    "pshufd $78, %%xmm9, %%xmm11\n\t"                                          \
    "pshufd $78, %%xmm2, %%xmm7\n\t"                                           \
    "pxor %%xmm9, %%xmm11\n\t" /* xmm11 holds 7:a0+a1 */                       \
    "pxor %%xmm2, %%xmm7\n\t"  /* xmm7 holds 7:b0+b1 */                        \
    "movdqa %%xmm9, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm2, %%xmm6\n\t"  /* xmm6 holds 7:a0*b0 */               \
    "pclmulqdq $17, %%xmm9, %%xmm2\n\t" /* xmm2 holds 7:a1*b1 */               \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t" /* xmm7 holds 7:(a0+a1)*(b0+b1) */     \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 7+8:a0*b0 */                       \
    "pxor %%xmm2, %%xmm1\n\t" /* xmm1 holds 7+8:a1*b1 */                       \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 7+8:(a0+a1)*(b0+b1) */             \
                                                                               \
    "movdqu 2*16(%[buf]), %%xmm5\n\t"                                          \
    "movdqu 3*16(%[buf]), %%xmm2\n\t"                                          \
    be_to_le("pshufb %%xmm15, %%xmm5\n\t") /* be => le */                      \
    be_to_le("pshufb %%xmm15, %%xmm2\n\t") /* be => le */                      \
                                                                               \
    "pshufd $78, %%xmm8, %%xmm11\n\t"                                          \
    "pshufd $78, %%xmm5, %%xmm7\n\t"                                           \
    "pxor %%xmm8, %%xmm11\n\t" /* xmm11 holds 6:a0+a1 */                       \
    "pxor %%xmm5, %%xmm7\n\t"  /* xmm7 holds 6:b0+b1 */                        \
    "movdqa %%xmm8, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm5, %%xmm6\n\t"  /* xmm6 holds 6:a0*b0 */               \
    "pclmulqdq $17, %%xmm8, %%xmm5\n\t" /* xmm5 holds 6:a1*b1 */               \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t" /* xmm7 holds 6:(a0+a1)*(b0+b1) */     \
                                                                               \
    /* Load H3, H4, H5. */                                                     \
    "movdqu 3*16(%[h_table]), %%xmm10\n\t"                                     \
    "movdqu 2*16(%[h_table]), %%xmm9\n\t"                                      \
    "movdqu 1*16(%[h_table]), %%xmm8\n\t"                                      \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 6+7+8:a0*b0 */                     \
    "pxor %%xmm5, %%xmm1\n\t" /* xmm1 holds 6+7+8:a1*b1 */                     \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 6+7+8:(a0+a1)*(b0+b1) */           \
                                                                               \
    "pshufd $78, %%xmm10, %%xmm11\n\t"                                         \
    "pshufd $78, %%xmm2, %%xmm7\n\t"                                           \
    "pxor %%xmm10, %%xmm11\n\t" /* xmm11 holds 5:a0+a1 */                      \
    "pxor %%xmm2, %%xmm7\n\t"   /* xmm7 holds 5:b0+b1 */                       \
    "movdqa %%xmm10, %%xmm6\n\t"                                               \
    "pclmulqdq $0, %%xmm2, %%xmm6\n\t"   /* xmm6 holds 5:a0*b0 */              \
    "pclmulqdq $17, %%xmm10, %%xmm2\n\t" /* xmm2 holds 5:a1*b1 */              \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t"  /* xmm7 holds 5:(a0+a1)*(b0+b1) */    \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 5+6+7+8:a0*b0 */                   \
    "pxor %%xmm2, %%xmm1\n\t" /* xmm1 holds 5+6+7+8:a1*b1 */                   \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 5+6+7+8:(a0+a1)*(b0+b1) */         \
                                                                               \
    "movdqu 4*16(%[buf]), %%xmm5\n\t"                                          \
    "movdqu 5*16(%[buf]), %%xmm2\n\t"                                          \
    be_to_le("pshufb %%xmm15, %%xmm5\n\t") /* be => le */                      \
    be_to_le("pshufb %%xmm15, %%xmm2\n\t") /* be => le */                      \
                                                                               \
    "pshufd $78, %%xmm9, %%xmm11\n\t"                                          \
    "pshufd $78, %%xmm5, %%xmm7\n\t"                                           \
    "pxor %%xmm9, %%xmm11\n\t" /* xmm11 holds 4:a0+a1 */                       \
    "pxor %%xmm5, %%xmm7\n\t"  /* xmm7 holds 4:b0+b1 */                        \
    "movdqa %%xmm9, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm5, %%xmm6\n\t"  /* xmm6 holds 4:a0*b0 */               \
    "pclmulqdq $17, %%xmm9, %%xmm5\n\t" /* xmm5 holds 4:a1*b1 */               \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t" /* xmm7 holds 4:(a0+a1)*(b0+b1) */     \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 4+5+6+7+8:a0*b0 */                 \
    "pxor %%xmm5, %%xmm1\n\t" /* xmm1 holds 4+5+6+7+8:a1*b1 */                 \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 4+5+6+7+8:(a0+a1)*(b0+b1) */       \
                                                                               \
    "pshufd $78, %%xmm8, %%xmm11\n\t"                                          \
    "pshufd $78, %%xmm2, %%xmm7\n\t"                                           \
    "pxor %%xmm8, %%xmm11\n\t" /* xmm11 holds 3:a0+a1 */                       \
    "pxor %%xmm2, %%xmm7\n\t"  /* xmm7 holds 3:b0+b1 */                        \
    "movdqa %%xmm8, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm2, %%xmm6\n\t"  /* xmm6 holds 3:a0*b0 */               \
    "pclmulqdq $17, %%xmm8, %%xmm2\n\t" /* xmm2 holds 3:a1*b1 */               \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t" /* xmm7 holds 3:(a0+a1)*(b0+b1) */     \
                                                                               \
    "movdqu 0*16(%[h_table]), %%xmm8\n\t" /* Load H2 */                        \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 3+4+5+6+7+8:a0*b0 */               \
    "pxor %%xmm2, %%xmm1\n\t" /* xmm1 holds 3+4+5+6+7+8:a1*b1 */               \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 3+4+5+6+7+8:(a0+a1)*(b0+b1) */     \
                                                                               \
    "movdqu 6*16(%[buf]), %%xmm5\n\t"                                          \
    "movdqu 7*16(%[buf]), %%xmm2\n\t"                                          \
    be_to_le("pshufb %%xmm15, %%xmm5\n\t") /* be => le */                      \
    be_to_le("pshufb %%xmm15, %%xmm2\n\t") /* be => le */                      \
                                                                               \
    "pshufd $78, %%xmm8, %%xmm11\n\t"                                          \
    "pshufd $78, %%xmm5, %%xmm7\n\t"                                           \
    "pxor %%xmm8, %%xmm11\n\t"  /* xmm11 holds 4:a0+a1 */                      \
    "pxor %%xmm5, %%xmm7\n\t"   /* xmm7 holds 4:b0+b1 */                       \
    "movdqa %%xmm8, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm5, %%xmm6\n\t"   /* xmm6 holds 4:a0*b0 */              \
    "pclmulqdq $17, %%xmm8, %%xmm5\n\t"  /* xmm5 holds 4:a1*b1 */              \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t"  /* xmm7 holds 4:(a0+a1)*(b0+b1) */    \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 2+3+4+5+6+7+8:a0*b0 */             \
    "pxor %%xmm5, %%xmm1\n\t" /* xmm1 holds 2+3+4+5+6+7+8:a1*b1 */             \
    "pxor %%xmm7, %%xmm4\n\t" /* xmm4 holds 2+3+4+5+6+7+8:(a0+a1)*(b0+b1) */   \
                                                                               \
    "pshufd $78, %%xmm0, %%xmm11\n\t"                                          \
    "pshufd $78, %%xmm2, %%xmm7\n\t"                                           \
    "pxor %%xmm0, %%xmm11\n\t" /* xmm11 holds 3:a0+a1 */                       \
    "pxor %%xmm2, %%xmm7\n\t"  /* xmm7 holds 3:b0+b1 */                        \
    "movdqa %%xmm0, %%xmm6\n\t"                                                \
    "pclmulqdq $0, %%xmm2, %%xmm6\n\t"  /* xmm6 holds 3:a0*b0 */               \
    "pclmulqdq $17, %%xmm0, %%xmm2\n\t" /* xmm2 holds 3:a1*b1 */               \
    "pclmulqdq $0, %%xmm11, %%xmm7\n\t" /* xmm7 holds 3:(a0+a1)*(b0+b1) */     \
                                                                               \
    "pxor %%xmm6, %%xmm3\n\t" /* xmm3 holds 1+2+3+3+4+5+6+7+8:a0*b0 */         \
    "pxor %%xmm2, %%xmm1\n\t" /* xmm1 holds 1+2+3+3+4+5+6+7+8:a1*b1 */         \
    "pxor %%xmm7, %%xmm4\n\t"/* xmm4 holds 1+2+3+3+4+5+6+7+8:(a0+a1)*(b0+b1) */\
                                                                               \
    /* aggregated reduction... */                                              \
    "movdqa %%xmm3, %%xmm5\n\t"                                                \
    "pxor %%xmm1, %%xmm5\n\t" /* xmm5 holds a0*b0+a1*b1 */                     \
    "pxor %%xmm5, %%xmm4\n\t" /* xmm4 holds a0*b0+a1*b1+(a0+a1)*(b0+b1) */     \
    "movdqa %%xmm4, %%xmm5\n\t"                                                \
    "psrldq $8, %%xmm4\n\t"                                                    \
    "pslldq $8, %%xmm5\n\t"                                                    \
    "pxor %%xmm5, %%xmm3\n\t"                                                  \
    "pxor %%xmm4, %%xmm1\n\t" /* <xmm1:xmm3> holds the result of the           \
                                  carry-less multiplication of xmm0            \
                                  by xmm1 */

static ASM_FUNC_ATTR_INLINE void
gfmul_pclmul_aggr8(const void *buf, const void *h_table)
{
  /* Input:
      H¹: XMM0
      bemask: XMM15
      Hash: XMM1
     Output:
      Hash: XMM1
     Inputs XMM0 and XMM15 stays unmodified.
   */
  asm volatile (GFMUL_AGGR8_ASM(be_to_le)
                :
                : [buf] "r" (buf),
                  [h_table] "r" (h_table)
                : "memory" );

  reduction();
}

static ASM_FUNC_ATTR_INLINE void
gfmul_pclmul_aggr8_le(const void *buf, const void *h_table)
{
  /* Input:
      H¹: XMM0
      Hash: XMM1
     Output:
      Hash: XMM1
     Inputs XMM0 and XMM15 stays unmodified.
   */
  asm volatile (GFMUL_AGGR8_ASM(le_to_le)
                :
                : [buf] "r" (buf),
                  [h_table] "r" (h_table)
                : "memory" );

  reduction();
}
#endif

static ASM_FUNC_ATTR_INLINE void gcm_lsh(void *h, unsigned int hoffs)
{
  static const u64 pconst[2] __attribute__ ((aligned (16))) =
    { U64_C(0x0000000000000001), U64_C(0xc200000000000000) };

  asm volatile ("movdqu (%[h]), %%xmm2\n\t"
                "pshufd $0xff, %%xmm2, %%xmm3\n\t"
                "movdqa %%xmm2, %%xmm4\n\t"
                "psrad $31, %%xmm3\n\t"
                "pslldq $8, %%xmm4\n\t"
                "pand %[pconst], %%xmm3\n\t"
                "paddq %%xmm2, %%xmm2\n\t"
                "psrlq $63, %%xmm4\n\t"
                "pxor %%xmm3, %%xmm2\n\t"
                "pxor %%xmm4, %%xmm2\n\t"
                "movdqu %%xmm2, (%[h])\n\t"
                :
                : [pconst] "m" (*pconst),
                  [h] "r" ((byte *)h + hoffs)
                : "memory" );
}

void ASM_FUNC_ATTR
_gcry_ghash_setup_intel_pclmul (gcry_cipher_hd_t c)
{
  static const unsigned char be_mask[16] __attribute__ ((aligned (16))) =
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
#if defined(__x86_64__) && defined(__WIN64__)
  char win64tmp[10 * 16];

  /* XMM6-XMM15 need to be restored after use. */
  asm volatile ("movdqu %%xmm6,  0*16(%0)\n\t"
                "movdqu %%xmm7,  1*16(%0)\n\t"
                "movdqu %%xmm8,  2*16(%0)\n\t"
                "movdqu %%xmm9,  3*16(%0)\n\t"
                "movdqu %%xmm10, 4*16(%0)\n\t"
                "movdqu %%xmm11, 5*16(%0)\n\t"
                "movdqu %%xmm12, 6*16(%0)\n\t"
                "movdqu %%xmm13, 7*16(%0)\n\t"
                "movdqu %%xmm14, 8*16(%0)\n\t"
                "movdqu %%xmm15, 9*16(%0)\n\t"
                :
                : "r" (win64tmp)
                : "memory" );
#endif

  /* Swap endianness of hsub. */
  asm volatile ("movdqu (%[key]), %%xmm0\n\t"
                "pshufb %[be_mask], %%xmm0\n\t"
                "movdqu %%xmm0, (%[key])\n\t"
                :
                : [key] "r" (c->u_mode.gcm.u_ghash_key.key),
                  [be_mask] "m" (*be_mask)
                : "memory");

  gcm_lsh(c->u_mode.gcm.u_ghash_key.key, 0); /* H <<< 1 */

  asm volatile ("movdqa %%xmm0, %%xmm1\n\t"
                "movdqu (%[key]), %%xmm0\n\t" /* load H <<< 1 */
                :
                : [key] "r" (c->u_mode.gcm.u_ghash_key.key)
                : "memory");

  gfmul_pclmul (); /* H<<<1•H => H² */

  asm volatile ("movdqu %%xmm1, 0*16(%[h_table])\n\t"
                "movdqa %%xmm1, %%xmm7\n\t"
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table)
                : "memory");

  gcm_lsh(c->u_mode.gcm.gcm_table, 0 * 16); /* H² <<< 1 */
  gfmul_pclmul (); /* H<<<1•H² => H³ */

  asm volatile ("movdqa %%xmm7, %%xmm0\n\t"
                "movdqu %%xmm1, 1*16(%[h_table])\n\t"
                "movdqu 0*16(%[h_table]), %%xmm1\n\t" /* load H² <<< 1 */
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table)
                : "memory");

  gfmul_pclmul (); /* H²<<<1•H² => H⁴ */

  asm volatile ("movdqu %%xmm1, 2*16(%[h_table])\n\t"
                "movdqa %%xmm1, %%xmm0\n\t"
                "movdqu (%[key]), %%xmm1\n\t" /* load H <<< 1 */
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table),
                  [key] "r" (c->u_mode.gcm.u_ghash_key.key)
                : "memory");

  gcm_lsh(c->u_mode.gcm.gcm_table, 1 * 16); /* H³ <<< 1 */
  gcm_lsh(c->u_mode.gcm.gcm_table, 2 * 16); /* H⁴ <<< 1 */

#ifdef __x86_64__
  gfmul_pclmul (); /* H<<<1•H⁴ => H⁵ */

  asm volatile ("movdqu %%xmm1, 3*16(%[h_table])\n\t"
                "movdqu 0*16(%[h_table]), %%xmm1\n\t" /* load H² <<< 1 */
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table)
                : "memory");

  gfmul_pclmul (); /* H²<<<1•H⁴ => H⁶ */

  asm volatile ("movdqu %%xmm1, 4*16(%[h_table])\n\t"
                "movdqu 1*16(%[h_table]), %%xmm1\n\t" /* load H³ <<< 1 */
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table)
                : "memory");

  gfmul_pclmul (); /* H³<<<1•H⁴ => H⁷ */

  asm volatile ("movdqu %%xmm1, 5*16(%[h_table])\n\t"
                "movdqu 2*16(%[h_table]), %%xmm1\n\t" /* load H⁴ <<< 1 */
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table)
                : "memory");

  gfmul_pclmul (); /* H³<<<1•H⁴ => H⁸ */

  asm volatile ("movdqu %%xmm1, 6*16(%[h_table])\n\t"
                :
                : [h_table] "r" (c->u_mode.gcm.gcm_table)
                : "memory");

  gcm_lsh(c->u_mode.gcm.gcm_table, 3 * 16); /* H⁵ <<< 1 */
  gcm_lsh(c->u_mode.gcm.gcm_table, 4 * 16); /* H⁶ <<< 1 */
  gcm_lsh(c->u_mode.gcm.gcm_table, 5 * 16); /* H⁷ <<< 1 */
  gcm_lsh(c->u_mode.gcm.gcm_table, 6 * 16); /* H⁸ <<< 1 */

#ifdef __WIN64__
  /* Clear/restore used registers. */
  asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm4\n\t"
                "pxor %%xmm5, %%xmm5\n\t"
                "movdqu 0*16(%0), %%xmm6\n\t"
                "movdqu 1*16(%0), %%xmm7\n\t"
                "movdqu 2*16(%0), %%xmm8\n\t"
                "movdqu 3*16(%0), %%xmm9\n\t"
                "movdqu 4*16(%0), %%xmm10\n\t"
                "movdqu 5*16(%0), %%xmm11\n\t"
                "movdqu 6*16(%0), %%xmm12\n\t"
                "movdqu 7*16(%0), %%xmm13\n\t"
                "movdqu 8*16(%0), %%xmm14\n\t"
                "movdqu 9*16(%0), %%xmm15\n\t"
                :
                : "r" (win64tmp)
                : "memory" );
#else
  /* Clear used registers. */
  asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm4\n\t"
                "pxor %%xmm5, %%xmm5\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                "pxor %%xmm8, %%xmm8\n\t"
                "pxor %%xmm9, %%xmm9\n\t"
                "pxor %%xmm10, %%xmm10\n\t"
                "pxor %%xmm11, %%xmm11\n\t"
                "pxor %%xmm12, %%xmm12\n\t"
                "pxor %%xmm13, %%xmm13\n\t"
                "pxor %%xmm14, %%xmm14\n\t"
                "pxor %%xmm15, %%xmm15\n\t"
                ::: "memory" );
#endif
#endif
}


unsigned int ASM_FUNC_ATTR
_gcry_ghash_intel_pclmul (gcry_cipher_hd_t c, byte *result, const byte *buf,
                          size_t nblocks)
{
  static const unsigned char be_mask[16] __attribute__ ((aligned (16))) =
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
  const unsigned int blocksize = GCRY_GCM_BLOCK_LEN;
#if defined(__x86_64__) && defined(__WIN64__)
  char win64tmp[10 * 16];
#endif

  if (nblocks == 0)
    return 0;

#if defined(__x86_64__) && defined(__WIN64__)
  /* XMM6-XMM15 need to be restored after use. */
  asm volatile ("movdqu %%xmm6,  0*16(%0)\n\t"
                "movdqu %%xmm7,  1*16(%0)\n\t"
                "movdqu %%xmm8,  2*16(%0)\n\t"
                "movdqu %%xmm9,  3*16(%0)\n\t"
                "movdqu %%xmm10, 4*16(%0)\n\t"
                "movdqu %%xmm11, 5*16(%0)\n\t"
                "movdqu %%xmm12, 6*16(%0)\n\t"
                "movdqu %%xmm13, 7*16(%0)\n\t"
                "movdqu %%xmm14, 8*16(%0)\n\t"
                "movdqu %%xmm15, 9*16(%0)\n\t"
                :
                : "r" (win64tmp)
                : "memory" );
#endif

  /* Preload hash. */
  asm volatile ("movdqa %[be_mask], %%xmm7\n\t"
                "movdqu %[hash], %%xmm1\n\t"
                "pshufb %%xmm7, %%xmm1\n\t" /* be => le */
                :
                : [hash] "m" (*result),
                  [be_mask] "m" (*be_mask)
                : "memory" );

#ifdef __x86_64__
  if (nblocks >= 8)
    {
      /* Preload H1. */
      asm volatile ("movdqa %%xmm7, %%xmm15\n\t"
                    "movdqa %[h_1], %%xmm0\n\t"
                    :
                    : [h_1] "m" (*c->u_mode.gcm.u_ghash_key.key)
                    : "memory" );

      while (nblocks >= 8)
        {
          gfmul_pclmul_aggr8 (buf, c->u_mode.gcm.gcm_table);

          buf += 8 * blocksize;
          nblocks -= 8;
        }
#ifndef __WIN64__
      /* Clear used x86-64/XMM registers. */
      asm volatile( "pxor %%xmm8, %%xmm8\n\t"
                    "pxor %%xmm9, %%xmm9\n\t"
                    "pxor %%xmm10, %%xmm10\n\t"
                    "pxor %%xmm11, %%xmm11\n\t"
                    "pxor %%xmm12, %%xmm12\n\t"
                    "pxor %%xmm13, %%xmm13\n\t"
                    "pxor %%xmm14, %%xmm14\n\t"
                    "pxor %%xmm15, %%xmm15\n\t"
                    ::: "memory" );
#endif
    }
#endif

  while (nblocks >= 4)
    {
      gfmul_pclmul_aggr4 (buf, c->u_mode.gcm.u_ghash_key.key,
                          c->u_mode.gcm.gcm_table, be_mask);

      buf += 4 * blocksize;
      nblocks -= 4;
    }

  if (nblocks)
    {
      /* Preload H1. */
      asm volatile ("movdqa %[h_1], %%xmm0\n\t"
                    :
                    : [h_1] "m" (*c->u_mode.gcm.u_ghash_key.key)
                    : "memory" );

      while (nblocks)
        {
          asm volatile ("movdqu %[buf], %%xmm2\n\t"
                        "pshufb %[be_mask], %%xmm2\n\t" /* be => le */
                        "pxor %%xmm2, %%xmm1\n\t"
                        :
                        : [buf] "m" (*buf), [be_mask] "m" (*be_mask)
                        : "memory" );

          gfmul_pclmul ();

          buf += blocksize;
          nblocks--;
        }
    }

  /* Store hash. */
  asm volatile ("pshufb %[be_mask], %%xmm1\n\t" /* be => le */
                "movdqu %%xmm1, %[hash]\n\t"
                : [hash] "=m" (*result)
                : [be_mask] "m" (*be_mask)
                : "memory" );

#if defined(__x86_64__) && defined(__WIN64__)
  /* Clear/restore used registers. */
  asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm4\n\t"
                "pxor %%xmm5, %%xmm5\n\t"
                "movdqu 0*16(%0), %%xmm6\n\t"
                "movdqu 1*16(%0), %%xmm7\n\t"
                "movdqu 2*16(%0), %%xmm8\n\t"
                "movdqu 3*16(%0), %%xmm9\n\t"
                "movdqu 4*16(%0), %%xmm10\n\t"
                "movdqu 5*16(%0), %%xmm11\n\t"
                "movdqu 6*16(%0), %%xmm12\n\t"
                "movdqu 7*16(%0), %%xmm13\n\t"
                "movdqu 8*16(%0), %%xmm14\n\t"
                "movdqu 9*16(%0), %%xmm15\n\t"
                :
                : "r" (win64tmp)
                : "memory" );
#else
  /* Clear used registers. */
  asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm4\n\t"
                "pxor %%xmm5, %%xmm5\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                ::: "memory" );
#endif

  return 0;
}

unsigned int ASM_FUNC_ATTR
_gcry_polyval_intel_pclmul (gcry_cipher_hd_t c, byte *result, const byte *buf,
                            size_t nblocks)
{
  static const unsigned char be_mask[16] __attribute__ ((aligned (16))) =
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
  const unsigned int blocksize = GCRY_GCM_BLOCK_LEN;
#if defined(__x86_64__) && defined(__WIN64__)
  char win64tmp[10 * 16];
#endif

  if (nblocks == 0)
    return 0;

#if defined(__x86_64__) && defined(__WIN64__)
  /* XMM6-XMM15 need to be restored after use. */
  asm volatile ("movdqu %%xmm6,  0*16(%0)\n\t"
                "movdqu %%xmm7,  1*16(%0)\n\t"
                "movdqu %%xmm8,  2*16(%0)\n\t"
                "movdqu %%xmm9,  3*16(%0)\n\t"
                "movdqu %%xmm10, 4*16(%0)\n\t"
                "movdqu %%xmm11, 5*16(%0)\n\t"
                "movdqu %%xmm12, 6*16(%0)\n\t"
                "movdqu %%xmm13, 7*16(%0)\n\t"
                "movdqu %%xmm14, 8*16(%0)\n\t"
                "movdqu %%xmm15, 9*16(%0)\n\t"
                :
                : "r" (win64tmp)
                : "memory" );
#endif

  /* Preload hash. */
  asm volatile ("pxor %%xmm7, %%xmm7\n\t"
                "movdqu %[hash], %%xmm1\n\t"
                "pshufb %[be_mask], %%xmm1\n\t" /* be => le */
                :
                : [hash] "m" (*result),
                  [be_mask] "m" (*be_mask)
                : "memory" );

#ifdef __x86_64__
  if (nblocks >= 8)
    {
      /* Preload H1. */
      asm volatile ("pxor %%xmm15, %%xmm15\n\t"
                    "movdqa %[h_1], %%xmm0\n\t"
                    :
                    : [h_1] "m" (*c->u_mode.gcm.u_ghash_key.key)
                    : "memory" );

      while (nblocks >= 8)
        {
          gfmul_pclmul_aggr8_le (buf, c->u_mode.gcm.gcm_table);

          buf += 8 * blocksize;
          nblocks -= 8;
        }
#ifndef __WIN64__
      /* Clear used x86-64/XMM registers. */
      asm volatile( "pxor %%xmm8, %%xmm8\n\t"
                    "pxor %%xmm9, %%xmm9\n\t"
                    "pxor %%xmm10, %%xmm10\n\t"
                    "pxor %%xmm11, %%xmm11\n\t"
                    "pxor %%xmm12, %%xmm12\n\t"
                    "pxor %%xmm13, %%xmm13\n\t"
                    "pxor %%xmm14, %%xmm14\n\t"
                    "pxor %%xmm15, %%xmm15\n\t"
                    ::: "memory" );
#endif
    }
#endif

  while (nblocks >= 4)
    {
      gfmul_pclmul_aggr4_le (buf, c->u_mode.gcm.u_ghash_key.key,
                             c->u_mode.gcm.gcm_table);

      buf += 4 * blocksize;
      nblocks -= 4;
    }

  if (nblocks)
    {
      /* Preload H1. */
      asm volatile ("movdqa %[h_1], %%xmm0\n\t"
                    :
                    : [h_1] "m" (*c->u_mode.gcm.u_ghash_key.key)
                    : "memory" );

      while (nblocks)
        {
          asm volatile ("movdqu %[buf], %%xmm2\n\t"
                        "pxor %%xmm2, %%xmm1\n\t"
                        :
                        : [buf] "m" (*buf)
                        : "memory" );

          gfmul_pclmul ();

          buf += blocksize;
          nblocks--;
        }
    }

  /* Store hash. */
  asm volatile ("pshufb %[be_mask], %%xmm1\n\t" /* be => le */
                "movdqu %%xmm1, %[hash]\n\t"
                : [hash] "=m" (*result)
                : [be_mask] "m" (*be_mask)
                : "memory" );

#if defined(__x86_64__) && defined(__WIN64__)
  /* Clear/restore used registers. */
  asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm4\n\t"
                "pxor %%xmm5, %%xmm5\n\t"
                "movdqu 0*16(%0), %%xmm6\n\t"
                "movdqu 1*16(%0), %%xmm7\n\t"
                "movdqu 2*16(%0), %%xmm8\n\t"
                "movdqu 3*16(%0), %%xmm9\n\t"
                "movdqu 4*16(%0), %%xmm10\n\t"
                "movdqu 5*16(%0), %%xmm11\n\t"
                "movdqu 6*16(%0), %%xmm12\n\t"
                "movdqu 7*16(%0), %%xmm13\n\t"
                "movdqu 8*16(%0), %%xmm14\n\t"
                "movdqu 9*16(%0), %%xmm15\n\t"
                :
                : "r" (win64tmp)
                : "memory" );
#else
  /* Clear used registers. */
  asm volatile( "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                "pxor %%xmm3, %%xmm3\n\t"
                "pxor %%xmm4, %%xmm4\n\t"
                "pxor %%xmm5, %%xmm5\n\t"
                "pxor %%xmm6, %%xmm6\n\t"
                "pxor %%xmm7, %%xmm7\n\t"
                ::: "memory" );
#endif

  return 0;
}

#if __clang__
#  pragma clang attribute pop
#endif

#endif /* GCM_USE_INTEL_PCLMUL */
