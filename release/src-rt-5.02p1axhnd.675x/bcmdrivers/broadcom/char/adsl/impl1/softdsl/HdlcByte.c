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
 * HdlcByte.c -- HDLC byte framer 
 *
 * Description:
 *	This file contains root HDLC byte framer functions
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.5 $
 *
 * $Id: HdlcByte.c,v 1.5 2008/03/20 01:10:45 tonytran Exp $
 *
 * $Log: HdlcByte.c,v $
 * Revision 1.5  2008/03/20 01:10:45  tonytran
 * Fixed the driver handling of received abort condition problem
 *
 * Revision 1.4  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.3  2003/07/18 18:51:05  ilyas
 * Added mode (default) to pass address and control field
 *
 * Revision 1.2  2001/12/13 21:44:05  liang
 * Add newline to end of file to prevent linux compiler warning.
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "DslFramer.h"
#include "HdlcFramer.h"
#include "BlockUtil.h"

Public void HdlcByteReset(void *gDslVars, hdlcByteControl *hbyCtrl)
{
	hbyCtrl->pTxData				= NULL;
	hbyCtrl->pTxDataEnd				= NULL;

	hbyCtrl->pRxData				= NULL;
	hbyCtrl->pRxDataEnd				= NULL;

	HdlcFramerTxFrameInit(hbyCtrl);
	HdlcFramerRxFrameInit(hbyCtrl);
}

Public Boolean HdlcByteInit(
	void							*gDslVars, 
	hdlcByteControl					*hbyCtrl,
	bitMap							setup, 
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler,
	dslFramerDataDoneHandlerType	rxDataDoneHandler,
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler,
	dslFramerDataDoneHandlerType	txDataDoneHandler)
{
	hbyCtrl->setup					= setup;
	hbyCtrl->rxDataGetPtrHandler	= rxDataGetPtrHandler;
	hbyCtrl->rxDataDoneHandler		= rxDataDoneHandler;
	hbyCtrl->txDataGetPtrHandler	= txDataGetPtrHandler;
	hbyCtrl->txDataDoneHandler		= txDataDoneHandler;

	HdlcByteReset(gDslVars, hbyCtrl);
	return true;
}

Private uchar * HdlcByteRxData(void *gDslVars, hdlcByteControl *hbyCtrl, uchar *srcPtr, int nBytes, Boolean *pEof)
{
	uchar	*srcEndPtr, *dstPtr, b;
	uint	crc;

	srcEndPtr   = srcPtr + nBytes;
	dstPtr		= hbyCtrl->pRxData;
	crc			= hbyCtrl->rxCrc;
	*pEof		= false;

	if ((hbyCtrl->rxEscChar) && (srcPtr != srcEndPtr)) {
		hbyCtrl->rxEscChar = false;
		b = *srcPtr++;
		if(HDLC_BYTE_FLAG == b) {		/* Frame abort condition */
			*pEof = true;
			hbyCtrl->rxCrc = HDLC16_CRC_INIT;	/* Make sure crc is NOT HDLC16_GOOD_CRC to force frame abort */
			return srcPtr;
		}
		else {
			b ^= 0x20;
			*dstPtr++ = b;
			crc = Hdlc16UpdateCrc(hbyCtrl->rxCrc, b);
			hbyCtrl->rxCrc = crc;
		}
	}
	
	while (srcPtr != srcEndPtr) {
		b = *srcPtr++;
		if (HDLC_BYTE_FLAG == b) {
			*pEof = true;
			break;
		}
		
		if (HDLC_BYTE_ESC == b) {
			if (srcPtr == srcEndPtr) {
				hbyCtrl->rxEscChar = true;
				break;
			}
			else if(HDLC_BYTE_FLAG == *srcPtr) {	/* Frame abort condition */
				*pEof = true;
				crc = HDLC16_CRC_INIT;	/* Make sure crc is not HDLC16_GOOD_CRC to force frame abort */
				break;
			}
			else
				b = *srcPtr++ ^ 0x20;
		}
		*dstPtr++ = b;
		crc = Hdlc16UpdateCrc(crc, b);
	}

	hbyCtrl->rxCrc		= crc;
	hbyCtrl->pRxData	= dstPtr;
	return srcPtr;
}

