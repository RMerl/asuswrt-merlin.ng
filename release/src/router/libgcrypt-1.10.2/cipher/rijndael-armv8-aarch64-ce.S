/* rijndael-armv8-aarch64-ce.S - ARMv8/CE accelerated AES
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
    defined(HAVE_GCC_INLINE_ASM_AARCH64_CRYPTO)

.cpu generic+simd+crypto

.text


/* Register macros */

#define vk0 v17
#define vk1 v18
#define vk2 v19
#define vk3 v20
#define vk4 v21
#define vk5 v22
#define vk6 v23
#define vk7 v24
#define vk8 v25
#define vk9 v26
#define vk10 v27
#define vk11 v28
#define vk12 v29
#define vk13 v30
#define vklast v31


/* Helper macros */

#define __ /*_*/
#define _(...) __VA_ARGS__


/* AES macros */

#define aes_preload_keys(keysched, nrounds) \
	cmp nrounds, #12; \
	ld1 {vk0.16b-vk3.16b}, [keysched], #64; \
	ld1 {vk4.16b-vk7.16b}, [keysched], #64; \
	ld1 {vk8.16b-vk10.16b}, [keysched], #48; \
	mov vklast.16b, vk10.16b; \
	b.lo 1f; \
	ld1 {vk11.16b-vk12.16b}, [keysched], #32; \
	mov vklast.16b, vk12.16b; \
	b.eq 1f; \
	ld1 {vk13.16b-vklast.16b}, [keysched]; \
1:	;

#define do_aes_one_part1(ed, mcimc, vb, vkfirst) \
	aes##ed    vb.16b, vkfirst.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk1.16b; \
	aes##mcimc vb.16b, vb.16b;

#define do_aes_one_part2_128(ed, mcimc, vb, iop1, iop2) \
	aes##ed    vb.16b, vk2.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk3.16b; \
	aes##mcimc vb.16b, vb.16b; \
	iop1; \
	aes##ed    vb.16b, vk4.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk5.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk6.16b; \
	aes##mcimc vb.16b, vb.16b; \
	iop2; \
	aes##ed    vb.16b, vk7.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk8.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk9.16b;

#define do_aes_one_part2_192(ed, mcimc, vb, iop1, iop2) \
	aes##ed    vb.16b, vk2.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk3.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk4.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk5.16b; \
	aes##mcimc vb.16b, vb.16b; \
	iop1; \
	aes##ed    vb.16b, vk6.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk7.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk8.16b; \
	aes##mcimc vb.16b, vb.16b; \
	iop2; \
	aes##ed    vb.16b, vk9.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk10.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk11.16b;

#define do_aes_one_part2_256(ed, mcimc, vb, iop1, iop2) \
	aes##ed    vb.16b, vk2.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk3.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk4.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk5.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk6.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk7.16b; \
	aes##mcimc vb.16b, vb.16b; \
	iop1; \
	aes##ed    vb.16b, vk8.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk9.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk10.16b; \
	aes##mcimc vb.16b, vb.16b; \
	iop2; \
	aes##ed    vb.16b, vk11.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk12.16b; \
	aes##mcimc vb.16b, vb.16b; \
	aes##ed    vb.16b, vk13.16b;

#define do_aes_one128(ed, mcimc, vo, vb, vkfirst) \
	do_aes_one_part1(ed, mcimc, vb, vkfirst); \
	do_aes_one_part2_128(ed, mcimc, vb, __, __); \
	eor vo.16b, vb.16b, vklast.16b;

#define do_aes_one192(ed, mcimc, vo, vb, vkfirst) \
	do_aes_one_part1(ed, mcimc, vb, vkfirst); \
	do_aes_one_part2_192(ed, mcimc, vb, __, __); \
	eor vo.16b, vb.16b, vklast.16b;

#define do_aes_one256(ed, mcimc, vo, vb, vkfirst) \
	do_aes_one_part1(ed, mcimc, vb, vkfirst); \
	do_aes_one_part2_256(ed, mcimc, vb, __, __); \
	eor vo.16b, vb.16b, vklast.16b;

#define aes_round_4_multikey(ed, mcimc, b0, b1, b2, b3, key0, key1, key2, key3) \
	aes##ed    b0.16b, key0.16b; \
	aes##mcimc b0.16b, b0.16b; \
	  aes##ed    b1.16b, key1.16b; \
	  aes##mcimc b1.16b, b1.16b; \
	    aes##ed    b2.16b, key2.16b; \
	    aes##mcimc b2.16b, b2.16b; \
	      aes##ed    b3.16b, key3.16b; \
	      aes##mcimc b3.16b, b3.16b;

#define aes_round_4(ed, mcimc, b0, b1, b2, b3, key) \
	aes_round_4_multikey(ed, mcimc, b0, b1, b2, b3, key, key, key, key);

#define aes_lastround_4(ed, o0, o1, o2, o3, b0, b1, b2, b3, key1, b0_key2, b1_key2, b2_key2, b3_key2) \
	aes##ed    b0.16b, key1.16b; \
	  aes##ed    b1.16b, key1.16b; \
	    aes##ed    b2.16b, key1.16b; \
	      aes##ed    b3.16b, key1.16b; \
	eor        o0.16b, b0.16b, b0_key2.16b; \
	  eor        o1.16b, b1.16b, b1_key2.16b; \
	    eor        o2.16b, b2.16b, b2_key2.16b; \
	      eor        o3.16b, b3.16b, b3_key2.16b;

#define do_aes_4_part1_multikey(ed, mcimc, b0, b1, b2, b3, key0, key1, key2, key3) \
	aes_round_4_multikey(ed, mcimc, b0, b1, b2, b3, key0, key1, key2, key3); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk1); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk2); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk3);

#define do_aes_4_part1(ed, mcimc, b0, b1, b2, b3, vkfirst) \
	do_aes_4_part1_multikey(ed, mcimc, b0, b1, b2, b3, vkfirst, vkfirst, vkfirst, vkfirst);

#define do_aes_4_part2_128(ed, mcimc, o0, o1, o2, o3, b0, b1, b2, b3, \
			   b0_key, b1_key, b2_key, b3_key) \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk4); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk5); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk6); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk7); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk8); \
	aes_lastround_4(ed, o0, o1, o2, o3, b0, b1, b2, b3, vk9, b0_key, b1_key, b2_key, b3_key);

#define do_aes_4_part2_192(ed, mcimc, o0, o1, o2, o3, b0, b1, b2, b3, \
			   b0_key, b1_key, b2_key, b3_key) \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk4); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk5); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk6); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk7); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk8); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk9); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk10); \
	aes_lastround_4(ed, o0, o1, o2, o3, b0, b1, b2, b3, vk11, b0_key, b1_key, b2_key, b3_key);

#define do_aes_4_part2_256(ed, mcimc, o0, o1, o2, o3, b0, b1, b2, b3, \
			   b0_key, b1_key, b2_key, b3_key) \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk4); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk5); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk6); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk7); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk8); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk9); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk10); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk11); \
	aes_round_4(ed, mcimc, b0, b1, b2, b3, vk12); \
	aes_lastround_4(ed, o0, o1, o2, o3, b0, b1, b2, b3, vk13, b0_key, b1_key, b2_key, b3_key);

#define do_aes_4_128(ed, mcimc, b0, b1, b2, b3) \
	do_aes_4_part1(ed, mcimc, b0, b1, b2, b3, vk0); \
	do_aes_4_part2_128(ed, mcimc, b0, b1, b2, b3, b0, b1, b2, b3, vklast, vklast, vklast, vklast);

#define do_aes_4_192(ed, mcimc, b0, b1, b2, b3) \
	do_aes_4_part1(ed, mcimc, b0, b1, b2, b3, vk0); \
	do_aes_4_part2_192(ed, mcimc, b0, b1, b2, b3, b0, b1, b2, b3, vklast, vklast, vklast, vklast);

#define do_aes_4_256(ed, mcimc, b0, b1, b2, b3) \
	do_aes_4_part1(ed, mcimc, b0, b1, b2, b3, vk0); \
	do_aes_4_part2_256(ed, mcimc, b0, b1, b2, b3, b0, b1, b2, b3, vklast, vklast, vklast, vklast);

/* Other functional macros */

