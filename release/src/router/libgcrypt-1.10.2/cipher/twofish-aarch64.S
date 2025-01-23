/* twofish-aarch64.S  -  ARMv8/AArch64 assembly implementation of Twofish cipher
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

/* structure of TWOFISH_context: */
#define s0 0
#define s1 ((s0) + 4 * 256)
#define s2 ((s1) + 4 * 256)
#define s3 ((s2) + 4 * 256)
#define w  ((s3) + 4 * 256)
#define k  ((w) + 4 * 8)

/* register macros */
#define CTX x0
#define RDST x1
#define RSRC x2
#define CTXs0 CTX
#define CTXs1 x3
#define CTXs2 x4
#define CTXs3 x5
#define CTXw x17

#define RA w6
#define RB w7
#define RC w8
#define RD w9

#define RX w10
#define RY w11

#define xRX x10
#define xRY x11

#define RMASK w12

#define RT0 w13
#define RT1 w14
#define RT2 w15
#define RT3 w16

#define xRT0 x13
#define xRT1 x14
#define xRT2 x15
#define xRT3 x16

/* helper macros */
#ifndef __AARCH64EL__
	/* bswap on big-endian */
	#define host_to_le(reg) \
		rev reg, reg;
	#define le_to_host(reg) \
		rev reg, reg;
#else
	/* nop on little-endian */
	#define host_to_le(reg) /*_*/
	#define le_to_host(reg) /*_*/
#endif