Public int HdlcByteRx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *srcPtr)
{
	uchar					*srcEndPtr, frameState, b;
	int						n;
	Boolean					flag;
	dslFramerBufDesc		bufDesc;

	srcEndPtr  = srcPtr + nBytes;
	frameState = hbyCtrl->rxFrameState;

	do {
		switch (frameState) {
			case HDLC_STATE_START_FLAG:
				b = *srcPtr++;
				if (HDLC_BYTE_FLAG == b)
					frameState		= HDLC_STATE_ADDRESS;
				break;
			case HDLC_STATE_ADDRESS:
				if (0 == (hbyCtrl->setup & kHdlcSpecialAddrCtrl)) {
					hbyCtrl->rxCrc = HDLC16_CRC_INIT;
					goto hdlcByteStartData;
				}

				b = *srcPtr++;
				if (HDLC_BYTE_FLAG != b) {
					hbyCtrl->rxCrc = Hdlc16UpdateCrc(HDLC16_CRC_INIT, b);
					frameState  = HDLC_STATE_CONTROL;
				}
				break;
			case HDLC_STATE_CONTROL:
				b = *srcPtr++;
				if (HDLC_BYTE_FLAG == b) {
					frameState = HDLC_STATE_ADDRESS;
					break;
				}
				hbyCtrl->rxCrc = Hdlc16UpdateCrc(hbyCtrl->rxCrc, b);

hdlcByteStartData:
				if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
					HdlcFramerRxGetData(hbyCtrl, kDslFramerStartNewFrame);

				if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
					frameState = HDLC_STATE_START_FLAG;
				else {
					frameState = HDLC_STATE_DATA;
					hbyCtrl->rxFrameLen= 0;
					hbyCtrl->rxEscChar	= false;
				}
				break;

			case HDLC_STATE_DATA:
				n = hbyCtrl->pRxDataEnd - hbyCtrl->pRxData;
				if (n > (srcEndPtr - srcPtr))
					n = srcEndPtr - srcPtr;

				srcPtr = HdlcByteRxData(gDslVars, hbyCtrl, srcPtr, n, &flag);
				if (flag) {
					bufDesc.bufFlags = (HDLC16_GOOD_CRC == hbyCtrl->rxCrc ? 
						kDslFramerEndOfFrame : kDslFramerAbortFrame);
					bufDesc.bufFlags |= 2 << kDslFramerExtraByteShift;
					bufDesc.bufLen = hbyCtrl->rxDataLen - (hbyCtrl->pRxDataEnd - hbyCtrl->pRxData);

					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
					frameState = HDLC_STATE_ADDRESS;
					hbyCtrl->pRxDataEnd = hbyCtrl->pRxData;
				}
				else if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd) {
					bufDesc.bufLen = hbyCtrl->rxDataLen;
					bufDesc.bufFlags = 0;
					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
					HdlcFramerRxGetData(hbyCtrl, 0);
					if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
						frameState = HDLC_STATE_START_FLAG;
				}
				break;
		}
	} while (srcPtr != srcEndPtr);

	hbyCtrl->rxFrameState = frameState;

	return nBytes;
}

Public int NoHdlcRxData(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *srcPtr)
{
	uchar					*srcEndPtr, frameState;
	int						n;
	dslFramerBufDesc		bufDesc;

	srcEndPtr  = srcPtr + nBytes;
	frameState = hbyCtrl->rxFrameState;

	do {
		switch (frameState) {
			case HDLC_STATE_START_FLAG:
				srcPtr = srcEndPtr;
				break;
			case HDLC_STATE_END_FLAG:
				hbyCtrl->rxFrameLen -= (srcEndPtr - srcPtr);
				if (hbyCtrl->rxFrameLen <= 0) {
					bufDesc.bufFlags = kDslFramerAbortFrame;
					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);

					frameState = HDLC_STATE_START_FLAG;
					hbyCtrl->pRxDataEnd = hbyCtrl->pRxData;
				}
				srcPtr = srcEndPtr;
				break;

			case HDLC_STATE_DATA:
				n = hbyCtrl->pRxDataEnd - hbyCtrl->pRxData;
				if (n > (srcEndPtr - srcPtr))
					n = srcEndPtr - srcPtr;
				BlockByteMove(n, srcPtr, hbyCtrl->pRxData);
				hbyCtrl->pRxData += n;
				srcPtr += n;
				hbyCtrl->rxFrameLen -= n;
				if (hbyCtrl->rxFrameLen <= 0) {
					bufDesc.bufFlags = kDslFramerEndOfFrame;
					// bufDesc.bufFlags |= 2 << kDslFramerExtraByteShift;
					bufDesc.bufLen = hbyCtrl->rxDataLen - (hbyCtrl->pRxDataEnd - hbyCtrl->pRxData);
					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);

					frameState = HDLC_STATE_START_FLAG;
					hbyCtrl->pRxData = hbyCtrl->pRxDataEnd;
					srcPtr = srcEndPtr;
				}
				else if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd) {
					bufDesc.bufFlags = 0;
					// bufDesc.bufFlags |= 2 << kDslFramerExtraByteShift;
					bufDesc.bufLen = hbyCtrl->rxDataLen;
					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
					HdlcFramerRxGetData(hbyCtrl, 0);
					if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
						frameState = HDLC_STATE_END_FLAG;
				}
				break;
		}
	} while (srcPtr != srcEndPtr);

	hbyCtrl->rxFrameState = frameState;
	return nBytes;
}