#define CLEAR_REG(reg) movi reg.16b, #0;

#define aes_clear_keys(nrounds) \
	CLEAR_REG(vk0); \
	CLEAR_REG(vk1); \
	CLEAR_REG(vk2); \
	CLEAR_REG(vk3); \
	CLEAR_REG(vk4); \
	CLEAR_REG(vk5); \
	CLEAR_REG(vk6); \
	CLEAR_REG(vk7); \
	CLEAR_REG(vk9); \
	CLEAR_REG(vk8); \
	CLEAR_REG(vk10); \
	CLEAR_REG(vk11); \
	CLEAR_REG(vk12); \
	CLEAR_REG(vk13); \
	CLEAR_REG(vklast);


/*
 * unsigned int _gcry_aes_enc_armv8_ce(void *keysched, byte *dst,
 *                                     const byte *src,
 *                                     unsigned int nrounds);
 */
.align 3
.globl _gcry_aes_enc_armv8_ce
ELF(.type  _gcry_aes_enc_armv8_ce,%function;)
_gcry_aes_enc_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: dst
   *    x2: src
   *    w3: nrounds
   */
  CFI_STARTPROC();

  aes_preload_keys(x0, w3);

  ld1 {v0.16b}, [x2]

  b.hi .Lenc1_256
  b.eq .Lenc1_192

.Lenc1_128:
  do_aes_one128(e, mc, v0, v0, vk0);

.Lenc1_tail:
  CLEAR_REG(vk0)
  CLEAR_REG(vk1)
  CLEAR_REG(vk2)
  CLEAR_REG(vk3)
  CLEAR_REG(vk4)
  CLEAR_REG(vk5)
  CLEAR_REG(vk6)
  CLEAR_REG(vk7)
  CLEAR_REG(vk8)
  CLEAR_REG(vk9)
  CLEAR_REG(vk10)
  CLEAR_REG(vklast)
  st1 {v0.16b}, [x1]
  CLEAR_REG(v0)

  mov x0, #0
  ret_spec_stop

.Lenc1_192:
  do_aes_one192(e, mc, v0, v0, vk0);

  CLEAR_REG(vk11)
  CLEAR_REG(vk12)
  b .Lenc1_tail

.Lenc1_256:
  do_aes_one256(e, mc, v0, v0, vk0);

  CLEAR_REG(vk11)
  CLEAR_REG(vk12)
  CLEAR_REG(vk13)
  b .Lenc1_tail
  CFI_ENDPROC();
ELF(.size _gcry_aes_enc_armv8_ce,.-_gcry_aes_enc_armv8_ce;)


/*
 * unsigned int _gcry_aes_dec_armv8_ce(void *keysched, byte *dst,
 *                                     const byte *src,
 *                                     unsigned int nrounds);
 */
.align 3
.globl _gcry_aes_dec_armv8_ce
ELF(.type  _gcry_aes_dec_armv8_ce,%function;)
_gcry_aes_dec_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: dst
   *    x2: src
   *    w3: nrounds
   */
  CFI_STARTPROC();

  aes_preload_keys(x0, w3);

  ld1 {v0.16b}, [x2]

  b.hi .Ldec1_256
  b.eq .Ldec1_192

.Ldec1_128:
  do_aes_one128(d, imc, v0, v0, vk0);

.Ldec1_tail:
  CLEAR_REG(vk0)
  CLEAR_REG(vk1)
  CLEAR_REG(vk2)
  CLEAR_REG(vk3)
  CLEAR_REG(vk4)
  CLEAR_REG(vk5)
  CLEAR_REG(vk6)
  CLEAR_REG(vk7)
  CLEAR_REG(vk8)
  CLEAR_REG(vk9)
  CLEAR_REG(vk10)
  CLEAR_REG(vklast)
  st1 {v0.16b}, [x1]
  CLEAR_REG(v0)

  mov x0, #0
  ret_spec_stop

.Ldec1_192:
  do_aes_one192(d, imc, v0, v0, vk0);

  CLEAR_REG(vk11)
  CLEAR_REG(vk12)
  b .Ldec1_tail

.Ldec1_256:
  do_aes_one256(d, imc, v0, v0, vk0);

  CLEAR_REG(vk11)
  CLEAR_REG(vk12)
  CLEAR_REG(vk13)
  b .Ldec1_tail
  CFI_ENDPROC();
ELF(.size _gcry_aes_dec_armv8_ce,.-_gcry_aes_dec_armv8_ce;)


/*
 * void _gcry_aes_cbc_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, size_t nblocks,
 *                                  int cbc_mac, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cbc_enc_armv8_ce
ELF(.type  _gcry_aes_cbc_enc_armv8_ce,%function;)
_gcry_aes_cbc_enc_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: outbuf
   *    x2: inbuf
   *    x3: iv
   *    x4: nblocks
   *    w5: cbc_mac
   *    w6: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lcbc_enc_skip

  cmp w5, #0
  ld1 {v4.16b}, [x3] /* load IV */
  csetm x5, eq

  aes_preload_keys(x0, w6);
  and x5, x5, #16

  ld1 {v3.16b}, [x2], #16; /* load plaintext */
  mov v0.16b, vk0.16b;
  sub x4, x4, #1;
  eor v16.16b, vk0.16b, vklast.16b;
  eor v4.16b, v4.16b, v3.16b;
  do_aes_one_part1(e, mc, v4, v0);

  b.eq .Lcbc_enc_entry_192
  b.hi .Lcbc_enc_entry_256

#define CBC_ENC(bits) \
  .Lcbc_enc_entry_##bits: \
    cbz x4, .Lcbc_enc_done_##bits; \
    \
  .Lcbc_enc_loop_##bits: \
    do_aes_one_part2_##bits(e, mc, v4, \
                            _(ld1 {v0.16b}, [x2], #16 /* load plaintext */), \
                            _(eor v0.16b, v0.16b, v16.16b)); \
    sub x4, x4, #1; \
    eor v3.16b, v4.16b, vklast.16b; \
    do_aes_one_part1(e, mc, v4, v0); \
    st1 {v3.16b}, [x1], x5; /* store ciphertext */ \
    cbnz x4, .Lcbc_enc_loop_##bits; \
    \
  .Lcbc_enc_done_##bits: \
    do_aes_one_part2_##bits(e, mc, v4, __, __); \
    b .Lcbc_enc_done;

  CBC_ENC(128)
  CBC_ENC(192)
  CBC_ENC(256)

#undef CBC_ENC

.Lcbc_enc_done:
  eor v3.16b, v4.16b, vklast.16b;
  st1 {v3.16b}, [x1]; /* store ciphertext */
  aes_clear_keys(w6)
  st1 {v3.16b}, [x3] /* store IV */

  CLEAR_REG(v16)
  CLEAR_REG(v4)
  CLEAR_REG(v3)
  CLEAR_REG(v0)

.Lcbc_enc_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_cbc_enc_armv8_ce,.-_gcry_aes_cbc_enc_armv8_ce;)

/*
 * void _gcry_aes_cbc_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cbc_dec_armv8_ce
ELF(.type  _gcry_aes_cbc_dec_armv8_ce,%function;)
_gcry_aes_cbc_dec_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: outbuf
   *    x2: inbuf
   *    x3: iv
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lcbc_dec_skip

  add sp, sp, #-64;
  CFI_ADJUST_CFA_OFFSET(64);

  ld1 {v16.16b}, [x3] /* load IV */

  aes_preload_keys(x0, w5);

  b.eq .Lcbc_dec_entry_192
  b.hi .Lcbc_dec_entry_256

