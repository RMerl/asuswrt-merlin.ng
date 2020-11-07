/* 
* <:copyright-BRCM:2012:proprietary:standard
* 
*    Copyright (c) 2012 Broadcom 
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

#include <bcmtypes.h>

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/version.h>
#include "softdsl/AdslCoreDefs.h"
#include "AdslCore.h"
#include "AdslMibDef.h"
#include "DiagDef.h"

#define EXCLUDE_CYGWIN32_TYPES
#include "softdsl/SoftDsl.h"

#include "BcmAdslDiagCommon.h"
#include "board.h"
#include "bcm_map.h"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <bcmxtmcfg.h>
#endif
#ifdef ADSL_MIB
#include "softdsl/AdslMib.h"
#endif

#define SYMB_VECT_SEGMENT		(1019)
#define ETH_HEADER_ITU_LENGTH	(22)
#define ETH_MAC_ADDR_LEN		(6)

#define LINEID_OFFSET			(0)
#define SYNCCOUNTER_OFFSET	(2)
#define PARTID_OFFSET			(4)
#define ERRORSAMPLES_OFFSET	(5)
#if 0
static unsigned char ethHeaderBuffer[ETH_HEADER_ITU_LENGTH]={
	0,0,0,0,0,0,	/* VCE MAC Address */
	0,0,0,0,0,0,	/* VTU-R MAC Address */
	0x04, 0x08,	/* Length <= 1032 */
	0xAA, 0xAA, 0x03,	/* LLC PDU header for SNAP */
	0x00, 0x19, 0xA7,	/* SNAP PDU : ITU OUI */
	0x00, 0x03	/* SNAP PDU : PRIVATE protocol ID */
	};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
/* Note: Linux4.04L.02 and older don't have "netdev_ops->ndo_start_xmit" */
#define	DEV_VECT_TRANSMIT(x)	(x)->dev->netdev_ops->ndo_start_xmit (x, (x)->dev)
#else
#define	DEV_VECT_TRANSMIT(x)	dev_queue_xmit(x)
#endif

#define VECT_FRAME_SIZE	(ETH_HEADER_ITU_LENGTH+ERRORSAMPLES_OFFSET+SYMB_VECT_SEGMENT)

#define VECT_SKBS_IN_POOL	96 /* It is possible SKBs get delayed freeing in different systems, so keep a larger pool of SKBs */

/* Headroom is required to add L1.5 header, in this case the possibility of Bonding header, which could range from
* 16-48 bytes plus another 48 bytes in case the header contents needs to be cacheline aligned.
* For non-bonded cases, no headroom is required but still provisioned for now to keep the implementation common.
*/
#define	SKB_HEADROOM_RESERVE	96

extern void *XdslCoreGetDslVars(unsigned char lineId);

static struct net_device *wanDev = NULL;
static struct sk_buff *vectSkbModel = NULL;
static dslDrvSkbPool *vectSkbDev = NULL;

#if defined(XTM_USE_DSL_WAN_NOTIFY)
static int wanDevIsUp = 0;
void BcmXdslWanDevIsUp(void)
{
	wanDevIsUp = 1;
	printk("%s: wanDevIsUp=1\n", __FUNCTION__);
}
#endif

unsigned int BcmXdslGetVectExtraSkbCnt(void)
{
	return (NULL != vectSkbDev) ? vectSkbDev->extraSkb: 0;
}

void BcmXdslDumpVectSkbDevInfo(unsigned char lineId)
{
	if(NULL != vectSkbDev) {
		DiagStrPrintf(lineId, DIAG_DSL_CLIENT, "VectSkbDevInfo: numOfSkbs=%d skbBufIndex = %d extraSkb=%d\n",
			vectSkbDev->numOfSkbs, vectSkbDev->skbBufIndex, vectSkbDev->extraSkb);
	}
}

void BcmXdslDiscardWanDev(unsigned char lineId, unsigned char bDevDown)
{
	if( NULL != wanDev ) {
#ifdef SUPPORT_DSL_BONDING
		if (!bDevDown && AdslCoreLinkState(lineId^1))
			return;
#endif
		wanDev = NULL;
#if defined(XTM_USE_DSL_WAN_NOTIFY)
		wanDevIsUp = 0;
#endif
		if( NULL != vectSkbDev ) {
			dslDrvSkbPool *skbDev = vectSkbDev;
			vectSkbDev = NULL;
			if(!DevSkbFree(skbDev, 0)) {
				vectSkbDev = skbDev;
				printk("%s: vectSkbDev is not freed!!\n", __FUNCTION__);
			}
		}
	}
	printk("%s: lineId=%d,bDevDown=%d\n", __FUNCTION__, lineId, bDevDown);
}

