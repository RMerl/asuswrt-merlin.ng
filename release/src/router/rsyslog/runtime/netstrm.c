/* netstrm.c
 *
 * This class implements a generic netstrmwork stream class. It supports
 * sending and receiving data streams over a netstrmwork. The class abstracts
 * the transport, though it is a safe assumption that TCP is being used.
 * The class has a number of properties, among which are also ones to
 * select privacy settings, eg by enabling TLS and/or GSSAPI. In the
 * long run, this class shall provide all stream-oriented netstrmwork
 * functionality inside rsyslog.
 *
 * It is a high-level class, which uses a number of helper objects
 * to carry out its work (including, and most importantly, transport
 * drivers).
 *
 * Work on this module begun 2008-04-17 by Rainer Gerhards. This code
 * borrows from librelp's tcp.c/.h code. librelp is dual licensed and
 * Rainer Gerhards and Adiscon GmbH have agreed to permit using the code
 * under the terms of the GNU Lesser General Public License.
 *
 * Copyright 2007-2009 Rainer Gerhards and Adiscon GmbH.
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
#include <assert.h>
#include <string.h>

#include "rsyslog.h"
#include "net.h"
#include "module-template.h"
#include "obj.h"
#include "errmsg.h"
#include "netstrms.h"
#include "netstrm.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(netstrms)


/* Standard-Constructor */
BEGINobjConstruct(netstrm) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(netstrm)


/* destructor for the netstrm object */
BEGINobjDestruct(netstrm) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(netstrm)
	if(pThis->pDrvrData != NULL)
		iRet = pThis->Drvr.Destruct(&pThis->pDrvrData);
ENDobjDestruct(netstrm)


/* ConstructionFinalizer */
static rsRetVal
netstrmConstructFinalize(netstrm_t *pThis)
{
	ISOBJ_TYPE_assert(pThis, netstrm);
	return pThis->Drvr.Construct(&pThis->pDrvrData);
}

/* abort a connection. This is much like Destruct(), but tries
 * to discard any unsent data. -- rgerhards, 2008-03-24
 */
static rsRetVal
AbortDestruct(netstrm_t **ppThis)
{
	DEFiRet;
	assert(ppThis != NULL);
	ISOBJ_TYPE_assert((*ppThis), netstrm);

	/* we do NOT exit on error, because that would make things worse */
	(*ppThis)->Drvr.Abort((*ppThis)->pDrvrData);
	iRet = netstrmDestruct(ppThis);

	RETiRet;
}


/* accept an incoming connection request
 * The netstrm instance that had the incoming request must be provided. If
 * the connection request succeeds, a new netstrm object is created and
 * passed back to the caller. The caller is responsible for destructing it.
 * pReq is the nsd_t obj that has the accept request.
 * rgerhards, 2008-04-21
 */
static rsRetVal
AcceptConnReq(netstrm_t *pThis, netstrm_t **ppNew)
{
	nsd_t *pNewNsd = NULL;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, netstrm);
	assert(ppNew != NULL);

	/* accept the new connection */
	CHKiRet(pThis->Drvr.AcceptConnReq(pThis->pDrvrData, &pNewNsd));
	/* construct our object so that we can use it... */
	CHKiRet(objUse(netstrms, DONT_LOAD_LIB)); /* use netstrms obj if not already done so */
	CHKiRet(netstrms.CreateStrm(pThis->pNS, ppNew));
	(*ppNew)->pDrvrData = pNewNsd;

finalize_it:
	if(iRet != RS_RET_OK) {
		/* the close may be redundant, but that doesn't hurt... */
		if(pNewNsd != NULL)
			pThis->Drvr.Destruct(&pNewNsd);
	}

	RETiRet;
}


/* make the netstrm listen to specified port and IP.
 * pLstnIP points to the port to listen to (NULL means "all"),
 * iMaxSess has the maximum number of sessions permitted (this ist just a hint).
 * pLstnPort must point to a port name or number. NULL is NOT permitted.
 * rgerhards, 2008-04-22
 */
