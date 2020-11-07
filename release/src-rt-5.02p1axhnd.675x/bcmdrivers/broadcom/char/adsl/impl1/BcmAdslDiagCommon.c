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
 * BcmAdslDiagCommon.c -- OS independent/common code for DslDiag support
 *
 * Description:
 *	This file contains OS independent/common code for DslDiag support
 *
 * Authors: Ilya Stomakhin
 *
 ****************************************************************************/

#define USE_FRAG

#include <bcmtypes.h>
#if defined(__ECOS)
#include <stdio.h>
#endif

#include "board.h"

#if defined(__KERNEL__) && defined(CONFIG_MIPS)
#include <linux/nbuff.h>
#ifdef TEXT
#undef TEXT
#endif
#endif

#include <DiagDef.h>
#include "BcmOs.h"
#include "bcmadsl.h"
#include "BcmAdslCore.h"

#if defined(__KERNEL__)
#include <linux/etherdevice.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
#include <linux/module.h> 
#endif
#include <bcmxtmcfg.h>
#elif defined(__ECOS)
#include <xtmcfgdrv.h>
#endif

#if defined(SUPPORT_2CHIP_BONDING)
#include "bcm_ext_bonding_comm.h"
#endif

#define EXCLUDE_CYGWIN32_TYPES
#include "softdsl/SoftDsl.h"
#include "softdsl/BlockUtil.h"

#include "AdslCore.h"
#include "BcmAdslDiag.h"
#include "BcmAdslDiagCommon.h"
#include "bcm_map.h"
#if defined(USE_PMC_API)
#include "pmc_drv.h"
#include "BPCM.h"
#include <linux/dma-mapping.h>
extern void BcmAdslCoreStop(void);
extern void BcmAdslCoreStart(int diagDataMap, Bool bRestoreImage);
#endif

extern struct device *pXdslDummyDevice;
#if defined(__KERNEL__)
#if defined(CONFIG_MIPS)
#define	Cache_Flush_Len(a, l)	cache_flush_len(a, l)
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
#define	Cache_Invalidate_Len(a, l)	cache_invalidate_len(a, l)
#else
#define	Cache_Invalidate_Len(a, l)	dma_cache_wback_inv((ulong)(a), l)
#endif
#elif defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#ifdef CONFIG_ARM64
#if 0
#define	Cache_Flush_Len(a, l)	dma_map_single(pXdslDummyDevice, a, l, DMA_TO_DEVICE)
#define	Cache_Invalidate_Len(a, l)	dma_map_single(pXdslDummyDevice, a, l, DMA_FROM_DEVICE)
#endif
// For the 64-bits chip, we can't allocate cacheable memory below 256MB where PHY can access, so
// we will use non-cacheable memory to exchange data b/w PHY and the Host
#define	Cache_Flush_Len(a, l)
#define	Cache_Invalidate_Len(a, l)
#else /* !CONFIG_ARM64 */
#define	Cache_Flush_Len(a, l)	dma_map_single(NULL, a, l, DMA_TO_DEVICE)
#define	Cache_Invalidate_Len(a, l)	dma_map_single(NULL, a, l, DMA_FROM_DEVICE)
#endif
#endif /* defined(CONFIG_MIPS) */
#endif /* defined(__KERNEL__) */

extern Bool	adslCoreInitialized;
extern void BcmAdslCoreReset(int diagDataMap);
extern void BcmAdslCoreSetWdTimer(uint timeUs);

extern Bool BcmXdslDiagStatSaveLocalIsActive(void);
extern void BcmXdslDiagStatSaveLocal(uint cmd, char *statStr, int n, char *p1, int n1);
extern void BcmAdslDiagEnablePrintStatCmd(void);

void DiagWriteDataCont(uint cmd, char *buf0, int len0, char *buf1, int len1);

#if (kDiagReceivedEocCommand != kDslReceivedEocCommand)
#error Inconsistent kDiagReceivedEocCommand definition
#endif
#if (kDiagStrPrintf != kDslStrPrintf )
#error Inconsistent kDiagStrPrintf definition
#endif
							

diagCtrlType diagCtrl = {0};
dumpBufferStruct dumpBuf = { NULL, 0, 0, 0 };
void             *dbPhysAddr = NULL;

/*
**
**	Socket diagnostic support
**
*/


#define DIAG_MAX_NUM_FRAG		100

struct diagClients{
	uint          client;
	dslDrvSkbPool  *diagSkbDev;
	void           (*pCallback)(unsigned char lineId, int diagCmd, int len, void *pCmdData);
	Boolean        isRegistered;
};

struct diagClients diagClientsTbl[4];

DiagConnectInfo diagConnectInfo = {{ 0 }, { 0 }};

int DiagWriteData(uint  cmd, char *buf0, int len0, char *buf1, int len1);

void IdleStatusCallback(unsigned char lineId, int diagCmd, int len, void *pCmdData) {

}

void BcmDiagsMgrRegisterClient(uint clientType, void *pCallback)
{
	diagClientsTbl[clientType & 0x3].isRegistered = TRUE;
	if (NULL == pCallback)
		diagClientsTbl[clientType & 0x3].pCallback = IdleStatusCallback;
	else
		diagClientsTbl[clientType & 0x3].pCallback = pCallback;
	printk("***BcmDiagsMgrRegisterClient: %d ***\n", (int) clientType);
}

void DiagsDeregisterClient(uint clientType)
{
	diagClientsTbl[clientType & 0x3].isRegistered = FALSE;
	diagClientsTbl[clientType & 0x3].pCallback = NULL;
}


ushort DiagIpComputeChecksum(diagIpHeader *ipHdr)
{
	ushort	*pHdr = (ushort	*) ipHdr;
	ushort	*pHdrEnd = pHdr + 10;
	uint	sum = 0;

	do {
		sum += pHdr[0];
		sum += pHdr[1];
		pHdr += 2;
	} while (pHdr != pHdrEnd);

	while (sum > 0xFFFF)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return sum ^ 0xFFFF;
}

ushort DiagIpUpdateChecksum(int sum, ushort oldv, ushort newv)
{
	ushort	tmp;

	tmp = (newv - oldv);
	tmp -= (tmp >> 15);
	sum = (sum ^ 0xFFFF) + tmp;
	sum = (sum & 0xFFFF) + (sum >> 16);
	return sum ^ 0xFFFF;
}

void DiagUpdateDataLen(diagSockFrame *diagFr, int dataLen)
{
	int	newIpHdrLen = dataLen + sizeof(LogProtoHeader) + sizeof(diagUdpHeader) + sizeof(diagIpHeader);
	
	diagFr->ipHdr.checksum = htons(DiagIpUpdateChecksum(htons(diagFr->ipHdr.checksum), htons((ushort)diagFr->ipHdr.len), newIpHdrLen));
	diagFr->ipHdr.len = htons(newIpHdrLen);
	diagFr->udpHdr.len = htons(newIpHdrLen - sizeof(diagIpHeader));
}

int DevSkbSendDiagsPacket(dslDrvSkbPool *skbDev, uint cmd, dslDrvFragment *pFrag, int nFrag)
{
	struct sk_buff  *skbDiag = NULL;
	int n = 0;
	int k = 0;

	if ((skbDev == NULL) || (pFrag == NULL) || (nFrag == 0))
		return 0;
	
	BcmCoreDpcSyncEnter(SYNC_DIAGS);
	if (nFrag == 1) {
		if( NULL != (skbDiag = GetSkb(skbDev, pFrag[0].len)) )
			n = __DiagWriteData(skbDiag, cmd, pFrag[0].pBuf, pFrag[0].len, NULL, 0);
		BcmCoreDpcSyncExit(SYNC_DIAGS);
		return n;
	}

	if ((nFrag == 2) &&  ((pFrag[0].len + pFrag[1].len) <= LOG_MAX_DATA_SIZE)) {
		if( NULL != (skbDiag = GetSkb(skbDev, (pFrag[0].len + pFrag[1].len))) )
			n = __DiagWriteData(skbDiag, cmd, pFrag[0].pBuf, pFrag[0].len, pFrag[1].pBuf, pFrag[1].len);
		BcmCoreDpcSyncExit(SYNC_DIAGS);
		return n;
	}
	
	//BcmCoreDpcSyncEnter();
	if( NULL != (skbDiag = GetSkb(skbDev, pFrag[0].len)) )
		n = __DiagWriteData(skbDiag, cmd | DIAG_SPLIT_MSG, pFrag[0].pBuf, pFrag[0].len, NULL, 0);
	else {
		//BcmCoreDpcSyncExit();
		BcmCoreDpcSyncExit(SYNC_DIAGS);
		return 0;
	}
	for (k = 1; k < (nFrag-1); k++){
		skbDiag = GetSkb(skbDev, pFrag[k].len);
		if (NULL != skbDiag)
			n += __DiagWriteData(skbDiag, (cmd & (DIAG_LINE_MASK | DIAG_TYPE_MASK)) | (statusInfoData-2), pFrag[k].pBuf, pFrag[k].len, NULL, 0);
		else {
			//BcmCoreDpcSyncExit();
			BcmCoreDpcSyncExit(SYNC_DIAGS);
			return n;
		}
	}
	skbDiag = GetSkb(skbDev, pFrag[k].len);
	if (NULL != skbDiag)
		n += __DiagWriteData(skbDiag, (cmd & (DIAG_LINE_MASK | DIAG_TYPE_MASK)) | (statusInfoData-3), pFrag[k].pBuf, pFrag[k].len, NULL, 0);
	//BcmCoreDpcSyncExit();
	BcmCoreDpcSyncExit(SYNC_DIAGS);
	
	return n;
}

int DiagBuildFragTbl(dslDrvFragment *frag, char *p0, int len0, char *p1,  int len1)
{
	int d, r, nFrag  = 0;

	if (((len0 + len1)) > (DIAG_MAX_NUM_FRAG*LOG_MAX_DATA_SIZE)) {
		return nFrag;
	}

	if (len0 > 0) {
		d = len0/LOG_MAX_DATA_SIZE;
		r = len0 - d * LOG_MAX_DATA_SIZE;
		frag[nFrag].pBuf = p0;
		frag[nFrag].len = (d > 0) ? LOG_MAX_DATA_SIZE:r;
		nFrag++;
		if (d > 0){
			for(; nFrag < d ; nFrag++){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = LOG_MAX_DATA_SIZE;
			}
			if (r > 0){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = r;
				nFrag++;
			}
		}
	}

	if (len1 > 0) {
		d = len1/LOG_MAX_DATA_SIZE;
		r = len1 - d * LOG_MAX_DATA_SIZE;
		frag[nFrag].pBuf = p1;
		frag[nFrag].len = (d > 0) ? LOG_MAX_DATA_SIZE:r;
		nFrag++;
		if (d > 0){
			int d1 = nFrag+d-1;
			for(; nFrag < d1 ; nFrag++){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = LOG_MAX_DATA_SIZE;
			}
			if (r > 0){
				frag[nFrag].pBuf = (char *) (frag[nFrag-1].pBuf) + LOG_MAX_DATA_SIZE;
				frag[nFrag].len = r;
				nFrag++;
			}
		}
	}

	return nFrag;
}


void DiagWriteStatusInfo(uint cmd, char *p, int n, char *p1, int n1)
{
#ifdef USE_FRAG
	dslDrvFragment frag[DIAG_MAX_NUM_FRAG];
	int nFrag;
	dslDrvSkbPool	*diagSkbDev = NULL;
	uint clientType;
	
	clientType = (cmd & DIAG_TYPE_MASK) >> DIAG_TYPE_SHIFT;
	diagSkbDev = diagClientsTbl[clientType & 0x3].diagSkbDev;
	if(NULL == diagSkbDev)
		return;
	nFrag = DiagBuildFragTbl(frag, p, n, p1, n1);
	DevSkbSendDiagsPacket(diagSkbDev, cmd, frag, nFrag);
#else
	if (n > LOG_MAX_DATA_SIZE) {
		DiagWriteData(cmd | DIAG_SPLIT_MSG, p, LOG_MAX_DATA_SIZE, NULL, 0);
#ifdef SUPPORT_DSL_BONDING
		DiagWriteDataCont((cmd & (DIAG_LINE_MASK | DIAG_TYPE_MASK)) | (statusInfoData-2), p + LOG_MAX_DATA_SIZE, n - LOG_MAX_DATA_SIZE, p1, n1);
#else
		DiagWriteDataCont(statusInfoData-2, p + LOG_MAX_DATA_SIZE, n - LOG_MAX_DATA_SIZE, p1, n1);
#endif
	}
	else if ((n + n1) > LOG_MAX_DATA_SIZE) {
		int	len1 = (LOG_MAX_DATA_SIZE - n) & ~1;
		DiagWriteData(cmd | DIAG_SPLIT_MSG, p, n, p1, len1);
#ifdef SUPPORT_DSL_BONDING
		DiagWriteDataCont((cmd & (DIAG_LINE_MASK | DIAG_TYPE_MASK)) | (statusInfoData-2), p1 + len1, n1 - len1, NULL, 0);
#else
		DiagWriteDataCont((cmd & DIAG_TYPE_MASK) | (statusInfoData-2), p1 + len1, n1 - len1, NULL, 0);
#endif
	}
	else
		DiagWriteData(cmd, p, n, p1, n1);
#endif
}

static char * DiagScrambleString(char *s)
{
	char	*p = s;

	while (*p != 0) {
		*p = ~(*p);
		p++;
	}

	return s;
}

