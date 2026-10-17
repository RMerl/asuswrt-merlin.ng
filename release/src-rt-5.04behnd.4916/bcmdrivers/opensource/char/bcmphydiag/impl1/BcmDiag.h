/*
   Copyright (c) 2002 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2002:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    :> 
*/

#ifndef _BCM_DIAG_H
#define _BCM_DIAG_H

/* Incldes. */
//#include <bcmadsl.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/vmalloc.h>
#include "BcmDiagDrv.h"


//#define	DIAG_ALLOC_SKB_DYNAMICALLY

#define SKB_PRE_ALLOC_SIZE		1536
#define SHORT_SKB_PRE_ALLOC_SIZE	128
#ifdef DIAG_ALLOC_SKB_DYNAMICALLY
#define NUM_OF_SKBS_IN_POOL		1
#define NUM_OF_SHORT_SKBS_IN_POOL	1
#else
#define NUM_OF_SKBS_IN_POOL		32
#define NUM_OF_SHORT_SKBS_IN_POOL	256
#endif

#if 0
typedef struct BcmDiagDrvSkbPool {
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
} BcmDiagDrvSkbPool;

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
#define DEV_TRANSMIT(x) BcmDiagPendingSkbAdd(x)
#endif
#endif

#define PTRDIAGSOCKFRAME(skb) ((diagSockFrame *) ((uintptr_t)skb->data-DIAG_FRAME_PAD_SIZE))

/**
 * When we rerouting skb's data back to userspace (ie, not using DEV_TRANSMIT_)
 * we re-use some of skb's fields to pass some information:
 * SKB_REROUTE_FIELD(skb)              - if equals to 0, means we want to reroute this skb
 * SKB_REROUTE_CLIENT_ADDR_FIELD(skb)  - placeholder to preserve the client's ip
 * SKB_REROUTE_DEST_FIELD(skb)         - placeholder to specify where are we sending the data to - diag, gdb, gui.
 */
#define SKB_REROUTE_FIELD(skb)                  ( PTRDIAGSOCKFRAME(skb)->dsfh.ipHdr.dstAddr )
#define SKB_REROUTE_CLIENT_ADDR_FIELD(skb)      ( PTRDIAGSOCKFRAME(skb)->dsfh.ipHdr.srcAddr )
#define SKB_REROUTE_DEST_FIELD(skb)             ( PTRDIAGSOCKFRAME(skb)->dsfh.udpHdr.dstPort )

/***************************/
typedef struct {
   void     *pBuf;
   int      len;
} diagDrvFragment;

typedef struct _diagIpHeader {
	uint8	ver_hl;			/* version & header length */
	uint8	tos;			/* type of service */
	uint16	len;			/* total length */
	uint16	id;				/* identification */
	uint16	off;			/* fragment offset field */
	uint8	ttl;			/* time to live */
	uint8	proto;			/* protocol */
	uint16	checksum;		/* checksum */
	uint32	srcAddr;
	uint32	dstAddr;		/* source and dest address */
} diagIpHeader;

typedef struct _diagUdpHeader {
	uint16	srcPort;		/* source port */
	uint16	dstPort;		/* destination port */
	uint16	len;			/* udp length */
	uint16	checksum;		/* udp checksum */
} diagUdpHeader;

#define DIAG_FRAME_PAD_SIZE		2
#define DIAG_DMA_MAX_DATA_SIZE	1200
#define DIAG_FRAME_HEADER_LEN	(sizeof(diagSockFrame) - DIAG_DMA_MAX_DATA_SIZE - DIAG_FRAME_PAD_SIZE)

struct diagSockFrameHdr {
	uint8			pad[DIAG_FRAME_PAD_SIZE];
	uint8			dstMacAddr[6];
	uint8			srcMacAddr[6];
	short			etype;
	diagIpHeader	ipHdr;
	diagUdpHeader	udpHdr;
	LogProtoHeader	diagHdr;
};

typedef struct _diagSockFrame {
	struct diagSockFrameHdr	dsfh;
	uint8			diagData[DIAG_DMA_MAX_DATA_SIZE];
} diagSockFrame;

typedef struct _diagSockFrameDataAlign {
	union {
		SINT64	pad64;
		struct {
			uint32	pad32;
			uint16	pad16;
			uint8	pad[DIAG_FRAME_PAD_SIZE];
		};
	};
	uint8			dstMacAddr[6];
	uint8			srcMacAddr[6];
	short			etype;
	diagIpHeader	ipHdr __attribute__((packed));
	diagUdpHeader	udpHdr __attribute__((packed));
	LogProtoHeader	diagHdr;
	uint8			diagData[DIAG_DMA_MAX_DATA_SIZE];
} diagSockFrameDataAlign;

typedef struct {
	uint32		diagDmaLogBlockNum;

	struct net_device	*dbgDev;
	struct net_device	*gdbDev;
	struct sk_buff		*skbModel;
	struct sk_buff		*skbGdb;

    // the following two flags have dual role:
    // indicate that the connection is established,
    // and the fact that we want skb reroute
    uint32                skbModelReroute; // reroute skb thru ioctl
    uint32                skbGdbReroute;   // reroute skb thru ioctl
} bcmDiagCtrlType;

void BcmDiagDrvInit(void);
int BcmDiagIsActive(void);

BCMDIAG_STATUS BcmDiagProcessCmd(PDIAGDRV_DIAG pBcmDiag);
int BcmDiagPendingSkbMsgQueueSize(void);
void BcmDiagPendingSkbMsgQueueEnable(int bEn);
void BcmDiagPendingSkbMsgQueuePurge(int bFreeMem);
void BcmDiagPendingSkbMsgQueueClear(void);
//int BcmDiagPendingSkbAdd(struct sk_buff* skb);
void BcmDiagPendingSkbMsgRead(void* d, long* dlen);
void BcmDiagProcWakeup(int procMsgType);

#endif

