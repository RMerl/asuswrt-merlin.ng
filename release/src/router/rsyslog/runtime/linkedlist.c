/* linkedlist.c
 * This file set implements a generic linked list object. It can be used
 * wherever a linke list is required.
 *
 * NOTE: we do not currently provide a constructor and destructor for the
 * object itself as we assume it will always be part of another strucuture.
 * Having a pointer to it, I think, does not really make sense but costs
 * performance. Consequently, there is is llInit() and llDestroy() and they
 * do what a constructor and destructur do, except for creating the
 * linkedList_t structure itself.
 *
 * File begun on 2007-07-31 by RGerhards
 *
 * Copyright (C) 2007-2012 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "rsyslog.h"
#include "linkedlist.h"


/* Initialize an existing linkedList_t structure
 * pKey destructor may be zero to take care of non-keyed lists.
 */
rsRetVal llInit(linkedList_t *pThis, rsRetVal (*pEltDestructor)(void*), rsRetVal (*pKeyDestructor)(void*),
int (*pCmpOp)(void*,void*))
{
	assert(pThis != NULL);
	assert(pEltDestructor != NULL);

	pThis->pEltDestruct = pEltDestructor;
	pThis->pKeyDestruct = pKeyDestructor;
	pThis->cmpOp = pCmpOp;
	pThis->pKey = NULL;
	pThis->iNumElts = 0;
	pThis->pRoot = NULL;
	pThis->pLast = NULL;

	return RS_RET_OK;
};
	

/* llDestroyEltData - destroys a list element
 * It is a separate function as the
 * functionality is needed in multiple code-pathes.
 */
static rsRetVal llDestroyElt(linkedList_t *pList, llElt_t *pElt)
{
	DEFiRet;

	assert(pList != NULL);
	assert(pElt != NULL);

	/* we ignore errors during destruction, as we need to try
	 * free the element in any case.
	 */
	if(pElt->pData != NULL)
		pList->pEltDestruct(pElt->pData);
	if(pElt->pKey != NULL)
		pList->pKeyDestruct(pElt->pKey);
	free(pElt);
	pList->iNumElts--; /* one less */

	RETiRet;
}


/* llDestroy - destroys a COMPLETE linkedList
 */
rsRetVal llDestroy(linkedList_t *pThis)
{
	DEFiRet;
	llElt_t *pElt;

	assert(pThis != NULL);

	pElt = pThis->pRoot;
	while(pElt != NULL) {
		/* keep the list structure in a consistent state as
		 * the destructor bellow may reference it again
		 */
		pThis->pRoot = pElt->pNext;
		if(pElt->pNext == NULL)
			pThis->pLast = NULL;

		/* we ignore errors during destruction, as we need to try
		 * finish the linked list in any case.
		 */
		llDestroyElt(pThis, pElt);
		pElt = pThis->pRoot;
	}

	RETiRet;
}

/* llDestroyRootElt - destroy the root element but otherwise
 * keeps this list intact.  -- rgerhards, 2007-08-03
 */
rsRetVal llDestroyRootElt(linkedList_t *pThis)
{
	DEFiRet;
	llElt_t *pPrev;
	
	if(pThis->pRoot == NULL) {
		ABORT_FINALIZE(RS_RET_EMPTY_LIST);
	}

	pPrev = pThis->pRoot;
	if(pPrev->pNext == NULL) {
		/* it was the only list element */
		pThis->pLast = NULL;
		pThis->pRoot = NULL;
	} else {
		/* there are other list elements */
		pThis->pRoot = pPrev->pNext;
	}

	CHKiRet(llDestroyElt(pThis, pPrev));

finalize_it:
	RETiRet;
}


/* get next user data element of a linked list. The caller must also
 * provide a "cookie" to the function. On initial call, it must be
 * NULL. Other than that, the caller is not allowed to to modify the
 * cookie. In the current implementation, the cookie is an actual
 * pointer to the current list element, but this is nothing that the
 * caller should rely on.
 */
