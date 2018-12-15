/* nspoll.c
 *
 * This is an io waiter interface utilizing the much-more-efficient poll/epoll API.
 * Note that it may not always be available for a given driver. If so, that is reported
 * back to the upper peer which then should consult a nssel-based io waiter.
 *
 * Work on this module begun 2009-11-18 by Rainer Gerhards.
 *
 * Copyright 2009-2014 Rainer Gerhards and Adiscon GmbH.
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
#include "nspoll.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)


/* load our low-level driver. This must be done before any
 * driver-specific functions (allmost all...) can be carried
 * out. Note that the driver's .ifIsLoaded is correctly
 * initialized by calloc() and we depend on that. Please note that
 * we do some name-mangeling. We know that each nsd driver also needs
 * a nspoll driver. So we simply append "sel" to the nsd driver name: This,
 * of course, means that the driver name must match these rules, but that
 * shouldn't be a real problem.
 * WARNING: this code is mostly identical to similar code in
 * netstrms.c - TODO: abstract it and move it to some common place.
 * rgerhards, 2008-04-28
 */
static rsRetVal
loadDrvr(nspoll_t *pThis)
{
	DEFiRet;
	uchar *pBaseDrvrName;
	uchar szDrvrName[48]; /* 48 shall be large enough */

	pBaseDrvrName = pThis->pBaseDrvrName;
	if(pBaseDrvrName == NULL) /* if no drvr name is set, use system default */
		pBaseDrvrName = glbl.GetDfltNetstrmDrvr();
	if(snprintf((char*)szDrvrName, sizeof(szDrvrName), "lmnsdpoll_%s", pBaseDrvrName) == sizeof(szDrvrName))
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
BEGINobjConstruct(nspoll) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(nspoll)


/* destructor for the nspoll object */
BEGINobjDestruct(nspoll) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(nspoll)
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
ENDobjDestruct(nspoll)


/* ConstructionFinalizer */
static rsRetVal
ConstructFinalize(nspoll_t *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nspoll);
	CHKiRet(loadDrvr(pThis));
	CHKiRet(pThis->Drvr.Construct(&pThis->pDrvrData));
finalize_it:
	RETiRet;
}


/* Carries out the actual wait (all done in lower layers)
 */
static rsRetVal
Wait(nspoll_t *pThis, int timeout, int *numEntries, nsd_epworkset_t workset[]) {
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nspoll);
	assert(workset != NULL);
	iRet = pThis->Drvr.Wait(pThis->pDrvrData, timeout, numEntries, workset);
	RETiRet;
}


/* set the base driver name. If the driver name
 * is set to NULL, the previously set name is deleted but
 * no name set again (which results in the system default being
 * used)-- rgerhards, 2008-05-05
 */
static rsRetVal
SetDrvrName(nspoll_t *pThis, uchar *pszName)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nspoll);
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


/* semantics like the epoll_ctl() function, does the same thing.
 * rgerhards, 2009-11-18
 */
static rsRetVal
Ctl(nspoll_t *pThis, netstrm_t *pStrm, int id, void *pUsr, int mode, int op) {
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nspoll);
	iRet = pThis->Drvr.Ctl(pThis->pDrvrData, pStrm->pDrvrData, id, pUsr, mode, op);
	RETiRet;
}


/* queryInterface function */
BEGINobjQueryInterface(nspoll)
CODESTARTobjQueryInterface(nspoll)
	if(pIf->ifVersion != nspollCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = nspollConstruct;
	pIf->ConstructFinalize = ConstructFinalize;
	pIf->SetDrvrName = SetDrvrName;
	pIf->Destruct = nspollDestruct;
	pIf->Wait = Wait;
	pIf->Ctl = Ctl;
finalize_it:
ENDobjQueryInterface(nspoll)


/* exit our class
 */
BEGINObjClassExit(nspoll, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nspoll)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
ENDObjClassExit(nspoll)


/* Initialize the nspoll class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(nspoll, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */
	DBGPRINTF("doing nspollClassInit\n");
	CHKiRet(objUse(glbl, CORE_COMPONENT));

	/* set our own handlers */
ENDObjClassInit(nspoll)
/* vi:set ai:
 */
