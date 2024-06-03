// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 1991,1992,1993,1997,1998,2003, 2005 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/* From glibc-2.14, sysdeps/i386/memset.c */

#include <linux/types.h>
#include <linux/compiler.h>
#include <asm/string.h>

typedef uint32_t op_t;

void *memset(void *dstpp, int c, size_t len)
{
	int d0;
	unsigned long int dstp = (unsigned long int) dstpp;

	/* This explicit register allocation improves code very much indeed. */
	register op_t x asm("ax");

	x = (unsigned char) c;

	/* Clear the direction flag, so filling will move forward.  */
	asm volatile("cld");

	/* This threshold value is optimal.  */
	if (len >= 12) {
		/* Fill X with four copies of the char we want to fill with. */
		x |= (x << 8);
		x |= (x << 16);

		/* Adjust LEN for the bytes handled in the first loop.  */
		len -= (-dstp) % sizeof(op_t);

		/*
		 * There are at least some bytes to set. No need to test for
		 * LEN == 0 in this alignment loop.
		 */

		/* Fill bytes until DSTP is aligned on a longword boundary. */
		asm volatile(
			"rep\n"
			"stosb" /* %0, %2, %3 */ :
			"=D" (dstp), "=c" (d0) :
			"0" (dstp), "1" ((-dstp) % sizeof(op_t)), "a" (x) :
			"memory");

		/* Fill longwords.  */
		asm volatile(
			"rep\n"
			"stosl" /* %0, %2, %3 */ :
			"=D" (dstp), "=c" (d0) :
			"0" (dstp), "1" (len / sizeof(op_t)), "a" (x) :
			"memory");
		len %= sizeof(op_t);
	}

	/* Write the last few bytes. */
	asm volatile(
		"rep\n"
		"stosb" /* %0, %2, %3 */ :
		"=D" (dstp), "=c" (d0) :
		"0" (dstp), "1" (len), "a" (x) :
		"memory");

	return dstpp;
}

#define	OP_T_THRES	8
#define OPSIZ	(sizeof(op_t))

#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes)				  \
do {									  \
	int __d0;							  \
	asm volatile(							  \
		/* Clear the direction flag, so copying goes forward.  */ \
		"cld\n"							  \
		/* Copy bytes.  */					  \
		"rep\n"							  \
		"movsb" :						  \
		"=D" (dst_bp), "=S" (src_bp), "=c" (__d0) :		  \
		"0" (dst_bp), "1" (src_bp), "2" (nbytes) :		  \
		"memory");						  \
} while (0)

#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		  \
do {									  \
	int __d0;							  \
	asm volatile(							  \
		/* Clear the direction flag, so copying goes forward.  */ \
		"cld\n"							  \
		/* Copy longwords.  */					  \
		"rep\n"							  \
		"movsl" :						  \
		"=D" (dst_bp), "=S" (src_bp), "=c" (__d0) :		  \
		"0" (dst_bp), "1" (src_bp), "2" ((nbytes) / 4) :	  \
		"memory");						  \
	(nbytes_left) = (nbytes) % 4;					  \
} while (0)

void *memcpy(void *dstpp, const void *srcpp, size_t len)
{
	unsigned long int dstp = (long int)dstpp;
	unsigned long int srcp = (long int)srcpp;

	/* Copy from the beginning to the end.  */

	/* If there not too few bytes to copy, use word copy.  */
	if (len >= OP_T_THRES) {
		/* Copy just a few bytes to make DSTP aligned.  */
		len -= (-dstp) % OPSIZ;
		BYTE_COPY_FWD(dstp, srcp, (-dstp) % OPSIZ);

		/* Copy from SRCP to DSTP taking advantage of the known
		 * alignment of DSTP.  Number of bytes remaining is put
		 * in the third argument, i.e. in LEN.  This number may
		 * vary from machine to machine.
		 */
		WORD_COPY_FWD(dstp, srcp, len, len);

		/* Fall out and copy the tail.  */
	}

	/* There are just a few bytes to copy. Use byte memory operations. */
	BYTE_COPY_FWD(dstp, srcp, len);

	return dstpp;
}

