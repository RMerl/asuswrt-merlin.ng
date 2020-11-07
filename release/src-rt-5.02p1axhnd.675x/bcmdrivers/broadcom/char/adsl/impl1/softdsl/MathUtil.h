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

/************************************************************************
 *
 *	MathUtil.h:
 *
 *	Description:
 *	This file contains the exported interface for MathUtil.c module.
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.6 $
 *
 * $Id: MathUtil.h,v 1.6 2004/04/13 00:21:13 ilyas Exp $
 *
 * $Log: MathUtil.h,v $
 * Revision 1.6  2004/04/13 00:21:13  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.5  2001/08/16 02:18:08  khp
 * - mark functions with FAST_TEXT to reduce cycle counts for QPROC targets
 *   (replaces use of LMEM_INSN)
 *
 * Revision 1.4  1999/10/06 04:55:22  liang
 * Added function to multiply two long values to save result as VeryLong.
 *
 * Revision 1.3  1999/08/05 19:42:52  liang
 * Merged with the softmodem top of the tree on 08/04/99 for assembly files.
 *
 * Revision 1.2  1999/03/26 03:29:59  liang
 * Export CosSin table.
 *
 * Revision 1.1  1998/10/28 01:28:07  liang
 * *** empty log message ***
 *
 * Revision 1.12  1998/02/10  17:19:49  scott
 * Changed MathVL routines to return arguments using pointers
 *
 * Revision 1.11  1997/12/13 06:12:07  mwg
 * Added more Atan2 flavors
 *
 * Revision 1.10  1997/11/18 01:11:48  mwg
 * Removed <CR> symbols which accidently slipped in.
 *
 * Revision 1.9  1997/11/03  19:07:52  scott
 * No longer redefine max() and min() if already defined
 *
 * Revision 1.8  1997/07/30 01:35:20  liang
 * Add more accurate atan2 function UtilLongLongAtan2.
 *
 * Revision 1.7  1997/07/21  20:23:19  mwg
 * Added new function: UtilBlockCos()
 *
 * Revision 1.6  1997/03/21  23:50:10  liang
 * Added initial version of V8bis module to CVS tree.
 *
 * Revision 1.5  1997/03/19  18:35:34  mwg
 * Changed copyright notice.
 *
 * Revision 1.4  1997/01/21  00:36:15  mwg
 * Added new function: UtilBlockCosSin()
 *
 * Revision 1.3  1996/06/18  21:14:45  mwg
 * Modified VLDivVL by allowing to specify the result scaling.
 *
 * Revision 1.2  1996/06/12  02:31:59  mwg
 * Added 64bit arithmetic functions.
 *
 * Revision 1.1.1.1  1996/02/14  02:35:15  mwg
 * Redesigned the project directory structure. Merged V.34 into the project.
 *
 * Revision 1.4  1995/12/04  23:08:15  liang
 * Add file Math/LinearToLog.c.
 *
 ************************************************************************/
#ifndef	MathUtilPh
#define	MathUtilPh

/* Exported tables */
extern	const short		UtilCosTable[];

/* Exported functions */
extern	ComplexShort	UtilCosSin(ushort angle);
extern	int			UtilBlockCosSin (int nValues, int angle, int delta, ComplexShort *dstPtr);
extern	int			UtilBlockCos (int nValues, int angle, int delta, short *dstPtr);
extern	ushort			UtilShortShortAtan2(ComplexShort point);
extern	ushort			UtilLongShortAtan2(ComplexLong point);
extern	uint			UtilShortLongAtan2(ComplexShort point) FAST_TEXT;
extern	uint			UtilLongLongAtan2(ComplexLong point) FAST_TEXT;
extern	ushort			UtilSqrt(uint y);
extern	ushort			UtilMaxMagnitude(int blkSize, ComplexShort *dataPtr);
extern	short			UtilQ0LinearToQ4dB (uint x);
extern	uint			UtilQ4dBToQ12Linear (short x);
extern	void			UtilAdjustComplexMagnitude(ComplexShort	*srcPtr, short mag, short adjustment);

extern	void VLMultLongByLong(int x, int y, VeryLong *dst);
extern	void VLMultShort	(VeryLong x, short y, VeryLong *dst);
extern	void VLAddVL		(VeryLong x, VeryLong y, VeryLong *dst);
extern	void VLAddLong	(VeryLong x, int y, VeryLong *dst);
extern	void VLSubVL		(VeryLong x, VeryLong y, VeryLong *dst);
extern	void VLSubLong	(VeryLong x, int y, VeryLong *dst);
extern	void VLDivVL		(VeryLong x, VeryLong y, int scale, int *dst);
extern	void VLShiftLeft(VeryLong x, int shift, VeryLong *dst);
extern	void VLShiftRight(VeryLong x, int shift, VeryLong *dst);


#define	UtilAtan2		UtilShortShortAtan2
#define	UtilLongAtan2	UtilLongShortAtan2

/* Standard Macros	*/
#undef abs
#define		abs(x)			((x) >= 0   ? (x) : -(x))

#undef max
#define		max(x, y)		((x) >= (y) ? (x) : (y))

#undef min
#define		min(x, y)		((x) <= (y) ? (x) : (y))

#endif	/* MathUtilPh */
