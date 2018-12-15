/* Definition of the linkedlist object.
 *
 * Copyright 2007-2012 Rainer Gerhards and Adiscon GmbH.
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

#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

/* this is a single entry for a parse routine. It describes exactly
 * one entry point/handler.
 * The short name is cslch (Configfile SysLine CommandHandler)
 */
struct llElt_s { /* config file sysline parse entry */
	struct llElt_s *pNext;
	void *pKey;		/* key for this element */
	void *pData;		/* user-supplied data pointer */
};
typedef struct llElt_s llElt_t;


/* this is the list of known configuration commands with pointers to
 * their handlers.
 * The short name is cslc (Configfile SysLine Command)
 */
struct linkedList_s { /* config file sysline parse entry */
	int iNumElts;		/* number of elements in list */
	rsRetVal (*pEltDestruct)(void*pData);	/* destructor for user pointer in llElt_t's */
	rsRetVal (*pKeyDestruct)(void*pKey);	/* destructor for key pointer in llElt_t's */
	int (*cmpOp)(void*, void*); /* pointer to key compare operation function, retval like strcmp */
	void *pKey;			/* the list key (searchable, if set) */
	llElt_t *pRoot;	/* list root */
	llElt_t *pLast;	/* list tail */
};
typedef struct linkedList_s linkedList_t;

typedef llElt_t* linkedListCookie_t;	/* this type avoids exposing internals and keeps us flexible */

/* prototypes */
rsRetVal llInit(linkedList_t *pThis, rsRetVal (*pEltDestructor)(), rsRetVal (*pKeyDestructor)(), int (*pCmpOp)());
rsRetVal llDestroy(linkedList_t *pThis);
rsRetVal llDestroyRootElt(linkedList_t *pThis);
rsRetVal llGetNextElt(linkedList_t *pThis, linkedListCookie_t *ppElt, void **ppUsr);
rsRetVal llAppend(linkedList_t *pThis, void *pKey, void *pData);
rsRetVal llFind(linkedList_t *pThis, void *pKey, void **ppData);
rsRetVal llGetKey(llElt_t *pThis, void *ppData);
rsRetVal llGetNumElts(linkedList_t *pThis, int *piCnt);
rsRetVal llExecFunc(linkedList_t *pThis, rsRetVal (*pFunc)(void*, void*), void* pParam);
rsRetVal llFindAndDelete(linkedList_t *pThis, void *pKey);
/* use the macro below to define a function that will be executed by
 * llExecFunc()
 */
#define DEFFUNC_llExecFunc(funcName)\
	static rsRetVal funcName(void __attribute__((unused)) *pData, void __attribute__((unused)) *pParam)

#endif /* #ifndef LINKEDLIST_H_INCLUDED */
