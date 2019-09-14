/* strmsrv.c
 *
 * This builds a basic stream server. It handles connection creation but
 * not any protocol. Instead, it calls a "data received" entry point of the
 * caller with any data received, in which case the caller must react accordingly.
 * This module works together with the netstream drivers.
 *
 * There are actually two classes within the stream server code: one is
 * the strmsrv itself, the other one is its sessions. This is a helper
 * class to strmsrv.
 *
 * File begun on 2009-06-01 by RGerhards based on strmsrv.c. Note that strmsrv is
 * placed under LGPL, which is possible because I carefully evaluated and
 * eliminated all those parts of strmsrv which were not written by me.
 *
 * TODO: I would consider it useful to migrate tcpsrv.c/tcps_sess.c to this stream
 * class here. The requires a little bit redesign, but should not be too hard. The
 * core idea, already begun here, is that we still support lots of callbacks, but
 * provide "canned" implementations for standard cases. That way, most upper-layer
 * modules can be kept rather simple and without any extra overhead. Note that
 * to support this, tcps_sess.c would need to extract the message reception state
 * machine to a separate module which then is called via the DoCharRcvd() interface
 * of this class here. -- rgerhards, 2009-06-01
 *
 * Copyright 2007-2016 Adiscon GmbH.
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
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "rsyslog.h"
#include "dirty.h"
#include "cfsysline.h"
#include "module-template.h"
#include "net.h"
#include "srUtils.h"
#include "conf.h"
#include "strmsrv.h"
#include "obj.h"
#include "glbl.h"
#include "netstrms.h"
#include "netstrm.h"
#include "nssel.h"
#include "errmsg.h"
#include "prop.h"
#include "unicode-helper.h"

#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif

MODULE_TYPE_LIB
MODULE_TYPE_NOKEEP

/* defines */
#define STRMSESS_MAX_DEFAULT 200 /* default for nbr of strm sessions if no number is given */
#define STRMLSTN_MAX_DEFAULT 20 /* default for nbr of listeners */

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(conf)
DEFobjCurrIf(glbl)
DEFobjCurrIf(strms_sess)
DEFobjCurrIf(net)
DEFobjCurrIf(netstrms)
DEFobjCurrIf(netstrm)
DEFobjCurrIf(nssel)
DEFobjCurrIf(prop)

/* forward definitions */
static rsRetVal create_strm_socket(strmsrv_t *pThis);

/* standard callbacks, if the caller did not provide us with them (this helps keep us
 * flexible while at the same time permits very simple upper-layer modules)
 */
/* this shall go into a specific ACL module! */
static int
isPermittedHost(struct sockaddr __attribute__((unused)) *addr, char __attribute__((unused)) *fromHostFQDN,
		void __attribute__((unused)) *pUsrSrv, void __attribute__((unused)) *pUsrSess)
{
	return 1;
}


static rsRetVal
doOpenLstnSocks(strmsrv_t *pSrv)
{
	ISOBJ_TYPE_assert(pSrv, strmsrv);
	return create_strm_socket(pSrv);
}


static rsRetVal
doRcvData(strms_sess_t *pSess, char *buf, size_t lenBuf, ssize_t *piLenRcvd, int *const oserr)
{
	DEFiRet;
	assert(pSess != NULL);
	assert(piLenRcvd != NULL);

	*piLenRcvd = lenBuf;
	iRet = netstrm.Rcv(pSess->pStrm, (uchar*) buf, piLenRcvd, oserr);
	RETiRet;
}

static rsRetVal
onRegularClose(strms_sess_t *pSess)
{
	DEFiRet;
	assert(pSess != NULL);

	/* process any incomplete frames left over */
	//strms_sess.PrepareClose(pSess);
	/* Session closed */
	strms_sess.Close(pSess);
	RETiRet;
}


static rsRetVal
onErrClose(strms_sess_t *pSess)
{
	DEFiRet;
	assert(pSess != NULL);

	strms_sess.Close(pSess);
	RETiRet;
}

/* ------------------------------ end callbacks ------------------------------ */

/* add new listener port to listener port list
 * rgerhards, 2009-05-21
 */
