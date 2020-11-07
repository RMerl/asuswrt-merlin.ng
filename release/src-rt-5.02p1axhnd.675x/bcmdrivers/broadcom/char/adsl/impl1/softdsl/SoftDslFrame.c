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
 * SoftDslFrame.c -- Software Modem main module
 *
 *
 * Description:
 *	This file contains native frame functions 
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.9 $
 *
 * $Id: SoftDslFrame.c,v 1.9 2004/04/13 00:18:50 ilyas Exp $
 *
 * $Log: SoftDslFrame.c,v $
 * Revision 1.9  2004/04/13 00:18:50  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.8  2001/02/10 03:03:09  ilyas
 * Added one more DslFrame function
 *
 * Revision 1.7  2000/07/23 20:52:52  ilyas
 * Added xxxFrameBufSetAddress() function for ATM framer layers
 * Rearranged linkLayer functions in one structure which is passed as a
 * parameter to xxxLinkLayerInit() function to be set there
 *
 * Revision 1.6  2000/07/18 20:03:24  ilyas
 * Changed DslFrame functions definitions to macros,
 * Removed gDslVars from their parameter list
 *
 * Revision 1.5  2000/07/17 21:08:15  lkaplan
 * removed global pointer
 *
 * Revision 1.4  2000/06/09 18:32:56  liang
 * Fixed Irix compiler warnings.
 *
 * Revision 1.3  2000/05/03 03:57:04  ilyas
 * Added LOG file support for writing ATM data
 *
 * Revision 1.2  2000/04/19 00:31:47  ilyas
 * Added global SoftDsl functions for Vc, added OOB info functions
 *
 * Revision 1.1  2000/04/04 01:47:21  ilyas
 * Implemented abstract dslFrame and dslFrameBuffer objects
 *
 *
 ******************************************************************************/

#include "SoftDsl.h"
#include "Que.h"

/*
**
**		Frame buffer processing functions
**
*/

Public uint DslFrameNativeBufferGetLength(dslFrameBuffer *fb)
{
	return fb->length;
}

Public void * DslFrameNativeBufferGetAddress(dslFrameBuffer *fb)	
{
	/* return ((uchar *)&fb->length + sizeof(fb->length)); */
	return fb->pData;
}

Public void DslFrameNativeBufferSetLength(dslFrameBuffer *fb, uint l)
{
	fb->length = l;
}

Public void DslFrameNativeBufferSetAddress(dslFrameBuffer *fb, void *p)	
{
	fb->pData = p;
}

/*
**
**		Frame processing functions
**
*/

Public void DslFrameNativeInit(dslFrame *f)
{
	f->totalLength	= 0;
	f->bufCnt		= 0;
	f->head			= NULL;
	f->tail			= NULL;
}

Public uint DslFrameNativeGetLength (dslFrame *pFrame)
{
	return pFrame->totalLength;
}

Public uint DslFrameNativeGetBufCnt(dslFrame *pFrame)
{
	return pFrame->bufCnt;
}

Public dslFrameBuffer * DslFrameNativeGetFirstBuffer(dslFrame *pFrame)
{
	return pFrame->head;
}

Public dslFrameBuffer * DslFrameNativeGetNextBuffer(dslFrameBuffer *pFrBuffer)
{
	return pFrBuffer->next;
}

Public void DslFrameNativeSetNextBuffer(dslFrameBuffer *pFrBuf, dslFrameBuffer *pFrBufNext)
{
	pFrBuf->next = pFrBufNext;
}

Public dslFrameBuffer * DslFrameNativeGetLastBuffer(dslFrame *pFrame)
{
	return pFrame->tail;
}


Public void * DslFrameNativeGetLinkFieldAddress(dslFrame *f)
{
	return (void *) &f->Reserved;
}

Public dslFrame* DslFrameNativeGetFrameAddressFromLink(void *lnk)
{
	return (void *) ((uchar *)lnk - FLD_OFFSET(dslFrame, Reserved));
}

Public Boolean DslFrameNativeGetOobInfo (dslFrame *f, dslOobFrameInfo *pOobInfo)
{
	return false;
}

Public Boolean DslFrameNativeSetOobInfo (dslFrame *f, dslOobFrameInfo *pOobInfo)
{
	return true;
}

/*
**
**		Queueing frame functions
**
*/

Public void DslFrameNativeEnqueBufferAtBack(dslFrame *f, dslFrameBuffer *b)
{
	QueAdd(&f->head, b);
	f->totalLength += b->length;
	f->bufCnt++;
}

Public void DslFrameNativeEnqueFrameAtBack(dslFrame *fMain, dslFrame *f)
{
	QueMerge(&fMain->head,&f->head);
	fMain->totalLength += f->totalLength;
	fMain->bufCnt	 += f->bufCnt;
}

Public void DslFrameNativeEnqueBufferAtFront(dslFrame *f, dslFrameBuffer *b)
{
	QueAddFirst(&f->head,b);
	f->totalLength += b->length;
	f->bufCnt++;
}

Public void DslFrameNativeEnqueFrameAtFront(dslFrame *fMain, dslFrame *f)
{
	QueMerge(&f->head, &fMain->head);
	QueCopy(&fMain->head,&f->head);
	fMain->totalLength += f->totalLength;
	fMain->bufCnt	 += f->bufCnt;
}


Public dslFrameBuffer * DslFrameNativeDequeBuffer(dslFrame *pFrame)
{
	dslFrameBuffer	*pBuf;

	pBuf = (dslFrameBuffer *) QueFirst((QueHeader *)&pFrame->head);
	if (NULL != pBuf) {
		pFrame->totalLength -= DslFrameNativeBufferGetLength(pBuf);
		pFrame->bufCnt--;
		QueRemove((QueHeader *)&pFrame->head);
	}

	return pBuf;
}

/*
**
**		Frame allocation functions
**
*/

Public void * DslFrameNativeAllocMemForFrames(uint frameNum)
{
	return NULL;
}

Public void DslFrameNativeFreeMemForFrames(void *hMem)
{
}

Public dslFrame * DslFrameNativeAllocFrame(void *handle)
{
	return NULL;
}

Public void DslFrameNativeFreeFrame(void *handle, dslFrame *pFrame)
{
}

Public void * DslFrameNativeAllocMemForBuffers(void **ppMemPool, uint bufNum, uint memSize)
{
	return NULL;
}

Public void DslFrameNativeFreeMemForBuffers(void *hMem, uint memSize, void *pMemPool)
{
}

Public dslFrameBuffer * DslFrameNativeAllocBuffer(void *handle, void *pMem, uint length)
{
	return NULL;
}

Public void DslFrameNativeFreeBuffer(void *handle, dslFrameBuffer *pBuf)
{
}

Public uint DslFrameNative2Id (void *handle, dslFrame *pFrame)
{
	return 0;
}

Public void * DslFrameNativeId2Frame (void *handle, uint frameId)
{
	return NULL;
}