void DiagWriteStatus(void *stat, char *pBuf, int len)
{
	dslStatusStruct *status = stat;
	int		statStrBuf[1000/4];
	char			*statStr, *p1;
	uint		cmd, clientType, statusCode;
	int			n,n1;
	uchar			lineId;

	if (!BcmAdslDiagIsActive() && !BcmXdslDiagStatSaveLocalIsActive())
		return;

	//BcmCoreDpcSyncEnter();
	statStr = pBuf;
	n = len;
	p1 = NULL;
	n1 = 0;

	lineId = DSL_LINE_ID(status->code);
	clientType = (status->code & DIAG_TYPE_MASK) >> DIAG_TYPE_SHIFT;
	cmd = statusInfoData | (clientType << DIAG_TYPE_SHIFT) | (lineId << DIAG_LINE_SHIFT);
	statusCode = DSL_STATUS_CODE(status->code);
	
	switch (statusCode) {
		case kDslPrintfStatus1:
		case kDslPrintfStatus:
		case kDslPrintfStatusSaveLocalOnly:
		{
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
			va_list arg;
#if defined(CONFIG_ARM64)
			if (kDslPrintfStatus != statusCode)
			  arg = *(va_list *) status->param.dslPrintfMsg.argPtr;
#else
			arg.__ap = status->param.dslPrintfMsg.argPtr;
#endif
#else /* !ARM  */
			va_list arg = status->param.dslPrintfMsg.argPtr;
#endif
			statStr = (char *) &statStrBuf[1];
#ifdef ADSLDRV_LITTLE_ENDIAN
			if((statusCode == kDslPrintfStatus) && (0 != status->param.dslPrintfMsg.argNum)) {
				BlockLongMoveReverse(status->param.dslPrintfMsg.argNum,
					(int *)status->param.dslPrintfMsg.argPtr,
					(int *)status->param.dslPrintfMsg.argPtr);
			}
#endif
#if defined(CONFIG_ARM64)
			if (kDslPrintfStatus == statusCode) {
				if (0xFF != *status->param.dslPrintfMsg.fmt) {
					uint *pArg   = status->param.dslPrintfMsg.argPtr;
					/* Can't use vsprintf since PHY arguments are 32bit not 64bit expected by vsprintf */
					n = sprintf (statStr, status->param.dslPrintfMsg.fmt, pArg[0], pArg[1], pArg[2], pArg[3], 
						pArg[4], pArg[5], pArg[6], pArg[7], pArg[8], pArg[9], pArg[10], pArg[11]) + 1;
				}
				else {
					UINT64 *pArg = (UINT64 *) status->param.dslPrintfMsg.argPtr;
					/* arg is not setup anyway */
					status->param.dslPrintfMsg.fmt++;
					n = sprintf (statStr, status->param.dslPrintfMsg.fmt, pArg[0], pArg[1], pArg[2], pArg[3], 
					pArg[4], pArg[5], pArg[6], pArg[7], pArg[8], pArg[9], pArg[10], pArg[11]) + 1;
				}
			}
			else
#endif
			n = vsprintf (statStr, status->param.dslPrintfMsg.fmt, arg) + 1;
			if (statusCode == kDslPrintfStatus) {
				cmd = LOG_CMD_SCRAMBLED_STRING | (lineId << DIAG_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);
				DiagScrambleString(statStr);
			}
			else {
				statStrBuf[0] = 0;	/* argNum */
				p1 = (char *) statStrBuf;
				n1 = n+4;
				statStr = (char *)stat;
				n = sizeof( status->code) + sizeof( status->param.dslClearEocMsg) - sizeof(uintptr_t);
				status->code = ADSL_ENDIAN_CONV_INT32(kDiagReceivedEocCommand | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT));
				status->param.dslClearEocMsg.msgId = ADSL_ENDIAN_CONV_INT32(kDiagStrPrintf);
				status->param.dslClearEocMsg.msgType = ADSL_ENDIAN_CONV_INT32(n1 | kDslClearEocMsgDataVolatile);
			}
			break;
		}
		case kDslReceivedEocCommand:
#ifdef ADSLDRV_LITTLE_ENDIAN
			if ( ((kDslClearEocSendFrame  == status->param.dslClearEocMsg.msgId) ||
					(kDslClearEocRcvedFrame == status->param.dslClearEocMsg.msgId) ||
					(kDslGeneralMsgStart <= status->param.dslClearEocMsg.msgId))
					&&
					(0 == (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)))
			{
				dslStatusStruct *status1 = (dslStatusStruct *)statStr;
				
				if(kDsl993p2TestHlin == status->param.dslClearEocMsg.msgId)
					n1 = status->param.dslClearEocMsg.msgType & 0x1FFFF;
				else
					n1 = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
				p1 = status->param.dslClearEocMsg.dataPtr;
				if(status != status1) {
					n -= sizeof(int);
					/* From PHY, kDslClearEocMsgDataVolatileMask is set directly in the flattened buffer */
					status1->param.dslClearEocMsg.msgType |= ADSL_ENDIAN_CONV_INT32(kDslClearEocMsgDataVolatileMask);
					if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgHostAddr) {
						Cache_Invalidate_Len(p1, n1);
#if defined(CONFIG_BCM963158)
						p1 = (void *) (((uintptr_t)dumpBuf.pBuf & 0xFFFFFFFF00000000) | (uintptr_t) p1);
						status->param.dslClearEocMsg.dataPtr = p1;
#endif
					}
				}
				else {
					n -= sizeof(uintptr_t);
					status1->code = ADSL_ENDIAN_CONV_INT32(status1->code);
					status1->param.dslClearEocMsg.msgId = ADSL_ENDIAN_CONV_INT32(status1->param.dslClearEocMsg.msgId);
					status1->param.dslClearEocMsg.msgType = ADSL_ENDIAN_CONV_INT32(status1->param.dslClearEocMsg.msgType | kDslClearEocMsgDataVolatileMask);
				}
			}
#else
			if ( ((kDslClearEocSendFrame  == status->param.dslClearEocMsg.msgId) ||
					(kDslClearEocRcvedFrame == status->param.dslClearEocMsg.msgId) ||
					(kDslGeneralMsgStart <= status->param.dslClearEocMsg.msgId))
					&&
					(0 == (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)))
			{
				dslStatusStruct *status1 = (dslStatusStruct *)statStr;
				
				((dslStatusStruct *)statStr)->param.dslClearEocMsg.msgType |= kDslClearEocMsgDataVolatileMask;
				
				if(kDsl993p2TestHlin == status->param.dslClearEocMsg.msgId)
					n1 = status->param.dslClearEocMsg.msgType & 0x1FFFF;
				else
					n1 = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
				p1 = status->param.dslClearEocMsg.dataPtr;
				if(status != status1) {
					n -= sizeof(int);
#if defined(CONFIG_BCM963146)
					if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgHostAddr) {
						Cache_Invalidate_Len(p1, n1);
						p1 = (void *) (((uintptr_t)dumpBuf.pBuf & 0xFFFFFFFF00000000) | (uintptr_t) p1);
						status->param.dslClearEocMsg.dataPtr = p1;
					}
#endif
				}
				else
					n -= sizeof(uintptr_t);
			}
#endif
			break;
		default:
#ifdef ADSLDRV_LITTLE_ENDIAN
		{
			dslStatusStruct *status1 = (dslStatusStruct *)statStr;
			if(status->code == status1->code) {
				if(kDslDataAvailStatus == status1->code) {
					status1->code = ADSL_ENDIAN_CONV_INT32(status1->code);
					status1->param.dslDataAvail.dataPtr = ADSL_ENDIAN_CONV_UINT32(status1->param.dslDataAvail.dataPtr);
					status1->param.dslDataAvail.dataLen = ADSL_ENDIAN_CONV_UINT32(status1->param.dslDataAvail.dataLen);
				}
				else {
					status1->code = ADSL_ENDIAN_CONV_INT32(status1->code);
					status1->param.value = ADSL_ENDIAN_CONV_INT32(status1->param.value);
				}
			}
		}
#endif
			break;
	}
	if(BcmXdslDiagStatSaveLocalIsActive())
		BcmXdslDiagStatSaveLocal(cmd, statStr, n, p1, n1);

	if (!BcmAdslDiagIsActive() || (kDslPrintfStatusSaveLocalOnly == statusCode)) {
		//BcmCoreDpcSyncExit();
		return;
	}
	DiagWriteStatusInfo(cmd, statStr, n, p1, n1);
	//BcmCoreDpcSyncExit();
}


void DiagWriteStatusShort(uint lineId, uint clientType, uint code, uint value)
{
	dslStatusStruct   status;
	status.code = code  | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);
	status.param.value = value;
	DiagWriteStatus(&status, (char *)&status, sizeof(status.code) + sizeof(status.param.value));
}

void DiagWriteStatusLong(uint lineId, uint clientType, uint  msgId, void *ptr, uint len, uint  flags)
{
	dslStatusStruct   status;
	status.code = kDslReceivedEocCommand | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);

	status.param.dslClearEocMsg.msgId    = msgId;
	status.param.dslClearEocMsg.msgType = (flags) | (len);
	status.param.dslClearEocMsg.dataPtr = (void *)ptr;

	DiagWriteStatus(&status, (char *)&status, sizeof(status.code) + sizeof(status.param.dslClearEocMsg));
}


void DiagWriteFile(uint lineId, uint clientType, char *fname, void *ptr, uint len)
{
	dslStatusStruct	  status;

	status.code = kDslReceivedEocCommand | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);
	status.param.dslClearEocMsg.msgId	= kDslGeneralMsgDbgFileName;
	status.param.dslClearEocMsg.msgType = strlen(fname) + 1;
	status.param.dslClearEocMsg.dataPtr = fname;
	DiagWriteStatus(&status, (char *)&status, sizeof(status.code) + sizeof(status.param.dslClearEocMsg));
#ifdef ADSLDRV_LITTLE_ENDIAN
	status.code = kDslReceivedEocCommand | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);
#endif
	status.param.dslClearEocMsg.msgId	= kDslGeneralMsgDbgWriteFile;
	status.param.dslClearEocMsg.msgType = len;
	status.param.dslClearEocMsg.dataPtr = ptr;
	DiagWriteStatus(&status, (char *)&status, sizeof(status.code) + sizeof(status.param.dslClearEocMsg));
}


void DiagOpenFile(uint lineId, uint clientType, char *fname)
{
	dslStatusStruct	  status;

	status.code = kDslReceivedEocCommand | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);
	status.param.dslClearEocMsg.msgId	= kDslGeneralMsgDbgFileName;
	status.param.dslClearEocMsg.msgType = (strlen(fname) + 1) | kDslDbgFileNameDelete;
	status.param.dslClearEocMsg.dataPtr = fname;
	DiagWriteStatus(&status, (char *)&status, sizeof(status.code) + sizeof(status.param.dslClearEocMsg));
}


void DiagDumpData(uint lineId, uint clientType, void *ptr, uint len, uint  flags)
{
	dslStatusStruct	  status;
	status.code = kDslReceivedEocCommand | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);

	status.param.dslClearEocMsg.msgId	= kDslGeneralMsgDbgDataPrint;
	status.param.dslClearEocMsg.msgType = (flags) | (len);
	status.param.dslClearEocMsg.dataPtr = (void *)ptr;

	DiagWriteStatus(&status, (char *)&status, sizeof(status.code) + sizeof(status.param.dslClearEocMsg));
}

#if defined(XDSLDRV_ENABLE_PARSER)
void DiagWriteString(uint lineId, uint clientType,  char *fmt, ...)
{
	char	buf[1000];
	va_list	ap;
	va_start(ap, fmt);
	vsprintf (buf, fmt, ap);
	AdslDrvPrintf (TEXT("%s"), buf);
	va_end(ap);
}

#else

void DiagWriteString(uint lineId, uint clientType,  char *fmt, ...)
{
	dslStatusStruct 	status;
	va_list 			ap;

	va_start(ap, fmt);

	status.code = kDslPrintfStatus1 | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);

	status.param.dslPrintfMsg.fmt = fmt;
	status.param.dslPrintfMsg.argNum = 0;
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#if defined(CONFIG_ARM64)
	status.param.dslPrintfMsg.argPtr = &ap;
#else
	status.param.dslPrintfMsg.argPtr = (void *)ap.__ap;
#endif
#else
	status.param.dslPrintfMsg.argPtr = (void *)ap;
#endif
	DiagWriteStatus(&status, NULL, 0);
	va_end(ap);
}
#endif

void DiagWriteStringV(uint lineId, uint clientType, const char *fmt, void *ap)
{
	dslStatusStruct 	status;

	status.code = kDslPrintfStatus1 | (lineId << DSL_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT);

	status.param.dslPrintfMsg.fmt = (char *) fmt;
	status.param.dslPrintfMsg.argNum = 0;
	status.param.dslPrintfMsg.argPtr = ap;


	DiagWriteStatus(&status, NULL, 0);
}

void __DiagStrPrintf(uint lineId, uint clientType, const char *fmt, int fmtLen, int argNum, ...)
{
	va_list	ap;
	void	*arg;
	
	va_start(ap, argNum);
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#if defined(CONFIG_ARM64)
	arg = (char *) ap.__gr_top + ap.__gr_offs;
#else
	arg = ap.__ap;
#endif
#else	/* !ARM */
	arg = ap;
#endif

#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#if defined(CONFIG_ARM64)
	{
		int i;
		uint *pData32 = (uint *)arg;
		ulong v;

		/* first 4 are 32bit packed */
		v = va_arg(ap, long);
		pData32[0] = ADSL_ENDIAN_CONV_INT32(v & 0xffffffff);
		pData32[1] = ADSL_ENDIAN_CONV_INT32(v >> 32);
		v = va_arg(ap, long);
		pData32[2] = ADSL_ENDIAN_CONV_INT32(v & 0xffffffff);
		pData32[3] = ADSL_ENDIAN_CONV_INT32(v >> 32);

		pData32 += 4;
		/* pack 64bit arguments to 32bit */
		for(i = 0; i < (argNum - 4); i++) {
			v = va_arg(ap, long);
			pData32[i] = ADSL_ENDIAN_CONV_INT32(v & 0xffffffff);
		}
	}
#else /* !CONFIG_ARM64 */
	{
		int i;
		uint *pData32 = (uint *)arg, v;
		for(i = 0; i < argNum; i++) {
			v = va_arg(ap, int);
			pData32[i] = ADSL_ENDIAN_CONV_INT32(v);
		}
	}
#endif
#endif
	if(BcmXdslDiagStatSaveLocalIsActive())
		BcmXdslDiagStatSaveLocal(statusInfoData | (lineId<<DIAG_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT), arg, (argNum << 2), (void *)fmt, fmtLen);
	DiagWriteStatusInfo(statusInfoData | (lineId<<DIAG_LINE_SHIFT) | (clientType << DIAG_TYPE_SHIFT), arg, (argNum << 2), (void *)fmt, fmtLen);
	va_end(ap);
}

void BcmAdslCoreDebugPrintToConsole(DiagDebugData *pDbgCmd,unsigned char *pBuf, int len)
{
	int i;
	
	printk("DebugReadMem: Addr 0x%08X - 0x%08X(0x%08X)\n", (uint)pDbgCmd->param1, (uint)pDbgCmd->param1+len, (uint)pDbgCmd->param2);
	for(i=0; i < len; i++) {
		if(0 == i%16)
			printk("\n");
		printk("%02X ", pBuf[i]);
	}
	printk("\n");
}

int DiagWriteData(uint  cmd, char *buf0, int len0, char *buf1, int len1) {
	struct sk_buff  *skbDiag = NULL;
	int n = 0;
	dslDrvSkbPool	*diagSkbDev = NULL;
	uint clientType;

	clientType = (cmd & DIAG_TYPE_MASK) >> DIAG_TYPE_SHIFT;
	diagSkbDev = diagClientsTbl[clientType & 0x3].diagSkbDev;
#if defined(SUPPORT_EXT_DSL_BONDING_SLAVE)
	if ((clientType == DIAG_DSL_CLIENT)&& (diagSkbDev == NULL)){
		BcmXdslCoreSendStatToExtBondDev(NULL, cmd, buf0, len0, buf1, len1);
		return 0;
	}
#endif
	BcmCoreDpcSyncEnter(SYNC_DIAGS);
	if (diagSkbDev != NULL) {
		skbDiag = GetSkb(diagSkbDev, (len0 + len1));
		if (skbDiag != NULL)
			n = __DiagWriteData(skbDiag, cmd, buf0, len0, buf1, len1);
	}
	BcmCoreDpcSyncExit(SYNC_DIAGS);
	
	return n;
}

void DiagWriteDataCont(uint cmd, char *buf0, int len0, char *buf1, int len1)
{
	if (len0 > LOG_MAX_DATA_SIZE) {
		DiagWriteData(cmd, buf0, LOG_MAX_DATA_SIZE, NULL, 0);
		DiagWriteDataCont(cmd, buf0 + LOG_MAX_DATA_SIZE, len0 - LOG_MAX_DATA_SIZE, buf1, len1);
	}
	else if ((len0 + len1) > LOG_MAX_DATA_SIZE) {
		int	len2 = (LOG_MAX_DATA_SIZE - len0) & ~1;

		DiagWriteData(cmd, buf0, len0, buf1, len2);
		DiagWriteDataCont(cmd, buf1 + len2, len1 - len2, NULL, 0);
	}
	else
#ifdef SUPPORT_DSL_BONDING
		DiagWriteData((cmd & DIAG_LINE_MASK) | (statusInfoData-3), buf0, len0, buf1, len1);
#else
		DiagWriteData(statusInfoData-3, buf0, len0, buf1, len1);
#endif
}

int BcmAdslCoreDiagWriteStatusData(uint cmd, char *buf0, int len0, char *buf1, int len1)
{
	return DiagWriteData(cmd, buf0, len0, buf1, len1);
}

int BcmAdslCoreDiagWrite(void *pBuf, int len)
{
	uint			cmd;
	DiagProtoFrame	*pDiagFrame = (DiagProtoFrame *) pBuf;
#ifdef SUPPORT_DSL_BONDING
	uchar			lineId = (pDiagFrame->diagHdr.logPartyId & DIAG_PARTY_LINEID_MASK) >> DIAG_PARTY_LINEID_SHIFT;
	cmd = (lineId << DIAG_LINE_SHIFT) | pDiagFrame->diagHdr.logCommmand;
#else
	cmd = pDiagFrame->diagHdr.logCommmand;
#endif
	return DiagWriteData(cmd, pDiagFrame->diagData, len - sizeof(LogProtoHeader), NULL, 0);
}

