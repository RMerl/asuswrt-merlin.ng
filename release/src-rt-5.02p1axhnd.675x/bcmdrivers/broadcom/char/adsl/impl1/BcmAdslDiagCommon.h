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
 * BcmAdslDiagCommon.h -- Internal definitions for ADSL diagnostic
 *
 * Description:
 *	Internal definitions for ADSL core driver
 *
 *
 ****************************************************************************/

#if !defined(_BCM_ADSL_DIAG_COMMON_H_)
#define _BCM_ADSL_DIAG_COMMON_H_

#include "DiagDef.h"
#include "bcmadsl.h"

#if defined(_NOOS)

#elif defined(__KERNEL__)
#include "BcmAdslDiagLinux.h"
#elif defined(__ECOS)
#include "BcmAdslDiagEcos.h"
#endif

typedef struct {
   void     *pBuf;
   int      len;
} dslDrvFragment;

typedef struct _diagIpHeader {
	uchar	ver_hl;			/* version & header length */
	uchar	tos;			/* type of service */
	ushort	len;			/* total length */
	ushort	id;				/* identification */
	ushort	off;			/* fragment offset field */
	uchar	ttl;			/* time to live */
	uchar	proto;			/* protocol */
	ushort	checksum;		/* checksum */
	uint	srcAddr;
	uint	dstAddr;		/* source and dest address */
} diagIpHeader;

typedef struct _diagUdpHeader {
	ushort	srcPort;		/* source port */
	ushort	dstPort;		/* destination port */
	ushort	len;			/* udp length */
	ushort	checksum;		/* udp checksum */
} diagUdpHeader;

#define DIAG_FRAME_PAD_SIZE		2
#define DIAG_DMA_MAX_DATA_SIZE	1200
#define DIAG_FRAME_HEADER_LEN	(sizeof(diagSockFrame) - DIAG_DMA_MAX_DATA_SIZE - DIAG_FRAME_PAD_SIZE)

struct diagSockFrameHdr {
	uchar			pad[DIAG_FRAME_PAD_SIZE];
	uchar			dstMacAddr[6];
	uchar			srcMacAddr[6];
	short			etype;
	diagIpHeader	ipHdr;
	diagUdpHeader	udpHdr;
	LogProtoHeader	diagHdr;
};

typedef struct _diagSockFrame {
	struct			diagSockFrameHdr;
	uchar			diagData[DIAG_DMA_MAX_DATA_SIZE];
} diagSockFrame;

typedef struct _diagSockFrameDataAlign {
	union {
		SINT64	pad64;
		struct {
			uint	pad32;
			ushort	pad16;
			uchar	pad[DIAG_FRAME_PAD_SIZE];
		};
	};
	uchar			dstMacAddr[6];
	uchar			srcMacAddr[6];
	short			etype;
	diagIpHeader	ipHdr __attribute__((packed));
	diagUdpHeader	udpHdr __attribute__((packed));
	LogProtoHeader	diagHdr;
	uchar			diagData[DIAG_DMA_MAX_DATA_SIZE];
} diagSockFrameDataAlign;

typedef struct {
#if defined(__KERNEL__)
	struct sk_buff		skb;
#elif defined(__ECOS)
	struct sk_buff		skb;
#endif
	uint				len;
	uint				frameNum;
	uint				mark;
	LogProtoHeader		diagHdrDma;
	diagSockFrame			dataFrame;
#if defined(__KERNEL__)
	struct skb_shared_info	skbShareInfo;
#endif
} diagDmaBlock;

typedef struct {
	uint		flags;
	uint		addr;
} adslDmaDesc;

#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define	UNCACHED(p)	(p)
#define	CACHED(p)	(p)
#else
#define	UNCACHED(p)				((void *)((int)(p) | 0x20000000))
#define	CACHED(p)				((void *)((int)(p) & ~0x20000000))
#endif

#define kDiagDmaBlockSizeShift	11
#define kDiagDmaBlockSize		(1 << kDiagDmaBlockSizeShift)

#define DIAG_DESC_TBL_ALIGN_SIZE	0x1000
#define DIAG_DESC_TBL_MAX_SIZE		0x800
#define DIAG_DESC_TBL_SIZE(x)		((x) * sizeof(adslDmaDesc))
#define DIAG_DMA_BLK_SIZE			( sizeof(diagDmaBlock) )