static rsRetVal
addNewLstnPort(strmsrv_t *pThis, uchar *pszPort)
{
	strmLstnPortList_t *pEntry;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strmsrv);

	/* create entry */
	CHKmalloc(pEntry = MALLOC(sizeof(strmLstnPortList_t)));
	pEntry->pszPort = pszPort;
	pEntry->pSrv = pThis;
	if((pEntry->pszInputName = ustrdup(pThis->pszInputName)) == NULL) {
		DBGPRINTF("strmsrv/addNewLstnPort: OOM in strdup()\n");
		free(pEntry);
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	/* and add to list */
	pEntry->pNext = pThis->pLstnPorts;
	pThis->pLstnPorts = pEntry;

finalize_it:
	RETiRet;
}


/* configure STRM listener settings.
 * Note: pszPort is handed over to us - the caller MUST NOT free it!
 * rgerhards, 2008-03-20
 */
static rsRetVal
configureSTRMListen(strmsrv_t *pThis, uchar *pszPort)
{
	int i;
	uchar *pPort = pszPort;
	DEFiRet;

	assert(pszPort != NULL);
	ISOBJ_TYPE_assert(pThis, strmsrv);

	/* extract port */
	i = 0;
	while(isdigit((int) *pPort)) {
		i = i * 10 + *pPort++ - '0';
	}

	if(i >= 0 && i <= 65535) {
		CHKiRet(addNewLstnPort(pThis, pszPort));
	} else {
		LogError(0, NO_ERRCODE, "Invalid STRM listen port %s - ignored.\n", pszPort);
	}

finalize_it:
	RETiRet;
}


/* Initialize the session table
 * returns 0 if OK, somewhat else otherwise
 */
static rsRetVal
STRMSessTblInit(strmsrv_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strmsrv);
	assert(pThis->pSessions == NULL);

	dbgprintf("Allocating buffer for %d STRM sessions.\n", pThis->iSessMax);
	if((pThis->pSessions = (strms_sess_t **) calloc(pThis->iSessMax, sizeof(strms_sess_t *))) == NULL) {
		dbgprintf("Error: STRMSessInit() could not alloc memory for STRM session table.\n");
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

finalize_it:
	RETiRet;
}


/* find a free spot in the session table. If the table
 * is full, -1 is returned, else the index of the free
 * entry (0 or higher).
 */
static int
STRMSessTblFindFreeSpot(strmsrv_t *pThis)
{
	register int i;

	ISOBJ_TYPE_assert(pThis, strmsrv);

	for(i = 0 ; i < pThis->iSessMax ; ++i) {
		if(pThis->pSessions[i] == NULL)
			break;
	}

	return((i < pThis->iSessMax) ? i : -1);
}


/* Get the next session index. Free session tables entries are
 * skipped. This function is provided the index of the last
 * session entry, or -1 if no previous entry was obtained. It
 * returns the index of the next session or -1, if there is no
 * further entry in the table. Please note that the initial call
 * might as well return -1, if there is no session at all in the
 * session table.
 */
static int
STRMSessGetNxtSess(strmsrv_t *pThis, int iCurr)
{
	register int i;

	BEGINfunc
	ISOBJ_TYPE_assert(pThis, strmsrv);
	assert(pThis->pSessions != NULL);
	for(i = iCurr + 1 ; i < pThis->iSessMax ; ++i) {
		if(pThis->pSessions[i] != NULL)
			break;
	}

	ENDfunc
	return((i < pThis->iSessMax) ? i : -1);
}


/* De-Initialize STRM listner sockets.
 * This function deinitializes everything, including freeing the
 * session table. No STRM listen receive operations are permitted
 * unless the subsystem is reinitialized.
 * rgerhards, 2007-06-21
 */
