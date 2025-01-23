/* sha1-intel-shaext.S - SHAEXT accelerated SHA-1 transform function
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
    defined(HAVE_GCC_INLINE_ASM_SSE41) && defined(USE_SHA1) && \
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

/*
 * Transform nblks*64 bytes (nblks*16 32-bit words) at DATA.
 */
unsigned int ASM_FUNC_ATTR
_gcry_sha1_transform_intel_shaext(void *state, const unsigned char *data,
                                  size_t nblks)
{
  static const unsigned char be_mask[16] __attribute__ ((aligned (16))) =
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
  char save_buf[2 * 16 + 15];
  char *abcd_save;
  char *e_save;
  shaext_prepare_variable;

  if (nblks == 0)
    return 0;

  shaext_prepare ();

  asm volatile ("" : "=r" (abcd_save) : "0" (save_buf) : "memory");
  abcd_save = abcd_save + (-(uintptr_t)abcd_save & 15);
  e_save = abcd_save + 16;

  /* byteswap mask => XMM7 */
  asm volatile ("movdqa %[mask], %%xmm7\n\t" /* Preload mask */
                :
                : [mask] "m" (*be_mask)
                : "memory");

  /* Load state.. ABCD => XMM4, E => XMM5 */
  asm volatile ("movd 16(%[state]), %%xmm5\n\t"
                "movdqu (%[state]), %%xmm4\n\t"
                "pslldq $12, %%xmm5\n\t"
                "pshufd $0x1b, %%xmm4, %%xmm4\n\t"
                "movdqa %%xmm5, (%[e_save])\n\t"
                "movdqa %%xmm4, (%[abcd_save])\n\t"
                :
                : [state] "r" (state), [abcd_save] "r" (abcd_save),
                  [e_save] "r" (e_save)
                : "memory" );

  /* DATA => XMM[0..4] */
  asm volatile ("movdqu 0(%[data]), %%xmm0\n\t"
                "movdqu 16(%[data]), %%xmm1\n\t"
                "movdqu 32(%[data]), %%xmm2\n\t"
                "movdqu 48(%[data]), %%xmm3\n\t"
                "pshufb %%xmm7, %%xmm0\n\t"
                "pshufb %%xmm7, %%xmm1\n\t"
                "pshufb %%xmm7, %%xmm2\n\t"
                "pshufb %%xmm7, %%xmm3\n\t"
                :
                : [data] "r" (data)
                : "memory" );
  data += 64;

  while (1)
    {
      /* Round 0..3 */
      asm volatile ("paddd %%xmm0, %%xmm5\n\t"
                    "movdqa %%xmm4, %%xmm6\n\t" /* ABCD => E1 */
                    "sha1rnds4 $0, %%xmm5, %%xmm4\n\t"
                    ::: "memory" );

      /* Round 4..7 */
      asm volatile ("sha1nexte %%xmm1, %%xmm6\n\t"
                    "movdqa %%xmm4, %%xmm5\n\t"
                    "sha1rnds4 $0, %%xmm6, %%xmm4\n\t"
                    "sha1msg1 %%xmm1, %%xmm0\n\t"
                    ::: "memory" );

      /* Round 8..11 */
      asm volatile ("sha1nexte %%xmm2, %%xmm5\n\t"
                    "movdqa %%xmm4, %%xmm6\n\t"
                    "sha1rnds4 $0, %%xmm5, %%xmm4\n\t"
                    "sha1msg1 %%xmm2, %%xmm1\n\t"
                    "pxor %%xmm2, %%xmm0\n\t"
                    ::: "memory" );

#define ROUND(imm, E0, E1, MSG0, MSG1, MSG2, MSG3) \
      asm volatile ("sha1nexte %%"MSG0", %%"E0"\n\t" \
                    "movdqa %%xmm4, %%"E1"\n\t" \
                    "sha1msg2 %%"MSG0", %%"MSG1"\n\t" \
                    "sha1rnds4 $"imm", %%"E0", %%xmm4\n\t" \
                    "sha1msg1 %%"MSG0", %%"MSG3"\n\t" \
                    "pxor %%"MSG0", %%"MSG2"\n\t" \
                    ::: "memory" )

      /* Rounds 12..15 to 64..67 */
      ROUND("0", "xmm6", "xmm5", "xmm3", "xmm0", "xmm1", "xmm2");
      ROUND("0", "xmm5", "xmm6", "xmm0", "xmm1", "xmm2", "xmm3");
      ROUND("1", "xmm6", "xmm5", "xmm1", "xmm2", "xmm3", "xmm0");
      ROUND("1", "xmm5", "xmm6", "xmm2", "xmm3", "xmm0", "xmm1");
      ROUND("1", "xmm6", "xmm5", "xmm3", "xmm0", "xmm1", "xmm2");
      ROUND("1", "xmm5", "xmm6", "xmm0", "xmm1", "xmm2", "xmm3");
      ROUND("1", "xmm6", "xmm5", "xmm1", "xmm2", "xmm3", "xmm0");
      ROUND("2", "xmm5", "xmm6", "xmm2", "xmm3", "xmm0", "xmm1");
      ROUND("2", "xmm6", "xmm5", "xmm3", "xmm0", "xmm1", "xmm2");
      ROUND("2", "xmm5", "xmm6", "xmm0", "xmm1", "xmm2", "xmm3");
      ROUND("2", "xmm6", "xmm5", "xmm1", "xmm2", "xmm3", "xmm0");
      ROUND("2", "xmm5", "xmm6", "xmm2", "xmm3", "xmm0", "xmm1");
      ROUND("3", "xmm6", "xmm5", "xmm3", "xmm0", "xmm1", "xmm2");
      ROUND("3", "xmm5", "xmm6", "xmm0", "xmm1", "xmm2", "xmm3");

      if (--nblks == 0)
        break;

      /* Round 68..71 */
      asm volatile ("movdqu 0(%[data]), %%xmm0\n\t"
                    "sha1nexte %%xmm1, %%xmm6\n\t"
                    "movdqa %%xmm4, %%xmm5\n\t"
                    "sha1msg2 %%xmm1, %%xmm2\n\t"
                    "sha1rnds4 $3, %%xmm6, %%xmm4\n\t"
                    "pxor %%xmm1, %%xmm3\n\t"
                    "pshufb %%xmm7, %%xmm0\n\t"
                    :
                    : [data] "r" (data)
                    : "memory" );

      /* Round 72..75 */
      asm volatile ("movdqu 16(%[data]), %%xmm1\n\t"
                    "sha1nexte %%xmm2, %%xmm5\n\t"
                    "movdqa %%xmm4, %%xmm6\n\t"
                    "sha1msg2 %%xmm2, %%xmm3\n\t"
                    "sha1rnds4 $3, %%xmm5, %%xmm4\n\t"
                    "pshufb %%xmm7, %%xmm1\n\t"
                    :
                    : [data] "r" (data)
                    : "memory" );

      /* Round 76..79 */
      asm volatile ("movdqu 32(%[data]), %%xmm2\n\t"
                    "sha1nexte %%xmm3, %%xmm6\n\t"
                    "movdqa %%xmm4, %%xmm5\n\t"
                    "sha1rnds4 $3, %%xmm6, %%xmm4\n\t"
                    "pshufb %%xmm7, %%xmm2\n\t"
                    :
                    : [data] "r" (data)
                    : "memory" );

      /* Merge states, store current. */
      asm volatile ("movdqu 48(%[data]), %%xmm3\n\t"
                    "sha1nexte (%[e_save]), %%xmm5\n\t"
                    "paddd (%[abcd_save]), %%xmm4\n\t"
                    "pshufb %%xmm7, %%xmm3\n\t"
                    "movdqa %%xmm5, (%[e_save])\n\t"
                    "movdqa %%xmm4, (%[abcd_save])\n\t"
                    :
                    : [abcd_save] "r" (abcd_save), [e_save] "r" (e_save),
                      [data] "r" (data)
                    : "memory" );

      data += 64;
    }

  /* Round 68..71 */
  asm volatile ("sha1nexte %%xmm1, %%xmm6\n\t"
                "movdqa %%xmm4, %%xmm5\n\t"
                "sha1msg2 %%xmm1, %%xmm2\n\t"
                "sha1rnds4 $3, %%xmm6, %%xmm4\n\t"
                "pxor %%xmm1, %%xmm3\n\t"
                ::: "memory" );

  /* Round 72..75 */
  asm volatile ("sha1nexte %%xmm2, %%xmm5\n\t"
                "movdqa %%xmm4, %%xmm6\n\t"
                "sha1msg2 %%xmm2, %%xmm3\n\t"
                "sha1rnds4 $3, %%xmm5, %%xmm4\n\t"
                ::: "memory" );

  /* Round 76..79 */
  asm volatile ("sha1nexte %%xmm3, %%xmm6\n\t"
                "movdqa %%xmm4, %%xmm5\n\t"
                "sha1rnds4 $3, %%xmm6, %%xmm4\n\t"
                ::: "memory" );

  /* Merge states. */
  asm volatile ("sha1nexte (%[e_save]), %%xmm5\n\t"
                "paddd (%[abcd_save]), %%xmm4\n\t"
                :
                : [abcd_save] "r" (abcd_save), [e_save] "r" (e_save)
                : "memory" );

  /* Save state */
  asm volatile ("pshufd $0x1b, %%xmm4, %%xmm4\n\t"
                "psrldq $12, %%xmm5\n\t"
                "movdqu %%xmm4, (%[state])\n\t"
                "movd %%xmm5, 16(%[state])\n\t"
                :
                : [state] "r" (state)
                : "memory" );

  shaext_cleanup (abcd_save, e_save);
  return 0;
}

#if __clang__
#  pragma clang attribute pop
#endif

#endif /* HAVE_GCC_INLINE_ASM_SHA_EXT */