#define DiagWriteMibData(dev,buf,len)		DiagWriteData(LOG_CMD_MIB_GET,buf,len,NULL,0)
#define DiagWriteStatusString(dev,str)		DiagWriteData(LOG_CMD_SCRAMBLED_STRING,str,strlen(str)+1,NULL,0)

void BcmAdslCoreDiagWriteLog(logDataCode logData, ...)
{
	static	char	logDataBuf[512];
	char			*logDataPtr = logDataBuf;
	int			n, i, datum, *pCmdData;
	va_list			ap;

	if ((NULL == diagCtrl.dbgDev) || (0 == (diagCtrl.diagDataMap[0] & DIAG_DATA_LOG)))
		return;

	va_start(ap, logData);

	switch	(logData) {
		case	commandInfoData:
			logDataPtr += sprintf(logDataPtr, "%d:\t", (int)logData);
			pCmdData = (void *) va_arg(ap, uintptr_t);
			n = va_arg(ap, int);
			for (i = 0; i < n ; i++)
				logDataPtr += sprintf(logDataPtr, "%d ", (int)pCmdData[i]);
			logDataPtr += sprintf(logDataPtr, "\n");
			break;
		case (inputSignalData - 2):
			datum = va_arg(ap, int);
			*logDataPtr++ = (char) datum;
			break;
		default:
			break;
	}

	if (logDataPtr != logDataBuf)
		DiagWriteData(logData | DIAG_MSG_NUM, logDataBuf, (logDataPtr - logDataBuf), NULL, 0);
	va_end(ap);
}

char *ConvertToDottedIpAddr(uint ipAddr, char *buf)
{
	if(NULL != buf) {
		sprintf(buf,"%d.%d.%d.%d",
			(int)((ipAddr>>24)&0xFF),
			(int)((ipAddr>>16)&0xFF),
			(int)((ipAddr>>8)&0xFF),
			(int)(ipAddr&0xFF));
	}
	return buf;
}

#ifdef CONFIG_ARM64
extern Bool	gSharedMemAllocFromUserContext;
#endif

void BcmAdslDiagSendHdr(void)
{
	//if((NULL != diagCtrl.dbgDev) && (NULL != diagCtrl.skbModel)) {
    if ( BcmAdslDiagIsConnected() ) {
		dslCommandStruct	cmd;
		cmd.command = kDslDiagFrameHdrCmd;
#ifndef __ECOS
#ifdef CONFIG_ARM64
		gSharedMemAllocFromUserContext=1;
		cmd.param.dslStatusBufSpec.pBuf = AdslCoreSharedMemAlloc(DIAG_FRAME_HEADER_LEN);
		//memcpy(cmd.param.dslStatusBufSpec.pBuf, (void *)diagCtrl.skbModel->data, DIAG_FRAME_HEADER_LEN);
		BlockByteMoveDstAlign(DIAG_FRAME_HEADER_LEN, (void *)diagCtrl.skbModel->data, cmd.param.dslStatusBufSpec.pBuf);
#else
		cmd.param.dslStatusBufSpec.pBuf = (void *)diagCtrl.skbModel->data;
		Cache_Flush_Len((void *)diagCtrl.skbModel->data, DIAG_FRAME_HEADER_LEN);
#endif
#endif
		cmd.param.dslStatusBufSpec.bufSize = DIAG_FRAME_HEADER_LEN;
		BcmAdslCoreDiagWriteStatusString(0, "Diag Hdr Addr: %p, Size %d\n",
			cmd.param.dslStatusBufSpec.pBuf, (int)cmd.param.dslStatusBufSpec.bufSize);
		BcmCoreCommandHandler(&cmd);
#ifdef CONFIG_ARM64
		gSharedMemAllocFromUserContext=0;
#endif
		BcmAdslCoreDiagWriteStatusString (0, "Sending DslDiags Hdr to PHY\n");
	}
}

void BcmAdslCoreDiagSetBufDesc(unsigned char lineId)
{
	dslCommandStruct	cmd;
	
	if ((NULL == diagCtrl.dbgDev) || (NULL == diagCtrl.pEyeDataAppCtrl[lineId]))
		return;
#ifdef SUPPORT_DSL_BONDING
	cmd.command = kDslDiagSetupBufDesc | (lineId << DSL_LINE_SHIFT);
#else
	cmd.command = kDslDiagSetupBufDesc;
#endif
	cmd.param.dslDiagBufDesc.descBufAddr = (void *)diagCtrl.pEyeDataAppCtrl[lineId]->pDescRing;
	cmd.param.dslDiagBufDesc.bufCnt = DIAG_ZEROCOPY_NBUF_MAX(diagCtrl.pEyeDataAppCtrl[lineId]);
	BcmAdslCoreDiagWriteStatusString(lineId, "Diag Desc Addr: %p, nBuf %d\n",
		diagCtrl.pEyeDataAppCtrl[lineId]->pDescRing,
		(int)DIAG_ZEROCOPY_NBUF_MAX(diagCtrl.pEyeDataAppCtrl[lineId]));
	BcmCoreCommandHandler(&cmd);
}

static int setupRawDataFlag = 7;

void BcmAdslCoreDiagSetupRawData(int flag)
{
	XTM_INTERFACE_LINK_INFO linkInfo;
	BCMXTM_STATUS			res;

	setupRawDataFlag = flag;

	if(flag&1)
	{
#if defined(__KERNEL__)
		linkInfo.ulLinkUsRate =  (unsigned int)(uintptr_t) diagCtrl.skbModel->dev;
#endif
		linkInfo.ulLinkState = LINK_START_TEQ_DATA;
	}
	else
	{
		linkInfo.ulLinkState = LINK_STOP_TEQ_DATA;
	}
	res = BcmXtm_SetInterfaceLinkInfo(PORT_TO_PORTID((flag>>1) & 0x3), &linkInfo);
	DiagWriteString(0, DIAG_DSL_CLIENT, "%s: %s TEQ data chanId=%d res=%d\n",
		__FUNCTION__,
		(flag&1)? "Start": "Stop", (flag>>1) & 0x3, res);
}

#ifdef USE_RESERVE_SHARE_MEM
extern struct sk_buff *skb_header_alloc(void);

void BcmCoreDiagZeroCopyStatBufRecycle(struct sk_buff *skb, unsigned long context, uint flags)
{
	//printk("%s: skb=%lx context=%lx flags=%x\n", __FUNCTION__, (uintptr_t)skb,context, flags);
}
void BcmCoreDiagReleaseReserveShareMem(void)
{
	if(NULL != diagCtrl.pEyeDataAppCtrl[0]) {
		BcmCoreDiagZeroCopyStatAppUnInit(diagCtrl.pEyeDataAppCtrl[0]);
		diagCtrl.pEyeDataAppCtrl[0] = NULL;
	}
#ifdef SUPPORT_DSL_BONDING
	if(NULL != diagCtrl.pEyeDataAppCtrl[1]) {
		BcmCoreDiagZeroCopyStatAppUnInit(diagCtrl.pEyeDataAppCtrl[1]);
		diagCtrl.pEyeDataAppCtrl[1] = NULL;
	}
#endif
}

#endif

diagZeroCopyAppCtrlType * BcmCoreDiagZeroCopyStatAppInit(unsigned char lineId, unsigned char logCmd, struct sk_buff *model, int bufSize, int numOfBuf, int dataAlignMask, int frameHeaderLength)
{
	int i, appIndex=0;
	dslDrvSkbPool *pDrvSkbPool;
	diagZeroCopyAppCtrlType *pAppCtrl;
#ifdef USE_RESERVE_SHARE_MEM
	struct sk_buff *skb;
	struct sk_buff ** ppskb;
	adslDmaDesc *pDescRing;
	int alignSbkLen;
	uchar *pData;
#else
	int dmaZone;
	gfp_t gfp_mask;
	if (kerSysGetSdramSize() >= 0x18000000) {
		dmaZone = 1;
		gfp_mask = GFP_ATOMIC | GFP_DMA;
	}
	else {
		dmaZone = 0;
		gfp_mask = GFP_ATOMIC;
	}
#endif
	if(diagCtrl.nZeroCopyAppsActive >= DIAG_ZEROCOPY_APP_MAX) {
		printk("%s() failed! Exceed number of max apps opened(%d)\n", __FUNCTION__, diagCtrl.nZeroCopyAppsActive);
		return NULL;
	}
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	for(i = 0; i < DIAG_ZEROCOPY_APP_MAX; i++) {
		if(NULL == diagCtrl.zeroCopyAppTbl[i]) {
			appIndex = i;
			break;
		}
	}
	
	if(i == DIAG_ZEROCOPY_APP_MAX) {
		printk("%s: No entry available in AppTbl\n", __FUNCTION__);	/* Should not occurred */
		BcmCoreDpcSyncExit(SYNC_RX);
		return NULL;
	}
	
	bufSize += sizeof(uint);	/* Reserve sizeof(uint) for adslDmaDesc->flags */
#ifdef USE_RESERVE_SHARE_MEM
	bufSize = (bufSize + 0xf) & ~0xf;
	alignSbkLen = bufSize + 0x10 + sizeof(struct skb_shared_info);
	alignSbkLen = (alignSbkLen + 0xf) & ~0xf;
	printk("%s: bufSize=%d alignSbkLen=%d sizeof(struct skb_shared_info)=%u\n", __FUNCTION__, bufSize, alignSbkLen, (uint)sizeof(struct skb_shared_info));
	pDescRing = XdslCoreReservedSharedMemAlloc(lineId, ((sizeof(adslDmaDesc) + alignSbkLen) * numOfBuf)+0xf);
	if(NULL == pDescRing) {
		BcmCoreDpcSyncExit(SYNC_RX);
		return NULL;
	}
	pData = (uchar *)(pDescRing + numOfBuf);
	pData = (uchar *)(((uintptr_t)pData+0xf) & ~0xf);
	pDrvSkbPool = (dslDrvSkbPool *) kmalloc(sizeof(dslDrvSkbPool) + sizeof(struct sk_buff **) * numOfBuf, GFP_ATOMIC);
	if(NULL == pDrvSkbPool) {
		printk("%s: Failed to allocate skbPools\n", __FUNCTION__);
		BcmCoreDpcSyncExit(SYNC_RX);
		return NULL;
	}
	pDrvSkbPool->skbPool = (struct sk_buff **)(pDrvSkbPool + 1);
	ppskb = pDrvSkbPool->skbPool;
	for (i = 0; i < numOfBuf; i++) {
		skb = skb_header_alloc();
		if(skb != NULL) {
			skb_headerinit(0, bufSize, skb, pData, (RecycleFuncP)BcmCoreDiagZeroCopyStatBufRecycle, (unsigned long)pData, NULL);
			skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */
			skb->dev = model->dev;
			skb->protocol = htons(eth_type_trans (skb, model->dev));
			skb->data = DIAG_DATA_ALIGN(skb->head, dataAlignMask);
#if 0
			printk("%s: skb=%lx head=%lx data=%lx tail=%lx end=%lx skb_shinfo=%lx,pData=%lx\n",
				__FUNCTION__, (uintptr_t)skb, (uintptr_t)skb->head, (uintptr_t)skb->data,
				(uintptr_t)skb->tail, (uintptr_t)skb->end, (uintptr_t)skb_shinfo(skb), (uintptr_t)pData);
#endif
			memcpy(skb->data, model->data, frameHeaderLength);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
			refcount_set(&skb->users, DIAG_SKB_USERS-1);
#else
			atomic_set(&skb->users, DIAG_SKB_USERS-1);
#endif
			ppskb[i] = skb;
			pData += alignSbkLen;
		} else {
			i--;
			break;
		}
	}
	pDrvSkbPool->skbModel = model;
	pDrvSkbPool->numOfSkbs = i;
	pDrvSkbPool->numOfShortSkbs = 0;
	pDrvSkbPool->skbLengh = bufSize;
	pDrvSkbPool->shortSkbLengh = 0;
	pDrvSkbPool->skbBufIndex = 0;
	pDrvSkbPool->shortSkbBufIndex = pDrvSkbPool->numOfSkbs;
	pDrvSkbPool->frameHeaderLen = frameHeaderLength;
	pDrvSkbPool->dataAlignMask = dataAlignMask;
	pDrvSkbPool->extraSkb = 0;
	pDrvSkbPool->skbHeadRoomReserve = 0;
#else
	bufSize = (bufSize+3) & ~3;
	pDrvSkbPool= DevSkbAllocate(model, bufSize, numOfBuf, 0, 0, dataAlignMask, frameHeaderLength, dmaZone, 0);
	if(NULL == pDrvSkbPool) {
		printk("%s: Failed to allocate skbPools\n", __FUNCTION__);
		BcmCoreDpcSyncExit(SYNC_RX);
		return NULL;
	}
#endif

	pDrvSkbPool->skbLengh -= sizeof(uint);	/* Reserved for pFlag */
#ifdef USE_RESERVE_SHARE_MEM
	pAppCtrl = (diagZeroCopyAppCtrlType *) kmalloc(sizeof(diagZeroCopyAppCtrlType), GFP_ATOMIC);
#else
	pAppCtrl = (diagZeroCopyAppCtrlType *) kmalloc(sizeof(diagZeroCopyAppCtrlType) + sizeof(adslDmaDesc) * pDrvSkbPool->numOfSkbs, gfp_mask);
#endif
	if(NULL == pAppCtrl) {
		printk("%s: Failed to allocate appCtrl\n", __FUNCTION__);
		BcmCoreDpcSyncExit(SYNC_RX);
		DevSkbFree(pDrvSkbPool, 0);
		return NULL;
	}
	
	memset(pAppCtrl, 0, sizeof(diagZeroCopyAppCtrlType));
	pAppCtrl->pBufPool = pDrvSkbPool;
#ifdef USE_RESERVE_SHARE_MEM
	pAppCtrl->pDescRing = pDescRing;
#else
	pAppCtrl->pDescRing = (adslDmaDesc *)(pAppCtrl+1);
#endif

	for(i = 0; i < DIAG_ZEROCOPY_NBUF_MAX(pAppCtrl); i++) {
		struct sk_buff *skb = pAppCtrl->pBufPool->skbPool[i];
		diagSockFrame *dd = (diagSockFrame *) ((uintptr_t)skb->data-DIAG_FRAME_PAD_SIZE);
		uint *pFlag = (uint *)DIAG_ZEROCOPY_PAD_PTR(skb, pAppCtrl);
		dd->diagHdr.logProtoId[0] = '*';
		dd->diagHdr.logProtoId[1] = 'L';
		dd->diagHdr.logPartyId = LOG_PARTY_CLIENT | (lineId << DIAG_PARTY_LINEID_SHIFT);
		dd->diagHdr.logCommmand = logCmd;
#ifdef USE_RESERVE_SHARE_MEM
		pAppCtrl->pDescRing[i].flags = (uint)UNCACHED(ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(pFlag)));
		pAppCtrl->pDescRing[i].addr = (uint)UNCACHED(ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(dd->diagData)));
#else
		pAppCtrl->pDescRing[i].flags = (uint)UNCACHED(ADSL_ENDIAN_CONV_UINT32(ANY_SDRAM_ADDR_TO_ADSL(pFlag)));
		pAppCtrl->pDescRing[i].addr = (uint)UNCACHED(ADSL_ENDIAN_CONV_UINT32(ANY_SDRAM_ADDR_TO_ADSL(dd->diagData)));
#endif
		*pFlag = 0;
		Cache_Flush_Len(skb->data, frameHeaderLength);
		Cache_Flush_Len(pFlag, sizeof(uint));
	}
	Cache_Flush_Len(pAppCtrl->pDescRing, DIAG_ZEROCOPY_NBUF_MAX(pAppCtrl)*sizeof(adslDmaDesc));
#if defined(CONFIG_MIPS)
	pAppCtrl->pDescRing = UNCACHED(pAppCtrl->pDescRing);
