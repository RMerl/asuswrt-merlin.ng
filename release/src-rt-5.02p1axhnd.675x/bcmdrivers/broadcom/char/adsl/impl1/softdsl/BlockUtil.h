/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/
/* BlockUtil.h
 *
 *	Description:
 *		This file contains the interfaces for the fixed point block
 *		processing utilities.
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.23 $
 *
 * $Id: BlockUtil.h,v 1.23 2004/04/13 00:31:10 ilyas Exp $
 *
 * $Log: BlockUtil.h,v $
 * Revision 1.23  2004/04/13 00:31:10  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.22  2003/07/11 01:49:01  gsyu
 * Added BlockShortClearByLong to speed up performance
 *
 * Revision 1.21  2003/07/10 22:35:23  gsyu
 * Speed up BlockByteXXX performance
 *
 * Revision 1.20  2003/07/10 22:15:51  gsyu
 * Added BlockByteMoveByWord to speed up performance
 *
 * Revision 1.19  2002/03/12 00:03:03  yongbing
 * Modify cplxScaleCplxSymbols to accept a shift value instead of an array of shifts
 *
 * Revision 1.18  2001/03/14 00:50:25  georgep
 * All targets use FEQ_PASS_FFTSHIFT, remove code for case where its not defined
 *
 * Revision 1.17  2000/11/30 03:54:09  khp
 * -BlockRealScaleCplxSymbols instead of BlockScaleComplexSymbols
 *
 * Revision 1.16  2000/11/29 20:42:12  liang
 * Add function for ADSL xmt gains with fixed shift.
 *
 * Revision 1.15  2000/10/02 19:24:08  georgep
 * Modify FEQ for new fft, fft outputs a shift for each block
 *
 * Revision 1.14  2000/09/09 00:23:48  liang
 * Add corresponding functions for the ComplexLong FEQ coef.
 *
 * Revision 1.13  2000/05/17 01:36:52  yongbing
 * Add Pentium MMX assembly codes for more block related functions
 *
 * Revision 1.12  2000/04/19 19:22:22  yongbing
 * Add BlockShortScaleby2 function used in G994p1
 *
 * Revision 1.11  2000/04/04 02:28:01  liang
 * Merged with SoftDsl_0_2 from old tree.
 *
 * Revision 1.11  2000/03/14 23:29:01  yongbing
 * Add Pentim MMX codes for BlockCplxSymbolUpdateCplxScale function
 *
 * Revision 1.10  2000/02/16 01:53:00  yongbing
 * Add Pentium MMX module for FEQ
 *
 * Revision 1.9  1999/11/02 02:49:55  liang
 * Add BlockComplexPower function.
 *
 * Revision 1.8  1999/08/05 19:42:34  liang
 * Merged with the softmodem top of the tree on 08/04/99 for assembly files.
 *
 * Revision 1.7  1999/06/16 00:54:39  liang
 * BlockRealScaleComplexSymbols takes a scale shift buffer now.
 *
 * Revision 1.6  1999/05/22 02:18:29  liang
 * Add one more parameter to BlockCplxSymbolUpdateCplxScale function.
 *
 * Revision 1.5  1999/05/14 22:49:39  liang
 * Added two more functions.
 *
 * Revision 1.4  1999/03/26 03:29:57  liang
 * Add function BlockComplexMultLongAcc.
 *
 * Revision 1.3  1999/02/22 22:40:59  liang
 * BlockByteSum takes uchar inputs instead of schar.
 *
 * Revision 1.2  1999/02/10 01:56:44  liang
 * Added BlockByteSum, BlockRealScaleComplexSymbols and BlockCplxScaleComplexSymbols.
 *
 * Revision 1.1  1998/10/28 01:35:38  liang
 * *** empty log message ***
 *
 * Revision 1.12  1998/07/08 17:09:25  scott
 * Removed unnecessary undefs
 *
 * Revision 1.11  1998/04/02 06:19:44  mwg
 * Added two new utilities.
 *
 * Revision 1.10  1998/03/26 23:20:55  liang
 * Added function BlockShortMultiply.
 *
 * Revision 1.9  1998/02/16  18:41:00  scott
 * Added MMX autodetect support
 *
 * Revision 1.8  1997/12/13 06:11:35  mwg
 * Added new functions:
 * BlockLongSubtract()
 * BlockLongAdd()
 * BlockLong2ShortSubtract()
 * BlockShort2LongMove()
 * BlockShortInterpolate()
 * BlockLongCorrelate()
 * BlockMapShort2Short()
 *
 * Revision 1.7  1997/03/19 18:35:10  mwg
 * Changed copyright notice.
 *
 * Revision 1.6  1997/02/11  00:08:18  mwg
 * Added BlockByteMove function
 *
 * Revision 1.5  1997/02/04  08:40:08  mwg
 * Changed interface forBlockReal2ComplexMacc()
 *
 * Revision 1.4  1997/01/23  02:04:28  mwg
 * Added return value to BlockShortMove
 *
 * Revision 1.3  1996/12/19  22:34:55  mwg
 * Added new function BlockFullPower().
 *
 * Revision 1.2  1996/02/21  03:59:15  mwg
 * Added new function BlockReal2ComplexMacc
 *
 * Revision 1.1.1.1  1996/02/14  02:35:13  mwg
 * Redesigned the project directory structure. Merged V.34 into the project.
 *
 * Revision 1.5  1995/04/04  06:09:32  mwg
 * Changed the SoftModem status reporting: now the status is a structure/union
 * where different fields used for different status code. This will enable
 * efficient status snooping for high level protocols on top of the softmodem.
 *
 */