void * BcmXdslCreatePktHdr(unsigned char lineId, struct sk_buff **ppSkb)
{
	int i, res;
	long len;
	adslMibInfo *pXdslMib;
	struct net_device *dev = NULL;
	char *wanDevNames[] = {"ptm0"};
	char vectState, oidStr[] = { kOidAdslPrivate, kOIdAdslPrivGetVectState };
	unsigned char ethHeaderBuffer[ETH_HEADER_ITU_LENGTH]={
		0,0,0,0,0,0,	/* VCE MAC Address */
		0,0,0,0,0,0,	/* VTU-R MAC Address */
		0x04, 0x08,	/* Length <= 1032 */
		0xAA, 0xAA, 0x03,	/* LLC PDU header for SNAP */
		0x00, 0x19, 0xA7,	/* SNAP PDU : ITU OUI */
		0x00, 0x03	/* SNAP PDU : PRIVATE protocol ID */
		};
	struct sk_buff *pSkb = *ppSkb;
	
	len = sizeof(adslMibInfo);
	pXdslMib = (void *) AdslMibGetObjectValue(XdslCoreGetDslVars(lineId), NULL, 0, NULL, &len);
	
	len = sizeof(vectState);
	if(kAdslMibStatusSuccess != (res = AdslMibGetObjectValue(XdslCoreGetDslVars(lineId), oidStr, sizeof(oidStr), &vectState, &len))) {
		printk("%s: Errored line%d get vectSM.state failed(%d)\n", __FUNCTION__, lineId, res);
		return NULL;
	}
	
	if(((vectState==VECT_FULL) || (vectState==VECT_RUNNING)) &&
		(0 == pXdslMib->vectData.macAddress.addressType))
		memcpy((void *)&ethHeaderBuffer[0], (void *)&pXdslMib->vectData.macAddress.macAddress[0], 6);
	else {
		printk("%s: Errored line%d vectSM.state=%d addressType = %d macAddr: %x-%x-%x-%x-%x-%x\n",
			__FUNCTION__, lineId, vectState, pXdslMib->vectData.macAddress.addressType,
			pXdslMib->vectData.macAddress.macAddress[0], pXdslMib->vectData.macAddress.macAddress[1],
			pXdslMib->vectData.macAddress.macAddress[2], pXdslMib->vectData.macAddress.macAddress[3],
			pXdslMib->vectData.macAddress.macAddress[4], pXdslMib->vectData.macAddress.macAddress[5]);
		return NULL;
	}
	
	for (i = 0; i < sizeof(wanDevNames)/sizeof(wanDevNames[0]); i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
		dev = __dev_get_by_name(&init_net, wanDevNames[i]);
#else
		dev = __dev_get_by_name(wanDevNames[i]);
#endif
		if (NULL == dev)
			continue;
		
		memcpy((void *)&ethHeaderBuffer[6], dev->dev_addr, 6);
		
		printk ("\tdev = %s(0x%px) macAddr=%X:%X:%X:%X:%X:%X\n",
			wanDevNames[i], dev,
			dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
			dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
		
		printk ("\tLine%d: VCE macAddr=%X:%X:%X:%X:%X:%X, VTU-R macAddr=%X:%X:%X:%X:%X:%X\n",
			lineId,
			ethHeaderBuffer[0], ethHeaderBuffer[1], ethHeaderBuffer[2],
			ethHeaderBuffer[3], ethHeaderBuffer[4], ethHeaderBuffer[5],
			ethHeaderBuffer[6], ethHeaderBuffer[7], ethHeaderBuffer[8],
			ethHeaderBuffer[9], ethHeaderBuffer[10], ethHeaderBuffer[11]);
		
		if (NULL == pSkb) {
			if(NULL == (pSkb = alloc_skb (ETH_HEADER_ITU_LENGTH, GFP_ATOMIC)))
				return NULL;
		}
		
		pSkb->dev = dev;
		pSkb->protocol = htons(eth_type_trans (pSkb, dev));
		pSkb->data = pSkb->head;
		memcpy(pSkb->data, ethHeaderBuffer, ETH_HEADER_ITU_LENGTH);
		*ppSkb = pSkb;
		
		return dev;
	}
	
	return NULL;
}

void BcmXdslSendErrorSamples(unsigned char lineId, VectorErrorSample *pVectErrorSample)
{
	unsigned char *pPayLoadBuffer;
	int lengthToCopy=SYMB_VECT_SEGMENT;
	struct sk_buff *skb;
	unsigned char segmentIdx;
	unsigned short nERBbytes = ADSL_ENDIAN_CONV_USHORT(pVectErrorSample->nERBbytes);
	unsigned short vectLineId = ADSL_ENDIAN_CONV_USHORT(pVectErrorSample->lineId);
	unsigned short syncCounter = ADSL_ENDIAN_CONV_USHORT(pVectErrorSample->syncCounter);
	unsigned char nbPartForSymbol = ((nERBbytes-1)/SYMB_VECT_SEGMENT)+1;
	long mibLen = sizeof(adslMibInfo);
	adslMibInfo *pXdslMib = (void *) AdslCoreGetObjectValue(lineId, NULL, 0, NULL, &mibLen);
	
	BcmCoreDpcSyncEnter(SYNC_RX);	// Avoid sending in the middle of WAN device down notification processing
	if(NULL == wanDev) {
		int	dataAlignMask;
		dataAlignMask = 3;
#if defined(XTM_USE_DSL_WAN_NOTIFY)
		if(!wanDevIsUp) {
			pXdslMib->vectData.vectStat.cntESStatDrop++;
			pXdslMib->vectData.vectStat.cntESPktDrop += nbPartForSymbol;
			BcmCoreDpcSyncExit(SYNC_RX);
			return;
		}
#endif
		if(NULL == (wanDev = BcmXdslCreatePktHdr(lineId, &vectSkbModel))) {
			pXdslMib->vectData.vectStat.cntESStatDrop++;
			pXdslMib->vectData.vectStat.cntESPktDrop += nbPartForSymbol;
			BcmCoreDpcSyncExit(SYNC_RX);
			return;
		}
		if(NULL != vectSkbDev) {
			if(!DevSkbFree(vectSkbDev, 0))
				printk("%s: Still not able to free previous vectSkbDev(0x%px). Discarding it anyway!\n", __FUNCTION__, vectSkbDev);
		}
		if(NULL == (vectSkbDev = DevSkbAllocate(vectSkbModel, VECT_FRAME_SIZE, VECT_SKBS_IN_POOL, 0,0, dataAlignMask, ETH_HEADER_ITU_LENGTH, 0, SKB_HEADROOM_RESERVE))) {
			printk("%s: Errored allocating skb buffer pool!\n", __FUNCTION__);
			wanDev = NULL;
			BcmCoreDpcSyncExit(SYNC_RX);
			return;
		}
	}
	
	for(segmentIdx=0; segmentIdx < nbPartForSymbol; segmentIdx++)
	{
		unsigned short hdrLen;
		unsigned char *pHdrLen;
		
		skb = GetSkb(vectSkbDev, VECT_FRAME_SIZE);
		if (NULL == skb) {
			pXdslMib->vectData.vectStat.cntESStatDrop++;
			pXdslMib->vectData.vectStat.cntESPktSend += segmentIdx;
			pXdslMib->vectData.vectStat.cntESPktDrop += (nbPartForSymbol-segmentIdx);
			BcmCoreDpcSyncExit(SYNC_RX);
			return;
		}
#ifdef SUPPORT_DSL_BONDING
		memcpy(skb->data, (void *)&pXdslMib->vectData.macAddress.macAddress[0], ETH_MAC_ADDR_LEN);
#endif
		pPayLoadBuffer = (unsigned char *)skb->data+ETH_HEADER_ITU_LENGTH;
		pPayLoadBuffer[LINEID_OFFSET] = (vectLineId >> 8) & 0xff;
		pPayLoadBuffer[LINEID_OFFSET+1] = vectLineId & 0xff;
		pPayLoadBuffer[SYNCCOUNTER_OFFSET] = (syncCounter >> 8) & 0xff;
		pPayLoadBuffer[SYNCCOUNTER_OFFSET+1] = syncCounter & 0xff;
		pPayLoadBuffer[PARTID_OFFSET] = segmentIdx;
		if(segmentIdx == (nbPartForSymbol-1))
		{
			pPayLoadBuffer[PARTID_OFFSET] |= 0xC0;		/* Indicates last segment */
			lengthToCopy = nERBbytes - segmentIdx*SYMB_VECT_SEGMENT;
		}
		memcpy(&pPayLoadBuffer[ERRORSAMPLES_OFFSET], &pVectErrorSample->errorMsg[(segmentIdx*SYMB_VECT_SEGMENT)], lengthToCopy);
		
		hdrLen = (unsigned short)(lengthToCopy + 13);	/* 13 => ERRORSAMPLES_OFFSET(5) + LLC PDU(3) + ITU OUI(3) + PRIVATE protocol ID(2) */
		pHdrLen = (unsigned char *)skb->data+(ETH_MAC_ADDR_LEN<<1);
		pHdrLen[0] = (hdrLen >> 8) & 0xff;
		pHdrLen[1] = hdrLen & 0xff;
		
		skb->len = ETH_HEADER_ITU_LENGTH + ERRORSAMPLES_OFFSET + lengthToCopy;
		skb->tail = (sk_buff_data_t)(uintptr_t)(skb->data + skb->len);
		skb->mark |= 0x7;	/* Change packet priority to highest */
		if(0 != DEV_VECT_TRANSMIT(skb)) {
			pXdslMib->vectData.vectStat.cntESStatDrop++;
			pXdslMib->vectData.vectStat.cntESPktSend += segmentIdx;
			pXdslMib->vectData.vectStat.cntESPktDrop += (nbPartForSymbol-segmentIdx);
			BcmCoreDpcSyncExit(SYNC_RX);
			return;
		}
	}
	
	pXdslMib->vectData.vectStat.cntESStatSend++;
	pXdslMib->vectData.vectStat.cntESPktSend += nbPartForSymbol;
	BcmCoreDpcSyncExit(SYNC_RX);
	
	return;
}