#endif
	diagCtrl.nZeroCopyAppsActive++;
	diagCtrl.zeroCopyAppTbl[appIndex] = pAppCtrl;
	BcmCoreDpcSyncExit(SYNC_RX);
	
	printk("%s: lineId=%d pAppCtrl=0x%px pDescRing=0x%px, bufCnt=%d bufLen=%d\n",
		__FUNCTION__, lineId, pAppCtrl, pAppCtrl->pDescRing, DIAG_ZEROCOPY_NBUF_MAX(pAppCtrl), pAppCtrl->pBufPool->skbLengh);
	
	return pAppCtrl;
}

void BcmCoreDiagZeroCopyStatAppUnInit(diagZeroCopyAppCtrlType *pAppCtrl)
{
	int i;
	for(i=0; i< DIAG_ZEROCOPY_APP_MAX; i++) {
		if(pAppCtrl == diagCtrl.zeroCopyAppTbl[i]) {
			struct sk_buff *skb = pAppCtrl->pBufPool->skbPool[0];
			diagSockFrame *dd = (diagSockFrame *) ((uintptr_t)skb->data-DIAG_FRAME_PAD_SIZE);
			uchar lineId = (dd->diagHdr.logPartyId & DIAG_PARTY_LINEID_MASK) >> DIAG_PARTY_LINEID_SHIFT;
			BcmCoreDpcSyncEnter(SYNC_RX);
			diagCtrl.nZeroCopyAppsActive--;
			diagCtrl.zeroCopyAppTbl[i] = NULL;
			BcmCoreDpcSyncExit(SYNC_RX);
			printk("%s: lineId=%d pAppCtrl=0x%px bufIndex=%d bufIndexTxDone=%d wrCnt=%u ovrCnt=%u wrErrCnt=%u maxLpCnt=%u\n",
				__FUNCTION__, lineId, pAppCtrl, pAppCtrl->bufIndex, pAppCtrl->bufIndexTxDone,
				(uint)pAppCtrl->wrCnt, (uint)pAppCtrl->ovrCnt, (uint)pAppCtrl->wrErrCnt, (uint)pAppCtrl->maxLpCnt);
			DevSkbFree(pAppCtrl->pBufPool, 1);
#if defined(USE_RESERVE_SHARE_MEM)
			XdslCoreReservedSharedMemFree(lineId, pAppCtrl->pDescRing);
#endif
			kfree(pAppCtrl);
			break;
		}
	}
}

static
void BcmAdslCoreAllocateBlankSkb( struct sk_buff **ppskb  )
{
    struct sk_buff *pskb = *ppskb;

    if ( pskb ) {
#if 1
        printk("%s: Warning: skb pointer is not null\n",__FUNCTION__);
#else
        *ppskb = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
        refcount_set(&pskb->users, 1);
#else
        atomic_set(&pskb->users, 1);
#endif
        kfree_skb(pskb);
#endif
    }

    pskb = alloc_skb (DIAG_FRAME_HEADER_LEN + 32, GFP_ATOMIC);
    
	pskb->data = pskb->head + DIAG_FRAME_PAD_SIZE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	refcount_set(&pskb->users, DIAG_SKB_USERS);
#else
	atomic_set(&pskb->users, DIAG_SKB_USERS);
#endif

	*ppskb = pskb;
}

struct net_device * BcmAdslCoreDiagInit(PADSL_DIAG pAdslDiag)
{
	struct net_device	*dev = NULL;
	int	dataAlignMask = 0;
    int allocSkbPool = 0;
    dataAlignMask = 3;

    if ( pAdslDiag->srvIpAddr == 0 ) {
        diagCtrl.skbModelReroute = 1;
        if ( ! diagCtrl.skbModel )
            BcmAdslCoreAllocateBlankSkb(&diagCtrl.skbModel);
        diagCtrl.skbModel->dev = NULL;
        // Indicating the corresponding skb pool will be re-routed
        SKB_REROUTE_FIELD(diagCtrl.skbModel) = 0;
        // Preserving client's ip, which is expected to be passed in gwIpAddr field
        SKB_REROUTE_CLIENT_ADDR_FIELD(diagCtrl.skbModel) = pAdslDiag->gwIpAddr;
        // Indicating the corresponding skb pool is for Diag
        SKB_REROUTE_DEST_FIELD(diagCtrl.skbModel) = DIAG_SKB_REROUTE_DATA;
        allocSkbPool = 1 && diagClientsTbl[DIAG_DSL_CLIENT].isRegistered;
    }
    else {
        diagCtrl.skbModelReroute = 0;
        dev = BcmAdslCoreInitNetDev(pAdslDiag, LOG_FILE_PORT, &diagCtrl.skbModel, "Diag");
        allocSkbPool = (dev != NULL) && (diagCtrl.skbModel != NULL) && diagClientsTbl[DIAG_DSL_CLIENT].isRegistered;
    }
    
	diagCtrl.diagDataMap[0] = pAdslDiag->diagMap & 0xFFFF;
	diagCtrl.diagLogTime[0] = pAdslDiag->logTime;

	if (allocSkbPool) {
	  diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev
	    = DevSkbAllocate(diagCtrl.skbModel, SKB_PRE_ALLOC_SIZE, NUM_OF_SKBS_IN_POOL,
                             SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
                             dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
	}
	return dev;
}

struct net_device *BcmAdslCoreGdbInit(PADSL_DIAG pAdslDiag)
{
#ifdef SUPPORT_XDSLDRV_GDB
    struct net_device *dev = NULL;
    
	GdbStubInit();

    if ( pAdslDiag->srvIpAddr == 0 ) {
        diagCtrl.skbGdbReroute = 1;
        if ( ! diagCtrl.skbGdb )
            BcmAdslCoreAllocateBlankSkb(&diagCtrl.skbGdb);
        diagCtrl.skbGdb->dev = NULL;
        // Indicating the corresponding skb pool will be re-routed
        SKB_REROUTE_FIELD(diagCtrl.skbGdb) = 0;
        // Preserving client's ip, which is expected to be passed in gwIpAddr field
        SKB_REROUTE_CLIENT_ADDR_FIELD(diagCtrl.skbGdb) = pAdslDiag->gwIpAddr;
        // Indicating the corresponding skb pool is for GDB
        SKB_REROUTE_DEST_FIELD(diagCtrl.skbGdb) = GDB_SKB_REROUTE_DATA;
    }
    else {
        diagCtrl.skbGdbReroute = 0;
        dev = BcmAdslCoreInitNetDev(pAdslDiag, GDB_PORT, &diagCtrl.skbGdb, "Gdb");
    }
    
    return dev;
#else
	return NULL;
#endif
}

void BcmAdslCoreDiagCommand(unsigned char lineId)
{
	dslCommandStruct	cmd;
#ifdef SUPPORT_DSL_BONDING
	cmd.command = kDslDiagSetupCmd | (lineId << DSL_LINE_SHIFT);
#else
	cmd.command = kDslDiagSetupCmd;
#endif
	cmd.param.dslDiagSpec.setup = 0;
	if (diagCtrl.diagDataMap[lineId] & DIAG_DATA_EYE)
		cmd.param.dslDiagSpec.setup |= kDslDiagEnableEyeData;
	if (diagCtrl.diagDataMap[lineId] & DIAG_DATA_LOG) {
		cmd.param.dslDiagSpec.setup |= kDslDiagEnableLogData;
		diagCtrl.diagDmaLogBlockNum = 0;
		BcmAdslCoreDiagWriteLog(inputSignalData - 2, AC_TRUE);
		diagCtrl.diagDmaLogBlockNum = 0;
		if (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev != NULL)
			diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev->extraSkb = 0;
	}
	if(kDslDiagEnableEyeData & cmd.param.dslDiagSpec.setup)
		BcmAdslCoreDiagSetBufDesc(lineId);
	cmd.param.dslDiagSpec.eyeConstIndex1 = 63; 
	cmd.param.dslDiagSpec.eyeConstIndex2 = 64;
	cmd.param.dslDiagSpec.logTime = diagCtrl.diagLogTime[lineId];
	BcmCoreCommandHandler(&cmd);
	if((0 == cmd.param.dslDiagSpec.setup) && BcmXdslDiagStatSaveLocalIsActive())
		BcmAdslDiagEnablePrintStatCmd();
}

void BcmAdslCoreDiagStartLog_1(unsigned char lineId, uint map, uint time)
{
	diagCtrl.diagDataMap[lineId] = 0;
	if (map & kDslDiagEnableEyeData)
		diagCtrl.diagDataMap[lineId] |= DIAG_DATA_EYE;
	if (map & (kDslDiagEnableLogData | kDslDiagEnableDebugData)) {
		BcmAdslCoreDiagSetupRawData(setupRawDataFlag | 1);
		diagCtrl.diagDataMap[lineId] |= DIAG_DATA_LOG;
		diagCtrl.diagDmaLogBlockNum = 0;
		BcmAdslCoreDiagWriteLog(inputSignalData - 2, AC_TRUE);
		diagCtrl.diagDmaLogBlockNum = 0;
		BcmAdslCoreDiagDataLogNotify(1);
	}
	else
		BcmAdslCoreDiagDataLogNotify(0);

	diagCtrl.diagLogTime[lineId] = time;
	BcmAdslCoreDiagDataInit(lineId);
	if(NULL != diagCtrl.pEyeDataAppCtrl[lineId]) {
		diagZeroCopyAppCtrlType *pAppCtrl = diagCtrl.pEyeDataAppCtrl[lineId];
		if(0==diagCtrl.diagDataMap[lineId])
			printk("%s: map=0x%X time=%d pAppCtrl=0x%px bufIndex=%d wrCnt=%u ovrCnt=%u wrErrCnt=%u maxLpCnt=%u\n",
				__FUNCTION__, (uint)map, (int)time,
				pAppCtrl, pAppCtrl->bufIndex, (uint)pAppCtrl->wrCnt,
				(uint)pAppCtrl->ovrCnt, (uint)pAppCtrl->wrErrCnt, (uint)pAppCtrl->maxLpCnt);
		else
			printk("%s: map=0x%X time=%d\n", __FUNCTION__, (uint)map, (int)time);

	}
}

void BcmAdslCoreDiagStartLog_2(unsigned char lineId, uint map, uint time)
{
	dslCommandStruct	cmd;
#ifdef SUPPORT_DSL_BONDING
	cmd.command = kDslDiagSetupCmd | (lineId << DSL_LINE_SHIFT);
#else
	cmd.command = kDslDiagSetupCmd;
#endif
	cmd.param.dslDiagSpec.setup = map;
	cmd.param.dslDiagSpec.eyeConstIndex1 = 0;
	cmd.param.dslDiagSpec.eyeConstIndex2 = 0;
	cmd.param.dslDiagSpec.logTime = time;
	if(map & (kDslDiagEnableLogData | kDslDiagEnableDebugData))
		BcmAdslDiagSendHdr();
	BcmCoreCommandHandler(&cmd);
	if(0 == map)
		BcmAdslDiagEnablePrintStatCmd();

}


#ifdef PHY_BLOCK_TEST
int BcmCoreDiagTeqLogOrPlaybackActive(void)
{
	return((0 != (diagCtrl.diagDataMap[0] & DIAG_DATA_LOG)) || (ADSL_PHY_SUPPORT(kAdslPhyPlayback)));
}
#endif

int BcmCoreDiagZeroCopyStatAvail(void)
{
	int i, res = 0;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	if(diagCtrl.nZeroCopyAppsActive) {
		for(i = 0; i < DIAG_ZEROCOPY_APP_MAX; i++) {
			diagZeroCopyAppCtrlType *pAppCtrl = diagCtrl.zeroCopyAppTbl[i];
			if(NULL != pAppCtrl) {
				struct sk_buff *skb = DIAG_ZEROCOPY_CURR_SKB(pAppCtrl);
				ushort *pPad = (ushort *)DIAG_ZEROCOPY_PAD_PTR(skb, pAppCtrl);
				Cache_Invalidate_Len(pPad, DIAG_FRAME_PAD_SIZE);
				if(ADSL_ENDIAN_CONV_USHORT(*pPad) & 1) {
					res = 1;
					break;
				}
			}
		}
	}
	BcmCoreDpcSyncExit(SYNC_RX);
	
	return res;
}

int BcmCoreDiagZeroCopyStatActive(void)
{
	return(0 != diagCtrl.nZeroCopyAppsActive);
}

void BcmCoreDiagZeroCopyReturnTxDoneBuffers(diagZeroCopyAppCtrlType *pAppCtrl)
{
	ushort *pPad, pad;
	struct sk_buff *skb;
	int loop=0;
	
	/* Return TxDone buffers */
	skb = DIAG_ZEROCOPY_CURR_TXDONE_SKB(pAppCtrl);
	pPad = (ushort *)DIAG_ZEROCOPY_PAD_PTR(skb, pAppCtrl);
	Cache_Invalidate_Len(pPad, DIAG_FRAME_PAD_SIZE);
	pad = ADSL_ENDIAN_CONV_USHORT(*pPad);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
	while((0 != pad) && (DIAG_SKB_USERS != refcount_read(&skb->users))) {
#else
	while((0 != pad) && (DIAG_SKB_USERS != atomic_read(&skb->users))) {
#endif
		if(0 == (pad & 1)) {
			*pPad = 0;
			Cache_Flush_Len(pPad, DIAG_FRAME_PAD_SIZE);
			if(++pAppCtrl->bufIndexTxDone >= DIAG_ZEROCOPY_NBUF_MAX(pAppCtrl))
				pAppCtrl->bufIndexTxDone = 0;
			loop++;
			skb = DIAG_ZEROCOPY_CURR_TXDONE_SKB(pAppCtrl);
			pPad = (ushort *)DIAG_ZEROCOPY_PAD_PTR(skb, pAppCtrl);
			Cache_Invalidate_Len(pPad, DIAG_FRAME_PAD_SIZE);
			pad = ADSL_ENDIAN_CONV_USHORT(*pPad);
		}
		else
			break;
	}
	
	if(loop > 13) {
		DiagWriteString(0, DIAG_DSL_CLIENT, "BcmCoreDiagZeroCopyReturnTxDoneBuffers: bufIndex=%d bufIndexTxDone=%d wrCnt=%u ovrCnt=%u wrErrCnt=%u retSkbNum=%d\n",
			pAppCtrl->bufIndex, pAppCtrl->bufIndexTxDone, pAppCtrl->wrCnt, pAppCtrl->ovrCnt, pAppCtrl->wrErrCnt, loop);
	}

}

#ifdef CONFIG_BCM963146
#define EYE_DATA_WORKAROUND
#endif

void BcmCoreDiagZeroCopyFrameSend(diagZeroCopyAppCtrlType *pAppCtrl)
{
	diagSockFrame *dd;
	struct sk_buff *skb;
	int len, n, loop=0;
	ushort *pPad, pad;
#ifdef EYE_DATA_WORKAROUND
	struct sk_buff *newSkb;
#endif
	if(NULL == diagCtrl.dbgDev)
		return;
	
	BcmCoreDiagZeroCopyReturnTxDoneBuffers(pAppCtrl);
	
	/* Check for buffer full to send to Diags */
	skb = DIAG_ZEROCOPY_CURR_SKB(pAppCtrl);
	pPad = (ushort *)DIAG_ZEROCOPY_PAD_PTR(skb, pAppCtrl);
	Cache_Invalidate_Len(pPad, DIAG_FRAME_PAD_SIZE);
	pad = ADSL_ENDIAN_CONV_USHORT(*pPad);
	
	while( pad & 1 ) {
		loop++;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
		if(DIAG_SKB_USERS != refcount_read(&skb->users))
#else
		if(DIAG_SKB_USERS != atomic_read(&skb->users))
#endif
		{
			len = pad >> 1;
			dd = (diagSockFrame *) ((uintptr_t)skb->data-DIAG_FRAME_PAD_SIZE);
#ifdef EYE_DATA_WORKAROUND
			newSkb = NULL;
			BcmCoreDpcSyncEnter(SYNC_DIAGS);
			if (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev != NULL) {
				newSkb = GetSkb(diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev, len);
				if (newSkb != NULL) {
					diagSockFrame *dd1 = (diagSockFrame *) ((uintptr_t)newSkb->data-DIAG_FRAME_PAD_SIZE);
					memcpy((void*)&dd1->diagHdr, (void*)&dd->diagHdr, sizeof(LogProtoHeader) + len);
					DiagUpdateDataLen(dd1, len);
					newSkb->len = DIAG_FRAME_HEADER_LEN + len;
					skb_set_tail_pointer(newSkb, newSkb->len);
					n = DEV_TRANSMIT(newSkb);
				}
			}
			BcmCoreDpcSyncExit(SYNC_DIAGS);
			*pPad = ADSL_ENDIAN_CONV_USHORT((ushort)len << 1);
			if(NULL == newSkb)
				n = 1;
#else
			Cache_Invalidate_Len(dd->diagData, len);
			DiagUpdateDataLen(dd, len);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
			refcount_set(&skb->users, DIAG_SKB_USERS);
#else
			atomic_set(&skb->users, DIAG_SKB_USERS);
#endif
			skb->dev = diagCtrl.dbgDev;
			skb->len = DIAG_FRAME_HEADER_LEN + len;
			skb_set_tail_pointer(skb, skb->len);
			Cache_Flush_Len(skb->data, DIAG_FRAME_HEADER_LEN);
			*pPad = ADSL_ENDIAN_CONV_USHORT((ushort)len << 1);
			Cache_Flush_Len(pPad, DIAG_FRAME_PAD_SIZE);
			n = DEV_TRANSMIT(skb);
#endif
			if ( 0 != n)
				pAppCtrl->wrErrCnt++;
			else
				pAppCtrl->wrCnt++;
		}
		else {
			pAppCtrl->ovrCnt++;
			break;
		}
		
		if(++pAppCtrl->bufIndex >= DIAG_ZEROCOPY_NBUF_MAX(pAppCtrl))
			pAppCtrl->bufIndex = 0;
		skb = DIAG_ZEROCOPY_CURR_SKB(pAppCtrl);
		pPad = (ushort *)DIAG_ZEROCOPY_PAD_PTR(skb, pAppCtrl);
		Cache_Invalidate_Len(pPad, DIAG_FRAME_PAD_SIZE);
		pad = ADSL_ENDIAN_CONV_USHORT(*pPad);
	}
	
	if(loop > pAppCtrl->maxLpCnt) {
		DiagWriteString(0, DIAG_DSL_CLIENT, "bufIndex=%d bufIndexTxDone=%d wrCnt=%u ovrCnt=%u wrErrCnt=%u maxLpCnt %u loop=%u\n",
			pAppCtrl->bufIndex, pAppCtrl->bufIndexTxDone, pAppCtrl->wrCnt, pAppCtrl->ovrCnt, pAppCtrl->wrErrCnt, pAppCtrl->maxLpCnt, loop);
		pAppCtrl->maxLpCnt = loop;
	}
}

void BcmCoreDiagZeroCopyTxDoneHandler(void)
{
	int i;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	if(diagCtrl.nZeroCopyAppsActive) {
		for(i = 0; i < DIAG_ZEROCOPY_APP_MAX; i++) {
			if(NULL != diagCtrl.zeroCopyAppTbl[i])
				BcmCoreDiagZeroCopyReturnTxDoneBuffers(diagCtrl.zeroCopyAppTbl[i]);
		}
	}
	BcmCoreDpcSyncExit(SYNC_RX);
}

void BcmCoreDiagZeroCopyStatHandler(void)
{
	int i;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	if(diagCtrl.nZeroCopyAppsActive) {
		for(i = 0; i < DIAG_ZEROCOPY_APP_MAX; i++) {
			if(NULL != diagCtrl.zeroCopyAppTbl[i])
				BcmCoreDiagZeroCopyFrameSend(diagCtrl.zeroCopyAppTbl[i]);
		}
	}
	BcmCoreDpcSyncExit(SYNC_RX);
}

int BcmAdslDiagGetConstellationPoints (int toneId, void *pointBuf, int numPoints)
{

	if (0 == diagCtrl.diagDataMap[0]) {
		diagCtrl.diagDataMap[0] = DIAG_DATA_EYE;
		diagCtrl.diagLogTime[0] = 0;
		BcmAdslCoreDiagDataInit(0);
		BcmAdslCoreDiagCommand(0);
		return 0;
	}
	
	return numPoints;
}

#ifdef PHY_PROFILE_SUPPORT
extern void BcmAdslCoreProfilingStop(void);
#endif


char	savedStatFileName[] = "savedStatusFile.bin";
char	successStr[] = "SUCCESS";
char	failStr[] = "FAIL";

static Boolean diagLockSession = 0;
static Boolean diagRegressionLock = 0;

void BcmAdslDiagDisconnect(int keepDiagConInfo)
{
	dslDrvSkbPool * pSkbDev;
	int clientType;
	
	BcmCoreDpcSyncEnter(SYNC_RX);
	diagCtrl.dbgDev = NULL;
	if(!keepDiagConInfo)
		memset((void*)&diagConnectInfo, 0, sizeof(diagConnectInfo));
	diagCtrl.diagDataMap[0] = 0;
#ifdef SUPPORT_DSL_BONDING
	diagCtrl.diagDataMap[1] = 0;
#endif
	diagLockSession = 0;
	diagRegressionLock = 0;
	BcmCoreDpcSyncExit(SYNC_RX);

	if (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev != NULL) {
		BcmAdslCoreDiagCommand(0);
#ifdef SUPPORT_DSL_BONDING
	if(ADSL_PHY_SUPPORT(kAdslPhyBonding))
		BcmAdslCoreDiagCommand(1);
#endif
#ifdef PHY_PROFILE_SUPPORT
		BcmAdslCoreProfilingStop();
#endif
		BcmAdslCoreDiagDataLogNotify(0);
	}
	
	for(clientType = 0; clientType < 4; clientType++) {
		pSkbDev = diagClientsTbl[clientType].diagSkbDev;
		if (pSkbDev) {
            struct sk_buff* model = pSkbDev->skbModel;
			diagClientsTbl[clientType].pCallback(0, LOG_CMD_DISABLE_CLIENT, 0, NULL);
			diagClientsTbl[clientType].diagSkbDev = NULL;
			DevSkbFree(pSkbDev, 1);

            if ( model == diagCtrl.skbModel )
                diagCtrl.skbModelReroute = 0;
            else if ( model == diagCtrl.skbGdb )
                diagCtrl.skbGdbReroute = 0;
            else
                printk("%s: Warning: unrecognized skb model\n",__FUNCTION__);
		}
	}
	
	if(NULL != diagCtrl.pEyeDataAppCtrl[0]) {
		BcmCoreDiagZeroCopyStatAppUnInit(diagCtrl.pEyeDataAppCtrl[0]);
		diagCtrl.pEyeDataAppCtrl[0] = NULL;
	}
#ifdef SUPPORT_DSL_BONDING
	if(NULL != diagCtrl.pEyeDataAppCtrl[1]) {
		BcmCoreDiagZeroCopyStatAppUnInit(diagCtrl.pEyeDataAppCtrl[1]);
		diagCtrl.pEyeDataAppCtrl[1] = NULL;
	}
#endif
}

#ifdef SUPPORT_VECTORING
extern void BcmXdslDumpVectSkbDevInfo(unsigned char lineId);
#endif

void BcmAdslCoreDiagClearStats(void)
{
	if (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev != NULL)
		diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev->extraSkb = 0;
}

void BcmAdslCoreDiagPrintStats(unsigned char lineId)
{
	int extraSkbCount = 0, numberOfSkbs = 0, numberOfShortSkbs = 0;

	if (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev != NULL) {
		extraSkbCount = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev->extraSkb;
		numberOfSkbs = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev->numOfSkbs;
		numberOfShortSkbs = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev->numOfShortSkbs;
	}
	BcmAdslCoreDiagWriteStatusString(lineId,
			"DiagLinux Statistics:\n"
			"   numberOfSkbs	= %d\n"
			"   numberOfShortSkbs	= %d\n"
			"   extraSkbCount	= %d\n",
			numberOfSkbs,
			numberOfShortSkbs,
			extraSkbCount);
#ifdef SUPPORT_VECTORING
	BcmXdslDumpVectSkbDevInfo(lineId);
#endif
}

#if defined(USE_PMC_API)

int BcmXdslReadAfePLLMdiv(uchar lineId,uint32 *pCh01_cfg, uint32 *pCh45_cfg)
{
	int ret;
	
	if(kPMC_NO_ERROR != (ret =ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset (ch01_cfg), pCh01_cfg))) {
		BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch01_cfg error(%d)\n", ret);
		return ret;
	}
	
	if(kPMC_NO_ERROR != (ret =ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset (ch45_cfg), pCh45_cfg)))
		BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read pllch45_cfg error(%d)\n", ret);
	
