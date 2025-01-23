/* sha512-arm.S  -  ARM assembly implementation of SHA-512 transform
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
#include <config.h>

#if defined(__ARMEL__)
#ifdef HAVE_COMPATIBLE_GCC_ARM_PLATFORM_AS

.text

.syntax unified
.arm

/* structure of SHA512_CONTEXT */
#define hd_a 0
#define hd_b ((hd_a) + 8)
#define hd_c ((hd_b) + 8)
#define hd_d ((hd_c) + 8)
#define hd_e ((hd_d) + 8)
#define hd_f ((hd_e) + 8)
#define hd_g ((hd_f) + 8)
#define hd_h ((hd_g) + 8)

/* register macros */
#define RK    %r2

#define RElo %r0
#define REhi %r1

#define RT1lo %r3
#define RT1hi %r4
#define RT2lo %r5
#define RT2hi %r6
#define RWlo  %r7
#define RWhi  %r8
#define RT3lo %r9
#define RT3hi %r10
#define RT4lo %r11
#define RT4hi %ip

#define RRND  %lr

/* variable offsets in stack */
#define ctx (0)
#define data ((ctx) + 4)
#define nblks ((data) + 4)
#define _a ((nblks) + 4)
#define _b ((_a) + 8)
#define _c ((_b) + 8)
#define _d ((_c) + 8)
#define _e ((_d) + 8)
#define _f ((_e) + 8)
#define _g ((_f) + 8)
#define _h ((_g) + 8)

#define w(i) ((_h) + 8 + ((i) % 16) * 8)

#define STACK_MAX (w(15) + 8)

