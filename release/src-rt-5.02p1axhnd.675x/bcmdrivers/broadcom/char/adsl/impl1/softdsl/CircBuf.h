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
 * CircBuf -- Generic Circular Buffer
 *
 * Description:
 *	Implementation of generic circular buffer algorithms
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.14 $
 *
 * $Id: CircBuf.h,v 1.14 2004/06/24 03:10:37 ilyas Exp $
 *
 * $Log: CircBuf.h,v $
 * Revision 1.14  2004/06/24 03:10:37  ilyas
 * Added extra macro to be able to use un-cached variable (for status write)
 *
 * Revision 1.13  2004/02/09 23:47:02  ilyas
 * Fixed last change
 *
 * Revision 1.12  2004/02/06 22:52:58  ilyas
 * Improved stretch buffer write
 *
 * Revision 1.11  2002/12/30 23:27:55  ilyas
 * Added macro for HostDma optimizations
 *
 * Revision 1.10  2002/10/26 02:15:02  ilyas
 * Optimized and added new macros for HostDma
 *
 * Revision 1.9  2002/01/22 23:59:29  ilyas
 * Added paraenthesis around macro argument
 *
 * Revision 1.8  2002/01/15 22:28:38  ilyas
 * Extended macro to support readPtr from uncached address
 *
 * Revision 1.7  2001/09/21 19:47:05  ilyas
 * Fixed compiler warnings for VxWorks build
 *
 * Revision 1.6  2001/06/07 18:47:56  ilyas
 * Added more macros for circular buffer arithmetics
 *
 * Revision 1.5  2001/04/18 03:58:34  ilyas
 * Added LOG file write granularity
 *
 * Revision 1.4  2001/01/19 04:34:12  ilyas
 * Added more macros to circular buffer implementation
 *
 * Revision 1.3  2001/01/06 04:01:41  ilyas
 * Changed the way we write status messages
 *
 * Revision 1.2  2001/01/04 05:52:21  ilyas
 * Added implementation of stretchable circular buffer used in LOG and Status
 * handlers
 *
 * Revision 1.1  2000/05/03 03:45:55  ilyas
 * Original implementation
 *
 *
 *****************************************************************************/

#ifndef	CircBufHeader_H_
#define	CircBufHeader_H_

#include "EndianUtil.h"

typedef struct {
	char	*pStart;
	char	*pEnd;
	char	*pRead;
	char	*pWrite;
} circBufferStruct;

/* Initialize circular buffer */

#define	CircBufferInit(pCB,buf,size)	do {			\
	(pCB)->pStart = (char *) (buf);						\
	(pCB)->pRead = (pCB)->pWrite = (pCB)->pStart;		\
	(pCB)->pEnd = (pCB)->pStart + size;					\
} while (0)

#define	CircBufferGetSize(pCB)			((pCB)->pEnd - (pCB)->pStart)
#define	CircBufferGetStartPtr(pCB)		((void *) (pCB)->pStart)
#define	CircBufferGetEndPtr(pCB)		((void *) (pCB)->pEnd)

#define	CircBufferReset(pCB)			(pCB)->pRead = (pCB)->pWrite = (pCB)->pStart


#define	CircBufferGetReadPtr(pCB)		((void *) (pCB)->pRead)
#define	CircBufferGetWritePtr(pCB)		((void *) (pCB)->pWrite)

#define	CircBufferSetRdPtrToWrPtr(pCB)		(pCB)->pRead = (pCB)->pWrite

#ifndef bcm47xx
#define	CircBufferDistance(pCB,p1,p2,d)	((char*)(p2) - (char*)(p1) - d >= 0 ?			\
											(char*)(p2) - (char*)(p1) - d :			\
											((char*)(p2)- (char*)(p1) - d + ((pCB)->pEnd - (pCB)->pStart)))

#define	CircBufferAddContig(pCB,p,n)	((char*)(p) + (n) == (pCB)->pEnd ? (pCB)->pStart : (char*)(p) + (n))
#else
static __inline int CircBufferDistance(circBufferStruct *pCB, char *p1, char *p2, int d)
{
	int tmp = p2 - p1 - d;

	return (tmp >= 0 ? tmp : tmp + (pCB->pEnd - pCB->pStart));
}

static __inline char * CircBufferAddContig(circBufferStruct *pCB, char *p, int n)
{
	p += n;
	return (p == pCB->pEnd ? pCB->pStart : p);
}
#endif

#define	CircBufferAdd(pCB,p,n)			((char*)(p) + (n) >= (pCB)->pEnd ?						\
											(pCB)->pStart + ((char*)(p) + (n) - (pCB)->pEnd) :	\
											(char*)(p) + (n))

#define	CircBufferReadUpdate(pCB,n)		(pCB)->pRead = CircBufferAdd(pCB,(pCB)->pRead,n)
#define	CircBufferWriteUpdate(pCB,n)	(pCB)->pWrite= CircBufferAdd(pCB,(pCB)->pWrite,n)