#define CBC_DEC(bits) \
  .Lcbc_dec_entry_##bits: \
    cmp x4, #4; \
    b.lo .Lcbc_dec_loop_##bits; \
    \
    ld1 {v0.16b-v3.16b}, [x2], #64; /* load ciphertext */ \
    cmp x4, #8; \
    sub x4, x4, #4; \
    eor v4.16b, v16.16b, vklast.16b; \
    eor v5.16b, v0.16b, vklast.16b; \
    eor v6.16b, v1.16b, vklast.16b; \
    eor v7.16b, v2.16b, vklast.16b; \
    mov v16.16b, v3.16b; /* next IV */ \
    \
    do_aes_4_part1(d, imc, v0, v1, v2, v3, vk0); \
    b.lo .Lcbc_dec_done4_##bits; \
    \
    st1 {v8.16b-v11.16b}, [sp]; /* store callee saved registers */ \
    \
  .Lcbc_dec_loop4_##bits: \
    do_aes_4_part2_##bits(d, imc, v8, v9, v10, v11, v0, v1, v2, v3, v4, v5, v6, v7); \
    ld1 {v0.16b-v3.16b}, [x2], #64; /* load ciphertext */ \
    cmp x4, #8; \
    sub x4, x4, #4; \
    eor v4.16b, v16.16b, vklast.16b; \
    eor v5.16b, v0.16b, vklast.16b; \
    eor v6.16b, v1.16b, vklast.16b; \
    eor v7.16b, v2.16b, vklast.16b; \
    mov v16.16b, v3.16b; /* next IV */ \
    \
    do_aes_4_part1(d, imc, v0, v1, v2, v3, vk0); \
    st1 {v8.16b-v11.16b}, [x1], #64; /* store plaintext */ \
    \
    b.hs .Lcbc_dec_loop4_##bits; \
    \
    ld1 {v8.16b-v11.16b}, [sp]; /* restore callee saved registers */ \
    \
  .Lcbc_dec_done4_##bits: \
    do_aes_4_part2_##bits(d, imc, v0, v1, v2, v3, v0, v1, v2, v3, v4, v5, v6, v7); \
    \
    CLEAR_REG(v4); \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    CLEAR_REG(v7); \
    st1 {v0.16b-v3.16b}, [x1], #64; /* store plaintext */ \
    CLEAR_REG(v0); \
    CLEAR_REG(v3); \
    cbz x4, .Lcbc_dec_done; \
    \
  .Lcbc_dec_loop_##bits: \
    ld1 {v1.16b}, [x2], #16; /* load ciphertext */ \
    sub x4, x4, #1; \
    eor v16.16b, v16.16b, vklast.16b; \
    mov v2.16b, v1.16b; \
    \
    do_aes_one_part1(d, imc, v1, vk0); \
    do_aes_one_part2_##bits(d, imc, v1, __, __); \
    eor v1.16b, v1.16b, v16.16b; \
    \
    mov v16.16b, v2.16b; \
    st1 {v1.16b}, [x1], #16; /* store plaintext */ \
    \
    cbnz x4, .Lcbc_dec_loop_##bits; \
    b .Lcbc_dec_done;

  CBC_DEC(128)
  CBC_DEC(192)
  CBC_DEC(256)

#undef CBC_DEC

.Lcbc_dec_done:
  aes_clear_keys(w5)

  st1 {v16.16b}, [x3] /* store IV */

  CLEAR_REG(v16)
  CLEAR_REG(v1)
  CLEAR_REG(v2)

  add sp, sp, #64;
  CFI_ADJUST_CFA_OFFSET(-64);

.Lcbc_dec_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_cbc_dec_armv8_ce,.-_gcry_aes_cbc_dec_armv8_ce;)


/*
 * void _gcry_aes_ctr_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_ctr_enc_armv8_ce
ELF(.type  _gcry_aes_ctr_enc_armv8_ce,%function;)
_gcry_aes_ctr_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lctr_enc_skip

  add x8, sp, #-64
  add sp, sp, #-128;
  CFI_ADJUST_CFA_OFFSET(128);

  mov w6, #(1 << 24)
  movi v16.16b, #0
  mov v16.S[3], w6 /* 1 */

  /* load IV */
  ldp x9, x10, [x3]
  ld1 {v0.16b}, [x3]
  rev x9, x9
  rev x10, x10

  mov x12, #(4 << 56)
  lsl x11, x10, #56

  aes_preload_keys(x0, w5);

  b.eq .Lctr_enc_entry_192
  b.hi .Lctr_enc_entry_256

#define CTR_ENC(bits) \
  .Lctr_enc_entry_##bits: \
    cmp x4, #4; \
    b.lo .Lctr_enc_loop_##bits; \
    \
    st1 {v8.16b-v11.16b}, [sp]; /* store callee saved registers */ \
    \
    adds x11, x11, x12; \
    add v9.4s, v16.4s, v16.4s; /* 2 */ \
    add v10.4s, v16.4s, v9.4s; /* 3 */ \
    add v11.4s, v9.4s, v9.4s; /* 4 */ \
    mov x7, #1; \
    sub x4, x4, #4; \
    ld1 {v5.16b-v8.16b}, [x2], #64; /* preload ciphertext */ \
    b.cs .Lctr_enc_carry4_##bits; \
    \
    mov v1.16b, v0.16b; \
    add x10, x10, #4; \
    add v2.16b, v0.16b, v16.16b; \
    add v3.8h, v0.8h, v9.8h; \
    add v4.4s, v0.4s, v10.4s; \
    add v0.2d, v0.2d, v11.2d; \
    \
  .Lctr_enc_entry4_##bits##_carry_done: \
    mov x7, #0; \
    cmp x4, #4; \
    do_aes_4_part1(e, mc, v1, v2, v3, v4, vk0); \
    b.lo .Lctr_enc_done4_##bits; \
    \
    st1 {v12.16b-v15.16b}, [x8]; /* store callee saved registers */ \
    \
  .Lctr_enc_loop4_##bits: \
    eor v5.16b, v5.16b, vklast.16b; \
    eor v6.16b, v6.16b, vklast.16b; \
    eor v7.16b, v7.16b, vklast.16b; \
    eor v8.16b, v8.16b, vklast.16b; \
    do_aes_4_part2_##bits(e, mc, v12, v13, v14, v15, v1, v2, v3, v4, v5, v6, v7, v8); \
    ld1 {v5.16b-v8.16b}, [x2], #64; /* preload ciphertext */ \
    adds x11, x11, x12; \
    sub x4, x4, #4; \
    b.cs .Lctr_enc_carry4_##bits; \
    \
    mov v1.16b, v0.16b; \
    add x10, x10, #4; \
    add v2.16b, v0.16b, v16.16b; \
    add v3.8h, v0.8h, v9.8h; \
    add v4.4s, v0.4s, v10.4s; \
    add v0.2d, v0.2d, v11.2d; \
    \
  .Lctr_enc_loop4_##bits##_carry_done: \
    cmp x4, #4; \
    do_aes_4_part1(e, mc, v1, v2, v3, v4, vk0); \
    st1 {v12.16b-v15.16b}, [x1], #64; /* store plaintext */ \
    \
    b.hs .Lctr_enc_loop4_##bits; \
    \
    ld1 {v12.16b-v15.16b}, [x8]; /* restore callee saved registers */ \
    \
  .Lctr_enc_done4_##bits: \
    eor v5.16b, v5.16b, vklast.16b; \
    eor v6.16b, v6.16b, vklast.16b; \
    eor v7.16b, v7.16b, vklast.16b; \
    eor v8.16b, v8.16b, vklast.16b; \
    do_aes_4_part2_##bits(e, mc, v5, v6, v7, v8, v1, v2, v3, v4, v5, v6, v7, v8); \
    \
    st1 {v5.16b-v8.16b}, [x1], #64; /* store plaintext */ \
    \
    CLEAR_REG(v3); \
    CLEAR_REG(v4); \
    ld1 {v8.16b-v11.16b}, [sp]; /* restore callee saved registers */ \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    CLEAR_REG(v7); \
    cbz x4, .Lctr_enc_done; \
    \
  .Lctr_enc_loop_##bits: \
    \
    adds x10, x10, #1; \
    mov v1.16b, v0.16b; \
    adc x9, x9, xzr; \
    dup v0.2d, x10; \
    sub x4, x4, #1; \
    ins v0.D[0], x9; \
    ld1 {v2.16b}, [x2], #16; /* load ciphertext */ \
    rev64 v0.16b, v0.16b; \
    \
    do_aes_one_part1(e, mc, v1, vk0); \
    eor v2.16b, v2.16b, vklast.16b; \
    do_aes_one_part2_##bits(e, mc, v1, __, __); \
    \
    eor v1.16b, v1.16b, v2.16b; \
    st1 {v1.16b}, [x1], #16; /* store plaintext */ \
    \
    cbnz x4, .Lctr_enc_loop_##bits; \
    b .Lctr_enc_done; \
    \
  .Lctr_enc_carry4_##bits: \
    \
    adds x13, x10, #1; \
    mov v1.16b, v0.16b; \
    adc x14, x9, xzr; \
    dup v2.2d, x13; \
      adds x13, x10, #2; \
    ins v2.D[0], x14; \
      adc x14, x9, xzr; \
    rev64 v2.16b, v2.16b; \
      dup v3.2d, x13; \
	adds x13, x10, #3; \
      ins v3.D[0], x14; \
	adc x14, x9, xzr; \
      rev64 v3.16b, v3.16b; \
	dup v4.2d, x13; \
	  adds x10, x10, #4; \
	ins v4.D[0], x14; \
	  adc x9, x9, xzr; \
	rev64 v4.16b, v4.16b; \
	  dup v0.2d, x10; \
	  ins v0.D[0], x9; \
	  rev64 v0.16b, v0.16b; \
    \
    cbz x7, .Lctr_enc_loop4_##bits##_carry_done; \
    b .Lctr_enc_entry4_##bits##_carry_done;

  CTR_ENC(128)
  CTR_ENC(192)
  CTR_ENC(256)

