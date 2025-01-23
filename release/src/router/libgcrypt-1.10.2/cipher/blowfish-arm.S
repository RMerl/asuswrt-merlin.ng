/* blowfish-arm.S  -  ARM assembly implementation of Blowfish cipher
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

/* structure of crypto context */
#define s0	0
#define s1	(s0 + (1 * 256) * 4)
#define s2	(s0 + (2 * 256) * 4)
#define s3	(s0 + (3 * 256) * 4)
#define p	(s3 + (1 * 256) * 4)

/* register macros */
#define CTXs0 %r0
#define CTXs1 %r9
#define CTXs2 %r8
#define CTXs3 %r10
#define RMASK %lr
#define RKEYL %r2
#define RKEYR %ip

#define RL0 %r3
#define RR0 %r4

#define RL1 %r9
#define RR1 %r10

#define RT0 %r11
#define RT1 %r7
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

/***********************************************************************
 * 1-way blowfish
 ***********************************************************************/
#define F(l, r) \
	and RT0, RMASK, l, lsr#(24 - 2); \
	and RT1, RMASK, l, lsr#(16 - 2); \
	ldr RT0, [CTXs0, RT0]; \
	and RT2, RMASK, l, lsr#(8 - 2); \
	ldr RT1, [CTXs1, RT1]; \
	and RT3, RMASK, l, lsl#2; \
	ldr RT2, [CTXs2, RT2]; \
	add RT0, RT1; \
	ldr RT3, [CTXs3, RT3]; \
	eor RT0, RT2; \
	add RT0, RT3; \
	eor r, RT0;