#define	CircBufferReadUpdateContig(pCB,n)	(pCB)->pRead = CircBufferAddContig(pCB,(pCB)->pRead,n)
#define	CircBufferWriteUpdateContig(pCB,n)	(pCB)->pWrite= CircBufferAddContig(pCB,(pCB)->pWrite,n)

#define	CircBufferGetReadAvail(pCB)		CircBufferDistance(pCB,(pCB)->pRead,(pCB)->pWrite,0)
#define	CircBufferIsReadEmpty(pCB)		((pCB)->pRead == (pCB)->pWrite)
#define	CircBufferGetWriteAvail(pCB)	CircBufferDistance(pCB,(pCB)->pWrite,(pCB)->pRead,1)
#define	CircBufferGetWriteAvailN(pCB,n)	CircBufferDistance(pCB,(pCB)->pWrite,(pCB)->pRead,n)

#define	CircBufferGetReadContig(pCB)	((uintptr_t)(pCB)->pWrite >= (uintptr_t) (pCB)->pRead ?	\
											(pCB)->pWrite - (pCB)->pRead :		\
											(pCB)->pEnd	  - (pCB)->pRead)

#define	CircBufferGetWriteContig(pCB)	((pCB)->pEnd - (pCB)->pWrite > CircBufferGetWriteAvail(pCB) ?	\
											CircBufferGetWriteAvail(pCB) :		\
											(pCB)->pEnd - (pCB)->pWrite)

/*
**
**		structure and macros for "strectch" buffer
**
*/

typedef struct {
	int	pStart;
	int	pEnd;
	int	pExtraEnd;
	int	pStretchEnd;
	int	pRead;
	int	pWrite;
} stretchBufferStruct;

#define	StretchBufferInit(pSB,buf,size,extra)	do {	\
	(pSB)->pStart = (int) (buf);						\
	(pSB)->pRead = (pSB)->pWrite = (pSB)->pStart;		\
	(pSB)->pEnd = (pSB)->pStart + (size);				\
	(pSB)->pStretchEnd = (pSB)->pEnd;					\
	(pSB)->pExtraEnd = (pSB)->pEnd+(extra);				\
} while (0)

typedef struct {
	char	*pStart;
	char	*pEnd;
	char	*pExtraEnd;
	char	*pStretchEnd;
	char	*pRead;
	char	*pWrite;
} stretchHostBufferStruct;

#define	StretchHostBufferInit(pSB,buf,size,extra)	do {	\
	(pSB)->pStart = (char *) (buf);						\
	(pSB)->pRead = (pSB)->pWrite = (pSB)->pStart;		\
	(pSB)->pEnd = (pSB)->pStart + (size);				\
	(pSB)->pStretchEnd = (pSB)->pEnd;					\
	(pSB)->pExtraEnd = (pSB)->pEnd+(extra);				\
} while (0)

#ifdef  ADSLDRV_LITTLE_ENDIAN
#define	SB_CONV_LONG(x)		(ADSL_ENDIAN_CONV_INT32((uint)x))
#else
#define	SB_CONV_LONG(x)		x
#endif

#define	StretchBufferGetSize(pSB)		(SB_CONV_LONG((pSB)->pEnd) - SB_CONV_LONG((pSB)->pStart))
#define	StretchBufferGetStartPtr(pSB)	((uintptr_t)(uint) SB_CONV_LONG((pSB)->pStart))
#define	StretchBufferGetReadPtr(pSB)	((uintptr_t)(uint) SB_CONV_LONG((pSB)->pRead))
#define	StretchBufferGetWritePtr(pSB)	((uintptr_t)(uint) SB_CONV_LONG((pSB)->pWrite))
#define	StretchBufferReset(pSB)			((pSB)->pRead = (pSB)->pWrite = (pSB)->pStart)

#define	StretchBufferGetReadToEnd(pSB)	(SB_CONV_LONG((pSB)->pStretchEnd) - SB_CONV_LONG((pSB)->pRead))

static __inline int _StretchBufferGetReadAvailTotal(stretchBufferStruct *fBuf, int rdPtr)
{
	int wrPtr = SB_CONV_LONG(fBuf->pWrite);
	return( ((wrPtr - rdPtr) >= 0) ? wrPtr - rdPtr: ((SB_CONV_LONG(fBuf->pEnd) - rdPtr) + (wrPtr -SB_CONV_LONG(fBuf->pStart))) );
}
#define	StretchBufferGetReadAvailTotal(pSB)		_StretchBufferGetReadAvailTotal(pSB, SB_CONV_LONG((pSB)->pRead))