	return ret;
}

void BcmXdslWriteAfePLLMdiv(uchar lineId, uint32 param1, uint32 param2)
{
	int ret;
	PLL_CHCFG_REG pllch01_cfg;
	PLL_CHCFG_REG pllch45_cfg;
	
	if(kPMC_NO_ERROR != (ret =ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset (ch01_cfg), &pllch01_cfg.Reg32))) {
		BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch01_cfg error(%d)\n", ret);
		return;
	}
	if(kPMC_NO_ERROR != (ret =ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset (ch45_cfg), &pllch45_cfg.Reg32))) {
		BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch45_cfg error(%d)\n", ret);
		return;
	}

	pllch01_cfg.Bits.mdiv0 = param1&0xFF;	//This should be the default value anyway from Ch0 Mdiv.  8 bit field to change div value
	pllch01_cfg.Bits.mdiv_override0 = 1;	//Override bit that allows the mdiv value to be seen by the PLL.

	pllch45_cfg.Bits.mdiv1 = param2&0xFF;	//This should now change the value of Ch5 of the PLL,  the default value of this should be 0x3.
	pllch45_cfg.Bits.mdiv_override1 = 1;	//Override bit that allows the mdiv value to be seen by the PLL. 

	if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch01_cfg), pllch01_cfg.Reg32))) {
		BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch01_cfg error(%d)\n", ret);
		return;
	}
	if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch45_cfg), pllch45_cfg.Reg32)))
		BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch45_cfg error(%d)\n", ret);
}

#endif

#if defined(SUPPORT_MULTI_PHY) && defined(CONFIG_BCM963268)
extern void BcmXdslCorePrintCurrentMedia(void);
#endif

static uintptr_t BcmAdslCoreDiagAddrConv(uintptr_t addr)
{
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)
	if(((addr&0xff000000)==0x80000000) || ((addr&0xff000000)==0xff000000)) {
#if defined(CONFIG_BCM963158)
		if((addr&0xff000000)==0x80000000)
			addr = (addr - DSLPHY_PHYS_BASE) + (uintptr_t)DSLPHY_BASE;	/* Assume DSL register address */
		else
			addr = (addr - PERF_PHYS_BASE) + (uintptr_t)PERF_BASE;	/* Assume PERF address */
#else
		addr = (uintptr_t)BCM_IO_ADDR(addr);	/* REG/PER address */
#endif
	}
	else
#endif
	if (ADSL_MIPS_LMEM_ADDR(addr) || ADSL_MIPS_SDRAM_ADDR(addr))
		addr = (uintptr_t)(ADSL_ADDR_TO_HOST(addr));
	/* else assumming Host address */
	
	return addr;
}

static void BcmAdslCoreDebugReadMem(unsigned char lineId, DiagDebugData *pDbgCmd)
{
	ULONG				n, frCnt, dataSize;
	DiagProtoFrame		*pDiagFrame;
	DiagDebugData		*pDbgRsp;
	char			testFrame[512], *pData, *pMem, *pMemEnd;

	pDiagFrame	= (void *) testFrame;
	pDbgRsp		= (void *) pDiagFrame->diagData;
	pData		= (char *) pDbgRsp->diagData;
	*(short *)pDiagFrame->diagHdr.logProtoId = *(short *) LOG_PROTO_ID;
	pDiagFrame->diagHdr.logPartyId	= LOG_PARTY_CLIENT | (lineId << DIAG_PARTY_LINEID_SHIFT);
	pDiagFrame->diagHdr.logCommmand = LOG_CMD_DEBUG;

	pDbgRsp->cmd = ADSL_ENDIAN_CONV_SHORT(pDbgCmd->cmd);
	pDbgRsp->cmdId = ADSL_ENDIAN_CONV_SHORT(pDbgCmd->cmdId);
	if(CMDID_READ_MEM_CHIPID == pDbgCmd->cmdId) {
		pDbgRsp->param1 = ADSL_ENDIAN_CONV_INT32(pDbgCmd->param1);
		pDbgRsp->param2 = ADSL_ENDIAN_CONV_INT32(8);
		*(uint *)pData = ADSL_ENDIAN_CONV_INT32(PERF->RevID);
		*((uint *)pData+1) = ADSL_ENDIAN_CONV_INT32(PERF->RevID >> CHIP_ID_SHIFT);
		if (BcmAdslDiagIsActive())
			BcmAdslCoreDiagWrite(testFrame, (pData - testFrame) + 8);
		else
			BcmAdslCoreDebugPrintToConsole(pDbgCmd, pData, 8);
		return;
	}
	dataSize = sizeof(testFrame) - (pData - testFrame) - 16;
	
	pMem = (char *)BcmAdslCoreDiagAddrConv(pDbgCmd->param1);
	pDbgCmd->param1 = (uint)(uintptr_t)pMem;
	pMemEnd = pMem + pDbgCmd->param2;
	pDbgCmd->param2 = (uint)(uintptr_t)pMemEnd;
	frCnt = 0;
	while (pMem != pMemEnd) {
		n = pMemEnd - pMem;
		if (n > dataSize)
			n = dataSize;

		pDbgRsp->param1 = ADSL_ENDIAN_CONV_INT32((uint)(uintptr_t)pMem);
		pDbgRsp->param2 = ADSL_ENDIAN_CONV_INT32(n);
#ifdef CONFIG_ARM64
		BlockByteMove(n, pMem, pData);
#else
		memcpy (pData, pMem, n);
#endif
		if (BcmAdslDiagIsActive())
			BcmAdslCoreDiagWrite(testFrame, (pData - testFrame) + n);
		else {
			pDbgCmd->param1 = (uint)(uintptr_t)pMem;
			BcmAdslCoreDebugPrintToConsole(pDbgCmd, pData, n);
		}
		pMem += n;
		frCnt = (frCnt + 1) & 0xFF;

		if (BcmAdslDiagIsActive() && (0 == (frCnt & 7)))
			BcmAdslCoreDelay(40);
	}
}

/* Same condition as for pXdslDummyDevice initialization in BcmAdslCore.c; will not be true for 63178  */
#if defined(CONFIG_ARM64) || defined(CONFIG_PHY_PARAM) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define USE_DMA_COHERENT	1
#endif

void *BcmAdslAllocPhyMem(uint size, void **ppPhysAddr)
{
	void		*p;
#ifdef ADSL_SDRAM_RESERVE_SIZE
	return AdslCoreSystemReservedMemAlloc(size, ppPhysAddr);
#elif !defined(USE_DMA_COHERENT)
	if (kerSysGetSdramSize() >= 0x18000000)
	  p = kmalloc(size, GFP_ATOMIC | GFP_DMA);
	else
	  p = calloc (1, size);
	if (NULL != p)
	  *ppPhysAddr = (void *) virt_to_phys(p);
#else
	dma_addr_t  physAddr;

	p = dma_alloc_coherent(pXdslDummyDevice, size, &physAddr, GFP_KERNEL);
	*ppPhysAddr = (void *) physAddr;
	//printk("BcmAdslAllocPhyMem: dev=0x%px p=0x%px phyAddr=0x%llX\n", pXdslDummyDevice, p, physAddr);
#endif
	return p;
}

void  BcmAdslFreePhyMem(void *addr, void *physAddr, uint size)
{
#ifdef ADSL_SDRAM_RESERVE_SIZE
	AdslCoreSystemReservedMemFree(addr, physAddr, size);
#elif !defined(USE_DMA_COHERENT)
	if (kerSysGetSdramSize() >= 0x18000000)
	  kfree(addr);
	else
	  free(addr);
#else
	dma_addr_t phys_addr = (dma_addr_t)physAddr;

	//printk("BcmAdslFreePhyMem: p=0x%px phyAddr=0x%llX size=%u\n", addr, phys_addr, size);
	dma_free_coherent(pXdslDummyDevice, size, addr, phys_addr); 
#endif
}