rsRetVal llGetNextElt(linkedList_t *pThis, linkedListCookie_t *ppElt, void **ppUsr)
{
	llElt_t *pElt;
	DEFiRet;

	assert(pThis != NULL);
	assert(ppElt != NULL);
	assert(ppUsr != NULL);

	pElt = *ppElt;

	pElt = (pElt == NULL) ? pThis->pRoot : pElt->pNext;

	if(pElt == NULL) {
		iRet = RS_RET_END_OF_LINKEDLIST;
	} else {
		*ppUsr = pElt->pData;
	}

	*ppElt = pElt;

	RETiRet;
}


/* return the key of an Elt
 * rgerhards, 2007-09-11: note that ppDatea is actually a void**,
 * but I need to make it a void* to avoid lots of compiler warnings.
 * It will be converted later down in the code.
 */
rsRetVal llGetKey(llElt_t *pThis, void *ppData)
{
	assert(pThis != NULL);
	assert(ppData != NULL);

	*(void**) ppData = pThis->pKey;

	return RS_RET_OK;
}


/* construct a new llElt_t
 */
static rsRetVal llEltConstruct(llElt_t **ppThis, void *pKey, void *pData)
{
	DEFiRet;
	llElt_t *pThis;

	assert(ppThis != NULL);

	if((pThis = (llElt_t*) calloc(1, sizeof(llElt_t))) == NULL) {
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	pThis->pKey = pKey;
	pThis->pData = pData;

finalize_it:
	*ppThis = pThis;
	RETiRet;
}


/* append a user element to the end of the linked list. This includes setting a key. If no
 * key is desired, simply pass in a NULL pointer for it.
 */
rsRetVal llAppend(linkedList_t *pThis, void *pKey, void *pData)
{
	llElt_t *pElt;
	DEFiRet;
	
	CHKiRet(llEltConstruct(&pElt, pKey, pData));

	pThis->iNumElts++; /* one more */
	if(pThis->pLast == NULL) {
		pThis->pRoot = pElt;
	} else {
		pThis->pLast->pNext = pElt;
	}
	pThis->pLast = pElt;

finalize_it:
	RETiRet;
}


/* unlink a requested element. As we have singly-linked lists, the
 * caller also needs to pass in the previous element (or NULL, if it is the
 * root element).
 * rgerhards, 2007-11-21
 */
static rsRetVal llUnlinkElt(linkedList_t *pThis, llElt_t *pElt, llElt_t *pEltPrev)
{
	assert(pElt != NULL);

	if(pEltPrev == NULL) { /* root element? */
		pThis->pRoot = pElt->pNext;
	} else { /* regular element */
		pEltPrev->pNext = pElt->pNext;
	}

	if(pElt == pThis->pLast)
		pThis->pLast = pEltPrev;

	return RS_RET_OK;
}


/* unlinks and immediately deletes an element. Previous element must
 * be given (or zero if the root element is to be deleted).
 * rgerhards, 2007-11-21
 */
static rsRetVal llUnlinkAndDelteElt(linkedList_t *pThis, llElt_t *pElt, llElt_t *pEltPrev)
{
	DEFiRet;

	assert(pElt != NULL);

	CHKiRet(llUnlinkElt(pThis, pElt, pEltPrev));
	CHKiRet(llDestroyElt(pThis, pElt));

finalize_it:
	RETiRet;
}

/* find a user element based on the provided key - this is the
 * internal variant, which also tracks the last element pointer
 * before the found element. This is necessary to delete elements.
 * NULL means there is no element in front of it, aka the found elt
 * is the root elt.
 * rgerhards, 2007-11-21
 */
static rsRetVal llFindElt(linkedList_t *pThis, void *pKey, llElt_t **ppElt, llElt_t **ppEltPrev)
{
	DEFiRet;
	llElt_t *pElt;
	llElt_t *pEltPrev = NULL;
	int bFound = 0;

	assert(pThis != NULL);
	assert(pKey != NULL);
	assert(ppElt != NULL);
	assert(ppEltPrev != NULL);

	pElt = pThis->pRoot;
	while(pElt != NULL && bFound == 0) {
		if(pThis->cmpOp(pKey, pElt->pKey) == 0)
			bFound = 1;
		else {
			pEltPrev = pElt;
			pElt = pElt->pNext;
		}
	}

	if(bFound == 1) {
		*ppElt = pElt;
		*ppEltPrev = pEltPrev;
	} else
		iRet = RS_RET_NOT_FOUND;

	RETiRet;
}


/* find a user element based on the provided key
 */
rsRetVal llFind(linkedList_t *pThis, void *pKey, void **ppData)
{
	DEFiRet;
	llElt_t *pElt;
	llElt_t *pEltPrev;

	CHKiRet(llFindElt(pThis, pKey, &pElt, &pEltPrev));

	/* if we reach this point, we have found the element */
	*ppData = pElt->pData;

finalize_it:
	RETiRet;
}


/* find a delete an element based on user-provided key. The element is
 * delete, the caller does not receive anything. If we need to receive
 * the element before destruction, we may implement an llFindAndUnlink()
 * at that time.
 * rgerhards, 2007-11-21
 */
rsRetVal llFindAndDelete(linkedList_t *pThis, void *pKey)
{
	DEFiRet;
	llElt_t *pElt;
	llElt_t *pEltPrev;

	CHKiRet(llFindElt(pThis, pKey, &pElt, &pEltPrev));

	/* if we reach this point, we have found an element */
	CHKiRet(llUnlinkAndDelteElt(pThis, pElt, pEltPrev));

finalize_it:
	RETiRet;
}


/* provide the count of linked list elements
 */
rsRetVal llGetNumElts(linkedList_t *pThis, int *piCnt)
{
	DEFiRet;

	assert(pThis != NULL);
	assert(piCnt != NULL);

	*piCnt = pThis->iNumElts;

	RETiRet;
}


/* execute a function on all list members. The functions receives a
 * user-supplied parameter, which may be either a simple value
 * or a pointer to a structure with more data. If the user-supplied
 * function does not return RS_RET_OK, this function here terminates.
 * rgerhards, 2007-08-02
 * rgerhards, 2007-11-21: added functionality to delete a list element.
 * If the called user function returns RS_RET_OK_DELETE_LISTENTRY the current element
 * is deleted.
 */
rsRetVal llExecFunc(linkedList_t *pThis, rsRetVal (*pFunc)(void*, void*), void* pParam)
{
	DEFiRet;
	rsRetVal iRetLL;
	void *pData;
	linkedListCookie_t llCookie = NULL;
	linkedListCookie_t llCookiePrev = NULL; /* previous list element (needed for deletion, NULL = at root) */

	assert(pThis != NULL);
	assert(pFunc != NULL);

	while((iRetLL = llGetNextElt(pThis, &llCookie, (void**)&pData)) == RS_RET_OK) {
		iRet = pFunc(pData, pParam);
		if(iRet == RS_RET_OK_DELETE_LISTENTRY) {
			/* delete element */
			CHKiRet(llUnlinkAndDelteElt(pThis, llCookie, llCookiePrev));
			/* we need to revert back, as we have just deleted the current element.
			 * So the actual current element is the one before it, which happens to be
			 * stored in llCookiePrev. -- rgerhards, 2007-11-21
			 */
			llCookie = llCookiePrev;
		} else if (iRet != RS_RET_OK) {
			FINALIZE;
		}
		llCookiePrev = llCookie;
	}

	if(iRetLL != RS_RET_END_OF_LINKEDLIST)
		iRet = iRetLL;

finalize_it:
	RETiRet;
}

/* vim:set ai:
 */