#undef CTR_ENC

.Lctr_enc_done:
  aes_clear_keys(w5)

  st1 {v0.16b}, [x3] /* store IV */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v16)

  add sp, sp, #128;
  CFI_ADJUST_CFA_OFFSET(-128);

.Lctr_enc_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_ctr_enc_armv8_ce,.-_gcry_aes_ctr_enc_armv8_ce;)


/*
 * void _gcry_aes_ctr32le_enc_armv8_ce (const void *keysched,
 *                                      unsigned char *outbuf,
 *                                      const unsigned char *inbuf,
 *                                      unsigned char *iv,
 *                                      unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_ctr32le_enc_armv8_ce
ELF(.type  _gcry_aes_ctr32le_enc_armv8_ce,%function;)
_gcry_aes_ctr32le_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lctr32le_enc_skip

  add x8, sp, #-64
  add sp, sp, #-128;
  CFI_ADJUST_CFA_OFFSET(128);

  mov w6, #1
  movi v16.16b, #0
  mov v16.S[0], w6

  /* load IV */
  ld1 {v0.16b}, [x3]

  aes_preload_keys(x0, w5);

  b.eq .Lctr32le_enc_entry_192
  b.hi .Lctr32le_enc_entry_256

#define CTR32LE_ENC(bits) \
  .Lctr32le_enc_entry_##bits: \
    cmp x4, #4; \
    b.lo .Lctr32le_enc_loop_##bits; \
    \
    st1 {v8.16b-v11.16b}, [sp]; /* store callee saved registers */ \
    add v9.4s, v16.4s, v16.4s; /* 2 */ \
    cmp x4, #8; \
    add v10.4s, v9.4s, v16.4s; /* 3 */ \
    sub x4, x4, #4; \
    add v11.4s, v9.4s, v9.4s;  /* 4 */ \
    \
    ld1 {v5.16b-v8.16b}, [x2], #64; /* preload ciphertext */ \
    \
    mov v1.16b, v0.16b; \
    add v2.4s, v0.4s, v16.4s; \
    add v3.4s, v0.4s, v9.4s; \
    add v4.4s, v0.4s, v10.4s; \
    add v0.4s, v0.4s, v11.4s; \
    \
    do_aes_4_part1(e, mc, v1, v2, v3, v4, vk0); \
    b.lo .Lctr32le_enc_done4_##bits; \
    \
    st1 {v12.16b-v15.16b}, [x8]; /* store callee saved registers */ \
    \
  .Lctr32le_enc_loop4_##bits: \
    eor v5.16b, v5.16b, vklast.16b; \
    eor v6.16b, v6.16b, vklast.16b; \
    eor v7.16b, v7.16b, vklast.16b; \
    eor v8.16b, v8.16b, vklast.16b; \
    do_aes_4_part2_##bits(e, mc, v12, v13, v14, v15, v1, v2, v3, v4, v5, v6, v7, v8); \
    ld1 {v5.16b-v8.16b}, [x2], #64; /* preload ciphertext */ \
    \
    cmp x4, #8; \
    sub x4, x4, #4; \
    \
    mov v1.16b, v0.16b; \
    add v2.4s, v0.4s, v16.4s; \
    add v3.4s, v0.4s, v9.4s; \
    add v4.4s, v0.4s, v10.4s; \
    add v0.4s, v0.4s, v11.4s; \
    \
    do_aes_4_part1(e, mc, v1, v2, v3, v4, vk0); \
    st1 {v12.16b-v15.16b}, [x1], #64; /* store plaintext */ \
    \
    b.hs .Lctr32le_enc_loop4_##bits; \
    \
    ld1 {v12.16b-v15.16b}, [x8]; /* restore callee saved registers */ \
    \
  .Lctr32le_enc_done4_##bits: \
    eor v5.16b, v5.16b, vklast.16b; \
    eor v6.16b, v6.16b, vklast.16b; \
    eor v7.16b, v7.16b, vklast.16b; \
    eor v8.16b, v8.16b, vklast.16b; \
    do_aes_4_part2_##bits(e, mc, v5, v6, v7, v8, v1, v2, v3, v4, v5, v6, v7, v8); \
    \
    st1 {v5.16b-v8.16b}, [x1], #64; /* store plaintext */ \
    CLEAR_REG(v3); \
    CLEAR_REG(v4); \
    ld1 {v8.16b-v11.16b}, [sp]; /* restore callee saved registers */ \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    CLEAR_REG(v7); \
    cbz x4, .Lctr32le_enc_done; \
    \
  .Lctr32le_enc_loop_##bits: \
    \
    mov v1.16b, v0.16b; \
    ld1 {v2.16b}, [x2], #16; /* load ciphertext */ \
    sub x4, x4, #1; \
    add v0.4s, v0.4s, v16.4s; \
    \
    do_aes_one_part1(e, mc, v1, vk0); \
    eor v2.16b, v2.16b, vklast.16b; \
    do_aes_one_part2_##bits(e, mc, v1, __, __); \
    \
    eor v1.16b, v1.16b, v2.16b; \
    st1 {v1.16b}, [x1], #16; /* store plaintext */ \
    \
    cbnz x4, .Lctr32le_enc_loop_##bits; \
    b .Lctr32le_enc_done;

  CTR32LE_ENC(128)
  CTR32LE_ENC(192)
  CTR32LE_ENC(256)

#undef CTR32LE_ENC

.Lctr32le_enc_done:
  aes_clear_keys(w5)

  st1 {v0.16b}, [x3] /* store IV */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v16)

  add sp, sp, #128;
  CFI_ADJUST_CFA_OFFSET(-128);

.Lctr32le_enc_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_ctr32le_enc_armv8_ce,.-_gcry_aes_ctr32le_enc_armv8_ce;)


/*
 * void _gcry_aes_cfb_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cfb_enc_armv8_ce
ELF(.type  _gcry_aes_cfb_enc_armv8_ce,%function;)
_gcry_aes_cfb_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lcfb_enc_skip

  /* load IV */
  ld1 {v0.16b}, [x3]

  aes_preload_keys(x0, w5);

  ld1 {v1.16b}, [x2], #16; /* load plaintext */
  eor v3.16b, vk0.16b, vklast.16b;
  eor v0.16b, v0.16b, vklast.16b;
  sub x4, x4, #1;
  mov v4.16b, v3.16b;
  do_aes_one_part1(e, mc, v0, v4);

  b.eq .Lcfb_enc_entry_192
  b.hi .Lcfb_enc_entry_256

