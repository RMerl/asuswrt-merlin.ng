/* sha256-intel-shaext.S - SHAEXT accelerated SHA-256 transform function
 * Copyright (C) 2018 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include <config.h>

#include "types.h"

#if defined(HAVE_GCC_INLINE_ASM_SHAEXT) && \
    defined(HAVE_GCC_INLINE_ASM_SSE41) && defined(USE_SHA256) && \
    defined(ENABLE_SHAEXT_SUPPORT)

#if _GCRY_GCC_VERSION >= 40400 /* 4.4 */
/* Prevent compiler from issuing SSE instructions between asm blocks. */
#  pragma GCC target("no-sse")
#endif
#if __clang__
#  pragma clang attribute push (__attribute__((target("no-sse"))), apply_to = function)
#endif

#define NO_INSTRUMENT_FUNCTION __attribute__((no_instrument_function))

#define ASM_FUNC_ATTR NO_INSTRUMENT_FUNCTION

/* Two macros to be called prior and after the use of SHA-EXT
   instructions.  There should be no external function calls between
   the use of these macros.  There purpose is to make sure that the
   SSE regsiters are cleared and won't reveal any information about
   the key or the data.  */
#ifdef __WIN64__
/* XMM6-XMM15 are callee-saved registers on WIN64. */
# define shaext_prepare_variable char win64tmp[2*16]
# define shaext_prepare_variable_size sizeof(win64tmp)
# define shaext_prepare()                                               \
   do { asm volatile ("movdqu %%xmm6, (%0)\n"                           \
                      "movdqu %%xmm7, (%1)\n"                           \
                      :                                                 \
                      : "r" (&win64tmp[0]), "r" (&win64tmp[16])         \
                      : "memory");                                      \
   } while (0)
# define shaext_cleanup(tmp0,tmp1)                                      \
   do { asm volatile ("movdqu (%0), %%xmm6\n"                           \
                      "movdqu (%1), %%xmm7\n"                           \
                      "pxor %%xmm0, %%xmm0\n"                           \
                      "pxor %%xmm1, %%xmm1\n"                           \
                      "pxor %%xmm2, %%xmm2\n"                           \
                      "pxor %%xmm3, %%xmm3\n"                           \
                      "pxor %%xmm4, %%xmm4\n"                           \
                      "pxor %%xmm5, %%xmm5\n"                           \
                      "movdqa %%xmm0, (%2)\n\t"                         \
                      "movdqa %%xmm0, (%3)\n\t"                         \
                      :                                                 \
                      : "r" (&win64tmp[0]), "r" (&win64tmp[16]),        \
                        "r" (tmp0), "r" (tmp1)                          \
                      : "memory");                                      \
   } while (0)
#else
# define shaext_prepare_variable
# define shaext_prepare_variable_size 0
# define shaext_prepare() do { } while (0)
# define shaext_cleanup(tmp0,tmp1)                                      \
   do { asm volatile ("pxor %%xmm0, %%xmm0\n"                           \
                      "pxor %%xmm1, %%xmm1\n"                           \
                      "pxor %%xmm2, %%xmm2\n"                           \
                      "pxor %%xmm3, %%xmm3\n"                           \
                      "pxor %%xmm4, %%xmm4\n"                           \
                      "pxor %%xmm5, %%xmm5\n"                           \
                      "pxor %%xmm6, %%xmm6\n"                           \
                      "pxor %%xmm7, %%xmm7\n"                           \
                      "movdqa %%xmm0, (%0)\n\t"                         \
                      "movdqa %%xmm0, (%1)\n\t"                         \
                      :                                                 \
                      : "r" (tmp0), "r" (tmp1)                          \
                      : "memory");                                      \
   } while (0)
#endif

typedef struct u128_s
{
  u32 a, b, c, d;
} u128_t;

/*
 * Transform nblks*64 bytes (nblks*16 32-bit words) at DATA.
 */
