/* rijndael-armv8-aarch32-ce.S - ARMv8/CE accelerated AES
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

#include <config.h>

#if defined(HAVE_ARM_ARCH_V6) && defined(__ARMEL__) && \
    defined(HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS) && \
    defined(HAVE_GCC_INLINE_ASM_AARCH32_CRYPTO)

.syntax unified
.arch armv8-a
.fpu crypto-neon-fp-armv8
.arm

.text

#ifdef __PIC__
#  define GET_DATA_POINTER(reg, name, rtmp) \
		ldr reg, 1f; \
		ldr rtmp, 2f; \
		b 3f; \
	1:	.word _GLOBAL_OFFSET_TABLE_-(3f+8); \
	2:	.word name(GOT); \
	3:	add reg, pc, reg; \
		ldr reg, [reg, rtmp];
#else
#  define GET_DATA_POINTER(reg, name, rtmp) ldr reg, =name
#endif


/* AES macros */

#define aes_preload_keys(keysched, rekeysched) \
        vldmia   keysched!, {q5-q7}; \
        mov      rekeysched, keysched; \
        vldmialo keysched!, {q8-q15}; /* 128-bit */ \
        addeq    keysched, #(2*16); \
        vldmiaeq keysched!, {q10-q15}; /* 192-bit */ \
        addhi    keysched, #(4*16); \
        vldmiahi keysched!, {q12-q15}; /* 256-bit */ \

#define do_aes_one128(ed, mcimc, qo, qb) \
        aes##ed.8    qb, q5; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q6; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q7; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q8; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q9; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q10; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q11; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q12; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q13; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q14; \
        veor         qo, qb, q15;

#define do_aes_one128re(ed, mcimc, qo, qb, keysched, rekeysched) \
        vldm         rekeysched, {q8-q9}; \
        do_aes_one128(ed, mcimc, qo, qb);

#define do_aes_one192(ed, mcimc, qo, qb, keysched, rekeysched) \
        vldm         rekeysched!, {q8}; \
        aes##ed.8    qb, q5; \
        aes##mcimc.8 qb, qb; \
        vldm         rekeysched, {q9}; \
        aes##ed.8    qb, q6; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q7; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q8; \
        aes##mcimc.8 qb, qb; \
        vldmia       keysched!, {q8}; \
        aes##ed.8    qb, q9; \
        aes##mcimc.8 qb, qb; \
        sub          rekeysched, #(1*16); \
        aes##ed.8    qb, q10; \
        aes##mcimc.8 qb, qb; \
        vldm         keysched, {q9}; \
        aes##ed.8    qb, q11; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q12; \
        aes##mcimc.8 qb, qb; \
        sub          keysched, #16; \
        aes##ed.8    qb, q13; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q14; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q15; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q8; \
        veor         qo, qb, q9; \

#define do_aes_one256(ed, mcimc, qo, qb, keysched, rekeysched) \
        vldmia       rekeysched!, {q8}; \
        aes##ed.8    qb, q5; \
        aes##mcimc.8 qb, qb; \
        vldmia       rekeysched!, {q9}; \
        aes##ed.8    qb, q6; \
        aes##mcimc.8 qb, qb; \
        vldmia       rekeysched!, {q10}; \
        aes##ed.8    qb, q7; \
        aes##mcimc.8 qb, qb; \
        vldm         rekeysched, {q11}; \
        aes##ed.8    qb, q8; \
        aes##mcimc.8 qb, qb; \
        vldmia       keysched!, {q8}; \
        aes##ed.8    qb, q9; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q10; \
        aes##mcimc.8 qb, qb; \
        vldmia       keysched!, {q9}; \
        aes##ed.8    qb, q11; \
        aes##mcimc.8 qb, qb; \
        sub          rekeysched, #(3*16); \
        aes##ed.8    qb, q12; \
        aes##mcimc.8 qb, qb; \
        vldmia       keysched!, {q10}; \
        aes##ed.8    qb, q13; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q14; \
        aes##mcimc.8 qb, qb; \
        vldm         keysched, {q11}; \
        aes##ed.8    qb, q15; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q8; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q9; \
        aes##mcimc.8 qb, qb; \
        aes##ed.8    qb, q10; \
        veor         qo, qb, q11; \
        sub          keysched, #(3*16); \

#define aes_round_4(ed, mcimc, b0, b1, b2, b3, key) \
        aes##ed.8    b0, key; \
        aes##mcimc.8 b0, b0; \
          aes##ed.8    b1, key; \
          aes##mcimc.8 b1, b1; \
            aes##ed.8    b2, key; \
            aes##mcimc.8 b2, b2; \
              aes##ed.8    b3, key; \
              aes##mcimc.8 b3, b3;

#define do_aes_4_128(ed, mcimc, b0, b1, b2, b3) \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q5); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q6); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q7); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q8); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q9); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q10); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q11); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q12); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q13); \
        aes##ed.8    b0, q14; \
        veor         b0, b0, q15; \
        aes##ed.8    b1, q14; \
        veor         b1, b1, q15; \
        aes##ed.8    b2, q14; \
        veor         b2, b2, q15; \
        aes##ed.8    b3, q14; \
        veor         b3, b3, q15;

#define do_aes_4_128re(ed, mcimc, b0, b1, b2, b3, keysched, rekeysched) \
        vldm         rekeysched, {q8-q9}; \
        do_aes_4_128(ed, mcimc, b0, b1, b2, b3);

#define do_aes_4_192(ed, mcimc, b0, b1, b2, b3, keysched, rekeysched) \
        vldm         rekeysched!, {q8}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q5); \
        vldm         rekeysched, {q9}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q6); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q7); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q8); \
        vldmia       keysched!, {q8}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q9); \
        sub          rekeysched, #(1*16); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q10); \
        vldm         keysched, {q9}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q11); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q12); \
        sub          keysched, #16; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q13); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q14); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q15); \
        aes##ed.8    b0, q8; \
        veor         b0, b0, q9; \
        aes##ed.8    b1, q8; \
        veor         b1, b1, q9; \
        aes##ed.8    b2, q8; \
        veor         b2, b2, q9; \
        aes##ed.8    b3, q8; \
        veor         b3, b3, q9;

#define do_aes_4_256(ed, mcimc, b0, b1, b2, b3, keysched, rekeysched) \
        vldmia       rekeysched!, {q8}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q5); \
        vldmia       rekeysched!, {q9}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q6); \
        vldmia       rekeysched!, {q10}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q7); \
        vldm         rekeysched, {q11}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q8); \
        vldmia       keysched!, {q8}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q9); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q10); \
        vldmia       keysched!, {q9}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q11); \
        sub          rekeysched, #(3*16); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q12); \
        vldmia       keysched!, {q10}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q13); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q14); \
        vldm         keysched, {q11}; \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q15); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q8); \
        aes_round_4(ed, mcimc, b0, b1, b2, b3, q9); \
        sub          keysched, #(3*16); \
        aes##ed.8    b0, q10; \
        veor         b0, b0, q11; \
        aes##ed.8    b1, q10; \
        veor         b1, b1, q11; \
        aes##ed.8    b2, q10; \
        veor         b2, b2, q11; \
        aes##ed.8    b3, q10; \
        veor         b3, b3, q11;