static void deinit_strm_listener(strmsrv_t *pThis)
{
	int i;
	strmLstnPortList_t *pEntry;
	strmLstnPortList_t *pDel;

	ISOBJ_TYPE_assert(pThis, strmsrv);

	if(pThis->pSessions != NULL) {
		/* close all STRM connections! */
		i = STRMSessGetNxtSess(pThis, -1);
		while(i != -1) {
			strms_sess.Destruct(&pThis->pSessions[i]);
			/* now get next... */
			i = STRMSessGetNxtSess(pThis, i);
		}

		/* we are done with the session table - so get rid of it...  */
		free(pThis->pSessions);
		pThis->pSessions = NULL; /* just to make sure... */
	}

	/* free list of strm listen ports */
	pEntry = pThis->pLstnPorts;
	while(pEntry != NULL) {
		free(pEntry->pszPort);
		free(pEntry->pszInputName);
		pDel = pEntry;
		pEntry = pEntry->pNext;
		free(pDel);
	}

	/* finally close our listen streams */
	for(i = 0 ; i < pThis->iLstnMax ; ++i) {
		netstrm.Destruct(pThis->ppLstn + i);
	}
}


/* add a listen socket to our listen socket array. This is a callback
 * invoked from the netstrm class. -- rgerhards, 2008-04-23
 */
static rsRetVal
addStrmLstn(void *pUsr, netstrm_t *pLstn)
{
	strmLstnPortList_t *pPortList = (strmLstnPortList_t *) pUsr;
	strmsrv_t *pThis = pPortList->pSrv;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strmsrv);
	ISOBJ_TYPE_assert(pLstn, netstrm);

	if(pThis->iLstnMax >= STRMLSTN_MAX_DEFAULT)
		ABORT_FINALIZE(RS_RET_MAX_LSTN_REACHED);

	pThis->ppLstn[pThis->iLstnMax] = pLstn;
	pThis->ppLstnPort[pThis->iLstnMax] = pPortList;
	++pThis->iLstnMax;

finalize_it:
	RETiRet;
}


/* Initialize STRM listener socket for a single port
 * rgerhards, 2009-05-21
 */
static rsRetVal
initSTRMListener(strmsrv_t *pThis, strmLstnPortList_t *pPortEntry)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strmsrv);
	assert(pPortEntry != NULL);

	/* TODO: add capability to specify local listen address! */
	CHKiRet(netstrm.LstnInit(pThis->pNS, (void*)pPortEntry, addStrmLstn, pPortEntry->pszPort,
		NULL, pThis->iSessMax, NULL));

finalize_it:
	RETiRet;
}


/* Initialize STRM sockets (for listener) and listens on them */
static rsRetVal
create_strm_socket(strmsrv_t *pThis)
{
	strmLstnPortList_t *pEntry;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strmsrv);

	/* init all configured ports */
	pEntry = pThis->pLstnPorts;
	while(pEntry != NULL) {
		CHKiRet(initSTRMListener(pThis, pEntry));
		pEntry = pEntry->pNext;
	}

	/* OK, we had success. Now it is also time to
	 * initialize our connections
	 */
	if(STRMSessTblInit(pThis) != 0) {
		/* OK, we are in some trouble - we could not initialize the
		 * session table, so we can not continue. We need to free all
		 * we have assigned so far, because we can not really use it...
		 */
		LogError(0, RS_RET_ERR, "Could not initialize STRM session table, suspending STRM "
				"message reception.");
		ABORT_FINALIZE(RS_RET_ERR);
	}

finalize_it:
	RETiRet;
}


/* Accept new STRM connection; make entry in session table. If there
 * is no more space left in the connection table, the new STRM
 * connection is immediately dropped.
 * ppSess has a pointer to the newly created session, if it succeeds.
 * If it does not succeed, no session is created and ppSess is
 * undefined. If the user has provided an OnSessAccept Callback,
 * this one is executed immediately after creation of the
 * session object, so that it can do its own initialization.
 * rgerhards, 2008-03-02
 */