void *memmove(void *dest, const void *src, size_t n)
{
	int d0, d1, d2, d3, d4, d5;
	char *ret = dest;

	__asm__ __volatile__(
		/* Handle more 16 bytes in loop */
		"cmp $0x10, %0\n\t"
		"jb	1f\n\t"

		/* Decide forward/backward copy mode */
		"cmp %2, %1\n\t"
		"jb	2f\n\t"

		/*
		 * movs instruction have many startup latency
		 * so we handle small size by general register.
		 */
		"cmp  $680, %0\n\t"
		"jb 3f\n\t"
		/* movs instruction is only good for aligned case */
		"mov %1, %3\n\t"
		"xor %2, %3\n\t"
		"and $0xff, %3\n\t"
		"jz 4f\n\t"
		"3:\n\t"
		"sub $0x10, %0\n\t"

		/* We gobble 16 bytes forward in each loop */
		"3:\n\t"
		"sub $0x10, %0\n\t"
		"mov 0*4(%1), %3\n\t"
		"mov 1*4(%1), %4\n\t"
		"mov  %3, 0*4(%2)\n\t"
		"mov  %4, 1*4(%2)\n\t"
		"mov 2*4(%1), %3\n\t"
		"mov 3*4(%1), %4\n\t"
		"mov  %3, 2*4(%2)\n\t"
		"mov  %4, 3*4(%2)\n\t"
		"lea  0x10(%1), %1\n\t"
		"lea  0x10(%2), %2\n\t"
		"jae 3b\n\t"
		"add $0x10, %0\n\t"
		"jmp 1f\n\t"

		/* Handle data forward by movs */
		".p2align 4\n\t"
		"4:\n\t"
		"mov -4(%1, %0), %3\n\t"
		"lea -4(%2, %0), %4\n\t"
		"shr $2, %0\n\t"
		"rep movsl\n\t"
		"mov %3, (%4)\n\t"
		"jmp 11f\n\t"
		/* Handle data backward by movs */
		".p2align 4\n\t"
		"6:\n\t"
		"mov (%1), %3\n\t"
		"mov %2, %4\n\t"
		"lea -4(%1, %0), %1\n\t"
		"lea -4(%2, %0), %2\n\t"
		"shr $2, %0\n\t"
		"std\n\t"
		"rep movsl\n\t"
		"mov %3,(%4)\n\t"
		"cld\n\t"
		"jmp 11f\n\t"

		/* Start to prepare for backward copy */
		".p2align 4\n\t"
		"2:\n\t"
		"cmp  $680, %0\n\t"
		"jb 5f\n\t"
		"mov %1, %3\n\t"
		"xor %2, %3\n\t"
		"and $0xff, %3\n\t"
		"jz 6b\n\t"

		/* Calculate copy position to tail */
		"5:\n\t"
		"add %0, %1\n\t"
		"add %0, %2\n\t"
		"sub $0x10, %0\n\t"

		/* We gobble 16 bytes backward in each loop */
		"7:\n\t"
		"sub $0x10, %0\n\t"

		"mov -1*4(%1), %3\n\t"
		"mov -2*4(%1), %4\n\t"
		"mov  %3, -1*4(%2)\n\t"
		"mov  %4, -2*4(%2)\n\t"
		"mov -3*4(%1), %3\n\t"
		"mov -4*4(%1), %4\n\t"
		"mov  %3, -3*4(%2)\n\t"
		"mov  %4, -4*4(%2)\n\t"
		"lea  -0x10(%1), %1\n\t"
		"lea  -0x10(%2), %2\n\t"
		"jae 7b\n\t"
		/* Calculate copy position to head */
		"add $0x10, %0\n\t"
		"sub %0, %1\n\t"
		"sub %0, %2\n\t"

		/* Move data from 8 bytes to 15 bytes */
		".p2align 4\n\t"
		"1:\n\t"
		"cmp $8, %0\n\t"
		"jb 8f\n\t"
		"mov 0*4(%1), %3\n\t"
		"mov 1*4(%1), %4\n\t"
		"mov -2*4(%1, %0), %5\n\t"
		"mov -1*4(%1, %0), %1\n\t"

		"mov  %3, 0*4(%2)\n\t"
		"mov  %4, 1*4(%2)\n\t"
		"mov  %5, -2*4(%2, %0)\n\t"
		"mov  %1, -1*4(%2, %0)\n\t"
		"jmp 11f\n\t"

		/* Move data from 4 bytes to 7 bytes */
		".p2align 4\n\t"
		"8:\n\t"
		"cmp $4, %0\n\t"
		"jb 9f\n\t"
		"mov 0*4(%1), %3\n\t"
		"mov -1*4(%1, %0), %4\n\t"
		"mov  %3, 0*4(%2)\n\t"
		"mov  %4, -1*4(%2, %0)\n\t"
		"jmp 11f\n\t"

		/* Move data from 2 bytes to 3 bytes */
		".p2align 4\n\t"
		"9:\n\t"
		"cmp $2, %0\n\t"
		"jb 10f\n\t"
		"movw 0*2(%1), %%dx\n\t"
		"movw -1*2(%1, %0), %%bx\n\t"
		"movw %%dx, 0*2(%2)\n\t"
		"movw %%bx, -1*2(%2, %0)\n\t"
		"jmp 11f\n\t"

		/* Move data for 1 byte */
		".p2align 4\n\t"
		"10:\n\t"
		"cmp $1, %0\n\t"
		"jb 11f\n\t"
		"movb (%1), %%cl\n\t"
		"movb %%cl, (%2)\n\t"
		".p2align 4\n\t"
		"11:"
		: "=&c" (d0), "=&S" (d1), "=&D" (d2),
		  "=r" (d3), "=r" (d4), "=r"(d5)
		: "0" (n),
		 "1" (src),
		 "2" (dest)
		: "memory");

	return ret;
}