#ifdef SECONDARY_AFEID_FN
extern unsigned int	overrideSecondaryAfeId[];
#endif

void BcmAdslCoreDiagCmd(unsigned char lineId, PADSL_DIAG pAdslDiag)
{
	uint	origDiagMap, map, clientType;
	int		dstPort;
	diagSockFrame	*dd;
	struct net_device	*pDev = NULL;
	char		buf[16], tmpStr[128];

#if 0 && defined(__ECOS)
	printk("BcmAdslCoreDiagCmd: cmd=%d, map=0x%X, logTime=0x%X, srvIp=0x%X, sock=0x%X\n", 
		pAdslDiag->diagCmd, pAdslDiag->diagMap, pAdslDiag->logTime, pAdslDiag->srvIpAddr, pAdslDiag->gwIpAddr);
#endif
	BcmAdslCoreDiagCmdNotify();
	dstPort = ((uint) pAdslDiag->diagMap) >> 16;
	map = (uint) pAdslDiag->diagMap & 0xFFFF;
	clientType = (uint) (((pAdslDiag->diagCmd & DIAG_TYPE_CMD_MASK) >> DIAG_TYPE_CMD_SHIFT) & 0x3);
	pAdslDiag->diagCmd &= (~DIAG_TYPE_CMD_MASK);

	switch (pAdslDiag->diagCmd) {
		case LOG_CMD_DIAG_CONNECT_INFO:
			memcpy((void *)&diagConnectInfo, (void *)pAdslDiag->diagMap, sizeof(diagConnectInfo));
			printk ("From DslDiagd: dstMACAddr = %02X:%02X:%02X:%02X:%02X:%02X, devName = %s\n",
				diagConnectInfo.macAddr[0], diagConnectInfo.macAddr[1], diagConnectInfo.macAddr[2],
				diagConnectInfo.macAddr[3], diagConnectInfo.macAddr[4], diagConnectInfo.macAddr[5],
				diagConnectInfo.devName);
			break;
		case LOG_CMD_CONNECT:
        {
            int skb_reroute = (pAdslDiag->srvIpAddr == 0);
			origDiagMap = diagCtrl.diagDataMap[lineId];
			printk("CONNECT request from srvIpAddr=%s, dstPort=%d, diagMap=0x%08X\n",
				ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr), buf), dstPort, (uint)map);
			if((0 != diagConnectInfo.devName[0]) &&
				(0 == (diagConnectInfo.macAddr[0] | diagConnectInfo.macAddr[1] | diagConnectInfo.macAddr[2] | diagConnectInfo.macAddr[3] | diagConnectInfo.macAddr[4] | diagConnectInfo.macAddr[5]))) {
				printk("*** Skip processing CONNECT request due to invalid dstMACAddr! ***\n");
				break;
			}
			
			printk ("***CONNECT: DSL:%d, WLAN:%d, XTM:%d\n",
					diagClientsTbl[0].isRegistered,
					diagClientsTbl[1].isRegistered,
					diagClientsTbl[2].isRegistered);


			if (pAdslDiag->diagMap & DIAG_DATA_GDB_ID) {
				BcmCoreDpcSyncEnter(SYNC_RX);
				pDev = BcmAdslCoreGdbInit(pAdslDiag);
				if( skb_reroute || (NULL != pDev) )
					diagCtrl.gdbDev = pDev;
				BcmCoreDpcSyncExit(SYNC_RX);
				return;
			}
			else {
				if(NULL != diagCtrl.skbModel)
					dd = (diagSockFrame *) DiagPacketStart(diagCtrl.skbModel);
				else
					dd = NULL;

				if ((0 == diagLockSession) ||
					((NULL != diagCtrl.dbgDev) && (NULL != dd) && ( pAdslDiag->srvIpAddr == dd->ipHdr.dstAddr ))) {
					pAdslDiag->diagMap = (int)((uint)pAdslDiag->diagMap & ~((uint)DIAG_LOCK_SESSION));
					if((NULL != diagCtrl.dbgDev) && (NULL != dd) && (pAdslDiag->srvIpAddr == dd->ipHdr.dstAddr)) {
						printk("Re-connecting to srvIpAddr %s\n", ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr),buf));
						BcmAdslDiagDisconnect(1);
					}
					else
						printk("Connecting to srvIpAddr %s\n", ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr),buf));
					
					BcmCoreDpcSyncEnter(SYNC_RX);
					pDev = BcmAdslCoreDiagInit(pAdslDiag);
					if(skb_reroute || ((NULL != pDev) && (NULL != diagCtrl.skbModel))) {
						diagLockSession = 1;
						diagCtrl.dbgDev = pDev;
					}
					BcmCoreDpcSyncExit(SYNC_RX);
					
					sprintf(tmpStr, "%s: pDev=0x%px diagCtrl.skbModel=0x%px\n", __FUNCTION__, pDev, diagCtrl.skbModel);
					printk("%s", tmpStr);
					DiagWriteString(lineId, DIAG_DSL_CLIENT, tmpStr);
				}
				else {
					static struct sk_buff *skbModelNew = NULL;
					struct sk_buff *skbModelTmp = diagCtrl.skbModel;
					dslDrvSkbPool *pDrvSkbPoolTmp = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev;
					dslDrvSkbPool *pDrvSkbPoolTmp1 = NULL;
					pDev = diagCtrl.dbgDev;
#if 1
                    if ( skb_reroute )
                        printk("%s: skb_reroute not handling this yet(reconn)\n",__FUNCTION__);
#endif
					sprintf(tmpStr, "Reject connection request from srvIpAddr %s\n", ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr),buf));
					printk("%s", tmpStr);
					DiagWriteString(lineId, DIAG_DSL_CLIENT, tmpStr);
					
					sprintf(tmpStr, "Connection is being locked by srvIpAddr %s\n", (dd) ? ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(dd->ipHdr.dstAddr),buf): "Unknown" );
					printk("%s", tmpStr);
					DiagWriteString(lineId, DIAG_DSL_CLIENT, tmpStr);
					
					sprintf(tmpStr,"Temporarily connect to rejected srvIpAddr %s to deliver unlock instructions\n", ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr),buf));
					printk("%s", tmpStr);
					DiagWriteString(lineId, DIAG_DSL_CLIENT, tmpStr);
					
					BcmCoreDpcSyncEnter(SYNC_RX);
					if((NULL != pDrvSkbPoolTmp) && (NULL != (diagCtrl.dbgDev = BcmAdslCoreInitNetDev(pAdslDiag, LOG_FILE_PORT, &skbModelNew, "Diag")))) {
						diagCtrl.skbModel = skbModelNew;
						diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = DevSkbAllocate(diagCtrl.skbModel, 0, 0,
							SHORT_SKB_PRE_ALLOC_SIZE, 5, pDrvSkbPoolTmp->dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
						DiagWriteString(lineId, DIAG_DSL_CLIENT, "Connection request is rejected!!!\n");
						DiagWriteString(lineId, DIAG_DSL_CLIENT, "Connection is being locked by srvIpAddr %s\n", (dd) ? ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(dd->ipHdr.dstAddr),buf): "Unkown");
						if(!diagRegressionLock)
							DiagWriteString(lineId, DIAG_DSL_CLIENT, "To establish a new connection, unlock the current connection(File Menu)\n");
						else
							DiagWriteString(lineId, DIAG_DSL_CLIENT, "The target is being locked for regression testing, please contact the owner\n");
						
						pDrvSkbPoolTmp1 = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev;
						
						diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = pDrvSkbPoolTmp;
						diagCtrl.skbModel = skbModelTmp;
						diagCtrl.dbgDev = pDev;
					}
					BcmCoreDpcSyncExit(SYNC_RX);
					
					if(NULL != pDrvSkbPoolTmp1)
						DevSkbFree(pDrvSkbPoolTmp1, 1);
					return;
				}
			}
			
			if ( adslCoreInitialized && ( (NULL != diagCtrl.dbgDev) || (skb_reroute && diagCtrl.skbModel) ) ) {
				adslVersionInfo		verInfo;
				adslMibInfo		*pAdslMib;
				long		mibLen;

				if (diagCtrl.diagDataMap[lineId] & DIAG_DATA_LOG) {
					BcmAdslCoreReset(diagCtrl.diagDataMap[lineId]);
					BcmAdslCoreDiagDataLogNotify(1);
				}
				else {
					BcmAdslCoreDiagDataInit(lineId);
					if (0 == origDiagMap) {
						BcmAdslCoreDiagCommand(lineId);
					}
				}

				BcmAdslCoreGetVersion(&verInfo);
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "ADSL version info: PHY=%s, Drv=%s",
					verInfo.phyVerStr, verInfo.drvVerStr);

				mibLen = sizeof(adslMibInfo);
				pAdslMib = (void *)BcmAdslCoreGetObjectValue (lineId, NULL, 0, NULL, &mibLen);
				if(NULL != pAdslMib) {
					diagSockFrame *pDiag = DiagPacketStart(diagCtrl.skbModel);

					DiagWriteString(lineId, DIAG_DSL_CLIENT, "MACAddress_AFEID: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n",
						pDiag->srcMacAddr[0], pDiag->srcMacAddr[1], pDiag->srcMacAddr[2],
						pDiag->srcMacAddr[3], pDiag->srcMacAddr[4], pDiag->srcMacAddr[5],
						pAdslMib->afeId[lineId]);
				}

				BcmXdslCoreDiagSendPhyInfo();
#if defined(SUPPORT_MULTI_PHY) && defined(CONFIG_BCM963268)
				BcmXdslCorePrintCurrentMedia();
