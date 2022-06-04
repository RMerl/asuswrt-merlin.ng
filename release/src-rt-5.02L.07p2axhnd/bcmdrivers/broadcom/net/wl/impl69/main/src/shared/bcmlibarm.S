/*
 * RTE ARM libary functions
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 */

#ifndef	__arm__
#error __arm__ is NOT defined
#endif // endif

#include <arminc.h>

	.text

#ifdef	__thumb__
	.thumb
#endif // endif

	.syntax unified

/* Bzero utility API
 * Input
 *  - a1 - Buffer address
 *  - a2 - Length of the buffer
 * Return
 *  - void
 */
FUNC(rte_bzero)
	stmfd sp!, {r4, r5}

	mov r2,#0

	/* Check if input buffer is aligned on 4 byte boundary.
	 * If not, zero fill unaligned portion in the front of the buffer.
	 * Jump to store multiple loops on reaching required alignment
	 */
align:
	cbz r1, end				/* Zero filled requested number of bytes */
	and r3, r0, #3				/* Check first 2 bits                    */
	cbz r3, skip_align			/* Jump to store multiple if aligned     */
	strb r2, [r0, #0]			/* Zero fill one byte at a time till -   */
	add r0, #1				/* reaching required alignment           */
	sub r1, r1, #1
	b align

skip_align:
	mov r3,#0				/* Init 4 register for 16 byte store op  */
	mov r4,#0
	mov r5,#0

	cmp r1, #16				/* Store 16 bytes at a time if remaining */
	blt z8					/* length is >= 16                       */
z16:
	stmia   r0!,{r2,r3,r4,r5}
	sub r1, r1, #16
	cmp r1, #15				/* Store 16 bytes at a time if remaining */
	bgt z16

z8:
	cmp r1, #8				/* Store 8 bytes at a time if remaining  */
	blt z4					/* length is >= 8                        */
	stmia   r0!,{r2,r3}
	sub r1, r1, #8

z4:
	cmp r1, #4				/* Store 4 bytes at a time if remaining */
	blt z1					/* length is >= 4                       */
	stmia   r0!,{r2}
	sub r1, r1, #4

z1:
	cbz r1, end				/* Fill remaining bytes one at a time   */
	strb r2, [r0, #0]
	add r0, #1
	sub r1, r1, #1
	b z1

end:
	ldmfd sp!, {r4, r5}
	bx lr

END(rte_bzero)

/* mem compare to zero utility API
 * Input
 *  - a1 - Buffer address
 *  - a2 - Length of the buffer
 * Return
 *  - zero on success, non zero if any byte contain non zero values
 */
FUNC(rte_bzerocmp)
	stmfd sp!, {r4, r5, r6, lr}

	mov r6,#0				/* R6 holds the final ORd value          */

#ifdef ZCMP_UNALIGNED_SUPPORT
	mov r2,#0

	/* Check if input buffer is aligned on 4 byte boundary.
	 * If not, compare unaligned portion in the front of the buffer.
	 * Jump to load multiple loops on reaching required alignment
	 */
zc_align:
	cbz r1, zc_end				/* Compared requested number of bytes    */
	and r3, r0, #3				/* Check for four byte alignment         */
	cbz r3, zc_skip_align			/* Jump to load multiple if aligned      */
	ldrb r2, [r0, #0]			/* Compare one byte at a time            */
	orr r6, r2				/* OR all loaded values to validate ZERO */
	add r0, #1
	sub r1, r1, #1
	b zc_align

zc_skip_align:
#endif /* ZCMP_UNALIGNED_SUPPORT */

	cmp r1, #16				/* Load 16 bytes at a time if remaining  */
	blt c8					/* length is >= 16                       */
c16:
	ldmia r0!,{r2,r3,r4,r5}
	orr r6, r2				/* OR all values to compare to ZERO      */
	orr r6, r3
	orr r6, r4
	orr r6, r5

	sub r1, r1, #16
	cmp r1, #15
	bgt c16

c8:
	cmp r1, #8				/* Load 8 bytes at a time if remaining   */
	blt c4					/* length is >= 8                        */
	ldmia r0!,{r2,r3}
	sub r1, r1, #8
	orr r6, r2
	orr r6, r3

c4:
#ifdef ZCMP_UNALIGNED_SUPPORT
	cmp r1, #4				/* Load 4 bytes at a time if remaining */
	blt c1					/* length is >= 4                       */
#else
	cbz r1, zc_end
#endif // endif
	ldmia r0!,{r2}
	orr r6, r2

#ifdef ZCMP_UNALIGNED_SUPPORT
	sub r1, r1, #4

c1:
	cbz r1, zc_end				/* Compare remaining bytes one at a time */
	ldrb r2, [r0, #0]
	sub r1, r1, #1
	add r0, #1
	orr r6, r2
	b c1
#endif // endif

zc_end:
	mov r0, r6
	ldmfd sp!, {r4, r5, r6, pc}

END(rte_bzerocmp)

/* basic mem xor checksum
 * Input
 *  - a1 - Buffer address
 *  - a2 - Length of the buffer
 * Return
 *  - Final XORed value.
 */
FUNC(rte_bxor)
	stmfd sp!, {r4, r5, r6, lr}

	mov r6, #0				/* R6 holds the final XORed value          */

#ifdef ZCMP_UNALIGNED_SUPPORT
	mov r2, #0

	/* Check if input buffer is aligned on 4 byte boundary.
	 * If not, read unaligned portion in the front of the buffer.
	 * Jump to load multiple loops on reaching required alignment
	 */
x_align:
	cbz r1, x_end				/* Requested number of bytes             */
	and r3, r0, #3				/* Check for four byte alignment         */
	cbz r3, x_skip_align			/* Jump to load multiple if aligned      */
	ldrb r2, [r0, #0]			/* Read one byte at a time               */
	eor r6, r2				/* XOR all loaded values                 */
	add r0, #1
	sub r1, r1, #1
	b x_align

x_skip_align:
#endif /* ZCMP_UNALIGNED_SUPPORT */

	cmp r1, #16				/* Load 16 bytes at a time if remaining  */
	blt x8					/* length is >= 16                       */
x16:
	ldmia r0!, {r2,r3,r4,r5}
	eor r2, r3				/* XOR all values */
	eor r2, r4
	eor r2, r5
	eor r6, r2
	sub r1, r1, #16
	cmp r1, #15
	bgt x16

x8:
	cmp r1, #8				/* Load 8 bytes at a time if remaining   */
	blt x4					/* length is >= 8                        */
	ldmia r0!,{r2,r3}
	sub r1, r1, #8
	eor r2, r3
	eor r6, r2

x4:
#ifdef ZCMP_UNALIGNED_SUPPORT
	cmp r1, #4				/* Load 4 bytes at a time if remaining  */
	blt x1					/* length is >= 4                       */
#else
	cbz r1, x_end
#endif // endif
	ldmia r0!,{r2}
	eor r6, r2

#ifdef ZCMP_UNALIGNED_SUPPORT
	sub r1, r1, #4

x1:
	cbz r1, x_end				/* XOR remaining bytes one at a time */
	ldrb r2, [r0, #0]
	sub r1, r1, #1
	add r0, #1
	eor r6, r2
	b x1
#endif // endif

x_end:
	mov r0, r6
	ldmfd sp!, {r4, r5, r6, pc}

END(rte_bxor)

/* 32 byte block copy
 * Input
 *  - a1 - Dest
 *  - a2 - Source
 *  - a3 - Number of 32 byte blocks to copy
 * Return
 *  - None
 */
FUNC(rte_memcpy32)
	stmfd sp!, {r4-r10, lr}
mcp_rpt:
	cbz r2, mcp_done		/* Done copying number of blocks */
	ldmia r1!, {r3-r10}		/* Load 32 bytes                 */
	stmia r0!, {r3-r10}		/* Store 32 bytes                */
	subs r2, r2, #1			/* Decrement remaining blocks    */
	b mcp_rpt

mcp_done:
	ldmfd sp!, {r4-r10, pc}
END(rte_memcpy32)