static rsRetVal
SessAccept(strmsrv_t *pThis, strmLstnPortList_t *pLstnInfo, strms_sess_t **ppSess, netstrm_t *pStrm)
{
	DEFiRet;
	strms_sess_t *pSess = NULL;
	netstrm_t *pNewStrm = NULL;
	int iSess = -1;
	struct sockaddr_storage *addr;
	uchar *fromHostFQDN = NULL;
	prop_t *ip = NULL;

	ISOBJ_TYPE_assert(pThis, strmsrv);
	assert(pLstnInfo != NULL);

	CHKiRet(netstrm.AcceptConnReq(pStrm, &pNewStrm));

	/* Add to session list */
	iSess = STRMSessTblFindFreeSpot(pThis);
	if(iSess == -1) {
		errno = 0;
		LogError(0, RS_RET_MAX_SESS_REACHED, "too many strm sessions - dropping incoming request");
		ABORT_FINALIZE(RS_RET_MAX_SESS_REACHED);
	}

	if(pThis->bUseKeepAlive) {
		CHKiRet(netstrm.SetKeepAliveProbes(pNewStrm, pThis->iKeepAliveProbes));
		CHKiRet(netstrm.SetKeepAliveTime(pNewStrm, pThis->iKeepAliveTime));
		CHKiRet(netstrm.SetKeepAliveIntvl(pNewStrm, pThis->iKeepAliveIntvl));
		CHKiRet(netstrm.EnableKeepAlive(pNewStrm));
	}

	/* we found a free spot and can construct our session object */
	CHKiRet(strms_sess.Construct(&pSess));
	CHKiRet(strms_sess.SetStrmsrv(pSess, pThis));
	CHKiRet(strms_sess.SetLstnInfo(pSess, pLstnInfo));

	/* get the host name */
	CHKiRet(netstrm.GetRemoteHName(pNewStrm, &fromHostFQDN));
	CHKiRet(netstrm.GetRemoteIP(pNewStrm, &ip));
	CHKiRet(netstrm.GetRemAddr(pNewStrm, &addr));
	/* TODO: check if we need to strip the domain name here -- rgerhards, 2008-04-24 */

	/* Here we check if a host is permitted to send us messages. If it isn't, we do not further
	 * process the message but log a warning (if we are configured to do this).
	 * rgerhards, 2005-09-26
	 */
	if(pThis->pIsPermittedHost != NULL
	   && !pThis->pIsPermittedHost((struct sockaddr*) addr, (char*) fromHostFQDN, pThis->pUsr, pSess->pUsr)) {
		dbgprintf("%s is not an allowed sender\n", fromHostFQDN);
		if(glbl.GetOption_DisallowWarning()) {
			errno = 0;
			LogError(0, RS_RET_HOST_NOT_PERMITTED, "STRM message from disallowed "
					"sender %s discarded", fromHostFQDN);
		}
		ABORT_FINALIZE(RS_RET_HOST_NOT_PERMITTED);
	}

	/* OK, we have an allowed sender, so let's continue, what
	 * means we can finally fill in the session object.
	 */
	CHKiRet(strms_sess.SetHost(pSess, fromHostFQDN));
	fromHostFQDN = NULL; /* we handed this string over */
	CHKiRet(strms_sess.SetHostIP(pSess, ip));
	ip = NULL; /* we handed this string over */
	CHKiRet(strms_sess.SetStrm(pSess, pNewStrm));
	pNewStrm = NULL; /* prevent it from being freed in error handler, now done in strms_sess! */
	CHKiRet(strms_sess.ConstructFinalize(pSess));

	/* check if we need to call our callback */
	if(pThis->pOnSessAccept != NULL) {
		CHKiRet(pThis->pOnSessAccept(pThis, pSess));
	}

	*ppSess = pSess;
	pThis->pSessions[iSess] = pSess;
	pSess = NULL; /* this is now also handed over */

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pSess != NULL)
			strms_sess.Destruct(&pSess);
		if(pNewStrm != NULL)
			netstrm.Destruct(&pNewStrm);
		free(fromHostFQDN);
		if(ip != NULL)
			prop.Destruct(&ip);
	}

	RETiRet;
}


static void
RunCancelCleanup(void *arg)
{
	nssel_t **ppSel = (nssel_t**) arg;

	if(*ppSel != NULL)
		nssel.Destruct(ppSel);
}


