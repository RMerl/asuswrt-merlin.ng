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
 * BcmCoreDiagNone.c -- Bcm ADSL core diagnostic over socket stubs
 *
 * Description:
 *	This file contains BCM ADSL core driver diagnostic stubs
 *
 *
 * Copyright (c) 2000-2001  Broadcom Corporation
 * All Rights Reserved
 * No portions of this material may be reproduced in any form without the
 * written permission of:
 *          Broadcom Corporation
 *          16215 Alton Parkway
 *          Irvine, California 92619
 * All information contained in this document is Broadcom Corporation
 * company private, proprietary, and trade secret.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: BcmAdslDiagNone.c,v 1.1 2004/04/08 21:24:49 ilyas Exp $
 *
 * $Log: BcmAdslDiagNone.c,v $
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

/* Only for references from BcmCoreTest.c (TBD) */
#include "BcmOs.h"

int		diagSock = -1;
int		diagEnableCnt = 0;
UINT32	adslCoreDiagMap = 0;

int BcmAdslDiagTaskInit(void)
{
	return 0;
}

void BcmAdslDiagTaskUninit(void)
{
}

int BcmAdslDiagInit(int diagDataMap)
{
	return 0;
}

int BcmAdslCoreDiagIntrHandler(void)
{
	return 0;
}

void BcmAdslCoreDiagIsrTask(void)
{
}

void BcmAdslCoreDiagWriteStatusData(UINT32 cmd, char *buf0, int len0,
    char *buf1, int len1)
{
}

void BcmAdslCoreDiagStartLog(UINT32 map, UINT32 time)
{
}

int BcmAdslDiagIsActive(void)
{
	return (0);
}

void BcmAdslDiagReset(int diagDataMap)
{
}

int  BcmAdslDiagGetConstellationPoints (int toneId, void *pointBuf, int numPoints)
{
	return 0;
}

int BcmAdslDiagDisable(void)
{
	diagEnableCnt--;
	return diagEnableCnt;
}

int BcmAdslDiagEnable(void)
{
	diagEnableCnt++;
	return diagEnableCnt;
}

void BcmAdslCoreDiagCmd(char *pAdslDiag)
{
}

int BcmAdslCoreDiagWrite(void *pBuf, int len)
{
    return( 0 );
}

void * BcmAdslCoreDiagGetDmaDataAddr(int descNum)
{
    return( (char *) 0 );
}

int BcmAdslCoreDiagGetDmaDataSize(int descNum)
{
    return( 0 );
}

int	 BcmAdslCoreDiagGetDmaBlockNum(void)
{
	return( 0 );
}

void BcmAdslCoreDiagDmaInit(void)
{
}

typedef     INT32 logDataCode;
void BcmAdslCoreDiagWriteLog(logDataCode logData, ...)
{
}

void DiagWriteStatusInfo(UINT32 cmd, char *p, int n, char *p1, int n1)
{
}

void BcmCoreDiagEyeDataFrameSend(void)
{
}

void BcmAdslCoreDiagStartLog_2(UINT32 map, UINT32 time)
{
}

void BcmAdslDiagDisconnect(void)
{
}

void BcmAdslCoreDiagSetBufDesc(void)
{
}

void DiagWriteFile(UINT32 lineId, UINT32 clientType, char *fname, void *ptr, UINT32 len)
{
}

void __DiagStrPrintf(UINT32 lineId, UINT32 clientType, const char *fmt, int fmtLen, int argNum, ...)
{
}

void BcmAdslDiagSendHdr(void)
{
}

void BcmDiagsMgrRegisterClient(UINT32 clientType, void *pCallback)
{
}

void BcmAdslCoreDiagSetupRawData(int flag)
{
}

void DiagWriteStatus(void *stat, char *pBuf, int len)
{
}

void DiagWriteString(UINT32 lineId, UINT32 clientType,  char *fmt, ...)
{
#ifdef _NOOS
	char	buf[1000];
	va_list	ap;
	va_start(ap, fmt);
	vsprintf (buf, fmt, ap);
	AdslDrvPrintf (TEXT("%s"), buf);
	va_end(ap);
#endif
}

int BcmCoreDiagLogActive(UINT32 map)
{
	return 0;
}

void BcmAdslCoreDiagStartLog_1(UINT32 map, UINT32 time)
{
}

int BcmCoreDiagXdslEyeDataAvail(void)
{
	return 0;
}

int BcmXdslCoreDiagProcFileCreate(void)
{
	return 0;
}

void BcmXdslCoreDiagProcFileRemove(void)
{
}

void BcmAdsl_Notify(unsigned char lineId)
{
}

