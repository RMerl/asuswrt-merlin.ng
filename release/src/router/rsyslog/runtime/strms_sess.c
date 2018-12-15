/* strms_sess.c
 *
 * This implements a session of the strmsrv object. For general
 * comments, see header of strmsrv.c.
 *
 * Copyright 2007-2012 Adiscon GmbH.
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
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#include "rsyslog.h"
#include "dirty.h"
#include "module-template.h"
#include "net.h"
#include "strmsrv.h"
#include "strms_sess.h"
#include "obj.h"
#include "errmsg.h"
#include "netstrm.h"
#include "msg.h"
#include "prop.h"
#include "datetime.h"


/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(prop)
DEFobjCurrIf(netstrm)
DEFobjCurrIf(datetime)

static int iMaxLine; /* maximum size of a single message */

/* forward definitions */
static rsRetVal Close(strms_sess_t *pThis);


/* Standard-Constructor */
BEGINobjConstruct(strms_sess) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(strms_sess)


/* ConstructionFinalizer
 */
static rsRetVal
strms_sessConstructFinalize(strms_sess_t *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strms_sess);
	if(pThis->pSrv->OnSessConstructFinalize != NULL) {
		CHKiRet(pThis->pSrv->OnSessConstructFinalize(&pThis->pUsr));
	}

finalize_it:
	RETiRet;
}


/* destructor for the strms_sess object */
BEGINobjDestruct(strms_sess) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(strms_sess)
	if(pThis->pStrm != NULL)
		netstrm.Destruct(&pThis->pStrm);

	if(pThis->pSrv->pOnSessDestruct != NULL) {
		pThis->pSrv->pOnSessDestruct(&pThis->pUsr);
	}
	/* now destruct our own properties */
	free(pThis->fromHost);
	if(pThis->fromHostIP != NULL)
		prop.Destruct(&pThis->fromHostIP);
ENDobjDestruct(strms_sess)


/* debugprint for the strms_sess object */
BEGINobjDebugPrint(strms_sess) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(strms_sess)
ENDobjDebugPrint(strms_sess)


/* set property functions */
/* set the hostname. Note that the caller *hands over* the string. That is,
 * the caller no longer controls it once SetHost() has received it. Most importantly,
 * the caller must not free it. -- rgerhards, 2008-04-24
 */
static rsRetVal
SetHost(strms_sess_t *pThis, uchar *pszHost)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strms_sess);
	free(pThis->fromHost);
	pThis->fromHost = pszHost;
	RETiRet;
}

/* set the remote host's IP. Note that the caller *hands over* the property. That is,
 * the caller no longer controls it once SetHostIP() has received it. Most importantly,
 * the caller must not destruct it. -- rgerhards, 2008-05-16
 */
static rsRetVal
SetHostIP(strms_sess_t *pThis, prop_t *ip)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strms_sess);
	if(pThis->fromHostIP != NULL)
		prop.Destruct(&pThis->fromHostIP);
	pThis->fromHostIP = ip;
	RETiRet;
}

static rsRetVal
SetStrm(strms_sess_t *pThis, netstrm_t *pStrm)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strms_sess);
	pThis->pStrm = pStrm;
	RETiRet;
}


/* set our parent, the strmsrv object */
static rsRetVal
SetStrmsrv(strms_sess_t *pThis, strmsrv_t *pSrv)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strms_sess);
	ISOBJ_TYPE_assert(pSrv, strmsrv);
	pThis->pSrv = pSrv;
	RETiRet;
}


/* set our parent listener info*/
static rsRetVal
SetLstnInfo(strms_sess_t *pThis, strmLstnPortList_t *pLstnInfo)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strms_sess);
	assert(pLstnInfo != NULL);
	pThis->pLstnInfo = pLstnInfo;
	RETiRet;
}


static rsRetVal
SetUsrP(strms_sess_t *pThis, void *pUsr)
{
	DEFiRet;
	pThis->pUsr = pUsr;
	RETiRet;
}


static void *
GetUsrP(strms_sess_t *pThis)
{
	return pThis->pUsr;
}


/* Closes a STRM session
 * No attention is paid to the return code
 * of close, so potential-double closes are not detected.
 */
static rsRetVal
Close(strms_sess_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strms_sess);
	netstrm.Destruct(&pThis->pStrm);
	free(pThis->fromHost);
	pThis->fromHost = NULL; /* not really needed, but... */
	if(pThis->fromHostIP != NULL)
		prop.Destruct(&pThis->fromHostIP);

	RETiRet;
}



/* Processes the data received via a STRM session. If there
 * is no other way to handle it, data is discarded.
 * Input parameter data is the data received, iLen is its
 * len as returned from recv(). iLen must be 1 or more (that
 * is errors must be handled by caller!). iSTRMSess must be
 * the index of the STRM session that received the data.
 * rgerhards 2005-07-04
 * And another change while generalizing. We now return either
 * RS_RET_OK, which means the session should be kept open
 * or anything else, which means it must be closed.
 * rgerhards, 2008-03-01
 */
static rsRetVal
DataRcvd(strms_sess_t *pThis, char *pData, size_t iLen)
{
	DEFiRet;
	char *pEnd;

	ISOBJ_TYPE_assert(pThis, strms_sess);
	assert(pData != NULL);
	assert(iLen > 0);

	 /* We now copy the message to the session buffer. */
	pEnd = pData + iLen; /* this is one off, which is intensional */

	while(pData < pEnd) {
		CHKiRet(pThis->pSrv->OnCharRcvd(pThis, (uchar)*pData++));
	}

finalize_it:
	RETiRet;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(strms_sess)
CODESTARTobjQueryInterface(strms_sess)
	if(pIf->ifVersion != strms_sessCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->DebugPrint = strms_sessDebugPrint;
	pIf->Construct = strms_sessConstruct;
	pIf->ConstructFinalize = strms_sessConstructFinalize;
	pIf->Destruct = strms_sessDestruct;

	pIf->Close = Close;
	pIf->DataRcvd = DataRcvd;

	pIf->SetUsrP = SetUsrP;
	pIf->GetUsrP = GetUsrP;
	pIf->SetStrmsrv = SetStrmsrv;
	pIf->SetLstnInfo = SetLstnInfo;
	pIf->SetHost = SetHost;
	pIf->SetHostIP = SetHostIP;
	pIf->SetStrm = SetStrm;
finalize_it:
ENDobjQueryInterface(strms_sess)


/* exit our class
 * rgerhards, 2008-03-10
 */
BEGINObjClassExit(strms_sess, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(strms_sess)
	/* release objects we no longer need */
	objRelease(netstrm, LM_NETSTRMS_FILENAME);
	objRelease(datetime, CORE_COMPONENT);
ENDObjClassExit(strms_sess)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-29
 */
BEGINObjClassInit(strms_sess, 1, OBJ_IS_CORE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	/* request objects we use */
	CHKiRet(objUse(netstrm, LM_NETSTRMS_FILENAME));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	CHKiRet(objUse(glbl, CORE_COMPONENT));
	iMaxLine = glbl.GetMaxLine(); /* get maximum size we currently support */
	objRelease(glbl, CORE_COMPONENT);

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, strms_sessDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, strms_sessConstructFinalize);
ENDObjClassInit(strms_sess)

/* vim:set ai:
 */