/* This function is called to gather input. */
#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wempty-body"
#endif
static rsRetVal
Run(strmsrv_t *pThis)
{
	DEFiRet;
	int nfds;
	int i;
	int iSTRMSess;
	int bIsReady;
	strms_sess_t *pNewSess;
	nssel_t *pSel;
	ssize_t iRcvd;
	rsRetVal localRet;
	int oserr;

	ISOBJ_TYPE_assert(pThis, strmsrv);

	/* this is an endless loop - it is terminated by the framework canelling
	 * this thread. Thus, we also need to instantiate a cancel cleanup handler
	 * to prevent us from leaking anything. -- rgerharsd, 20080-04-24
	 */
	pthread_cleanup_push(RunCancelCleanup, (void*) &pSel);
	while(1) {
		CHKiRet(nssel.Construct(&pSel));
		// TODO: set driver
		CHKiRet(nssel.ConstructFinalize(pSel));

		/* Add the STRM listen sockets to the list of read descriptors. */
		for(i = 0 ; i < pThis->iLstnMax ; ++i) {
			CHKiRet(nssel.Add(pSel, pThis->ppLstn[i], NSDSEL_RD));
		}

		/* do the sessions */
		iSTRMSess = STRMSessGetNxtSess(pThis, -1);
		while(iSTRMSess != -1) {
			/* TODO: access to pNsd is NOT really CLEAN, use method... */
			CHKiRet(nssel.Add(pSel, pThis->pSessions[iSTRMSess]->pStrm, NSDSEL_RD));
			/* now get next... */
			iSTRMSess = STRMSessGetNxtSess(pThis, iSTRMSess);
		}

		/* wait for io to become ready */
		CHKiRet(nssel.Wait(pSel, &nfds));

		for(i = 0 ; i < pThis->iLstnMax ; ++i) {
			CHKiRet(nssel.IsReady(pSel, pThis->ppLstn[i], NSDSEL_RD, &bIsReady, &nfds));
			if(bIsReady) {
				dbgprintf("New connect on NSD %p.\n", pThis->ppLstn[i]);
				SessAccept(pThis, pThis->ppLstnPort[i], &pNewSess, pThis->ppLstn[i]);
				--nfds; /* indicate we have processed one */
			}
		}

		/* now check the sessions */
		iSTRMSess = STRMSessGetNxtSess(pThis, -1);
		while(nfds && iSTRMSess != -1) {
			CHKiRet(nssel.IsReady(pSel, pThis->pSessions[iSTRMSess]->pStrm, NSDSEL_RD, &bIsReady, &nfds));
			if(bIsReady) {
				char buf[8*1024]; /* reception buffer - may hold a partial or multiple messages */
				dbgprintf("netstream %p with new data\n", pThis->pSessions[iSTRMSess]->pStrm);

				/* Receive message */
				iRet = pThis->pRcvData(pThis->pSessions[iSTRMSess], buf, sizeof(buf), &iRcvd, &oserr);
				switch(iRet) {
				case RS_RET_CLOSED:
					pThis->pOnRegularClose(pThis->pSessions[iSTRMSess]);
					strms_sess.Destruct(&pThis->pSessions[iSTRMSess]);
					break;
				case RS_RET_RETRY:
					/* we simply ignore retry - this is not an error, but we also
					have not received anything */
					break;
				case RS_RET_OK:
					/* valid data received, process it! */
					localRet = strms_sess.DataRcvd(pThis->pSessions[iSTRMSess], buf, iRcvd);
					if(localRet != RS_RET_OK) {
						/* in this case, something went awfully wrong.
						 * We are instructed to terminate the session.
						 */
						LogError(0, localRet, "Tearing down STRM Session %d - see "
							    "previous messages for reason(s)\n", iSTRMSess);
						pThis->pOnErrClose(pThis->pSessions[iSTRMSess]);
						strms_sess.Destruct(&pThis->pSessions[iSTRMSess]);
					}
					break;
				default:
					LogError(oserr, iRet, "netstream session %p will be closed due to error\n",
						pThis->pSessions[iSTRMSess]->pStrm);
					pThis->pOnErrClose(pThis->pSessions[iSTRMSess]);
					strms_sess.Destruct(&pThis->pSessions[iSTRMSess]);
					break;
				}
				--nfds; /* indicate we have processed one */
			}
			iSTRMSess = STRMSessGetNxtSess(pThis, iSTRMSess);
		}
		CHKiRet(nssel.Destruct(&pSel));
finalize_it: /* this is a very special case - this time only we do not exit the function,
	      * because that would not help us either. So we simply retry it. Let's see
	      * if that actually is a better idea. Exiting the loop wasn't we always
	      * crashed, which made sense (the rest of the engine was not prepared for
	      * that) -- rgerhards, 2008-05-19
	      */
		/*EMPTY*/;
	}

	/* note that this point is usually not reached */
	pthread_cleanup_pop(0); /* remove cleanup handler */

	RETiRet;
}
#if !defined(_AIX)
#pragma GCC diagnostic warning "-Wempty-body"
#endif


