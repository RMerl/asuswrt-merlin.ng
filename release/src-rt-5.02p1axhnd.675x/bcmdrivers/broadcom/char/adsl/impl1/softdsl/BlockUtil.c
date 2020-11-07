/* File:	BlockUtil.c
 **
 **	Description:
 **		This file contains the fixed point block processing utilities.

 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 $Revision: 1.28 $

 $Id: BlockUtil.c,v 1.28 2011/06/22 16:03:05 ovandewi Exp $

 $Log: BlockUtil.c,v $
 Revision 1.28  2011/06/22 16:03:05  ovandewi
 PR5739: fix mexFile compilation

 Revision 1.27  2010/03/25 18:44:08  ilyas
 Use built in 64bit operations in VL functions

 Revision 1.26  2003/11/11 22:35:09  linyin
 Use WARN instead of ASSERT to make simulation work, should be checked the reason later

 Revision 1.25  2002/03/26 23:56:59  linyin
 add rounding to BlockCplxScaleComplexSymbol() to match Qproc CVMUL

 Revision 1.24  2002/03/12 00:02:59  yongbing
 Modify cplxScaleCplxSymbols to accept a shift value instead of an array of shifts

 Revision 1.23  2001/04/24 22:48:26  yongbing
 Change requirement of Size to size>=4 in BlockShortScale

 Revision 1.22  2001/03/27 18:11:31  yongbing
 Change requirement of sample size from multiple of 8 to 4 in BlockShortScale

 Revision 1.21  2000/12/08 19:13:07  yongbing
 Add ASSERT to block utility functions on negative and zero block sizes

 Revision 1.20  2000/11/30 03:54:08  khp
 -BlockRealScaleCplxSymbols instead of BlockScaleComplexSymbols

 Revision 1.19  2000/11/29 20:42:11  liang
 Add function for ADSL xmt gains with fixed shift.

 Revision 1.18  2000/09/15 17:50:16  yjchen
 move Feq-related functions to BlockFeq.c

 Revision 1.17  2000/09/13 16:57:20  yongbing
 Write Pentium MMX assembly codes for new FEQ coef format

 Revision 1.16  2000/09/09 00:23:45  liang
 Add corresponding functions for the ComplexLong FEQ coef.

 Revision 1.15  2000/08/20 22:42:17  liang
 Separate out more smaller functions for pipeline.

 Revision 1.14  2000/06/16 20:23:40  yongbing
 Correct an error in function BlockShortScaleByShift

 Revision 1.13  2000/05/17 01:36:41  yongbing
 Add Pentium MMX assembly codes for more block related functions

 Revision 1.12  2000/04/19 19:22:18  yongbing
 Add BlockShortScaleby2 function used in G994p1

 Revision 1.11  2000/04/18 00:50:36  yongbing
 Remove ASSERT on requirements of even size value in BlockByteMove function, used in new frame code

 Revision 1.10  2000/04/04 02:28:01  liang
 Merged with SoftDsl_0_2 from old tree.

 Revision 1.10  2000/03/14 23:27:51  yongbing
 Move BlockCplxSymbolUpdateCplxScale function to BlockSymbolUtil.c; file may be closed

 Revision 1.9  2000/03/02 03:55:20  liang
 Minor data type casting change.

 Revision 1.8  2000/03/01 02:00:17  yongbing
 Add warning for integer overflow in BlockCplxScaleComplexSymbols (FEQ)

 Revision 1.7  2000/02/16 01:51:52  yongbing
 Move function BlockCplxScaleComplexSymbols from BlockSymbolUtil.c file

 Revision 1.6  1999/08/05 19:42:33  liang
 Merged with the softmodem top of the tree on 08/04/99 for assembly files.

 Revision 1.5  1999/05/14 22:49:38  liang
 Added BlockComplexShortFill and moved two functions to another file.

 Revision 1.4  1999/03/26 03:29:56  liang
 Add function BlockComplexMultLongAcc.

 Revision 1.3  1999/02/22 22:40:58  liang
 BlockByteSum takes uchar inputs instead of schar.

 Revision 1.2  1999/02/10 01:56:42  liang
 Added BlockByteSum, BlockRealScaleComplexSymbols and BlockCplxScaleComplexSymbols.

 Revision 1.1  1998/10/28 01:35:38  liang
 *** empty log message ***

 Revision 1.33  1998/09/30 00:07:41  scott
 Added ENABLE_THUNKS support for BlockByteMove

 Revision 1.32  1998/09/29 03:34:39  mwg
 Added #ifdefs for BlockUtil.c

 Revision 1.31  1998/08/28 19:35:49  scott
 Corrected some 6-specific asserts

 Revision 1.30  1998/08/25 21:56:50  scott
 Added more 6 asserts

 Revision 1.29  1998/08/18 05:08:48  mwg
 Implemented SH3 assembly builds

 Revision 1.28  1998/08/15 07:21:01  scott
 Fixed problem with discarding high bits in 40-bit BlockPower implementation

 Revision 1.27  1998/07/30 04:05:42  scott
 Removed asm version of BlockByteMove

 Revision 1.26  1998/07/30 00:13:18  scott
 Moved BlockMapShort2Complex to 6 assembler

 Revision 1.25  1998/07/28 23:30:18  scott
 Moved BlockByteMove, BlockShortClear, BlockSum, BlockFullPower to assembly.
 Added 40-bit C version of BlockPower.

 Revision 1.24  1998/07/17 18:25:24  scott
 Changed criteria for BlockByteFillFast

 Revision 1.23  1998/07/16 16:53:41  scott
 Added BlockShortOffset to list of asm files for C6x

 Revision 1.22  1998/07/11 01:52:27  scott
 For 6, moved BlockFullPower, BlockComplexMult and BlockShortSubtract to asm.

 Revision 1.21  1998/07/08 17:09:22  scott
 Added C6X_ASM defines and removed now-obsolete undefs

 Revision 1.20  1998/06/04 04:40:22  mwg
 Added assert in BlockShortClear

 Revision 1.19  1998/06/04 04:37:10  mwg
 Reduced the loop unrolling for BlockByteFill

 Revision 1.18  1998/04/27 22:51:42  scott
 Added an extra assert for BlockCorrelate

 Revision 1.17  1998/03/19 21:59:09  mwg
 Changed the unrolling factor in BlockByteFill to 2 from 4.

 * Revision 1.16  1998/03/07  05:39:13  mwg
 * Optimized MIPS versions of BlockShortMove() BlockReal2ComplexMacc() and
 * BlockPower().
 *
 * Revision 1.15  1998/02/16  18:40:59  scott
 * Added MMX autodetect support
 *
 Revision 1.14  1998/01/27 01:40:29  mwg
 Moved some of the utilities to BlockUtilSlow.c to avoid
 optimizing them in assembly. We will later reconsider if
 some of the functions are to be optimized.

 * Revision 1.13  1997/12/13  06:11:28  mwg
 * Added new functions:
 * BlockLongSubtract()
 * BlockLongAdd()
 * BlockLong2ShortSubtract()
 * BlockShort2LongMove()
 * BlockShortInterpolate()
 * BlockLongCorrelate()
 * BlockMapShort2Short()
 *
 Revision 1.12  1997/09/08 23:56:54  scott
 Conditionally exclude certain functions for PENTIUM_MMX build

 Revision 1.11  1997/03/19 18:35:10  mwg
 Changed copyright notice.

 * Revision 1.10  1997/02/14  03:35:39  liang
 * Added asserts for all functions so that their MIPS asm versions will work.
 *
 * Revision 1.9  1997/02/11  00:08:17  mwg
 * Added BlockByteMove function
 *
 * Revision 1.8  1997/02/04  08:40:08  mwg
 * Changed interface forBlockReal2ComplexMacc()
 *
 * Revision 1.7  1997/01/23  02:04:28  mwg
 * Added return value to BlockShortMove
 *
 * Revision 1.6  1996/12/19  22:34:53  mwg
 * Added new function BlockFullPower().
 *
 * Revision 1.5  1996/05/02  08:40:17  mwg
 * Merged in Chromatic bug fixes.
 *
 * Revision 1.4  1996/03/08  22:46:36  mwg
 * Replaced all add's and sub's with addu's and subu's to avoid any
 * possibility of arithmetic exception.
 *
 * Revision 1.3  1996/03/02  00:57:30  liang
 * Replace #include "SoftModem.gh" with #include "SoftModem.h"
 *
 * Revision 1.2  1996/02/21  03:59:14  mwg
 * Added new function BlockReal2ComplexMacc
 *
 * Revision 1.1.1.1  1996/02/14  02:35:13  mwg
 * Redesigned the project directory structure. Merged V.34 into the project.
 *
 * Revision 1.8  1995/04/25  01:13:47  anand
 * _
 *
 */