#define CFB_ENC(bits) \
  .Lcfb_enc_entry_##bits: \
    cbz x4, .Lcfb_enc_done_##bits; \
    \
  .Lcfb_enc_loop_##bits: \
    eor v2.16b, v1.16b, vklast.16b; \
    do_aes_one_part2_##bits(e, mc, v0, \
			    _(eor v4.16b, v3.16b, v1.16b), \
			    _(ld1 {v1.16b}, [x2], #16 /* load plaintext */)); \
    sub x4, x4, #1; \
    eor v2.16b, v2.16b, v0.16b; \
    do_aes_one_part1(e, mc, v0, v4); \
    st1 {v2.16b}, [x1], #16; /* store ciphertext */ \
    cbnz x4, .Lcfb_enc_loop_##bits; \
    \
  .Lcfb_enc_done_##bits: \
    eor v2.16b, v1.16b, vklast.16b; \
    do_aes_one_part2_##bits(e, mc, v0, __, __); \
    b .Lcfb_enc_done;

  CFB_ENC(128)
  CFB_ENC(192)
  CFB_ENC(256)

#undef CFB_ENC

.Lcfb_enc_done:
  eor v2.16b, v2.16b, v0.16b;
  st1 {v2.16b}, [x1]; /* store ciphertext */
  aes_clear_keys(w5)
  st1 {v2.16b}, [x3] /* store IV */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v3)
  CLEAR_REG(v4)

.Lcfb_enc_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_cfb_enc_armv8_ce,.-_gcry_aes_cfb_enc_armv8_ce;)


/*
 * void _gcry_aes_cfb_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *iv, unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_cfb_dec_armv8_ce
ELF(.type  _gcry_aes_cfb_dec_armv8_ce,%function;)
_gcry_aes_cfb_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: iv
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lcfb_dec_skip

  add sp, sp, #-64;
  CFI_ADJUST_CFA_OFFSET(64);

  /* load IV */
  ld1 {v0.16b}, [x3]

  aes_preload_keys(x0, w5);

  b.eq .Lcfb_dec_entry_192
  b.hi .Lcfb_dec_entry_256

#define CFB_DEC(bits) \
  .Lcfb_dec_entry_##bits: \
    cmp x4, #4; \
    b.lo .Lcfb_dec_loop_##bits; \
    \
    ld1 {v2.16b-v5.16b}, [x2], #64; /* load ciphertext */ \
    cmp x4, #8; \
    mov v1.16b, v0.16b; \
    sub x4, x4, #4; \
    eor v6.16b, v2.16b, vklast.16b; \
    eor v7.16b, v3.16b, vklast.16b; \
    eor v16.16b, v4.16b, vklast.16b; \
    mov v0.16b, v5.16b; /* next IV */ \
    eor v5.16b, v5.16b, vklast.16b; \
    \
    do_aes_4_part1(e, mc, v1, v2, v3, v4, vk0); \
    b.lo .Lcfb_dec_done4_##bits; \
    \
    st1 {v8.16b-v11.16b}, [sp]; /* store callee saved registers */ \
    \
  .Lcfb_dec_loop4_##bits: \
    do_aes_4_part2_##bits(e, mc, v8, v9, v10, v11, v1, v2, v3, v4, v6, v7, v16, v5); \
    ld1 {v2.16b-v5.16b}, [x2], #64; /* load ciphertext */ \
    cmp x4, #8; \
    mov v1.16b, v0.16b; \
    sub x4, x4, #4; \
    eor v6.16b, v2.16b, vklast.16b; \
    eor v7.16b, v3.16b, vklast.16b; \
    eor v16.16b, v4.16b, vklast.16b; \
    mov v0.16b, v5.16b; /* next IV */ \
    eor v5.16b, v5.16b, vklast.16b; \
    \
    do_aes_4_part1(e, mc, v1, v2, v3, v4, vk0); \
    st1 {v8.16b-v11.16b}, [x1], #64; /* store plaintext */ \
    \
    b.hs .Lcfb_dec_loop4_##bits; \
    \
    ld1 {v8.16b-v11.16b}, [sp]; /* restore callee saved registers */ \
    \
  .Lcfb_dec_done4_##bits: \
    do_aes_4_part2_##bits(e, mc, v1, v2, v3, v4, v1, v2, v3, v4, v6, v7, v16, v5); \
    \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    CLEAR_REG(v7); \
    st1 {v1.16b-v4.16b}, [x1], #64; /* store plaintext */ \
    CLEAR_REG(v3); \
    CLEAR_REG(v4); \
    cbz x4, .Lcfb_dec_done; \
    \
  .Lcfb_dec_loop_##bits: \
    ld1 {v1.16b}, [x2], #16; /* load ciphertext */ \
    sub x4, x4, #1; \
    \
    do_aes_one_part1(e, mc, v0, vk0); \
    eor v2.16b, v1.16b, vklast.16b; \
    do_aes_one_part2_##bits(e, mc, v0, __, __); \
    eor v2.16b, v2.16b, v0.16b; \
    \
    mov v0.16b, v1.16b; \
    st1 {v2.16b}, [x1], #16; /* store plaintext */ \
    \
    cbnz x4, .Lcfb_dec_loop_##bits; \
    b .Lcfb_dec_done;

  CFB_DEC(128)
  CFB_DEC(192)
  CFB_DEC(256)

#undef CFB_DEC

.Lcfb_dec_done:
  aes_clear_keys(w5)

  st1 {v0.16b}, [x3] /* store IV */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v16)

  add sp, sp, #64;
  CFI_ADJUST_CFA_OFFSET(-64);

.Lcfb_dec_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_cfb_dec_armv8_ce,.-_gcry_aes_cfb_dec_armv8_ce;)


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
ELF(.type  _gcry_aes_ocb_enc_armv8_ce,%function;)
_gcry_aes_ocb_enc_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: outbuf
   *    x2: inbuf
   *    x3: offset
   *    x4: checksum
   *    x5: Ltable
   *    x6: nblocks (0 < nblocks)
   *    w7: nrounds
   *    %st+0: blkn => w12
   */
  CFI_STARTPROC();

  ldr w12, [sp]
  ld1 {v0.16b}, [x3] /* load offset */
  ld1 {v16.16b}, [x4] /* load checksum */

  add x16, sp, #-64;
  add sp, sp, #-128;
  CFI_ADJUST_CFA_OFFSET(128);

  aes_preload_keys(x0, w7);

  st1 {v8.16b-v11.16b}, [sp]; /* store callee saved registers */

  eor v0.16b, v0.16b, vk0.16b; /* offset ^ first key */
  eor v9.16b, vk0.16b, vklast.16b; /* first key ^ last key */

  b.eq .Locb_ecry_entry_192
  b.hi .Locb_ecry_entry_256