#ifndef	BlockUtilPh
#define	BlockUtilPh

extern	void	BlockLongAdd				(int, int*, int*, int*);
extern	void	BlockLong2ShortSubtract		(int, int*, int*, short*);
extern	void	BlockShort2LongMove			(int, short*, int*);
extern	void	BlockShortMultiply			(int, int, short*, short*, short*);
extern	void	BlockByteMoveUnaligned		(int size, uchar *srcPtr, uchar *dstPtr);
extern	void	BlockShortOffset			(int, short, short*, short*);
extern	int	BlockShortInterpolateWithIncrement (int size, int scaleFactor, int increment, int shift, short* src1Ptr, short* src2Ptr, short* dstPtr);
extern	void	BlockReal2ComplexMult		(int, short*, ComplexShort*, ComplexShort*);
extern	void	BlockComplexConjigateMult	(int, ComplexShort*, ComplexShort*, ComplexShort*);

extern	int	BlockSum					(int, short*);
extern	int	BlockByteSum				(int, uchar*);
extern	void	BlockComplexSum				(int, ComplexShort*, ComplexLong*);
extern	void	BlockComplexPower			(int, int, ComplexShort*, int*);
extern	int	BlockFullPower				(int, short*);
extern	int	BlockLongCorrelate			(int, int*, int*);

extern	int		BlockSymbol2Byte			(int, int, ushort*, uchar*);
extern	int		BlockByte2Symbol			(int, int, uchar*, ushort*);

extern	void	BlockMapShort2Complex		(int, ushort*, ComplexByte*, ComplexShort*);
extern	void	BlockMapShort2Short			(int size, ushort *srcPtr, short *mapPtr, short *dstPtr);
extern	void	BlockMapByte2Byte			(int size, uchar *srcPtr, uchar *mapPtr, uchar *dstPtr);
extern	void	BlockMapByte2Short			(int size, uchar *srcPtr, short *mapPtr, short *dstPtr);
extern	void	BlockShortMult				(int size, int shift, short* src1Ptr, short* src2Ptr, short* dstPtr);

extern	int		SM_DECL Idle(void);

extern	void	BlockGenerateAngles(int size, ComplexShort *anglePtr, ComplexShort *incPtr, ComplexShort *dstPtr);
extern	void	BlockExtractRealPart(int size, ComplexShort *srcPtr, short *dstPtr);
extern	void	BlockShortScaleByShift (int size, int shift, short* srcPtr, short* dstPtr);

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
extern  void *BlockMemCopy(void *dst, void *src, int size, int nAlign);

#define BlockByteMoveSrcAlign(len,src,dst)		BlockMemCopy(dst,src,len, (-((long) (src))) & 0x7)
#define BlockByteMoveDstAlign(len,src,dst)		BlockMemCopy(dst,src,len, (-((long) (dst))) & 0x7)
#define BlockByteMove(len,src,dst)				BlockByteMoveSrcAlign(len,src,dst)

#define BlockShortMoveSrcAlign(len,src,dst)		BlockByteMoveSrcAlign((len) << 1,src,dst)
#define BlockShortMoveDstAlign(len,src,dst)		BlockByteMoveDstAlign((len) << 1,src,dst)
#define BlockShortMove(len,src,dst)				BlockShortMoveSrcAlign(len,src,dst)

#define BlockLongMoveSrcAlign(len,src,dst)		BlockByteMoveSrcAlign((len) << 2,src,dst)
#define BlockLongMoveDstAlign(len,src,dst)		BlockByteMoveDstAlign((len) << 2,src,dst)
#define BlockLongMove(len,src,dst)				BlockLongMoveSrcAlign(len,src,dst)

#else

extern	void	BlockByteMove				(int, uchar*, uchar*);
extern	void	BlockLongMove				(int, int*, int*);
extern	int		SM_DECL BlockShortMove				(int, short*, short*);

#define BlockByteMoveSrcAlign(len,src,dst)		BlockByteMove(len,src,dst)
#define BlockByteMoveDstAlign(len,src,dst)		BlockByteMove(len,src,dst)