unsigned int ASM_FUNC_ATTR
_gcry_sha256_transform_intel_shaext(u32 state[8], const unsigned char *data,
                                    size_t nblks)
{
  static const unsigned char bshuf_mask[16] __attribute__ ((aligned (16))) =
    { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12 };
  static const u128_t K[16] __attribute__ ((aligned (16))) =
  {
    { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5 },
    { 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5 },
    { 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3 },
    { 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174 },
    { 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc },
    { 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da },
    { 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7 },
    { 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967 },
    { 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13 },
    { 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85 },
    { 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3 },
    { 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070 },
    { 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5 },
    { 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3 },
    { 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208 },
    { 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 }
  };
  char save_buf[2 * 16 + 15];
  char *abef_save;
  char *cdgh_save;
  shaext_prepare_variable;

  if (nblks == 0)
    return 0;

  shaext_prepare ();

  asm volatile ("" : "=r" (abef_save) : "0" (save_buf) : "memory");
  abef_save = abef_save + (-(uintptr_t)abef_save & 15);
  cdgh_save = abef_save + 16;

  /* byteswap mask => XMM7 */
  asm volatile ("movdqa %[mask], %%xmm7\n\t" /* Preload mask */
                :
                : [mask] "m" (*bshuf_mask)
                : "memory");

  /* Load state.. ABEF_SAVE => STATE0 XMM1, CDGH_STATE => STATE1 XMM2 */
  asm volatile ("movups 16(%[state]), %%xmm1\n\t" /* HGFE (xmm=EFGH) */
                "movups  0(%[state]), %%xmm0\n\t" /* DCBA (xmm=ABCD) */
                "movaps %%xmm1, %%xmm2\n\t"
                "shufps $0x11, %%xmm0, %%xmm1\n\t" /* ABEF (xmm=FEBA) */
                "shufps $0xbb, %%xmm0, %%xmm2\n\t" /* CDGH (xmm=HGDC) */
                :
                : [state] "r" (state)
                : "memory" );

  /* Load message */
  asm volatile ("movdqu 0*16(%[data]), %%xmm3\n\t"
                "movdqu 1*16(%[data]), %%xmm4\n\t"
                "movdqu 2*16(%[data]), %%xmm5\n\t"
                "movdqu 3*16(%[data]), %%xmm6\n\t"
                "pshufb %%xmm7, %%xmm3\n\t"
                "pshufb %%xmm7, %%xmm4\n\t"
                "pshufb %%xmm7, %%xmm5\n\t"
                "pshufb %%xmm7, %%xmm6\n\t"
                :
                : [data] "r" (data)
                : "memory" );
  data += 64;

  do
    {
      /* Save state */
      asm volatile ("movdqa %%xmm1, (%[abef_save])\n\t"
                    "movdqa %%xmm2, (%[cdgh_save])\n\t"
                    :
                    : [abef_save] "r" (abef_save), [cdgh_save] "r" (cdgh_save)
                    : "memory" );

      /* Round 0..3 */
      asm volatile ("movdqa %%xmm3, %%xmm0\n\t"
                      "paddd %[constants], %%xmm0\n\t"
                      "sha256rnds2 %%xmm1, %%xmm2\n\t"
                      "psrldq $8, %%xmm0\n\t"
                      "sha256rnds2 %%xmm2, %%xmm1\n\t"
                    :
                    : [constants] "m" (K[0].a)
                    : "memory" );

      /* Round 4..7 */
      asm volatile ("movdqa %%xmm4, %%xmm0\n\t"
                      "paddd %[constants], %%xmm0\n\t"
                      "sha256rnds2 %%xmm1, %%xmm2\n\t"
                      "psrldq $8, %%xmm0\n\t"
                      "sha256rnds2 %%xmm2, %%xmm1\n\t"
                    "sha256msg1 %%xmm4, %%xmm3\n\t"
                    :
                    : [constants] "m" (K[1].a)
                    : "memory" );

      /* Round 8..11 */
      asm volatile ("movdqa %%xmm5, %%xmm0\n\t"
                      "paddd %[constants], %%xmm0\n\t"
                      "sha256rnds2 %%xmm1, %%xmm2\n\t"
                      "psrldq $8, %%xmm0\n\t"
                      "sha256rnds2 %%xmm2, %%xmm1\n\t"
                    "sha256msg1 %%xmm5, %%xmm4\n\t"
                    :
                    : [constants] "m" (K[2].a)
                    : "memory" );

#define ROUND(k, MSG0, MSG1, MSG2, MSG3) \
      asm volatile ("movdqa %%"MSG0", %%xmm0\n\t" \
                      "paddd %[constants], %%xmm0\n\t" \
                      "sha256rnds2 %%xmm1, %%xmm2\n\t" \
                    "movdqa %%"MSG0", %%xmm7\n\t" \
                    "palignr $4, %%"MSG3", %%xmm7\n\t" \
                    "paddd %%xmm7, %%"MSG1"\n\t" \
                    "sha256msg2 %%"MSG0", %%"MSG1"\n\t" \
                      "psrldq $8, %%xmm0\n\t" \
                      "sha256rnds2 %%xmm2, %%xmm1\n\t" \
                    "sha256msg1 %%"MSG0", %%"MSG3"\n\t" \
                    : \
                    : [constants] "m" (K[k].a) \
                    : "memory" )

      /* Rounds 12..15 to 48..51 */
      ROUND(3, "xmm6", "xmm3", "xmm4", "xmm5");
      ROUND(4, "xmm3", "xmm4", "xmm5", "xmm6");
      ROUND(5, "xmm4", "xmm5", "xmm6", "xmm3");
      ROUND(6, "xmm5", "xmm6", "xmm3", "xmm4");
      ROUND(7, "xmm6", "xmm3", "xmm4", "xmm5");
      ROUND(8, "xmm3", "xmm4", "xmm5", "xmm6");
      ROUND(9, "xmm4", "xmm5", "xmm6", "xmm3");
      ROUND(10, "xmm5", "xmm6", "xmm3", "xmm4");
      ROUND(11, "xmm6", "xmm3", "xmm4", "xmm5");
      ROUND(12, "xmm3", "xmm4", "xmm5", "xmm6");

      if (--nblks == 0)
        break;

      /* Round 52..55 */
      asm volatile ("movdqa %%xmm4, %%xmm0\n\t"
                      "paddd %[constants], %%xmm0\n\t"
                      "sha256rnds2 %%xmm1, %%xmm2\n\t"
                    "movdqa %%xmm4, %%xmm7\n\t"
                    "palignr $4, %%xmm3, %%xmm7\n\t"
                    "movdqu 0*16(%[data]), %%xmm3\n\t"
                    "paddd %%xmm7, %%xmm5\n\t"
                    "sha256msg2 %%xmm4, %%xmm5\n\t"
                      "psrldq $8, %%xmm0\n\t"
                      "sha256rnds2 %%xmm2, %%xmm1\n\t"
                    :
                    : [constants] "m" (K[13].a), [data] "r" (data)
                    : "memory" );

      /* Round 56..59 */
      asm volatile ("movdqa %%xmm5, %%xmm0\n\t"
                      "paddd %[constants], %%xmm0\n\t"
                      "sha256rnds2 %%xmm1, %%xmm2\n\t"
                    "movdqa %%xmm5, %%xmm7\n\t"
                    "palignr $4, %%xmm4, %%xmm7\n\t"
                    "movdqu 1*16(%[data]), %%xmm4\n\t"
                    "paddd %%xmm7, %%xmm6\n\t"
                    "movdqa %[mask], %%xmm7\n\t" /* Reload mask */
                    "sha256msg2 %%xmm5, %%xmm6\n\t"
                    "movdqu 2*16(%[data]), %%xmm5\n\t"
                      "psrldq $8, %%xmm0\n\t"
                      "sha256rnds2 %%xmm2, %%xmm1\n\t"
                    :
                    : [constants] "m" (K[14].a), [mask] "m" (*bshuf_mask),
                      [data] "r" (data)
                    : "memory" );

      /* Round 60..63 */
      asm volatile ("movdqa %%xmm6, %%xmm0\n\t"
                    "pshufb %%xmm7, %%xmm3\n\t"
                    "movdqu 3*16(%[data]), %%xmm6\n\t"
                      "paddd %[constants], %%xmm0\n\t"
                    "pshufb %%xmm7, %%xmm4\n\t"
                      "sha256rnds2 %%xmm1, %%xmm2\n\t"
                      "psrldq $8, %%xmm0\n\t"
                    "pshufb %%xmm7, %%xmm5\n\t"
                      "sha256rnds2 %%xmm2, %%xmm1\n\t"
                    :
                    : [constants] "m" (K[15].a), [data] "r" (data)
                    : "memory" );
      data += 64;

      /* Merge states */
      asm volatile ("paddd (%[abef_save]), %%xmm1\n\t"
                    "paddd (%[cdgh_save]), %%xmm2\n\t"
                    "pshufb %%xmm7, %%xmm6\n\t"
                    :
                    : [abef_save] "r" (abef_save), [cdgh_save] "r" (cdgh_save)
                    : "memory" );
    }
  while (1);

  /* Round 52..55 */
  asm volatile ("movdqa %%xmm4, %%xmm0\n\t"
                  "paddd %[constants], %%xmm0\n\t"
                  "sha256rnds2 %%xmm1, %%xmm2\n\t"
                "movdqa %%xmm4, %%xmm7\n\t"
                "palignr $4, %%xmm3, %%xmm7\n\t"
                "paddd %%xmm7, %%xmm5\n\t"
                "sha256msg2 %%xmm4, %%xmm5\n\t"
                  "psrldq $8, %%xmm0\n\t"
                  "sha256rnds2 %%xmm2, %%xmm1\n\t"
                :
                : [constants] "m" (K[13].a)
                : "memory" );

  /* Round 56..59 */
  asm volatile ("movdqa %%xmm5, %%xmm0\n\t"
                  "paddd %[constants], %%xmm0\n\t"
                  "sha256rnds2 %%xmm1, %%xmm2\n\t"
                "movdqa %%xmm5, %%xmm7\n\t"
                "palignr $4, %%xmm4, %%xmm7\n\t"
                "paddd %%xmm7, %%xmm6\n\t"
                "movdqa %[mask], %%xmm7\n\t" /* Reload mask */
                "sha256msg2 %%xmm5, %%xmm6\n\t"
                  "psrldq $8, %%xmm0\n\t"
                  "sha256rnds2 %%xmm2, %%xmm1\n\t"
                :
                : [constants] "m" (K[14].a), [mask] "m" (*bshuf_mask)
                : "memory" );

  /* Round 60..63 */
  asm volatile ("movdqa %%xmm6, %%xmm0\n\t"
                  "paddd %[constants], %%xmm0\n\t"
                  "sha256rnds2 %%xmm1, %%xmm2\n\t"
                  "psrldq $8, %%xmm0\n\t"
                  "sha256rnds2 %%xmm2, %%xmm1\n\t"
                :
                : [constants] "m" (K[15].a)
                : "memory" );

  /* Merge states */
  asm volatile ("paddd (%[abef_save]), %%xmm1\n\t"
                "paddd (%[cdgh_save]), %%xmm2\n\t"
                :
                : [abef_save] "r" (abef_save), [cdgh_save] "r" (cdgh_save)
                : "memory" );

  /* Save state (XMM1=FEBA, XMM2=HGDC) */
  asm volatile ("movaps %%xmm1, %%xmm0\n\t"
                "shufps $0x11, %%xmm2, %%xmm1\n\t" /* xmm=ABCD */
                "shufps $0xbb, %%xmm2, %%xmm0\n\t" /* xmm=EFGH */
                "movups %%xmm1, 16(%[state])\n\t"
                "movups %%xmm0,  0(%[state])\n\t"
                :
                : [state] "r" (state)
                : "memory" );

  shaext_cleanup (abef_save, cdgh_save);
  return 0;
}

#if __clang__
#  pragma clang attribute pop
#endif

#endif /* HAVE_GCC_INLINE_ASM_SHA_EXT */