static rsRetVal
LstnInit(netstrms_t *pNS, void *pUsr, rsRetVal(*fAddLstn)(void*,netstrm_t*),
	 uchar *pLstnPort, uchar *pLstnIP, int iSessMax,
	 uchar *pszLstnPortFileName)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pNS, netstrms);
	assert(fAddLstn != NULL);
	assert(pLstnPort != NULL);

	CHKiRet(pNS->Drvr.LstnInit(pNS, pUsr, fAddLstn, pLstnPort, pLstnIP, iSessMax, pszLstnPortFileName));

finalize_it:
	RETiRet;
}


/* receive data from a tcp socket
 * The lenBuf parameter must contain the max buffer size on entry and contains
 * the number of octets read (or -1 in case of error) on exit. This function
 * never blocks, not even when called on a blocking socket. That is important
 * for client sockets, which are set to block during send, but should not
 * block when trying to read data. If *pLenBuf is -1, an error occured and
 * oserr holds the exact error cause.
 * rgerhards, 2008-03-17
 */
static rsRetVal
Rcv(netstrm_t *pThis, uchar *pBuf, ssize_t *pLenBuf, int *const oserr)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.Rcv(pThis->pDrvrData, pBuf, pLenBuf, oserr);
	RETiRet;
}

/* here follows a number of methods that shuffle authentication settings down
 * to the drivers. Drivers not supporting these settings may return an error
 * state.
 * -------------------------------------------------------------------------- */

/* set the driver mode
 * rgerhards, 2008-04-28
 */
static rsRetVal
SetDrvrMode(netstrm_t *pThis, int iMode)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetMode(pThis->pDrvrData, iMode);
	RETiRet;
}


/* set the driver authentication mode -- rgerhards, 2008-05-16
 */
static rsRetVal
SetDrvrAuthMode(netstrm_t *pThis, uchar *mode)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetAuthMode(pThis->pDrvrData, mode);
	RETiRet;
}


/* set the driver's permitted peers -- rgerhards, 2008-05-19 */
static rsRetVal
SetDrvrPermPeers(netstrm_t *pThis, permittedPeers_t *pPermPeers)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetPermPeers(pThis->pDrvrData, pPermPeers);
	RETiRet;
}


/* End of methods to shuffle autentication settings to the driver.
 * -------------------------------------------------------------------------- */


/* send a buffer. On entry, pLenBuf contains the number of octets to
 * write. On exit, it contains the number of octets actually written.
 * If this number is lower than on entry, only a partial buffer has
 * been written.
 * rgerhards, 2008-03-19
 */
static rsRetVal
Send(netstrm_t *pThis, uchar *pBuf, ssize_t *pLenBuf)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.Send(pThis->pDrvrData, pBuf, pLenBuf);
	RETiRet;
}

/* Enable Keep-Alive handling for those drivers that support it.
 * rgerhards, 2009-06-02
 */
static rsRetVal
EnableKeepAlive(netstrm_t *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.EnableKeepAlive(pThis->pDrvrData);
	RETiRet;
}

/* Keep-Alive options
 */
static rsRetVal
SetKeepAliveProbes(netstrm_t *pThis, int keepAliveProbes)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetKeepAliveProbes(pThis->pDrvrData, keepAliveProbes);
	RETiRet;
}

/* Keep-Alive options
 */
static rsRetVal
SetKeepAliveTime(netstrm_t *pThis, int keepAliveTime)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetKeepAliveTime(pThis->pDrvrData, keepAliveTime);
	RETiRet;
}

/* Keep-Alive options
 */
static rsRetVal
SetKeepAliveIntvl(netstrm_t *pThis, int keepAliveIntvl)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetKeepAliveIntvl(pThis->pDrvrData, keepAliveIntvl);
	RETiRet;
}

/* gnutls priority string */
static rsRetVal
SetGnutlsPriorityString(netstrm_t *pThis, uchar *gnutlsPriorityString)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.SetGnutlsPriorityString(pThis->pDrvrData, gnutlsPriorityString);
	RETiRet;
}