#define BlockShortMoveSrcAlign(len,src,dst)		BlockShortMove(len,src,dst)
#define BlockShortMoveDstAlign(len,src,dst)		BlockShortMove(len,src,dst)

#define BlockLongMoveSrcAlign(len,src,dst)		BlockLongMove(len,src,dst)
#define BlockLongMoveDstAlign(len,src,dst)		BlockLongMove(len,src,dst)

#endif

#ifndef PENTIUM_REDEFS /* only if these have not been redefined to function pointers */
extern	int	BlockPower					(int, short*);
extern	void	BlockReal2ComplexMacc		(int, int, short*, ComplexShort*, ComplexLong*);
extern	void	BlockComplexMult			(int, ComplexShort*, ComplexShort*, ComplexShort*);
extern	void	BlockShortScale				(int, short, int, short*, short*);
extern	int	BlockCorrelate				(int, short*, short*);

extern	void	BlockRealScaleComplexSymbols(int, int, uchar*, short*, ComplexShort*, ComplexShort*);
/* FIXME -- the following 3 functions can be removed */
extern	void	BlockCplxScaleComplexSymbols(int, int, int, ComplexShort*, ComplexShort*, ComplexShort*);
extern	void	BlockCplxSymbolUpdateCplxScale(int, int, int, uchar*, ComplexShort*,
					ComplexShort*, ComplexShort*, ComplexShort*);
extern	void	BlockComplexShortFill		(int, short, short, ComplexShort*);


extern	void	BlockRealScaleCplxSymbols(int, int, int, short*, ComplexShort*, ComplexShort*);
extern	void	BlockCplxLongConjigateMultCplxShort(int, ComplexLong*, ComplexShort*, ComplexLong*);

extern	void	BlockCplxLongScaleCplxSymbols(int, int, int, ComplexLong*, ComplexShort*, short*, ComplexShort*);
extern	void	BlockCplxSymbolUpdateCplxLongScale(int, int, int, int,
					ComplexShort*, short *, ComplexLong*, ComplexShort*);

extern	void	BlockComplexLongFill		(int, int, int, ComplexLong*);

extern	void	BlockShortSubtract			(int, short*, short*, short*);
extern	void	BlockLongSubtract			(int, int*, int*, int*);
extern	void	BlockShortAdd				(int, short*, short*, short*);
extern	void	BlockByteMoveByLong			(int, uchar*, uchar*);
extern	void	SM_DECL BlockByteFill		(int, uchar, uchar*);
extern	void	BlockByteFillByLong 		(int, uchar, uchar*);
extern	void	BlockByteClear				(int, uchar*);
extern	void	BlockByteClearByLong		(int, uchar*);
extern	void	BlockShortFill				(int, short, short*);
extern	void	BlockShortClear				(int, short*);
extern	void	BlockShortClearByLong		(int, short*);
extern	void	BlockLongFill				(int, int, int*);
extern	void	BlockLongClear				(int, int*);
extern	void	BlockComplexShortClear		(int, ComplexShort*);
extern	void	BlockShortInvert			(int, short*, short*);
extern	void	BlockShortScaleDown			(int, short*);
extern	void	BlockShortInterpolate		(int, short, int, short*, short*, short*);
extern	void	BlockComplexMultLongAcc     (int, int, ComplexShort*, ComplexShort*, ComplexLong*);

#if defined(ADSLDRV_LITTLE_ENDIAN) && (defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64))

extern  void *BlockShortRev(int size, void *src, void *dst, int nAlign);
extern  void *BlockLongRev(int size, void *src, void *dst, int nAlign);

#define BlockShortRevSrcAlign(len,src,dst)		BlockShortRev(len,src,dst, (-((long) (src))) & 0x7)
#define BlockShortRevDstAlign(len,src,dst)		BlockShortRev(len,src,dst, (-((long) (dst))) & 0x7)
#define	BlockShortMoveReverse(len,src,dst)		BlockShortRevSrcAlign(len,src,dst)

#define BlockLongRevSrcAlign(len,src,dst)		BlockLongRev(len,src,dst, (-((long) (src))) & 0x7)
#define BlockLongRevDstAlign(len,src,dst)		BlockLongRev(len,src,dst, (-((long) (dst))) & 0x7)
#define	BlockLongMoveReverse(len,src,dst)		BlockLongRevSrcAlign(len,src,dst)

#else
#define	BlockLongMoveReverse(sz, src, dst)	BlockByteMove((sz)<<2, (void*)(src), (void*)(dst))
#define	BlockShortMoveReverse(sz, src, dst)	BlockByteMove((sz)<<1, (void*)(src), (void*)(dst))
#endif
#endif
#endif	/* BlockUtilPh */
