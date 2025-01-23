/* sha1-armv8-aarch64-ce.S - ARM/CE accelerated SHA-1 transform function
 * Copyright (C) 2016 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include "asm-common-aarch64.h"

#if defined(__AARCH64EL__) && \
    defined(HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO) && defined(USE_SHA1)

.cpu generic+simd+crypto

.text


/* Constants */

#define K1  0x5A827999
#define K2  0x6ED9EBA1
#define K3  0x8F1BBCDC
#define K4  0xCA62C1D6
.align 4
gcry_sha1_aarch64_ce_K_VEC:
.LK_VEC:
.LK1:	.long K1, K1, K1, K1
.LK2:	.long K2, K2, K2, K2
.LK3:	.long K3, K3, K3, K3
.LK4:	.long K4, K4, K4, K4


/* Register macros */

#define sH4    s0
#define vH4    v0
#define vH0123 v1

#define qABCD q2
#define sABCD s2
#define vABCD v2
#define sE0   s3
#define vE0   v3
#define sE1   s4
#define vE1   v4

#define vT0   v5
#define vT1   v6

#define vW0 v16
#define vW1 v17
#define vW2 v18
#define vW3 v19

#define vK1 v20
#define vK2 v21
#define vK3 v22
#define vK4 v23


/* Round macros */

#define _(...) /*_*/
#define do_add(dst, src0, src1) add dst.4s, src0.4s, src1.4s;
#define do_sha1su0(w0,w1,w2) sha1su0 w0.4s,w1.4s,w2.4s;
#define do_sha1su1(w0,w3) sha1su1 w0.4s,w3.4s;

#define do_rounds(f, e0, e1, t, k, w0, w1, w2, w3, add_fn, sha1su0_fn, sha1su1_fn) \
        sha1su1_fn( v##w3, v##w2     ); \
        sha1h       e0, sABCD; \
        sha1##f     qABCD, e1, v##t.4s; \
        add_fn(     v##t, v##w2, v##k   ); \
        sha1su0_fn( v##w0, v##w1, v##w2 );


/* Other functional macros */

#define CLEAR_REG(reg) movi reg.16b, #0;


/*
 * unsigned int
 * _gcry_sha1_transform_armv8_ce (void *ctx, const unsigned char *data,
 *                                size_t nblks)
 */
.align 3
.globl _gcry_sha1_transform_armv8_ce
ELF(.type  _gcry_sha1_transform_armv8_ce,%function;)
_gcry_sha1_transform_armv8_ce:
  /* input:
   *	x0: ctx, CTX
   *	x1: data (64*nblks bytes)
   *	x2: nblks
   */
  CFI_STARTPROC();

  cbz x2, .Ldo_nothing;

  GET_DATA_POINTER(x4, .LK_VEC);

  ld1 {vH0123.4s}, [x0]     /* load h0,h1,h2,h3 */
  ld1 {vK1.4s-vK4.4s}, [x4] /* load K1,K2,K3,K4 */
  ldr sH4, [x0, #16]        /* load h4 */

  ld1 {vW0.16b-vW3.16b}, [x1], #64
  mov vABCD.16b, vH0123.16b

  rev32 vW0.16b, vW0.16b
  rev32 vW1.16b, vW1.16b
  rev32 vW2.16b, vW2.16b
  do_add(vT0, vW0, vK1)
  rev32 vW3.16b, vW3.16b
  do_add(vT1, vW1, vK1)

.Loop:
  do_rounds(c, sE1, sH4, T0, K1, W0, W1, W2, W3, do_add, do_sha1su0, _)
  sub x2, x2, #1
  do_rounds(c, sE0, sE1, T1, K1, W1, W2, W3, W0, do_add, do_sha1su0, do_sha1su1)
  do_rounds(c, sE1, sE0, T0, K1, W2, W3, W0, W1, do_add, do_sha1su0, do_sha1su1)
  do_rounds(c, sE0, sE1, T1, K2, W3, W0, W1, W2, do_add, do_sha1su0, do_sha1su1)
  do_rounds(c, sE1, sE0, T0, K2, W0, W1, W2, W3, do_add, do_sha1su0, do_sha1su1)

  do_rounds(p, sE0, sE1, T1, K2, W1, W2, W3, W0, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, sE1, sE0, T0, K2, W2, W3, W0, W1, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, sE0, sE1, T1, K2, W3, W0, W1, W2, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, sE1, sE0, T0, K3, W0, W1, W2, W3, do_add, do_sha1su0, do_sha1su1)
  do_rounds(p, sE0, sE1, T1, K3, W1, W2, W3, W0, do_add, do_sha1su0, do_sha1su1)

  do_rounds(m, sE1, sE0, T0, K3, W2, W3, W0, W1, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, sE0, sE1, T1, K3, W3, W0, W1, W2, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, sE1, sE0, T0, K3, W0, W1, W2, W3, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, sE0, sE1, T1, K4, W1, W2, W3, W0, do_add, do_sha1su0, do_sha1su1)
  do_rounds(m, sE1, sE0, T0, K4, W2, W3, W0, W1, do_add, do_sha1su0, do_sha1su1)

  do_rounds(p, sE0, sE1, T1, K4, W3, W0, W1, W2, do_add, do_sha1su0, do_sha1su1)
  cbz x2, .Lend

  ld1 {vW0.16b-vW1.16b}, [x1], #32 /* preload */
  do_rounds(p, sE1, sE0, T0, K4, _  , _  , W2, W3, do_add, _, do_sha1su1)
  rev32 vW0.16b, vW0.16b
  ld1 {vW2.16b}, [x1], #16
  rev32 vW1.16b, vW1.16b
  do_rounds(p, sE0, sE1, T1, K4, _  , _  , W3, _  , do_add, _, _)
  ld1 {vW3.16b}, [x1], #16
  rev32 vW2.16b, vW2.16b
  do_rounds(p, sE1, sE0, T0, _, _, _, _, _, _, _, _)
  rev32 vW3.16b, vW3.16b
  do_rounds(p, sE0, sE1, T1, _, _, _, _, _, _, _, _)

  do_add(vT0, vW0, vK1)
  add vH4.2s, vH4.2s, vE0.2s
  add vABCD.4s, vABCD.4s, vH0123.4s
  do_add(vT1, vW1, vK1)

  mov vH0123.16b, vABCD.16b

  b .Loop

.Lend:
  do_rounds(p, sE1, sE0, T0, K4, _  , _  , W2, W3, do_add, _, do_sha1su1)
  do_rounds(p, sE0, sE1, T1, K4, _  , _  , W3, _  , do_add, _, _)
  do_rounds(p, sE1, sE0, T0, _, _, _, _, _, _, _, _)
  do_rounds(p, sE0, sE1, T1, _, _, _, _, _, _, _, _)

  add vH4.2s, vH4.2s, vE0.2s
  add vH0123.4s, vH0123.4s, vABCD.4s

  CLEAR_REG(vW0)
  CLEAR_REG(vW1)
  CLEAR_REG(vW2)
  CLEAR_REG(vW3)
  CLEAR_REG(vABCD)
  CLEAR_REG(vE1)
  CLEAR_REG(vE0)

  str sH4, [x0, #16]    /* store h4 */
  st1 {vH0123.4s}, [x0] /* store h0,h1,h2,h3 */

  CLEAR_REG(vH0123)
  CLEAR_REG(vH4)

.Ldo_nothing:
  mov x0, #0
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_sha1_transform_armv8_ce,.-_gcry_sha1_transform_armv8_ce;)

#endif