#define OCB_CRYPT(bits, ed, mcimc) \
  .Locb_##ed##cry_entry_##bits: \
    /* Get number of blocks to align nblk to 4. */ \
    neg x13, x12; \
    add x12, x12, #1; /* Pre-increment nblk for ntz calculation */ \
    and x13, x13, #(4-1); \
    cmp x13, x6; \
    csel x13, x6, x13, hi; \
    cbz x13, .Locb_##ed##cry_alignment_ok_##bits; \
    \
    /* Number of blocks after alignment. */ \
    sub x14, x6, x13; \
    \
    /* If number after alignment is less than 4, skip aligned handling \
     * completely. */ \
    cmp x14, #4; \
    csel x13, x6, x13, lo; \
    \
  .Locb_##ed##cry_unaligned_entry_##bits: \
    cmp x13, #4; \
    \
  .Locb_##ed##cry_loop1_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */ \
    \
    rbit x8, x12; \
    add x12, x12, #1; \
    clz x8, x8; /* ntz(i) */ \
    add x8, x5, x8, lsl #4; \
    \
    ld1 {v1.16b}, [x2], #16; /* load plaintext */ \
    ld1 {v2.16b}, [x8]; /* load L_{ntz(i)} */ \
    eor v0.16b, v0.16b, v2.16b; \
    sub x13, x13, #1; \
    ENC(eor v16.16b, v16.16b, v1.16b); \
    sub x6, x6, #1; \
    \
    do_aes_one_part1(ed, mcimc, v1, v0); \
    eor v2.16b, v0.16b, v9.16b; \
    do_aes_one_part2_##bits(ed, mcimc, v1, __, __); \
    eor v1.16b, v1.16b, v2.16b; \
    st1 {v1.16b}, [x1], #16; /* store ciphertext */ \
    DEC(eor v16.16b, v16.16b, v1.16b); \
    \
    cbnz x13, .Locb_##ed##cry_loop1_##bits; \
    \
    cbz x6, .Locb_##ed##cry_done; \
    \
    /* nblk is now aligned and we have 4 or more blocks. So jump directly to \
     * aligned processing. */ \
    b .Locb_##ed##cry_aligned_entry_##bits; \
    \
  .Locb_##ed##cry_alignment_ok_##bits: \
    cbz x6, .Locb_##ed##cry_done; \
    \
    /* Short buffers do not benefit from L-array optimization. */ \
    cmp x6, #4; \
    mov x13, x6; \
    b.lo .Locb_##ed##cry_unaligned_entry_##bits; \
    \
  .Locb_##ed##cry_aligned_entry_##bits: \
    /* Prepare L-array optimization. \
     * Since nblk is aligned to 4, offsets will have following construction: \
     *  - block1 = ntz{0} = offset ^ L[0] \
     *  - block2 = ntz{1} = offset ^ L[0] ^ L[1] \
     *  - block3 = ntz{0} = offset ^ L[1] \
     *  - block4 = ntz{x} = offset ^ L[1] ^ L[ntz{x}] \
     */ \
    ld1 {v10.16b-v11.16b}, [x5];        /* preload L[0] && L[1] */ \
    mov x15, #4; \
    \
    st1 {v12.16b-v15.16b}, [x16]; /* store callee saved registers */ \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */ \
    \
    add x11, x12, #3; \
    ld1 {v1.16b-v4.16b}, [x2], #64;     /* load P_i+<0-3> */ \
    rbit x11, x11; \
    eor v6.16b, v10.16b, v11.16b;       /* L[0] ^ L[1] */ \
    ENC(eor v16.16b, v16.16b, v1.16b);  /* Checksum_i+0 */ \
    add x12, x12, #4; \
    clz x11, x11; /* ntz(i+3) */ \
    add x15, x15, #4; \
    add x11, x5, x11, lsl #4; \
    \
    eor v5.16b, v0.16b, v10.16b;        /* Offset_i+0 */ \
    ENC(eor v16.16b, v16.16b, v2.16b);  /* Checksum_i+1 */ \
    ld1 {v8.16b}, [x11];                /* load L_{ntz(i+3)} */ \
    ENC(eor v16.16b, v16.16b, v3.16b);  /* Checksum_i+2 */ \
    eor v6.16b, v0.16b, v6.16b;         /* Offset_i+1 */ \
    ENC(eor v16.16b, v16.16b, v4.16b);  /* Checksum_i+3 */ \
    eor v7.16b, v0.16b, v11.16b;        /* Offset_i+2 */ \
    eor v8.16b, v8.16b, v11.16b;        /* L[1] ^ L[ntz{x}] */ \
    cmp x15, x13; \
    eor v0.16b, v0.16b, v8.16b;         /* Offset_i+3 */ \
    \
    do_aes_4_part1_multikey(ed, mcimc, v1, v2, v3, v4, v5, v6, v7, v0); /* P_i+j xor Offset_i+j */ \
    b.hi .Locb_##ed##cry_aligned_done4_##bits; \
    \
  .Locb_##ed##cry_aligned_loop4_##bits: \
    add x11, x12, #3; \
    eor v5.16b, v5.16b, v9.16b; \
    eor v6.16b, v6.16b, v9.16b; \
    rbit x11, x11; \
    eor v7.16b, v7.16b, v9.16b; \
    eor v8.16b, v0.16b, v9.16b; \
    clz x11, x11; /* ntz(i+3) */ \
    do_aes_4_part2_##bits(ed, mcimc, v12, v13, v14, v15, v1, v2, v3, v4, v5, v6, v7, v8); /* xor Offset_i+j */ \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Checksum_i = Checksum_{i-1} xor P_i  */ \
    /* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */ \
    \
    add x12, x12, #4; \
    ld1 {v1.16b-v4.16b}, [x2], #64;     /* load P_i+<0-3> */ \
    eor v6.16b, v10.16b, v11.16b;       /* L[0] ^ L[1] */ \
    add x15, x15, #4; \
    DEC(eor v16.16b, v16.16b, v12.16b); /* Checksum_i+0 */ \
    ENC(eor v16.16b, v16.16b, v1.16b);  /* Checksum_i+0 */ \
    add x11, x5, x11, lsl #4; \
    \
    eor v5.16b, v0.16b, v10.16b;        /* Offset_i+0 */ \
    ENC(eor v16.16b, v16.16b, v2.16b);  /* Checksum_i+1 */ \
    DEC(eor v16.16b, v16.16b, v13.16b); /* Checksum_1+2 */ \
    ld1 {v8.16b}, [x11];                /* load L_{ntz(i+3)} */ \
    ENC(eor v16.16b, v16.16b, v3.16b);  /* Checksum_i+2 */ \
    DEC(eor v16.16b, v16.16b, v14.16b); /* Checksum_i+0+3 */ \
    eor v6.16b, v0.16b, v6.16b;         /* Offset_i+1 */ \
    ENC(eor v16.16b, v16.16b, v4.16b);  /* Checksum_i+3 */ \
    DEC(eor v16.16b, v16.16b, v15.16b); /* Checksum_i+0+1+2 */ \
    eor v7.16b, v0.16b, v11.16b;        /* Offset_i+2 */ \
    eor v8.16b, v8.16b, v11.16b;        /* L[1] ^ L[ntz{x}] */ \
    cmp x15, x13; \
    eor v0.16b, v0.16b, v8.16b;         /* Offset_i+3 */ \
    \
    do_aes_4_part1_multikey(ed, mcimc, v1, v2, v3, v4, v5, v6, v7, v0); /* P_i+j xor Offset_i+j */ \
    st1 {v12.16b-v15.16b}, [x1], #64; \
    \
    b.ls .Locb_##ed##cry_aligned_loop4_##bits; \
    \
  .Locb_##ed##cry_aligned_done4_##bits: \
    eor v5.16b, v5.16b, v9.16b; \
    eor v6.16b, v6.16b, v9.16b; \
    eor v7.16b, v7.16b, v9.16b; \
    eor v8.16b, v0.16b, v9.16b; \
    do_aes_4_part2_##bits(ed, mcimc, v1, v2, v3, v4, v1, v2, v3, v4, v5, v6, v7, v8); /* xor Offset_i+j */ \
    DEC(eor v16.16b, v16.16b, v1.16b);  /* Checksum_i+0 */ \
    DEC(eor v5.16b, v2.16b, v3.16b);    /* Checksum_1+2 */ \
    DEC(eor v16.16b, v16.16b, v4.16b);  /* Checksum_i+0+3 */ \
    st1 {v1.16b-v4.16b}, [x1], #64; \
    DEC(eor v16.16b, v16.16b, v5.16b);  /* Checksum_i+0+1+2 */ \
    \
    sub x15, x15, #4; \
    CLEAR_REG(v3); \
    CLEAR_REG(v4); \
    ld1 {v12.16b-v15.16b}, [x16]; /* restore callee saved registers */ \
    sub x13, x13, x15; \
    sub x6, x6, x15; \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    \
    /* Handle tailing 1…3 blocks in unaligned loop. */ \
    mov x13, x6; \
    cbnz x6, .Locb_##ed##cry_unaligned_entry_##bits; \
    \
    b .Locb_##ed##cry_done;

#define ENC(...) __VA_ARGS__
#define DEC(...) /*_*/
  OCB_CRYPT(128, e, mc)
  OCB_CRYPT(192, e, mc)
  OCB_CRYPT(256, e, mc)
#undef ENC
#undef DEC

.Locb_ecry_done:
  eor v0.16b, v0.16b, vk0.16b; /* restore offset */

  ld1 {v8.16b-v11.16b}, [sp]; /* restore callee saved registers */
  aes_clear_keys(w7)

  st1 {v16.16b}, [x4] /* store checksum */
  st1 {v0.16b}, [x3] /* store offset */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v7)
  CLEAR_REG(v16)

  add sp, sp, #128;
  CFI_ADJUST_CFA_OFFSET(-128);

  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_ocb_enc_armv8_ce,.-_gcry_aes_ocb_enc_armv8_ce;)


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
ELF(.type  _gcry_aes_ocb_dec_armv8_ce,%function;)
_gcry_aes_ocb_dec_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: outbuf
   *    x2: inbuf
   *    x3: offset
   *    x4: checksum
   *    x5: Ltable
   *    x6: nblocks (0 < nblocks)
   *    w7: nrounds
   *    %st+0: blkn => w12
   */
  CFI_STARTPROC();

  ldr w12, [sp]
  ld1 {v0.16b}, [x3] /* load offset */
  ld1 {v16.16b}, [x4] /* load checksum */

  add x16, sp, #-64;
  add sp, sp, #-128;
  CFI_ADJUST_CFA_OFFSET(128);

  aes_preload_keys(x0, w7);

  st1 {v8.16b-v11.16b}, [sp]; /* store callee saved registers */

  eor v0.16b, v0.16b, vk0.16b; /* offset ^ first key */
  eor v9.16b, vk0.16b, vklast.16b; /* first key ^ last key */

  b.eq .Locb_dcry_entry_192
  b.hi .Locb_dcry_entry_256

