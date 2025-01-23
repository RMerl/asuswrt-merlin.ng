/* rijndael-arm.S  -  ARM assembly implementation of AES cipher
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

/* register macros */
#define CTX	%r0
#define RTAB	%lr
#define RMASK	%ip

#define RA	%r4
#define RB	%r5
#define RC	%r6
#define RD	%r7

#define RNA	%r8
#define RNB	%r9
#define RNC	%r10
#define RND	%r11

#define RT0	%r1
#define RT1	%r2
#define RT2	%r3

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

/***********************************************************************
 * ARM assembly implementation of the AES cipher
 ***********************************************************************/
#define preload_first_key(round, ra) \
	ldr ra, [CTX, #(((round) * 16) + 0 * 4)];

#define dummy(round, ra) /* nothing */

#define addroundkey(ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	ldm CTX, {rna, rnb, rnc, rnd}; \
	eor ra, rna; \
	eor rb, rnb; \
	eor rc, rnc; \
	preload_key(1, rna); \
	eor rd, rnd;

#define do_encround(next_r, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	ldr rnb, [CTX, #(((next_r) * 16) + 1 * 4)]; \
	\
	and RT0, RMASK, ra, lsl#2; \
	ldr rnc, [CTX, #(((next_r) * 16) + 2 * 4)]; \
	and RT1, RMASK, ra, lsr#(8 - 2); \
	ldr rnd, [CTX, #(((next_r) * 16) + 3 * 4)]; \
	and RT2, RMASK, ra, lsr#(16 - 2); \
	ldr RT0, [RTAB, RT0]; \
	and ra,  RMASK, ra, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rna, rna, RT0; \
	ldr RT2, [RTAB, RT2]; \
	and RT0, RMASK, rd, lsl#2; \
	ldr ra,  [RTAB, ra]; \
	\
	eor rnd, rnd, RT1, ror #24; \
	and RT1, RMASK, rd, lsr#(8 - 2); \
	eor rnc, rnc, RT2, ror #16; \
	and RT2, RMASK, rd, lsr#(16 - 2); \
	eor rnb, rnb, ra, ror #8; \
	ldr RT0, [RTAB, RT0]; \
	and rd,  RMASK, rd, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rnd, rnd, RT0; \
	ldr RT2, [RTAB, RT2]; \
	and RT0, RMASK, rc, lsl#2; \
	ldr rd,  [RTAB, rd]; \
	\
	eor rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#(8 - 2); \
	eor rnb, rnb, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#(16 - 2); \
	eor rna, rna, rd, ror #8; \
	ldr RT0, [RTAB, RT0]; \
	and rc,  RMASK, rc, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rnc, rnc, RT0; \
	ldr RT2, [RTAB, RT2]; \
	and RT0, RMASK, rb, lsl#2; \
	ldr rc,  [RTAB, rc]; \
	\
	eor rnb, rnb, RT1, ror #24; \
	and RT1, RMASK, rb, lsr#(8 - 2); \
	eor rna, rna, RT2, ror #16; \
	and RT2, RMASK, rb, lsr#(16 - 2); \
	eor rnd, rnd, rc, ror #8; \
	ldr RT0, [RTAB, RT0]; \
	and rb,  RMASK, rb, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rnb, rnb, RT0; \
	ldr RT2, [RTAB, RT2]; \
	eor rna, rna, RT1, ror #24; \
	ldr rb,  [RTAB, rb]; \
	\
	eor rnd, rnd, RT2, ror #16; \
	preload_key((next_r) + 1, ra); \
	eor rnc, rnc, rb, ror #8;

#define do_lastencround(ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	and RT0, RMASK, ra, lsl#2; \
	and RT1, RMASK, ra, lsr#(8 - 2); \
	and RT2, RMASK, ra, lsr#(16 - 2); \
	ldrb rna, [RTAB, RT0]; \
	and ra,  RMASK, ra, lsr#(24 - 2); \
	ldrb rnd, [RTAB, RT1]; \
	and RT0, RMASK, rd, lsl#2; \
	ldrb rnc, [RTAB, RT2]; \
	mov rnd, rnd, ror #24; \
	ldrb rnb, [RTAB, ra]; \
	and RT1, RMASK, rd, lsr#(8 - 2); \
	mov rnc, rnc, ror #16; \
	and RT2, RMASK, rd, lsr#(16 - 2); \
	mov rnb, rnb, ror #8; \
	ldrb RT0, [RTAB, RT0]; \
	and rd,  RMASK, rd, lsr#(24 - 2); \
	ldrb RT1, [RTAB, RT1]; \
	\
	orr rnd, rnd, RT0; \
	ldrb RT2, [RTAB, RT2]; \
	and RT0, RMASK, rc, lsl#2; \
	ldrb rd,  [RTAB, rd]; \
	orr rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#(8 - 2); \
	orr rnb, rnb, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#(16 - 2); \
	orr rna, rna, rd, ror #8; \
	ldrb RT0, [RTAB, RT0]; \
	and rc,  RMASK, rc, lsr#(24 - 2); \
	ldrb RT1, [RTAB, RT1]; \
	\
	orr rnc, rnc, RT0; \
	ldrb RT2, [RTAB, RT2]; \
	and RT0, RMASK, rb, lsl#2; \
	ldrb rc,  [RTAB, rc]; \
	orr rnb, rnb, RT1, ror #24; \
	and RT1, RMASK, rb, lsr#(8 - 2); \
	orr rna, rna, RT2, ror #16; \
	ldrb RT0, [RTAB, RT0]; \
	and RT2, RMASK, rb, lsr#(16 - 2); \
	ldrb RT1, [RTAB, RT1]; \
	orr rnd, rnd, rc, ror #8; \
	ldrb RT2, [RTAB, RT2]; \
	and rb,  RMASK, rb, lsr#(24 - 2); \
	ldrb rb,  [RTAB, rb]; \
	\
	orr rnb, rnb, RT0; \
	orr rna, rna, RT1, ror #24; \
	orr rnd, rnd, RT2, ror #16; \
	orr rnc, rnc, rb, ror #8;

#define firstencround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	addroundkey(ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_first_key); \
	do_encround((round) + 1, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_first_key);

#define encround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	do_encround((round) + 1, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key);

#define lastencround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	add CTX, #(((round) + 1) * 16); \
	add RTAB, #1; \
	do_lastencround(ra, rb, rc, rd, rna, rnb, rnc, rnd); \
	addroundkey(rna, rnb, rnc, rnd, ra, rb, rc, rd, dummy);

.align 3
.globl _gcry_aes_arm_encrypt_block
.type   _gcry_aes_arm_encrypt_block,%function;

_gcry_aes_arm_encrypt_block:
	/* input:
	 *	%r0: keysched, CTX
	 *	%r1: dst
	 *	%r2: src
	 *	%r3: number of rounds.. 10, 12 or 14
	 *      %st+0: encryption table
	 */
	push {%r4-%r11, %ip, %lr};

	/* read input block */

	/* test if src is unaligned */
	tst	%r2, #3;
	beq	1f;

	/* unaligned load */
	ldr_unaligned_le(RA, %r2, 0, RNA);
	ldr_unaligned_le(RB, %r2, 4, RNB);
	ldr_unaligned_le(RC, %r2, 8, RNA);
	ldr_unaligned_le(RD, %r2, 12, RNB);
	b	2f;
.ltorg
1:
	/* aligned load */
	ldm	%r2, {RA, RB, RC, RD};
#ifndef __ARMEL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif
2:
	ldr     RTAB, [%sp, #40];
	sub	%sp, #16;

	str	%r1, [%sp, #4];		/* dst */
	mov	RMASK, #0xff;
	str	%r3, [%sp, #8];		/* nrounds */
	mov	RMASK, RMASK, lsl#2;	/* byte mask */

	firstencround(0, RA, RB, RC, RD, RNA, RNB, RNC, RND);
	encround(1, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(2, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(3, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(4, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(5, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(6, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(7, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);

	ldr	RT0, [%sp, #8];		/* nrounds */
	cmp	RT0, #12;
	bge	.Lenc_not_128;

	encround(8, RA, RB, RC, RD, RNA, RNB, RNC, RND, dummy);
	lastencround(9, RNA, RNB, RNC, RND, RA, RB, RC, RD);

.Lenc_done:
	ldr	RT0, [%sp, #4];		/* dst */
	add	%sp, #16;

	/* store output block */

	/* test if dst is unaligned */
	tst	RT0, #3;
	beq	1f;

	/* unaligned store */
	str_unaligned_le(RA, RT0, 0, RNA, RNB);
	str_unaligned_le(RB, RT0, 4, RNA, RNB);
	str_unaligned_le(RC, RT0, 8, RNA, RNB);
	str_unaligned_le(RD, RT0, 12, RNA, RNB);
	b	2f;
.ltorg
1:
	/* aligned store */
#ifndef __ARMEL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif
	/* write output block */
	stm	RT0, {RA, RB, RC, RD};
2:

	mov     r0, #(10 * 4);
	pop {%r4-%r11, %ip, %pc};

.ltorg
.Lenc_not_128:
	beq .Lenc_192

	encround(8, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(9, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(10, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(11, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(12, RA, RB, RC, RD, RNA, RNB, RNC, RND, dummy);
	lastencround(13, RNA, RNB, RNC, RND, RA, RB, RC, RD);

	b .Lenc_done;

.ltorg
.Lenc_192:
	encround(8, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(9, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(10, RA, RB, RC, RD, RNA, RNB, RNC, RND, dummy);
	lastencround(11, RNA, RNB, RNC, RND, RA, RB, RC, RD);

	b .Lenc_done;
.size _gcry_aes_arm_encrypt_block,.-_gcry_aes_arm_encrypt_block;

#define addroundkey_dec(round, ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	ldr rna, [CTX, #(((round) * 16) + 0 * 4)]; \
	ldr rnb, [CTX, #(((round) * 16) + 1 * 4)]; \
	eor ra, rna; \
	ldr rnc, [CTX, #(((round) * 16) + 2 * 4)]; \
	eor rb, rnb; \
	ldr rnd, [CTX, #(((round) * 16) + 3 * 4)]; \
	eor rc, rnc; \
	preload_first_key((round) - 1, rna); \
	eor rd, rnd;

#define do_decround(next_r, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	ldr rnb, [CTX, #(((next_r) * 16) + 1 * 4)]; \
	\
	and RT0, RMASK, ra, lsl#2; \
	ldr rnc, [CTX, #(((next_r) * 16) + 2 * 4)]; \
	and RT1, RMASK, ra, lsr#(8 - 2); \
	ldr rnd, [CTX, #(((next_r) * 16) + 3 * 4)]; \
	and RT2, RMASK, ra, lsr#(16 - 2); \
	ldr RT0, [RTAB, RT0]; \
	and ra,  RMASK, ra, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rna, rna, RT0; \
	ldr RT2, [RTAB, RT2]; \
	and RT0, RMASK, rb, lsl#2; \
	ldr ra,  [RTAB, ra]; \
	\
	eor rnb, rnb, RT1, ror #24; \
	and RT1, RMASK, rb, lsr#(8 - 2); \
	eor rnc, rnc, RT2, ror #16; \
	and RT2, RMASK, rb, lsr#(16 - 2); \
	eor rnd, rnd, ra, ror #8; \
	ldr RT0, [RTAB, RT0]; \
	and rb,  RMASK, rb, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rnb, rnb, RT0; \
	ldr RT2, [RTAB, RT2]; \
	and RT0, RMASK, rc, lsl#2; \
	ldr rb,  [RTAB, rb]; \
	\
	eor rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#(8 - 2); \
	eor rnd, rnd, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#(16 - 2); \
	eor rna, rna, rb, ror #8; \
	ldr RT0, [RTAB, RT0]; \
	and rc,  RMASK, rc, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rnc, rnc, RT0; \
	ldr RT2, [RTAB, RT2]; \
	and RT0, RMASK, rd, lsl#2; \
	ldr rc,  [RTAB, rc]; \
	\
	eor rnd, rnd, RT1, ror #24; \
	and RT1, RMASK, rd, lsr#(8 - 2); \
	eor rna, rna, RT2, ror #16; \
	and RT2, RMASK, rd, lsr#(16 - 2); \
	eor rnb, rnb, rc, ror #8; \
	ldr RT0, [RTAB, RT0]; \
	and rd,  RMASK, rd, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, RT1]; \
	eor rnd, rnd, RT0; \
	ldr RT2, [RTAB, RT2]; \
	eor rna, rna, RT1, ror #24; \
	ldr rd,  [RTAB, rd]; \
	\
	eor rnb, rnb, RT2, ror #16; \
	preload_key((next_r) - 1, ra); \
	eor rnc, rnc, rd, ror #8;

#define do_lastdecround(ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	and RT0, RMASK, ra; \
	and RT1, RMASK, ra, lsr#8; \
	and RT2, RMASK, ra, lsr#16; \
	ldrb rna, [RTAB, RT0]; \
	mov ra,  ra, lsr#24; \
	ldrb rnb, [RTAB, RT1]; \
	and RT0, RMASK, rb; \
	ldrb rnc, [RTAB, RT2]; \
	mov rnb, rnb, ror #24; \
	ldrb rnd, [RTAB, ra]; \
	and RT1, RMASK, rb, lsr#8; \
	mov rnc, rnc, ror #16; \
	and RT2, RMASK, rb, lsr#16; \
	mov rnd, rnd, ror #8; \
	ldrb RT0, [RTAB, RT0]; \
	mov rb,  rb, lsr#24; \
	ldrb RT1, [RTAB, RT1]; \
	\
	orr rnb, rnb, RT0; \
	ldrb RT2, [RTAB, RT2]; \
	and RT0, RMASK, rc; \
	ldrb rb,  [RTAB, rb]; \
	orr rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#8; \
	orr rnd, rnd, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#16; \
	orr rna, rna, rb, ror #8; \
	ldrb RT0, [RTAB, RT0]; \
	mov rc,  rc, lsr#24; \
	ldrb RT1, [RTAB, RT1]; \
	\
	orr rnc, rnc, RT0; \
	ldrb RT2, [RTAB, RT2]; \
	and RT0, RMASK, rd; \
	ldrb rc,  [RTAB, rc]; \
	orr rnd, rnd, RT1, ror #24; \
	and RT1, RMASK, rd, lsr#8; \
	orr rna, rna, RT2, ror #16; \
	ldrb RT0, [RTAB, RT0]; \
	and RT2, RMASK, rd, lsr#16; \
	ldrb RT1, [RTAB, RT1]; \
	orr rnb, rnb, rc, ror #8; \
	ldrb RT2, [RTAB, RT2]; \
	mov rd,  rd, lsr#24; \
	ldrb rd,  [RTAB, rd]; \
	\
	orr rnd, rnd, RT0; \
	orr rna, rna, RT1, ror #24; \
	orr rnb, rnb, RT2, ror #16; \
	orr rnc, rnc, rd, ror #8;

#define firstdecround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	addroundkey_dec(((round) + 1), ra, rb, rc, rd, rna, rnb, rnc, rnd); \
	do_decround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_first_key);

#define decround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	do_decround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key);

#define set_last_round_rmask(_, __) \
	mov RMASK, #0xff;

#define lastdecround(round, ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	add RTAB, #(4 * 256); \
	do_lastdecround(ra, rb, rc, rd, rna, rnb, rnc, rnd); \
	addroundkey(rna, rnb, rnc, rnd, ra, rb, rc, rd, dummy);

.align 3
.globl _gcry_aes_arm_decrypt_block
.type   _gcry_aes_arm_decrypt_block,%function;

_gcry_aes_arm_decrypt_block:
	/* input:
	 *	%r0: keysched, CTX
	 *	%r1: dst
	 *	%r2: src
	 *	%r3: number of rounds.. 10, 12 or 14
	 *      %st+0: decryption table
	 */
	push {%r4-%r11, %ip, %lr};

	/* read input block */

	/* test if src is unaligned */
	tst	%r2, #3;
	beq	1f;

	/* unaligned load */
	ldr_unaligned_le(RA, %r2, 0, RNA);
	ldr_unaligned_le(RB, %r2, 4, RNB);
	ldr_unaligned_le(RC, %r2, 8, RNA);
	ldr_unaligned_le(RD, %r2, 12, RNB);
	b	2f;
.ltorg
1:
	/* aligned load */
	ldm	%r2, {RA, RB, RC, RD};
#ifndef __ARMEL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif
2:
	ldr     RTAB, [%sp, #40];
	sub	%sp, #16;

	mov	RMASK, #0xff;
	str	%r1, [%sp, #4];		/* dst */
	mov	RMASK, RMASK, lsl#2;	/* byte mask */

	cmp	%r3, #12;
	bge	.Ldec_256;

	firstdecround(9, RA, RB, RC, RD, RNA, RNB, RNC, RND);
.Ldec_tail:
	decround(8, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(7, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	decround(6, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(5, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	decround(4, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(3, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	decround(2, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(1, RA, RB, RC, RD, RNA, RNB, RNC, RND, set_last_round_rmask);
	lastdecround(0, RNA, RNB, RNC, RND, RA, RB, RC, RD);

	ldr	RT0, [%sp, #4];		/* dst */
	add	%sp, #16;

	/* store output block */

	/* test if dst is unaligned */
	tst	RT0, #3;
	beq	1f;

	/* unaligned store */
	str_unaligned_le(RA, RT0, 0, RNA, RNB);
	str_unaligned_le(RB, RT0, 4, RNA, RNB);
	str_unaligned_le(RC, RT0, 8, RNA, RNB);
	str_unaligned_le(RD, RT0, 12, RNA, RNB);
	b	2f;
.ltorg
1:
	/* aligned store */
#ifndef __ARMEL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif
	/* write output block */
	stm	RT0, {RA, RB, RC, RD};
2:
	mov     r0, #(10 * 4);
	pop {%r4-%r11, %ip, %pc};

.ltorg
.Ldec_256:
	beq .Ldec_192;

	firstdecround(13, RA, RB, RC, RD, RNA, RNB, RNC, RND);
	decround(12, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(11, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	decround(10, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(9, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);

	b .Ldec_tail;

.ltorg
.Ldec_192:
	firstdecround(11, RA, RB, RC, RD, RNA, RNB, RNC, RND);
	decround(10, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	decround(9, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);

	b .Ldec_tail;
.size _gcry_aes_arm_encrypt_block,.-_gcry_aes_arm_encrypt_block;

#endif /*HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS*/
#endif /*__ARMEL__ */
