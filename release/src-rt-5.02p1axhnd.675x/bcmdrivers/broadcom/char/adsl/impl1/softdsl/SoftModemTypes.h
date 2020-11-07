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
 * SoftModemTypes.h 
 *
 *
 * Description:
 *	This file contains some of the type declarations for SoftModem
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.9 $
 *
 * $Id: SoftModemTypes.h,v 1.9 2004/04/13 00:16:59 ilyas Exp $
 *
 * $Log: SoftModemTypes.h,v $
 * Revision 1.9  2004/04/13 00:16:59  ilyas
 * Merged the latest ADSL driver changes
 *
 * Revision 1.8  2004/01/24 01:35:33  ytan
 * add multi-section lmem swap
 *
 * Revision 1.7  2001/09/21 19:19:01  ilyas
 * Minor fixes for VxWorks build
 *
 * Revision 1.6  2001/08/16 02:16:39  khp
 * - added definitions for SLOW_DATA and FAST_TEXT, defined to nothing
 *   except when bcm47xx && USE_SLOW_DATA or USE_FAST_TEXT.  Any function
 *   that needs to run fast should be marked with FAST_TEXT.  Any data that
 *   is not referenced often should be marked with SLOW_DATA.
 *
 * Revision 1.5  2001/03/30 00:49:59  liang
 * Changed warning output message.
 *
 * Revision 1.4  2000/06/21 22:24:40  yongbing
 * Modify WARN micro to limit the number of same warnings printed
 *
 * Revision 1.3  1999/08/05 20:02:13  liang
 * Merged with the softmodem top of the tree on 08/04/99.
 *
 * Revision 1.2  1999/01/27 22:14:29  liang
 * Merge with SoftModem_3_1_02.
 *
 * Revision 1.19  1998/11/17 04:02:39  yura
 * Fixed WARN and ASSERT redefinition warning for WinNT targets
 *
 * Revision 1.18  1998/08/26 19:20:43  scott
 * Commented out EXCLUDE_CYGWIN32_TYPES define
 *
 * Revision 1.17  1998/08/13 19:03:06  scott
 * Added BitField definition and INT_IS_LONG
 *
 * Revision 1.16  1998/08/08 03:39:55  scott
 * The DEBUG_PTR_ENABLED macro can be used to enable only the DEBUG_PTR macros
 *
 * Revision 1.15  1998/07/28 22:21:31  mwg
 * Fixed problems with NULL & nil being defined incorrectly
 *
 * Revision 1.14  1998/07/08 17:09:17  scott
 * Define ASSERT and WARN only if not already defined
 *
 * Revision 1.13  1998/07/02 20:46:34  scott
 * Added workaround for building certain builds with older SunOS
 *
 * Revision 1.12  1998/02/09 18:24:49  scott
 * Defined "Private" as nothing for GreenHill (to prevent erroneous section
 * allocations for data)
 *
 * Revision 1.11  1997/08/29 21:39:24  scott
 * Added check for LONG_IS_INT define (for TI C6X support)
 *
 * Revision 1.10  1997/05/29 19:50:23  mwg
 * Added code to avoid type redefintions under SunOS.
 *
 * Revision 1.9  1997/03/19 18:35:08  mwg
 * Changed copyright notice.
 *
 * Revision 1.8  1997/02/11  00:05:53  mwg
 * Minor adjustments for Pentium optimization.
 *
 * Revision 1.7  1997/01/11  01:30:47  mwg
 * Added new macro WARN -- the same as ASSERT but without exit.
 *
 * Revision 1.6  1996/08/22  20:07:39  liang
 * When ASSERT fires, only print out information, don't exit.
 *
 * Revision 1.5  1996/05/06  06:49:10  mwg
 * Fixed linux problems.
 *
 * Revision 1.4  1996/05/02  08:40:16  mwg
 * Merged in Chromatic bug fixes.
 *
 * Revision 1.3  1996/04/01  20:59:53  mwg
 * Added macros to setup and use debug pointer.
 *
 * Revision 1.2  1996/02/27  01:50:04  mwg
 * Added ASSERT() macro.
 *
 * Revision 1.1.1.1  1996/02/14  02:35:13  mwg
 * Redesigned the project directory structure. Merged V.34 into the project.
 *
 * Revision 1.2  1995/12/03  06:59:31  mwg
 * Fixed all gcc varnings. We are now running under Linux on a PC!
 *
 *****************************************************************************/
#ifndef	SoftModemTypesh
#define	SoftModemTypesh

#ifdef LONG_SHORTS
#define	short	long
#define	ushort	unsigned long
#endif

typedef signed char			schar;
typedef unsigned char		uchar;

#if 0 /* This is not currently required */
#if defined(_CYGWIN32) && defined(DEBUG)
#define EXCLUDE_CYGWIN32_TYPES
#endif
#endif

#if !defined(_SYS_TYPES_H) || !defined(TARG_OS_RTEMS)
#if defined(_CFE_) || defined(__ECOS) || defined(_NOOS)
 typedef unsigned int		uint;
 typedef unsigned long		ulong;
 typedef unsigned short		ushort;