/* Other functional macros */

#define CLEAR_REG(reg) vmov.i8 reg, #0;


/*
 * unsigned int _gcry_aes_enc_armv8_ce(void *keysched, byte *dst,
 *                                     const byte *src,
 *                                     unsigned int nrounds);
 */
.align 3
.globl _gcry_aes_enc_armv8_ce
.type  _gcry_aes_enc_armv8_ce,%function;
_gcry_aes_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: dst
   *    r2: src
   *    r3: nrounds
   */

  vldmia r0!, {q1-q3} /* load 3 round keys */

  cmp r3, #12

  vld1.8 {q0}, [r2]

  bhi .Lenc1_256
  beq .Lenc1_192

.Lenc1_128:

.Lenc1_tail:
  vldmia r0, {q8-q15} /* load 8 round keys */

  aese.8  q0, q1
  aesmc.8 q0, q0
  CLEAR_REG(q1)

  aese.8  q0, q2
  aesmc.8 q0, q0
  CLEAR_REG(q2)

  aese.8  q0, q3
  aesmc.8 q0, q0
  CLEAR_REG(q3)

  aese.8  q0, q8
  aesmc.8 q0, q0
  CLEAR_REG(q8)

  aese.8  q0, q9
  aesmc.8 q0, q0
  CLEAR_REG(q9)

  aese.8  q0, q10
  aesmc.8 q0, q0
  CLEAR_REG(q10)

  aese.8  q0, q11
  aesmc.8 q0, q0
  CLEAR_REG(q11)

  aese.8  q0, q12
  aesmc.8 q0, q0
  CLEAR_REG(q12)

  aese.8  q0, q13
  aesmc.8 q0, q0
  CLEAR_REG(q13)

  aese.8  q0, q14
  veor    q0, q15
  CLEAR_REG(q14)
  CLEAR_REG(q15)

  vst1.8 {q0}, [r1]
  CLEAR_REG(q0)

  mov r0, #0
  bx lr

.Lenc1_192:
  aese.8  q0, q1
  aesmc.8 q0, q0
  vmov q1, q3

  aese.8  q0, q2
  aesmc.8 q0, q0
  vldm r0!, {q2-q3} /* load 3 round keys */

  b .Lenc1_tail

.Lenc1_256:
  vldm r0!, {q15}   /* load 1 round key */
  aese.8  q0, q1
  aesmc.8 q0, q0

  aese.8  q0, q2
  aesmc.8 q0, q0

  aese.8  q0, q3
  aesmc.8 q0, q0
  vldm r0!, {q1-q3} /* load 3 round keys */

  aese.8  q0, q15
  aesmc.8 q0, q0

  b .Lenc1_tail
.size _gcry_aes_enc_armv8_ce,.-_gcry_aes_enc_armv8_ce;


/*
 * unsigned int _gcry_aes_dec_armv8_ce(void *keysched, byte *dst,
 *                                     const byte *src,
 *                                     unsigned int nrounds);
 */
.align 3
.globl _gcry_aes_dec_armv8_ce
.type  _gcry_aes_dec_armv8_ce,%function;
_gcry_aes_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: dst
   *    r2: src
   *    r3: nrounds
   */

  vldmia r0!, {q1-q3} /* load 3 round keys */

  cmp r3, #12

  vld1.8 {q0}, [r2]

  bhi .Ldec1_256
  beq .Ldec1_192

.Ldec1_128:

.Ldec1_tail:
  vldmia r0, {q8-q15} /* load 8 round keys */

  aesd.8   q0, q1
  aesimc.8 q0, q0
  CLEAR_REG(q1)

  aesd.8   q0, q2
  aesimc.8 q0, q0
  CLEAR_REG(q2)

  aesd.8   q0, q3
  aesimc.8 q0, q0
  CLEAR_REG(q3)

  aesd.8   q0, q8
  aesimc.8 q0, q0
  CLEAR_REG(q8)

  aesd.8   q0, q9
  aesimc.8 q0, q0
  CLEAR_REG(q9)

  aesd.8   q0, q10
  aesimc.8 q0, q0
  CLEAR_REG(q10)

  aesd.8   q0, q11
  aesimc.8 q0, q0
  CLEAR_REG(q11)

  aesd.8   q0, q12
  aesimc.8 q0, q0
  CLEAR_REG(q12)

  aesd.8   q0, q13
  aesimc.8 q0, q0
  CLEAR_REG(q13)

  aesd.8   q0, q14
  veor     q0, q15
  CLEAR_REG(q14)
  CLEAR_REG(q15)

  vst1.8 {q0}, [r1]
  CLEAR_REG(q0)

  mov r0, #0
  bx lr

.Ldec1_192:
  aesd.8   q0, q1
  aesimc.8 q0, q0
  vmov q1, q3

  aesd.8   q0, q2
  aesimc.8 q0, q0
  vldm r0!, {q2-q3} /* load 3 round keys */

  b .Ldec1_tail

.Ldec1_256:
  vldm r0!, {q15}   /* load 1 round key */
  aesd.8   q0, q1
  aesimc.8 q0, q0

  aesd.8   q0, q2
  aesimc.8 q0, q0

  aesd.8  q0, q3
  aesimc.8 q0, q0
  vldm r0!, {q1-q3} /* load 3 round keys */

  aesd.8   q0, q15
  aesimc.8 q0, q0

  b .Ldec1_tail
.size _gcry_aes_dec_armv8_ce,.-_gcry_aes_dec_armv8_ce;