#define ldr_input_aligned_le(rin, a, b, c, d) \
	ldr a, [rin, #0]; \
	ldr b, [rin, #4]; \
	le_to_host(a); \
	ldr c, [rin, #8]; \
	le_to_host(b); \
	ldr d, [rin, #12]; \
	le_to_host(c); \
	le_to_host(d);

#define str_output_aligned_le(rout, a, b, c, d) \
	le_to_host(a); \
	le_to_host(b); \
	str a, [rout, #0]; \
	le_to_host(c); \
	str b, [rout, #4]; \
	le_to_host(d); \
	str c, [rout, #8]; \
	str d, [rout, #12];

/* unaligned word reads/writes allowed */
#define ldr_input_le(rin, ra, rb, rc, rd, rtmp) \
	ldr_input_aligned_le(rin, ra, rb, rc, rd)

#define str_output_le(rout, ra, rb, rc, rd, rtmp0, rtmp1) \
	str_output_aligned_le(rout, ra, rb, rc, rd)

/**********************************************************************
  1-way twofish
 **********************************************************************/
#define encrypt_round(a, b, rc, rd, n, ror_a, adj_a) \
	and RT0, RMASK, b, lsr#(8 - 2); \
	and RY, RMASK, b, lsr#(16 - 2); \
	and RT1, RMASK, b, lsr#(24 - 2); \
	ldr RY, [CTXs3, xRY]; \
	and RT2, RMASK, b, lsl#(2); \
	ldr RT0, [CTXs2, xRT0]; \
	and RT3, RMASK, a, lsr#(16 - 2 + (adj_a)); \
	ldr RT1, [CTXs0, xRT1]; \
	and RX, RMASK, a, lsr#(8 - 2 + (adj_a)); \
	ldr RT2, [CTXs1, xRT2]; \
	ldr RX, [CTXs1, xRX]; \
	ror_a(a); \
	\
	eor RY, RY, RT0; \
	ldr RT3, [CTXs2, xRT3]; \
	and RT0, RMASK, a, lsl#(2); \
	eor RY, RY, RT1; \
	and RT1, RMASK, a, lsr#(24 - 2); \
	eor RY, RY, RT2; \
	ldr RT0, [CTXs0, xRT0]; \
	eor RX, RX, RT3; \
	ldr RT1, [CTXs3, xRT1]; \
	eor RX, RX, RT0; \
	\
	ldr RT3, [CTXs3, #(k - s3 + 8 * (n) + 4)]; \
	eor RX, RX, RT1; \
	ldr RT2, [CTXs3, #(k - s3 + 8 * (n))]; \
	\
	add RT0, RX, RY, lsl #1; \
	add RX, RX, RY; \
	add RT0, RT0, RT3; \
	add RX, RX, RT2; \
	eor rd, RT0, rd, ror #31; \
	eor rc, rc, RX;

#define dummy(x) /*_*/

#define ror1(r) \
	ror r, r, #1;

#define decrypt_round(a, b, rc, rd, n, ror_b, adj_b) \
	and RT3, RMASK, b, lsl#(2 - (adj_b)); \
	and RT1, RMASK, b, lsr#(8 - 2 + (adj_b)); \
	ror_b(b); \
	and RT2, RMASK, a, lsl#(2); \
	and RT0, RMASK, a, lsr#(8 - 2); \
	\
	ldr RY, [CTXs1, xRT3]; \
	ldr RX, [CTXs0, xRT2]; \
	and RT3, RMASK, b, lsr#(16 - 2); \
	ldr RT1, [CTXs2, xRT1]; \
	and RT2, RMASK, a, lsr#(16 - 2); \
	ldr RT0, [CTXs1, xRT0]; \
	\
	ldr RT3, [CTXs3, xRT3]; \
	eor RY, RY, RT1; \
	\
	and RT1, RMASK, b, lsr#(24 - 2); \
	eor RX, RX, RT0; \
	ldr RT2, [CTXs2, xRT2]; \
	and RT0, RMASK, a, lsr#(24 - 2); \
	\
	ldr RT1, [CTXs0, xRT1]; \
	\
	eor RY, RY, RT3; \
	ldr RT0, [CTXs3, xRT0]; \
	eor RX, RX, RT2; \
	eor RY, RY, RT1; \
	\
	ldr RT1, [CTXs3, #(k - s3 + 8 * (n) + 4)]; \
	eor RX, RX, RT0; \
	ldr RT2, [CTXs3, #(k - s3 + 8 * (n))]; \
	\
	add RT0, RX, RY, lsl #1; \
	add RX, RX, RY; \
	add RT0, RT0, RT1; \
	add RX, RX, RT2; \
	eor rd, rd, RT0; \
	eor rc, RX, rc, ror #31;

#define first_encrypt_cycle(nc) \
	encrypt_round(RA, RB, RC, RD, (nc) * 2, dummy, 0); \
	encrypt_round(RC, RD, RA, RB, (nc) * 2 + 1, ror1, 1);

#define encrypt_cycle(nc) \
	encrypt_round(RA, RB, RC, RD, (nc) * 2, ror1, 1); \
	encrypt_round(RC, RD, RA, RB, (nc) * 2 + 1, ror1, 1);

#define last_encrypt_cycle(nc) \
	encrypt_round(RA, RB, RC, RD, (nc) * 2, ror1, 1); \
	encrypt_round(RC, RD, RA, RB, (nc) * 2 + 1, ror1, 1); \
	ror1(RA);

#define first_decrypt_cycle(nc) \
	decrypt_round(RC, RD, RA, RB, (nc) * 2 + 1, dummy, 0); \
	decrypt_round(RA, RB, RC, RD, (nc) * 2, ror1, 1);

#define decrypt_cycle(nc) \
	decrypt_round(RC, RD, RA, RB, (nc) * 2 + 1, ror1, 1); \
	decrypt_round(RA, RB, RC, RD, (nc) * 2, ror1, 1);

#define last_decrypt_cycle(nc) \
	decrypt_round(RC, RD, RA, RB, (nc) * 2 + 1, ror1, 1); \
	decrypt_round(RA, RB, RC, RD, (nc) * 2, ror1, 1); \
	ror1(RD);

.globl _gcry_twofish_arm_encrypt_block
ELF(.type   _gcry_twofish_arm_encrypt_block,%function;)

_gcry_twofish_arm_encrypt_block:
	/* input:
	 *	x0: ctx
	 *	x1: dst
	 *	x2: src
	 */
	CFI_STARTPROC();

	add CTXw, CTX, #(w);

	ldr_input_le(RSRC, RA, RB, RC, RD, RT0);

	/* Input whitening */
	ldp RT0, RT1, [CTXw, #(0*8)];
	ldp RT2, RT3, [CTXw, #(1*8)];
	add CTXs3, CTX, #(s3);
	add CTXs2, CTX, #(s2);
	add CTXs1, CTX, #(s1);
	mov RMASK, #(0xff << 2);
	eor RA, RA, RT0;
	eor RB, RB, RT1;
	eor RC, RC, RT2;
	eor RD, RD, RT3;

	first_encrypt_cycle(0);
	encrypt_cycle(1);
	encrypt_cycle(2);
	encrypt_cycle(3);
	encrypt_cycle(4);
	encrypt_cycle(5);
	encrypt_cycle(6);
	last_encrypt_cycle(7);

	/* Output whitening */
	ldp RT0, RT1, [CTXw, #(2*8)];
	ldp RT2, RT3, [CTXw, #(3*8)];
	eor RC, RC, RT0;
	eor RD, RD, RT1;
	eor RA, RA, RT2;
	eor RB, RB, RT3;

	str_output_le(RDST, RC, RD, RA, RB, RT0, RT1);

	ret_spec_stop;
	CFI_ENDPROC();
.ltorg
ELF(.size _gcry_twofish_arm_encrypt_block,.-_gcry_twofish_arm_encrypt_block;)

.globl _gcry_twofish_arm_decrypt_block
ELF(.type   _gcry_twofish_arm_decrypt_block,%function;)

_gcry_twofish_arm_decrypt_block:
	/* input:
	 *	%r0: ctx
	 *	%r1: dst
	 *	%r2: src
	 */
	CFI_STARTPROC();

	add CTXw, CTX, #(w);

	ldr_input_le(RSRC, RC, RD, RA, RB, RT0);

	/* Input whitening */
	ldp RT0, RT1, [CTXw, #(2*8)];
	ldp RT2, RT3, [CTXw, #(3*8)];
	add CTXs3, CTX, #(s3);
	add CTXs2, CTX, #(s2);
	add CTXs1, CTX, #(s1);
	mov RMASK, #(0xff << 2);
	eor RC, RC, RT0;
	eor RD, RD, RT1;
	eor RA, RA, RT2;
	eor RB, RB, RT3;

	first_decrypt_cycle(7);
	decrypt_cycle(6);
	decrypt_cycle(5);
	decrypt_cycle(4);
	decrypt_cycle(3);
	decrypt_cycle(2);
	decrypt_cycle(1);
	last_decrypt_cycle(0);

	/* Output whitening */
	ldp RT0, RT1, [CTXw, #(0*8)];
	ldp RT2, RT3, [CTXw, #(1*8)];
	eor RA, RA, RT0;
	eor RB, RB, RT1;
	eor RC, RC, RT2;
	eor RD, RD, RT3;

	str_output_le(RDST, RA, RB, RC, RD, RT0, RT1);

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_twofish_arm_decrypt_block,.-_gcry_twofish_arm_decrypt_block;)

#endif /*HAVE_COMPATIBLE_GCC_AARCH64_PLATFORM_AS*/
#endif /*__AARCH64EL__*/