#elif defined(TARG_OS_RTEMS)
#if defined(HOST_ARCH_LINUX)
 typedef unsigned int		uint;
#endif
 typedef unsigned long		ulong;
#if defined(HOST_ARCH_LINUX)
 typedef unsigned short		ushort;
#endif
#elif defined(EXCLUDE_CYGWIN32_TYPES) || (!defined _NO_TYPE_DEFS_ && !defined _SYS_TYPES_H && !defined __SYS_TYPES_H__ && !defined _SYS_BSD_TYPES_H && !defined _LINUX_TYPES_H) || defined(__sparc__)
#ifndef EXCLUDE_CYGWIN32_TYPES
 typedef unsigned int		uint;
#endif
#ifndef _LINUX_TYPES_H
 typedef unsigned long		ulong;
#endif
#if !defined(ushort) && !defined(EXCLUDE_CYGWIN32_TYPES) && !defined(__INCvxTypesOldh)
 typedef unsigned short		ushort;
#endif
#endif
#else
typedef unsigned long		ulong;
#endif

#if defined(GREENHILL) || defined(GNUTX39) /* GH allocates private data to incorrect section */
#define Private
#else
#define Private             static
#endif

#define Public

#ifdef NULL
#undef NULL
#endif
#ifdef nil
#undef nil
#endif

#define NULL 0
#define nil 0

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

typedef unsigned long long ulonglong;
typedef unsigned char Boolean;
typedef unsigned int BitField; /* this must occur BEFORE long_is_int/int_is_long defs */

#ifdef LONG_IS_INT
#define long int
#define ulong uint
#endif

#ifdef INT_IS_LONG
#define int long
#define uint ulong
#endif

#define POSTULATE(postulate)											\
   do																	\
	   {																\
	   typedef struct													\
		   {															\
		   char	NegativeSizeIfPostulateFalse[((int)(postulate))*2 - 1];	\
		   } PostulateCheckStruct;										\
	   }																\
   while (0)

#if defined(DEBUG) && !defined(__KERNEL__) && !defined(_NOOS)
#ifndef WARN
#define	kDSLNumberWarnTimes	10
#define WARN(assertion) \
	{ static int	warnSeveralTimes=0;	\
	  if ((!(assertion))&(warnSeveralTimes<kDSLNumberWarnTimes)) \
		{ \
		fprintf(stderr, "Warning, failed: %s\n", #assertion); \
		fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); \
		warnSeveralTimes++;	\
		} \
	}
#endif
#ifndef ASSERT
#define ASSERT(assertion) \
	{ if (!(assertion)) \
		{ \
		fprintf(stderr, "Assertion failed: %s\n", #assertion); \
		fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); \
		exit(1); \
		} \
	}
#endif
#else


#undef ASSERT
#define	ASSERT(a)

#endif

/*
 * memory allocation macros
 */

#if defined(bcm47xx) && defined(USE_SLOW_DATA)
#define SLOW_DATA __attribute__ ((section(".slow_data")))
#else
#define SLOW_DATA
#endif

#if defined(bcm47xx) && defined(USE_FAST_TEXT)
#define FAST_TEXT __attribute__ ((section(".fast_text")))
#else
#define FAST_TEXT
#endif

#if defined(bcm47xx) && defined(SWAP_LMEM)
#define SWAP_TEXT1_1 __attribute__ ((section(".swap_text1_1")))
#define SWAP_TEXT1_2 __attribute__ ((section(".swap_text1_2")))
#define SWAP_TEXT2_1 __attribute__ ((section(".swap_text2_1")))
#define SWAP_TEXT2_2 __attribute__ ((section(".swap_text2_2")))
#define SWAP_TEXT3_1 __attribute__ ((section(".swap_text3_1")))
#define SWAP_TEXT3_2 __attribute__ ((section(".swap_text3_2")))
#else
#define SWAP_TEXT1_1 FAST_TEXT
#define SWAP_TEXT1_2 FAST_TEXT
#define SWAP_TEXT2_1 FAST_TEXT
#define SWAP_TEXT2_2 FAST_TEXT
#define SWAP_TEXT3_1 FAST_TEXT
#define SWAP_TEXT3_2 FAST_TEXT
#endif

/*
 * Debug stuff
 */
#if defined(DEBUG) || defined(DEBUG_PTR_ENABLED)
#define	DECLARE_DEBUG_PTR(type)		static	type	*gv;
#define	SETUP_DEBUG_PTR()			gv = &globalVar
#else
#define	DECLARE_DEBUG_PTR(type)
#define	SETUP_DEBUG_PTR()
#endif
/*
 * Obsolete stuff
 */
#ifdef DEBUG
#define	HereIsTheGlobalVarPointerMacro		SETUP_DEBUG_PTR();
#else
#define	HereIsTheGlobalVarPointerMacro
#endif
#endif