#define ENC(...) /*_*/
#define DEC(...) __VA_ARGS__
  OCB_CRYPT(128, d, imc)
  OCB_CRYPT(192, d, imc)
  OCB_CRYPT(256, d, imc)
#undef ENC
#undef DEC

#undef OCB_CRYPT

.Locb_dcry_done:
  eor v0.16b, v0.16b, vk0.16b; /* restore offset */

  ld1 {v8.16b-v11.16b}, [sp]; /* restore callee saved registers */
  aes_clear_keys(w7)

  st1 {v16.16b}, [x4] /* store checksum */
  st1 {v0.16b}, [x3] /* store offset */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v16)

  add sp, sp, #128;
  CFI_ADJUST_CFA_OFFSET(-128);

  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_ocb_dec_armv8_ce,.-_gcry_aes_ocb_dec_armv8_ce;)


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
ELF(.type  _gcry_aes_ocb_auth_armv8_ce,%function;)
_gcry_aes_ocb_auth_armv8_ce:
  /* input:
   *    x0: keysched
   *    x1: abuf
   *    x2: offset => x3
   *    x3: checksum => x4
   *    x4: Ltable => x5
   *    x5: nblocks => x6  (0 < nblocks <= 32)
   *    w6: nrounds => w7
   *    w7: blkn => w12
   */
  CFI_STARTPROC();

  mov w12, w7
  mov w7, w6
  mov x6, x5
  mov x5, x4
  mov x4, x3
  mov x3, x2

  aes_preload_keys(x0, w7);

  ld1 {v0.16b}, [x3] /* load offset */
  ld1 {v16.16b}, [x4] /* load checksum */

  beq .Locb_auth_entry_192
  bhi .Locb_auth_entry_256

#define OCB_AUTH(bits) \
  .Locb_auth_entry_##bits: \
    cmp x6, #4; \
    add w12, w12, #1; \
    b.lo .Locb_auth_loop_##bits; \
    \
  .Locb_auth_loop4_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */ \
    \
    add w9, w12, #1; \
    add w10, w12, #2; \
    add w11, w12, #3; \
    rbit w8, w12; \
    add w12, w12, #4; \
    rbit w9, w9; \
    rbit w10, w10; \
    rbit w11, w11; \
    clz w8, w8; /* ntz(i+0) */ \
    clz w9, w9; /* ntz(i+1) */ \
    clz w10, w10; /* ntz(i+2) */ \
    clz w11, w11; /* ntz(i+3) */ \
    add x8, x5, x8, lsl #4; \
    ld1 {v1.16b-v4.16b}, [x1], #64;   /* load A_i+<0-3> */ \
    add x9, x5, x9, lsl #4; \
    add x10, x5, x10, lsl #4; \
    add x11, x5, x11, lsl #4; \
    \
    sub x6, x6, #4; \
    \
    ld1 {v5.16b}, [x8];               /* load L_{ntz(i+0)} */ \
    ld1 {v6.16b}, [x9];               /* load L_{ntz(i+1)} */ \
    ld1 {v7.16b}, [x10];              /* load L_{ntz(i+2)} */ \
    eor v5.16b, v5.16b, v0.16b;       /* Offset_i+0 */ \
    ld1 {v0.16b}, [x11];              /* load L_{ntz(i+3)} */ \
    eor v6.16b, v6.16b, v5.16b;       /* Offset_i+1 */ \
    eor v1.16b, v1.16b, v5.16b;       /* A_i+0 xor Offset_i+0 */ \
    eor v7.16b, v7.16b, v6.16b;       /* Offset_i+2 */ \
    eor v2.16b, v2.16b, v6.16b;       /* A_i+1 xor Offset_i+1 */ \
    eor v0.16b, v0.16b, v7.16b;       /* Offset_i+3 */ \
    cmp x6, #4; \
    eor v3.16b, v3.16b, v7.16b;       /* A_i+2 xor Offset_i+2 */ \
    eor v4.16b, v4.16b, v0.16b;       /* A_i+3 xor Offset_i+3 */ \
    \
    do_aes_4_##bits(e, mc, v1, v2, v3, v4); \
    \
    eor v1.16b, v1.16b, v2.16b; \
    eor v16.16b, v16.16b, v3.16b; \
    eor v1.16b, v1.16b, v4.16b; \
    eor v16.16b, v16.16b, v1.16b; \
    \
    b.hs .Locb_auth_loop4_##bits; \
    CLEAR_REG(v3); \
    CLEAR_REG(v4); \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    CLEAR_REG(v7); \
    cbz x6, .Locb_auth_done; \
    \
  .Locb_auth_loop_##bits: \
    \
    /* Offset_i = Offset_{i-1} xor L_{ntz(i)} */ \
    /* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */ \
    \
    rbit w8, w12; \
    add w12, w12, #1; \
    clz w8, w8; /* ntz(i) */ \
    add x8, x5, x8, lsl #4; \
    \
    ld1 {v1.16b}, [x1], #16; /* load aadtext */ \
    ld1 {v2.16b}, [x8]; /* load L_{ntz(i)} */ \
    sub x6, x6, #1; \
    eor v0.16b, v0.16b, v2.16b; \
    eor v1.16b, v1.16b, v0.16b; \
    \
    do_aes_one##bits(e, mc, v1, v1, vk0) \
    \
    eor v16.16b, v16.16b, v1.16b; \
    \
    cbnz x6, .Locb_auth_loop_##bits; \
    b .Locb_auth_done;

  OCB_AUTH(128)
  OCB_AUTH(192)
  OCB_AUTH(256)

#undef OCB_AUTH

.Locb_auth_done:
  aes_clear_keys(w7)

  st1 {v16.16b}, [x4] /* store checksum */
  st1 {v0.16b}, [x3] /* store offset */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v16)

  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_ocb_auth_armv8_ce,.-_gcry_aes_ocb_auth_armv8_ce;)


/*
 * void _gcry_aes_xts_enc_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *tweak,
 *                                  size_t nblocks,
 *                                  unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_xts_enc_armv8_ce
ELF(.type  _gcry_aes_xts_enc_armv8_ce,%function;)
_gcry_aes_xts_enc_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: tweak
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lxts_enc_skip

  add x16, sp, #-64;
  add sp, sp, #-128;
  CFI_ADJUST_CFA_OFFSET(128);

  /* load tweak */
  ld1 {v0.16b}, [x3]

  /* load gfmul mask */
  mov x6, #0x87
  mov x7, #0x01
  mov v16.D[0], x6
  mov v16.D[1], x7

  aes_preload_keys(x0, w5);
  eor vklast.16b, vklast.16b, vk0.16b;

  b.eq .Lxts_ecry_entry_192
  b.hi .Lxts_ecry_entry_256

