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
 * AdslSelfTest.c -- ADSL self test module
 *
 * Description:
 *	ADSL self test module
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslSelfTest.c,v 1.1 2004/04/08 21:24:49 ilyas Exp $
 *
 * $Log: AdslSelfTest.c,v $
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/


#include "AdslMibDef.h"
#include "AdslCore.h"
#include "AdslCoreMap.h"
#include "AdslSelfTest.h"

#ifdef ADSL_SELF_TEST

#include "softdsl/SoftDsl.h"
#include "softdsl/BlockUtil.h"
#include "softdsl/AdslXfaceData.h"

#ifdef ADSL_PHY_FILE
#include "AdslFile.h"
#else

#if defined(CONFIG_BCM963x8)
#include "adslcoreTest6348/adsl_selftest_lmem.h"
#include "adslcoreTest6348/adsl_selftest_sdram.h"
#endif

#endif /* ADSL_PHY_FILE */

/* from AdslCore.c */

extern AdslXfaceData	*pAdslXface;

/* Locals */

int							adslStMode = 0;
int							adslStRes  = 0;

#define RESET_ADSL_CORE(pAdslReg) do {		\
	DISABLE_ADSL_CORE(pAdslReg);			\
	ENABLE_ADSL_CORE(pAdslReg);				\
} while (0);

#define ENABLE_ADSL_CORE(pAdslReg) do {		\
	pAdslReg[ADSL_CORE_RESET] = 0;			\
    HOST_MIPS_STALL(20);					\
} while (0)

#define DISABLE_ADSL_CORE(pAdslReg) do {	\
	pAdslReg[ADSL_CORE_RESET] = 1;			\
    HOST_MIPS_STALL(20);					\
} while (0)

#define ENABLE_ADSL_MIPS(pAdslReg) do {		\
    pAdslReg[ADSL_MIPS_RESET] = 0x2;		\
} while (0)

#define HOST_MIPS_STALL(cnt)				\
    do { int _stall_count = (cnt); while (_stall_count--) ; } while (0)

#define	MEM_PATTERN0	0x55555555
#define	MEM_PATTERN1	0xAAAAAAAA

AC_BOOL AdslLmemTestPattern(ulong pattern1, ulong pattern2)
{
	volatile	ulong	*pLmem	= (void *) HOST_LMEM_BASE;
	volatile	ulong	*pLmemEnd= (void *) (HOST_LMEM_BASE + 0x18000);
	volatile	static  ulong	val;
	register	ulong	i, newVal;
	register	AC_BOOL	bTestRes = AC_TRUE;

	AdslDrvPrintf(TEXT("ST: LMEM test started, pattern1 = 0x%lX pattern2 = 0x%lX\n"), pattern1, pattern2);
	do {
		*(pLmem + 0) = pattern1;
		*(pLmem + 1) = pattern2;
		pLmem += 2;
	} while (pLmem != pLmemEnd);

	pLmem	= (void *) HOST_LMEM_BASE;

	do {
		for (i = 0; i < 10; i++)
			val = 0;
		newVal = *(pLmem+0);
		if (pattern1 != newVal) {
			AdslDrvPrintf(TEXT("ST: LMEM error at address 0x%lX, pattern=0x%08lX, memVal=0x%08lX(0x%08lX), diff=0x%08lX\n"), 
				(long) pLmem, pattern1, newVal, *(pLmem+0), newVal ^ pattern1);
			bTestRes = AC_FALSE;
		}

		for (i = 0; i < 10; i++)
			val = 0;
		newVal = *(pLmem+1);
		if (pattern2 != newVal) {
			AdslDrvPrintf(TEXT("ST: LMEM error at address 0x%lX, pattern=0x%08lX, memVal=0x%08lX(0x%08lX), diff=0x%08lX\n"), 
				(long) (pLmem+1), pattern2, newVal, *(pLmem+1), newVal ^ pattern2);
			bTestRes = AC_FALSE;
		}

		pLmem += 2;
	} while (pLmem != pLmemEnd);
	AdslDrvPrintf(TEXT("ST: LMEM test finished\n"));
	return bTestRes;
}

