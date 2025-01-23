/* sha256-armv8-aarch64-ce.S - ARM/CE accelerated SHA-256 transform function
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
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO) && defined(USE_SHA256)

.cpu generic+simd+crypto

.text


/* Constants */

.align 4
gcry_sha256_aarch64_ce_K:
.LK:
  .long 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5
  .long 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5
  .long 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3
  .long 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174
  .long 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc
  .long 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da
  .long 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7
  .long 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967
  .long 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13
  .long 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85
  .long 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3
  .long 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070
  .long 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5
  .long 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3
  .long 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208
  .long 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2


/* Register macros */

#define vH0123 v0
#define vH4567 v1

#define vABCD0 v2
#define qABCD0 q2
#define vABCD1 v3
#define qABCD1 q3
#define vEFGH  v4
#define qEFGH  q4

#define vT0 v5
#define vT1 v6

#define vW0 v16
#define vW1 v17
#define vW2 v18
#define vW3 v19

#define vK0 v20
#define vK1 v21
#define vK2 v22
#define vK3 v23


/* Round macros */

#define _(...) /*_*/

#define do_loadk(nk0, nk1) ld1 {nk0.16b-nk1.16b},[x3],#32;
#define do_add(a, b) add a.4s, a.4s, b.4s;
#define do_sha256su0(w0, w1) sha256su0 w0.4s, w1.4s;
#define do_sha256su1(w0, w2, w3) sha256su1 w0.4s, w2.4s, w3.4s;

#define do_rounds(k, nk0, nk1, w0, w1, w2, w3, loadk_fn, add_fn, su0_fn, su1_fn) \
        loadk_fn(   v##nk0, v##nk1     ); \
        su0_fn(     v##w0, v##w1       ); \
        mov         vABCD1.16b, vABCD0.16b; \
        sha256h     qABCD0, qEFGH, v##k.4s; \
        sha256h2    qEFGH, qABCD1, v##k.4s; \
        add_fn(     v##nk0, v##w2      ); \
        su1_fn(     v##w0, v##w2, v##w3   );


/* Other functional macros */

#define CLEAR_REG(reg) movi reg.16b, #0;


/*
 * unsigned int
 * _gcry_sha256_transform_armv8_ce (u32 state[8], const void *input_data,
 *                                  size_t num_blks)
 */
.align 3
.globl _gcry_sha256_transform_armv8_ce
ELF(.type  _gcry_sha256_transform_armv8_ce,%function;)
_gcry_sha256_transform_armv8_ce:
  /* input:
   *	r0: ctx, CTX
   *	r1: data (64*nblks bytes)
   *	r2: nblks
   */
  CFI_STARTPROC();

  cbz x2, .Ldo_nothing;

  GET_DATA_POINTER(x3, .LK);
  mov x4, x3

  ld1 {vH0123.4s-vH4567.4s}, [x0]  /* load state */

  ld1 {vW0.16b-vW1.16b}, [x1], #32
  do_loadk(vK0, vK1)
  ld1 {vW2.16b-vW3.16b}, [x1], #32
  mov vABCD0.16b, vH0123.16b
  mov vEFGH.16b, vH4567.16b

  rev32 vW0.16b, vW0.16b
  rev32 vW1.16b, vW1.16b
  rev32 vW2.16b, vW2.16b
  do_add(vK0, vW0)
  rev32 vW3.16b, vW3.16b
  do_add(vK1, vW1)

.Loop:
  do_rounds(K0, K2, K3, W0, W1, W2, W3, do_loadk, do_add, do_sha256su0, do_sha256su1)
  sub x2,x2,#1
  do_rounds(K1, K3, _ , W1, W2, W3, W0, _       , do_add, do_sha256su0, do_sha256su1)
  do_rounds(K2, K0, K1, W2, W3, W0, W1, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(K3, K1, _ , W3, W0, W1, W2, _       , do_add, do_sha256su0, do_sha256su1)

  do_rounds(K0, K2, K3, W0, W1, W2, W3, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(K1, K3, _ , W1, W2, W3, W0, _       , do_add, do_sha256su0, do_sha256su1)
  do_rounds(K2, K0, K1, W2, W3, W0, W1, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(K3, K1, _ , W3, W0, W1, W2, _       , do_add, do_sha256su0, do_sha256su1)

  do_rounds(K0, K2, K3, W0, W1, W2, W3, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(K1, K3, _ , W1, W2, W3, W0, _       , do_add, do_sha256su0, do_sha256su1)
  do_rounds(K2, K0, K1, W2, W3, W0, W1, do_loadk, do_add, do_sha256su0, do_sha256su1)
  do_rounds(K3, K1, _ , W3, W0, W1, W2, _       , do_add, do_sha256su0, do_sha256su1)

  cbz x2, .Lend

  do_rounds(K0, K2, K3, W0, _  , W2, W3, do_loadk, do_add, _, _)
  ld1 {vW0.16b}, [x1], #16
  mov x3, x4
  do_rounds(K1, K3, _ , W1, _  , W3, _  , _       , do_add, _, _)
  ld1 {vW1.16b}, [x1], #16
  rev32 vW0.16b, vW0.16b
  do_rounds(K2, K0, K1, W2, _  , W0, _  , do_loadk, do_add, _, _)
  rev32 vW1.16b, vW1.16b
  ld1 {vW2.16b}, [x1], #16
  do_rounds(K3, K1, _ , W3, _  , W1, _  , _       , do_add, _, _)
  ld1 {vW3.16b}, [x1], #16

  do_add(vH0123, vABCD0)
  do_add(vH4567, vEFGH)

  rev32 vW2.16b, vW2.16b
  mov vABCD0.16b, vH0123.16b
  rev32 vW3.16b, vW3.16b
  mov vEFGH.16b, vH4567.16b

  b .Loop

.Lend:

  do_rounds(K0, K2, K3, W0, _  , W2, W3, do_loadk, do_add, _, _)
  do_rounds(K1, K3, _ , W1, _  , W3, _  , _       , do_add, _, _)
  do_rounds(K2, _ , _ , W2, _  , _  , _  , _       , _, _, _)
  do_rounds(K3, _ , _ , W3, _  , _  , _  , _       , _, _, _)

  CLEAR_REG(vW0)
  CLEAR_REG(vW1)
  CLEAR_REG(vW2)
  CLEAR_REG(vW3)
  CLEAR_REG(vK0)
  CLEAR_REG(vK1)
  CLEAR_REG(vK2)
  CLEAR_REG(vK3)

  do_add(vH0123, vABCD0)
  do_add(vH4567, vEFGH)

  CLEAR_REG(vABCD0)
  CLEAR_REG(vABCD1)
  CLEAR_REG(vEFGH)

  st1 {vH0123.4s-vH4567.4s}, [x0] /* store state */

  CLEAR_REG(vH0123)
  CLEAR_REG(vH4567)

.Ldo_nothing:
  mov x0, #0
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_sha256_transform_armv8_ce,.-_gcry_sha256_transform_armv8_ce;)

#endif
