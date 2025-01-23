/* twofish-arm.S  -  ARM assembly implementation of Twofish cipher
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

/* structure of TWOFISH_context: */
#define s0 0
#define s1 ((s0) + 4 * 256)
#define s2 ((s1) + 4 * 256)
#define s3 ((s2) + 4 * 256)
#define w  ((s3) + 4 * 256)
#define k  ((w) + 4 * 8)

/* register macros */
#define CTX %r0
#define CTXs0 %r0
#define CTXs1 %r1
#define CTXs3 %r7

#define RA %r3
#define RB %r4
#define RC %r5
#define RD %r6

#define RX %r2
#define RY %ip

#define RMASK %lr

#define RT0 %r8
#define RT1 %r9
#define RT2 %r10
#define RT3 %r11

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

#ifndef __ARMEL__
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

#ifdef __ARM_FEATURE_UNALIGNED
	/* unaligned word reads/writes allowed */
	#define ldr_input_le(rin, ra, rb, rc, rd, rtmp) \
		ldr_input_aligned_le(rin, ra, rb, rc, rd)

	#define str_output_le(rout, ra, rb, rc, rd, rtmp0, rtmp1) \
		str_output_aligned_le(rout, ra, rb, rc, rd)
#else
	/* need to handle unaligned reads/writes by byte reads */
	#define ldr_input_le(rin, ra, rb, rc, rd, rtmp0) \
		tst rin, #3; \
		beq 1f; \
			ldr_unaligned_le(ra, rin, 0, rtmp0); \
			ldr_unaligned_le(rb, rin, 4, rtmp0); \
			ldr_unaligned_le(rc, rin, 8, rtmp0); \
			ldr_unaligned_le(rd, rin, 12, rtmp0); \
			b 2f; \
		1:;\
			ldr_input_aligned_le(rin, ra, rb, rc, rd); \
		2:;

	#define str_output_le(rout, ra, rb, rc, rd, rtmp0, rtmp1) \
		tst rout, #3; \
		beq 1f; \
			str_unaligned_le(ra, rout, 0, rtmp0, rtmp1); \
			str_unaligned_le(rb, rout, 4, rtmp0, rtmp1); \
			str_unaligned_le(rc, rout, 8, rtmp0, rtmp1); \
			str_unaligned_le(rd, rout, 12, rtmp0, rtmp1); \
			b 2f; \
		1:;\
			str_output_aligned_le(rout, ra, rb, rc, rd); \
		2:;
#endif

/**********************************************************************
  1-way twofish
 **********************************************************************/
#define encrypt_round(a, b, rc, rd, n, ror_a, adj_a) \
	and RT0, RMASK, b, lsr#(8 - 2); \
	and RY, RMASK, b, lsr#(16 - 2); \
	add RT0, RT0, #(s2 - s1); \
	and RT1, RMASK, b, lsr#(24 - 2); \
	ldr RY, [CTXs3, RY]; \
	and RT2, RMASK, b, lsl#(2); \
	ldr RT0, [CTXs1, RT0]; \
	and RT3, RMASK, a, lsr#(16 - 2 + (adj_a)); \
	ldr RT1, [CTXs0, RT1]; \
	and RX, RMASK, a, lsr#(8 - 2 + (adj_a)); \
	ldr RT2, [CTXs1, RT2]; \
	add RT3, RT3, #(s2 - s1); \
	ldr RX, [CTXs1, RX]; \
	ror_a(a); \
	\
	eor RY, RY, RT0; \
	ldr RT3, [CTXs1, RT3]; \
	and RT0, RMASK, a, lsl#(2); \
	eor RY, RY, RT1; \
	and RT1, RMASK, a, lsr#(24 - 2); \
	eor RY, RY, RT2; \
	ldr RT0, [CTXs0, RT0]; \
	eor RX, RX, RT3; \
	ldr RT1, [CTXs3, RT1]; \
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
	ldr RY, [CTXs1, RT3]; \
	add RT1, RT1, #(s2 - s1); \
	ldr RX, [CTXs0, RT2]; \
	and RT3, RMASK, b, lsr#(16 - 2); \
	ldr RT1, [CTXs1, RT1]; \
	and RT2, RMASK, a, lsr#(16 - 2); \
	ldr RT0, [CTXs1, RT0]; \
	\
	add RT2, RT2, #(s2 - s1); \
	ldr RT3, [CTXs3, RT3]; \
	eor RY, RY, RT1; \
	\
	and RT1, RMASK, b, lsr#(24 - 2); \
	eor RX, RX, RT0; \
	ldr RT2, [CTXs1, RT2]; \
	and RT0, RMASK, a, lsr#(24 - 2); \
	\
	ldr RT1, [CTXs0, RT1]; \
	\
	eor RY, RY, RT3; \
	ldr RT0, [CTXs3, RT0]; \
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

.align 3
.globl _gcry_twofish_arm_encrypt_block
.type   _gcry_twofish_arm_encrypt_block,%function;

_gcry_twofish_arm_encrypt_block:
	/* input:
	 *	%r0: ctx
	 *	%r1: dst
	 *	%r2: src
	 */
	push {%r1, %r4-%r11, %ip, %lr};

	add RY, CTXs0, #w;

	ldr_input_le(%r2, RA, RB, RC, RD, RT0);

	/* Input whitening */
	ldm RY, {RT0, RT1, RT2, RT3};
	add CTXs3, CTXs0, #(s3 - s0);
	add CTXs1, CTXs0, #(s1 - s0);
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

	add RY, CTXs3, #(w + 4*4 - s3);
	pop {%r1}; /* dst */

	/* Output whitening */
	ldm RY, {RT0, RT1, RT2, RT3};
	eor RC, RC, RT0;
	eor RD, RD, RT1;
	eor RA, RA, RT2;
	eor RB, RB, RT3;

	str_output_le(%r1, RC, RD, RA, RB, RT0, RT1);

	pop {%r4-%r11, %ip, %pc};
.ltorg
.size _gcry_twofish_arm_encrypt_block,.-_gcry_twofish_arm_encrypt_block;

.align 3
.globl _gcry_twofish_arm_decrypt_block
.type   _gcry_twofish_arm_decrypt_block,%function;

_gcry_twofish_arm_decrypt_block:
	/* input:
	 *	%r0: ctx
	 *	%r1: dst
	 *	%r2: src
	 */
	push {%r1, %r4-%r11, %ip, %lr};

	add CTXs3, CTXs0, #(s3 - s0);

	ldr_input_le(%r2, RC, RD, RA, RB, RT0);

	add RY, CTXs3, #(w + 4*4 - s3);
	add CTXs3, CTXs0, #(s3 - s0);

	/* Input whitening */
	ldm RY, {RT0, RT1, RT2, RT3};
	add CTXs1, CTXs0, #(s1 - s0);
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

	add RY, CTXs0, #w;
	pop {%r1}; /* dst */

	/* Output whitening */
	ldm RY, {RT0, RT1, RT2, RT3};
	eor RA, RA, RT0;
	eor RB, RB, RT1;
	eor RC, RC, RT2;
	eor RD, RD, RT3;

	str_output_le(%r1, RA, RB, RC, RD, RT0, RT1);

	pop {%r4-%r11, %ip, %pc};
.size _gcry_twofish_arm_decrypt_block,.-_gcry_twofish_arm_decrypt_block;

#endif /*HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS*/
#endif /*__ARMEL__*/