#endif
				BcmXdslCoreSendAfeInfo(0);	/* Send AFE info to DslDiags */
				BcmAdslDiagSendHdr();
			}
        } /* end of case LOG_CMD_CONNECT */
			break;
		case LOG_CMD_ENABLE_CLIENT: /* sent after connect command */
		{
			char clientId;
			int	dataAlignMask;
			dataAlignMask = 3;
			if (diagCtrl.skbModel != NULL) {
				clientId = *((char *)pAdslDiag->diagMap);
				if ((clientId & DIAG_DATA_DSL_ID) && diagClientsTbl[DIAG_DSL_CLIENT].isRegistered &&
						!diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev) {
					printk("DSL Diag Enable Request Received\n");
					diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = DevSkbAllocate(diagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
							NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
							dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
					diagClientsTbl[DIAG_DSL_CLIENT].pCallback(lineId, LOG_CMD_ENABLE_CLIENT, 0, NULL);
				}
				if ((clientId & DIAG_DATA_XTM_ID) && diagClientsTbl[DIAG_XTM_CLIENT].isRegistered &&
						!diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev) {
					printk("XTM Diag Enable Request Received\n");
					diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev = DevSkbAllocate(diagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
							NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
							dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
					diagClientsTbl[DIAG_XTM_CLIENT].pCallback(lineId, LOG_CMD_ENABLE_CLIENT, 0, NULL);
				}
				if ((clientId & DIAG_DATA_WLAN_ID) && diagClientsTbl[DIAG_WLAN_CLIENT].isRegistered &&
						!diagClientsTbl[DIAG_WLAN_CLIENT].diagSkbDev) {
					printk("WLAN Diag Enable Request Received\n");
					diagClientsTbl[DIAG_WLAN_CLIENT].diagSkbDev = DevSkbAllocate(diagCtrl.skbModel, SKB_PRE_ALLOC_SIZE,
							NUM_OF_SKBS_IN_POOL, SHORT_SKB_PRE_ALLOC_SIZE, NUM_OF_SHORT_SKBS_IN_POOL,
							dataAlignMask, DIAG_FRAME_HEADER_LEN, 0, 0);
					diagClientsTbl[DIAG_WLAN_CLIENT].pCallback(lineId, LOG_CMD_ENABLE_CLIENT, 0, NULL);
				}
			}
		}
		break;
		case LOG_CMD_DISABLE_CLIENT:
		{
			dslDrvSkbPool * pSkbDev;
			char clientId;

			clientId = *((char *)pAdslDiag->diagMap);
			if ((clientId & DIAG_DATA_DSL_ID) && (diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev)) {
				printk("DSL Diag Disable Request Received\n");
				diagClientsTbl[DIAG_DSL_CLIENT].pCallback(lineId, LOG_CMD_DISABLE_CLIENT, 0, NULL);
				pSkbDev = diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev;
				diagClientsTbl[DIAG_DSL_CLIENT].diagSkbDev = NULL;
				DevSkbFree(pSkbDev,1);
			}
			if ((clientId & DIAG_DATA_WLAN_ID) && (diagClientsTbl[DIAG_WLAN_CLIENT].diagSkbDev)) {
				printk("WLAN Diag Disable Request Received\n");
				diagClientsTbl[DIAG_WLAN_CLIENT].pCallback(lineId, LOG_CMD_DISABLE_CLIENT, 0, NULL);
				pSkbDev = diagClientsTbl[DIAG_WLAN_CLIENT].diagSkbDev;
				diagClientsTbl[DIAG_WLAN_CLIENT].diagSkbDev = NULL;
				DevSkbFree(pSkbDev,1);
			}
			if ((clientId & DIAG_DATA_XTM_ID) && (diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev)) {
				printk("XTM Diag Disable Request Received\n");
				diagClientsTbl[DIAG_XTM_CLIENT].pCallback(lineId, LOG_CMD_DISABLE_CLIENT, 0, NULL);
				pSkbDev = diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev;
				diagClientsTbl[DIAG_XTM_CLIENT].diagSkbDev = NULL;
				DevSkbFree(pSkbDev,1);
			}
		}
		break;
		case LOG_CMD_PING_REQ:
		{
			uint tempclientType = 0;
			for (tempclientType = 0; tempclientType < 4; tempclientType++) {
				if (diagClientsTbl[tempclientType].diagSkbDev)
					break;
			}
			DiagWriteData(((tempclientType << DIAG_TYPE_SHIFT) | LOG_CMD_PING_RSP), buf, 2, NULL, 0);
		}
		break;
		case LOG_CMD_DISCONNECT:
        {
            int ip_mismatch = 0;
			printk ("DISCONNECT Request(%s) from srvIpAddr=%s diagLockSession=%d regressionLock=%d\n",
				(map & DIAG_FORCE_DISCONNECT) ? "forced": "not-forced",
				ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr), buf),
				diagLockSession, diagRegressionLock);
			
			if( ! BcmAdslDiagIsConnected() ) {
				printk ("CPE is already in Diags disconnected state!\n");
				break;
			}
			
			if(NULL != diagCtrl.skbModel) {
				dd = (diagSockFrame *) DiagPacketStart(diagCtrl.skbModel);
                if ( diagCtrl.skbModelReroute )
                    ip_mismatch = (pAdslDiag->srvIpAddr != SKB_REROUTE_CLIENT_ADDR_FIELD(diagCtrl.skbModel));
                else
                    ip_mismatch = (pAdslDiag->srvIpAddr != dd->ipHdr.dstAddr);
            }
			else
				dd = NULL;
			
			if( (NULL != dd) && ip_mismatch )
				DiagWriteString(lineId, DIAG_DSL_CLIENT, "DISCONNECT Request(%s) from srvIpAddr = %s diagLockSession=%d regressionLock=%d\n",
					(map & DIAG_FORCE_DISCONNECT) ? "forced": "not-forced",
					ConvertToDottedIpAddr(ADSL_ENDIAN_CONV_INT32(pAdslDiag->srvIpAddr), buf), diagLockSession, diagRegressionLock);
			
			if( 0 == (map & DIAG_FORCE_DISCONNECT) ) {
				if( diagLockSession && (NULL != dd) ){
					if ( ip_mismatch ) {
						break;	/* Reject DISCONNECT */
                    }
				}
			}
			else if( diagRegressionLock && (NULL != dd) ){
				if( ip_mismatch ) {
					break;	/* Reject DISCONNECT */
                }
			}
			
			BcmAdslDiagDisconnect(0);
        } // end of case LOG_CMD_DISCONNECT
			break;
		case LOG_CMD_DEBUG:
		{
			DiagDebugData	*pDbgCmd = (void *)pAdslDiag->diagMap;
			
			printk("%s: Executing dbgcmd=%d 0x%08x 0x%08x\n", __FUNCTION__, pDbgCmd->cmd, pDbgCmd->param1, pDbgCmd->param2);
			
			switch (pDbgCmd->cmd) {
#ifdef SECONDARY_AFEID_FN
				case DIAG_DEBUG_CMD_OVERRIDE_2ND_AFEIDS:
					AdslDrvPrintf(TEXT("Current overrideSecondaryAfeId = 0x%08x/0x%08x, pDbgCmd->param1/2 = 0x%08x/0x%08x\n"),
						overrideSecondaryAfeId[0], overrideSecondaryAfeId[1], pDbgCmd->param1, pDbgCmd->param2);
					overrideSecondaryAfeId[0] = pDbgCmd->param1;
					overrideSecondaryAfeId[1] = pDbgCmd->param2;
					if(adslCoreInitialized)
						BcmXdslCoreSendAfeInfo(1);
					break;
#endif
#if defined(USE_PMC_API)
				case DIAG_DEBUG_CMD_READ_AFEPLLMDIV:
				{
					int ret;
					PLL_CHCFG_REG pllch01_cfg, pllch45_cfg;
					ret = BcmXdslReadAfePLLMdiv(lineId, &pllch01_cfg.Reg32, &pllch45_cfg.Reg32);
					
					if(kPMC_NO_ERROR == ret)
						BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch01_cfg = 0x%08x mdiv0=%d overide0=%d, ch45_cfg = 0x%08x mdiv1=%d overide1=%d\n",
							(uint)pllch01_cfg.Reg32, pllch01_cfg.Bits.mdiv0, pllch01_cfg.Bits.mdiv_override0,
							(uint)pllch45_cfg.Reg32, pllch45_cfg.Bits.mdiv1, pllch45_cfg.Bits.mdiv_override1);
				}
					break;
					
				case DIAG_DEBUG_CMD_WRITE_AFEPLLMDIV:
					BcmXdslWriteAfePLLMdiv(lineId, pDbgCmd->param1, pDbgCmd->param2);
					break;
					
				case DIAG_DEBUG_CMD_READ_AFEPLLNDIV:
				{
					int ret;
					PLL_NDIV_REG pll_ndiv;
					ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(ndiv), &pll_ndiv.Reg32);
					if (ret) {
						BcmAdslCoreDiagWriteStatusString(lineId, "Read NDIV error(%d)\n", ret);
						return;
					}
					BcmAdslCoreDiagWriteStatusString(lineId, "Read pll_ndiv = 0x%08x --> ndiv_int = 0x%x\n", pll_ndiv.Reg32, pll_ndiv.Bits.ndiv_int);
					break;
				}
				case DIAG_DEBUG_CMD_WRITE_AFEPLLNDIV:
				{
					int ret;
					PLL_NDIV_REG pll_ndiv;
					PLL_CTRL_REG pll_ctrl;
					PLL_LOOP1_REG pll_loop1;
					
					/* Stop PHY MIPS */
					BcmAdslCoreStop();
					
					/* Put the PLL in reset */
					ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
					if (ret)
						return;
					pll_ctrl.Bits.resetb = 0;
					pll_ctrl.Bits.post_resetb = 0;
					ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
					if (ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "Put the PLL in reset\n");
					
					/* Change NDIV Field */
					ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(ndiv), &pll_ndiv.Reg32);
					if (ret)
						return;
					pll_ndiv.Bits.ndiv_int = pDbgCmd->param1 & 0x3FF;	/* ndiv_int[9:0] */
					pll_ndiv.Bits.reserved0 = 1 << 1;					/* reserved0[31:30], Set NDIV Override bit 31*/
					ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(ndiv), pll_ndiv.Reg32);
					if (ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "Change NDIV Field to %d\n", pll_ndiv.Bits.ndiv_int);
					
					/* Release resetb */
					ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
					if (ret)
						return;
					pll_ctrl.Bits.resetb = 1;
					ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
					if (ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "Release resetb\n");
					
					do {
						ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
								PLLBPCMRegOffset(loop1), &pll_loop1.Reg32);
						if (ret)
							return;
					} while (!(pll_loop1.Bits.ssc_mode));
					BcmAdslCoreDiagWriteStatusString(lineId, "The PLL is locked\n");
					
					/* Release post_resetb */
					pll_ctrl.Bits.post_resetb = 1;
					ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
							PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
					if (ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "Release post_resetb\n");
					
					/* Start PHY MIPS */
					BcmAdslCoreStart(DIAG_DATA_EYE, AC_TRUE);
					break;
				}
				case DIAG_DEBUG_CMD_SET_AFEPLLHOLDENABLEBITS:
				{
					int ret;
					PLL_CHCFG_REG pllch45_cfg;
					
					if(kPMC_NO_ERROR != (ret =ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset (ch45_cfg), &pllch45_cfg.Reg32))) {
						BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch45_cfg error(%d)\n", ret);
						return;
					}
	
					//Set Hold of ch[5], setting this bit to 1 will hold the clock low.	 Change to 0 to enable the clock again.  Change should be glitchless. 
					pllch45_cfg.Bits.hold_ch1 = (0 != pDbgCmd->param1)? 1: 0;
					
					if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch45_cfg), pllch45_cfg.Reg32))) {
						BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch45_cfg error(%d)\n", ret);
						return;
					}
					
					//Similarly for enable_ch[5], setting this bit to 1 will disable the PLL Channel. 
					//Setting this bit to 0 will enable the Channel. 
					
					if(kPMC_NO_ERROR != (ret =ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset (ch45_cfg), &pllch45_cfg.Reg32))) {
						BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch45_cfg error(%d)\n", ret);
						return;
					}
					
					//Set Enable_b of ch[5], setting this bit to 1 will disable the PLL Post divider.	Change to 0 to enable the again. 
					pllch45_cfg.Bits.enableb_ch1 = (0 != pDbgCmd->param2)? 1: 0;
					
					if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch45_cfg), pllch45_cfg.Reg32))) {
						BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch45_cfg error(%d)\n", ret);
						return;
					}
					break;
				}
				case DIAG_DEBUG_CMD_READ_AFEPLLMDEL:
				{
					int ret;
					PLL_CHCFG_REG pllch01_cfg, pllch45_cfg;
					
					ret = BcmXdslReadAfePLLMdiv(lineId, &pllch01_cfg.Reg32, &pllch45_cfg.Reg32);
					if(kPMC_NO_ERROR == ret)
						BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Read ch01_cfg = 0x%08x mdel0=%d mdel1=%d, ch45_cfg = 0x%08x mddel0=%d mdel1=%d\n",
							(uint)pllch01_cfg.Reg32, pllch01_cfg.Bits.mdel0, pllch01_cfg.Bits.mdel1,
							(uint)pllch45_cfg.Reg32, pllch45_cfg.Bits.mdel0, pllch45_cfg.Bits.mdel1);
				}
					break;
				case DIAG_DEBUG_CMD_TOGGLE_AFEPLLMDEL:
				{
					int ret;
					PLL_CHCFG_REG pllch01_cfg;
					PLL_CHCFG_REG pllch45_cfg;
					
					ret = BcmXdslReadAfePLLMdiv(lineId, &pllch01_cfg.Reg32, &pllch45_cfg.Reg32);
					if(kPMC_NO_ERROR != ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "Before invert mdiv0/1, ch01_cfg=0x%08x mdel0=%d mdel1=%d, ch45_cfg=0x%08x mddel0=%d mdel1=%d\n",
						(uint)pllch01_cfg.Reg32, pllch01_cfg.Bits.mdel0, pllch01_cfg.Bits.mdel1,
						(uint)pllch45_cfg.Reg32, pllch45_cfg.Bits.mdel0, pllch45_cfg.Bits.mdel1);
					
					/* Invert mdiv0/1 */
					if(pDbgCmd->param1&0x3) {
						if(pDbgCmd->param1&0x1)
							pllch01_cfg.Bits.mdel0 ^= 1;
						if(pDbgCmd->param1&0x2)
							pllch01_cfg.Bits.mdel1 ^= 1;
						if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch01_cfg), pllch01_cfg.Reg32))) {
							BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch01_cfg error(%d)\n", ret);
							return;
						}
					}
					if(pDbgCmd->param2&0x3) {
						if(pDbgCmd->param2&0x1)
							pllch45_cfg.Bits.mdel0 ^= 1;
						if(pDbgCmd->param2&0x2)
							pllch45_cfg.Bits.mdel1 ^= 1;
						if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch45_cfg), pllch45_cfg.Reg32))) {
							BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch45_cfg error(%d)\n", ret);
							return;
						}
					}
					
					ret = BcmXdslReadAfePLLMdiv(lineId, &pllch01_cfg.Reg32, &pllch45_cfg.Reg32);
					if(kPMC_NO_ERROR != ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "After invert mdiv0/1, ch01_cfg=0x%08x mdel0=%d mdel1=%d, ch45_cfg=0x%08x mddel0=%d mdel1=%d\n",
						(uint)pllch01_cfg.Reg32, pllch01_cfg.Bits.mdel0, pllch01_cfg.Bits.mdel1,
						(uint)pllch45_cfg.Reg32, pllch45_cfg.Bits.mdel0, pllch45_cfg.Bits.mdel1);
					
					/* Restore mdiv0/1 */
					if(pDbgCmd->param1&0x3) {
						if(pDbgCmd->param1&0x1)
							pllch01_cfg.Bits.mdel0 ^= 1;
						if(pDbgCmd->param1&0x2)
							pllch01_cfg.Bits.mdel1 ^= 1;
						if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch01_cfg), pllch01_cfg.Reg32))) {
							BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch01_cfg error(%d)\n", ret);
							return;
						}
					}
					if(pDbgCmd->param2&0x3) {
						if(pDbgCmd->param2&0x1)
							pllch45_cfg.Bits.mdel0 ^= 1;
						if(pDbgCmd->param2&0x2)
							pllch45_cfg.Bits.mdel1 ^= 1;
						if(kPMC_NO_ERROR != (ret =WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ch45_cfg), pllch45_cfg.Reg32))) {
							BcmAdslCoreDiagWriteStatusString(lineId, "AFE PLL Write ch45_cfg error(%d)\n", ret);
							return;
						}
					}
					
					ret = BcmXdslReadAfePLLMdiv(lineId, &pllch01_cfg.Reg32, &pllch45_cfg.Reg32);
					if(kPMC_NO_ERROR != ret)
						return;
					BcmAdslCoreDiagWriteStatusString(lineId, "After restore mdiv0/1, ch01_cfg=0x%08x mdel0=%d mdel1=%d, ch45_cfg=0x%08x mddel0=%d mdel1=%d\n",
						(uint)pllch01_cfg.Reg32, pllch01_cfg.Bits.mdel0, pllch01_cfg.Bits.mdel1,
						(uint)pllch45_cfg.Reg32, pllch45_cfg.Bits.mdel0, pllch45_cfg.Bits.mdel1);
				}
					break;