/* check connection - slim wrapper for NSD driver function */
static rsRetVal
CheckConnection(netstrm_t *pThis)
{
	ISOBJ_TYPE_assert(pThis, netstrm);
	return pThis->Drvr.CheckConnection(pThis->pDrvrData);
}


/* get remote hname - slim wrapper for NSD driver function */
static rsRetVal
GetRemoteHName(netstrm_t *pThis, uchar **ppsz)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.GetRemoteHName(pThis->pDrvrData, ppsz);
	RETiRet;
}


/* get remote IP - slim wrapper for NSD driver function */
static rsRetVal
GetRemoteIP(netstrm_t *pThis, prop_t **ip)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.GetRemoteIP(pThis->pDrvrData, ip);
	RETiRet;
}


/* get remote addr - slim wrapper for NSD driver function */
static rsRetVal
GetRemAddr(netstrm_t *pThis, struct sockaddr_storage **ppAddr)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	iRet = pThis->Drvr.GetRemAddr(pThis->pDrvrData, ppAddr);
	RETiRet;
}


/* open a connection to a remote host (server).
 * rgerhards, 2008-03-19
 */
static rsRetVal
Connect(netstrm_t *pThis, int family, uchar *port, uchar *host, char *device)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	assert(port != NULL);
	assert(host != NULL);
	iRet = pThis->Drvr.Connect(pThis->pDrvrData, family, port, host, device);
	RETiRet;
}


/* Provide access to the underlying OS socket. This is dirty
 * and scheduled to be removed. Does not work with all nsd drivers.
 * See comment in netstrm interface for details.
 * rgerhards, 2008-05-05
 */
static rsRetVal
GetSock(netstrm_t *pThis, int *pSock)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, netstrm);
	assert(pSock != NULL);
	iRet = pThis->Drvr.GetSock(pThis->pDrvrData, pSock);
	RETiRet;
}


/* queryInterface function
 */
BEGINobjQueryInterface(netstrm)
CODESTARTobjQueryInterface(netstrm)
	if(pIf->ifVersion != netstrmCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = netstrmConstruct;
	pIf->ConstructFinalize = netstrmConstructFinalize;
	pIf->Destruct = netstrmDestruct;
	pIf->AbortDestruct = AbortDestruct;
	pIf->Rcv = Rcv;
	pIf->Send = Send;
	pIf->Connect = Connect;
	pIf->LstnInit = LstnInit;
	pIf->AcceptConnReq = AcceptConnReq;
	pIf->GetRemoteHName = GetRemoteHName;
	pIf->GetRemoteIP = GetRemoteIP;
	pIf->GetRemAddr = GetRemAddr;
	pIf->SetDrvrMode = SetDrvrMode;
	pIf->SetDrvrAuthMode = SetDrvrAuthMode;
	pIf->SetDrvrPermPeers = SetDrvrPermPeers;
	pIf->CheckConnection = CheckConnection;
	pIf->GetSock = GetSock;
	pIf->EnableKeepAlive = EnableKeepAlive;
	pIf->SetKeepAliveProbes = SetKeepAliveProbes;
	pIf->SetKeepAliveTime = SetKeepAliveTime;
	pIf->SetKeepAliveIntvl = SetKeepAliveIntvl;
	pIf->SetGnutlsPriorityString = SetGnutlsPriorityString;
finalize_it:
ENDobjQueryInterface(netstrm)


/* exit our class
 */
BEGINObjClassExit(netstrm, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(netstrm)
	/* release objects we no longer need */
	objRelease(netstrms, DONT_LOAD_LIB);
ENDObjClassExit(netstrm)


/* Initialize the netstrm class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINAbstractObjClassInit(netstrm, 1, OBJ_IS_CORE_MODULE) /* class, version */
	/* request objects we use */

	/* set our own handlers */
ENDObjClassInit(netstrm)
/* vi:set ai:
 */