/* helper macros */
#define ldr_unaligned_be(rout, rsrc, offs, rtmp) \
    ldrb rout, [rsrc, #((offs) + 3)]; \
    ldrb rtmp, [rsrc, #((offs) + 2)]; \
    orr rout, rout, rtmp, lsl #8; \
    ldrb rtmp, [rsrc, #((offs) + 1)]; \
    orr rout, rout, rtmp, lsl #16; \
    ldrb rtmp, [rsrc, #((offs) + 0)]; \
    orr rout, rout, rtmp, lsl #24;

#ifdef __ARMEL__
    /* bswap on little-endian */
#ifdef HAVE_ARM_ARCH_V6
    #define be_to_host(reg, rtmp) \
	rev reg, reg;
#else
    #define be_to_host(reg, rtmp) \
	eor rtmp, reg, reg, ror #16; \
	mov rtmp, rtmp, lsr #8; \
	bic rtmp, rtmp, #65280; \
	eor reg, rtmp, reg, ror #8;
#endif
#else
    /* nop on big-endian */
    #define be_to_host(reg, rtmp) /*_*/
#endif

#define host_to_host(x, y) /*_*/

#define read_u64_aligned_4(rin, offs, lo0, hi0, lo1, hi1, lo2, hi2, lo3, hi3, convert, rtmp) \
    ldr lo0, [rin, #((offs) + 0 * 8 + 4)]; \
    ldr hi0, [rin, #((offs) + 0 * 8 + 0)]; \
    ldr lo1, [rin, #((offs) + 1 * 8 + 4)]; \
    ldr hi1, [rin, #((offs) + 1 * 8 + 0)]; \
    ldr lo2, [rin, #((offs) + 2 * 8 + 4)]; \
    convert(lo0, rtmp); \
    ldr hi2, [rin, #((offs) + 2 * 8 + 0)]; \
    convert(hi0, rtmp); \
    ldr lo3, [rin, #((offs) + 3 * 8 + 4)]; \
    convert(lo1, rtmp); \
    ldr hi3, [rin, #((offs) + 3 * 8 + 0)]; \
    convert(hi1, rtmp); \
    convert(lo2, rtmp); \
    convert(hi2, rtmp); \
    convert(lo3, rtmp); \
    convert(hi3, rtmp);

#define read_be64_aligned_4(rin, offs, lo0, hi0, lo1, hi1, lo2, hi2, lo3, hi3, rtmp0) \
    read_u64_aligned_4(rin, offs, lo0, hi0, lo1, hi1, lo2, hi2, lo3, hi3, be_to_host, rtmp0)

/* need to handle unaligned reads by byte reads */
#define read_be64_unaligned_4(rin, offs, lo0, hi0, lo1, hi1, lo2, hi2, lo3, hi3, rtmp0) \
    ldr_unaligned_be(lo0, rin, (offs) + 0 * 8 + 4, rtmp0); \
    ldr_unaligned_be(hi0, rin, (offs) + 0 * 8 + 0, rtmp0); \
    ldr_unaligned_be(lo1, rin, (offs) + 1 * 8 + 4, rtmp0); \
    ldr_unaligned_be(hi1, rin, (offs) + 1 * 8 + 0, rtmp0); \
    ldr_unaligned_be(lo2, rin, (offs) + 2 * 8 + 4, rtmp0); \
    ldr_unaligned_be(hi2, rin, (offs) + 2 * 8 + 0, rtmp0); \
    ldr_unaligned_be(lo3, rin, (offs) + 3 * 8 + 4, rtmp0); \
    ldr_unaligned_be(hi3, rin, (offs) + 3 * 8 + 0, rtmp0);

/***********************************************************************
 * ARM assembly implementation of sha512 transform
 ***********************************************************************/

/* Round function */

#define R(_a,_b,_c,_d,_e,_f,_g,_h,W,wi) \
    /* Message expansion, t1 = _h + w[i] */ \
    W(_a,_h,wi); \
    \
    /* w = Sum1(_e) */ \
    mov RWlo, RElo, lsr#14; \
    ldm RK!, {RT2lo-RT2hi}; \
    mov RWhi, REhi, lsr#14; \
    eor RWlo, RWlo, RElo, lsr#18; \
    eor RWhi, RWhi, REhi, lsr#18; \
    ldr RT3lo, [%sp, #(_f)]; \
    adds RT1lo, RT2lo; /* t1 += K */ \
    ldr RT3hi, [%sp, #(_f) + 4]; \
    adc RT1hi, RT2hi; \
    ldr RT4lo, [%sp, #(_g)]; \
    eor RWlo, RWlo, RElo, lsl#23; \
    ldr RT4hi, [%sp, #(_g) + 4]; \
    eor RWhi, RWhi, REhi, lsl#23; \
    eor RWlo, RWlo, REhi, lsl#18; \
    eor RWhi, RWhi, RElo, lsl#18; \
    eor RWlo, RWlo, REhi, lsl#14; \
    eor RWhi, RWhi, RElo, lsl#14; \
    eor RWlo, RWlo, REhi, lsr#9; \
    eor RWhi, RWhi, RElo, lsr#9; \
    \
    /* Cho(_e,_f,_g) => (_e & _f) ^ (~_e & _g) */ \
    adds RT1lo, RWlo; /* t1 += Sum1(_e) */ \
    and RT3lo, RT3lo, RElo; \
    adc RT1hi, RWhi; \
    and RT3hi, RT3hi, REhi; \
    bic RT4lo, RT4lo, RElo; \
    bic RT4hi, RT4hi, REhi; \
    eor RT3lo, RT3lo, RT4lo; \
    eor RT3hi, RT3hi, RT4hi; \
    \
    /* Load D */ \
    /* t1 += Cho(_e,_f,_g) */ \
    ldr RElo, [%sp, #(_d)]; \
    adds RT1lo, RT3lo; \
    ldr REhi, [%sp, #(_d) + 4]; \
    adc RT1hi, RT3hi; \
    \
    /* Load A */ \
    ldr RT3lo, [%sp, #(_a)]; \
    \
    /* _d += t1 */ \
    adds RElo, RT1lo; \
    ldr RT3hi, [%sp, #(_a) + 4]; \
    adc REhi, RT1hi; \
    \
    /* Store D */ \
    str RElo, [%sp, #(_d)]; \
    \
    /* t2 = Sum0(_a) */ \
    mov RT2lo, RT3lo, lsr#28; \
    str REhi, [%sp, #(_d) + 4]; \
    mov RT2hi, RT3hi, lsr#28; \
    ldr RWlo, [%sp, #(_b)]; \
    eor RT2lo, RT2lo, RT3lo, lsl#30; \
    ldr RWhi, [%sp, #(_b) + 4]; \
    eor RT2hi, RT2hi, RT3hi, lsl#30; \
    eor RT2lo, RT2lo, RT3lo, lsl#25; \
    eor RT2hi, RT2hi, RT3hi, lsl#25; \
    eor RT2lo, RT2lo, RT3hi, lsl#4; \
    eor RT2hi, RT2hi, RT3lo, lsl#4; \
    eor RT2lo, RT2lo, RT3hi, lsr#2; \
    eor RT2hi, RT2hi, RT3lo, lsr#2; \
    eor RT2lo, RT2lo, RT3hi, lsr#7; \
    eor RT2hi, RT2hi, RT3lo, lsr#7; \
    \
    /* t2 += t1 */ \
    adds RT2lo, RT1lo; \
    ldr RT1lo, [%sp, #(_c)]; \
    adc RT2hi, RT1hi; \
    \
    /* Maj(_a,_b,_c) => ((_a & _b) ^ (_c & (_a ^ _b))) */ \
    ldr RT1hi, [%sp, #(_c) + 4]; \
    and RT4lo, RWlo, RT3lo; \
    and RT4hi, RWhi, RT3hi; \
    eor RWlo, RWlo, RT3lo; \
    eor RWhi, RWhi, RT3hi; \
    and RWlo, RWlo, RT1lo; \
    and RWhi, RWhi, RT1hi; \
    eor RWlo, RWlo, RT4lo; \
    eor RWhi, RWhi, RT4hi; \

/* Message expansion */

#define W_0_63(_a,_h,i) \
    ldr RT3lo, [%sp, #(w(i-2))]; \
    adds RT2lo, RWlo; /* _h = t2 + Maj(_a,_b,_c) */ \
    ldr RT3hi, [%sp, #(w(i-2)) + 4]; \
    adc RT2hi, RWhi; \
    /* nw = S1(w[i-2]) */ \
    ldr RT1lo, [%sp, #(_h)]; /* Load H */ \
    mov RWlo, RT3lo, lsr#19; \
    str RT2lo, [%sp, #(_a)]; \
    eor RWlo, RWlo, RT3lo, lsl#3; \
    ldr RT1hi, [%sp, #(_h) + 4]; \
    mov RWhi, RT3hi, lsr#19; \
    ldr RT2lo, [%sp, #(w(i-7))]; \
    eor RWhi, RWhi, RT3hi, lsl#3; \
    str RT2hi, [%sp, #(_a) + 4]; \
    eor RWlo, RWlo, RT3lo, lsr#6; \
    ldr RT2hi, [%sp, #(w(i-7)) + 4]; \
    eor RWhi, RWhi, RT3hi, lsr#6; \
    eor RWlo, RWlo, RT3hi, lsl#13; \
    eor RWhi, RWhi, RT3lo, lsl#13; \
    eor RWlo, RWlo, RT3hi, lsr#29; \
    eor RWhi, RWhi, RT3lo, lsr#29; \
    ldr RT3lo, [%sp, #(w(i-15))]; \
    eor RWlo, RWlo, RT3hi, lsl#26; \
    ldr RT3hi, [%sp, #(w(i-15)) + 4]; \
    \
    adds RT2lo, RWlo; /* nw += w[i-7] */ \
    ldr RWlo, [%sp, #(w(i-16))]; \
    adc RT2hi, RWhi; \
    mov RT4lo, RT3lo, lsr#1; /* S0(w[i-15]) */ \
    ldr RWhi, [%sp, #(w(i-16)) + 4]; \
    mov RT4hi, RT3hi, lsr#1; \
    adds RT2lo, RWlo; /* nw += w[i-16] */ \
    eor RT4lo, RT4lo, RT3lo, lsr#8; \
    eor RT4hi, RT4hi, RT3hi, lsr#8; \
    eor RT4lo, RT4lo, RT3lo, lsr#7; \
    eor RT4hi, RT4hi, RT3hi, lsr#7; \
    eor RT4lo, RT4lo, RT3hi, lsl#31; \
    eor RT4hi, RT4hi, RT3lo, lsl#31; \
    eor RT4lo, RT4lo, RT3hi, lsl#24; \
    eor RT4hi, RT4hi, RT3lo, lsl#24; \
    eor RT4lo, RT4lo, RT3hi, lsl#25; \
    adc RT2hi, RWhi; \
    \
    /* nw += S0(w[i-15]) */ \
    adds RT2lo, RT4lo; \
    adc RT2hi, RT4hi; \
    \
    /* w[0] = nw */ \
    str RT2lo, [%sp, #(w(i))]; \
    adds RT1lo, RWlo; \
    str RT2hi, [%sp, #(w(i)) + 4]; \
    adc RT1hi, RWhi;

#define W_64_79(_a,_h,i) \
    adds RT2lo, RWlo; /* _h = t2 + Maj(_a,_b,_c) */ \
    ldr RWlo, [%sp, #(w(i-16))]; \
    adc RT2hi, RWhi; \
    ldr RWhi, [%sp, #(w(i-16)) + 4]; \
    ldr RT1lo, [%sp, #(_h)]; /* Load H */ \
    ldr RT1hi, [%sp, #(_h) + 4]; \
    str RT2lo, [%sp, #(_a)]; \
    str RT2hi, [%sp, #(_a) + 4]; \
    adds RT1lo, RWlo; \
    adc RT1hi, RWhi;

.align 3
.globl _gcry_sha512_transform_arm
.type  _gcry_sha512_transform_arm,%function;

_gcry_sha512_transform_arm:
	/* Input:
	 *	%r0: SHA512_CONTEXT
	 *	%r1: data
	 *	%r2: u64 k[] constants
	 *	%r3: nblks
	 */
	push {%r4-%r11, %ip, %lr};
	sub %sp, %sp, #STACK_MAX;
	movs RWlo, %r3;
	str %r0, [%sp, #(ctx)];

	beq .Ldone;

.Loop_blocks:
	str RWlo, [%sp, #nblks];

	/* Load context to stack */
	add RWhi, %sp, #(_a);
	ldm %r0!,  {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}
	ldm %r0,  {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}
	stm RWhi, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	/* Load input to w[16] */

	/* test if data is unaligned */
	tst %r1, #3;
	beq 1f;

	/* unaligned load */
	add RWhi, %sp, #(w(0));
	read_be64_unaligned_4(%r1, 0 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	read_be64_unaligned_4(%r1, 4 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	read_be64_unaligned_4(%r1, 8 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	read_be64_unaligned_4(%r1, 12 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	b 2f;
1:
	/* aligned load */
	add RWhi, %sp, #(w(0));
	read_be64_aligned_4(%r1, 0 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	read_be64_aligned_4(%r1, 4 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	read_be64_aligned_4(%r1, 8 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
	stm RWhi!, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}

	read_be64_aligned_4(%r1, 12 * 8, RT1lo, RT1hi, RT2lo, RT2hi, RT3lo, RT3hi, RT4lo, RT4hi, RWlo);
2:
	add %r1, #(16 * 8);
	stm RWhi, {RT1lo,RT1hi,RT2lo,RT2hi,RT3lo,RT3hi,RT4lo,RT4hi}
	str %r1, [%sp, #(data)];

	/* preload E & A */
	ldr RElo, [%sp, #(_e)];
	ldr REhi, [%sp, #(_e) + 4];
	mov RWlo, #0;
	ldr RT2lo, [%sp, #(_a)];
	mov RRND, #(80-16);
	ldr RT2hi, [%sp, #(_a) + 4];
	mov RWhi, #0;

.Loop_rounds:
	R(_a, _b, _c, _d, _e, _f, _g, _h, W_0_63, 16);
	R(_h, _a, _b, _c, _d, _e, _f, _g, W_0_63, 17);
	R(_g, _h, _a, _b, _c, _d, _e, _f, W_0_63, 18);
	R(_f, _g, _h, _a, _b, _c, _d, _e, W_0_63, 19);
	R(_e, _f, _g, _h, _a, _b, _c, _d, W_0_63, 20);
	R(_d, _e, _f, _g, _h, _a, _b, _c, W_0_63, 21);
	R(_c, _d, _e, _f, _g, _h, _a, _b, W_0_63, 22);
	R(_b, _c, _d, _e, _f, _g, _h, _a, W_0_63, 23);
	R(_a, _b, _c, _d, _e, _f, _g, _h, W_0_63, 24);
	R(_h, _a, _b, _c, _d, _e, _f, _g, W_0_63, 25);
	R(_g, _h, _a, _b, _c, _d, _e, _f, W_0_63, 26);
	R(_f, _g, _h, _a, _b, _c, _d, _e, W_0_63, 27);
	R(_e, _f, _g, _h, _a, _b, _c, _d, W_0_63, 28);
	R(_d, _e, _f, _g, _h, _a, _b, _c, W_0_63, 29);
	R(_c, _d, _e, _f, _g, _h, _a, _b, W_0_63, 30);
	R(_b, _c, _d, _e, _f, _g, _h, _a, W_0_63, 31);

	subs RRND, #16;
	bne .Loop_rounds;

	R(_a, _b, _c, _d, _e, _f, _g, _h, W_64_79, 16);
	R(_h, _a, _b, _c, _d, _e, _f, _g, W_64_79, 17);
	R(_g, _h, _a, _b, _c, _d, _e, _f, W_64_79, 18);
	R(_f, _g, _h, _a, _b, _c, _d, _e, W_64_79, 19);
	R(_e, _f, _g, _h, _a, _b, _c, _d, W_64_79, 20);
	R(_d, _e, _f, _g, _h, _a, _b, _c, W_64_79, 21);
	R(_c, _d, _e, _f, _g, _h, _a, _b, W_64_79, 22);
	R(_b, _c, _d, _e, _f, _g, _h, _a, W_64_79, 23);
	R(_a, _b, _c, _d, _e, _f, _g, _h, W_64_79, 24);
	R(_h, _a, _b, _c, _d, _e, _f, _g, W_64_79, 25);
	R(_g, _h, _a, _b, _c, _d, _e, _f, W_64_79, 26);
	R(_f, _g, _h, _a, _b, _c, _d, _e, W_64_79, 27);
	R(_e, _f, _g, _h, _a, _b, _c, _d, W_64_79, 28);
	R(_d, _e, _f, _g, _h, _a, _b, _c, W_64_79, 29);
	R(_c, _d, _e, _f, _g, _h, _a, _b, W_64_79, 30);
	R(_b, _c, _d, _e, _f, _g, _h, _a, W_64_79, 31);

	ldr %r0, [%sp, #(ctx)];
	adds RT2lo, RWlo; /* _h = t2 + Maj(_a,_b,_c) */
	ldr %r1, [%sp, #(data)];
	adc RT2hi, RWhi;

	ldm %r0, {RT1lo,RT1hi,RWlo,RWhi,RT3lo,RT3hi,RT4lo,RT4hi}
	adds RT1lo, RT2lo;
	ldr RT2lo, [%sp, #(_b + 0)];
	adc  RT1hi, RT2hi;
	ldr RT2hi, [%sp, #(_b + 4)];
	adds RWlo, RT2lo;
	ldr RT2lo, [%sp, #(_c + 0)];
	adc  RWhi, RT2hi;
	ldr RT2hi, [%sp, #(_c + 4)];
	adds RT3lo, RT2lo;
	ldr RT2lo, [%sp, #(_d + 0)];
	adc  RT3hi, RT2hi;
	ldr RT2hi, [%sp, #(_d + 4)];
	adds RT4lo, RT2lo;
	ldr RT2lo, [%sp, #(_e + 0)];
	adc  RT4hi, RT2hi;
	stm %r0!, {RT1lo,RT1hi,RWlo,RWhi,RT3lo,RT3hi,RT4lo,RT4hi}

	ldr RT2hi, [%sp, #(_e + 4)];
	ldm %r0, {RT1lo,RT1hi,RWlo,RWhi,RT3lo,RT3hi,RT4lo,RT4hi}
	adds RT1lo, RT2lo;
	ldr RT2lo, [%sp, #(_f + 0)];
	adc  RT1hi, RT2hi;
	ldr RT2hi, [%sp, #(_f + 4)];
	adds RWlo, RT2lo;
	ldr RT2lo, [%sp, #(_g + 0)];
	adc  RWhi, RT2hi;
	ldr RT2hi, [%sp, #(_g + 4)];
	adds RT3lo, RT2lo;
	ldr RT2lo, [%sp, #(_h + 0)];
	adc  RT3hi, RT2hi;
	ldr RT2hi, [%sp, #(_h + 4)];
	adds RT4lo, RT2lo;
	adc  RT4hi, RT2hi;
	stm %r0, {RT1lo,RT1hi,RWlo,RWhi,RT3lo,RT3hi,RT4lo,RT4hi}
	sub %r0, %r0, #(4 * 8);
	ldr RWlo, [%sp, #nblks];

	sub RK, #(80 * 8);
	subs RWlo, #1;
	bne .Loop_blocks;

.Ldone:
	mov %r0, #STACK_MAX;
__out:
	add %sp, %sp, #STACK_MAX;
	pop {%r4-%r11, %ip, %pc};
.size _gcry_sha512_transform_arm,.-_gcry_sha512_transform_arm;

#endif
#endif