#endif	/* USE_PMC_API */
				case DIAG_DEBUG_CMD_STAT_SAVE_LOCAL:
				{
					static void 		*pMem = NULL;
					BCMADSL_STATUS	res;

					if( 1 == pDbgCmd->param1 ) {	/* Init logging statuses */
						res = BcmAdsl_DiagStatSaveStop();
						if(pMem)
							vfree(pMem);

						pMem = vmalloc(2 * pDbgCmd->param2);
						if( NULL == pMem ) {
							printk("%s - %s vmalloc(%d)\n",
									"BcmAdsl_DiagStatSaveInit", failStr, pDbgCmd->param2);
							break;
						}
						res = BcmAdsl_DiagStatSaveInit( pMem, 2 * pDbgCmd->param2);
						printk("%s - %s pMem=0x%px len=%d\n",
								"BcmAdsl_DiagStatSaveInit",
								( BCMADSL_STATUS_SUCCESS == res ) ? successStr: failStr,
										pMem, pDbgCmd->param2);
					}
					else if( 2 == pDbgCmd->param1 ) { /* Enable logging statuses */
						res = BcmAdsl_DiagStatSaveStart();
						printk("%s - %s\n",
								"BcmAdsl_DiagStatSaveStart", ( BCMADSL_STATUS_SUCCESS == res ) ? successStr: failStr);
					}
					else if( 3 == pDbgCmd->param1 ) {	/* Disable logging statuses */
						res = BcmAdsl_DiagStatSaveStop();
						printk("%s - %s\n",
								"BcmAdsl_DiagStatSaveStop", ( BCMADSL_STATUS_SUCCESS == res ) ? successStr: failStr);
					}
					else if( 4 == pDbgCmd->param1 ) {	/* Write statuses to DslDiag */
						ADSL_SAVEDSTATUS_INFO savedStatInfo;
						int len, dataCnt, dataSize = 7*LOG_MAX_DATA_SIZE;

						BcmAdsl_DiagStatSaveStop();
						res = BcmAdsl_DiagStatSaveGet(&savedStatInfo);
						sprintf(tmpStr, "BcmAdsl_DiagStatSaveGet - %s\n", ( BCMADSL_STATUS_SUCCESS == res ) ? successStr: failStr);
						DiagWriteString(lineId, DIAG_DSL_CLIENT, tmpStr);
						printk("%s", tmpStr);
						
						if( BCMADSL_STATUS_SUCCESS == res ) {
							sprintf(tmpStr, "nStatus=%d pAddr=0x%px len=%d pAddr1=0x%px len1=%d\n",
								savedStatInfo.nStatus, savedStatInfo.pAddr, savedStatInfo.len,
								savedStatInfo.pAddr1, savedStatInfo.len1);
							DiagWriteString(lineId, DIAG_DSL_CLIENT, tmpStr);
							printk("%s", tmpStr);
							
							BcmAdslCoreDiagOpenFile(lineId, savedStatFileName);
							dataCnt = 0;
							while (dataCnt < savedStatInfo.len) {
								len = savedStatInfo.len - dataCnt;
								if (len > dataSize)
									len = dataSize;
								BcmAdslCoreDiagWriteFile(lineId, savedStatFileName, ((char*)savedStatInfo.pAddr)+dataCnt, len);
								dataCnt += len;
								BcmAdslCoreDelay(40);
							}
							if( savedStatInfo.len1 > 0 ) {
								dataCnt = 0;
								while (dataCnt < savedStatInfo.len1) {
									len = savedStatInfo.len1 - dataCnt;
									if (len > dataSize)
										len = dataSize;
									BcmAdslCoreDiagWriteFile(lineId, savedStatFileName, ((char *)savedStatInfo.pAddr1)+dataCnt, len);
									dataCnt += len;
									BcmAdslCoreDelay(40);
								}
							}
						}
					}
					else if( 5 == pDbgCmd->param1 ) {	/* Enable logging statuses continously */
						res = BcmAdsl_DiagStatSaveContinous();
						printk("%s - %s\n",
								"BcmAdsl_DiagStatSaveContinous", ( BCMADSL_STATUS_SUCCESS == res ) ? successStr: failStr);
					}
					else if( (6 == pDbgCmd->param1) || (9 == pDbgCmd->param1) ) {	/* start logging statuses continously/until retrain */
						BcmAdsl_DiagStatSaveStop();
						if(pMem)
							vfree(pMem);
						pMem = vmalloc(pDbgCmd->param2*2);
						if( NULL == pMem ) {
							printk("%s - %s vmalloc(%d)\n",
									"BcmAdsl_DiagStatSaveInit", failStr,pDbgCmd->param2);
							break;
						}
						res = BcmAdsl_DiagStatSaveInit( pMem, 2 * pDbgCmd->param2);
						if( BCMADSL_STATUS_SUCCESS != res ) {
							printk("%s - %s pMem=0x%px len=%u\n",
									"BcmAdsl_DiagStatSaveInit", failStr, pMem, pDbgCmd->param2);
							break;
						}
						BcmAdsl_DiagStatSaveContinous();
						if(9 == pDbgCmd->param1)
							BcmXdslCoreDiagStatSaveDisableOnRetrainSet();
						BcmAdsl_DiagStatSaveStart();
						if(9 == pDbgCmd->param1)
							printk("** Logging until retrain is started(0x%x) ** \n", pDbgCmd->param2);
						else
							printk("** Logging continously started(0x%x) ** \n", pDbgCmd->param2);
					}
					else if( 7 == pDbgCmd->param1 ) {	/* Stop logging statuses */
						res = BcmAdsl_DiagStatSaveUnInit();
						DiagWriteString(lineId, DIAG_DSL_CLIENT, "%s - %s\n",
								"BcmAdsl_DiagStatSaveUnInit", ( BCMADSL_STATUS_SUCCESS == res ) ? successStr: failStr);
						if(pMem)
							vfree(pMem);
						pMem = NULL;
					}
					else if( 8 == pDbgCmd->param1 ) {	/* start logging until buffer is full */
						BcmAdsl_DiagStatSaveStop();
						if(pMem)
							vfree(pMem);
						pMem = vmalloc(pDbgCmd->param2*2);
						if( NULL == pMem ) {
							printk("%s - %s vmalloc(%d)\n",
									"BcmAdsl_DiagStatSaveInit", failStr,pDbgCmd->param2);
							break;
						}
						res = BcmAdsl_DiagStatSaveInit( pMem, 2 * pDbgCmd->param2);
						if( BCMADSL_STATUS_SUCCESS != res ) {
							DiagWriteString(lineId, DIAG_DSL_CLIENT, "%s - %s pMem=0x%px len=%d\n",
									"BcmAdsl_DiagStatSaveInit", failStr, pMem, pDbgCmd->param2);
							break;
						}
						BcmAdsl_DiagStatSaveStart();
						printk("** Logging until buffer is full started ** \n");
					}
					else if( 10 == pDbgCmd->param1 ) {	/* Pause logging */
						ADSL_SAVEDSTATUS_INFO savedStatInfo;

						BcmAdsl_DiagStatSaveStop();
						printk("** Logging paused ** \n");
						res = BcmAdsl_DiagStatSaveGet(&savedStatInfo);
						if( BCMADSL_STATUS_SUCCESS == res ) {
							BcmAdslCoreDiagWriteStatusString(lineId, "	nStatus=%d pAddr=0x%px len=%d pAddr1=0x%px len1=%d\n",
									savedStatInfo.nStatus, savedStatInfo.pAddr, savedStatInfo.len,
									savedStatInfo.pAddr1, savedStatInfo.len1);
							printk("	nStatus=%d pAddr=0x%px len=%d pAddr1=0x%px len1=%d\n",
									savedStatInfo.nStatus, savedStatInfo.pAddr, savedStatInfo.len,
									savedStatInfo.pAddr1, savedStatInfo.len1);
						}
					}
					break;
				}
				case DIAG_DEBUG_CMD_DUMPBUF_CFG:
					{
					dslCommandStruct		cmd;
					void					*pBufTmp;

					cmd.command = kDslSendEocCommand;
					cmd.param.dslClearEocMsg.msgId = kDslDumpBufferCfg;
					cmd.param.dslClearEocMsg.dataPtr = (void *) &dumpBuf;
					cmd.param.dslClearEocMsg.msgType = sizeof(dumpBuf) | kDslClearEocMsgDataVolatile;
					if (0 == pDbgCmd->param1) {
					  if (dumpBuf.pBuf != NULL) {
					    pBufTmp = dumpBuf.pBuf;
					    dumpBuf.pBuf = NULL;
					    BcmCoreCommandHandler(&cmd);
					    if (!AdslCorePhyReservedMemFree(pBufTmp))
					      BcmAdslFreePhyMem(pBufTmp, dbPhysAddr, dumpBuf.bufSize);
					  }
					}
					else if (kDumpSaveCmd == pDbgCmd->param1) {
					  dumpBufferStruct dumpBufTmp;
					  cmd.param.dslClearEocMsg.dataPtr = (void *) &dumpBufTmp;
					  dumpBufTmp.pBuf    = (void *)(uintptr_t) ADSL_ENDIAN_CONV_INT32(kDumpSaveCmd);
					  dumpBufTmp.bufSize = ADSL_ENDIAN_CONV_INT32(pDbgCmd->param2);
					  BcmCoreCommandHandler(&cmd);
					}
					else {
					  void *pPhys;

					  if (NULL != dumpBuf.pBuf) {
					    if (!AdslCorePhyReservedMemFree(dumpBuf.pBuf))
						  BcmAdslFreePhyMem(dumpBuf.pBuf, dbPhysAddr, dumpBuf.bufSize);
					  }
					  dumpBuf.bufSize = pDbgCmd->param1 << 10;
					  dumpBuf.pBuf = BcmAdslAllocPhyMem(dumpBuf.bufSize, &dbPhysAddr);
					  if (NULL != dumpBuf.pBuf) {
					   pPhys = dbPhysAddr;
					   if (ADSL_PHY_SUPPORT(kAdslPhyVmAddrSupport)) {
						uintptr_t  pa = (uintptr_t) pPhys;
					    if ( ((pa < 0x19800000) && ((pa + dumpBuf.bufSize) > 0x19000000)) ||
							 ((pa < 0x20000000) && ((pa + dumpBuf.bufSize) > 0x1FE00000)) ) {
					      /* in PHY LMEM/RMEM or register space */
					      printk("DIAG_DEBUG_CMD_DUMPBUF_CFG: Kernel buffer allocated in PHY LMEM or reg address space ptr=0x%px phyAddr=0x%px\n", dumpBuf.pBuf, pPhys);
					      BcmAdslFreePhyMem(dumpBuf.pBuf, dbPhysAddr, dumpBuf.bufSize);
					      dumpBuf.pBuf = NULL;
					    }
					   }
					   else {
					    if ((((uintptr_t) pPhys) + dumpBuf.bufSize) > 0x19000000) {
					      /* too high for PHY to reach; free and try to allocate in reserved memory */
					      printk("DIAG_DEBUG_CMD_DUMPBUF_CFG: Kernel buffer allocated too high ptr=0x%px phyAddr=0x%px\n", dumpBuf.pBuf, pPhys);
					      BcmAdslFreePhyMem(dumpBuf.pBuf, dbPhysAddr, dumpBuf.bufSize);
					      dumpBuf.pBuf = NULL;
					    }
					   }
					  }

					  if (NULL == dumpBuf.pBuf) {
					      dumpBuf.pBuf = AdslCorePhyReservedMemAlloc(dumpBuf.bufSize);
					      if (NULL == dumpBuf.pBuf)
					        break;
					      pPhys = (void *) (uintptr_t) SDRAM_ADDR_TO_PHYS(dumpBuf.pBuf);
					  }
					  printk("DIAG_DEBUG_CMD_DUMPBUF_CFG: Buffer allocated ptr=0x%px size=%d phyAddr=0x%px\n", dumpBuf.pBuf, (int)dumpBuf.bufSize, pPhys);

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)	// TO DO: Tony - Double check
					  dumpBuf.pHostAddr = (void *) ADSL_ENDIAN_CONV_INT64(((uintptr_t)dumpBuf.pBuf));
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963178)
					  dumpBuf.pHostAddr = (void *) ADSL_ENDIAN_CONV_INT32(((uint)dumpBuf.pBuf));
#endif
					  dumpBuf.bufSize |= (pDbgCmd->param2 & 7) << 29;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)
					  pBufTmp = dumpBuf.pBuf;
					  if (ADSL_PHY_SUPPORT(kAdslPhyVmAddrSupport))
					    pPhys = (void *) (((uintptr_t)pPhys) | kDumpCtrlAbsMemAddr);
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)	// TO DO: Tony - Double check
					  dumpBuf.pBuf = (void *) ADSL_ENDIAN_CONV_INT64((uintptr_t)pPhys);
#else
					  dumpBuf.pBuf = (void *) ADSL_ENDIAN_CONV_INT32((uint)pPhys);
#endif
					  dumpBuf.bufSize = ADSL_ENDIAN_CONV_INT32(dumpBuf.bufSize);
#endif
					  BcmCoreCommandHandler(&cmd);
					  while (AdslCoreCommandIsPending())
					    bcmOsSleep(1);
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)
					  dumpBuf.pBuf = pBufTmp;
#endif
					  dumpBuf.bufSize = pDbgCmd->param1 << 10;
					}
					}
					break;
				case DIAG_DEBUG_CMD_CLEAR_STAT:
					BcmAdslCoreDiagClearStats();
					break;
				case DIAG_DEBUG_CMD_PRINT_STAT:
					BcmAdslCoreDiagPrintStats(lineId);
					break;
				case DIAG_DEBUG_CMD_SET_REGRESSION_LOCK:
					diagRegressionLock = 1;
					break;
				case DIAG_DEBUG_CMD_CLR_REGRESSION_LOCK:
					diagRegressionLock = 0;
					diagLockSession = 0;
					break;
				case DIAG_DEBUG_CMD_READ_MEM:
					BcmAdslCoreDebugReadMem(lineId, pDbgCmd);
					break;

				case DIAG_DEBUG_CMD_SET_MEM:
				{
					uint	*pAddr = (uint *)BcmAdslCoreDiagAddrConv(pDbgCmd->param1);
					*pAddr = pDbgCmd->param2;
					break;
				}
				case DIAG_DEBUG_CMD_WRITE_FILE:
				{
					char fileName[32], *pData;
					int len, dataCnt, dataSize = 7*LOG_MAX_DATA_SIZE;
					
					sprintf(fileName, "0x%08x", (uint)pDbgCmd->param1);
					
					pData = (char *)BcmAdslCoreDiagAddrConv(pDbgCmd->param1);
					pDbgCmd->param1 = (uint)(uintptr_t)pData;
					
					BcmAdslCoreDiagOpenFile(lineId, fileName);
					dataCnt = 0;
					while (dataCnt < pDbgCmd->param2) {
						len = pDbgCmd->param2 - dataCnt;
						if (len > dataSize)
							len = dataSize;
						BcmAdslCoreDiagWriteFile(lineId, fileName, pData+dataCnt, len);
						dataCnt += len;
						BcmAdslCoreDelay(40);
					}
					break;
				}
				case DIAG_DEBUG_CMD_RESET_CHIP:
					BcmAdslCoreSetWdTimer (1000);
					break;

				case DIAG_DEBUG_CMD_EXEC_FUNC:
				{
					int res, (*pExecFunc) (uint param) = (void *)(uintptr_t)(pDbgCmd->param1);

					res = (*pExecFunc) (pDbgCmd->param2);
					BcmAdslCoreDiagWriteStatusString(lineId, "CMD_EXEC_FUNC at 0x%px param=%d: result=%d", pExecFunc, pDbgCmd->param2, res);
				}
					break;
#ifdef ADSLDRV_SILENT_MODE
				case DIAG_DEBUG_CMD_SILENT_MODE:
				{
					extern int gXtmNotify;
					gXtmNotify = (int) pDbgCmd->param1;
					BcmAdslCoreDiagWriteStatusString(lineId, "xTM Notify=%d", gXtmNotify);
					break;
				}
#endif
#ifdef SUPPORT_PHY_BIN_FROM_TMP
				case DIAG_DEBUG_CMD_PHY_FROM_TMP:
				{
				extern char loadImageFromTmp;
				loadImageFromTmp = pDbgCmd->param1 != 0;
				printk("DIAG_DEBUG_phyTmp=%d \n", loadImageFromTmp);
				}
				break;
#endif
			}
		}
		/*  Fall Through for client specific log debug commands */
		default:
			switch(clientType) {
				case DIAG_DSL_CLIENT:
					if (diagClientsTbl[DIAG_DSL_CLIENT].isRegistered)
						diagClientsTbl[DIAG_DSL_CLIENT].pCallback(lineId, pAdslDiag->diagCmd, pAdslDiag->logTime, (void *)pAdslDiag->diagMap);
					break;
				case DIAG_WLAN_CLIENT:
					if (diagClientsTbl[DIAG_WLAN_CLIENT].isRegistered)
						diagClientsTbl[DIAG_WLAN_CLIENT].pCallback(lineId, pAdslDiag->diagCmd, pAdslDiag->logTime, (void *)pAdslDiag->diagMap);
					break;
				case DIAG_XTM_CLIENT:
					if (diagClientsTbl[DIAG_XTM_CLIENT].isRegistered)
						diagClientsTbl[DIAG_XTM_CLIENT].pCallback(lineId, pAdslDiag->diagCmd, pAdslDiag->logTime, (void *)pAdslDiag->diagMap);
					break;
			}
	}
}

int BcmAdslGdbIsConnected(void)
{
	return diagCtrl.skbGdb && ( (diagCtrl.skbGdbReroute) || (NULL != diagCtrl.gdbDev) );
}

int BcmAdslDiagIsConnected(void)
{
	return diagCtrl.skbModel && ( (diagCtrl.skbModelReroute) || (NULL != diagCtrl.dbgDev)  );
}

int BcmAdslDiagIsActive(void)
{
#ifndef SUPPORT_EXT_DSL_BONDING_SLAVE
	return diagCtrl.skbModel && ( (diagCtrl.skbModelReroute) || (NULL != diagCtrl.dbgDev) );
#else
	return AC_TRUE;
#endif
}

/***************************************************************************
** Function Name: BcmAdslDiagReset
** Description  : This function resets diag support after ADSL MIPS reset
** Returns      : None.
***************************************************************************/
void BcmAdslDiagReset(int map)
{

	diagCtrl.diagDataMap[0] &= map & (DIAG_DATA_LOG | DIAG_DATA_EYE);
#ifdef SUPPORT_DSL_BONDING
	diagCtrl.diagDataMap[1] &= map & (DIAG_DATA_LOG | DIAG_DATA_EYE);
#endif

	if (diagCtrl.diagDataMap[0] & (DIAG_DATA_LOG | DIAG_DATA_EYE)) {
		BcmAdslCoreDiagDataInit(0);
	}
	

	if (diagCtrl.diagDataMap[0] & (DIAG_DATA_LOG | DIAG_DATA_EYE))
		BcmAdslCoreDiagCommand(0);
#ifdef SUPPORT_DSL_BONDING
	if((ADSL_PHY_SUPPORT(kAdslPhyBonding)) && (diagCtrl.diagDataMap[1] & (DIAG_DATA_LOG | DIAG_DATA_EYE))) {
		BcmAdslCoreDiagDataInit(1);
		BcmAdslCoreDiagCommand(1);
	}
#endif
}

/***************************************************************************
** Function Name: BcmAdslDiagInit
** Description  : This function intializes diag support on Host and ADSL MIPS
** Returns      : None.
***************************************************************************/
int BcmAdslDiagInit(int map)
{
	BcmAdslCoreDiagDataInit(0);
	BcmAdslCoreDiagCommand(0);
	return 0;
}

uint BcmXdslDiagGetSrvIpAddr(void)
{
	uint res = 0;
	if(NULL != diagCtrl.skbModel) {
        if ( diagCtrl.skbModelReroute ) {
            res = SKB_REROUTE_CLIENT_ADDR_FIELD(diagCtrl.skbModel);
        }
        else {
            diagSockFrame *dd = (diagSockFrame *) DiagPacketStart(diagCtrl.skbModel);
            res = dd->ipHdr.dstAddr;
        }
        res = htonl(res);
	}
	return res;
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(BcmDiagsMgrRegisterClient);
EXPORT_SYMBOL(DiagsDeregisterClient);


EXPORT_SYMBOL(DiagWriteStatusShort);
EXPORT_SYMBOL(DiagWriteStatusLong);
EXPORT_SYMBOL(DiagWriteFile);
EXPORT_SYMBOL(DiagOpenFile);
EXPORT_SYMBOL(DiagDumpData);
EXPORT_SYMBOL(DiagWriteString);
EXPORT_SYMBOL(DiagWriteStringV);
#endif
