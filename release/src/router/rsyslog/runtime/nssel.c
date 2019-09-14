/* nssel.c
 *
 * The io waiter is a helper object enabling us to wait on a set of streams to become
 * ready for IO - this is modelled after select(). We need this, because
 * stream drivers may have different concepts. Consequently,
 * the structure must contain nsd_t's from the same stream driver type
 * only. This is implemented as a singly-linked list where every
 * new element is added at the top of the list.
 *
 * Work on this module begun 2008-04-22 by Rainer Gerhards.
 *
 * Copyright 2008-2014 Adiscon GmbH.
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

#include "rsyslog.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "rsyslog.h"
#include "obj.h"
#include "module-template.h"
#include "netstrm.h"
#include "nssel.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)


/* load our low-level driver. This must be done before any
 * driver-specific functions (allmost all...) can be carried
 * out. Note that the driver's .ifIsLoaded is correctly
 * initialized by calloc() and we depend on that. Please note that
 * we do some name-mangeling. We know that each nsd driver also needs
 * a nssel driver. So we simply append "sel" to the nsd driver name: This,
 * of course, means that the driver name must match these rules, but that
 * shouldn't be a real problem.
 * WARNING: this code is mostly identical to similar code in
 * netstrms.c - TODO: abstract it and move it to some common place.
 * rgerhards, 2008-04-28
 */
static rsRetVal
loadDrvr(nssel_t *pThis)
{
	DEFiRet;
	uchar *pBaseDrvrName;
	uchar szDrvrName[48]; /* 48 shall be large enough */

	pBaseDrvrName = pThis->pBaseDrvrName;
	if(pBaseDrvrName == NULL) /* if no drvr name is set, use system default */
		pBaseDrvrName = glbl.GetDfltNetstrmDrvr();
	if(snprintf((char*)szDrvrName, sizeof(szDrvrName), "lmnsdsel_%s", pBaseDrvrName) == sizeof(szDrvrName))
		ABORT_FINALIZE(RS_RET_DRVRNAME_TOO_LONG);
	CHKmalloc(pThis->pDrvrName = (uchar*) strdup((char*)szDrvrName));

	pThis->Drvr.ifVersion = nsdCURR_IF_VERSION;
	/* The pDrvrName+2 below is a hack to obtain the object name. It
	 * safes us to have yet another variable with the name without "lm" in
	 * front of it. If we change the module load interface, we may re-think
	 * about this hack, but for the time being it is efficient and clean
	 * enough. -- rgerhards, 2008-04-18
	 */
	CHKiRet(obj.UseObj(__FILE__, szDrvrName+2, DONT_LOAD_LIB, (void*) &pThis->Drvr));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pThis->pDrvrName != NULL) {
			free(pThis->pDrvrName);
			pThis->pDrvrName = NULL;
		}
	}
	RETiRet;
}


/* Standard-Constructor */
BEGINobjConstruct(nssel) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(nssel)


/* destructor for the nssel object */
BEGINobjDestruct(nssel) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(nssel)
	if(pThis->pDrvrData != NULL)
		pThis->Drvr.Destruct(&pThis->pDrvrData);

	/* and now we must release our driver, if we got one. We use the presence of
	 * a driver name string as load indicator (because we also need that string
	 * to release the driver
	 */
	free(pThis->pBaseDrvrName);
	if(pThis->pDrvrName != NULL) {
		obj.ReleaseObj(__FILE__, pThis->pDrvrName+2, DONT_LOAD_LIB, (void*) &pThis->Drvr);
		free(pThis->pDrvrName);
	}
ENDobjDestruct(nssel)


/* ConstructionFinalizer */
static rsRetVal
ConstructFinalize(nssel_t *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nssel);
	CHKiRet(loadDrvr(pThis));
	CHKiRet(pThis->Drvr.Construct(&pThis->pDrvrData));
finalize_it:
	RETiRet;
}


/* set the base driver name. If the driver name
 * is set to NULL, the previously set name is deleted but
 * no name set again (which results in the system default being
 * used)-- rgerhards, 2008-05-05
 */
static rsRetVal
SetDrvrName(nssel_t *pThis, uchar *pszName)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nssel);
	if(pThis->pBaseDrvrName != NULL) {
		free(pThis->pBaseDrvrName);
		pThis->pBaseDrvrName = NULL;
	}

	if(pszName != NULL) {
		CHKmalloc(pThis->pBaseDrvrName = (uchar*) strdup((char*) pszName));
	}
finalize_it:
	RETiRet;
}


/* Add a stream object to the current select() set.
 * Note that a single stream may have multiple "sockets" if
 * it is a listener. If so, all of them are begin added.
 */
static rsRetVal
Add(nssel_t *pThis, netstrm_t *pStrm, nsdsel_waitOp_t waitOp)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nssel);
	ISOBJ_TYPE_assert(pStrm, netstrm);
	
	CHKiRet(pThis->Drvr.Add(pThis->pDrvrData, pStrm->pDrvrData, waitOp));

finalize_it:
	RETiRet;
}


/* wait for IO to happen on one of our netstreams. iNumReady has
 * the number of ready "sockets" after the call. This function blocks
 * until some are ready. EAGAIN is retried.
 */
static rsRetVal
Wait(nssel_t *pThis, int *piNumReady)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nssel);
	assert(piNumReady != NULL);
	iRet = pThis->Drvr.Select(pThis->pDrvrData, piNumReady);
	RETiRet;
}


/* Check if a stream is ready for IO. *piNumReady contains the remaining number
 * of ready streams. Note that this function may say the stream is not ready
 * but still decrement *piNumReady. This can happen when (e.g. with TLS) the low
 * level driver requires some IO which is hidden from the upper layer point of view.
 * rgerhards, 2008-04-23
 */
static rsRetVal
IsReady(nssel_t *pThis, netstrm_t *pStrm, nsdsel_waitOp_t waitOp, int *pbIsReady,
int __attribute__((unused)) *piNumReady)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nssel);
	ISOBJ_TYPE_assert(pStrm, netstrm);
	assert(pbIsReady != NULL);
	assert(piNumReady != NULL);
	iRet = pThis->Drvr.IsReady(pThis->pDrvrData, pStrm->pDrvrData, waitOp, pbIsReady);
	RETiRet;
}


/* queryInterface function */
BEGINobjQueryInterface(nssel)
CODESTARTobjQueryInterface(nssel)
	if(pIf->ifVersion != nsselCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = nsselConstruct;
	pIf->ConstructFinalize = ConstructFinalize;
	pIf->Destruct = nsselDestruct;
	pIf->SetDrvrName = SetDrvrName;
	pIf->Add = Add;
	pIf->Wait = Wait;
	pIf->IsReady = IsReady;
finalize_it:
ENDobjQueryInterface(nssel)


/* exit our class
 */
BEGINObjClassExit(nssel, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nssel)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
ENDObjClassExit(nssel)


/* Initialize the nssel class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(nssel, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */
	DBGPRINTF("doing nsselClassInit\n");
	CHKiRet(objUse(glbl, CORE_COMPONENT));

	/* set our own handlers */
ENDObjClassInit(nssel)
/* vi:set ai:
 */