/* Common Diags control structure */
#if !defined(_NOOS)
#define	DIAG_ZEROCOPY_APP_MAX	4

#define	DIAG_ZEROCOPY_CURR_SKB(x)	((x)->pBufPool->skbPool[(x)->bufIndex])
#define	DIAG_ZEROCOPY_CURR_TXDONE_SKB(x)	((x)->pBufPool->skbPool[(x)->bufIndexTxDone])
#define	DIAG_ZEROCOPY_NBUF_MAX(x)	((x)->pBufPool->numOfSkbs)
#define	DIAG_ZEROCOPY_PAD_PTR(s, p)	((void*)((uintptr_t)(s)->data + (p)->pBufPool->skbLengh))

typedef struct {
	adslDmaDesc		*pDescRing;
	dslDrvSkbPool		*pBufPool;
	int	bufIndexTxDone;
	int	bufIndex;
	uint	wrCnt;
	uint	ovrCnt;
	uint	wrErrCnt;
	uint	maxLpCnt;
} diagZeroCopyAppCtrlType;


typedef struct {
	diagZeroCopyAppCtrlType	*pEyeDataAppCtrl[MAX_DSL_LINE];
	int			nZeroCopyAppsActive;
	diagZeroCopyAppCtrlType	*zeroCopyAppTbl[DIAG_ZEROCOPY_APP_MAX];
	uint			diagDmaLogBlockNum;
	uint			diagDataMap[MAX_DSL_LINE];
	uint			diagLogTime[MAX_DSL_LINE];

	struct net_device	*dbgDev;
	struct net_device	*gdbDev;
	struct sk_buff		*skbModel;
	struct sk_buff		*skbGdb;

    // the following two flags have dual role:
    // indicate that the connection is established,
    // and the fact that we want skb reroute
    uint                skbModelReroute; // reroute skb thru ioctl
    uint                skbGdbReroute;   // reroute skb thru ioctl
} diagCtrlType;

extern diagCtrlType diagCtrl;

/* functions used by BcmAdslDiagCommon.c */
dslDrvSkbPool * DevSkbAllocate(struct sk_buff *model,
	int skbMaxBufSize, int numOfSkbsInPool,
	int shortSkbMaxBufSize, int numOfShortSkbsInPool,
	int dataAlignMask, int frameHeaderLength, int dmaZone, int skbHeadRoomReserve);
struct sk_buff * GetSkb(dslDrvSkbPool *skbDev, int len);
int DevSkbFree(dslDrvSkbPool *skbDev, int enabeWA);
int DevSkbSendDiagsPacket(dslDrvSkbPool *skbDev, uint cmd, dslDrvFragment *pFrag, int nFrag);
int __DiagWriteData(struct sk_buff *diagBuf, uint cmd, char *buf0, int len0, char *buf1, int len1);
struct net_device * BcmAdslCoreInitNetDev(PADSL_DIAG pAdslDiag, int port, struct sk_buff **ppskb, char *devname);
diagZeroCopyAppCtrlType * BcmCoreDiagZeroCopyStatAppInit(unsigned char lineId, unsigned char logCmd, struct sk_buff *model, int bufSize, int numOfBuf, 	int dataAlignMask, int frameHeaderLength);
void BcmCoreDiagZeroCopyStatAppUnInit(diagZeroCopyAppCtrlType *pAppCtrl);
int BcmCoreDiagZeroCopyStatAvail(void);
int BcmCoreDiagZeroCopyStatActive(void);
void BcmCoreDiagZeroCopyStatHandler(void);
void BcmCoreDiagZeroCopyTxDoneHandler(void);
#ifdef USE_RESERVE_SHARE_MEM
void BcmCoreDiagReleaseReserveShareMem(void);
#endif

#endif	/* !_NOOS */

void DiagUpdateSkbForDmaBlock(diagDmaBlock *db, int len);
void DiagWriteStatus(void *stat, char *pBuf, int len);

ushort DiagIpComputeChecksum(diagIpHeader *ipHdr);
ushort DiagIpUpdateChecksum(int sum, ushort oldv, ushort newv);
void DiagUpdateDataLen(diagSockFrame *diagFr, int dataLen);

char *ConvertToDottedIpAddr(uint ipAddr, char *buf);

void BcmAdslCoreDiagDataInit(unsigned char lineId);
#endif /* _BCM_ADSL_DIAG_COMMON_H_ */