/* Standard-Constructor */
BEGINobjConstruct(strmsrv) /* be sure to specify the object type also in END macro! */
	pThis->iSessMax = STRMSESS_MAX_DEFAULT; /* TODO: useful default ;) */
	/* set default callbacks (used if caller does not overwrite them) */
	pThis->pIsPermittedHost = isPermittedHost;
	pThis->OpenLstnSocks = doOpenLstnSocks;
	pThis->pRcvData = doRcvData;
	pThis->pOnRegularClose = onRegularClose;
	pThis->pOnErrClose = onErrClose;
	/* session specific callbacks */
	//pThis->OnSessConstructFinalize =
	//pThis->pOnSessDestruct =
ENDobjConstruct(strmsrv)


/* ConstructionFinalizer */
static rsRetVal
strmsrvConstructFinalize(strmsrv_t *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strmsrv);

	/* prepare network stream subsystem */
	CHKiRet(netstrms.Construct(&pThis->pNS));
	CHKiRet(netstrms.SetDrvrMode(pThis->pNS, pThis->iDrvrMode));
	if(pThis->pszDrvrAuthMode != NULL)
		CHKiRet(netstrms.SetDrvrAuthMode(pThis->pNS, pThis->pszDrvrAuthMode));
	if(pThis->pPermPeers != NULL)
		CHKiRet(netstrms.SetDrvrPermPeers(pThis->pNS, pThis->pPermPeers));
	// TODO: set driver!
	CHKiRet(netstrms.ConstructFinalize(pThis->pNS));

	/* set up listeners */
	CHKmalloc(pThis->ppLstn = calloc(STRMLSTN_MAX_DEFAULT, sizeof(netstrm_t*)));
	CHKmalloc(pThis->ppLstnPort = calloc(STRMLSTN_MAX_DEFAULT, sizeof(strmLstnPortList_t*)));
	iRet = pThis->OpenLstnSocks(pThis);

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pThis->pNS != NULL)
			netstrms.Destruct(&pThis->pNS);
	}
	RETiRet;
}


/* destructor for the strmsrv object */
BEGINobjDestruct(strmsrv) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(strmsrv)
	if(pThis->OnDestruct != NULL)
		pThis->OnDestruct(pThis->pUsr);

	deinit_strm_listener(pThis);

	if(pThis->pNS != NULL)
		netstrms.Destruct(&pThis->pNS);
	free(pThis->pszDrvrAuthMode);
	free(pThis->ppLstn);
	free(pThis->ppLstnPort);
	free(pThis->pszInputName);
ENDobjDestruct(strmsrv)


/* debugprint for the strmsrv object */
BEGINobjDebugPrint(strmsrv) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(strmsrv)
ENDobjDebugPrint(strmsrv)

/* set functions */
static rsRetVal
SetCBIsPermittedHost(strmsrv_t *pThis, int (*pCB)(struct sockaddr *addr, char *fromHostFQDN, void*, void*))
{
	DEFiRet;
	pThis->pIsPermittedHost = pCB;
	RETiRet;
}

static rsRetVal
SetCBOnSessAccept(strmsrv_t *pThis, rsRetVal (*pCB)(strmsrv_t*, strms_sess_t*))
{
	DEFiRet;
	pThis->pOnSessAccept = pCB;
	RETiRet;
}

