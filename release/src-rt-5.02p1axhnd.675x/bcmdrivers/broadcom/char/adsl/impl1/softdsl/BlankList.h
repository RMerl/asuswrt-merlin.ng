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
 * BlankList.h 
 *
 * Description:
 *	Definition and implementation (via macros and inline functions)
 *  of blank list - list of unused items of any size (not less than 
 *	sizeof(void *)
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 *****************************************************************************/

#ifndef	BlankListHeader
#define	BlankListHeader

#define BlankListPeek(head)		((void *) (head))
#define BlankListNext(p)		(*(void **) (p))

#define BlankListAdd(pHead,p)	do {			\
  BlankListNext(p)	   = BlankListNext(pHead);	\
  BlankListNext(pHead) = (void *) (p);			\
} while (0)

#define BlankListAddList(pHead,pFirst,pLast) do {	\
  if (NULL != (pLast)) {							\
	BlankListNext(pLast) = BlankListNext(pHead);	\
	BlankListNext(pHead) = (void *) (pFirst);		\
  }													\
} while (0)

#define BlankListGet(pHead)							\
  BlankListNext(pHead);								\
  {													\
	void	**__p;									\
	__p = (void	**) BlankListNext(pHead);			\
	if (NULL != __p)								\
	  BlankListNext(pHead) = *__p;					\
  }


#define	BlankListForEach(pHead,f,ref)	do {		\
  void	*p = BlankListNext(pHead);					\
													\
  while (NULL != p) {								\
	if ( (f)((p), ref) ) break;						\
	p = BlankListNext(p);							\
  }													\
} while (0)


#include	"Que.h"

#define BlankListAddQue(pHead,pqHdr)	do {							\
  if (NULL != ((QueHeader *)(pqHdr))->tail) {							\
	BlankListNext(((QueHeader *)(pqHdr))->tail) = BlankListNext(pHead);	\
	BlankListNext(pHead) = ((QueHeader *)(pqHdr))->head;				\
  }																		\
} while (0)

#include	"DList.h"

#define BlankListAddDList(pHead,pDListHead)	do {						\
  if (!DListEmpty(pDListHead)) {										\
	BlankListNext(DListLast(pDListHead)) = BlankListNext(pHead);		\
	BlankListNext(pHead) = DListFirst(pDListHead);						\
  }																		\
} while (0)

#endif	/* BlankListHeader */

