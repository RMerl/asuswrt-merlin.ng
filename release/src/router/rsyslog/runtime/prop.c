/* prop.c - rsyslog's prop object
 *
 * This object is meant to support message properties that are stored
 * seperately from the message. The main intent is to support properties
 * that are "constant" during a period of time, so that many messages may
 * contain a reference to the same property. It is important, though, that
 * properties are destroyed when they are no longer needed.
 *
 * Please note that this is a performance-critical part of the software and
 * as such we may use some methods in here which do not look elegant, but
 * which are fast...
 *
 * Module begun 2009-06-17 by Rainer Gerhards
 *
 * Copyright 2009-2016 Adiscon GmbH.
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "rsyslog.h"
#include "obj.h"
#include "obj-types.h"
#include "unicode-helper.h"
#include "atomic.h"
#include "prop.h"

/* static data */
DEFobjStaticHelpers

//extern uchar *propGetSzStr(prop_t *pThis); /* expand inline function here */

/* Standard-Constructor
 */
BEGINobjConstruct(prop) /* be sure to specify the object type also in END macro! */
	pThis->iRefCount = 1;
	INIT_ATOMIC_HELPER_MUT(pThis->mutRefCount);
ENDobjConstruct(prop)


/* destructor for the prop object */
BEGINobjDestruct(prop) /* be sure to specify the object type also in END and CODESTART macros! */
	int currRefCount;
CODESTARTobjDestruct(prop)
	currRefCount = ATOMIC_DEC_AND_FETCH(&pThis->iRefCount, &pThis->mutRefCount);
	if(currRefCount == 0) {
		/* (only) in this case we need to actually destruct the object */
		if(pThis->len >= CONF_PROP_BUFSIZE)
			free(pThis->szVal.psz);
		DESTROY_ATOMIC_HELPER_MUT(pThis->mutRefCount);
	} else {
		pThis = NULL; /* tell framework NOT to destructing the object! */
	}
ENDobjDestruct(prop)

/* set string, we make our own private copy! This MUST only be called BEFORE
 * ConstructFinalize()!
 */
static rsRetVal SetString(prop_t *pThis, const uchar *psz, const int len)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, prop);
	if(pThis->len >= CONF_PROP_BUFSIZE)
		free(pThis->szVal.psz);
	pThis->len = len;
	if(len < CONF_PROP_BUFSIZE) {
		memcpy(pThis->szVal.sz, psz, len + 1);
	} else {
		CHKmalloc(pThis->szVal.psz = MALLOC(len + 1));
		memcpy(pThis->szVal.psz, psz, len + 1);
	}

finalize_it:
	RETiRet;
}


/* get string length */
static int GetStringLen(prop_t *pThis)
{
	return pThis->len;
}


/* get string */
static rsRetVal GetString(prop_t *pThis, uchar **ppsz, int *plen)
{
	BEGINfunc
	ISOBJ_TYPE_assert(pThis, prop);
	if(pThis->len < CONF_PROP_BUFSIZE) {
		*ppsz = pThis->szVal.sz;
	} else {
		*ppsz = pThis->szVal.psz;
	}
	*plen = pThis->len;
	ENDfunc
	return RS_RET_OK;
}


/* ConstructionFinalizer
 * rgerhards, 2008-01-09
 */
static rsRetVal
propConstructFinalize(prop_t __attribute__((unused)) *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, prop);
	RETiRet;
}


/* add a new reference. It is VERY IMPORTANT to call this function whenever
 * the property is handed over to some entitiy that later call Destruct() on it.
 */
static rsRetVal AddRef(prop_t *pThis)
{
	if(pThis == NULL)  {
		DBGPRINTF("prop/AddRef is passed a NULL ptr - ignoring it "
			"- further problems may occur\n");
		FINALIZE;
	}
	ATOMIC_INC(&pThis->iRefCount, &pThis->mutRefCount);
finalize_it:
	return RS_RET_OK;
}


/* this is a "do it all in one shot" function that creates a new property,
 * assigns the provided string to it and finalizes the property. Among the
 * convenience, it is also (very, very) slightly faster.
 * rgerhards, 2009-07-01
 */
static rsRetVal CreateStringProp(prop_t **ppThis, const uchar* psz, const int len)
{
	prop_t *pThis = NULL;
	DEFiRet;

	CHKiRet(propConstruct(&pThis));
	CHKiRet(SetString(pThis, psz, len));
	CHKiRet(propConstructFinalize(pThis));
	*ppThis = pThis;
finalize_it:
	if(iRet != RS_RET_OK) {
		if(pThis != NULL)
			propDestruct(&pThis);
	}

	RETiRet;
}

/* another one-stop function, quite useful: it takes a property pointer and
 * a string. If the string is already contained in the property, nothing happens.
 * If the string is different (or the pointer NULL), the current property
 * is destructed and a new one created. This can be used to get a specific
 * name in those cases where there is a good chance that the property
 * immediatly previously processed already contained the value we need - in
 * which case we save us all the creation overhead by just reusing the already
 * existing property).
 * rgerhards, 2009-07-01
 */
static rsRetVal CreateOrReuseStringProp(prop_t **ppThis, const uchar *psz, const int len)
{
	uchar *pszPrev;
	int lenPrev;
	DEFiRet;
	assert(ppThis != NULL);

	if(*ppThis == NULL) {
		/* we need to create a property */
		CHKiRet(CreateStringProp(ppThis, psz, len));
	} else {
		/* already exists, check if we can re-use it */
		GetString(*ppThis, &pszPrev, &lenPrev);
		if(len != lenPrev || ustrcmp(psz, pszPrev)) {
			/* different, need to discard old & create new one */
			propDestruct(ppThis);
			CHKiRet(CreateStringProp(ppThis, psz, len));
		} /* else we can re-use the existing one! */
	}

finalize_it:
	RETiRet;
}


/* debugprint for the prop object */
BEGINobjDebugPrint(prop) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(prop)
	dbgprintf("prop object %p - no further debug info implemented\n", pThis);
ENDobjDebugPrint(prop)


/* queryInterface function
 * rgerhards, 2008-02-21
 */
BEGINobjQueryInterface(prop)
CODESTARTobjQueryInterface(prop)
	if(pIf->ifVersion != propCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = propConstruct;
	pIf->ConstructFinalize = propConstructFinalize;
	pIf->Destruct = propDestruct;
	pIf->DebugPrint = propDebugPrint;
	pIf->SetString = SetString;
	pIf->GetString = GetString;
	pIf->GetStringLen = GetStringLen;
	pIf->AddRef = AddRef;
	pIf->CreateStringProp = CreateStringProp;
	pIf->CreateOrReuseStringProp = CreateOrReuseStringProp;

finalize_it:
ENDobjQueryInterface(prop)


/* Exit the prop class.
 * rgerhards, 2009-04-06
 */
BEGINObjClassExit(prop, OBJ_IS_CORE_MODULE) /* class, version */
ENDObjClassExit(prop)


/* Initialize the prop class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(prop, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, propDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, propConstructFinalize);
ENDObjClassInit(prop)

/* vi:set ai:
 */
