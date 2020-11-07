/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/****************************************************************************
 *
 * Que.h
 *
 * Description:
 *	Definition and implementation (via macros and inline functions)
 *  of a simple queue
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 *****************************************************************************/

#ifndef	QueHeader_H_
#define	QueHeader_H_

typedef void *		_QueItem;

typedef struct _QueHeader
	{
	_QueItem	*head;		/* first item in the queue */
	_QueItem	*tail;		/* last item in the queue */
	} QueHeader;

/* Queue management macros */

#define QueInit(pqHdr)		(((QueHeader *)(pqHdr))->head = ((QueHeader *)(pqHdr))->tail = NULL)
#define QueEmpty(pqHdr)     (NULL == ((QueHeader *)(pqHdr))->head)

#define QueFirst(pqHdr)		((QueHeader *)(pqHdr))->head
#define QueLast(pqHdr)		((QueHeader *)(pqHdr))->tail 
#define QueNext(pqItem)		(*((void **)(pqItem)))


#define QueRemoveFirst(pqHdr)	do {										\
    if (!QueEmpty(pqHdr)) {													\
      ((QueHeader *)(pqHdr))->head = *((QueHeader *)(pqHdr))->head;			\
      if (QueEmpty(pqHdr))													\
        ((QueHeader *)(pqHdr))->tail = NULL;								\
    }																		\
} while (0)
#define QueRemove(pqHdr)		QueRemoveFirst(pqHdr)


#define QueAddLast(pqHdr,pqItem) do {						\
    QueNext(pqItem) = NULL;									\
    if (NULL != ((QueHeader *)(pqHdr))->tail)				\
      *((QueHeader *)(pqHdr))->tail = (pqItem);				\
    else													\
      ((QueHeader *)(pqHdr))->head = (_QueItem *)(pqItem);  \
    ((QueHeader *)(pqHdr))->tail = (_QueItem *)(pqItem);    \
} while (0)
#define QueAdd(pqHdr,pItem)    QueAddLast(pqHdr,pItem)

#define QueAddFirst(pqHdr,pqItem)	do {					\
    if (NULL == ((QueHeader *)(pqHdr))->tail)				\
	  ((QueHeader *)(pqHdr))->tail = (_QueItem *)(pqItem);  \
    QueNext(pqItem) = ((QueHeader *)(pqHdr))->head;			\
    ((QueHeader *)(pqHdr))->head = (_QueItem *)(pqItem);	\
} while (0)


#define QueGet(pqHdr)			\
  (void *) QueFirst(pqHdr);		\
  QueRemove(pqHdr);

#define QueMerge(pqHdr1,pqHdr2)	do {											\
  if (NULL == ((QueHeader *)(pqHdr1))->tail)									\
	((QueHeader *)(pqHdr1))->head = ((QueHeader *)(pqHdr2))->head;				\
  else																			\
    QueNext(((QueHeader *)(pqHdr1))->tail) = ((QueHeader *)(pqHdr2))->head;		\
  if (NULL != ((QueHeader *)(pqHdr2))->tail)                 					\
	((QueHeader *)(pqHdr1))->tail = ((QueHeader *)(pqHdr2))->tail;				\
} while (0)

#define QueCopy(pqHdr1,pqHdr2)	do {											\
	((QueHeader *)(pqHdr1))->head = ((QueHeader *)(pqHdr2))->head;				\
	((QueHeader *)(pqHdr1))->tail = ((QueHeader *)(pqHdr2))->tail;				\
} while (0)

#define	QueForEach(pqHdr,f,ref)		do {			\
	_QueItem	*p = ((QueHeader *)(pqHdr))->head;	\
													\
	while (NULL != p) {								\
	  if ( (f)((void *)p, ref) ) break;				\
	  p = QueNext(p);								\
	}												\
} while (0)

#endif	/* QueHeader_H_ */

