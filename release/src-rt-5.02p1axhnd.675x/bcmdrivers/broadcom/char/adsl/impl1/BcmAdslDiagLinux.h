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

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/vmalloc.h>

//#define	DIAG_ALLOC_SKB_DYNAMICALLY

#define	DIAG_SKB_USERS		0x3FFFFFFF
#define SKB_PRE_ALLOC_SIZE		1536
#define SHORT_SKB_PRE_ALLOC_SIZE	128
#ifdef DIAG_ALLOC_SKB_DYNAMICALLY
#define NUM_OF_SKBS_IN_POOL		1
#define NUM_OF_SHORT_SKBS_IN_POOL	1
#else
#define NUM_OF_SKBS_IN_POOL		32
#define NUM_OF_SHORT_SKBS_IN_POOL	256
#endif

#define DiagPacketStart(skb)  ((diagSockFrame *) (skb)->head)

typedef struct dslDrvSkbPool {
	struct sk_buff **skbPool;
	struct sk_buff *skbModel;
	int            numOfSkbs;
	int            numOfShortSkbs;
	int            skbLengh;
	int            shortSkbLengh;
	int            skbBufIndex;
	int            shortSkbBufIndex;
	int            frameHeaderLen;
	int            dataAlignMask;
	unsigned int   extraSkb;
	int            skbHeadRoomReserve;
} dslDrvSkbPool;

#ifndef USE_DEV_TRANSMIT   /* In case the definition is coming from outside, we want to use that */
#define USE_DEV_TRANSMIT 0
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
/* Note: Linux4.04L.02 and older don't have "netdev_ops->ndo_start_xmit" */
#define	DEV_TRANSMIT_(x)	(x)->dev->netdev_ops->ndo_start_xmit (x, (x)->dev)
#else
#define	DEV_TRANSMIT_(x)	dev_queue_xmit(x)
#endif

#if defined(USE_DEV_TRANSMIT) && USE_DEV_TRANSMIT
#define DEV_TRANSMIT(x) DEV_TRANSMIT_(x)
#else
#define DEV_TRANSMIT(x) BcmAdslPendingSkbAdd(x)
#endif

#define PTRDIAGSOCKFRAME(skb) ((diagSockFrame *) ((uintptr_t)skb->data-DIAG_FRAME_PAD_SIZE))

/**
 * When we rerouting skb's data back to userspace (ie, not using DEV_TRANSMIT_)
 * we re-use some of skb's fields to pass some information:
 * SKB_REROUTE_FIELD(skb)              - if equals to 0, means we want to reroute this skb
 * SKB_REROUTE_CLIENT_ADDR_FIELD(skb)  - placeholder to preserve the client's ip
 * SKB_REROUTE_DEST_FIELD(skb)         - placeholder to specify where are we sending the data to - diag, gdb, gui.
 */
#define SKB_REROUTE_FIELD(skb)                  ( PTRDIAGSOCKFRAME(skb)->ipHdr.dstAddr )
#define SKB_REROUTE_CLIENT_ADDR_FIELD(skb)      ( PTRDIAGSOCKFRAME(skb)->ipHdr.srcAddr )
#define SKB_REROUTE_DEST_FIELD(skb)             ( PTRDIAGSOCKFRAME(skb)->udpHdr.dstPort )

int BcmAdslPendingSkbAdd(struct sk_buff* skb);
// void BcmAdslPendingSkbMsgRead(void* d, long* dlen); this dec moved to BcmAdslDiag.h

#define	DIAG_DATA_ALIGN(x, y)	((unsigned char *)(((unsigned long)(x)+(unsigned long)(y)) & ~(y)))