static rsRetVal
SetCBOnDestruct(strmsrv_t *pThis, rsRetVal (*pCB)(void*))
{
	DEFiRet;
	pThis->OnDestruct = pCB;
	RETiRet;
}

static rsRetVal
SetCBOnSessConstructFinalize(strmsrv_t *pThis, rsRetVal (*pCB)(void*))
{
	DEFiRet;
	pThis->OnSessConstructFinalize = pCB;
	RETiRet;
}

static rsRetVal
SetCBOnSessDestruct(strmsrv_t *pThis, rsRetVal (*pCB)(void*))
{
	DEFiRet;
	pThis->pOnSessDestruct = pCB;
	RETiRet;
}

static rsRetVal
SetCBOnRegularClose(strmsrv_t *pThis, rsRetVal (*pCB)(strms_sess_t*))
{
	DEFiRet;
	pThis->pOnRegularClose = pCB;
	RETiRet;
}

static rsRetVal
SetCBOnErrClose(strmsrv_t *pThis, rsRetVal (*pCB)(strms_sess_t*))
{
	DEFiRet;
	pThis->pOnErrClose = pCB;
	RETiRet;
}

static rsRetVal
SetCBOpenLstnSocks(strmsrv_t *pThis, rsRetVal (*pCB)(strmsrv_t*))
{
	DEFiRet;
	pThis->OpenLstnSocks = pCB;
	RETiRet;
}

static rsRetVal
SetUsrP(strmsrv_t *pThis, void *pUsr)
{
	DEFiRet;
	pThis->pUsr = pUsr;
	RETiRet;
}

static rsRetVal
SetKeepAlive(strmsrv_t *pThis, int iVal)
{
	DEFiRet;
	dbgprintf("strmsrv: keep-alive set to %d\n", iVal);
	pThis->bUseKeepAlive = iVal;
	RETiRet;
}

static rsRetVal
SetKeepAliveIntvl(strmsrv_t *pThis, int iVal)
{
	DEFiRet;
	DBGPRINTF("strmsrv: keep-alive set to %d\n", iVal);
	pThis->iKeepAliveIntvl = iVal;
	RETiRet;
}

static rsRetVal
SetKeepAliveProbes(strmsrv_t *pThis, int iVal)
{
	DEFiRet;
	DBGPRINTF("strmsrv: keep-alive set to %d\n", iVal);
	pThis->iKeepAliveProbes = iVal;
	RETiRet;
}

static rsRetVal
SetKeepAliveTime(strmsrv_t *pThis, int iVal)
{
	DEFiRet;
	DBGPRINTF("strmsrv: keep-alive set to %d\n", iVal);
	pThis->iKeepAliveTime = iVal;
	RETiRet;
}

static rsRetVal
SetOnCharRcvd(strmsrv_t *pThis, rsRetVal (*OnCharRcvd)(strms_sess_t*, uchar))
{
	DEFiRet;
	assert(OnCharRcvd != NULL);
	pThis->OnCharRcvd = OnCharRcvd;
	RETiRet;
}

/* Set the input name to use -- rgerhards, 2008-12-10 */
static rsRetVal
SetInputName(strmsrv_t *pThis, uchar *name)
{
	uchar *pszName;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strmsrv);
	if(name == NULL)
		pszName = NULL;
	else
		CHKmalloc(pszName = ustrdup(name));
	free(pThis->pszInputName);
	pThis->pszInputName = pszName;
finalize_it:
	RETiRet;
}


/* here follows a number of methods that shuffle authentication settings down
 * to the drivers. Drivers not supporting these settings may return an error
 * state.
 * -------------------------------------------------------------------------- */

/* set the driver mode -- rgerhards, 2008-04-30 */
static rsRetVal
SetDrvrMode(strmsrv_t *pThis, int iMode)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strmsrv);
	pThis->iDrvrMode = iMode;
	RETiRet;
}


/* set the driver authentication mode -- rgerhards, 2008-05-19 */
static rsRetVal
SetDrvrAuthMode(strmsrv_t *pThis, uchar *mode)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strmsrv);
	CHKmalloc(pThis->pszDrvrAuthMode = ustrdup(mode));
finalize_it:
	RETiRet;
}


