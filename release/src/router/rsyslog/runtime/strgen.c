/* strgen.c
 * Module to handle string generators. These are C modules that receive
 * the message object and return a custom-built string. The primary purpose
 * for their existance is performance -- they do the same as template strings, but
 * potentially faster (if well implmented).
 *
 * Module begun 2010-06-01 by Rainer Gerhards
 *
 * Copyright 2010 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rsyslog.h"
#include "msg.h"
#include "obj.h"
#include "errmsg.h"
#include "strgen.h"
#include "ruleset.h"
#include "unicode-helper.h"
#include "cfsysline.h"

/* definitions for objects we access */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(ruleset)

/* static data */

/* config data */

/* This is the list of all strgens known to us.
 * This is also used to unload all modules on shutdown.
 */
strgenList_t *pStrgenLstRoot = NULL;


/* intialize (but NOT allocate) a strgen list. Primarily meant as a hook
 * which can be used to extend the list in the future. So far, just sets
 * it to NULL.
 */
static rsRetVal
InitStrgenList(strgenList_t **pListRoot)
{
	*pListRoot = NULL;
	return RS_RET_OK;
}


/* destruct a strgen list. The list elements are destroyed, but the strgen objects
 * themselves are not modified. (That is done at a late stage during rsyslogd
 * shutdown and need not be considered here.)
 */
static rsRetVal
DestructStrgenList(strgenList_t **ppListRoot)
{
	strgenList_t *pStrgenLst;
	strgenList_t *pStrgenLstDel;

	pStrgenLst = *ppListRoot;
	while(pStrgenLst != NULL) {
		pStrgenLstDel = pStrgenLst;
		pStrgenLst = pStrgenLst->pNext;
		free(pStrgenLstDel);
	}
	*ppListRoot = NULL;
	return RS_RET_OK;
}


/* Add a strgen to the list. We use a VERY simple and ineffcient algorithm,
 * but it is employed only for a few milliseconds during config processing. So
 * I prefer to keep it very simple and with simple data structures. Unfortunately,
 * we need to preserve the order, but I don't like to add a tail pointer as that
 * would require a container object. So I do the extra work to skip to the tail
 * when adding elements...
 */
static rsRetVal
AddStrgenToList(strgenList_t **ppListRoot, strgen_t *pStrgen)
{
	strgenList_t *pThis;
	strgenList_t *pTail;
	DEFiRet;

	CHKmalloc(pThis = MALLOC(sizeof(strgenList_t)));
	pThis->pStrgen = pStrgen;
	pThis->pNext = NULL;

	if(*ppListRoot == NULL) {
		pThis->pNext = *ppListRoot;
		*ppListRoot = pThis;
	} else {
		/* find tail first */
		for(pTail = *ppListRoot ; pTail->pNext != NULL ; pTail = pTail->pNext)
			/* just search, do nothing else */;
		/* add at tail */
		pTail->pNext = pThis;
	}

finalize_it:
	RETiRet;
}


/* find a strgen based on the provided name */
static rsRetVal
FindStrgen(strgen_t **ppStrgen, uchar *pName)
{
	strgenList_t *pThis;
	DEFiRet;
	
	for(pThis = pStrgenLstRoot ; pThis != NULL ; pThis = pThis->pNext) {
		if(ustrcmp(pThis->pStrgen->pName, pName) == 0) {
			*ppStrgen = pThis->pStrgen;
			FINALIZE;	/* found it, iRet still eq. OK! */
		}
	}

	iRet = RS_RET_PARSER_NOT_FOUND;

finalize_it:
	RETiRet;
}


/* --- END helper functions for strgen list handling --- */


BEGINobjConstruct(strgen) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(strgen)

/* ConstructionFinalizer. The most important chore is to add the strgen object
 * to our global list of available strgens.
 * rgerhards, 2009-11-03
 */
static rsRetVal
strgenConstructFinalize(strgen_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strgen);
	CHKiRet(AddStrgenToList(&pStrgenLstRoot, pThis));
	DBGPRINTF("Strgen '%s' added to list of available strgens.\n", pThis->pName);

finalize_it:
	RETiRet;
}

PROTOTYPEobjDestruct(strgen);
BEGINobjDestruct(strgen) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(strgen)
	dbgprintf("destructing strgen '%s'\n", pThis->pName);
	free(pThis->pName);
ENDobjDestruct(strgen)

/* set the strgen name - string is copied over, call can continue to use it,
 * but must free it if desired.
 */
static rsRetVal
SetName(strgen_t *pThis, uchar *name)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strgen);
	assert(name != NULL);

	if(pThis->pName != NULL) {
		free(pThis->pName);
		pThis->pName = NULL;
	}

	CHKmalloc(pThis->pName = ustrdup(name));

finalize_it:
	RETiRet;
}


/* set a pointer to "our" module. Note that no module
 * pointer must already be set.
 */
static rsRetVal
SetModPtr(strgen_t *pThis, modInfo_t *pMod)
{
	ISOBJ_TYPE_assert(pThis, strgen);
	assert(pMod != NULL);
	assert(pThis->pModule == NULL);
	pThis->pModule = pMod;
	return RS_RET_OK;
}


/* queryInterface function-- rgerhards, 2009-11-03
 */
BEGINobjQueryInterface(strgen)
CODESTARTobjQueryInterface(strgen)
	if(pIf->ifVersion != strgenCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = strgenConstruct;
	pIf->ConstructFinalize = strgenConstructFinalize;
	pIf->Destruct = strgenDestruct;
	pIf->SetName = SetName;
	pIf->SetModPtr = SetModPtr;
	pIf->InitStrgenList = InitStrgenList;
	pIf->DestructStrgenList = DestructStrgenList;
	pIf->AddStrgenToList = AddStrgenToList;
	pIf->FindStrgen = FindStrgen;
finalize_it:
ENDobjQueryInterface(strgen)


/* This destroys the master strgenlist and all of its strgen entries. MUST only be
 * done when the module is shut down. Strgen modules are NOT unloaded, rsyslog
 * does that at a later stage for all dynamically loaded modules.
 */
static void
destroyMasterStrgenList(void)
{
	strgenList_t *pStrgenLst;
	strgenList_t *pStrgenLstDel;

	pStrgenLst = pStrgenLstRoot;
	while(pStrgenLst != NULL) {
		strgenDestruct(&pStrgenLst->pStrgen);
		pStrgenLstDel = pStrgenLst;
		pStrgenLst = pStrgenLst->pNext;
		free(pStrgenLstDel);
	}
}

/* Exit our class.
 * rgerhards, 2009-11-04
 */
BEGINObjClassExit(strgen, OBJ_IS_CORE_MODULE) /* class, version */
	destroyMasterStrgenList();
	objRelease(glbl, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);
ENDObjClassExit(strgen)


/* Initialize the strgen class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2009-11-02
 */
BEGINObjClassInit(strgen, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));
	InitStrgenList(&pStrgenLstRoot);
ENDObjClassInit(strgen)