/******************* Header files ************************/

#include "SoftModem.h"
#include "BlockUtil.h"
#include "EndianUtil.h"

#ifdef __KERNEL__
#include "linux/version.h"
#undef WARN
#define	WARN(a)
#endif

#ifdef C54_COMPATIBILITY_MODE
#define BLOCKPOWER_40BIT
#endif

#ifdef BLOCKPOWER_40BIT
#include "MathUtil.h"
#endif

#if defined(CONFIG_ARM64)

#define CPY1(d,s) do {												\
	register int tmp;												\
	__asm volatile("ldrb  %w0, [%x1], #1" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strb  %w1, [%x0], #1" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY2(d,s) do {												\
	register int  tmp;												\
	__asm volatile("ldrh  %w0, [%x1], #2" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strh  %w1, [%x0], #2" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY4(d,s) do {												\
	register int  tmp;												\
	__asm volatile("ldr  %w0, [%x1], #4" : "=r" (tmp), "+r" (s));	\
	__asm volatile("str  %w1, [%x0], #4" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY8(d,s) do {												\
	register long  tmp;												\
	__asm volatile("ldr  %x0, [%x1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("str  %x1, [%x0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY16(d,s) do {																	\
	register long  tmp1, tmp2;															\
	__asm volatile("ldp  %x0, %x1, [%x2], #16" : "=r" (tmp1), "=r" (tmp2), "+r" (s));	\
	__asm volatile("stp  %x1, %x2, [%x0], #16" : "+r" (d) : "r" (tmp1), "r" (tmp2));	\
} while (0)

void *BlockMemCopy(void *dst, void *src, int size, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	if (size < nAlign) {
		char *srcEndPtr = srcPtr + size;
		while (srcPtr != srcEndPtr)
		  CPY1(dstPtr, srcPtr);
		return dst;
	}

	if (nAlign) {
	  if (nAlign & 1)
		CPY1(dstPtr, srcPtr);
	  if (nAlign & 2)
		CPY2(dstPtr, srcPtr);
	  if (nAlign & 4)
		CPY4(dstPtr, srcPtr);
	  size -= nAlign;
	}

	if (size >=64) {
		/* copy by 64 bytes */
	    src64End = srcPtr + (size & ~0x3F);
		do {
			CPY16(dstPtr, srcPtr);
			CPY16(dstPtr, srcPtr);
			CPY16(dstPtr, srcPtr);
			CPY16(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 16 bytes */
	if (size & 0x20) {
		CPY16(dstPtr, srcPtr);
		CPY16(dstPtr, srcPtr);
	}
	if (size & 0x10)
		CPY16(dstPtr, srcPtr);

	/* copy remaining */
	if (size & 8)    // >= 8
		CPY8(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		CPY4(dstPtr, srcPtr);
	if (size & 2)    // >= 2
		CPY2(dstPtr, srcPtr);
	if (size & 1)    // >= 1
		CPY1(dstPtr, srcPtr);

	return dst;
}

#define REV16_1(d,s) do {											\
	register int  tmp;												\
	__asm volatile("ldrh  %w0, [%x1], #2" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev16 %w0, %w1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("strh  %w1, [%x0], #2" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV16_2(d,s) do {											\
	register int  tmp;												\
	__asm volatile("ldr  %w0, [%x1], #4" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev16 %w0, %w1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("str  %w1, [%x0], #4" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV16_4(d,s) do {											\
	register long  tmp;												\
	__asm volatile("ldr  %x0, [%x1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev16 %x0, %x1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("str  %x1, [%x0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV16_8(d,s) do {																\
	register long  tmp1, tmp2;															\
	__asm volatile("ldp  %x0, %x1, [%x2], #16" : "=r" (tmp1), "=r" (tmp2), "+r" (s));	\
	__asm volatile("rev16 %x0, %x1" : "=r" (tmp1) : "r" (tmp1));		\
	__asm volatile("rev16 %x0, %x1" : "=r" (tmp2) : "r" (tmp2));		\
	__asm volatile("stp  %x1, %x2, [%x0], #16" : "+r" (d) : "r" (tmp1), "r" (tmp2));	\
} while (0)

void *BlockShortRev(int size, void *src, void *dst, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	size <<= 1;
	if (size < nAlign) {
		char *srcEndPtr = srcPtr + size;
		while (srcPtr != srcEndPtr)
		  REV16_1(dstPtr, srcPtr);
		return dst;
	}

	if (nAlign) {
	  if (nAlign & 2)
		REV16_1(dstPtr, srcPtr);
	  if (nAlign & 4)
		REV16_2(dstPtr, srcPtr);
	  size -= nAlign;
	}

	if (size >=64) {
		/* copy by 64 bytes */
	    src64End = srcPtr + (size & ~0x3F);
		do {
			REV16_8(dstPtr, srcPtr);
			REV16_8(dstPtr, srcPtr);
			REV16_8(dstPtr, srcPtr);
			REV16_8(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 16 bytes */
	if (size & 0x20) {
		REV16_8(dstPtr, srcPtr);
		REV16_8(dstPtr, srcPtr);
	}
	if (size & 0x10)
		REV16_8(dstPtr, srcPtr);

	/* copy remaining */
	if (size & 8)    // >= 8
		REV16_4(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		REV16_2(dstPtr, srcPtr);
	if (size & 2)    // >= 2
		REV16_1(dstPtr, srcPtr);

	return dst;
}

#define REV32_1(d,s) do {											\
	register int  tmp;												\
	__asm volatile("ldr  %w0, [%x1], #4" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev  %w0, %w1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("str  %w1, [%x0], #4" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV32_2(d,s) do {											\
	register long  tmp;												\
	__asm volatile("ldr  %x0, [%x1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev32 %x0, %x1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("str  %x1, [%x0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV32_4(d,s) do {																\
	register long  tmp1, tmp2;															\
	__asm volatile("ldp  %x0, %x1, [%x2], #16" : "=r" (tmp1), "=r" (tmp2), "+r" (s));	\
	__asm volatile("rev32 %x0, %x1" : "=r" (tmp1) : "r" (tmp1));		\
	__asm volatile("rev32 %x0, %x1" : "=r" (tmp2) : "r" (tmp2));		\
	__asm volatile("stp  %x1, %x2, [%x0], #16" : "+r" (d) : "r" (tmp1), "r" (tmp2));	\
} while (0)

void *BlockLongRev(int size, void *src, void *dst, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	if (0 == size)
		return dst;

	size <<= 2;
	if (nAlign & 4) {
		REV32_1(dstPtr, srcPtr);
	    size -= 4;
	}

	if (size >=64) {
		/* copy by 64 bytes */
	    src64End = srcPtr + (size & ~0x3F);
		do {
			REV32_4(dstPtr, srcPtr);
			REV32_4(dstPtr, srcPtr);
			REV32_4(dstPtr, srcPtr);
			REV32_4(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 16 bytes */
	if (size & 0x20) {
		REV32_4(dstPtr, srcPtr);
		REV32_4(dstPtr, srcPtr);
	}
	if (size & 0x10)
		REV32_4(dstPtr, srcPtr);

	/* copy remaining */
	if (size & 8)    // >= 8
		REV32_2(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		REV32_1(dstPtr, srcPtr);

	return dst;
}

#elif defined(CONFIG_ARM)

#define CPY1(d,s) do {												\
	register int tmp;												\
	__asm volatile("ldrb  %0, [%1], #1" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strb  %1, [%0], #1" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY2(d,s) do {												\
	register int  tmp;												\
	__asm volatile("ldrh  %0, [%1], #2" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strh  %1, [%0], #2" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY4(d,s) do {												\
	register int  tmp;												\
	__asm volatile("ldr  %0, [%1], #4" : "=r" (tmp), "+r" (s));	\
	__asm volatile("str  %1, [%0], #4" : "+r" (d) : "r" (tmp));	\
} while (0)

#define CPY8(d,s) do {												\
	register long long tmp;											\
	__asm volatile("ldrd  %0, [%1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("strd  %1, [%0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

void *BlockMemCopy(void *dst, void *src, int size, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	if (size < nAlign) {
		char *srcEndPtr = srcPtr + size;
		while (srcPtr != srcEndPtr)
		  CPY1(dstPtr, srcPtr);
		return dst;
	}

	if (nAlign) {
	  if (nAlign & 1)
		CPY1(dstPtr, srcPtr);
	  if (nAlign & 2)
		CPY2(dstPtr, srcPtr);
	  if (nAlign & 4)
		CPY4(dstPtr, srcPtr);
	  size -= nAlign;
	}

	if (size >=32) {
		/* copy by 32 bytes */
	    src64End = srcPtr + (size & ~0x1F);
		do {
			CPY8(dstPtr, srcPtr);
			CPY8(dstPtr, srcPtr);
			CPY8(dstPtr, srcPtr);
			CPY8(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 8 bytes */
	if (size & 0x10) {
		CPY8(dstPtr, srcPtr);
		CPY8(dstPtr, srcPtr);
	}

	/* copy remaining */
	if (size & 8)    // >= 8
		CPY8(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		CPY4(dstPtr, srcPtr);
	if (size & 2)    // >= 2
		CPY2(dstPtr, srcPtr);
	if (size & 1)    // >= 1
		CPY1(dstPtr, srcPtr);

	return dst;
}

#define REV16_1(d,s) do {											\
	register int  tmp;												\
	__asm volatile("ldrh  %0, [%1], #2" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev16 %0, %1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("strh  %1, [%0], #2" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV16_2(d,s) do {											\
	register int  tmp;												\
	__asm volatile("ldr  %0, [%1], #4" : "=r" (tmp), "+r" (s));		\
	__asm volatile("rev16 %0, %1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("str  %1, [%0], #4" : "+r" (d) : "r" (tmp));		\
} while (0)

#define REV16_4(d,s) do {											\
	register long long tmp asm("r4");								\
	__asm volatile("ldrd  %0, [%1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev16 r4, r4");									\
	__asm volatile("rev16 r5, r5");									\
	__asm volatile("strd  %1, [%0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

void *BlockShortRev(int size, void *src, void *dst, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	size <<= 1;
	if (size < nAlign) {
		char *srcEndPtr = srcPtr + size;
		while (srcPtr != srcEndPtr)
		  REV16_1(dstPtr, srcPtr);
		return dst;
	}

	if (nAlign) {
	  if (nAlign & 2)
		REV16_1(dstPtr, srcPtr);
	  if (nAlign & 4)
		REV16_2(dstPtr, srcPtr);
	  size -= nAlign;
	}

	if (size >=32) {
		/* copy by 32 bytes */
	    src64End = srcPtr + (size & ~0x1F);
		do {
			REV16_4(dstPtr, srcPtr);
			REV16_4(dstPtr, srcPtr);
			REV16_4(dstPtr, srcPtr);
			REV16_4(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 8 bytes */
	if (size & 0x10) {
		REV16_4(dstPtr, srcPtr);
		REV16_4(dstPtr, srcPtr);
	}

	/* copy remaining */
	if (size & 8)    // >= 8
		REV16_4(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		REV16_2(dstPtr, srcPtr);
	if (size & 2)    // >= 2
		REV16_1(dstPtr, srcPtr);

	return dst;
}

#define REV32_1(d,s) do {										\
	register int  tmp;											\
	__asm volatile("ldr  %0, [%1], #4" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev  %0, %1" : "=r" (tmp) : "r" (tmp));		\
	__asm volatile("str  %1, [%0], #4" : "+r" (d) : "r" (tmp));	\
} while (0)

#define REV32_2(d,s) do {											\
	register long  long tmp asm("r4");								\
	__asm volatile("ldrd  %0, [%1], #8" : "=r" (tmp), "+r" (s));	\
	__asm volatile("rev   r4, r4");									\
	__asm volatile("rev   r5, r5");									\
	__asm volatile("strd  %1, [%0], #8" : "+r" (d) : "r" (tmp));	\
} while (0)

void *BlockLongRev(int size, void *src, void *dst, int nAlign)
{
	char *dstPtr = dst, *srcPtr = src;
	char *src64End;

	if (0 == size)
		return dst;

	size <<= 2;
	if (nAlign & 4) {
		REV32_1(dstPtr, srcPtr);
	    size -= 4;
	}

	if (size >=32) {
		/* copy by 32 bytes */
	    src64End = srcPtr + (size & ~0x1F);
		do {
			REV32_2(dstPtr, srcPtr);
			REV32_2(dstPtr, srcPtr);
			REV32_2(dstPtr, srcPtr);
			REV32_2(dstPtr, srcPtr);
		} while (srcPtr != src64End);
	}
	/* copy remaining by 16 bytes */
	if (size & 0x10) {
		REV32_2(dstPtr, srcPtr);
		REV32_2(dstPtr, srcPtr);
	}

	/* copy remaining */
	if (size & 8)    // >= 8
		REV32_2(dstPtr, srcPtr);
	if (size & 4)    // >= 4
		REV32_1(dstPtr, srcPtr);

	return dst;
}

#endif /* CONFIG_ARM64,  CONFIG_ARM */

Public	void
BlockLongSubtract (int size, int* src1Ptr, int* src2Ptr, int* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block suburaction of two short type arrays		*/	
	/****************************************************************************/
	{
	int	*srcEndPtr = src1Ptr + size;
	
	ASSERT((size & 1) == 0);
	/*C6x ASSERT(size >= 4);*/
	ASSERT(size > 0);
	
	do
		{
		*dstPtr++ = *src1Ptr++ - *src2Ptr++;
		} while (src1Ptr != srcEndPtr);
	}

Public	void
BlockShort2LongMove (int size, short* srcPtr, int* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block suburaction of two short type arrays		*/	
	/****************************************************************************/
	{
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 1) == 0);
	ASSERT(size > 0);

	do
		{
		*dstPtr++ = *srcPtr++;
		} while (srcPtr != srcEndPtr);
	}


#if !defined C6X_ASM && !defined C54_ASM
Public	void
BlockShortAdd (int size, short* src1Ptr, short* src2Ptr, short* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block addution of two short type arrays			*/	
	/****************************************************************************/
	{
	short	*srcEndPtr = src1Ptr + size;
	
	ASSERT((size & 1) == 0);
	ASSERT(size > 0);
	
	do
		{
		*dstPtr++ = *src1Ptr++ + *src2Ptr++;
		} while (src1Ptr != srcEndPtr);
	}
#endif


#if !defined C6X_ASM && !defined SH3_ASM && !defined C54_ASM && !defined DSP16K_ASM && !defined PPC_ASM
Public	void
BlockShortScale (int size, short scaleFactor, int shift, short* srcPtr, short* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block scaling of the short type array				*/	
	/****************************************************************************/
	{
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 3) == 0); /* Pentium MMX codes process 4 samples at one time */
	ASSERT(size >= 4);
	
	do
		{
		*dstPtr++ = ( ((int)*srcPtr++) * (int)scaleFactor) >> shift;
		} while (srcPtr != srcEndPtr);
	}
#endif

#if !defined C6X_ASM && !defined C54_ASM
Public	void
BlockShortOffset (int size, short offset, short* srcPtr, short* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block offsetining of the short type array				*/	
	/****************************************************************************/
	{
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);

	do
		{
		*dstPtr++ = *srcPtr++ + offset;
		} while (srcPtr != srcEndPtr);
	}
#endif

#if !defined C6X_ASM && !defined SH3_ASM && !defined C54_ASM && !defined DSP16K_ASM && !defined PPC_ASM && !defined(MATLAB_MEX)
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
Public	int BlockShortMove(int size, short* srcPtr, short* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block move of a short type data					*/	
	/****************************************************************************/
{
	ASSERT((size & 1) == 0);
	/*C6x*/ ASSERT(size >= 4);
	ASSERT(size > 0);
	if(size > 0) {
		short	*srcEndPtr = srcPtr + size;
		do {
			*dstPtr++ = *srcPtr++;
		} while (srcPtr != srcEndPtr);
	}
	return size;
}
#endif
#endif

#if !defined(C54_ASM) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
Public	void
BlockByteMove (int size, uchar* srcPtr, uchar* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block move of a short type data					*/	
	/****************************************************************************/
{
	/*ASSERT((size & 1) == 0);*/	/* not an absolute requirement */
	/*ASSERT(((uintptr_t)srcPtr & 1)==0);*/ /* C6x */
	/*ASSERT(((uintptr_t)dstPtr & 1)==0);*/ /* C6x */
	ASSERT(size > 0);
	if(size > 0) {
		uchar	*srcEndPtr = srcPtr + size;
		do {
			*dstPtr++ = *srcPtr++;
		} while (srcPtr != srcEndPtr);
	}
}
#endif

#if !defined C54_ASM
Public	void SM_DECL
BlockByteFill (int size, uchar pattern, uchar* dstPtr)
	/****************************************************************************/
	/*	This function fills a block with a constant								*/	
	/****************************************************************************/
	{
	uchar	*dstEndPtr = dstPtr + size;

	ASSERT(size > 0);

#ifdef C6X_ASM
#ifdef ENABLE_THUNKS
	extern TIBlockByteFillFast(int size, uchar pattenr, uchar *dstPtr);
	
	if (((uintptr_t)dstPtr & 1)==0 && (size & 1)==0)
		{
		TIBlockByteFillFast(size, pattern, dstPtr);
		return;
		}
#else
	extern BlockByteFillFast(int size, uchar pattenr, uchar *dstPtr);
	
	if (((uintptr_t)dstPtr & 1)==0 && (size & 1)==0)
		{
		BlockByteFillFast(size, pattern, dstPtr);
		return;
		}
#endif

#endif	

	do
		{
		*dstPtr++ = pattern;
		} while (dstPtr != dstEndPtr);
	}

Public	void
BlockByteClear (int size, uchar* dstPtr)
	/****************************************************************************/
	/*	This function fills a block with a constant								*/	
	/****************************************************************************/
	{
	uchar	*dstEndPtr = dstPtr + size;

	ASSERT(size > 0);

#ifdef C6X_ASM
#ifdef ENABLE_THUNKS
	extern TIBlockByteFillFast(int size, uchar pattenr, uchar *dstPtr);
	
	if (((uintptr_t)dstPtr & 1)==0 && (size & 1)==0)
		{
		TIBlockByteFillFast(size, 0, dstPtr);
		return;
		}
#else
	extern BlockByteFillFast(int size, uchar pattenr, uchar *dstPtr);
	
	if (((uintptr_t)dstPtr & 1)==0 && (size & 1)==0)
		{
		BlockByteFillFast(size, 0, dstPtr);
		return;
		}
#endif

#endif	

	do
		{
		*dstPtr++ = 0;
		} while (dstPtr != dstEndPtr);
	}

#endif

#if !defined C6X_ASM && !defined C54_ASM
Public	void
BlockShortFill (int size, short pattern, short* dstPtr)
	/****************************************************************************/
	/*	This function fills a block with a constant								*/	
	/****************************************************************************/
	{
	short	*dstEndPtr = dstPtr + size;
	
	ASSERT((size & 1) == 0);
	ASSERT(size > 0);
	
	do
		{
		*dstPtr++ = pattern;
		} while (dstPtr != dstEndPtr);
	}
#endif

#if !defined C6X_ASM && !defined C54_ASM && !defined(MATLAB_MEX)
Public	void
BlockShortClear (int size, short* dstPtr)
	/****************************************************************************/
	/*	This function fills a block with a constant								*/	
	/****************************************************************************/
	{
	short	*dstEndPtr = dstPtr + size;
	
	ASSERT((size & 1) == 0);
	ASSERT(size > 0);

#ifdef TI_C6X
	ASSERT(((uintptr_t)dstPtr & 1)==0);
#endif
	
	do
		{
		*dstPtr++ = 0;
		} while (dstPtr != dstEndPtr);
	}
#endif

Public	void
BlockLongFill (int size, int pattern, int* dstPtr)
	/****************************************************************************/
	/*	This function fills a block with a constant								*/	
	/****************************************************************************/
	{
	int	*dstEndPtr = dstPtr + size;
	
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);
	
	do
		{
		*dstPtr++ = pattern;
		} while (dstPtr != dstEndPtr);
	}

Public	void
BlockLongClear (int size, int* dstPtr)
	/****************************************************************************/
	/*	This function fills a block with a constant								*/	
	/****************************************************************************/
	{
	int	*dstEndPtr = dstPtr + size;
	
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);
	
	do
		{
		*dstPtr++ = 0;
		} while (dstPtr != dstEndPtr);
	}

Public	void
BlockComplexShortFill(int size, short xFill, short yFill, ComplexShort *dstPtr)
	{
	ComplexShort	*dstEndPtr = dstPtr + size;

	ASSERT(size > 0);

	do
		{
		dstPtr->x = xFill;
		dstPtr->y = yFill;
		} while (++dstPtr != dstEndPtr);
	}

Public	void
BlockComplexLongFill(int size, int xFill, int yFill, ComplexLong *dstPtr)
	{
	ComplexLong	*dstEndPtr = dstPtr + size;

	ASSERT(size > 0);

	do
		{
		dstPtr->x = xFill;
		dstPtr->y = yFill;
		} while (++dstPtr != dstEndPtr);
	}

Public	void
BlockComplexShortClear(int size, ComplexShort *dstPtr)
	{
	ComplexShort	*dstEndPtr = dstPtr + size;

	ASSERT(size > 0);

	do
		{
		dstPtr->x = 0;
		dstPtr->y = 0;
		} while (++dstPtr != dstEndPtr);
	}

Public	void
BlockShortInvert (int size, short* srcPtr, short* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block move of a short type data					*/	
	/****************************************************************************/
	{
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);
	
	do
		{
		*dstPtr++ = -(*srcPtr++);
		} while (srcPtr != srcEndPtr);
	}

	
#ifndef C6X_ASM
Public void
BlockShortScaleDown (int size, short* dstPtr)
	/****************************************************************************/
	/*	This function scales down by one an array of short data					*/	
	/****************************************************************************/
	{

	short	*dstEndPtr = dstPtr + size;

	ASSERT((size & 1) == 0);
	/*C6x*/ ASSERT(size >= 2);
	
	ASSERT(size > 0);
	
	do
		{
		if (*dstPtr >= 0)
			(*dstPtr)--;
		if (*dstPtr < 0)
			(*dstPtr)++;
		} while (++dstPtr != dstEndPtr);

	}
#endif

#ifndef C54_ASM
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
Public	void
BlockLongMove (int size, int* srcPtr, int* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block move of a long type data					*/	
	/****************************************************************************/
{
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);
	if(size > 0) {
		int	*srcEndPtr = srcPtr + size;
		do {
			*dstPtr++ = *srcPtr++;
		} while (srcPtr != srcEndPtr);
	}
}
#endif

#endif

Public	void
BlockShortInterpolate (int size, short scaleFactor, int shift, short* src1Ptr, short* src2Ptr, short* dstPtr)
	/****************************************************************************/
	/*	This function perfoms linear interpolation of two arrays				*/	
	/****************************************************************************/
	{
	short	*src1EndPtr = src1Ptr + size;
	
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);
	
	do
		{
		*dstPtr = *src1Ptr + (((int)(*src2Ptr - *src1Ptr) * scaleFactor) >> shift);
		src1Ptr++;
		src2Ptr++;
		dstPtr++;
		} while (src1Ptr != src1EndPtr);
	}


Public	void
BlockReal2ComplexMult	(int size,
						 short*			src1Ptr,
						 ComplexShort*	src2Ptr,
						 ComplexShort*	dstPtr)
	/****************************************************************************/
	/*	This function perfoms block multiply of two short and ComplexShort arrays	*/	
	/****************************************************************************/
	{
	short	*src1EndPtr = src1Ptr + size;
	
	ASSERT(size > 0);

	do
		{
		dstPtr->x = ((int)(*src1Ptr) * (int)(src2Ptr->x)) >> 14;
		dstPtr->y = ((int)(*src1Ptr) * (int)(src2Ptr->y)) >> 14;
		src1Ptr++;
		src2Ptr++;
		dstPtr++;
		} while (src1Ptr != src1EndPtr);
	}

Public	void
BlockComplexMultLongAcc	(int size,
						 int shift,
						 ComplexShort* src1Ptr,
						 ComplexShort* src2Ptr,
						 ComplexLong* dstPtr)
	/****************************************************************************
	 *	This function perfoms block complex multiply of two ComplexShort arrays,
	 *	shift the product, and then add to the dstPtr buffer.
	 ****************************************************************************/
	{
	ComplexShort	*src1EndPtr = src1Ptr + size;
	
	ASSERT(size > 0);

	do
		{
		dstPtr->x += ( (int)(src1Ptr->x) * (int)(src2Ptr->x) -
		               (int)(src1Ptr->y) * (int)(src2Ptr->y) ) >> shift;
		dstPtr->y += ( (int)(src1Ptr->x) * (int)(src2Ptr->y) +
		               (int)(src1Ptr->y) * (int)(src2Ptr->x) ) >> shift;
		
		src1Ptr++;
		src2Ptr++;
		dstPtr++;
		} while (src1Ptr != src1EndPtr);
	}

#if !defined C6X_ASM && !defined C54_ASM && !defined ARM_PICCOLO_ASM && !defined PPC_ASM
Public	void
BlockComplexMult	(int size,
					 ComplexShort* src1Ptr,
					 ComplexShort* src2Ptr,
					 ComplexShort* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block complex multiply of two ComplexShort arrays	*/	
	/****************************************************************************/
	{
	ComplexShort	*src1EndPtr = src1Ptr + size, 
					shortComplexTemp;

	/*C6x ASSERT(size >= 3);*/
	
	ASSERT(size > 0);

	do
		{
		shortComplexTemp.x = (	(int)(src1Ptr->x) * (int)(src2Ptr->x)
		             		  -	(int)(src1Ptr->y) * (int)(src2Ptr->y) ) >> 14;
		shortComplexTemp.y = (	(int)(src1Ptr->x) * (int)(src2Ptr->y)
		             		  +	(int)(src1Ptr->y) * (int)(src2Ptr->x) ) >> 14;
		*dstPtr = shortComplexTemp;
		src1Ptr++;
		src2Ptr++;
		dstPtr++;
		} while (src1Ptr != src1EndPtr);
	}
#endif

#if !defined C6X_ASM && !defined C54_ASM && !defined ARM_PICCOLO_ASM && !defined PPC_ASM
Public	void
BlockComplexConjigateMult	(int size,
							 ComplexShort* src1Ptr,
							 ComplexShort* src2Ptr,
							 ComplexShort* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block complex conjigate multiply of two arrays	*/	
	/****************************************************************************/
	{
	ComplexShort	*src1EndPtr = src1Ptr + size, 
					shortComplexTemp;

	/*C6x ASSERT(size >= 3);*/
	
	ASSERT(size > 0);

	do
		{
		shortComplexTemp.x = (	(int)(src1Ptr->x) * (int)(src2Ptr->x)
		             		  +	(int)(src1Ptr->y) * (int)(src2Ptr->y) ) >> 14;
		shortComplexTemp.y = (	(int)(src1Ptr->y) * (int)(src2Ptr->x)
		             		  -	(int)(src1Ptr->x) * (int)(src2Ptr->y) ) >> 14;
		*dstPtr = shortComplexTemp;
		src1Ptr++;
		src2Ptr++;
		dstPtr++;
		} while (src1Ptr != src1EndPtr);
	}
#endif

#if !defined C6X_ASM && !defined SH3_ASM && !defined C54_ASM && !defined DSP16K_ASM && !defined ARM_PICCOLO_ASM && !defined PPC_ASM 
Public	void
BlockReal2ComplexMacc	(int size,
						 int shift,
						 short*			src1Ptr,
						 ComplexShort*	src2Ptr,
						 ComplexLong*	dstPtr)
	/****************************************************************************/
	/*	This function perfoms block multiply / accumulte  of short and ComplexShort arrays	*/	
	/****************************************************************************/
	{
	short		*src1EndPtr = src1Ptr + size;
	ComplexLong	sum;

	/*C6x*/ ASSERT(size >= 6);
	/*C6x*/ ASSERT((size & 1)==0);
	/*C54*/ ASSERT(shift  <= 16);

	ASSERT(size > 0);
	
	sum.x = 0;
	sum.y = 0;
	
	ASSERT((size & 7) == 0);
	do
		{
		sum.x += ((int)(*src1Ptr) * (int)(src2Ptr->x));
		sum.y += ((int)(*src1Ptr) * (int)(src2Ptr->y));
		src1Ptr++;
		src2Ptr++;
		} while (src1Ptr != src1EndPtr);

	dstPtr->x = sum.x >> shift;
	dstPtr->y = sum.y >> shift;
	}
#endif

#if !defined C6X_ASM && !defined C54_ASM
Public	int
BlockSum (int size, short* srcPtr)
	/****************************************************************************/
	/*	This function calculates sum of the short type array 			*/	
	/****************************************************************************/
	{
	int	sum = 0;
	short	*srcEndPtr = srcPtr + size;

	WARN((size & 3) == 0);
	
	ASSERT(size > 0);

	do
		{
		sum += (int)(*srcPtr++);
		} while (srcPtr != srcEndPtr);
	return	sum;
	}
#endif

Public	int
BlockByteSum (int size, uchar* srcPtr)
	/****************************************************************************/
	/*	This function calculates sum of the short type array 			*/	
	/****************************************************************************/
	{
	int	sum = 0;
	uchar	*srcEndPtr = srcPtr + size;

	ASSERT((size & 3) == 0);
	
	ASSERT(size > 0);

	do
		{
		sum += (int)(*srcPtr++);
		} while (srcPtr != srcEndPtr);
	return	sum;
	}

Public	void
BlockComplexSum (int size, ComplexShort* srcPtr, ComplexLong *result)
	/****************************************************************************/
	/*	This function calculates sum of the complex array. The result is placed */
	/*  in *result																*/	
	/****************************************************************************/
	{
	int			sumX = 0, 
					sumY = 0;
	ComplexShort	*srcEndPtr = srcPtr + size;

	ASSERT((size & 1) == 0);
	ASSERT(size > 0);
	
	do
		{
		sumX += (int)(srcPtr->x);
		sumY += (int)(srcPtr->y);
		} while (++srcPtr != srcEndPtr);
	result->x = sumX;
	result->y = sumY;
	}

#if !defined C6X_ASM && !defined C54_ASM && !defined DSP16K_ASM && !defined PPC_ASM
#ifdef BLOCKPOWER_40BIT
/* Special version of BlockPower code used for checksum testing against
 * assembly implementation where a 40-bit accumulator is used.  This is
 * slow and should not be used in production code.
 */
Public	int
BlockPower (int size, short* srcPtr)
	{
	VeryLong	power1, power2;
	short	*srcEndPtr = srcPtr + size;

	ASSERT((size & 1) == 0);
	ASSERT(size > 0);

	power1.x0 = 0;
	power1.x1 = 0;
	power1.x2 = 0;
	power1.x3 = 0;

	power2.x0 = 0;
	power2.x1 = 0;
	power2.x2 = 0;
	power2.x3 = 0;

	ASSERT((size & 1) == 0);

	do
		{
		power1 = VLAddLong(power1, (int)srcPtr[0] * (int)srcPtr[0]);
#ifdef BLOCKPOWER_40BIT_SINGLE_ACCUMULATOR
		power1 = VLAddLong(power1, (int)srcPtr[1] * (int)srcPtr[1]);
#else
		power2 = VLAddLong(power2, (int)srcPtr[1] * (int)srcPtr[1]);
#endif
		srcPtr += 2;
		
		} while (srcPtr != srcEndPtr);

	return	(((power1.x0 >> 4) | (power1.x1 << 12)) | ((power1.x2 & 0xf) << 28)) +
			((power2.x0 >> 4) | (power2.x1 << 12) | ((power2.x2 & 0xf) << 28));
	}
#else
#ifndef SH3_ASM
Public	int
BlockPower (int size, short* srcPtr)
	/****************************************************************************/
	/*	This function calculates average power of the short type array 			*/	
	/*	The result is Q-4 (it should be preserved across)						*/
	/****************************************************************************/
	{
	int	power = 0;
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 1) == 0);
	ASSERT(size > 0);
	
	do
		{
		power += ((int)(*srcPtr) >> 2) * ((int)(*srcPtr) >> 2);

		WARN(power >= 0);
		} while (++srcPtr != srcEndPtr);

	return	power;
	}
#endif
#endif
#endif

#if !defined C6X_ASM && !defined SH3_ASM && !defined C54_ASM && !defined PPC_ASM
Public	int
BlockFullPower (int size, short* srcPtr)
	/****************************************************************************/
	/*	This function calculates average power of the short type array 			*/	
	/*	The result is Q-4 (it should be preserved across)						*/
	/****************************************************************************/
	{
	int	power = 0;
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 1) == 0);
	/*C6x*/ ASSERT(size >= 2);
	ASSERT(size > 0);
	
	do
		{
		power += ((int)(*srcPtr)) * ((int)(*srcPtr));

		WARN(power >= 0);
		} while (++srcPtr != srcEndPtr);
	return	power;
	}
#endif

#if !defined C6X_ASM && !defined SH3_ASM && !defined C54_ASM
Public	int
BlockCorrelate (int size, short* src1Ptr, short* src2Ptr)
	/****************************************************************************/
	/*	This function calculates correlation of the short type array 			*/	
	/****************************************************************************/
	{
	int	correlation = 0;
	short	*src1EndPtr = src1Ptr + size;
	
	ASSERT((size & 1) == 0);
	ASSERT(size >= 4);
	/*C6x*/ ASSERT(size >= 4);	
	ASSERT(size > 0);
	
	do
		{
		correlation += ((int)(*(src1Ptr + 0)) >> 2) * ((int)(*(src2Ptr + 0)) >> 2);
		correlation += ((int)(*(src1Ptr + 1)) >> 2) * ((int)(*(src2Ptr + 1)) >> 2);
		src1Ptr += 2;
		src2Ptr += 2;
		} while (src1Ptr != src1EndPtr);

	return	correlation;
	}
#endif

Public int
BlockSymbol2Byte (int size, int bitsPerSymbol, ushort *srcPtr, uchar *dstPtr)
	/***********************************************************
	*
	* Description:
	*	This function performs data format conversion.
	*	It takes an array of symbols (each symbol is
	*	represented by a 16bit data word with symbol bits aligned 
	*	to the LSB ) and converts it into bytes array. 
	*	It is essentual that the total number of input bits 
	*	should be a multiple of 8
	*	for proper operation of the function.
	*	The function returns the number of bytes writted to the
	*	output buffer.
	*
	************************************************************/ 
	{
	ushort	*srcEndPtr = srcPtr + size;
	int	bitCount = 0,
			byteCount = 0;
	ushort	bitsDelayLine = 0;
	
	ASSERT(size > 0);

	do
		{
		bitsDelayLine  |= ((*srcPtr++) << bitCount);
		bitCount += bitsPerSymbol;
		if (bitCount >= 8)
			{
			*dstPtr++ = (int)bitsDelayLine;
			bitsDelayLine >>= 8;
			bitCount -= 8;
			byteCount++;
			}
		} while (srcPtr != srcEndPtr);
	return byteCount;
	}

Public int
BlockByte2Symbol (int byteCount, int bitsPerSymbol, uchar *srcPtr, ushort *dstPtr)
	/*******************************************************
	*
	* Description:
	*	This function performs data format conversion.
	*	It takes byte array and converts it into baud
	*	array: 16bit words array, where each element corresponds
	*	to one modulation symbol with data aligned to the LSB.
	*	It is essentual that the total number of input bits 
	*	(e.g. 8*byteCount) should be a multiple of globalVar.bitsPerSymbol
	*	for proper operation of the function.
	*	It returns the size of the output symbols array
	*
	********************************************************/ 
	{

	int	symbolCount, 
			bitCount;
	ushort	bitsDelayLine,
			mask;

	ASSERT(byteCount > 0);

	symbolCount = 0;
	mask = (1 << bitsPerSymbol) - 1;
	bitsDelayLine = *srcPtr++;
	bitsDelayLine |= ((ushort)(*srcPtr++) << 8 );
	bitCount = 16;

	do
		{
		*dstPtr++ = (bitsDelayLine & mask);
		bitsDelayLine >>= bitsPerSymbol;
		bitCount -= bitsPerSymbol;
		symbolCount++;
		if (bitCount <= 8)
			{
			bitsDelayLine |= ((ushort)(*srcPtr++) << bitCount);
			bitCount += 8;
			byteCount--;
			}
		} while (byteCount);
	return symbolCount;	
	}

#ifndef C6X_ASM
Public void
BlockMapShort2Complex (int size, ushort *srcPtr, ComplexByte *mapPtr, ComplexShort *dstPtr)
	/*******************************************************
	*
	* Description:
	*	This function performs signal element coding.
	*	It takes input data array and maps it into signal
	*	element space. 
	*
	********************************************************/ 
	{
	ushort			*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 3) == 0);
	ASSERT(size > 0);
	
	do
		{
		dstPtr->x = mapPtr[*srcPtr].x;
		dstPtr->y = mapPtr[*srcPtr].y;
		srcPtr++;
		dstPtr++;
		} while (srcPtr != srcEndPtr);
	}
#endif

Public	int SM_DECL
Idle (void)
	{
	return 0;
	}
	
Public void
BlockCplxScaleComplexSymbols(
		int				nSymbols,
		int				symSize,
		int				scaleShift,
		ComplexShort*	scaleBuffer,
		ComplexShort*	srcPtr,
		ComplexShort*	dstPtr)
/******************************************************************************
 *	This function scales a block of complex symbols by the complex scale factors,
 *	each scale factor corresponds to one element of the symbol.
 ******************************************************************************/
	{
	ComplexShort	*srcEndPtr, *symbolEndPtr, temp;
	ComplexShort	*scalePtr;
	int			tempX, tempY, rnd;
	
	ASSERT((symSize & 1) == 0);		/* Pentium MMX requires mutiple of 2 */
	ASSERT(nSymbols > 0);
	
	srcEndPtr = srcPtr + nSymbols * symSize;
	do
		{
		symbolEndPtr = srcPtr + symSize;
		scalePtr = scaleBuffer;
        rnd = (scaleShift > 0) ? 1 << (scaleShift-1) : 0;
		do
			{
			tempX = ( (
				(int)(scalePtr->x) * (int)(srcPtr->x) -
				(int)(scalePtr->y) * (int)(srcPtr->y) + rnd) >> scaleShift);
			tempY = ( (
				(int)(scalePtr->x) * (int)(srcPtr->y) +
				(int)(scalePtr->y) * (int)(srcPtr->x) + rnd) >> scaleShift);

/* This overflow may produce unpredictable results depending on */
/* different C compilers and hardware platforms */

			WARN( tempX == (int)((short)tempX) );
			WARN( tempY == (int)((short)tempY) );

#ifdef PENTIUM_OVERFLOW

			if(tempX > 32767) tempX = 32767;
			if(tempX < -32767) tempX = -32768;
			if(tempY > 32767) tempY = 32767;
			if(tempY < -32767) tempY = -32768;

#endif

			temp.x = (short) tempX;
			temp.y = (short) tempY;

			*(dstPtr++) = temp;
			scalePtr++;
			} while (++srcPtr != symbolEndPtr);
		} while (srcPtr != srcEndPtr);
	}

Public void
BlockRealScaleComplexSymbols(
		int				nSymbols,
		int				symSize,
		uchar*			scaleShiftBuffer,
		short*			scaleBuffer,
		ComplexShort*	srcPtr,
		ComplexShort*	dstPtr)
/******************************************************************************
 *	This function scales a block of complex symbols by the real scale factors,
 *	each scale factor corresponds to one element of the symbol.
 ******************************************************************************/
	{
	ComplexShort	*srcEndPtr, *symbolEndPtr, temp;
	short			*scalePtr;
	uchar			*shiftPtr;
	int				scaleShift;
	int			tempX, tempY;
	
	ASSERT((symSize & 7) == 0);		/* Pentium MMX requires mutiple of 8 */
	ASSERT(nSymbols > 0);
	
	srcEndPtr = srcPtr + nSymbols * symSize;
	do
		{
		symbolEndPtr = srcPtr + symSize;
		scalePtr = scaleBuffer;
		shiftPtr = scaleShiftBuffer;
		do
			{
			scaleShift = (int)(*shiftPtr++);
			tempX = (((int)(*scalePtr) * (int)(srcPtr->x)) >> scaleShift);
			tempY = (((int)(*scalePtr) * (int)(srcPtr->y)) >> scaleShift);

			WARN( tempX == (int)((short)tempX) );
			WARN( tempY == (int)((short)tempY) );

			temp.x = (short) tempX;
			temp.y = (short) tempY;
			*(dstPtr++) = temp;
			scalePtr++;
			} while (++srcPtr != symbolEndPtr);
		} while (srcPtr != srcEndPtr);
	}

Public void
BlockCplxSymbolUpdateCplxScale(
		int				nSymbols,
		int				symSize,
		int				nCarrsUsed,
		uchar*			gainShiftPtr,
		ComplexShort*	inputPtr,
		ComplexShort*	coefPtr,
		ComplexShort*	coefLowPtr,
		ComplexShort*	errorPtr)
/***************************************************************************
 *
 *	Description:
 *		This function performs stochastic gradient update algorithm on the
 *		complex one-tap per carrier coefficients.
 *
 *	Parameters:
 *		nSymbols	-- number of input symbols
 *		symSize		-- number of carriers per symbol
 *		nCarrsUsed	-- number of carriers used
 *		gainShiftPtr -- update gain shift buffer, different gain for each carrier
 *		inputPtr	-- input data delay line pointer
 *		coefPtr		-- filter coefficient buffer
 *		coefLowPtr	-- filter coefficient 16 LSBs buffer
 *		errorPtr	-- error buffer pointer
 *
 *	Return:
 *		void
 *
 ****************************************************************************/
	{
	ComplexShort	*coefEndPtr = coefPtr + nCarrsUsed;
	int				totalInputs = nSymbols * symSize, gainShift, gainShift1;
	int			roundOff, cx, cy;
	ComplexShort	*bp1, *bp2, *bp3;
	
	ASSERT(nSymbols > 0);

	do
		{
		gainShift = (int)(*(gainShiftPtr++));
		gainShift1 = gainShift - 16;
		cx = 0;
		cy = 0;
		bp1 = inputPtr;
		bp2 = errorPtr;
		bp3 = bp2 + totalInputs;
		do
			{
			cx  += (int)(bp1->x) * (int)(bp2->x)
			     + (int)(bp1->y) * (int)(bp2->y);
			cy  += (int)(bp1->x) * (int)(bp2->y)
			     - (int)(bp1->y) * (int)(bp2->x);
			bp1 += symSize;
			bp2 += symSize;
			} while (bp2 != bp3);
		
		if(gainShift1 >= 0)
			{
			cx += (int)((uint)coefLowPtr->x << gainShift1);
			cy += (int)((uint)coefLowPtr->y << gainShift1);
			coefLowPtr->x = cx >> gainShift1;
			coefLowPtr->y = cy >> gainShift1;			
			}
		else
			{
			cx += (int)((uint)coefLowPtr->x >> (-gainShift1));
			cy += (int)((uint)coefLowPtr->y >> (-gainShift1));
			coefLowPtr->x = cx << (-gainShift1);
			coefLowPtr->y = cy << (-gainShift1);	
			}		

		roundOff = (int)1 << (gainShift - 1);		
		coefPtr->x += (cx+roundOff) >> gainShift;
		coefPtr->y += (cy+roundOff) >> gainShift;			

		inputPtr++;
		errorPtr++;
		coefLowPtr++;
		} while (++coefPtr != coefEndPtr);
	
	}


Public void
BlockRealScaleCplxSymbols(
		int				nSymbols,
		int				symSize,
		int				scaleShift,
		short*			scaleBuffer,
		ComplexShort*	srcPtr,
		ComplexShort*	dstPtr)
/******************************************************************************
 *	This function scales a block of complex symbols by the real scale factors,
 *	each scale factor corresponds to one element of the symbol.
 ******************************************************************************/
	{
	ComplexShort	*srcEndPtr, *symbolEndPtr, temp;
	short			*scalePtr;
	int			tempX, tempY;
	
	ASSERT((symSize & 7) == 0);		/* Pentium MMX requires mutiple of 8 */
	ASSERT(nSymbols > 0);
	
	srcEndPtr = srcPtr + nSymbols * symSize;
	do
		{
		symbolEndPtr = srcPtr + symSize;
		scalePtr = scaleBuffer;
		do
			{
			tempX = (((int)(*scalePtr) * (int)(srcPtr->x)) >> scaleShift);
			tempY = (((int)(*scalePtr) * (int)(srcPtr->y)) >> scaleShift);

			WARN( tempX == (int)((short)tempX) );
			WARN( tempY == (int)((short)tempY) );

			temp.x = (short) tempX;
			temp.y = (short) tempY;
			*(dstPtr++) = temp;
			scalePtr++;
			} while (++srcPtr != symbolEndPtr);
		} while (srcPtr != srcEndPtr);
	}

Public	void
BlockCplxLongConjigateMultCplxShort	(int size,
							 ComplexLong* src1Ptr,
							 ComplexShort* src2Ptr,
							 ComplexLong* dstPtr)
	/****************************************************************************/
	/*	This function perfoms block complex conjigate multiply of two arrays	*/	
	/****************************************************************************/
	{
	ComplexLong	*src1EndPtr = src1Ptr + size;
	int		tempX0, tempX1, tempY0, tempY1, temp0, temp1;

	ASSERT(size > 0);

	do
		{
		tempX0 = (src1Ptr->x) & 0x00007FFF;		/* modified for efficient Pentium implementation */
		tempX1 = (src1Ptr->x) >> 15;			/* bit exact as before */
		tempY0 = (src1Ptr->y) & 0x00007FFF;
		tempY1 = (src1Ptr->y) >> 15;
		
		temp0 = tempX0 * (int)(src2Ptr->x) + tempY0 * (int)(src2Ptr->y);
		temp1 = tempX1 * (int)(src2Ptr->x) + tempY1 * (int)(src2Ptr->y);
		dstPtr->x = (temp0 >> 12) + (temp1 << 3);
		
		temp0 = tempY0 * (int)(src2Ptr->x) - tempX0 * (int)(src2Ptr->y);
		temp1 = tempY1 * (int)(src2Ptr->x) - tempX1 * (int)(src2Ptr->y);
		dstPtr->y = (temp0 >> 12) + (temp1 << 3);
		
		src1Ptr++;
		src2Ptr++;
		dstPtr++;
		} while (src1Ptr != src1EndPtr);
	}


Public	void
BlockShortScaleByShift (int size, int shift, short* srcPtr, short* dstPtr)
	/****************************************************************************/
	/*This function perfoms block scaling in factor of 2 of the short type array*/	
	/****************************************************************************/
	{
	short	*srcEndPtr = srcPtr + size;
	
	ASSERT((size & 7) == 0);
	ASSERT(size > 0);
	
	if ((shift == 0) && (srcPtr == dstPtr))
		;										/* do nothing */
	
	else if (shift == 0)
		{
		do										/* move only */
			{
			*dstPtr++ = *srcPtr++;
			} while (srcPtr != srcEndPtr);
		}
	
	else if (shift > 0)
		{
		do										/* shift to right */
			{
			*dstPtr++ = (*srcPtr++) >> shift;
			} while (srcPtr != srcEndPtr);
		}
	
	else if (shift < 0)
		{
		do										/* shift to left */
			{
			*dstPtr++ = (*srcPtr++) << (-shift);
			} while (srcPtr != srcEndPtr);
		}

	return;
	}