/*
 * void _gcry_aes_cbc_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, size_t nblocks,
 *                                  int cbc_mac, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cbc_enc_armv8_ce
.type  _gcry_aes_cbc_enc_armv8_ce,%function;
_gcry_aes_cbc_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: cbc_mac => r5
   *    %st+8: nrounds => r6
   */

  push {r4-r6,lr} /* 4*4 = 16b */
  ldr r4, [sp, #(16+0)]
  ldr r5, [sp, #(16+4)]
  cmp r4, #0
  ldr r6, [sp, #(16+8)]
  beq .Lcbc_enc_skip
  cmp r5, #0
  vpush {q4-q7}
  moveq r5, #16
  movne r5, #0

  cmp r6, #12
  vld1.8 {q1}, [r3] /* load IV */

  aes_preload_keys(r0, lr);

  beq .Lcbc_enc_loop192
  bhi .Lcbc_enc_loop256

#define CBC_ENC(bits, ...) \
  .Lcbc_enc_loop##bits: \
    vld1.8 {q0}, [r2]!; /* load plaintext */ \
    veor q1, q0, q1; \
    subs r4, r4, #1; \
    \
    do_aes_one##bits(e, mc, q1, q1, ##__VA_ARGS__); \
    \
    vst1.8 {q1}, [r1], r5; /* store ciphertext */ \
    \
    bne .Lcbc_enc_loop##bits; \
    b .Lcbc_enc_done;

  CBC_ENC(128)
  CBC_ENC(192, r0, lr)
  CBC_ENC(256, r0, lr)

#undef CBC_ENC

.Lcbc_enc_done:
  vst1.8 {q1}, [r3] /* store IV */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  vpop {q4-q7}
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lcbc_enc_skip:
  pop {r4-r6,pc}
.size _gcry_aes_cbc_enc_armv8_ce,.-_gcry_aes_cbc_enc_armv8_ce;


/*
 * void _gcry_aes_cbc_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cbc_dec_armv8_ce
.type  _gcry_aes_cbc_dec_armv8_ce,%function;
_gcry_aes_cbc_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  push {r4-r6,lr} /* 4*4 = 16b */
  ldr r4, [sp, #(16+0)]
  ldr r5, [sp, #(16+4)]
  cmp r4, #0
  beq .Lcbc_dec_skip
  vpush {q4-q7}

  cmp r5, #12
  vld1.8 {q0}, [r3] /* load IV */

  aes_preload_keys(r0, r6);

  beq .Lcbc_dec_entry_192
  bhi .Lcbc_dec_entry_256

#define CBC_DEC(bits, ...) \
  .Lcbc_dec_entry_##bits: \
    cmp r4, #4; \
    blo .Lcbc_dec_loop_##bits; \
    \
  .Lcbc_dec_loop4_##bits: \
    \
    vld1.8 {q1-q2}, [r2]!; /* load ciphertext */ \
    sub r4, r4, #4; \
    vld1.8 {q3-q4}, [r2]; /* load ciphertext */ \
    cmp r4, #4; \
    sub r2, #32; \
    \
    do_aes_4_##bits(d, imc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    veor q1, q1, q0; \
    vld1.8 {q0}, [r2]!; /* load next IV */ \
    veor q2, q2, q0; \
    vld1.8 {q0}, [r2]!; /* load next IV */ \
    vst1.8 {q1-q2}, [r1]!; /* store plaintext */ \
    veor q3, q3, q0; \
    vld1.8 {q0}, [r2]!; /* load next IV */ \
    veor q4, q4, q0; \
    vld1.8 {q0}, [r2]!; /* load next IV */ \
    vst1.8 {q3-q4}, [r1]!; /* store plaintext */ \
    \
    bhs .Lcbc_dec_loop4_##bits; \
    cmp r4, #0; \
    beq .Lcbc_dec_done; \
    \
  .Lcbc_dec_loop_##bits: \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    subs r4, r4, #1; \
    vmov q2, q1; \
    \
    do_aes_one##bits(d, imc, q1, q1, ##__VA_ARGS__); \
    \
    veor q1, q1, q0; \
    vmov q0, q2; \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    \
    bne .Lcbc_dec_loop_##bits; \
    b .Lcbc_dec_done;

  CBC_DEC(128)
  CBC_DEC(192, r0, r6)
  CBC_DEC(256, r0, r6)

#undef CBC_DEC

.Lcbc_dec_done:
  vst1.8 {q0}, [r3] /* store IV */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  vpop {q4-q7}
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lcbc_dec_skip:
  pop {r4-r6,pc}
.size _gcry_aes_cbc_dec_armv8_ce,.-_gcry_aes_cbc_dec_armv8_ce;


/*
 * void _gcry_aes_cfb_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cfb_enc_armv8_ce
.type  _gcry_aes_cfb_enc_armv8_ce,%function;
_gcry_aes_cfb_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  push {r4-r6,lr} /* 4*4 = 16b */
  ldr r4, [sp, #(16+0)]
  ldr r5, [sp, #(16+4)]
  cmp r4, #0
  beq .Lcfb_enc_skip
  vpush {q4-q7}

  cmp r5, #12
  vld1.8 {q0}, [r3] /* load IV */

  aes_preload_keys(r0, r6);

  beq .Lcfb_enc_entry_192
  bhi .Lcfb_enc_entry_256

#define CFB_ENC(bits, ...) \
  .Lcfb_enc_entry_##bits: \
  .Lcfb_enc_loop_##bits: \
    vld1.8 {q1}, [r2]!; /* load plaintext */ \
    subs r4, r4, #1; \
    \
    do_aes_one##bits(e, mc, q0, q0, ##__VA_ARGS__); \
    \
    veor q0, q1, q0; \
    vst1.8 {q0}, [r1]!; /* store ciphertext */ \
    \
    bne .Lcfb_enc_loop_##bits; \
    b .Lcfb_enc_done;

  CFB_ENC(128)
  CFB_ENC(192, r0, r6)
  CFB_ENC(256, r0, r6)

#undef CFB_ENC

.Lcfb_enc_done:
  vst1.8 {q0}, [r3] /* store IV */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  vpop {q4-q7}
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lcfb_enc_skip:
  pop {r4-r6,pc}
.size _gcry_aes_cfb_enc_armv8_ce,.-_gcry_aes_cfb_enc_armv8_ce;


/*
 * void _gcry_aes_cfb_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cfb_dec_armv8_ce
.type  _gcry_aes_cfb_dec_armv8_ce,%function;
_gcry_aes_cfb_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  push {r4-r6,lr} /* 4*4 = 16b */
  ldr r4, [sp, #(16+0)]
  ldr r5, [sp, #(16+4)]
  cmp r4, #0
  beq .Lcfb_dec_skip
  vpush {q4-q7}

  cmp r5, #12
  vld1.8 {q0}, [r3] /* load IV */

  aes_preload_keys(r0, r6);

  beq .Lcfb_dec_entry_192
  bhi .Lcfb_dec_entry_256

#define CFB_DEC(bits, ...) \
  .Lcfb_dec_entry_##bits: \
    cmp r4, #4; \
    blo .Lcfb_dec_loop_##bits; \
    \
  .Lcfb_dec_loop4_##bits: \
    \
    vld1.8 {q2-q3}, [r2]!; /* load ciphertext */ \
    vmov q1, q0; \
    sub r4, r4, #4; \
    vld1.8 {q4}, [r2]; /* load ciphertext */ \
    sub r2, #32; \
    cmp r4, #4; \
    \
    do_aes_4_##bits(e, mc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    veor q1, q1, q0; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    veor q2, q2, q0; \
    vst1.8 {q1-q2}, [r1]!; /* store plaintext */ \
    vld1.8 {q0}, [r2]!; \
    veor q3, q3, q0; \
    vld1.8 {q0}, [r2]!; /* load next IV / ciphertext */ \
    veor q4, q4, q0; \
    vst1.8 {q3-q4}, [r1]!; /* store plaintext */ \
    \
    bhs .Lcfb_dec_loop4_##bits; \
    cmp r4, #0; \
    beq .Lcfb_dec_done; \
    \
  .Lcfb_dec_loop_##bits: \
    \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    \
    subs r4, r4, #1; \
    \
    do_aes_one##bits(e, mc, q0, q0, ##__VA_ARGS__); \
    \
    veor q2, q1, q0; \
    vmov q0, q1; \
    vst1.8 {q2}, [r1]!; /* store plaintext */ \
    \
    bne .Lcfb_dec_loop_##bits; \
    b .Lcfb_dec_done;

  CFB_DEC(128)
  CFB_DEC(192, r0, r6)
  CFB_DEC(256, r0, r6)

#undef CFB_DEC

.Lcfb_dec_done:
  vst1.8 {q0}, [r3] /* store IV */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  vpop {q4-q7}
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lcfb_dec_skip:
  pop {r4-r6,pc}
.size _gcry_aes_cfb_dec_armv8_ce,.-_gcry_aes_cfb_dec_armv8_ce;


/*
 * void _gcry_aes_ctr_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_ctr_enc_armv8_ce
.type  _gcry_aes_ctr_enc_armv8_ce,%function;
_gcry_aes_ctr_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r4, [sp, #(104+0)]
  ldr r5, [sp, #(104+4)]
  cmp r4, #0
  beq .Lctr_enc_skip

  cmp r5, #12
  ldm r3, {r7-r10}
  vld1.8 {q0}, [r3] /* load IV */
  rev r7, r7
  rev r8, r8
  rev r9, r9
  rev r10, r10

  aes_preload_keys(r0, r6);

  beq .Lctr_enc_entry_192
  bhi .Lctr_enc_entry_256

#define CTR_ENC(bits, ...) \
  .Lctr_enc_entry_##bits: \
    cmp r4, #4; \
    blo .Lctr_enc_loop_##bits; \
    \
  .Lctr_enc_loop4_##bits: \
    cmp r10, #0xfffffffc; \
    sub r4, r4, #4; \
    blo .Lctr_enc_loop4_##bits##_nocarry; \
    cmp r9, #0xffffffff; \
    bne .Lctr_enc_loop4_##bits##_nocarry; \
    \
    adds r10, #1; \
    vmov q1, q0; \
    blcs .Lctr_overflow_one; \
    rev r11, r10; \
    vmov.32 d1[1], r11; \
    \
    adds r10, #1; \
    vmov q2, q0; \
    blcs .Lctr_overflow_one; \
    rev r11, r10; \
    vmov.32 d1[1], r11; \
    \
    adds r10, #1; \
    vmov q3, q0; \
    blcs .Lctr_overflow_one; \
    rev r11, r10; \
    vmov.32 d1[1], r11; \
    \
    adds r10, #1; \
    vmov q4, q0; \
    blcs .Lctr_overflow_one; \
    rev r11, r10; \
    vmov.32 d1[1], r11; \
    \
    b .Lctr_enc_loop4_##bits##_store_ctr; \
    \
  .Lctr_enc_loop4_##bits##_nocarry: \
    \
    veor q2, q2; \
    vrev64.8 q1, q0; \
    vceq.u32 d5, d5; \
    vadd.u64 q3, q2, q2; \
    vadd.u64 q4, q3, q2; \
    vadd.u64 q0, q3, q3; \
    vsub.u64 q2, q1, q2; \
    vsub.u64 q3, q1, q3; \
    vsub.u64 q4, q1, q4; \
    vsub.u64 q0, q1, q0; \
    vrev64.8 q1, q1; \
    vrev64.8 q2, q2; \
    vrev64.8 q3, q3; \
    vrev64.8 q0, q0; \
    vrev64.8 q4, q4; \
    add r10, #4; \
    \
  .Lctr_enc_loop4_##bits##_store_ctr: \
    \
    vst1.8 {q0}, [r3]; \
    cmp r4, #4; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    \
    do_aes_4_##bits(e, mc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    veor q1, q1, q0; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    veor q2, q2, q0; \
    veor q3, q3, q1; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    vst1.8 {q2}, [r1]!; /* store plaintext */ \
    veor q4, q4, q0; \
    vld1.8 {q0}, [r3]; /* reload IV */ \
    vst1.8 {q3-q4}, [r1]!; /* store plaintext */ \
    \
    bhs .Lctr_enc_loop4_##bits; \
    cmp r4, #0; \
    beq .Lctr_enc_done; \
    \
  .Lctr_enc_loop_##bits: \
    \
    adds r10, #1; \
    vmov q1, q0; \
    blcs .Lctr_overflow_one; \
    rev r11, r10; \
    subs r4, r4, #1; \
    vld1.8 {q2}, [r2]!; /* load ciphertext */ \
    vmov.32 d1[1], r11; \
    \
    do_aes_one##bits(e, mc, q1, q1, ##__VA_ARGS__); \
    \
    veor q1, q2, q1; \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    \
    bne .Lctr_enc_loop_##bits; \
    b .Lctr_enc_done;

  CTR_ENC(128)
  CTR_ENC(192, r0, r6)
  CTR_ENC(256, r0, r6)

#undef CTR_ENC

.Lctr_enc_done:
  vst1.8 {q0}, [r3] /* store IV */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lctr_enc_skip:
  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr

.Lctr_overflow_one:
  adcs r9, #0
  adcs r8, #0
  adc r7, #0
  rev r11, r9
  rev r12, r8
  vmov.32 d1[0], r11
  rev r11, r7
  vmov.32 d0[1], r12
  vmov.32 d0[0], r11
  bx lr
.size _gcry_aes_ctr_enc_armv8_ce,.-_gcry_aes_ctr_enc_armv8_ce;


/*
 * void _gcry_aes_ctr32le_enc_armv8_ce (const void *keysched,
 *                                      unsigned char *outbuf,
 *                                      const unsigned char *inbuf,
 *                                      unsigned char *iv,
 *                                      unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_ctr32le_enc_armv8_ce
.type  _gcry_aes_ctr32le_enc_armv8_ce,%function;
_gcry_aes_ctr32le_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r4, [sp, #(104+0)]
  ldr r5, [sp, #(104+4)]
  cmp r4, #0
  beq .Lctr32le_enc_skip

  cmp r5, #12
  vld1.8 {q0}, [r3] /* load IV */

  aes_preload_keys(r0, r6);

  beq .Lctr32le_enc_entry_192
  bhi .Lctr32le_enc_entry_256

#define CTR_ENC(bits, ...) \
  .Lctr32le_enc_entry_##bits: \
    cmp r4, #4; \
    blo .Lctr32le_enc_loop_##bits; \
    \
  .Lctr32le_enc_loop4_##bits: \
    veor q2, q2; \
    sub r4, r4, #4; \
    vmov.i64 d4, #0xffffffff; /* q2 <= -1:0:0:0 */ \
    vmov q1, q0; \
    vadd.u32 q3, q2, q2; /* q3 <= -2:0:0:0 */ \
    vadd.u32 q0, q3, q3; /* q0 <= -4:0:0:0 */ \
    vadd.u32 q4, q3, q2; /* q4 <= -3:0:0:0 */ \
    vsub.u32 q0, q1, q0; \
    vsub.u32 q2, q1, q2; \
    vst1.8 {q0}, [r3]; \
    vsub.u32 q3, q1, q3; \
    vsub.u32 q4, q1, q4; \
    \
    cmp r4, #4; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    \
    do_aes_4_##bits(e, mc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    veor q1, q1, q0; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    veor q2, q2, q0; \
    veor q3, q3, q1; \
    vld1.8 {q0}, [r2]!; /* load ciphertext */ \
    vst1.8 {q2}, [r1]!; /* store plaintext */ \
    veor q4, q4, q0; \
    vld1.8 {q0}, [r3]; /* reload IV */ \
    vst1.8 {q3-q4}, [r1]!; /* store plaintext */ \
    \
    bhs .Lctr32le_enc_loop4_##bits; \
    cmp r4, #0; \
    beq .Lctr32le_enc_done; \
    \
  .Lctr32le_enc_loop_##bits: \
    \
    veor q2, q2; \
    vmov q1, q0; \
    vmov.i64 d4, #0xffffffff; /* q2 <= -1:0:0:0 */ \
    subs r4, r4, #1; \
    vsub.u32 q0, q0, q2; \
    vld1.8 {q2}, [r2]!; /* load ciphertext */ \
    \
    do_aes_one##bits(e, mc, q1, q1, ##__VA_ARGS__); \
    \
    veor q1, q2, q1; \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    \
    bne .Lctr32le_enc_loop_##bits; \
    b .Lctr32le_enc_done;

  CTR_ENC(128)
  CTR_ENC(192, r0, r6)
  CTR_ENC(256, r0, r6)

#undef CTR_ENC

.Lctr32le_enc_done:
  vst1.8 {q0}, [r3] /* store IV */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lctr32le_enc_skip:
  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr
.size _gcry_aes_ctr32le_enc_armv8_ce,.-_gcry_aes_ctr32le_enc_armv8_ce;


/*
 * void _gcry_aes_ocb_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *offset,
 *                                  unsigned char *checksum,
 *                                  unsigned char *L_table,
 *                                  size_t nblocks,
 *                                  unsigned int nrounds,
 *                                  unsigned int blkn);
 */

.align 3
.globl _gcry_aes_ocb_enc_armv8_ce
.type  _gcry_aes_ocb_enc_armv8_ce,%function;
_gcry_aes_ocb_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: offset
   *    %st+0: checksum => r4
   *    %st+4: Ls => r5
   *    %st+8: nblocks => r6  (0 < nblocks <= 32)
   *    %st+12: nrounds => r7
   *    %st+16: blkn => lr
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r7, [sp, #(104+12)]
  ldr r4, [sp, #(104+0)]
  ldr r5, [sp, #(104+4)]
  ldr r6, [sp, #(104+8)]
  ldr lr, [sp, #(104+16)]

  cmp r7, #12
  vld1.8 {q0}, [r3] /* load offset */

  aes_preload_keys(r0, r12);

  beq .Locb_enc_entry_192
  bhi .Locb_enc_entry_256

#define OCB_ENC(bits, ...) \
  .Locb_enc_entry_##bits: \
    cmp r6, #4; \
    add lr, #1; \
    blo .Locb_enc_loop_##bits; \
    \
  .Locb_enc_loop4_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */ \
    \
    add r9, lr, #1; \
    add r10, lr, #2; \
    add r11, lr, #3; \
    rbit r8, lr; \
    add lr, lr, #4; \
    rbit r9, r9; \
    rbit r10, r10; \
    rbit r11, r11; \
    clz r8, r8; /* ntz(i+0) */ \
    clz r9, r9; /* ntz(i+1) */ \
    clz r10, r10; /* ntz(i+2) */ \
    clz r11, r11; /* ntz(i+3) */ \
    add r8, r5, r8, lsl #4; \
    add r9, r5, r9, lsl #4; \
    add r10, r5, r10, lsl #4; \
    add r11, r5, r11, lsl #4; \
    \
    sub r6, #4; \
    \
    vld1.8 {q9}, [r8];     /* load L_{ntz(i+0)} */ \
    vld1.8 {q1-q2}, [r2]!; /* load P_i+<0-1> */ \
    vld1.8 {q8}, [r4];     /* load Checksum_{i-1} */ \
    veor q0, q0, q9;       /* Offset_i+0 */ \
    vld1.8 {q9}, [r9];     /* load L_{ntz(i+1)} */ \
    veor q8, q8, q1;       /* Checksum_i+0 */ \
    veor q1, q1, q0;       /* P_i+0 xor Offset_i+0 */\
    vld1.8 {q3-q4}, [r2]!; /* load P_i+<2-3> */ \
    vst1.8 {q0}, [r1]!;    /* store Offset_i+0 */\
    veor q0, q0, q9;       /* Offset_i+1 */ \
    vld1.8 {q9}, [r10];    /* load L_{ntz(i+2)} */ \
    veor q8, q8, q2;       /* Checksum_i+1 */ \
    veor q2, q2, q0;       /* P_i+1 xor Offset_i+1 */\
    vst1.8 {q0}, [r1]!;    /* store Offset_i+1 */\
    veor q0, q0, q9;       /* Offset_i+2 */ \
    vld1.8 {q9}, [r11];    /* load L_{ntz(i+3)} */ \
    veor q8, q8, q3;       /* Checksum_i+2 */ \
    veor q3, q3, q0;       /* P_i+2 xor Offset_i+2 */\
    vst1.8 {q0}, [r1]!;    /* store Offset_i+2 */\
    veor q0, q0, q9;       /* Offset_i+3 */ \
    veor q8, q8, q4;       /* Checksum_i+3 */ \
    veor q4, q4, q0;       /* P_i+3 xor Offset_i+3 */\
    vst1.8 {q0}, [r1];     /* store Offset_i+3 */\
    sub r1, #(3*16); \
    vst1.8 {q8}, [r4];     /* store Checksum_i+3 */\
    \
    cmp r6, #4; \
    \
    do_aes_4_##bits(e, mc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    mov r8, r1; \
    vld1.8 {q8-q9}, [r1]!; \
    veor q1, q1, q8; \
    veor q2, q2, q9; \
    vld1.8 {q8-q9}, [r1]!; \
    vst1.8 {q1-q2}, [r8]!; \
    veor q3, q3, q8; \
    veor q4, q4, q9; \
    vst1.8 {q3-q4}, [r8]; \
    \
    bhs .Locb_enc_loop4_##bits; \
    cmp r6, #0; \
    beq .Locb_enc_done; \
    \
  .Locb_enc_loop_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */ \
    \
    rbit r8, lr; \
    add lr, #1; \
    clz r8, r8; /* ntz(i) */ \
    add r8, r5, r8, lsl #4; \
    \
    vld1.8 {q1}, [r2]!; /* load plaintext */ \
    vld1.8 {q2}, [r8]; /* load L_{ntz(i)} */ \
    vld1.8 {q3}, [r4]; /* load checksum */ \
    subs r6, #1; \
    veor q0, q0, q2; \
    veor q3, q3, q1; \
    veor q1, q1, q0; \
    vst1.8 {q3}, [r4]; /* store checksum */ \
    \
    do_aes_one##bits(e, mc, q1, q1, ##__VA_ARGS__); \
    \
    veor q1, q1, q0; \
    vst1.8 {q1}, [r1]!; /* store ciphertext */ \
    \
    bne .Locb_enc_loop_##bits; \
    b .Locb_enc_done;

  OCB_ENC(128re, r0, r12)
  OCB_ENC(192, r0, r12)
  OCB_ENC(256, r0, r12)

#undef OCB_ENC

.Locb_enc_done:
  vst1.8 {q0}, [r3] /* store offset */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr
.size _gcry_aes_ocb_enc_armv8_ce,.-_gcry_aes_ocb_enc_armv8_ce;


/*
 * void _gcry_aes_ocb_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *offset,
 *                                  unsigned char *checksum,
 *                                  unsigned char *L_table,
 *                                  size_t nblocks,
 *                                  unsigned int nrounds,
 *                                  unsigned int blkn);
 */

.align 3
.globl _gcry_aes_ocb_dec_armv8_ce
.type  _gcry_aes_ocb_dec_armv8_ce,%function;
_gcry_aes_ocb_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: offset
   *    %st+0: checksum => r4
   *    %st+4: Ls => r5
   *    %st+8: nblocks => r6  (0 < nblocks <= 32)
   *    %st+12: nrounds => r7
   *    %st+16: blkn => lr
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r7, [sp, #(104+12)]
  ldr r4, [sp, #(104+0)]
  ldr r5, [sp, #(104+4)]
  ldr r6, [sp, #(104+8)]
  ldr lr, [sp, #(104+16)]

  cmp r7, #12
  vld1.8 {q0}, [r3] /* load offset */

  aes_preload_keys(r0, r12);

  beq .Locb_dec_entry_192
  bhi .Locb_dec_entry_256

#define OCB_DEC(bits, ...) \
  .Locb_dec_entry_##bits: \
    cmp r6, #4; \
    add lr, #1; \
    blo .Locb_dec_loop_##bits; \
    \
  .Locb_dec_loop4_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    \
    add r9, lr, #1; \
    add r10, lr, #2; \
    add r11, lr, #3; \
    rbit r8, lr; \
    add lr, lr, #4; \
    rbit r9, r9; \
    rbit r10, r10; \
    rbit r11, r11; \
    clz r8, r8; /* ntz(i+0) */ \
    clz r9, r9; /* ntz(i+1) */ \
    clz r10, r10; /* ntz(i+2) */ \
    clz r11, r11; /* ntz(i+3) */ \
    add r8, r5, r8, lsl #4; \
    add r9, r5, r9, lsl #4; \
    add r10, r5, r10, lsl #4; \
    add r11, r5, r11, lsl #4; \
    \
    sub r6, #4; \
    \
    vld1.8 {q9}, [r8];     /* load L_{ntz(i+0)} */ \
    vld1.8 {q1-q2}, [r2]!; /* load P_i+<0-1> */ \
    veor q0, q0, q9;       /* Offset_i+0 */ \
    vld1.8 {q9}, [r9];     /* load L_{ntz(i+1)} */ \
    veor q1, q1, q0;       /* P_i+0 xor Offset_i+0 */\
    vld1.8 {q3-q4}, [r2]!; /* load P_i+<2-3> */ \
    vst1.8 {q0}, [r1]!;    /* store Offset_i+0 */\
    veor q0, q0, q9;       /* Offset_i+1 */ \
    vld1.8 {q9}, [r10];    /* load L_{ntz(i+2)} */ \
    veor q2, q2, q0;       /* P_i+1 xor Offset_i+1 */\
    vst1.8 {q0}, [r1]!;    /* store Offset_i+1 */\
    veor q0, q0, q9;       /* Offset_i+2 */ \
    vld1.8 {q9}, [r11];    /* load L_{ntz(i+3)} */ \
    veor q3, q3, q0;       /* P_i+2 xor Offset_i+2 */\
    vst1.8 {q0}, [r1]!;    /* store Offset_i+2 */\
    veor q0, q0, q9;       /* Offset_i+3 */ \
    veor q4, q4, q0;       /* P_i+3 xor Offset_i+3 */\
    vst1.8 {q0}, [r1];     /* store Offset_i+3 */\
    sub r1, #(3*16); \
    \
    cmp r6, #4; \
    \
    do_aes_4_##bits(d, imc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    mov r8, r1; \
    vld1.8 {q8-q9}, [r1]!; \
    veor q1, q1, q8; \
    veor q2, q2, q9; \
    vld1.8 {q8-q9}, [r1]!; \
    vst1.8 {q1-q2}, [r8]!; \
    veor q1, q1, q2; \
    vld1.8 {q2}, [r4];     /* load Checksum_{i-1} */ \
    veor q3, q3, q8; \
    veor q1, q1, q3; \
    veor q4, q4, q9; \
    veor q1, q1, q4; \
    vst1.8 {q3-q4}, [r8]; \
    veor q2, q2, q1; \
    vst1.8 {q2}, [r4];     /* store Checksum_i+3 */ \
    \
    bhs .Locb_dec_loop4_##bits; \
    cmp r6, #0; \
    beq .Locb_dec_done; \
    \
  .Locb_dec_loop_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    \
    rbit r8, lr; \
    add lr, #1; \
    clz r8, r8; /* ntz(i) */ \
    add r8, r5, r8, lsl #4; \
    \
    vld1.8 {q2}, [r8]; /* load L_{ntz(i)} */ \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    subs r6, #1; \
    veor q0, q0, q2; \
    veor q1, q1, q0; \
    \
    do_aes_one##bits(d, imc, q1, q1, ##__VA_ARGS__) \
    \
    vld1.8 {q2}, [r4]; /* load checksum */ \
    veor q1, q1, q0; \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    veor q2, q2, q1; \
    vst1.8 {q2}, [r4]; /* store checksum */ \
    \
    bne .Locb_dec_loop_##bits; \
    b .Locb_dec_done;

  OCB_DEC(128re, r0, r12)
  OCB_DEC(192, r0, r12)
  OCB_DEC(256, r0, r12)

#undef OCB_DEC

.Locb_dec_done:
  vst1.8 {q0}, [r3] /* store offset */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr
.size _gcry_aes_ocb_dec_armv8_ce,.-_gcry_aes_ocb_dec_armv8_ce;


/*
 * void _gcry_aes_ocb_auth_armv8_ce (const void *keysched,
 *                                   const unsigned char *abuf,
 *                                   unsigned char *offset,
 *                                   unsigned char *checksum,
 *                                   unsigned char *L_table,
 *                                   size_t nblocks,
 *                                   unsigned int nrounds,
 *                                   unsigned int blkn);
 */

.align 3
.globl _gcry_aes_ocb_auth_armv8_ce
.type  _gcry_aes_ocb_auth_armv8_ce,%function;
_gcry_aes_ocb_auth_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: abuf
   *    r2: offset
   *    r3: checksum
   *    %st+0: Ls => r5
   *    %st+4: nblocks => r6  (0 < nblocks <= 32)
   *    %st+8: nrounds => r7
   *    %st+12: blkn => lr
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r7, [sp, #(104+8)]
  ldr r5, [sp, #(104+0)]
  ldr r6, [sp, #(104+4)]
  ldr lr, [sp, #(104+12)]

  cmp r7, #12
  vld1.8 {q0}, [r2] /* load offset */

  aes_preload_keys(r0, r12);

  beq .Locb_auth_entry_192
  bhi .Locb_auth_entry_256

#define OCB_AUTH(bits, ...) \
  .Locb_auth_entry_##bits: \
    cmp r6, #4; \
    add lr, #1; \
    blo .Locb_auth_loop_##bits; \
    \
  .Locb_auth_loop4_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */ \
    \
    add r9, lr, #1; \
    add r10, lr, #2; \
    add r11, lr, #3; \
    rbit r8, lr; \
    add lr, lr, #4; \
    rbit r9, r9; \
    rbit r10, r10; \
    rbit r11, r11; \
    clz r8, r8; /* ntz(i+0) */ \
    clz r9, r9; /* ntz(i+1) */ \
    clz r10, r10; /* ntz(i+2) */ \
    clz r11, r11; /* ntz(i+3) */ \
    add r8, r5, r8, lsl #4; \
    add r9, r5, r9, lsl #4; \
    add r10, r5, r10, lsl #4; \
    add r11, r5, r11, lsl #4; \
    \
    sub r6, #4; \
    \
    vld1.8 {q9}, [r8];     /* load L_{ntz(i+0)} */ \
    vld1.8 {q1-q2}, [r1]!; /* load A_i+<0-1> */ \
    veor q0, q0, q9;       /* Offset_i+0 */ \
    vld1.8 {q9}, [r9];     /* load L_{ntz(i+1)} */ \
    veor q1, q1, q0;       /* A_i+0 xor Offset_i+0 */\
    vld1.8 {q3-q4}, [r1]!; /* load A_i+<2-3> */ \
    veor q0, q0, q9;       /* Offset_i+1 */ \
    vld1.8 {q9}, [r10];    /* load L_{ntz(i+2)} */ \
    veor q2, q2, q0;       /* A_i+1 xor Offset_i+1 */\
    veor q0, q0, q9;       /* Offset_i+2 */ \
    vld1.8 {q9}, [r11];    /* load L_{ntz(i+3)} */ \
    veor q3, q3, q0;       /* A_i+2 xor Offset_i+2 */\
    veor q0, q0, q9;       /* Offset_i+3 */ \
    veor q4, q4, q0;       /* A_i+3 xor Offset_i+3 */\
    \
    cmp r6, #4; \
    \
    do_aes_4_##bits(e, mc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    veor q1, q1, q2; \
    veor q3, q3, q4; \
    vld1.8 {q2}, [r3]; \
    veor q1, q1, q3; \
    veor q2, q2, q1; \
    vst1.8 {q2}, [r3]; \
    \
    bhs .Locb_auth_loop4_##bits; \
    cmp r6, #0; \
    beq .Locb_auth_done; \
    \
  .Locb_auth_loop_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */ \
    \
    rbit r8, lr; \
    add lr, #1; \
    clz r8, r8; /* ntz(i) */ \
    add r8, r5, r8, lsl #4; \
    \
    vld1.8 {q2}, [r8];  /* load L_{ntz(i)} */ \
    vld1.8 {q1}, [r1]!; /* load aadtext */ \
    subs r6, #1; \
    veor q0, q0, q2; \
    vld1.8 {q2}, [r3]; /* load checksum */ \
    veor q1, q1, q0; \
    \
    do_aes_one##bits(e, mc, q1, q1, ##__VA_ARGS__) \
    \
    veor q2, q2, q1; \
    vst1.8 {q2}, [r3]; /* store checksum */ \
    \
    bne .Locb_auth_loop_##bits; \
    b .Locb_auth_done;

  OCB_AUTH(128re, r0, r12)
  OCB_AUTH(192, r0, r12)
  OCB_AUTH(256, r0, r12)

#undef OCB_AUTH

.Locb_auth_done:
  vst1.8 {q0}, [r2] /* store offset */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr
.size _gcry_aes_ocb_auth_armv8_ce,.-_gcry_aes_ocb_auth_armv8_ce;



/*
 * void _gcry_aes_xts_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_xts_enc_armv8_ce
.type  _gcry_aes_xts_enc_armv8_ce,%function;
_gcry_aes_xts_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r4, [sp, #(104+0)]
  ldr r5, [sp, #(104+4)]
  cmp r4, #0
  beq .Lxts_enc_skip

  cmp r5, #12

  vld1.8 {q0}, [r3] /* load tweak */
  mov r7, #0x87;

  aes_preload_keys(r0, r6);

  beq .Lxts_enc_entry_192
  bhi .Lxts_enc_entry_256

#define CTR_XTS(bits, ...) \
  .Lxts_enc_entry_##bits: \
    cmp r4, #4; \
    blo .Lxts_enc_loop_##bits; \
    \
  .Lxts_enc_loop4_##bits: \
    sub r4, r4, #4; \
    veor q9, q9, q9; \
    \
    vld1.8 {q1-q2}, [r2]!; /* load plaintext */ \
    veor q1, q1, q0; \
    cmp r4, #4; \
    vmov.u32 d18[0], r7; \
    vst1.8 {q0}, [r1]!; /* store tweak0 to temp */ \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    vld1.8 {q3-q4}, [r2]!; /* load plaintext */ \
    veor q2, q2, q0; \
    vst1.8 {q0}, [r1]!; /* store tweak1 to temp */ \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    veor q3, q3, q0; \
    vst1.8 {q0}, [r1]!; /* store tweak2 to temp */ \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    veor q4, q4, q0; \
    vst1.8 {q0}, [r1]; /* store tweak3 to temp */ \
    sub r1, r1, #48; \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    do_aes_4_##bits(e, mc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    vld1.8 {q8-q9}, [r1]!; /* load tweak from temp */ \
    veor q1, q1, q8; \
    veor q2, q2, q9; \
    vld1.8 {q8-q9}, [r1]; /* load tweak from temp */ \
    sub r1, r1, #32; \
    veor q3, q3, q8; \
    veor q4, q4, q9; \
    vst1.8 {q1-q2}, [r1]!; /* store plaintext */ \
    vst1.8 {q3-q4}, [r1]!; /* store plaintext */ \
    \
    bhs .Lxts_enc_loop4_##bits; \
    cmp r4, #0; \
    beq .Lxts_enc_done; \
    \
  .Lxts_enc_loop_##bits: \
    \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    \
    veor q9, q9, q9; \
    veor q1, q1, q0; \
    vmov.u32 d18[0], r7; \
    vmov q2, q0; \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    subs r4, r4, #1; \
    \
    do_aes_one##bits(e, mc, q1, q1, ##__VA_ARGS__); \
    \
    veor q1, q1, q2; \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    \
    bne .Lxts_enc_loop_##bits; \
    b .Lxts_enc_done;

  CTR_XTS(128re, r0, r6)
  CTR_XTS(192, r0, r6)
  CTR_XTS(256, r0, r6)

#undef CTR_XTS

.Lxts_enc_done:
  vst1.8 {q0}, [r3] /* store tweak */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lxts_enc_skip:
  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr
.size _gcry_aes_xts_enc_armv8_ce,.-_gcry_aes_xts_enc_armv8_ce;


/*
 * void _gcry_aes_xts_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_xts_dec_armv8_ce
.type  _gcry_aes_xts_dec_armv8_ce,%function;
_gcry_aes_xts_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    %st+0: nblocks => r4
   *    %st+4: nrounds => r5
   */

  vpush {q4-q7}
  push {r4-r12,lr} /* 4*16 + 4*10 = 104b */
  ldr r4, [sp, #(104+0)]
  ldr r5, [sp, #(104+4)]
  cmp r4, #0
  beq .Lxts_dec_skip

  cmp r5, #12

  vld1.8 {q0}, [r3] /* load tweak */
  mov r7, #0x87;

  aes_preload_keys(r0, r6);

  beq .Lxts_dec_entry_192
  bhi .Lxts_dec_entry_256

#define CTR_XTS(bits, ...) \
  .Lxts_dec_entry_##bits: \
    cmp r4, #4; \
    blo .Lxts_dec_loop_##bits; \
    \
  .Lxts_dec_loop4_##bits: \
    sub r4, r4, #4; \
    veor q9, q9, q9; \
    \
    vld1.8 {q1-q2}, [r2]!; /* load plaintext */ \
    veor q1, q1, q0; \
    cmp r4, #4; \
    vmov.u32 d18[0], r7; \
    vst1.8 {q0}, [r1]!; /* store tweak0 to temp */ \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    vld1.8 {q3-q4}, [r2]!; /* load plaintext */ \
    veor q2, q2, q0; \
    vst1.8 {q0}, [r1]!; /* store tweak1 to temp */ \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    veor q3, q3, q0; \
    vst1.8 {q0}, [r1]!; /* store tweak2 to temp */ \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    veor q4, q4, q0; \
    vst1.8 {q0}, [r1]; /* store tweak3 to temp */ \
    sub r1, r1, #48; \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    \
    do_aes_4_##bits(d, imc, q1, q2, q3, q4, ##__VA_ARGS__); \
    \
    vld1.8 {q8-q9}, [r1]!; /* load tweak from temp */ \
    veor q1, q1, q8; \
    veor q2, q2, q9; \
    vld1.8 {q8-q9}, [r1]; /* load tweak from temp */ \
    sub r1, r1, #32; \
    veor q3, q3, q8; \
    veor q4, q4, q9; \
    vst1.8 {q1-q2}, [r1]!; /* store plaintext */ \
    vst1.8 {q3-q4}, [r1]!; /* store plaintext */ \
    \
    bhs .Lxts_dec_loop4_##bits; \
    cmp r4, #0; \
    beq .Lxts_dec_done; \
    \
  .Lxts_dec_loop_##bits: \
    \
    vld1.8 {q1}, [r2]!; /* load ciphertext */ \
    \
    veor q9, q9, q9; \
    veor q1, q1, q0; \
    vmov.u32 d18[0], r7; \
    vmov q2, q0; \
    \
    vshr.s64 d16, d1, #63; \
    vshr.u64 d17, d0, #63; \
    vadd.u64 q0, q0, q0; \
    vand d16, d16, d18; \
    veor q0, q0, q8; \
    subs r4, r4, #1; \
    \
    do_aes_one##bits(d, imc, q1, q1, ##__VA_ARGS__); \
    \
    veor q1, q1, q2; \
    vst1.8 {q1}, [r1]!; /* store plaintext */ \
    \
    bne .Lxts_dec_loop_##bits; \
    b .Lxts_dec_done;

  CTR_XTS(128re, r0, r6)
  CTR_XTS(192, r0, r6)
  CTR_XTS(256, r0, r6)

#undef CTR_XTS

.Lxts_dec_done:
  vst1.8 {q0}, [r3] /* store tweak */

  CLEAR_REG(q0)
  CLEAR_REG(q1)
  CLEAR_REG(q2)
  CLEAR_REG(q3)
  CLEAR_REG(q8)
  CLEAR_REG(q9)
  CLEAR_REG(q10)
  CLEAR_REG(q11)
  CLEAR_REG(q12)
  CLEAR_REG(q13)
  CLEAR_REG(q14)

.Lxts_dec_skip:
  pop {r4-r12,lr}
  vpop {q4-q7}
  bx lr
.size _gcry_aes_xts_dec_armv8_ce,.-_gcry_aes_xts_dec_armv8_ce;


/*
 * u32 _gcry_aes_sbox4_armv8_ce(u32 in4b);
 */
.align 3
.globl _gcry_aes_sbox4_armv8_ce
.type  _gcry_aes_sbox4_armv8_ce,%function;
_gcry_aes_sbox4_armv8_ce:
  /* See "Gouvêa, C. P. L. & López, J. Implementing GCM on ARMv8. Topics in
   * Cryptology — CT-RSA 2015" for details.
   */
  vmov.i8 q0, #0x52
  vmov.i8 q1, #0
  vmov s0, r0
  aese.8 q0, q1
  veor d0, d1
  vpadd.i32 d0, d0, d1
  vmov r0, s0
  CLEAR_REG(q0)
  bx lr
.size _gcry_aes_sbox4_armv8_ce,.-_gcry_aes_sbox4_armv8_ce;


/*
 * void _gcry_aes_invmixcol_armv8_ce(void *dst, const void *src);
 */
.align 3
.globl _gcry_aes_invmixcol_armv8_ce
.type  _gcry_aes_invmixcol_armv8_ce,%function;
_gcry_aes_invmixcol_armv8_ce:
  vld1.8 {q0}, [r1]
  aesimc.8 q0, q0
  vst1.8 {q0}, [r0]
  CLEAR_REG(q0)
  bx lr
.size _gcry_aes_invmixcol_armv8_ce,.-_gcry_aes_invmixcol_armv8_ce;

#endif