/* set the driver's permitted peers -- rgerhards, 2008-05-19 */
static rsRetVal
SetDrvrPermPeers(strmsrv_t *pThis, permittedPeers_t *pPermPeers)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strmsrv);
	pThis->pPermPeers = pPermPeers;
	RETiRet;
}


/* End of methods to shuffle autentication settings to the driver.;

 * -------------------------------------------------------------------------- */


/* set max number of sessions
 * this must be called before ConstructFinalize, or it will have no effect!
 * rgerhards, 2009-04-09
 */
static rsRetVal
SetSessMax(strmsrv_t *pThis, int iMax)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strmsrv);
	pThis->iSessMax = iMax;
	RETiRet;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(strmsrv)
CODESTARTobjQueryInterface(strmsrv)
	if(pIf->ifVersion != strmsrvCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->DebugPrint = strmsrvDebugPrint;
	pIf->Construct = strmsrvConstruct;
	pIf->ConstructFinalize = strmsrvConstructFinalize;
	pIf->Destruct = strmsrvDestruct;
	pIf->configureSTRMListen = configureSTRMListen;
	pIf->create_strm_socket = create_strm_socket;
	pIf->Run = Run;
	pIf->SetKeepAlive = SetKeepAlive;
	pIf->SetKeepAliveIntvl = SetKeepAliveIntvl;
	pIf->SetKeepAliveProbes = SetKeepAliveProbes;
	pIf->SetKeepAliveTime = SetKeepAliveTime;
	pIf->SetUsrP = SetUsrP;
	pIf->SetInputName = SetInputName;
	pIf->SetSessMax = SetSessMax;
	pIf->SetDrvrMode = SetDrvrMode;
	pIf->SetDrvrAuthMode = SetDrvrAuthMode;
	pIf->SetDrvrPermPeers = SetDrvrPermPeers;
	pIf->SetCBIsPermittedHost = SetCBIsPermittedHost;
	pIf->SetCBOpenLstnSocks = SetCBOpenLstnSocks;
	pIf->SetCBOnSessAccept = SetCBOnSessAccept;
	pIf->SetCBOnSessConstructFinalize = SetCBOnSessConstructFinalize;
	pIf->SetCBOnSessDestruct = SetCBOnSessDestruct;
	pIf->SetCBOnDestruct = SetCBOnDestruct;
	pIf->SetCBOnRegularClose = SetCBOnRegularClose;
	pIf->SetCBOnErrClose = SetCBOnErrClose;
	pIf->SetOnCharRcvd = SetOnCharRcvd;

finalize_it:
ENDobjQueryInterface(strmsrv)


/* exit our class
 * rgerhards, 2008-03-10
 */
BEGINObjClassExit(strmsrv, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(strmsrv)
	/* release objects we no longer need */
	objRelease(strms_sess, DONT_LOAD_LIB);
	objRelease(conf, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(netstrms, DONT_LOAD_LIB);
	objRelease(nssel, DONT_LOAD_LIB);
	objRelease(netstrm, LM_NETSTRMS_FILENAME);
	objRelease(net, LM_NET_FILENAME);
ENDObjClassExit(strmsrv)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-29
 */
BEGINObjClassInit(strmsrv, 1, OBJ_IS_LOADABLE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	/* request objects we use */
	CHKiRet(objUse(net, LM_NET_FILENAME));
	CHKiRet(objUse(netstrms, LM_NETSTRMS_FILENAME));
	CHKiRet(objUse(netstrm, DONT_LOAD_LIB));
	CHKiRet(objUse(nssel, DONT_LOAD_LIB));
	CHKiRet(objUse(strms_sess, DONT_LOAD_LIB));
	CHKiRet(objUse(conf, CORE_COMPONENT));
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, strmsrvDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, strmsrvConstructFinalize);
ENDObjClassInit(strmsrv)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
	/* de-init in reverse order! */
	strmsrvClassExit();
	strms_sessClassExit();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(strms_sessClassInit(pModInfo));
	CHKiRet(strmsrvClassInit(pModInfo)); /* must be done after strms_sess, as we use it */
ENDmodInit

/* vim:set ai:
 */
