/* cast5-arm.S  -  ARM assembly implementation of CAST5 cipher
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

#if defined(__ARMEL__)
#ifdef HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS

.text

.syntax unified
.arm

.extern _gcry_cast5_s1to4;

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

/* structure of crypto context */
#define Km 0
#define Kr (Km + (16 * 4))
#define Kr_arm_enc (Kr + (16))
#define Kr_arm_dec (Kr_arm_enc + (16))

/* register macros */
#define CTX %r0
#define Rs1 %r7
#define Rs2 %r8
#define Rs3 %r9
#define Rs4 %r10
#define RMASK %r11
#define RKM %r1
#define RKR %r2

#define RL0 %r3
#define RR0 %r4

#define RL1 %r9
#define RR1 %r10

#define RT0 %lr
#define RT1 %ip
#define RT2 %r5
#define RT3 %r6

/* helper macros */
#define ldr_unaligned_le(rout, rsrc, offs, rtmp) \
	ldrb rout, [rsrc, #((offs) + 0)]; \
	ldrb rtmp, [rsrc, #((offs) + 1)]; \
	orr rout, rout, rtmp, lsl #8; \
	ldrb rtmp, [rsrc, #((offs) + 2)]; \
	orr rout, rout, rtmp, lsl #16; \
	ldrb rtmp, [rsrc, #((offs) + 3)]; \
	orr rout, rout, rtmp, lsl #24;

#define str_unaligned_le(rin, rdst, offs, rtmp0, rtmp1) \
	mov rtmp0, rin, lsr #8; \
	strb rin, [rdst, #((offs) + 0)]; \
	mov rtmp1, rin, lsr #16; \
	strb rtmp0, [rdst, #((offs) + 1)]; \
	mov rtmp0, rin, lsr #24; \
	strb rtmp1, [rdst, #((offs) + 2)]; \
	strb rtmp0, [rdst, #((offs) + 3)];

#define ldr_unaligned_be(rout, rsrc, offs, rtmp) \
	ldrb rout, [rsrc, #((offs) + 3)]; \
	ldrb rtmp, [rsrc, #((offs) + 2)]; \
	orr rout, rout, rtmp, lsl #8; \
	ldrb rtmp, [rsrc, #((offs) + 1)]; \
	orr rout, rout, rtmp, lsl #16; \
	ldrb rtmp, [rsrc, #((offs) + 0)]; \
	orr rout, rout, rtmp, lsl #24;

#define str_unaligned_be(rin, rdst, offs, rtmp0, rtmp1) \
	mov rtmp0, rin, lsr #8; \
	strb rin, [rdst, #((offs) + 3)]; \
	mov rtmp1, rin, lsr #16; \
	strb rtmp0, [rdst, #((offs) + 2)]; \
	mov rtmp0, rin, lsr #24; \
	strb rtmp1, [rdst, #((offs) + 1)]; \
	strb rtmp0, [rdst, #((offs) + 0)];

#ifdef __ARMEL__
	#define ldr_unaligned_host ldr_unaligned_le
	#define str_unaligned_host str_unaligned_le

	/* bswap on little-endian */
#ifdef HAVE_ARM_ARCH_V6
	#define host_to_be(reg, rtmp) \
		rev reg, reg;
	#define be_to_host(reg, rtmp) \
		rev reg, reg;
#else
	#define host_to_be(reg, rtmp) \
		eor	rtmp, reg, reg, ror #16; \
		mov	rtmp, rtmp, lsr #8; \
		bic	rtmp, rtmp, #65280; \
		eor	reg, rtmp, reg, ror #8;
	#define be_to_host(reg, rtmp) \
		eor	rtmp, reg, reg, ror #16; \
		mov	rtmp, rtmp, lsr #8; \
		bic	rtmp, rtmp, #65280; \
		eor	reg, rtmp, reg, ror #8;
#endif
#else
	#define ldr_unaligned_host ldr_unaligned_be
	#define str_unaligned_host str_unaligned_be

	/* nop on big-endian */
	#define host_to_be(reg, rtmp) /*_*/
	#define be_to_host(reg, rtmp) /*_*/
#endif

#define host_to_host(x, y) /*_*/

/**********************************************************************
  1-way cast5
 **********************************************************************/

#define dummy(n) /*_*/

#define load_kr(n) \
	ldr RKR, [CTX, #(Kr_arm_enc + (n))]; /* Kr[n] */

#define load_dec_kr(n) \
	ldr RKR, [CTX, #(Kr_arm_dec + (n) - 3)]; /* Kr[n] */

#define load_km(n) \
	ldr RKM, [CTX, #(Km + (n) * 4)]; /* Km[n] */

#define shift_kr(dummy) \
	mov RKR, RKR, lsr #8;

#define F(n, rl, rr, op1, op2, op3, op4, dec, loadkm, shiftkr, loadkr) \
	op1 RKM, rr; \
	mov RKM, RKM, ror RKR; \
	\
	and RT0, RMASK, RKM, ror #(24); \
	and RT1, RMASK, RKM, lsr #(16); \
	and RT2, RMASK, RKM, lsr #(8); \
	ldr RT0, [Rs1, RT0]; \
	and RT3, RMASK, RKM; \
	ldr RT1, [Rs2, RT1]; \
	shiftkr(RKR); \
	\
	ldr RT2, [Rs3, RT2]; \
	\
	op2 RT0, RT1; \
	ldr RT3, [Rs4, RT3]; \
	op3 RT0, RT2; \
	loadkm((n) + (1 - ((dec) * 2))); \
	op4 RT0, RT3; \
	loadkr((n) + (1 - ((dec) * 2))); \
	eor rl, RT0;

#define F1(n, rl, rr, dec, loadkm, shiftkr, loadkr) \
	F(n, rl, rr, add, eor, sub, add, dec, loadkm, shiftkr, loadkr)
#define F2(n, rl, rr, dec, loadkm, shiftkr, loadkr) \
	F(n, rl, rr, eor, sub, add, eor, dec, loadkm, shiftkr, loadkr)
#define F3(n, rl, rr, dec, loadkm, shiftkr, loadkr) \
	F(n, rl, rr, sub, add, eor, sub, dec, loadkm, shiftkr, loadkr)

#define enc_round(n, Fx, rl, rr, loadkm, shiftkr, loadkr) \
	Fx(n, rl, rr, 0, loadkm, shiftkr, loadkr)

#define dec_round(n, Fx, rl, rr, loadkm, shiftkr, loadkr) \
	Fx(n, rl, rr, 1, loadkm, shiftkr, loadkr)

#define read_block_aligned(rin, offs, l0, r0, convert, rtmp) \
	ldr l0, [rin, #((offs) + 0)]; \
	ldr r0, [rin, #((offs) + 4)]; \
	convert(l0, rtmp); \
	convert(r0, rtmp);

#define write_block_aligned(rout, offs, l0, r0, convert, rtmp) \
	convert(l0, rtmp); \
	convert(r0, rtmp); \
	str l0, [rout, #((offs) + 0)]; \
	str r0, [rout, #((offs) + 4)];

#ifdef __ARM_FEATURE_UNALIGNED
	/* unaligned word reads allowed */
	#define read_block(rin, offs, l0, r0, rtmp0) \
		read_block_aligned(rin, offs, l0, r0, host_to_be, rtmp0)

	#define write_block(rout, offs, r0, l0, rtmp0, rtmp1) \
		write_block_aligned(rout, offs, r0, l0, be_to_host, rtmp0)

	#define read_block_host(rin, offs, l0, r0, rtmp0) \
		read_block_aligned(rin, offs, l0, r0, host_to_host, rtmp0)

	#define write_block_host(rout, offs, r0, l0, rtmp0, rtmp1) \
		write_block_aligned(rout, offs, r0, l0, host_to_host, rtmp0)
#else
	/* need to handle unaligned reads by byte reads */
	#define read_block(rin, offs, l0, r0, rtmp0) \
		tst rin, #3; \
		beq 1f; \
			ldr_unaligned_be(l0, rin, (offs) + 0, rtmp0); \
			ldr_unaligned_be(r0, rin, (offs) + 4, rtmp0); \
			b 2f; \
		1:;\
			read_block_aligned(rin, offs, l0, r0, host_to_be, rtmp0); \
		2:;

	#define write_block(rout, offs, l0, r0, rtmp0, rtmp1) \
		tst rout, #3; \
		beq 1f; \
			str_unaligned_be(l0, rout, (offs) + 0, rtmp0, rtmp1); \
			str_unaligned_be(r0, rout, (offs) + 4, rtmp0, rtmp1); \
			b 2f; \
		1:;\
			write_block_aligned(rout, offs, l0, r0, be_to_host, rtmp0); \
		2:;

	#define read_block_host(rin, offs, l0, r0, rtmp0) \
		tst rin, #3; \
		beq 1f; \
			ldr_unaligned_host(l0, rin, (offs) + 0, rtmp0); \
			ldr_unaligned_host(r0, rin, (offs) + 4, rtmp0); \
			b 2f; \
		1:;\
			read_block_aligned(rin, offs, l0, r0, host_to_host, rtmp0); \
		2:;

	#define write_block_host(rout, offs, l0, r0, rtmp0, rtmp1) \
		tst rout, #3; \
		beq 1f; \
			str_unaligned_host(l0, rout, (offs) + 0, rtmp0, rtmp1); \
			str_unaligned_host(r0, rout, (offs) + 4, rtmp0, rtmp1); \
			b 2f; \
		1:;\
			write_block_aligned(rout, offs, l0, r0, host_to_host, rtmp0); \
		2:;
#endif

.align 3
.globl _gcry_cast5_arm_encrypt_block
.type  _gcry_cast5_arm_encrypt_block,%function;

_gcry_cast5_arm_encrypt_block:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst
	 *	%r2: src
	 */
	push {%r1, %r4-%r11, %ip, %lr};

	GET_DATA_POINTER(Rs1, _gcry_cast5_s1to4, Rs2);
	mov RMASK, #(0xff << 2);
	add Rs2, Rs1, #(0x100*4);
	add Rs3, Rs1, #(0x100*4*2);
	add Rs4, Rs1, #(0x100*4*3);

	read_block(%r2, 0, RL0, RR0, RT0);

	load_km(0);
	load_kr(0);
	enc_round(0, F1, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(1, F2, RR0, RL0, load_km, shift_kr, dummy);
	enc_round(2, F3, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(3, F1, RR0, RL0, load_km, dummy, load_kr);
	enc_round(4, F2, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(5, F3, RR0, RL0, load_km, shift_kr, dummy);
	enc_round(6, F1, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(7, F2, RR0, RL0, load_km, dummy, load_kr);
	enc_round(8, F3, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(9, F1, RR0, RL0, load_km, shift_kr, dummy);
	enc_round(10, F2, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(11, F3, RR0, RL0, load_km, dummy, load_kr);
	enc_round(12, F1, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(13, F2, RR0, RL0, load_km, shift_kr, dummy);
	enc_round(14, F3, RL0, RR0, load_km, shift_kr, dummy);
	enc_round(15, F1, RR0, RL0, dummy, dummy, dummy);

	ldr %r1, [%sp], #4;
	write_block(%r1, 0, RR0, RL0, RT0, RT1);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_cast5_arm_encrypt_block,.-_gcry_cast5_arm_encrypt_block;

.align 3
.globl _gcry_cast5_arm_decrypt_block
.type  _gcry_cast5_arm_decrypt_block,%function;

_gcry_cast5_arm_decrypt_block:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst
	 *	%r2: src
	 */
	push {%r1, %r4-%r11, %ip, %lr};

	GET_DATA_POINTER(Rs1, _gcry_cast5_s1to4, Rs2);
	mov RMASK, #(0xff << 2);
	add Rs2, Rs1, #(0x100 * 4);
	add Rs3, Rs1, #(0x100 * 4 * 2);
	add Rs4, Rs1, #(0x100 * 4 * 3);

	read_block(%r2, 0, RL0, RR0, RT0);

	load_km(15);
	load_dec_kr(15);
	dec_round(15, F1, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(14, F3, RR0, RL0, load_km, shift_kr, dummy);
	dec_round(13, F2, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(12, F1, RR0, RL0, load_km, dummy, load_dec_kr);
	dec_round(11, F3, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(10, F2, RR0, RL0, load_km, shift_kr, dummy);
	dec_round(9, F1, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(8, F3, RR0, RL0, load_km, dummy, load_dec_kr);
	dec_round(7, F2, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(6, F1, RR0, RL0, load_km, shift_kr, dummy);
	dec_round(5, F3, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(4, F2, RR0, RL0, load_km, dummy, load_dec_kr);
	dec_round(3, F1, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(2, F3, RR0, RL0, load_km, shift_kr, dummy);
	dec_round(1, F2, RL0, RR0, load_km, shift_kr, dummy);
	dec_round(0, F1, RR0, RL0, dummy, dummy, dummy);

	ldr %r1, [%sp], #4;
	write_block(%r1, 0, RR0, RL0, RT0, RT1);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_cast5_arm_decrypt_block,.-_gcry_cast5_arm_decrypt_block;

/**********************************************************************
  2-way cast5
 **********************************************************************/

#define F_2w(n, rl0, rr0, rl1, rr1, op1, op2, op3, op4, dec, loadkm, shiftkr, \
	     loadkr) \
	op1 RT3, RKM, rr0; \
	op1 RKM, RKM, rr1; \
	mov RT3, RT3, ror RKR; \
	mov RKM, RKM, ror RKR; \
	\
	and RT0, RMASK, RT3, ror #(24); \
	and RT1, RMASK, RT3, lsr #(16); \
	and RT2, RMASK, RT3, lsr #(8); \
	and RT3, RMASK, RT3; \
	\
	ldr RT0, [Rs1, RT0]; \
	add RT2, #(0x100 * 4); \
	ldr RT1, [Rs2, RT1]; \
	add RT3, #(0x100 * 4 * 2); \
	\
	ldr RT2, [Rs2, RT2]; \
	\
	op2 RT0, RT1; \
	ldr RT3, [Rs2, RT3]; \
	and RT1, RMASK, RKM, ror #(24); \
	op3 RT0, RT2; \
	and RT2, RMASK, RKM, lsr #(16); \
	op4 RT0, RT3; \
	and RT3, RMASK, RKM, lsr #(8); \
	eor rl0, RT0; \
	add RT3, #(0x100 * 4); \
	ldr RT1, [Rs1, RT1]; \
	and RT0, RMASK, RKM; \
	ldr RT2, [Rs2, RT2]; \
	add RT0, #(0x100 * 4 * 2); \
	\
	ldr RT3, [Rs2, RT3]; \
	\
	op2 RT1, RT2; \
	ldr RT0, [Rs2, RT0]; \
	op3 RT1, RT3; \
	loadkm((n) + (1 - ((dec) * 2))); \
	op4 RT1, RT0; \
	loadkr((n) + (1 - ((dec) * 2))); \
	shiftkr(RKR); \
	eor rl1, RT1;

#define F1_2w(n, rl0, rr0, rl1, rr1, dec, loadkm, shiftkr, loadkr) \
	F_2w(n, rl0, rr0, rl1, rr1, add, eor, sub, add, dec, \
	     loadkm, shiftkr, loadkr)
#define F2_2w(n, rl0, rr0, rl1, rr1, dec, loadkm, shiftkr, loadkr) \
	F_2w(n, rl0, rr0, rl1, rr1, eor, sub, add, eor, dec, \
	     loadkm, shiftkr, loadkr)
#define F3_2w(n, rl0, rr0, rl1, rr1, dec, loadkm, shiftkr, loadkr) \
	F_2w(n, rl0, rr0, rl1, rr1, sub, add, eor, sub, dec, \
	     loadkm, shiftkr, loadkr)

#define enc_round2(n, Fx, rl, rr, loadkm, shiftkr, loadkr) \
	Fx##_2w(n, rl##0, rr##0, rl##1, rr##1, 0, loadkm, shiftkr, loadkr)

#define dec_round2(n, Fx, rl, rr, loadkm, shiftkr, loadkr) \
	Fx##_2w(n, rl##0, rr##0, rl##1, rr##1, 1, loadkm, shiftkr, loadkr)

#define read_block2_aligned(rin, l0, r0, l1, r1, convert, rtmp) \
	ldr l0, [rin, #(0)]; \
	ldr r0, [rin, #(4)]; \
	convert(l0, rtmp); \
	ldr l1, [rin, #(8)]; \
	convert(r0, rtmp); \
	ldr r1, [rin, #(12)]; \
	convert(l1, rtmp); \
	convert(r1, rtmp);

#define write_block2_aligned(rout, l0, r0, l1, r1, convert, rtmp) \
	convert(l0, rtmp); \
	convert(r0, rtmp); \
	convert(l1, rtmp); \
	str l0, [rout, #(0)]; \
	convert(r1, rtmp); \
	str r0, [rout, #(4)]; \
	str l1, [rout, #(8)]; \
	str r1, [rout, #(12)];

#ifdef __ARM_FEATURE_UNALIGNED
	/* unaligned word reads allowed */
	#define read_block2(rin, l0, r0, l1, r1, rtmp0) \
		read_block2_aligned(rin, l0, r0, l1, r1, host_to_be, rtmp0)

	#define write_block2(rout, l0, r0, l1, r1, rtmp0, rtmp1) \
		write_block2_aligned(rout, l0, r0, l1, r1, be_to_host, rtmp0)

	#define read_block2_host(rin, l0, r0, l1, r1, rtmp0) \
		read_block2_aligned(rin, l0, r0, l1, r1, host_to_host, rtmp0)

	#define write_block2_host(rout, l0, r0, l1, r1, rtmp0, rtmp1) \
		write_block2_aligned(rout, l0, r0, l1, r1, host_to_host, rtmp0)
#else
	/* need to handle unaligned reads by byte reads */
	#define read_block2(rin, l0, r0, l1, r1, rtmp0) \
		tst rin, #3; \
		beq 1f; \
			ldr_unaligned_be(l0, rin, 0, rtmp0); \
			ldr_unaligned_be(r0, rin, 4, rtmp0); \
			ldr_unaligned_be(l1, rin, 8, rtmp0); \
			ldr_unaligned_be(r1, rin, 12, rtmp0); \
			b 2f; \
		1:;\
			read_block2_aligned(rin, l0, r0, l1, r1, host_to_be, rtmp0); \
		2:;

	#define write_block2(rout, l0, r0, l1, r1, rtmp0, rtmp1) \
		tst rout, #3; \
		beq 1f; \
			str_unaligned_be(l0, rout, 0, rtmp0, rtmp1); \
			str_unaligned_be(r0, rout, 4, rtmp0, rtmp1); \
			str_unaligned_be(l1, rout, 8, rtmp0, rtmp1); \
			str_unaligned_be(r1, rout, 12, rtmp0, rtmp1); \
			b 2f; \
		1:;\
			write_block2_aligned(rout, l0, r0, l1, r1, be_to_host, rtmp0); \
		2:;

	#define read_block2_host(rin, l0, r0, l1, r1, rtmp0) \
		tst rin, #3; \
		beq 1f; \
			ldr_unaligned_host(l0, rin, 0, rtmp0); \
			ldr_unaligned_host(r0, rin, 4, rtmp0); \
			ldr_unaligned_host(l1, rin, 8, rtmp0); \
			ldr_unaligned_host(r1, rin, 12, rtmp0); \
			b 2f; \
		1:;\
			read_block2_aligned(rin, l0, r0, l1, r1, host_to_host, rtmp0); \
		2:;

	#define write_block2_host(rout, l0, r0, l1, r1, rtmp0, rtmp1) \
		tst rout, #3; \
		beq 1f; \
			str_unaligned_host(l0, rout, 0, rtmp0, rtmp1); \
			str_unaligned_host(r0, rout, 4, rtmp0, rtmp1); \
			str_unaligned_host(l1, rout, 8, rtmp0, rtmp1); \
			str_unaligned_host(r1, rout, 12, rtmp0, rtmp1); \
			b 2f; \
		1:;\
			write_block2_aligned(rout, l0, r0, l1, r1, host_to_host, rtmp0); \
		2:;
#endif

.align 3
.type  _gcry_cast5_arm_enc_blk2,%function;

_gcry_cast5_arm_enc_blk2:
	/* input:
	 *	preloaded: CTX
	 *	[RL0, RR0], [RL1, RR1]: src
	 * output:
	 *	[RR0, RL0], [RR1, RL1]: dst
	 */
	push {%lr};

	GET_DATA_POINTER(Rs1, _gcry_cast5_s1to4, Rs2);
	mov RMASK, #(0xff << 2);
	add Rs2, Rs1, #(0x100 * 4);

	load_km(0);
	load_kr(0);
	enc_round2(0, F1, RL, RR, load_km, shift_kr, dummy);
	enc_round2(1, F2, RR, RL, load_km, shift_kr, dummy);
	enc_round2(2, F3, RL, RR, load_km, shift_kr, dummy);
	enc_round2(3, F1, RR, RL, load_km, dummy, load_kr);
	enc_round2(4, F2, RL, RR, load_km, shift_kr, dummy);
	enc_round2(5, F3, RR, RL, load_km, shift_kr, dummy);
	enc_round2(6, F1, RL, RR, load_km, shift_kr, dummy);
	enc_round2(7, F2, RR, RL, load_km, dummy, load_kr);
	enc_round2(8, F3, RL, RR, load_km, shift_kr, dummy);
	enc_round2(9, F1, RR, RL, load_km, shift_kr, dummy);
	enc_round2(10, F2, RL, RR, load_km, shift_kr, dummy);
	enc_round2(11, F3, RR, RL, load_km, dummy, load_kr);
	enc_round2(12, F1, RL, RR, load_km, shift_kr, dummy);
	enc_round2(13, F2, RR, RL, load_km, shift_kr, dummy);
	enc_round2(14, F3, RL, RR, load_km, shift_kr, dummy);
	enc_round2(15, F1, RR, RL, dummy, dummy, dummy);

	host_to_be(RR0, RT0);
	host_to_be(RL0, RT0);
	host_to_be(RR1, RT0);
	host_to_be(RL1, RT0);

	pop {%pc};
.ltorg
.size _gcry_cast5_arm_enc_blk2,.-_gcry_cast5_arm_enc_blk2;

.align 3
.globl _gcry_cast5_arm_cfb_dec;
.type  _gcry_cast5_arm_cfb_dec,%function;

_gcry_cast5_arm_cfb_dec:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst (2 blocks)
	 *	%r2: src (2 blocks)
	 *	%r3: iv (64bit)
	 */
	push {%r1, %r2, %r4-%r11, %ip, %lr};

	mov %lr, %r3;

	/* Load input (iv/%r3 is aligned, src/%r2 might not be) */
	ldm %r3, {RL0, RR0};
	host_to_be(RL0, RT1);
	host_to_be(RR0, RT1);
	read_block(%r2, 0, RL1, RR1, %ip);

	/* Update IV, load src[1] and save to iv[0] */
	read_block_host(%r2, 8, %r5, %r6, %r7);
	stm %lr, {%r5, %r6};

	bl _gcry_cast5_arm_enc_blk2;
	/* result in RR0:RL0, RR1:RL1 = %r4:%r3, %r10:%r9 */

	/* %r0: dst, %r1: %src */
	pop {%r0, %r1};

	/* dst = src ^ result */
	read_block2_host(%r1, %r5, %r6, %r7, %r8, %lr);
	eor %r5, %r4;
	eor %r6, %r3;
	eor %r7, %r10;
	eor %r8, %r9;
	write_block2_host(%r0, %r5, %r6, %r7, %r8, %r1, %r2);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_cast5_arm_cfb_dec,.-_gcry_cast5_arm_cfb_dec;

.align 3
.globl _gcry_cast5_arm_ctr_enc;
.type  _gcry_cast5_arm_ctr_enc,%function;

_gcry_cast5_arm_ctr_enc:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst (2 blocks)
	 *	%r2: src (2 blocks)
	 *	%r3: iv (64bit, big-endian)
	 */
	push {%r1, %r2, %r4-%r11, %ip, %lr};

	mov %lr, %r3;

	/* Load IV (big => host endian) */
	read_block_aligned(%lr, 0, RL0, RR0, be_to_host, RT1);

	/* Construct IVs */
	adds RR1, RR0, #1; /* +1 */
	adc RL1, RL0, #0;
	adds %r6, RR1, #1; /* +2 */
	adc %r5, RL1, #0;

	/* Store new IV (host => big-endian) */
	write_block_aligned(%lr, 0, %r5, %r6, host_to_be, RT1);

	bl _gcry_cast5_arm_enc_blk2;
	/* result in RR0:RL0, RR1:RL1 = %r4:%r3, %r10:%r9 */

	/* %r0: dst, %r1: %src */
	pop {%r0, %r1};

	/* XOR key-stream with plaintext */
	read_block2_host(%r1, %r5, %r6, %r7, %r8, %lr);
	eor %r5, %r4;
	eor %r6, %r3;
	eor %r7, %r10;
	eor %r8, %r9;
	write_block2_host(%r0, %r5, %r6, %r7, %r8, %r1, %r2);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_cast5_arm_ctr_enc,.-_gcry_cast5_arm_ctr_enc;

.align 3
.type  _gcry_cast5_arm_dec_blk2,%function;

_gcry_cast5_arm_dec_blk2:
	/* input:
	 *	preloaded: CTX
	 *	[RL0, RR0], [RL1, RR1]: src
	 * output:
	 *	[RR0, RL0], [RR1, RL1]: dst
	 */

	GET_DATA_POINTER(Rs1, _gcry_cast5_s1to4, Rs2);
	mov RMASK, #(0xff << 2);
	add Rs2, Rs1, #(0x100 * 4);

	load_km(15);
	load_dec_kr(15);
	dec_round2(15, F1, RL, RR, load_km, shift_kr, dummy);
	dec_round2(14, F3, RR, RL, load_km, shift_kr, dummy);
	dec_round2(13, F2, RL, RR, load_km, shift_kr, dummy);
	dec_round2(12, F1, RR, RL, load_km, dummy, load_dec_kr);
	dec_round2(11, F3, RL, RR, load_km, shift_kr, dummy);
	dec_round2(10, F2, RR, RL, load_km, shift_kr, dummy);
	dec_round2(9, F1, RL, RR, load_km, shift_kr, dummy);
	dec_round2(8, F3, RR, RL, load_km, dummy, load_dec_kr);
	dec_round2(7, F2, RL, RR, load_km, shift_kr, dummy);
	dec_round2(6, F1, RR, RL, load_km, shift_kr, dummy);
	dec_round2(5, F3, RL, RR, load_km, shift_kr, dummy);
	dec_round2(4, F2, RR, RL, load_km, dummy, load_dec_kr);
	dec_round2(3, F1, RL, RR, load_km, shift_kr, dummy);
	dec_round2(2, F3, RR, RL, load_km, shift_kr, dummy);
	dec_round2(1, F2, RL, RR, load_km, shift_kr, dummy);
	dec_round2(0, F1, RR, RL, dummy, dummy, dummy);

	host_to_be(RR0, RT0);
	host_to_be(RL0, RT0);
	host_to_be(RR1, RT0);
	host_to_be(RL1, RT0);

	b .Ldec_cbc_tail;
.ltorg
.size _gcry_cast5_arm_dec_blk2,.-_gcry_cast5_arm_dec_blk2;

.align 3
.globl _gcry_cast5_arm_cbc_dec;
.type  _gcry_cast5_arm_cbc_dec,%function;

_gcry_cast5_arm_cbc_dec:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst (2 blocks)
	 *	%r2: src (2 blocks)
	 *	%r3: iv (64bit)
	 */
	push {%r1-%r11, %ip, %lr};

	read_block2(%r2, RL0, RR0, RL1, RR1, RT0);

	/* dec_blk2 is only used by cbc_dec, jump directly in/out instead
	 * of function call. */
	b _gcry_cast5_arm_dec_blk2;
.Ldec_cbc_tail:
	/* result in RR0:RL0, RR1:RL1 = %r4:%r3, %r10:%r9 */

	/* %r0: dst, %r1: %src, %r2: iv */
	pop {%r0-%r2};

	/* load IV+1 (src[0]) to %r7:%r8. Might be unaligned. */
	read_block_host(%r1, 0, %r7, %r8, %r5);
	/* load IV (iv[0]) to %r5:%r6. 'iv' is aligned. */
	ldm %r2, {%r5, %r6};

	/* out[1] ^= IV+1 */
	eor %r10, %r7;
	eor %r9, %r8;
	/* out[0] ^= IV */
	eor %r4, %r5;
	eor %r3, %r6;

	/* load IV+2 (src[1]) to %r7:%r8. Might be unaligned. */
	read_block_host(%r1, 8, %r7, %r8, %r5);
	/* store IV+2 to iv[0] (aligned). */
	stm %r2, {%r7, %r8};

	/* store result to dst[0-3]. Might be unaligned. */
	write_block2_host(%r0, %r4, %r3, %r10, %r9, %r5, %r6);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_cast5_arm_cbc_dec,.-_gcry_cast5_arm_cbc_dec;

#endif /*HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS*/
#endif /*__ARM_ARCH >= 6*/
