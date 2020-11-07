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

/****************************************************************************
 *
 * MiscUtil.h -- Miscellaneous utilities
 *
 * Description:
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg Haixiang Liang
 *
 * $Revision: 1.4 $
 *
 * $Id: MiscUtil.h,v 1.4 2004/04/13 00:21:46 ilyas Exp $
 *
 * $Log: MiscUtil.h,v $
 * Revision 1.4  2004/04/13 00:21:46  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.3  2001/07/21 01:21:06  ilyas
 * Added more functions for int to string conversion used by log file
 *
 * Revision 1.2  1999/08/05 19:42:56  liang
 * Merged with the softmodem top of the tree on 08/04/99 for assembly files.
 *
 * Revision 1.1  1999/01/27 22:10:12  liang
 * Initial version.
 *
 * Revision 1.1  1997/07/10 01:18:45  mwg
 * Initial revision.
 *
 *
 *
 *****************************************************************************/
#ifndef _MISC_UTIL_H_
#define _MISC_UTIL_H_

extern int		SM_DECL	GetRateValue(dataRateMap rate);
extern int 		SM_DECL	DecToString(uint value, uchar *dstPtr, uint nDigits);
extern int 		SM_DECL	HexToString(uint value, uchar *dstPtr, uint nDigits);
extern char *	SM_DECL	DecToStr(char *s, uint num);
extern char *	SM_DECL	SignedToStr(char *s, int num);
extern char *	SM_DECL	HexToStr(char *s, uint num);

#define	EvenParityBit(x)	((z = (y = x ^ (x >> 4)) ^ (y >> 2)) ^ (z >> 1))
#define	OddParityBit(x)		(EvenParityBit(x) ^ 1)

extern void	ParityApply(int nBytes, int nDataBits, int parity, uchar *srcPtr, uchar *dstPtr);
extern void	ParityStrip(int nBytes, int nDataBits, int parity, uchar *srcPtr, uchar *dstPtr, statusHandlerType	statusHandler);

#endif
