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
 * DList.h
 *
 * Description:
 *	Definition and implementation (via macros and inline functions)
 *  of double-linked list
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 *****************************************************************************/

#ifndef	DListHeader_H_
#define	DListHeader_H_

typedef struct _DListHeader
	{
	struct _DListHeader	*next;		/* next item in the list */
	struct _DListHeader	*prev;		/* prev item in the list */
	} DListHeader;

typedef struct _DListUpHeader
	{
	struct _DListUpHeader	*next;	/* next item in the list */
	struct _DListUpHeader	*prev;	/* prev item in the list */
	struct _DListUpHeader	*head;	/* head of the list */
	} DListUpHeader;

/* Double linked list DList management macros */

#define	DListInit(pDListHead)	do {					\
	((DListHeader *)(pDListHead))->next = pDListHead;	\
	((DListHeader *)(pDListHead))->prev = pDListHead;	\
} while (0)

#define	DListNext(pDListEntry)		(((DListHeader *)(pDListEntry))->next)
#define	DListPrev(pDListEntry)		(((DListHeader *)(pDListEntry))->prev)

#define	DListEntryLinked(pDListEntry)	(NULL != DListNext(pDListEntry))
#define	DListUnlinkEntry(pDListEntry)	(DListNext(pDListEntry) = DListPrev(pDListEntry) = NULL)

#define	DListFirst(pDListHead)		DListNext(pDListHead)
#define	DListLast(pDListHead)		DListPrev(pDListHead)
#define	DListValid(pDListHead,pEntry)	((void *)(pDListHead) != (pEntry))
#define	DListEmpty(pDListHead)		((void *)pDListHead == ((DListHeader *)pDListHead)->next)

#define	DListInsertAfter(pDListEntry,pEntry)	 do {						\
	((DListHeader *)(pEntry))->next = ((DListHeader *)(pDListEntry))->next;	\
	((DListHeader *)(pEntry))->prev = (DListHeader *)(pDListEntry);			\
	((DListHeader *)(pDListEntry))->next->prev = (DListHeader *) (pEntry);	\
	((DListHeader *)(pDListEntry))->next = (DListHeader *) (pEntry);		\
} while (0)

#define	DListInsertBefore(pDListEntry,pEntry) do {							\
	((DListHeader *)(pEntry))->next = (DListHeader *)(pDListEntry);			\
	((DListHeader *)(pEntry))->prev = ((DListHeader *)(pDListEntry))->prev;	\
	((DListHeader *)(pDListEntry))->prev->next = (DListHeader *) (pEntry);	\
	((DListHeader *)(pDListEntry))->prev = (DListHeader *) (pEntry);		\
} while (0)

#define	DListInsertTail(pDListHead,pEntry)	DListInsertBefore(pDListHead,pEntry)
#define	DListInsertHead(pDListHead,pEntry)	DListInsertAfter(pDListHead,pEntry)

#define	DListRemove(pDListEntry)	do {				\
	((DListHeader *)(pDListEntry))->prev->next = ((DListHeader *)(pDListEntry))->next;	\
	((DListHeader *)(pDListEntry))->next->prev = ((DListHeader *)(pDListEntry))->prev;	\
} while (0)


#define	DListForEach(pDListHead,f,ref)		do {			\
	DListHeader	*p = ((DListHeader *)(pDListHead))->next;	\
															\
	while (DListValid(pDListHead,p)) {						\
	  DListHeader *p0 = p;									\
	  p = DListNext(p);										\
	  if ( (f)((void *)p0, ref) ) break;					\
	}														\
} while (0)


/* Double linked list with up link DListUp management macros */

#define	DListUpInit(pDListHead)	do {										\
	((DListUpHeader *)(pDListHead))->next = (DListUpHeader *) (pDListHead);	\
	((DListUpHeader *)(pDListHead))->prev = (DListUpHeader *) (pDListHead);	\
	((DListUpHeader *)(pDListHead))->head = (DListUpHeader *) (pDListHead);	\
} while (0)

#define	DListUpNext(pDListEntry)	((DListUpHeader *) DListNext(pDListEntry))
#define	DListUpPrev(pDListEntry)	((DListUpHeader *) DListPrev(pDListEntry))
#define	DListUpHead(pDListEntry)	(((DListUpHeader *)(pDListEntry))->head)

#define	DListUpFirst(pDListHead)	DListUpNext(pDListHead)
#define	DListUpLast(pDListHead)		DListUpPrev(pDListHead)		
#define	DListUpValid(pEntry)		(((DListUpHeader *)(pEntry))->head != (void *) pEntry)
#define	DListUpEmpty(pDListHead)	DListEmpty(pDListHead)		

#define	DListUpInsertAfter(pDListEntry,pEntry)	 do {							\
	DListInsertAfter(pDListEntry,pEntry);										\
	((DListUpHeader *)(pEntry))->head = ((DListUpHeader *)(pDListEntry))->head;	\
} while (0)

#define	DListUpInsertBefore(pDListEntry,pEntry)  do {							\
	DListInsertBefore(pDListEntry,pEntry);										\
	((DListUpHeader *)(pEntry))->head = ((DListUpHeader *)(pDListEntry))->head;	\
} while (0)

#define	DListUpInsertTail(pDListHead,pEntry)	DListUpInsertBefore(pDListHead,pEntry)
#define	DListUpInsertHead(pDListHead,pEntry)	DListUpInsertAfter(pDListHead,pEntry)

#define	DListUpRemove(pDListEntry)		DListRemove(pDListEntry)
#define	DListUpForEach(pDListHead,f,ref) DListForEach((DListHeader *)(pDListHead),f,ref)

#endif	/* DListHeader_H_ */

