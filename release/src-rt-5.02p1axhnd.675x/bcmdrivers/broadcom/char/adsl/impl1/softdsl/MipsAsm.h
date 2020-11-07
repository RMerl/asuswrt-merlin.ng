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
 *	MipsAsm.h:
 *
 *	Description:
 *	This file contains definitions specific to MIPS assembly 
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.7 $
 *
 * $Id: MipsAsm.h,v 1.7 2009/03/18 04:26:15 ilyas Exp $
 *
 * $Log: MipsAsm.h,v $
 * Revision 1.7  2009/03/18 04:26:15  ilyas
 * Chnage break code to distinguish from the one used by gcc for zero divide
 *
 * Revision 1.6  2007/01/04 21:05:20  ilyas
 * Optimized error checking in MIPS assembly functions
 *
 * Revision 1.5  2004/04/13 00:16:59  ilyas
 * Merged the latest ADSL driver changes
 *
 * Revision 1.4  2002/09/12 04:08:50  ilyas
 * Added macros for BCM MIPS specific instructions
 *
 * Revision 1.3  2000/11/18 21:28:19  mprahlad
 * ifdef bcm47xx -
 * define MSUB(src1,src2) msub src1, src2
 * change Mult(dst, src1, src2) to use "mul" instead of "mult; mflo"
 * define Mul(src1, src2) mult src1, src2
 *
 * Revision 1.2  2000/07/28 21:05:05  mprahlad
 * Macros specific to bcm47xx added.
 *
 * Revision 1.1  1999/08/05 19:52:57  liang
 * Copied from the softmodem top of the tree on 08/04/99.
 *
 * Revision 1.5  1999/04/02 23:16:21  mwg
 * Fixed a minor comatibility issue with mult
 *
 * Revision 1.4  1999/02/03 20:25:43  mwg
 * Added an option for R4010
 *
 * Revision 1.3  1998/10/30 02:21:34  mwg
 * Added targets for 4640
 *
 * Revision 1.2  1998/10/16 18:52:09  ilyas
 * Added ASM_PROLOG[5-7] macros to save on stores
 *
 * Revision 1.1  1998/06/03 23:28:39  mwg
 * Renamed from DinoDefs.h
 *
 * Revision 1.6  1998/02/09  18:23:11  scott
 * Added EMBEDDED_CALLING_CONVENTION (GreenHill) and R3900/R4102
 *
 * Revision 1.5  1997/03/19 18:35:02  mwg
 * Changed copyright notice.
 *
 * Revision 1.4  1996/10/02  20:28:41  liang
 * Remove parameter "acc" from the non-DINO version of MAD.
 *
 * Revision 1.3  1996/10/02  19:44:36  liang
 * Separated MultAdd into MAD and MADW, added NO_DINO_WRITEBACK option.
 *
 * Revision 1.2  1996/08/14  03:06:07  liang
 * Modified macro MultAdd so that the assembly code build works.
 *
 * Revision 1.1.1.1  1996/02/14  02:35:13  mwg
 * Redesigned the project directory structure. Merged V.34 into the project.
 *
 * Revision 1.5  1994/11/04  22:41:29  mwg
 * Added #ifdefs for different targets.
 *
 ************************************************************************/

#ifndef _MIPS_ASM_H_
#define	_MIPS_ASM_H_

#define zero	$0
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16	
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27
#define gp	$28
#define sp	$29
#define fp	$30
#define s8	$30
#define ra	$31

#ifdef EMBEDDED_CALLING_CONVENTION

/* Support for GreenHills embedded calling convention */

#define ASM_PROLOG	subu	sp, 32; \
					sw		$8, 16(sp); \
					sw		$9, 20(sp); \
					sw		$10, 24(sp); \
					sw		$11, 28(sp);

#define ASM_PROLOG5	subu	sp, 32; \
					sw		$8, 16(sp);

#define ASM_PROLOG6	subu	sp, 32; \
					sw		$8, 16(sp); \
					sw		$9, 20(sp);

#define ASM_PROLOG7	subu	sp, 32; \
					sw		$8, 16(sp); \
					sw		$9, 20(sp); \
					sw		$10, 24(sp);

#define ASM_EPILOG	addu	sp, 32

#else
#define ASM_PROLOG
#define ASM_PROLOG5
#define ASM_PROLOG6
#define ASM_PROLOG7
#define ASM_EPILOG
#endif

#ifdef	DINO	/* Special DSP extensions to MIPS core */