Public void NoHdlcRxStart(void *gDslVars, hdlcByteControl *hbyCtrl, uchar *hdrPtr)
{
	dslFramerBufDesc		bufDesc;
	int frameLen = ((((uint) hdrPtr[0] << 8) | hdrPtr[1]) >> 4) & 0x3FF;

	if (hbyCtrl->rxFrameLen > 0) {
		bufDesc.bufFlags = kDslFramerAbortFrame;
		(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
		hbyCtrl->pRxData = hbyCtrl->pRxDataEnd;
		hbyCtrl->rxFrameLen = 0;
		hbyCtrl->rxFrameState = HDLC_STATE_START_FLAG;
	}

	if (frameLen < 1)  {
		hbyCtrl->rxFrameState = HDLC_STATE_START_FLAG;
		return;
	}

	if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
		HdlcFramerRxGetData(hbyCtrl, kDslFramerStartNewFrame);

	if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd) {
		bufDesc.bufFlags = kDslFramerAbortFrame;
		(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
		hbyCtrl->rxFrameState = HDLC_STATE_START_FLAG;
		return;
	}

	hbyCtrl->rxFrameState  = HDLC_STATE_DATA;
	hbyCtrl->pRxData[0] = hdrPtr[0];
	hbyCtrl->pRxData[1] = hdrPtr[1];
	hbyCtrl->pRxData += 2;
	hbyCtrl->rxFrameLen = frameLen - 1; /* process 2 byte control field */
}

Public void HdlcByteRxFlush(void *gDslVars, hdlcByteControl *hbyCtrl)
{
	hbyCtrl->rxFrameState = HDLC_STATE_START_FLAG;
	hbyCtrl->pRxData = hbyCtrl->pRxDataEnd;
	hbyCtrl->rxFrameLen = 0;
}


Private uchar * HdlcByteTxData(void *gDslVars, hdlcByteControl *hbyCtrl, uchar *dstPtr, int nBytes)
{
	uchar	*dstEndPtr, *srcPtr, b;
	uint	crc;

	dstEndPtr   = dstPtr + nBytes;
	srcPtr		= hbyCtrl->pTxData;
	crc			= hbyCtrl->txCrc;

	do {
		b = *srcPtr++;
		crc = Hdlc16UpdateCrc(crc, b);
		if ((HDLC_BYTE_FLAG == b) || (HDLC_BYTE_ESC == b)) {
			*dstPtr++ = HDLC_BYTE_ESC;
			if (dstPtr == dstEndPtr) {
				hbyCtrl->txCharPending = b ^ 0x20;
				break;
			}
			else
				*dstPtr = b ^ 0x20;
		}
		else
			*dstPtr = b;
	} while (++dstPtr != dstEndPtr);

	hbyCtrl->txCrc		= crc;
	hbyCtrl->pTxData	= srcPtr;
	return	dstPtr;
}

Public int HdlcByteTx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *dstPtr)
{
	uchar	*dstEndPtr, b;
	uchar	frameState;
	int		n;

	dstEndPtr  = dstPtr + nBytes;
	frameState = hbyCtrl->txFrameState;

	do {
		switch (frameState) {
			case HDLC_STATE_START_FLAG:
			case HDLC_STATE_END_FLAG:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
					break;
				}

				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd)
					HdlcFramerTxGetData(hbyCtrl);

				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd) {
					if (hbyCtrl->setup & kHdlcTxIdleStop) {
						*dstPtr++  = HDLC_BYTE_FLAG;
						frameState = HDLC_STATE_START_FLAG;
						nBytes = dstPtr - (dstEndPtr - nBytes);
					}
					else 
						BlockByteFill(dstEndPtr-dstPtr, HDLC_BYTE_FLAG, dstPtr);

					dstPtr = dstEndPtr;
				}
				else {
					*dstPtr++ = HDLC_BYTE_FLAG;
					frameState= HDLC_STATE_ADDRESS;
				}
				break;

			case HDLC_STATE_ADDRESS:
				if (0 == (hbyCtrl->setup & kHdlcSpecialAddrCtrl)) {
					hbyCtrl->txCrc = HDLC16_CRC_INIT;
					frameState = HDLC_STATE_DATA;
					break;
				}

				*dstPtr++ = HDLC_ADDR;
				hbyCtrl->txCrc = Hdlc16UpdateCrc(HDLC16_CRC_INIT, HDLC_ADDR);
				frameState = HDLC_STATE_CONTROL;
				break;
			case HDLC_STATE_CONTROL:
				*dstPtr++ = HDLC_CTRL;
				hbyCtrl->txCrc = Hdlc16UpdateCrc(hbyCtrl->txCrc, HDLC_CTRL);
				frameState = HDLC_STATE_DATA;
				break;
			case HDLC_STATE_FCS1:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
				}
				else {
					hbyCtrl->txCrc = HDLC16_CRC_FINAL(hbyCtrl->txCrc);
					b = hbyCtrl->txCrc & 0xFF;
					if ((HDLC_BYTE_FLAG == b) || (HDLC_BYTE_ESC == b)) {
						hbyCtrl->txCharPending = b ^ 0x20;
						b = HDLC_BYTE_ESC;
					}
					*dstPtr++ = b;
					frameState = HDLC_STATE_FCS2;
				}
				break;
			case HDLC_STATE_FCS2:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
				}
				else {
					b= hbyCtrl->txCrc >> 8;
					if ((HDLC_BYTE_FLAG == b) || (HDLC_BYTE_ESC == b)) {
						hbyCtrl->txCharPending = b ^ 0x20;
						b = HDLC_BYTE_ESC;
					}
					*dstPtr++ = b;
					frameState = HDLC_STATE_END_FLAG;
				}
				break;

			case HDLC_STATE_DATA:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
					break;
				}

				n = hbyCtrl->pTxDataEnd - hbyCtrl->pTxData;
				if (n > (dstEndPtr - dstPtr))
					n = dstEndPtr - dstPtr;
				
				dstPtr = HdlcByteTxData(gDslVars, hbyCtrl, dstPtr, n);
				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd) {
					dslFramerBufDesc	bufDesc;

					bufDesc.bufLen = hbyCtrl->txDataLen;
					(hbyCtrl->txDataDoneHandler) (gDslVars, &bufDesc);
					if (0 == (bufDesc.bufFlags & kDslFramerEndOfFrame)) {
						HdlcFramerTxGetData(hbyCtrl);
					}
					else
						frameState = HDLC_STATE_FCS1;
				}
				break;
		}
	} while (dstPtr != dstEndPtr);

	hbyCtrl->txFrameState = frameState;

	return nBytes;
}

