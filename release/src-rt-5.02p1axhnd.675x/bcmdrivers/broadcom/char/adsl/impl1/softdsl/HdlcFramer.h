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
 * HdlcFramer.h 
 *
 * Description:
 *	This file contains common HDLC definitions for bit/byte stuffing
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.3 $
 *
 * $Id: HdlcFramer.h,v 1.3 2004/07/21 01:39:41 ilyas Exp $
 *
 * $Log: HdlcFramer.h,v $
 * Revision 1.3  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.2  2003/07/18 18:51:05  ilyas
 * Added mode (default) to pass address and control field
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *
 *****************************************************************************/

#ifndef	HdlcFramerHeader
#define	HdlcFramerHeader

/* setup bitmap definitions */

#define	kHdlcSetupShift			16
#define	kHdlcSetupMask			((int)0xFFFF << kHdlcSetupShift)

#define	kHdlcCrcMask			0x00030000
#define	kHdlcCrcNone			0x00000000
#define	kHdlcCrc16				0x00010000
#define	kHdlcCrc32				0x00020000

#define	kHdlcTxIdleStop			0x00040000
#define	kHdlcSpecialAddrCtrl	0x00080000

extern	ushort HdlcCrc16Table[];

#define HDLC16_CRC_INIT			0xFFFF
#define HDLC16_CRC_FINAL(crc)	((crc) ^ 0xFFFF)
#define HDLC16_GOOD_CRC			0xF0B8
#define	Hdlc16UpdateCrc(crc,b)	((crc) >> 8) ^ HdlcCrc16Table[((crc) ^ (b)) & 0xFF]

extern	uint  HdlcCrc32Table[];

#define HDLC32_CRC_INIT			0xFFFFFFFF
#define HDLC32_CRC_FINAL(crc)	((crc) ^ 0xFFFFFFFF)
#define HDLC32_GOOD_CRC			0xDEBB20E3
#define	Hdlc32UpdateCrc(crc,b)	((crc) >> 8) ^ HdlcCrc32Table[((crc) ^ (b)) & 0xFF]

extern	uint  HdlcCrc32Table[];

/* HDLC common fields */

#define	HDLC_ADDR				0xFF
#define	HDLC_CTRL				0x3

#define	HDLC_BYTE_FLAG			0x7E
#define	HDLC_BYTE_ESC			0x7D

/* HDLC frame assembly states */

#define	HDLC_STATE_START_FLAG	0
#define	HDLC_STATE_ADDRESS		(HDLC_STATE_START_FLAG + 1)
#define	HDLC_STATE_CONTROL		(HDLC_STATE_START_FLAG + 2)
#define	HDLC_STATE_DATA			(HDLC_STATE_START_FLAG + 3)
#define	HDLC_STATE_FCS1			(HDLC_STATE_START_FLAG + 4)
#define	HDLC_STATE_FCS2			(HDLC_STATE_START_FLAG + 5)
#define	HDLC_STATE_END_FLAG		(HDLC_STATE_START_FLAG + 6)


/* HDLC common types */

typedef struct _hdlcByteControl {
	bitMap					setup;
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler;
	dslFramerDataDoneHandlerType	rxDataDoneHandler;
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler;
	dslFramerDataDoneHandlerType	txDataDoneHandler;

	/* RX working data set */

	uchar					rxFrameState;
	int						rxFrameLen;
	uint					rxCrc;
	Boolean					rxEscChar;

	uchar					*pRxData;
	uchar					*pRxDataEnd;
	int					rxDataLen;

	/* TX working data set */

	uchar					txFrameState;
	int						txFrameLen;
	uint					txCrc;
	int						txCharPending;

	uchar					*pTxData;
	uchar					*pTxDataEnd;
	int					txDataLen;
} hdlcByteControl;

typedef struct _hdlcBitControl {
	bitMap					setup;
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler;
	dslFramerDataDoneHandlerType	rxDataDoneHandler;
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler;
	dslFramerDataDoneHandlerType	txDataDoneHandler;

	/* RX working data set */

	uchar					rxFrameState;
	int						rxFrameLen;
	uint					rxCrc;
	int						rxNibblePending;
	int						rxOutVal;
	uchar					rxOutBits;
	uchar					rxRem1Bits;
	Boolean					rxEscChar; /* ???? */

	uchar					*pRxData;
	uchar					*pRxDataEnd;
	int					rxDataLen;

	/* TX working data set */

	uchar					txFrameState;
	int						txFrameLen;
	uint					txCrc;
	int						txOutVal;
	uchar					txOutBits;
	uchar					txLast1Bits;
	int						txCharPending;	/* ???? */

	uchar					*pTxData;
	uchar					*pTxDataEnd;
	int					txDataLen;
} hdlcBitControl;