#define XTS_CRYPT(bits, ed, mcimc) \
  .Lxts_##ed##cry_entry_##bits: \
    cmp x4, #4; \
    b.lo .Lxts_##ed##cry_loop_##bits; \
    \
    st1 {v8.16b}, [sp]; /* store callee saved registers */ \
    ext v4.16b, v0.16b, v0.16b, #8; \
    mov v8.16b, v0.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v5.2d, v0.2d, v0.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v5.16b, v5.16b, v2.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v6.2d, v5.2d, v5.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v6.16b, v6.16b, v2.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v7.2d, v6.2d, v6.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v7.16b, v7.16b, v2.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v3.2d, v7.2d, v7.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v0.16b, v3.16b, v2.16b; \
    ld1 {v1.16b-v4.16b}, [x2], #64; /* load plaintext */ \
    cmp x4, #8; \
    sub x4, x4, #4; \
    \
    eor v8.16b, v8.16b, vk0.16b; \
    eor v5.16b, v5.16b, vk0.16b; \
    eor v6.16b, v6.16b, vk0.16b; \
    eor v7.16b, v7.16b, vk0.16b; \
    \
    do_aes_4_part1_multikey(ed, mcimc, v1, v2, v3, v4, v8, v5, v6, v7); \
    b.lo .Lxts_##ed##cry_done4_##bits; \
    \
    st1 {v9.16b-v12.16b}, [x16]; /* store callee saved registers */ \
    \
  .Lxts_##ed##cry_loop4_##bits: \
    eor v8.16b, v8.16b, vklast.16b; \
    eor v5.16b, v5.16b, vklast.16b; \
    eor v6.16b, v6.16b, vklast.16b; \
    eor v7.16b, v7.16b, vklast.16b; \
    do_aes_4_part2_##bits(ed, mcimc, v9, v10, v11, v12, v1, v2, v3, v4, v8, v5, v6, v7); \
    \
    ext v4.16b, v0.16b, v0.16b, #8; \
    mov v8.16b, v0.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v5.2d, v0.2d, v0.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v5.16b, v5.16b, v2.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v6.2d, v5.2d, v5.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v6.16b, v6.16b, v2.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v7.2d, v6.2d, v6.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v7.16b, v7.16b, v2.16b; \
    \
    sshr v2.2d, v4.2d, #63; \
    add v3.2d, v7.2d, v7.2d; \
    and v2.16b, v2.16b, v16.16b; \
    add v4.2d, v4.2d, v4.2d; \
    eor v0.16b, v3.16b, v2.16b; \
    ld1 {v1.16b-v4.16b}, [x2], #64; /* load plaintext */ \
    cmp x4, #8; \
    sub x4, x4, #4; \
    \
    eor v8.16b, v8.16b, vk0.16b; \
    eor v5.16b, v5.16b, vk0.16b; \
    eor v6.16b, v6.16b, vk0.16b; \
    eor v7.16b, v7.16b, vk0.16b; \
    \
    do_aes_4_part1_multikey(ed, mcimc, v1, v2, v3, v4, v8, v5, v6, v7); \
    \
    st1 {v9.16b-v12.16b}, [x1], #64; /* store plaintext */ \
    \
    b.hs .Lxts_##ed##cry_loop4_##bits; \
    \
    ld1 {v9.16b-v12.16b}, [x16]; /* restore callee saved registers */ \
    \
  .Lxts_##ed##cry_done4_##bits: \
    eor v8.16b, v8.16b, vklast.16b; \
    eor v5.16b, v5.16b, vklast.16b; \
    eor v6.16b, v6.16b, vklast.16b; \
    eor v7.16b, v7.16b, vklast.16b; \
    do_aes_4_part2_##bits(ed, mcimc, v1, v2, v3, v4, v1, v2, v3, v4, v8, v5, v6, v7); \
    \
    st1 {v1.16b-v4.16b}, [x1], #64; /* store plaintext */ \
    \
    CLEAR_REG(v4); \
    ld1 {v8.16b}, [sp]; /* restore callee saved registers */ \
    CLEAR_REG(v5); \
    CLEAR_REG(v6); \
    CLEAR_REG(v7); \
    cbz x4, .Lxts_##ed##cry_done; \
    \
  .Lxts_##ed##cry_loop_##bits: \
    \
    ld1 {v1.16b}, [x2], #16; /* load plaintext */ \
    ext v3.16b, v0.16b, v0.16b, #8; \
    eor v2.16b, v0.16b, vk0.16b; \
    sshr v3.2d, v3.2d, #63; \
    add v0.2d, v0.2d, v0.2d; \
    and v3.16b, v3.16b, v16.16b; \
    sub x4, x4, #1; \
    eor v0.16b, v0.16b, v3.16b; \
    \
    do_aes_one_part1(ed, mcimc, v1, v2); \
    eor v2.16b, v2.16b, vklast.16b; \
    do_aes_one_part2_##bits(ed, mcimc, v1, __, __); \
    eor v1.16b, v1.16b, v2.16b; \
    \
    st1 {v1.16b}, [x1], #16; /* store ciphertext */ \
    \
    cbnz x4, .Lxts_##ed##cry_loop_##bits; \
    b .Lxts_##ed##cry_done;

  XTS_CRYPT(128, e, mc)
  XTS_CRYPT(192, e, mc)
  XTS_CRYPT(256, e, mc)

.Lxts_ecry_done:
  aes_clear_keys(w5)

  st1 {v0.16b}, [x3] /* store tweak */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)
  CLEAR_REG(v3)
  CLEAR_REG(v16)

  add sp, sp, 128;
  CFI_ADJUST_CFA_OFFSET(-128);

.Lxts_enc_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_xts_enc_armv8_ce,.-_gcry_aes_xts_enc_armv8_ce;)


/*
 * void _gcry_aes_xts_dec_armv8_ce (const void *keysched,
 *                                  unsigned char *outbuf,
 *                                  const unsigned char *inbuf,
 *                                  unsigned char *tweak,
 *                                  size_t nblocks,
 *                                  unsigned int nrounds);
 */

.align 3
.globl _gcry_aes_xts_dec_armv8_ce
ELF(.type  _gcry_aes_xts_dec_armv8_ce,%function;)
_gcry_aes_xts_dec_armv8_ce:
  /* input:
   *    r0: keysched
   *    r1: outbuf
   *    r2: inbuf
   *    r3: tweak
   *    x4: nblocks
   *    w5: nrounds
   */
  CFI_STARTPROC();

  cbz x4, .Lxts_dec_skip

  add x16, sp, #-64;
  add sp, sp, #-128;
  CFI_ADJUST_CFA_OFFSET(128);

  /* load tweak */
  ld1 {v0.16b}, [x3]

  /* load gfmul mask */
  mov x6, #0x87
  mov x7, #0x01
  mov v16.D[0], x6
  mov v16.D[1], x7

  aes_preload_keys(x0, w5);
  eor vklast.16b, vklast.16b, vk0.16b;

  b.eq .Lxts_dcry_entry_192
  b.hi .Lxts_dcry_entry_256

  XTS_CRYPT(128, d, imc)
  XTS_CRYPT(192, d, imc)
  XTS_CRYPT(256, d, imc)

#undef XTS_CRYPT

.Lxts_dcry_done:
  aes_clear_keys(w5)

  st1 {v0.16b}, [x3] /* store tweak */

  CLEAR_REG(v0)
  CLEAR_REG(v1)
  CLEAR_REG(v2)

  add sp, sp, 128;
  CFI_ADJUST_CFA_OFFSET(-128);

.Lxts_dec_skip:
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_xts_dec_armv8_ce,.-_gcry_aes_xts_dec_armv8_ce;)


/*
 * u32 _gcry_aes_sbox4_armv8_ce(u32 in4b);
 */
.align 3
.globl _gcry_aes_sbox4_armv8_ce
ELF(.type  _gcry_aes_sbox4_armv8_ce,%function;)
_gcry_aes_sbox4_armv8_ce:
  /* See "Gouvêa, C. P. L. & López, J. Implementing GCM on ARMv8. Topics in
   * Cryptology — CT-RSA 2015" for details.
   */
  CFI_STARTPROC();
  movi v0.16b, #0x52
  movi v1.16b, #0
  mov v0.S[0], w0
  aese v0.16b, v1.16b
  addv s0, v0.4s
  mov w0, v0.S[0]
  CLEAR_REG(v0)
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_sbox4_armv8_ce,.-_gcry_aes_sbox4_armv8_ce;)


/*
 * void _gcry_aes_invmixcol_armv8_ce(void *dst, const void *src);
 */
.align 3
.globl _gcry_aes_invmixcol_armv8_ce
ELF(.type  _gcry_aes_invmixcol_armv8_ce,%function;)
_gcry_aes_invmixcol_armv8_ce:
  CFI_STARTPROC();
  ld1 {v0.16b}, [x1]
  aesimc v0.16b, v0.16b
  st1 {v0.16b}, [x0]
  CLEAR_REG(v0)
  ret_spec_stop
  CFI_ENDPROC();
ELF(.size _gcry_aes_invmixcol_armv8_ce,.-_gcry_aes_invmixcol_armv8_ce;)

#endif