Public Boolean HdlcByteTxIdle(void *gDslVars, hdlcByteControl *hbyCtrl)
{
	return (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd) && (HDLC_STATE_START_FLAG == hbyCtrl->txFrameState);
}

Public int NoHdlcGetTxData(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *dstPtr)
{
	uchar	*dstEndPtr, *srcPtr, *srcEndPtr, b, frameState;
	uint	crc;
	int		n, flag = 0;

	dstEndPtr   = dstPtr + nBytes;
	frameState = hbyCtrl->txFrameState;

	do {
		switch (frameState) {
			case HDLC_STATE_START_FLAG:
				HdlcFramerTxGetData(hbyCtrl);
				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd)
					goto tx_done;
				frameState = HDLC_STATE_DATA;
				flag = kDslClearEocMsgGfastHdr;
				break;

			case HDLC_STATE_DATA:
				srcPtr = hbyCtrl->pTxData;
				crc	   = hbyCtrl->txCrc;
				n      = hbyCtrl->pTxDataEnd - srcPtr;
				if (n > (dstEndPtr - dstPtr))
					n = dstEndPtr - dstPtr;
				srcEndPtr = srcPtr + n;
				do {
					b = *srcPtr++;
					crc = Hdlc16UpdateCrc(crc, b);
					*dstPtr++ = b;
				} while (srcPtr != srcEndPtr);
				if (srcPtr == hbyCtrl->pTxDataEnd) {
					dslFramerBufDesc	bufDesc;

					bufDesc.bufLen = hbyCtrl->txDataLen;
					(hbyCtrl->txDataDoneHandler) (gDslVars, &bufDesc);
					if (0 == (bufDesc.bufFlags & kDslFramerEndOfFrame)) {
						HdlcFramerTxGetData(hbyCtrl);
						srcPtr = hbyCtrl->pTxData;
					}
					else
						frameState = HDLC_STATE_FCS1;
				}
				hbyCtrl->txCrc		= crc;
				hbyCtrl->pTxData	= srcPtr;
				break;

			case HDLC_STATE_FCS1:
				hbyCtrl->txCrc = HDLC16_CRC_FINAL(hbyCtrl->txCrc);
				//*dstPtr++ = hbyCtrl->txCrc >> 8;
				*dstPtr++ = hbyCtrl->txCrc & 0xFF;
				frameState = HDLC_STATE_FCS2;
				break;
			case HDLC_STATE_FCS2:
				//*dstPtr++ = hbyCtrl->txCrc & 0xFF;
				*dstPtr++ = hbyCtrl->txCrc >> 8;
				frameState = HDLC_STATE_START_FLAG;
				hbyCtrl->txCrc = HDLC16_CRC_INIT;
				goto tx_done;
		}
	} while (dstPtr != dstEndPtr);

tx_done:
	hbyCtrl->txFrameState = frameState;
	return	(dstPtr - (dstEndPtr - nBytes)) | flag;
}
