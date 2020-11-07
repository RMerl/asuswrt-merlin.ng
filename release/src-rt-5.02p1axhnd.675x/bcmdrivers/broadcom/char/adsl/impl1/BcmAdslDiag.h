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
 * BcmAdslDiag.h -- Internal definitions for ADSL diagnostic
 *
 * Description:
 *	Internal definitions for ADSL core driver
 *
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: BcmAdslDiag.h,v 1.1 2004/04/08 21:24:49 ilyas Exp $
 *
 * $Log: BcmAdslDiag.h,v $
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

#if !defined(_BCM_ADSL_DIAG_H_)
#define _BCM_ADSL_DIAG_H_

#include "DiagDef.h"
#include "bcmadsl.h"

#define			DIAG_SPLIT_MSG		0x80000000
#define			DIAG_MSG_NUM		0x40000000
#define			DIAG_LINE_MASK		0x0C000000
#define			DIAG_LINE_SHIFT		26


/* Diag interface functions */

int	 BcmAdslDiagTaskInit(void);
void BcmAdslDiagTaskUninit(void);
int  BcmAdslDiagInit(int diagDataMap);
void BcmAdslCoreDiagDmaInit(void);
void * BcmAdslCoreDiagGetDmaDataAddr(int descNum);
int	 BcmAdslCoreDiagGetDmaDataSize(int descNum);
int	 BcmAdslCoreDiagGetDmaBlockNum(void);
void BcmAdslDiagReset(int diagDataMap);
int BcmAdslCoreDiagWriteStatusData(uint cmd, char *buf0, int len0, char *buf1, int len1);
void BcmAdslCoreDiagStatusSnooper(dslStatusStruct *status, char *pBuf, int len);
void BcmAdslCoreDiagSaveStatusString(char *fmt, ...);
#if defined(XDSLDRV_ENABLE_PARSER)
#define BcmAdslCoreDiagWriteStatusString(lId,fmt, ...)  DiagWriteString(lId,DIAG_DSL_CLIENT,fmt, ## __VA_ARGS__)
#else
#define BcmAdslCoreDiagWriteStatusString(lId,fmt, ...)  DiagStrPrintf(lId,DIAG_DSL_CLIENT,fmt, ## __VA_ARGS__)
#endif
int  BcmAdslCoreDiagWrite(void *pBuf, int len);
void BcmAdslCoreDiagWriteLog(int logData, ...);
int  BcmAdslCoreDiagIntrHandler(void);
void BcmAdslCoreLogWriteConnectionParam(dslCommandStruct *pDslCmd);
int  BcmAdslDiagGetConstellationPoints (int toneId, void *pointBuf, int numPoints);
int	 BcmAdslDiagDisable(void);
int	 BcmAdslDiagEnable(void);
void BcmAdslDiagDisconnect(int keepDiagConInfo);
int  BcmAdslDiagIsActive(void);
uint BcmXdslDiagGetSrvIpAddr(void);
int  BcmAdslDiagIsConnected(void);
int  BcmAdslGdbIsConnected(void);
void BcmAdslCoreDiagStartLog_1(unsigned char lineId, uint map, uint time);
void BcmAdslCoreDiagStartLog_2(unsigned char lineId, uint map, uint time);
void BcmAdslCoreWriteOvhMsg(void *gDslVars, char *hdr, dslFrame *pFrame);
void BcmAdslCoreDiagSetBufDesc(unsigned char lineId);

#define BcmAdslCoreDiagWriteFile(lineId, fname, ptr, len)	DiagWriteFile(lineId,DIAG_DSL_CLIENT, fname, ptr, len)
#define BcmAdslCoreDiagOpenFile(lineId, fname)				DiagOpenFile(lineId,DIAG_DSL_CLIENT, fname)

void BcmAdslCoreDiagDataLogNotify(int set);
void BcmXdslCoreDiagStatSaveDisableOnRetrainSet(void);
void BcmXdslCoreDiagSendPhyInfo(void);
void BcmAdslDiagSendHdr(void);

#define BcmAdslCoreDiagWriteFile(lineId, fname, ptr, len)		DiagWriteFile(lineId,DIAG_DSL_CLIENT, fname, ptr, len)
#define BcmAdslCoreDiagOpenFile(lineId, fname)		DiagOpenFile(lineId,DIAG_DSL_CLIENT, fname)

void BcmAdslPendingSkbMsgRead(void* d, long* dlen);
int  BcmAdslPendingSkbMsgQueueSize(void);
void BcmAdslPendingSkbMsgQueueEnable(int bEn);
void BcmAdslPendingSkbMsgQueueClear(void);

#endif /* _BCM_ADSL_DIAG_H_ */
