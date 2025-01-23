/* rijndael-aarch64.S  -  ARMv8/AArch64 assembly implementation of AES cipher
 *
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

#if defined(__AARCH64EL__)
#ifdef HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS

.text

/* register macros */
#define CTX	x0
#define RDST	x1
#define RSRC	x2
#define NROUNDS	w3
#define RTAB	x4
#define RMASK	w5

#define RA	w8
#define RB	w9
#define RC	w10
#define RD	w11

#define RNA	w12
#define RNB	w13
#define RNC	w14
#define RND	w15

#define RT0	w6
#define RT1	w7
#define RT2	w16
#define xRT0	x6
#define xRT1	x7
#define xRT2	x16

#define xw8	x8
#define xw9	x9
#define xw10	x10
#define xw11	x11

#define xw12	x12
#define xw13	x13
#define xw14	x14
#define xw15	x15

/***********************************************************************
 * ARMv8/AArch64 assembly implementation of the AES cipher
 ***********************************************************************/
#define preload_first_key(round, ra) \
	ldr ra, [CTX, #(((round) * 16) + 0 * 4)];

#define dummy(round, ra) /* nothing */

#define addroundkey(ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	ldp rna, rnb, [CTX]; \
	ldp rnc, rnd, [CTX, #8]; \
	eor ra, ra, rna; \
	eor rb, rb, rnb; \
	eor rc, rc, rnc; \
	preload_key(1, rna); \
	eor rd, rd, rnd;

#define do_encround(next_r, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	ldr rnb, [CTX, #(((next_r) * 16) + 1 * 4)]; \
	\
	and RT0, RMASK, ra, lsl#2; \
	ldr rnc, [CTX, #(((next_r) * 16) + 2 * 4)]; \
	and RT1, RMASK, ra, lsr#(8 - 2); \
	ldr rnd, [CTX, #(((next_r) * 16) + 3 * 4)]; \
	and RT2, RMASK, ra, lsr#(16 - 2); \
	ldr RT0, [RTAB, xRT0]; \
	and ra,  RMASK, ra, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rna, rna, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rd, lsl#2; \
	ldr ra,  [RTAB, x##ra]; \
	\
	eor rnd, rnd, RT1, ror #24; \
	and RT1, RMASK, rd, lsr#(8 - 2); \
	eor rnc, rnc, RT2, ror #16; \
	and RT2, RMASK, rd, lsr#(16 - 2); \
	eor rnb, rnb, ra, ror #8; \
	ldr RT0, [RTAB, xRT0]; \
	and rd,  RMASK, rd, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rnd, rnd, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rc, lsl#2; \
	ldr rd,  [RTAB, x##rd]; \
	\
	eor rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#(8 - 2); \
	eor rnb, rnb, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#(16 - 2); \
	eor rna, rna, rd, ror #8; \
	ldr RT0, [RTAB, xRT0]; \
	and rc,  RMASK, rc, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rnc, rnc, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rb, lsl#2; \
	ldr rc,  [RTAB, x##rc]; \
	\
	eor rnb, rnb, RT1, ror #24; \
	and RT1, RMASK, rb, lsr#(8 - 2); \
	eor rna, rna, RT2, ror #16; \
	and RT2, RMASK, rb, lsr#(16 - 2); \
	eor rnd, rnd, rc, ror #8; \
	ldr RT0, [RTAB, xRT0]; \
	and rb,  RMASK, rb, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rnb, rnb, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	eor rna, rna, RT1, ror #24; \
	ldr rb,  [RTAB, x##rb]; \
	\
	eor rnd, rnd, RT2, ror #16; \
	preload_key((next_r) + 1, ra); \
	eor rnc, rnc, rb, ror #8;

#define do_lastencround(ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	and RT0, RMASK, ra, lsl#2; \
	and RT1, RMASK, ra, lsr#(8 - 2); \
	and RT2, RMASK, ra, lsr#(16 - 2); \
	ldrb rna, [RTAB, xRT0]; \
	and ra,  RMASK, ra, lsr#(24 - 2); \
	ldrb rnd, [RTAB, xRT1]; \
	and RT0, RMASK, rd, lsl#2; \
	ldrb rnc, [RTAB, xRT2]; \
	ror rnd, rnd, #24; \
	ldrb rnb, [RTAB, x##ra]; \
	and RT1, RMASK, rd, lsr#(8 - 2); \
	ror rnc, rnc, #16; \
	and RT2, RMASK, rd, lsr#(16 - 2); \
	ror rnb, rnb, #8; \
	ldrb RT0, [RTAB, xRT0]; \
	and rd,  RMASK, rd, lsr#(24 - 2); \
	ldrb RT1, [RTAB, xRT1]; \
	\
	orr rnd, rnd, RT0; \
	ldrb RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rc, lsl#2; \
	ldrb rd,  [RTAB, x##rd]; \
	orr rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#(8 - 2); \
	orr rnb, rnb, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#(16 - 2); \
	orr rna, rna, rd, ror #8; \
	ldrb RT0, [RTAB, xRT0]; \
	and rc,  RMASK, rc, lsr#(24 - 2); \
	ldrb RT1, [RTAB, xRT1]; \
	\
	orr rnc, rnc, RT0; \
	ldrb RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rb, lsl#2; \
	ldrb rc,  [RTAB, x##rc]; \
	orr rnb, rnb, RT1, ror #24; \
	and RT1, RMASK, rb, lsr#(8 - 2); \
	orr rna, rna, RT2, ror #16; \
	ldrb RT0, [RTAB, xRT0]; \
	and RT2, RMASK, rb, lsr#(16 - 2); \
	ldrb RT1, [RTAB, xRT1]; \
	orr rnd, rnd, rc, ror #8; \
	ldrb RT2, [RTAB, xRT2]; \
	and rb,  RMASK, rb, lsr#(24 - 2); \
	ldrb rb,  [RTAB, x##rb]; \
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
	add CTX, CTX, #(((round) + 1) * 16); \
	add RTAB, RTAB, #1; \
	do_lastencround(ra, rb, rc, rd, rna, rnb, rnc, rnd); \
	addroundkey(rna, rnb, rnc, rnd, ra, rb, rc, rd, dummy);

.globl _gcry_aes_arm_encrypt_block
ELF(.type   _gcry_aes_arm_encrypt_block,%function;)

_gcry_aes_arm_encrypt_block:
	/* input:
	 *	%x0: keysched, CTX
	 *	%x1: dst
	 *	%x2: src
	 *	%w3: number of rounds.. 10, 12 or 14
	 *      %x4: encryption table
	 */
	CFI_STARTPROC();

	/* read input block */

	/* aligned load */
	ldp	RA, RB, [RSRC];
	ldp	RC, RD, [RSRC, #8];
#ifndef __AARCH64EL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif

	mov	RMASK, #(0xff<<2);

	firstencround(0, RA, RB, RC, RD, RNA, RNB, RNC, RND);
	encround(1, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(2, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(3, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(4, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(5, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);
	encround(6, RA, RB, RC, RD, RNA, RNB, RNC, RND, preload_first_key);
	encround(7, RNA, RNB, RNC, RND, RA, RB, RC, RD, preload_first_key);

	cmp	NROUNDS, #12;
	bge	.Lenc_not_128;

	encround(8, RA, RB, RC, RD, RNA, RNB, RNC, RND, dummy);
	lastencround(9, RNA, RNB, RNC, RND, RA, RB, RC, RD);

.Lenc_done:

	/* store output block */

	/* aligned store */
#ifndef __AARCH64EL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif
	/* write output block */
	stp	RA, RB, [RDST];
	stp	RC, RD, [RDST, #8];

	mov     x0, #(0);
	ret_spec_stop;

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
	CFI_ENDPROC();
ELF(.size _gcry_aes_arm_encrypt_block,.-_gcry_aes_arm_encrypt_block;)

#define addroundkey_dec(round, ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	ldr rna, [CTX, #(((round) * 16) + 0 * 4)]; \
	ldr rnb, [CTX, #(((round) * 16) + 1 * 4)]; \
	eor ra, ra, rna; \
	ldr rnc, [CTX, #(((round) * 16) + 2 * 4)]; \
	eor rb, rb, rnb; \
	ldr rnd, [CTX, #(((round) * 16) + 3 * 4)]; \
	eor rc, rc, rnc; \
	preload_first_key((round) - 1, rna); \
	eor rd, rd, rnd;

#define do_decround(next_r, ra, rb, rc, rd, rna, rnb, rnc, rnd, preload_key) \
	ldr rnb, [CTX, #(((next_r) * 16) + 1 * 4)]; \
	\
	and RT0, RMASK, ra, lsl#2; \
	ldr rnc, [CTX, #(((next_r) * 16) + 2 * 4)]; \
	and RT1, RMASK, ra, lsr#(8 - 2); \
	ldr rnd, [CTX, #(((next_r) * 16) + 3 * 4)]; \
	and RT2, RMASK, ra, lsr#(16 - 2); \
	ldr RT0, [RTAB, xRT0]; \
	and ra,  RMASK, ra, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rna, rna, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rb, lsl#2; \
	ldr ra,  [RTAB, x##ra]; \
	\
	eor rnb, rnb, RT1, ror #24; \
	and RT1, RMASK, rb, lsr#(8 - 2); \
	eor rnc, rnc, RT2, ror #16; \
	and RT2, RMASK, rb, lsr#(16 - 2); \
	eor rnd, rnd, ra, ror #8; \
	ldr RT0, [RTAB, xRT0]; \
	and rb,  RMASK, rb, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rnb, rnb, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rc, lsl#2; \
	ldr rb,  [RTAB, x##rb]; \
	\
	eor rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#(8 - 2); \
	eor rnd, rnd, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#(16 - 2); \
	eor rna, rna, rb, ror #8; \
	ldr RT0, [RTAB, xRT0]; \
	and rc,  RMASK, rc, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rnc, rnc, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rd, lsl#2; \
	ldr rc,  [RTAB, x##rc]; \
	\
	eor rnd, rnd, RT1, ror #24; \
	and RT1, RMASK, rd, lsr#(8 - 2); \
	eor rna, rna, RT2, ror #16; \
	and RT2, RMASK, rd, lsr#(16 - 2); \
	eor rnb, rnb, rc, ror #8; \
	ldr RT0, [RTAB, xRT0]; \
	and rd,  RMASK, rd, lsr#(24 - 2); \
	\
	ldr RT1, [RTAB, xRT1]; \
	eor rnd, rnd, RT0; \
	ldr RT2, [RTAB, xRT2]; \
	eor rna, rna, RT1, ror #24; \
	ldr rd,  [RTAB, x##rd]; \
	\
	eor rnb, rnb, RT2, ror #16; \
	preload_key((next_r) - 1, ra); \
	eor rnc, rnc, rd, ror #8;

#define do_lastdecround(ra, rb, rc, rd, rna, rnb, rnc, rnd) \
	and RT0, RMASK, ra; \
	and RT1, RMASK, ra, lsr#8; \
	and RT2, RMASK, ra, lsr#16; \
	ldrb rna, [RTAB, xRT0]; \
	lsr ra,  ra, #24; \
	ldrb rnb, [RTAB, xRT1]; \
	and RT0, RMASK, rb; \
	ldrb rnc, [RTAB, xRT2]; \
	ror rnb, rnb, #24; \
	ldrb rnd, [RTAB, x##ra]; \
	and RT1, RMASK, rb, lsr#8; \
	ror rnc, rnc, #16; \
	and RT2, RMASK, rb, lsr#16; \
	ror rnd, rnd, #8; \
	ldrb RT0, [RTAB, xRT0]; \
	lsr rb,  rb, #24; \
	ldrb RT1, [RTAB, xRT1]; \
	\
	orr rnb, rnb, RT0; \
	ldrb RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rc; \
	ldrb rb,  [RTAB, x##rb]; \
	orr rnc, rnc, RT1, ror #24; \
	and RT1, RMASK, rc, lsr#8; \
	orr rnd, rnd, RT2, ror #16; \
	and RT2, RMASK, rc, lsr#16; \
	orr rna, rna, rb, ror #8; \
	ldrb RT0, [RTAB, xRT0]; \
	lsr rc,  rc, #24; \
	ldrb RT1, [RTAB, xRT1]; \
	\
	orr rnc, rnc, RT0; \
	ldrb RT2, [RTAB, xRT2]; \
	and RT0, RMASK, rd; \
	ldrb rc,  [RTAB, x##rc]; \
	orr rnd, rnd, RT1, ror #24; \
	and RT1, RMASK, rd, lsr#8; \
	orr rna, rna, RT2, ror #16; \
	ldrb RT0, [RTAB, xRT0]; \
	and RT2, RMASK, rd, lsr#16; \
	ldrb RT1, [RTAB, xRT1]; \
	orr rnb, rnb, rc, ror #8; \
	ldrb RT2, [RTAB, xRT2]; \
	lsr rd,  rd, #24; \
	ldrb rd,  [RTAB, x##rd]; \
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
	add RTAB, RTAB, #(4 * 256); \
	do_lastdecround(ra, rb, rc, rd, rna, rnb, rnc, rnd); \
	addroundkey(rna, rnb, rnc, rnd, ra, rb, rc, rd, dummy);

.globl _gcry_aes_arm_decrypt_block
ELF(.type   _gcry_aes_arm_decrypt_block,%function;)

_gcry_aes_arm_decrypt_block:
	/* input:
	 *	%x0: keysched, CTX
	 *	%x1: dst
	 *	%x2: src
	 *	%w3: number of rounds.. 10, 12 or 14
	 *      %x4: decryption table
	 */
	CFI_STARTPROC();

	/* read input block */

	/* aligned load */
	ldp	RA, RB, [RSRC];
	ldp	RC, RD, [RSRC, #8];
#ifndef __AARCH64EL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif

	mov	RMASK, #(0xff << 2);

	cmp	NROUNDS, #12;
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

	/* store output block */

	/* aligned store */
#ifndef __AARCH64EL__
	rev	RA, RA;
	rev	RB, RB;
	rev	RC, RC;
	rev	RD, RD;
#endif
	/* write output block */
	stp	RA, RB, [RDST];
	stp	RC, RD, [RDST, #8];

	mov     x0, #(0);
	ret_spec_stop;

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
	CFI_ENDPROC();
ELF(.size _gcry_aes_arm_decrypt_block,.-_gcry_aes_arm_decrypt_block;)

#endif /*HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS*/
#endif /*__AARCH64EL__ */