AC_BOOL AdslLmemTest(void)
{
	AC_BOOL		bTestRes;

	bTestRes  = AdslLmemTestPattern(MEM_PATTERN0, MEM_PATTERN1);
	bTestRes &= AdslLmemTestPattern(MEM_PATTERN1, MEM_PATTERN0);
	return bTestRes;
}

int	AdslSelfTestRun(int stMode)
{
	volatile ulong	*pAdslEnum = (ulong *) XDSL_ENUM_BASE;
	volatile ulong	*pAdslLMem = (ulong *) HOST_LMEM_BASE;
	volatile AdslXfaceData	*pAdslX = (AdslXfaceData *) ADSL_LMEM_XFACE_DATA;
	ulong			*pAdslSdramImage;
	ulong	 i, tmp;

	adslStMode = stMode;
	if (0 == stMode) {
		adslStRes = kAdslSelfTestCompleted;
		return adslStRes;
	}

	adslStRes = kAdslSelfTestInProgress;

	/* take ADSL core out of reset */
	RESET_ADSL_CORE(pAdslEnum);

	pAdslEnum[ADSL_INTMASK_I] = 0;
	pAdslEnum[ADSL_INTMASK_F] = 0;

	if (!AdslLmemTest()) {
		adslStRes = kAdslSelfTestCompleted;
		return adslStRes;
	}


	/* Copying ADSL core program to LMEM and SDRAM */
#ifdef ADSL_PHY_FILE
	if (!AdslFileLoadImage("/etc/adsl/adsl_test_phy.bin", pAdslLMem, NULL)) {
		adslStRes = kAdslSelfTestCompleted;
		return adslStRes;
	}
	pAdslSdramImage = AdslCoreGetSdramImageStart();
#else
#ifndef  ADSLDRV_LITTLE_ENDIAN
	BlockByteMove ((sizeof(adsl_selftest_lmem)+0xF) & ~0xF, (void *)adsl_selftest_lmem, (uchar *) pAdslLMem);
#else
	for (tmp = 0; tmp < ((sizeof(adsl_selftest_lmem)+3) >> 2); tmp++)
		pAdslLMem[tmp] = ADSL_ENDIAN_CONV_LONG(((ulong *)adsl_selftest_lmem)[tmp]);
#endif
	pAdslSdramImage = AdslCoreSetSdramImageAddr(((ulong *) adsl_selftest_lmem)[2], ((ulong *) adsl_selftest_lmem)[3], sizeof(adsl_selftest_sdram));
	BlockByteMove (AdslCoreGetSdramImageSize(), (void *)adsl_selftest_sdram, (void*)pAdslSdramImage);
#endif

	BlockByteClear (sizeof(AdslXfaceData), (void *)pAdslX);
	pAdslX->sdramBaseAddr = (void *) pAdslSdramImage;

	pAdslEnum[ADSL_HOSTMESSAGE] = stMode;

	ENABLE_ADSL_MIPS(pAdslEnum);

	/* wait for ADSL MIPS to start self-test */
	 
	for (i = 0; i < 10000; i++) {
		tmp = pAdslEnum[ADSL_HOSTMESSAGE];
		if (tmp & (kAdslSelfTestInProgress | kAdslSelfTestCompleted)) {
			break;
		}
		HOST_MIPS_STALL(40);
	}
	AdslDrvPrintf(TEXT("ST: Wait to Start. tmp = 0x%lX\n"), tmp);

	do {
		if (tmp & kAdslSelfTestCompleted) {
			adslStRes = tmp & (~kAdslSelfTestInProgress);
			break;
		}
		if (0 == (tmp & kAdslSelfTestInProgress)) {
			adslStRes = tmp;
			break;
		}

		/* wait for ADSL MIPS to finish self-test */

		for (i = 0; i < 10000000; i++) {
			tmp = pAdslEnum[ADSL_HOSTMESSAGE];
			if (0 == (tmp & kAdslSelfTestInProgress))
				break;
			HOST_MIPS_STALL(40);
		}
		adslStRes = tmp;
	} while (0);
	AdslDrvPrintf(TEXT("ST: Completed. tmp = 0x%lX, res = 0x%lX\n"), tmp, adslStRes);

	RESET_ADSL_CORE(pAdslEnum);
	return adslStRes;
}

int AdslSelfTestGetResults(void)
{
	return adslStRes;
}


#endif /* ADSL_SELF_TEST */