#define load_roundkey_enc(n) \
	ldr RKEYL, [CTXs2, #((p - s2) + (4 * (n) + 0))]; \
	ldr RKEYR, [CTXs2, #((p - s2) + (4 * (n) + 4))];

#define add_roundkey_enc() \
	eor RL0, RKEYL; \
	eor RR0, RKEYR;

#define round_enc(n) \
	add_roundkey_enc(); \
	load_roundkey_enc(n); \
	\
	F(RL0, RR0); \
	F(RR0, RL0);

#define load_roundkey_dec(n) \
	ldr RKEYL, [CTXs2, #((p - s2) + (4 * ((n) - 1) + 4))]; \
	ldr RKEYR, [CTXs2, #((p - s2) + (4 * ((n) - 1) + 0))];

#define add_roundkey_dec() \
	eor RL0, RKEYL; \
	eor RR0, RKEYR;

#define round_dec(n) \
	add_roundkey_dec(); \
	load_roundkey_dec(n); \
	\
	F(RL0, RR0); \
	F(RR0, RL0);

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
			write_block_aligned(rout, offs, l0, r0, host_to_host); \
		2:;
#endif

.align 3
.type  __blowfish_enc_blk1,%function;

__blowfish_enc_blk1:
	/* input:
	 *	preloaded: CTX
	 *	[RL0, RR0]: src
	 * output:
	 *	[RR0, RL0]: dst
	 */
	push {%lr};

	add CTXs1, CTXs0, #(s1 - s0);
	add CTXs2, CTXs0, #(s2 - s0);
	mov RMASK, #(0xff << 2); /* byte mask */
	add CTXs3, CTXs1, #(s3 - s1);

	load_roundkey_enc(0);
	round_enc(2);
	round_enc(4);
	round_enc(6);
	round_enc(8);
	round_enc(10);
	round_enc(12);
	round_enc(14);
	round_enc(16);
	add_roundkey_enc();

	pop {%pc};
.size __blowfish_enc_blk1,.-__blowfish_enc_blk1;

.align 8
.globl  _gcry_blowfish_arm_do_encrypt
.type   _gcry_blowfish_arm_do_encrypt,%function;

_gcry_blowfish_arm_do_encrypt:
	/* input:
	 *	%r0: ctx, CTX
	 *	%r1: u32 *ret_xl
	 *	%r2: u32 *ret_xr
	 */
	push {%r2, %r4-%r11, %ip, %lr};

	ldr RL0, [%r1];
	ldr RR0, [%r2];

	bl __blowfish_enc_blk1;

	pop {%r2};
	str RR0, [%r1];
	str RL0, [%r2];

	pop {%r4-%r11, %ip, %pc};
.size _gcry_blowfish_arm_do_encrypt,.-_gcry_blowfish_arm_do_encrypt;

.align 3
.globl _gcry_blowfish_arm_encrypt_block
.type   _gcry_blowfish_arm_encrypt_block,%function;

_gcry_blowfish_arm_encrypt_block:
	/* input:
	 *	%r0: ctx, CTX
	 *	%r1: dst
	 *	%r2: src
	 */
	push {%r4-%r11, %ip, %lr};

	read_block(%r2, 0, RL0, RR0, RT0);

	bl __blowfish_enc_blk1;

	write_block(%r1, 0, RR0, RL0, RT0, RT1);

	pop {%r4-%r11, %ip, %pc};
.size _gcry_blowfish_arm_encrypt_block,.-_gcry_blowfish_arm_encrypt_block;

.align 3
.globl _gcry_blowfish_arm_decrypt_block
.type   _gcry_blowfish_arm_decrypt_block,%function;

_gcry_blowfish_arm_decrypt_block:
	/* input:
	 *	%r0: ctx, CTX
	 *	%r1: dst
	 *	%r2: src
	 */
	push {%r4-%r11, %ip, %lr};

	add CTXs1, CTXs0, #(s1 - s0);
	add CTXs2, CTXs0, #(s2 - s0);
	mov RMASK, #(0xff << 2); /* byte mask */
	add CTXs3, CTXs1, #(s3 - s1);

	read_block(%r2, 0, RL0, RR0, RT0);

	load_roundkey_dec(17);
	round_dec(15);
	round_dec(13);
	round_dec(11);
	round_dec(9);
	round_dec(7);
	round_dec(5);
	round_dec(3);
	round_dec(1);
	add_roundkey_dec();

	write_block(%r1, 0, RR0, RL0, RT0, RT1);

	pop {%r4-%r11, %ip, %pc};
.size _gcry_blowfish_arm_decrypt_block,.-_gcry_blowfish_arm_decrypt_block;

/***********************************************************************
 * 2-way blowfish
 ***********************************************************************/
#define F2(n, l0, r0, l1, r1, set_nextk, dec) \
	\
	and RT0, RMASK, l0, lsr#(24 - 2); \
	and RT1, RMASK, l0, lsr#(16 - 2); \
	and RT2, RMASK, l0, lsr#(8 - 2); \
	add RT1, #(s1 - s0); \
	\
	ldr RT0, [CTXs0, RT0]; \
	and RT3, RMASK, l0, lsl#2; \
	ldr RT1, [CTXs0, RT1]; \
	add RT3, #(s3 - s2); \
	ldr RT2, [CTXs2, RT2]; \
	add RT0, RT1; \
	ldr RT3, [CTXs2, RT3]; \
	\
	and RT1, RMASK, l1, lsr#(24 - 2); \
	eor RT0, RT2; \
	and RT2, RMASK, l1, lsr#(16 - 2); \
	add RT0, RT3; \
	add RT2, #(s1 - s0); \
	and RT3, RMASK, l1, lsr#(8 - 2); \
	eor r0, RT0; \
	\
	ldr RT1, [CTXs0, RT1]; \
	and RT0, RMASK, l1, lsl#2; \
	ldr RT2, [CTXs0, RT2]; \
	add RT0, #(s3 - s2); \
	ldr RT3, [CTXs2, RT3]; \
	add RT1, RT2; \
	ldr RT0, [CTXs2, RT0]; \
	\
	and RT2, RMASK, r0, lsr#(24 - 2); \
	eor RT1, RT3; \
	and RT3, RMASK, r0, lsr#(16 - 2); \
	add RT1, RT0; \
	add RT3, #(s1 - s0); \
	and RT0, RMASK, r0, lsr#(8 - 2); \
	eor r1, RT1; \
	\
	ldr RT2, [CTXs0, RT2]; \
	and RT1, RMASK, r0, lsl#2; \
	ldr RT3, [CTXs0, RT3]; \
	add RT1, #(s3 - s2); \
	ldr RT0, [CTXs2, RT0]; \
	add RT2, RT3; \
	ldr RT1, [CTXs2, RT1]; \
	\
	and RT3, RMASK, r1, lsr#(24 - 2); \
	eor RT2, RT0; \
	and RT0, RMASK, r1, lsr#(16 - 2); \
	add RT2, RT1; \
	add RT0, #(s1 - s0); \
	and RT1, RMASK, r1, lsr#(8 - 2); \
	eor l0, RT2; \
	\
	ldr RT3, [CTXs0, RT3]; \
	and RT2, RMASK, r1, lsl#2; \
	ldr RT0, [CTXs0, RT0]; \
	add RT2, #(s3 - s2); \
	ldr RT1, [CTXs2, RT1]; \
	eor l1, RKEYL; \
	ldr RT2, [CTXs2, RT2]; \
	\
	eor r0, RKEYR; \
	add RT3, RT0; \
	eor r1, RKEYR; \
	eor RT3, RT1; \
	eor l0, RKEYL; \
	add RT3, RT2; \
	set_nextk(RKEYL, (p - s2) + (4 * (n) + ((dec) * 4))); \
	eor l1, RT3; \
	set_nextk(RKEYR, (p - s2) + (4 * (n) + (!(dec) * 4)));

#define load_n_add_roundkey_enc2(n) \
	load_roundkey_enc(n); \
	eor RL0, RKEYL; \
	eor RR0, RKEYR; \
	eor RL1, RKEYL; \
	eor RR1, RKEYR; \
	load_roundkey_enc((n) + 2);

#define next_key(reg, offs) \
	ldr reg, [CTXs2, #(offs)];

#define dummy(x, y) /* do nothing */

#define round_enc2(n, load_next_key) \
	F2((n) + 2, RL0, RR0, RL1, RR1, load_next_key, 0);

#define load_n_add_roundkey_dec2(n) \
	load_roundkey_dec(n); \
	eor RL0, RKEYL; \
	eor RR0, RKEYR; \
	eor RL1, RKEYL; \
	eor RR1, RKEYR; \
	load_roundkey_dec((n) - 2);

#define round_dec2(n, load_next_key) \
	F2((n) - 3, RL0, RR0, RL1, RR1, load_next_key, 1);

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
.type  _gcry_blowfish_arm_enc_blk2,%function;

_gcry_blowfish_arm_enc_blk2:
	/* input:
	 *	preloaded: CTX
	 *	[RL0, RR0], [RL1, RR1]: src
	 * output:
	 *	[RR0, RL0], [RR1, RL1]: dst
	 */
	push {RT0,%lr};

	add CTXs2, CTXs0, #(s2 - s0);
	mov RMASK, #(0xff << 2); /* byte mask */

	load_n_add_roundkey_enc2(0);
	round_enc2(2, next_key);
	round_enc2(4, next_key);
	round_enc2(6, next_key);
	round_enc2(8, next_key);
	round_enc2(10, next_key);
	round_enc2(12, next_key);
	round_enc2(14, next_key);
	round_enc2(16, dummy);

	host_to_be(RR0, RT0);
	host_to_be(RL0, RT0);
	host_to_be(RR1, RT0);
	host_to_be(RL1, RT0);

	pop {RT0,%pc};
.size _gcry_blowfish_arm_enc_blk2,.-_gcry_blowfish_arm_enc_blk2;

.align 3
.globl _gcry_blowfish_arm_cfb_dec;
.type  _gcry_blowfish_arm_cfb_dec,%function;

_gcry_blowfish_arm_cfb_dec:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst (2 blocks)
	 *	%r2: src (2 blocks)
	 *	%r3: iv (64bit)
	 */
	push {%r2, %r4-%r11, %ip, %lr};

	mov %lr, %r3;

	/* Load input (iv/%r3 is aligned, src/%r2 might not be) */
	ldm %r3, {RL0, RR0};
	host_to_be(RL0, RT0);
	host_to_be(RR0, RT0);
	read_block(%r2, 0, RL1, RR1, RT0);

	/* Update IV, load src[1] and save to iv[0] */
	read_block_host(%r2, 8, %r5, %r6, RT0);
	stm %lr, {%r5, %r6};

	bl _gcry_blowfish_arm_enc_blk2;
	/* result in RR0:RL0, RR1:RL1 = %r4:%r3, %r10:%r9 */

	/* %r1: dst, %r0: %src */
	pop {%r0};

	/* dst = src ^ result */
	read_block2_host(%r0, %r5, %r6, %r7, %r8, %lr);
	eor %r5, %r4;
	eor %r6, %r3;
	eor %r7, %r10;
	eor %r8, %r9;
	write_block2_host(%r1, %r5, %r6, %r7, %r8, %r9, %r10);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_blowfish_arm_cfb_dec,.-_gcry_blowfish_arm_cfb_dec;

.align 3
.globl _gcry_blowfish_arm_ctr_enc;
.type  _gcry_blowfish_arm_ctr_enc,%function;

_gcry_blowfish_arm_ctr_enc:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst (2 blocks)
	 *	%r2: src (2 blocks)
	 *	%r3: iv (64bit, big-endian)
	 */
	push {%r2, %r4-%r11, %ip, %lr};

	mov %lr, %r3;

	/* Load IV (big => host endian) */
	read_block_aligned(%lr, 0, RL0, RR0, be_to_host, RT0);

	/* Construct IVs */
	adds RR1, RR0, #1; /* +1 */
	adc RL1, RL0, #0;
	adds %r6, RR1, #1; /* +2 */
	adc %r5, RL1, #0;

	/* Store new IV (host => big-endian) */
	write_block_aligned(%lr, 0, %r5, %r6, host_to_be, RT0);

	bl _gcry_blowfish_arm_enc_blk2;
	/* result in RR0:RL0, RR1:RL1 = %r4:%r3, %r10:%r9 */

	/* %r1: dst, %r0: %src */
	pop {%r0};

	/* XOR key-stream with plaintext */
	read_block2_host(%r0, %r5, %r6, %r7, %r8, %lr);
	eor %r5, %r4;
	eor %r6, %r3;
	eor %r7, %r10;
	eor %r8, %r9;
	write_block2_host(%r1, %r5, %r6, %r7, %r8, %r9, %r10);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_blowfish_arm_ctr_enc,.-_gcry_blowfish_arm_ctr_enc;

.align 3
.type  _gcry_blowfish_arm_dec_blk2,%function;

_gcry_blowfish_arm_dec_blk2:
	/* input:
	 *	preloaded: CTX
	 *	[RL0, RR0], [RL1, RR1]: src
	 * output:
	 *	[RR0, RL0], [RR1, RL1]: dst
	 */
	add CTXs2, CTXs0, #(s2 - s0);
	mov RMASK, #(0xff << 2); /* byte mask */

	load_n_add_roundkey_dec2(17);
	round_dec2(15, next_key);
	round_dec2(13, next_key);
	round_dec2(11, next_key);
	round_dec2(9, next_key);
	round_dec2(7, next_key);
	round_dec2(5, next_key);
	round_dec2(3, next_key);
	round_dec2(1, dummy);

	host_to_be(RR0, RT0);
	host_to_be(RL0, RT0);
	host_to_be(RR1, RT0);
	host_to_be(RL1, RT0);

	b .Ldec_cbc_tail;
.ltorg
.size _gcry_blowfish_arm_dec_blk2,.-_gcry_blowfish_arm_dec_blk2;

.align 3
.globl _gcry_blowfish_arm_cbc_dec;
.type  _gcry_blowfish_arm_cbc_dec,%function;

_gcry_blowfish_arm_cbc_dec:
	/* input:
	 *	%r0: CTX
	 *	%r1: dst (2 blocks)
	 *	%r2: src (2 blocks)
	 *	%r3: iv (64bit)
	 */
	push {%r2-%r11, %ip, %lr};

	read_block2(%r2, RL0, RR0, RL1, RR1, RT0);

	/* dec_blk2 is only used by cbc_dec, jump directly in/out instead
	 * of function call. */
	b _gcry_blowfish_arm_dec_blk2;
.Ldec_cbc_tail:
	/* result in RR0:RL0, RR1:RL1 = %r4:%r3, %r10:%r9 */

	/* %r0: %src, %r1: dst, %r2: iv */
	pop {%r0, %r2};

	/* load IV+1 (src[0]) to %r7:%r8. Might be unaligned. */
	read_block_host(%r0, 0, %r7, %r8, %r5);
	/* load IV (iv[0]) to %r5:%r6. 'iv' is aligned. */
	ldm %r2, {%r5, %r6};

	/* out[1] ^= IV+1 */
	eor %r10, %r7;
	eor %r9, %r8;
	/* out[0] ^= IV */
	eor %r4, %r5;
	eor %r3, %r6;

	/* load IV+2 (src[1]) to %r7:%r8. Might be unaligned. */
	read_block_host(%r0, 8, %r7, %r8, %r5);
	/* store IV+2 to iv[0] (aligned). */
	stm %r2, {%r7, %r8};

	/* store result to dst[0-3]. Might be unaligned. */
	write_block2_host(%r1, %r4, %r3, %r10, %r9, %r5, %r6);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_blowfish_arm_cbc_dec,.-_gcry_blowfish_arm_cbc_dec;

#endif /*HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS*/
#endif /*__ARM_ARCH >= 6*/