static __inline int StretchBufferGetReadAvail(stretchBufferStruct *fBuf)
{
	int wrPtr = SB_CONV_LONG(fBuf->pWrite);
	int rdPtr = SB_CONV_LONG(fBuf->pRead);
	return( ((wrPtr - rdPtr) >= 0) ? wrPtr - rdPtr: SB_CONV_LONG(fBuf->pStretchEnd) - rdPtr );
}
static __inline int _StretchBufferGetWriteAvail(stretchBufferStruct *fBuf, int rdPtr)
{
	int wrPtr = SB_CONV_LONG(fBuf->pWrite);
	return( ((rdPtr - wrPtr) > 0) ? rdPtr - wrPtr - 1: SB_CONV_LONG(fBuf->pExtraEnd) - wrPtr );
}
#define	StretchBufferGetWriteAvail(pSB)		_StretchBufferGetWriteAvail(pSB, SB_CONV_LONG((pSB)->pRead))

#define	StretchBufferReadUpdate(pSB,n)		do {								\
	int p;																	\
																				\
	p = SB_CONV_LONG((pSB)->pRead) + (n);										\
	(pSB)->pRead = (p >= SB_CONV_LONG((pSB)->pEnd) ? (pSB)->pStart : SB_CONV_LONG(p));	\
} while (0)

#define	_StretchBufferWriteUpdate(pSB,rd,n)	do {								\
	int p;																	\
																				\
	p = SB_CONV_LONG((pSB)->pWrite) + (n);										\
	if (p >= SB_CONV_LONG((pSB)->pEnd)) {										\
		if ((rd) != (pSB)->pStart) {											\
			(pSB)->pStretchEnd = SB_CONV_LONG(p);								\
			(pSB)->pWrite = (pSB)->pStart;										\
		}																		\
	}																			\
	else																		\
		(pSB)->pWrite = SB_CONV_LONG(p);										\
} while (0)

#define	StretchBufferWriteUpdate(pSB,n)		_StretchBufferWriteUpdate(pSB, (pSB)->pRead, n)

#ifdef SUPPORT_STATUS_BACKUP
#define	HostStretchBufferGetSize(pSB)		((pSB)->pEnd - (pSB)->pStart)
#define	HostStretchBufferGetStartPtr(pSB)	((void *) (pSB)->pStart)
#define	HostStretchBufferGetReadPtr(pSB)	((void *) (pSB)->pRead)
#define	HostStretchBufferGetWritePtr(pSB)	((void *) (pSB)->pWrite)
#define	HostStretchBufferGetReadToEnd(pSB)	((pSB)->pStretchEnd - (pSB)->pRead)

static __inline int _HostStretchBufferGetReadAvailTotal(stretchHostBufferStruct *fBuf, char *rdPtr)
{
	char *wrPtr = fBuf->pWrite;
	return( ((wrPtr - rdPtr) >= 0) ? wrPtr - rdPtr: ((fBuf->pEnd - rdPtr) + (wrPtr -fBuf->pStart)));
}
#define	HostStretchBufferGetReadAvailTotal(pSB)		_HostStretchBufferGetReadAvailTotal(pSB, (pSB)->pRead)

static __inline int HostStretchBufferGetReadAvail(stretchHostBufferStruct *fBuf)
{
	char *wrPtr = fBuf->pWrite;
	char *rdPtr = fBuf->pRead;
	return( ((wrPtr - rdPtr) >= 0) ? wrPtr - rdPtr: fBuf->pStretchEnd - rdPtr );
}
static __inline int _HostStretchBufferGetWriteAvail(stretchHostBufferStruct *fBuf, char *rdPtr)
{
	char *wrPtr = fBuf->pWrite;
	return( ((rdPtr - wrPtr) > 0) ?  rdPtr - wrPtr - 1: fBuf->pExtraEnd - wrPtr );
}
#define	HostStretchBufferGetWriteAvail(pSB)		_HostStretchBufferGetWriteAvail(pSB, (pSB)->pRead)

#define	HostStretchBufferReadUpdate(pSB,n)		do {							\
	char	*p;																	\
																				\
	p = (pSB)->pRead + (n);														\
	(pSB)->pRead = (p >= (pSB)->pEnd ? (pSB)->pStart : p);						\
} while (0)

#define	_HostStretchBufferWriteUpdate(pSB,rd,n)	do {							\
	char	*p;																	\
																				\
	p = (pSB)->pWrite + (n);													\
	if (p >= (pSB)->pEnd) {														\
		if ((rd) != (pSB)->pStart) {											\
			(pSB)->pStretchEnd = p;												\
			(pSB)->pWrite = (pSB)->pStart;										\
		}																		\
	}																			\
	else																		\
		(pSB)->pWrite = p;														\
} while (0)

#define	HostStretchBufferWriteUpdate(pSB,n)		_HostStretchBufferWriteUpdate(pSB,(pSB)->pRead,n)
#endif	/* SUPPORT_STATUS_BACKUP */

#endif	/* CircBufHeader_H_ */