/* HDLC common functions */

#define HdlcFramerTxFrameInit(ctrl)	do {			\
	ctrl->txFrameState = HDLC_STATE_START_FLAG;		\
	ctrl->txFrameLen   = 0;							\
	ctrl->txCrc		   = HDLC16_CRC_INIT;			\
	ctrl->txCharPending= -1;						\
} while (0)

#define HdlcFramerRxFrameInit(ctrl) do {			\
	ctrl->rxFrameState = HDLC_STATE_START_FLAG;		\
	ctrl->rxFrameLen   = 0;							\
	ctrl->rxCrc		   = HDLC16_CRC_INIT;			\
	ctrl->rxEscChar	   = false;						\
} while (0)

#define HdlcFramerTxGetData(ctrl)  do {								\
	dslFramerBufDesc	bufDesc;									\
																	\
	if ((ctrl->txDataGetPtrHandler) (gDslVars, &bufDesc)) {			\
		ctrl->pTxData = bufDesc.bufPtr;								\
		ctrl->pTxDataEnd = (uchar*)bufDesc.bufPtr + bufDesc.bufLen;	\
		ctrl->txDataLen	 = bufDesc.bufLen;							\
		if (bufDesc.bufFlags & kDslFramerStartNewFrame)				\
			HdlcFramerTxFrameInit(ctrl);							\
	}																\
	else {															\
		HdlcFramerTxFrameInit(ctrl);								\
	}																\
} while (0);

#define HdlcFramerRxGetData(ctrl, frFlag)  do {						\
	dslFramerBufDesc		bufDesc;								\
																	\
	bufDesc.bufFlags = frFlag;										\
	if ((ctrl->rxDataGetPtrHandler) (gDslVars, &bufDesc)) {			\
		ctrl->pRxData = bufDesc.bufPtr;								\
		ctrl->pRxDataEnd = (uchar*)bufDesc.bufPtr + bufDesc.bufLen;	\
		ctrl->rxDataLen	 = bufDesc.bufLen;							\
	}																\
} while (0);

/* HDLC functions headers */

extern Boolean HdlcByteInit(
	void							*gDslVars, 
	hdlcByteControl					*hbyCtrl,
	bitMap							setup, 
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler,
	dslFramerDataDoneHandlerType	rxDataDoneHandler,
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler,
	dslFramerDataDoneHandlerType	txDataDoneHandler);

extern void HdlcByteReset(void *gDslVars, hdlcByteControl *hbyCtrl);
extern void HdlcByteRxFlush(void *gDslVars, hdlcByteControl *hbyCtrl);
extern int HdlcByteRx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *srcPtr) FAST_TEXT;
extern int NoHdlcRxData(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *srcPtr) FAST_TEXT;
extern void NoHdlcRxStart(void *gDslVars, hdlcByteControl *hbyCtrl, uchar *hdrPtr);
extern int HdlcByteTx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *dstPtr) FAST_TEXT;
extern Boolean HdlcByteTxIdle(void *gDslVars, hdlcByteControl *hbyCtrl);
extern int NoHdlcGetTxData(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *dstPtr) FAST_TEXT;

extern Boolean HdlcBitInit(
	void							*gDslVars, 
	hdlcBitControl					*hbiCtrl,
	bitMap							setup, 
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler,
	dslFramerDataDoneHandlerType	rxDataDoneHandler,
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler,
	dslFramerDataDoneHandlerType	txDataDoneHandler);

extern void HdlcBitReset(void *gDslVars, hdlcByteControl *hbiCtrl);
extern int HdlcBitRx(void *gDslVars, hdlcBitControl *hbiCtrl, int nBytes, uchar *srcPtr) FAST_TEXT;
extern int HdlcBitTx(void *gDslVars, hdlcBitControl *hbiCtrl, int nBytes, uchar *dstPtr) FAST_TEXT;

#endif	/* HdlcFramerHeader */
