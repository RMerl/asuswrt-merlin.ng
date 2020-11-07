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
 * DslFramer.h 
 *
 * Description:
 *	This file contains common DSL framer definitions
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.3 $
 *
 * $Id: DslFramer.h,v 1.3 2004/07/21 01:39:41 ilyas Exp $
 *
 * $Log: DslFramer.h,v $
 * Revision 1.3  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.2  2004/04/12 23:41:10  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *
 *****************************************************************************/

#ifndef	DslFramerHeader
#define	DslFramerHeader

#include "DList.h"

#define	kDslFramerInitialized			0x80000000

/* status codes */

#define	kDslFramerRxFrame				1
#define	kDslFramerRxFrameErr			2
#define kDslFramerTxFrame				3
#define	kDslFramerTxFrameErr			4

#define	kDslFramerRxFrameErrFlushed		1
#define	kDslFramerRxFrameErrAbort		2
#define	kDslFramerRxFrameErrPhy			3

#define	kDslFramerTxFrameErrFlushed		1


typedef	struct _dslFramerBufDesc {
	int		pkId;
	int		bufFlags;
	void		*bufPtr;
	int		bufLen;
} dslFramerBufDesc;

/* data bufDesc flags */

#define kDslFramerStartNewFrame			1
#define kDslFramerEndOfFrame			2
#define kDslFramerAbortFrame			4

#define kDslFramerExtraByteShift		3
#define kDslFramerExtraByteMask			(0x7 << kDslFramerExtraByteShift)

typedef struct _dslFramerControl {
	bitMap					setup;
	dslFrameHandlerType		rxIndicateHandlerPtr;
	dslFrameHandlerType		txCompleteHandlerPtr;
	dslStatusHandlerType	statusHandlerPtr;
	uint					statusCode;
	uint					statusOffset;

	int						nRxBuffers;
	int						nRxBufSize;
	int						nRxPackets;

	dslFrame				*freeBufListPtr;
	void					*freeBufPool;
	void					*pBufMemory;

	dslFrame				*freePacketListPtr;
	void					*freePacketPool;

	/* RX working data set */

	dslFrame				*pRxFrame;
	dslFrameBuffer			*pRxBuf;
	uchar					*pRxBufData;
	uchar					*pRxBufDataEnd;
	int						rxFrameLen;

	/* TX working data set */

	DListHeader				dlistTxWaiting;
	dslFrame				*pTxFrame;
	dslFrameBuffer			*pTxBuf;
	uchar					*pTxBufData;
	uchar					*pTxBufDataEnd;

	/* stats data */

	uint					dslByteCntRxTotal;
	uint					dslByteCntTxTotal;

	uint					dslFrameCntRxTotal;
	uint					dslFrameCntRxErr;
	uint					dslFrameCntTxTotal;
	
} dslFramerControl;


extern Boolean  DslFramerInit(
			void					*gDslVars,
			dslFramerControl		*dfCtrl,
			bitMap					setup,
			uint					statusCode,
			uint					statusOffset,
			dslFrameHandlerType		rxIndicateHandlerPtr,
			dslFrameHandlerType		txCompleteHandlerPtr,
			dslStatusHandlerType	statusHandlerPtr,
			int						rxBufNum,
			int						rxBufSize,
			int						rxPacketNum);
extern void DslFramerClose(void *gDslVars, dslFramerControl *dfCtrl);
extern void DslFramerSendFrame(void *gDslVars, dslFramerControl *dfCtrl, dslFrame *pFrame);
extern void DslFramerReturnFrame(void *gDslVars, dslFramerControl *dfCtrl, dslFrame *pFrame);


extern Boolean DslFramerRxGetPtr(void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern void	DslFramerRxDone  (void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern Boolean	DslFramerTxGetPtr(void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern void	DslFramerTxDone(void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern Boolean DslFramerTxIdle (void *gDslVars, dslFramerControl *dfCtrl);
extern void DslFramerTxFlush(void *gDslVars, dslFramerControl *dfCtrl);

extern void * DslFramerGetFramePoolHandler(dslFramerControl *dfCtrl);
extern void DslFramerClearStat(dslFramerControl *dfCtrl);

extern void DslFramerRxFlushFrame (void *gDslVars, dslFramerControl *dfCtrl, int errCode);
extern void DslFramerRxFlush(void *gDslVars, dslFramerControl *dfCtrl);

#endif	/* DslFramerHeader */