#ifndef	NO_DINO_WRITEBACK	/* DSP extensions with writeback register */

#define	MAD(src1, src2)			.set	noreorder	; mad	$0, src1, src2	; .set	reorder
#define	MADW(acc, src1, src2)	.set	noreorder	; mad	acc, src1, src2	; .set	reorder
#define	Mult(dst, src1, src2)	.set	noreorder	; mult	dst, src1, src2	; .set	reorder	
#define	MultU(dst, src1, src2)	.set	noreorder	; multu	dst, src1, src2	; .set	reorder		

#else	/* NO_DINO_WRITEBACK */

#define	MAD(src1, src2)			.set	noreorder	; mad	$0, src1, src2	; .set	reorder
#define	MADW(acc, src1, src2)	.set	noreorder	; mad	$0, src1, src2	; mflo acc ; .set	reorder
#define	Mult(dst, src1, src2)	multu	src1, src2 ; mflo dst
#define	MultU(dst, src1, src2)	multu	src1, src2 ; mflo dst		

#endif	/* NO_DINO_WRITEBACK */

#else	/* DINO */

#if defined(R3900)

#define	MAD(src1, src2)		madd	$0, src1, src2
#define	MADW(acc, src1, src2)	madd	acc, src1, src2
#define	Mult(dst, src1, src2)	mult	dst, src1, src2
#define	MultU(dst, src1, src2)	multu	dst, src1, src2

#elif defined(bcm47xx_INSTR_MACROS) && defined(bcm47xx)

#define mips_froo(s1,s2,s3)			s1##s2##s3
#define	MSUB(s1,s2)					.set noreorder ; mips_froo(msub_,s1,s2) ; .set reorder
#define MAD(s1,s2) 					.set noreorder ; mips_froo(mad_,s1,s2) ; .set reorder
#define MADW(acc, s1,s2)			.set noreorder ; mips_froo(mad_,s1,s2) ; mflo acc ; .set reorder

#include "BCM4710.h"

#define	Mult(dst, src1, src2)		mul		dst, src1, src2
#define	Mul( src1, src2)			mult	src1, src2 ; 
#define	MultU(dst, src1, src2)		multu	src1, src2	; mflo dst

#elif defined(bcm47xx)
#define	MSUB(src1, src2)			msub	src1, src2
#define	MAD(src1, src2)			madd	src1, src2
#define	MADW(acc, src1, src2)	.set noreorder ; madd	src1, src2; mflo acc ; .set reorder
/*
#define	Mult(dst, src1, src2)	mult	src1, src2 ; mflo dst
*/
#define	Mult(dst, src1, src2)	mul	dst , src1, src2 ; 
#define	Mul( src1, src2)	mult	src1, src2 ; 
#define	MultU(dst, src1, src2)	multu	src1, src2 ; mflo dst

#else

#ifdef R4102
#define	MAD(src1, src2)			madd16	src1, src2
#define	MADW(acc, src1, src2)	madd16	src1, src2	; mflo acc
#else /* R4102 */

#ifdef R4640

#define	MAD(src1, src2)			madd	$0, src1, src2
#define	MADW(acc, src1, src2)	madd	src1, src2; mflo acc

#else /* R4640 */

#ifdef R4010

#define	MAD(src1, src2)			madd	src1, src2
#define	MADW(acc, src1, src2)	madd	src1, src2; mflo acc

#else
#define	MAD(src1, src2)				.set	noat		;\
									mflo	$at			;\
									sw		$2,   -4(sp)	;\
									multu	src1, src2	;\
									mflo	$2			;\
									addu	$at, $2, $at	;\
									lw		$2,   -4(sp)	;\
									mtlo	$at		;\
									.set	at

#define	MADW(acc, src1, src2)		.set	noat		;\
									mflo	$at			;\
									sw		$2,   -4(sp)	;\
									multu	src1, src2	;\
									mflo	$2			;\
									addu	$at, $2, $at	;\
									lw		$2,   -4(sp)	;\
									move	acc, $at	;\
									mtlo	$at		;\
									.set	at
#endif /* R4010 */
#endif /* R4102 */
#endif /* R4640 */

#define	Mult(dst, src1, src2)		mul		dst, src1, src2
#define	MultU(dst, src1, src2)		multu	src1, src2	; mflo dst

#endif	/* !3900 */
#endif	/* DINO */


#define ASSERT_INTR					break	8

#endif	/* _MIPS_ASM_H_ */
