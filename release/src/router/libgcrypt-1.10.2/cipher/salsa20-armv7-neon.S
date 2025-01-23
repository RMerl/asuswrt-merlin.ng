/* salsa-armv7-neon.S  -  ARM NEON implementation of Salsa20 cipher
 *
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
    defined(HAVE_GCC_INLINE_ASM_NEON) && defined(USE_SALSA20)

/*
 * Based on public domain implementation from SUPERCOP benchmarking framework
 * by Peter Schwabe and D. J. Bernstein. Paper about the implementation at:
 *   http://cryptojedi.org/papers/#neoncrypto
 */

.syntax unified
.arm
.fpu neon
.text

.align 2
.globl _gcry_arm_neon_salsa20_encrypt
.type  _gcry_arm_neon_salsa20_encrypt,%function;
_gcry_arm_neon_salsa20_encrypt:
	/* Modifications:
	 *  - arguments changed to (void *c, const void *m, unsigned int nblks,
         *    void *ctx, unsigned int rounds) from (void *c, const void *m,
         *    unsigned long long mlen, const void *n, const void *k)
	 *  - nonce and key read from 'ctx' as well as sigma and counter.
	 *  - read in counter from 'ctx' at the start.
         *  - update counter in 'ctx' at the end.
	 *  - length is input as number of blocks, so don't handle tail bytes
	 *    (this is done in salsa20.c).
	 */
	lsl r2,r2,#6
	vpush {q4,q5,q6,q7}
	mov r12,sp
	sub sp,sp,#352
	and sp,sp,#0xffffffe0
	strd r4,[sp,#0]
	strd r6,[sp,#8]
	strd r8,[sp,#16]
	strd r10,[sp,#24]
	str r14,[sp,#224]
	str r12,[sp,#228]
	str r0,[sp,#232]
	str r1,[sp,#236]
	str r2,[sp,#240]
	ldr r4,[r12,#64]
	str r4,[sp,#244]
	mov r2,r3
	add r3,r2,#48
	vld1.8 {q3},[r2]
	add r0,r2,#32
	add r14,r2,#40
	vmov.i64 q3,#0xff
	str r14,[sp,#160]
	ldrd r8,[r2,#4]
	vld1.8 {d0},[r0]
	ldrd r4,[r2,#20]
	vld1.8 {d8-d9},[r2]!
	ldrd r6,[r0,#0]
	vmov d4,d9
	ldr r0,[r14]
	vrev64.i32 d0,d0
	ldr r1,[r14,#4]
	vld1.8 {d10-d11},[r2]
	strd r6,[sp,#32]
	sub r2,r2,#16
	strd r0,[sp,#40]
	vmov d5,d11
	strd r8,[sp,#48]
	vext.32 d1,d0,d10,#1
	strd r4,[sp,#56]
	ldr r1,[r2,#0]
	vshr.u32 q3,q3,#7
	ldr r4,[r2,#12]
	vext.32 d3,d11,d9,#1
	ldr r11,[r2,#16]
	vext.32 d2,d8,d0,#1
	ldr r8,[r2,#28]
	vext.32 d0,d10,d8,#1
	ldr r0,[r3,#0]
	add r2,r2,#44
	vmov q4,q3
	vld1.8 {d6-d7},[r14]
	vadd.i64 q3,q3,q4
	ldr r5,[r3,#4]
	add r12,sp,#256
	vst1.8 {d4-d5},[r12,: 128]
	ldr r10,[r3,#8]
	add r14,sp,#272
	vst1.8 {d2-d3},[r14,: 128]
	ldr r9,[r3,#12]
	vld1.8 {d2-d3},[r3]
	strd r0,[sp,#64]
	ldr r0,[sp,#240]
	strd r4,[sp,#72]
	strd r10,[sp,#80]
	strd r8,[sp,#88]
	nop
	cmp r0,#192
	blo .L_mlenlowbelow192
.L_mlenatleast192:
	ldrd r2,[sp,#48]
	vext.32 d7,d6,d6,#1
	vmov q8,q1
	ldrd r6,[sp,#32]
	vld1.8 {d18-d19},[r12,: 128]
	vmov q10,q0
	str r0,[sp,#240]
	vext.32 d4,d7,d19,#1
	vmov q11,q8
	vext.32 d10,d18,d7,#1
	vadd.i64 q3,q3,q4
	ldrd r0,[sp,#64]
	vld1.8 {d24-d25},[r14,: 128]
	vmov d5,d24
	add r8,sp,#288
	ldrd r4,[sp,#72]
	vmov d11,d25
	add r9,sp,#304
	ldrd r10,[sp,#80]
	vst1.8 {d4-d5},[r8,: 128]
	strd r2,[sp,#96]
	vext.32 d7,d6,d6,#1
	vmov q13,q10
	strd r6,[sp,#104]
	vmov d13,d24
	vst1.8 {d10-d11},[r9,: 128]
	add r2,sp,#320
	vext.32 d12,d7,d19,#1
	vmov d15,d25
	add r6,sp,#336
	ldr r12,[sp,#244]
	vext.32 d14,d18,d7,#1
	vadd.i64 q3,q3,q4
	ldrd r8,[sp,#88]
	vst1.8 {d12-d13},[r2,: 128]
	ldrd r2,[sp,#56]
	vst1.8 {d14-d15},[r6,: 128]
	ldrd r6,[sp,#40]
.L_mainloop2:
	str r12,[sp,#248]
	vadd.i32 q4,q10,q8
	vadd.i32 q9,q13,q11
	add r12,r0,r2
	add r14,r5,r1
	vshl.i32 q12,q4,#7
	vshl.i32 q14,q9,#7
	vshr.u32 q4,q4,#25
	vshr.u32 q9,q9,#25
	eor r4,r4,r12,ROR #25
	eor r7,r7,r14,ROR #25
	add r12,r4,r0
	add r14,r7,r5
	veor q5,q5,q12
	veor q7,q7,q14
	veor q4,q5,q4
	veor q5,q7,q9
	eor r6,r6,r12,ROR #23
	eor r3,r3,r14,ROR #23
	add r12,r6,r4
	str r7,[sp,#116]
	add r7,r3,r7
	ldr r14,[sp,#108]
	vadd.i32 q7,q8,q4
	vadd.i32 q9,q11,q5
	vshl.i32 q12,q7,#9
	vshl.i32 q14,q9,#9
	vshr.u32 q7,q7,#23
	vshr.u32 q9,q9,#23
	veor q2,q2,q12
	veor q6,q6,q14
	veor q2,q2,q7
	veor q6,q6,q9
	eor r2,r2,r12,ROR #19
	str r2,[sp,#120]
	eor r1,r1,r7,ROR #19
	ldr r7,[sp,#96]
	add r2,r2,r6
	str r6,[sp,#112]
	add r6,r1,r3
	ldr r12,[sp,#104]
	vadd.i32 q7,q4,q2
	vext.32 q4,q4,q4,#3
	vadd.i32 q9,q5,q6
	vshl.i32 q12,q7,#13
	vext.32 q5,q5,q5,#3
	vshl.i32 q14,q9,#13
	eor r0,r0,r2,ROR #14
	eor r2,r5,r6,ROR #14
	str r3,[sp,#124]
	add r3,r10,r12
	ldr r5,[sp,#100]
	add r6,r9,r11
	vshr.u32 q7,q7,#19
	vshr.u32 q9,q9,#19
	veor q10,q10,q12
	veor q12,q13,q14
	eor r8,r8,r3,ROR #25
	eor r3,r5,r6,ROR #25
	add r5,r8,r10
	add r6,r3,r9
	veor q7,q10,q7
	veor q9,q12,q9
	eor r5,r7,r5,ROR #23
	eor r6,r14,r6,ROR #23
	add r7,r5,r8
	add r14,r6,r3
	vadd.i32 q10,q2,q7
	vswp d4,d5
	vadd.i32 q12,q6,q9
	vshl.i32 q13,q10,#18
	vswp d12,d13
	vshl.i32 q14,q12,#18
	eor r7,r12,r7,ROR #19
	eor r11,r11,r14,ROR #19
	add r12,r7,r5
	add r14,r11,r6
	vshr.u32 q10,q10,#14
	vext.32 q7,q7,q7,#1
	vshr.u32 q12,q12,#14
	veor q8,q8,q13
	vext.32 q9,q9,q9,#1
	veor q11,q11,q14
	eor r10,r10,r12,ROR #14
	eor r9,r9,r14,ROR #14
	add r12,r0,r3
	add r14,r2,r4
	veor q8,q8,q10
	veor q10,q11,q12
	eor r1,r1,r12,ROR #25
	eor r7,r7,r14,ROR #25
	add r12,r1,r0
	add r14,r7,r2
	vadd.i32 q11,q4,q8
	vadd.i32 q12,q5,q10
	vshl.i32 q13,q11,#7
	vshl.i32 q14,q12,#7
	eor r5,r5,r12,ROR #23
	eor r6,r6,r14,ROR #23
	vshr.u32 q11,q11,#25
	vshr.u32 q12,q12,#25
	add r12,r5,r1
	add r14,r6,r7
	veor q7,q7,q13
	veor q9,q9,q14
	veor q7,q7,q11
	veor q9,q9,q12
	vadd.i32 q11,q8,q7
	vadd.i32 q12,q10,q9
	vshl.i32 q13,q11,#9
	vshl.i32 q14,q12,#9
	eor r3,r3,r12,ROR #19
	str r7,[sp,#104]
	eor r4,r4,r14,ROR #19
	ldr r7,[sp,#112]
	add r12,r3,r5
	str r6,[sp,#108]
	add r6,r4,r6
	ldr r14,[sp,#116]
	eor r0,r0,r12,ROR #14
	str r5,[sp,#96]
	eor r5,r2,r6,ROR #14
	ldr r2,[sp,#120]
	vshr.u32 q11,q11,#23
	vshr.u32 q12,q12,#23
	veor q2,q2,q13
	veor q6,q6,q14
	veor q2,q2,q11
	veor q6,q6,q12
	add r6,r10,r14
	add r12,r9,r8
	vadd.i32 q11,q7,q2
	vext.32 q7,q7,q7,#3
	vadd.i32 q12,q9,q6
	vshl.i32 q13,q11,#13
	vext.32 q9,q9,q9,#3
	vshl.i32 q14,q12,#13
	vshr.u32 q11,q11,#19
	vshr.u32 q12,q12,#19
	eor r11,r11,r6,ROR #25
	eor r2,r2,r12,ROR #25
	add r6,r11,r10
	str r3,[sp,#100]
	add r3,r2,r9
	ldr r12,[sp,#124]
	veor q4,q4,q13
	veor q5,q5,q14
	veor q4,q4,q11
	veor q5,q5,q12
	eor r6,r7,r6,ROR #23
	eor r3,r12,r3,ROR #23
	add r7,r6,r11
	add r12,r3,r2
	vadd.i32 q11,q2,q4
	vswp d4,d5
	vadd.i32 q12,q6,q5
	vshl.i32 q13,q11,#18
	vswp d12,d13
	vshl.i32 q14,q12,#18
	eor r7,r14,r7,ROR #19
	eor r8,r8,r12,ROR #19
	add r12,r7,r6
	add r14,r8,r3
	vshr.u32 q11,q11,#14
	vext.32 q4,q4,q4,#1
	vshr.u32 q12,q12,#14
	veor q8,q8,q13
	vext.32 q5,q5,q5,#1
	veor q10,q10,q14
	eor r10,r10,r12,ROR #14
	veor q8,q8,q11
	eor r9,r9,r14,ROR #14
	veor q10,q10,q12
	vadd.i32 q11,q7,q8
	vadd.i32 q12,q9,q10
	add r12,r0,r2
	add r14,r5,r1
	vshl.i32 q13,q11,#7
	vshl.i32 q14,q12,#7
	vshr.u32 q11,q11,#25
	vshr.u32 q12,q12,#25
	eor r4,r4,r12,ROR #25
	eor r7,r7,r14,ROR #25
	add r12,r4,r0
	add r14,r7,r5
	veor q4,q4,q13
	veor q5,q5,q14
	veor q4,q4,q11
	veor q5,q5,q12
	eor r6,r6,r12,ROR #23
	eor r3,r3,r14,ROR #23
	add r12,r6,r4
	str r7,[sp,#116]
	add r7,r3,r7
	ldr r14,[sp,#108]
	vadd.i32 q11,q8,q4
	vadd.i32 q12,q10,q5
	vshl.i32 q13,q11,#9
	vshl.i32 q14,q12,#9
	vshr.u32 q11,q11,#23
	vshr.u32 q12,q12,#23
	veor q2,q2,q13
	veor q6,q6,q14
	veor q2,q2,q11
	veor q6,q6,q12
	eor r2,r2,r12,ROR #19
	str r2,[sp,#120]
	eor r1,r1,r7,ROR #19
	ldr r7,[sp,#96]
	add r2,r2,r6
	str r6,[sp,#112]
	add r6,r1,r3
	ldr r12,[sp,#104]
	vadd.i32 q11,q4,q2
	vext.32 q4,q4,q4,#3
	vadd.i32 q12,q5,q6
	vshl.i32 q13,q11,#13
	vext.32 q5,q5,q5,#3
	vshl.i32 q14,q12,#13
	eor r0,r0,r2,ROR #14
	eor r2,r5,r6,ROR #14
	str r3,[sp,#124]
	add r3,r10,r12
	ldr r5,[sp,#100]
	add r6,r9,r11
	vshr.u32 q11,q11,#19
	vshr.u32 q12,q12,#19
	veor q7,q7,q13
	veor q9,q9,q14
	eor r8,r8,r3,ROR #25
	eor r3,r5,r6,ROR #25
	add r5,r8,r10
	add r6,r3,r9
	veor q7,q7,q11
	veor q9,q9,q12
	eor r5,r7,r5,ROR #23
	eor r6,r14,r6,ROR #23
	add r7,r5,r8
	add r14,r6,r3
	vadd.i32 q11,q2,q7
	vswp d4,d5
	vadd.i32 q12,q6,q9
	vshl.i32 q13,q11,#18
	vswp d12,d13
	vshl.i32 q14,q12,#18
	eor r7,r12,r7,ROR #19
	eor r11,r11,r14,ROR #19
	add r12,r7,r5
	add r14,r11,r6
	vshr.u32 q11,q11,#14
	vext.32 q7,q7,q7,#1
	vshr.u32 q12,q12,#14
	veor q8,q8,q13
	vext.32 q9,q9,q9,#1
	veor q10,q10,q14
	eor r10,r10,r12,ROR #14
	eor r9,r9,r14,ROR #14
	add r12,r0,r3
	add r14,r2,r4
	veor q8,q8,q11
	veor q11,q10,q12
	eor r1,r1,r12,ROR #25
	eor r7,r7,r14,ROR #25
	add r12,r1,r0
	add r14,r7,r2
	vadd.i32 q10,q4,q8
	vadd.i32 q12,q5,q11
	vshl.i32 q13,q10,#7
	vshl.i32 q14,q12,#7
	eor r5,r5,r12,ROR #23
	eor r6,r6,r14,ROR #23
	vshr.u32 q10,q10,#25
	vshr.u32 q12,q12,#25
	add r12,r5,r1
	add r14,r6,r7
	veor q7,q7,q13
	veor q9,q9,q14
	veor q7,q7,q10
	veor q9,q9,q12
	vadd.i32 q10,q8,q7
	vadd.i32 q12,q11,q9
	vshl.i32 q13,q10,#9
	vshl.i32 q14,q12,#9
	eor r3,r3,r12,ROR #19
	str r7,[sp,#104]
	eor r4,r4,r14,ROR #19
	ldr r7,[sp,#112]
	add r12,r3,r5
	str r6,[sp,#108]
	add r6,r4,r6
	ldr r14,[sp,#116]
	eor r0,r0,r12,ROR #14
	str r5,[sp,#96]
	eor r5,r2,r6,ROR #14
	ldr r2,[sp,#120]
	vshr.u32 q10,q10,#23
	vshr.u32 q12,q12,#23
	veor q2,q2,q13
	veor q6,q6,q14
	veor q2,q2,q10
	veor q6,q6,q12
	add r6,r10,r14
	add r12,r9,r8
	vadd.i32 q12,q7,q2
	vext.32 q10,q7,q7,#3
	vadd.i32 q7,q9,q6
	vshl.i32 q14,q12,#13
	vext.32 q13,q9,q9,#3
	vshl.i32 q9,q7,#13
	vshr.u32 q12,q12,#19
	vshr.u32 q7,q7,#19
	eor r11,r11,r6,ROR #25
	eor r2,r2,r12,ROR #25
	add r6,r11,r10
	str r3,[sp,#100]
	add r3,r2,r9
	ldr r12,[sp,#124]
	veor q4,q4,q14
	veor q5,q5,q9
	veor q4,q4,q12
	veor q7,q5,q7
	eor r6,r7,r6,ROR #23
	eor r3,r12,r3,ROR #23
	add r7,r6,r11
	add r12,r3,r2
	vadd.i32 q5,q2,q4
	vswp d4,d5
	vadd.i32 q9,q6,q7
	vshl.i32 q12,q5,#18
	vswp d12,d13
	vshl.i32 q14,q9,#18
	eor r7,r14,r7,ROR #19
	eor r8,r8,r12,ROR #19
	add r12,r7,r6
	add r14,r8,r3
	vshr.u32 q15,q5,#14
	vext.32 q5,q4,q4,#1
	vshr.u32 q4,q9,#14
	veor q8,q8,q12
	vext.32 q7,q7,q7,#1
	veor q9,q11,q14
	eor r10,r10,r12,ROR #14
	ldr r12,[sp,#248]
	veor q8,q8,q15
	eor r9,r9,r14,ROR #14
	veor q11,q9,q4
	subs r12,r12,#4
	bhi .L_mainloop2
	strd r8,[sp,#112]
	ldrd r8,[sp,#64]
	strd r2,[sp,#120]
	ldrd r2,[sp,#96]
	add r0,r0,r8
	strd r10,[sp,#96]
	add r1,r1,r9
	ldrd r10,[sp,#48]
	ldrd r8,[sp,#72]
	add r2,r2,r10
	strd r6,[sp,#128]
	add r3,r3,r11
	ldrd r6,[sp,#104]
	ldrd r10,[sp,#32]
	ldr r12,[sp,#236]
	add r4,r4,r8
	add r5,r5,r9
	add r6,r6,r10
	add r7,r7,r11
	cmp r12,#0
	beq .L_nomessage1
	ldr r8,[r12,#0]
	ldr r9,[r12,#4]
	ldr r10,[r12,#8]
	ldr r11,[r12,#12]
	eor r0,r0,r8
	ldr r8,[r12,#16]
	eor r1,r1,r9
	ldr r9,[r12,#20]
	eor r2,r2,r10
	ldr r10,[r12,#24]
	eor r3,r3,r11
	ldr r11,[r12,#28]
	eor r4,r4,r8
	eor r5,r5,r9
	eor r6,r6,r10
	eor r7,r7,r11
.L_nomessage1:
	ldr r14,[sp,#232]
	vadd.i32 q4,q8,q1
	str r0,[r14,#0]
	add r0,sp,#304
	str r1,[r14,#4]
	vld1.8 {d16-d17},[r0,: 128]
	str r2,[r14,#8]
	vadd.i32 q5,q8,q5
	str r3,[r14,#12]
	add r0,sp,#288
	str r4,[r14,#16]
	vld1.8 {d16-d17},[r0,: 128]
	str r5,[r14,#20]
	vadd.i32 q9,q10,q0
	str r6,[r14,#24]
	vadd.i32 q2,q8,q2
	str r7,[r14,#28]
	vmov.i64 q8,#0xffffffff
	ldrd r6,[sp,#128]
	vext.32 d20,d8,d10,#1
	ldrd r0,[sp,#40]
	vext.32 d25,d9,d11,#1
	ldrd r2,[sp,#120]
	vbif q4,q9,q8
	ldrd r4,[sp,#56]
	vext.32 d21,d5,d19,#1
	add r6,r6,r0
	vext.32 d24,d4,d18,#1
	add r7,r7,r1
	vbif q2,q5,q8
	add r2,r2,r4
	vrev64.i32 q5,q10
	add r3,r3,r5
	vrev64.i32 q9,q12
	adds r0,r0,#3
	vswp d5,d9
	adc r1,r1,#0
	strd r0,[sp,#40]
	ldrd r8,[sp,#112]
	ldrd r0,[sp,#88]
	ldrd r10,[sp,#96]
	ldrd r4,[sp,#80]
	add r0,r8,r0
	add r1,r9,r1
	add r4,r10,r4
	add r5,r11,r5
	add r8,r14,#64
	cmp r12,#0
	beq .L_nomessage2
	ldr r9,[r12,#32]
	ldr r10,[r12,#36]
	ldr r11,[r12,#40]
	ldr r14,[r12,#44]
	eor r6,r6,r9
	ldr r9,[r12,#48]
	eor r7,r7,r10
	ldr r10,[r12,#52]
	eor r4,r4,r11
	ldr r11,[r12,#56]
	eor r5,r5,r14
	ldr r14,[r12,#60]
	add r12,r12,#64
	eor r2,r2,r9
	vld1.8 {d20-d21},[r12]!
	veor q4,q4,q10
	eor r3,r3,r10
	vld1.8 {d20-d21},[r12]!
	veor q5,q5,q10
	eor r0,r0,r11
	vld1.8 {d20-d21},[r12]!
	veor q2,q2,q10
	eor r1,r1,r14
	vld1.8 {d20-d21},[r12]!
	veor q9,q9,q10
.L_nomessage2:
	vst1.8 {d8-d9},[r8]!
	vst1.8 {d10-d11},[r8]!
	vmov.i64 q4,#0xff
	vst1.8 {d4-d5},[r8]!
	vst1.8 {d18-d19},[r8]!
	str r6,[r8,#-96]
	add r6,sp,#336
	str r7,[r8,#-92]
	add r7,sp,#320
	str r4,[r8,#-88]
	vadd.i32 q2,q11,q1
	vld1.8 {d10-d11},[r6,: 128]
	vadd.i32 q5,q5,q7
	vld1.8 {d14-d15},[r7,: 128]
	vadd.i32 q9,q13,q0
	vadd.i32 q6,q7,q6
	str r5,[r8,#-84]
	vext.32 d14,d4,d10,#1
	str r2,[r8,#-80]
	vext.32 d21,d5,d11,#1
	str r3,[r8,#-76]
	vbif q2,q9,q8
	str r0,[r8,#-72]
	vext.32 d15,d13,d19,#1
	vshr.u32 q4,q4,#7
	str r1,[r8,#-68]
	vext.32 d20,d12,d18,#1
	vbif q6,q5,q8
	ldr r0,[sp,#240]
	vrev64.i32 q5,q7
	vrev64.i32 q7,q10
	vswp d13,d5
	vadd.i64 q3,q3,q4
	sub r0,r0,#192
	cmp r12,#0
	beq .L_nomessage21
	vld1.8 {d16-d17},[r12]!
	veor q2,q2,q8
	vld1.8 {d16-d17},[r12]!
	veor q5,q5,q8
	vld1.8 {d16-d17},[r12]!
	veor q6,q6,q8
	vld1.8 {d16-d17},[r12]!
	veor q7,q7,q8
.L_nomessage21:
	vst1.8 {d4-d5},[r8]!
	vst1.8 {d10-d11},[r8]!
	vst1.8 {d12-d13},[r8]!
	vst1.8 {d14-d15},[r8]!
	str r12,[sp,#236]
	add r14,sp,#272
	add r12,sp,#256
	str r8,[sp,#232]
	cmp r0,#192
	bhs .L_mlenatleast192
.L_mlenlowbelow192:
	cmp r0,#0
	beq .L_done
	b .L_mlenatleast1
.L_nextblock:
	sub r0,r0,#64
.L_mlenatleast1:
.L_handleblock:
	str r0,[sp,#248]
	ldrd r2,[sp,#48]
	ldrd r6,[sp,#32]
	ldrd r0,[sp,#64]
	ldrd r4,[sp,#72]
	ldrd r10,[sp,#80]
	ldrd r8,[sp,#88]
	strd r2,[sp,#96]
	strd r6,[sp,#104]
	ldrd r2,[sp,#56]
	ldrd r6,[sp,#40]
	ldr r12,[sp,#244]
.L_mainloop1:
	str r12,[sp,#252]
	add r12,r0,r2
	add r14,r5,r1
	eor r4,r4,r12,ROR #25
	eor r7,r7,r14,ROR #25
	add r12,r4,r0
	add r14,r7,r5
	eor r6,r6,r12,ROR #23
	eor r3,r3,r14,ROR #23
	add r12,r6,r4
	str r7,[sp,#132]
	add r7,r3,r7
	ldr r14,[sp,#104]
	eor r2,r2,r12,ROR #19
	str r6,[sp,#128]
	eor r1,r1,r7,ROR #19
	ldr r7,[sp,#100]
	add r6,r2,r6
	str r2,[sp,#120]
	add r2,r1,r3
	ldr r12,[sp,#96]
	eor r0,r0,r6,ROR #14
	str r3,[sp,#124]
	eor r2,r5,r2,ROR #14
	ldr r3,[sp,#108]
	add r5,r10,r14
	add r6,r9,r11
	eor r8,r8,r5,ROR #25
	eor r5,r7,r6,ROR #25
	add r6,r8,r10
	add r7,r5,r9
	eor r6,r12,r6,ROR #23
	eor r3,r3,r7,ROR #23
	add r7,r6,r8
	add r12,r3,r5
	eor r7,r14,r7,ROR #19
	eor r11,r11,r12,ROR #19
	add r12,r7,r6
	add r14,r11,r3
	eor r10,r10,r12,ROR #14
	eor r9,r9,r14,ROR #14
	add r12,r0,r5
	add r14,r2,r4
	eor r1,r1,r12,ROR #25
	eor r7,r7,r14,ROR #25
	add r12,r1,r0
	add r14,r7,r2
	eor r6,r6,r12,ROR #23
	eor r3,r3,r14,ROR #23
	add r12,r6,r1
	str r7,[sp,#104]
	add r7,r3,r7
	ldr r14,[sp,#128]
	eor r5,r5,r12,ROR #19
	str r3,[sp,#108]
	eor r4,r4,r7,ROR #19
	ldr r7,[sp,#132]
	add r12,r5,r6
	str r6,[sp,#96]
	add r3,r4,r3
	ldr r6,[sp,#120]
	eor r0,r0,r12,ROR #14
	str r5,[sp,#100]
	eor r5,r2,r3,ROR #14
	ldr r3,[sp,#124]
	add r2,r10,r7
	add r12,r9,r8
	eor r11,r11,r2,ROR #25
	eor r2,r6,r12,ROR #25
	add r6,r11,r10
	add r12,r2,r9
	eor r6,r14,r6,ROR #23
	eor r3,r3,r12,ROR #23
	add r12,r6,r11
	add r14,r3,r2
	eor r7,r7,r12,ROR #19
	eor r8,r8,r14,ROR #19
	add r12,r7,r6
	add r14,r8,r3
	eor r10,r10,r12,ROR #14
	eor r9,r9,r14,ROR #14
	ldr r12,[sp,#252]
	subs r12,r12,#2
	bhi .L_mainloop1
	strd r6,[sp,#128]
	strd r2,[sp,#120]
	strd r10,[sp,#112]
	strd r8,[sp,#136]
	ldrd r2,[sp,#96]
	ldrd r6,[sp,#104]
	ldrd r8,[sp,#64]
	ldrd r10,[sp,#48]
	add r0,r0,r8
	add r1,r1,r9
	add r2,r2,r10
	add r3,r3,r11
	ldrd r8,[sp,#72]
	ldrd r10,[sp,#32]
	add r4,r4,r8
	add r5,r5,r9
	add r6,r6,r10
	add r7,r7,r11
	ldr r12,[sp,#236]
	cmp r12,#0
	beq .L_nomessage10
	ldr r8,[r12,#0]
	ldr r9,[r12,#4]
	ldr r10,[r12,#8]
	ldr r11,[r12,#12]
	eor r0,r0,r8
	ldr r8,[r12,#16]
	eor r1,r1,r9
	ldr r9,[r12,#20]
	eor r2,r2,r10
	ldr r10,[r12,#24]
	eor r3,r3,r11
	ldr r11,[r12,#28]
	eor r4,r4,r8
	eor r5,r5,r9
	eor r6,r6,r10
	eor r7,r7,r11
.L_nomessage10:
	ldr r14,[sp,#232]
	str r0,[r14,#0]
	str r1,[r14,#4]
	str r2,[r14,#8]
	str r3,[r14,#12]
	str r4,[r14,#16]
	str r5,[r14,#20]
	str r6,[r14,#24]
	str r7,[r14,#28]
	ldrd r6,[sp,#128]
	ldrd r10,[sp,#112]
	ldrd r0,[sp,#40]
	ldrd r4,[sp,#80]
	add r6,r6,r0
	add r7,r7,r1
	add r10,r10,r4
	add r11,r11,r5
	adds r0,r0,#1
	adc r1,r1,#0
	strd r0,[sp,#40]
	ldrd r2,[sp,#120]
	ldrd r8,[sp,#136]
	ldrd r4,[sp,#56]
	ldrd r0,[sp,#88]
	add r2,r2,r4
	add r3,r3,r5
	add r0,r8,r0
	add r1,r9,r1
	cmp r12,#0
	beq .L_nomessage11
	ldr r4,[r12,#32]
	ldr r5,[r12,#36]
	ldr r8,[r12,#40]
	ldr r9,[r12,#44]
	eor r6,r6,r4
	ldr r4,[r12,#48]
	eor r7,r7,r5
	ldr r5,[r12,#52]
	eor r10,r10,r8
	ldr r8,[r12,#56]
	eor r11,r11,r9
	ldr r9,[r12,#60]
	eor r2,r2,r4
	eor r3,r3,r5
	eor r0,r0,r8
	eor r1,r1,r9
	add r4,r12,#64
	str r4,[sp,#236]
.L_nomessage11:
	str r6,[r14,#32]
	str r7,[r14,#36]
	str r10,[r14,#40]
	str r11,[r14,#44]
	str r2,[r14,#48]
	str r3,[r14,#52]
	str r0,[r14,#56]
	str r1,[r14,#60]
	add r0,r14,#64
	str r0,[sp,#232]
	ldr r0,[sp,#248]
	cmp r0,#64
	bhi .L_nextblock
.L_done:
	ldr r2,[sp,#160]
	ldrd r4,[sp,#0]
	ldrd r6,[sp,#8]
	ldrd r8,[sp,#16]
	ldrd r10,[sp,#24]
	ldr r12,[sp,#228]
	ldr r14,[sp,#224]
	ldrd r0,[sp,#40]
	strd r0,[r2]
	sub r0,r12,sp
	mov sp,r12
	vpop {q4,q5,q6,q7}
	add r0,r0,#64
	bx lr
.size _gcry_arm_neon_salsa20_encrypt,.-_gcry_arm_neon_salsa20_encrypt;

#endif
