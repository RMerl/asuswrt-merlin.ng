/* tcps_sess.c
 *
 * This implements a session of the tcpsrv object. For general
 * comments, see header of tcpsrv.c.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2008-03-01 by RGerhards (extracted from tcpsrv.c, which
 * based on the BSD-licensed syslogd.c)
 *
 * Copyright 2007-2012 Adiscon GmbH.
 *
 * This file is part of rsyslog.
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
#include "unicode-helper.h"
#include "module-template.h"
#include "net.h"
#include "tcpsrv.h"
#include "tcps_sess.h"
#include "obj.h"
#include "errmsg.h"
#include "netstrm.h"
#include "msg.h"
#include "datetime.h"
#include "prop.h"
#include "ratelimit.h"
#include "debug.h"


/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(netstrm)
DEFobjCurrIf(prop)
DEFobjCurrIf(datetime)


/* forward definitions */
static rsRetVal Close(tcps_sess_t *pThis);


/* Standard-Constructor */
BEGINobjConstruct(tcps_sess) /* be sure to specify the object type also in END macro! */
		pThis->iMsg = 0; /* just make sure... */
		pThis->inputState = eAtStrtFram; /* indicate frame header expected */
		pThis->eFraming = TCP_FRAMING_OCTET_STUFFING; /* just make sure... */
		/* now allocate the message reception buffer */
		CHKmalloc(pThis->pMsg = (uchar*) MALLOC(glbl.GetMaxLine() + 1));
finalize_it:
ENDobjConstruct(tcps_sess)


/* ConstructionFinalizer
 */
static rsRetVal
tcps_sessConstructFinalize(tcps_sess_t __attribute__((unused)) *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	if(pThis->pSrv->OnSessConstructFinalize != NULL) {
		CHKiRet(pThis->pSrv->OnSessConstructFinalize(&pThis->pUsr));
	}

finalize_it:
	RETiRet;
}


/* destructor for the tcps_sess object */
BEGINobjDestruct(tcps_sess) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(tcps_sess)
//printf("sess %p destruct, pStrm %p\n", pThis, pThis->pStrm);
	if(pThis->pStrm != NULL)
		netstrm.Destruct(&pThis->pStrm);

	if(pThis->pSrv->pOnSessDestruct != NULL) {
		pThis->pSrv->pOnSessDestruct(&pThis->pUsr);
	}
	/* now destruct our own properties */
	if(pThis->fromHost != NULL)
		CHKiRet(prop.Destruct(&pThis->fromHost));
	if(pThis->fromHostIP != NULL)
		CHKiRet(prop.Destruct(&pThis->fromHostIP));
	free(pThis->pMsg);
ENDobjDestruct(tcps_sess)


/* debugprint for the tcps_sess object */
BEGINobjDebugPrint(tcps_sess) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(tcps_sess)
ENDobjDebugPrint(tcps_sess)


/* set property functions */
/* set the hostname. Note that the caller *hands over* the string. That is,
 * the caller no longer controls it once SetHost() has received it. Most importantly,
 * the caller must not free it. -- rgerhards, 2008-04-24
 */
static rsRetVal
SetHost(tcps_sess_t *pThis, uchar *pszHost)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, tcps_sess);

	if(pThis->fromHost == NULL)
		CHKiRet(prop.Construct(&pThis->fromHost));

	CHKiRet(prop.SetString(pThis->fromHost, pszHost, ustrlen(pszHost)));

finalize_it:
	free(pszHost); /* we must free according to our (old) calling conventions */
	RETiRet;
}

/* set the remote host's IP. Note that the caller *hands over* the property. That is,
 * the caller no longer controls it once SetHostIP() has received it. Most importantly,
 * the caller must not destruct it. -- rgerhards, 2008-05-16
 */
static rsRetVal
SetHostIP(tcps_sess_t *pThis, prop_t *ip)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);

	if(pThis->fromHostIP != NULL) {
		prop.Destruct(&pThis->fromHostIP);
	}
	pThis->fromHostIP = ip;
	RETiRet;
}

static rsRetVal
SetStrm(tcps_sess_t *pThis, netstrm_t *pStrm)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	pThis->pStrm = pStrm;
	RETiRet;
}


static rsRetVal
SetMsgIdx(tcps_sess_t *pThis, int idx)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	pThis->iMsg = idx;
	RETiRet;
}


/* set our parent, the tcpsrv object */
static rsRetVal
SetTcpsrv(tcps_sess_t *pThis, tcpsrv_t *pSrv)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	ISOBJ_TYPE_assert(pSrv, tcpsrv);
	pThis->pSrv = pSrv;
	RETiRet;
}


/* set our parent listener info*/
static rsRetVal
SetLstnInfo(tcps_sess_t *pThis, tcpLstnPortList_t *pLstnInfo)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	assert(pLstnInfo != NULL);
	pThis->pLstnInfo = pLstnInfo;
	/* set cached elements */
	pThis->bSuppOctetFram = pLstnInfo->bSuppOctetFram;
	pThis->bSPFramingFix = pLstnInfo->bSPFramingFix;
	RETiRet;
}


static rsRetVal
SetUsrP(tcps_sess_t *pThis, void *pUsr)
{
	DEFiRet;
	pThis->pUsr = pUsr;
	RETiRet;
}


static rsRetVal
SetOnMsgReceive(tcps_sess_t *pThis, rsRetVal (*OnMsgReceive)(tcps_sess_t*, uchar*, int))
{
	DEFiRet;
	pThis->DoSubmitMessage = OnMsgReceive;
	RETiRet;
}


/* This is a helper for submitting the message to the rsyslog core.
 * It does some common processing, including resetting the various
 * state variables to a "processed" state.
 * Note that this function is also called if we had a buffer overflow
 * due to a too-long message. So far, there is no indication this
 * happened and it may be worth thinking about different handling
 * of this case (what obviously would require a change to this
 * function or some related code).
 * rgerhards, 2009-04-23
 */
static rsRetVal
defaultDoSubmitMessage(tcps_sess_t *pThis, struct syslogTime *stTime, time_t ttGenTime, multi_submit_t *pMultiSub)
{
	smsg_t *pMsg;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, tcps_sess);
	
	if(pThis->iMsg == 0) {
		DBGPRINTF("discarding zero-sized message\n");
		FINALIZE;
	}

	if(pThis->DoSubmitMessage != NULL) {
		pThis->DoSubmitMessage(pThis, pThis->pMsg, pThis->iMsg);
		FINALIZE;
	}

	/* we now create our own message object and submit it to the queue */
	CHKiRet(msgConstructWithTime(&pMsg, stTime, ttGenTime));
	MsgSetRawMsg(pMsg, (char*)pThis->pMsg, pThis->iMsg);
	MsgSetInputName(pMsg, pThis->pLstnInfo->pInputName);
	if(pThis->pLstnInfo->dfltTZ[0] != '\0')
		MsgSetDfltTZ(pMsg, (char*) pThis->pLstnInfo->dfltTZ);
	MsgSetFlowControlType(pMsg, pThis->pSrv->bUseFlowControl
			            ? eFLOWCTL_LIGHT_DELAY : eFLOWCTL_NO_DELAY);
	pMsg->msgFlags  = NEEDS_PARSING | PARSE_HOSTNAME;
	MsgSetRcvFrom(pMsg, pThis->fromHost);
	CHKiRet(MsgSetRcvFromIP(pMsg, pThis->fromHostIP));
	MsgSetRuleset(pMsg, pThis->pLstnInfo->pRuleset);

	STATSCOUNTER_INC(pThis->pLstnInfo->ctrSubmit, pThis->pLstnInfo->mutCtrSubmit);
	ratelimitAddMsg(pThis->pLstnInfo->ratelimiter, pMultiSub, pMsg);

finalize_it:
	/* reset status variables */
	pThis->iMsg = 0;

	RETiRet;
}



/* This should be called before a normal (non forced) close
 * of a TCP session. This function checks if there is any unprocessed
 * message left in the TCP stream. Such a message is probably a
 * fragement. If evrything goes well, we must be right at the
 * beginnig of a new frame without any data received from it. If
 * not, there is some kind of a framing error. I think I remember that
 * some legacy syslog/TCP implementations have non-LF terminated
 * messages at the end of the stream. For now, we allow this behaviour.
 * Later, it should probably become a configuration option.
 * rgerhards, 2006-12-07
 */
static rsRetVal
PrepareClose(tcps_sess_t *pThis)
{
	struct syslogTime stTime;
	time_t ttGenTime;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, tcps_sess);
	
	if(pThis->inputState == eAtStrtFram) {
		/* this is how it should be. There is no unprocessed
		 * data left and such we have nothing to do. For simplicity
		 * reasons, we immediately return in that case.
		 */
		 FINALIZE;
	}

	/* we have some data left! */
	if(pThis->eFraming == TCP_FRAMING_OCTET_COUNTING) {
		/* In this case, we have an invalid frame count and thus
		 * generate an error message and discard the frame.
		 */
		LogError(0, NO_ERRCODE, "Incomplete frame at end of stream in session %p - "
			    "ignoring extra data (a message may be lost).", pThis->pStrm);
		/* nothing more to do */
	} else { /* here, we have traditional framing. Missing LF at the end
		 * of message may occur. As such, we process the message in
		 * this case.
		 */
		DBGPRINTF("Extra data at end of stream in legacy syslog/tcp message - processing\n");
		datetime.getCurrTime(&stTime, &ttGenTime, TIME_IN_LOCALTIME);
		defaultDoSubmitMessage(pThis, &stTime, ttGenTime, NULL);
	}

finalize_it:
	RETiRet;
}


/* Closes a TCP session
 * No attention is paid to the return code
 * of close, so potential-double closes are not detected.
 */
static rsRetVal
Close(tcps_sess_t *pThis)
{
	DEFiRet;

//printf("sess %p close\n", pThis);
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	netstrm.Destruct(&pThis->pStrm);
	if(pThis->fromHost != NULL) {
		prop.Destruct(&pThis->fromHost);
	}
	if(pThis->fromHostIP != NULL)
		prop.Destruct(&pThis->fromHostIP);

	RETiRet;
}


/* process the data received. As TCP is stream based, we need to process the
 * data inside a state machine. The actual data received is passed in byte-by-byte
 * from DataRcvd, and this function here compiles messages from them and submits
 * the end result to the queue. Introducing this function fixes a long-term bug ;)
 * rgerhards, 2008-03-14
 */
static rsRetVal
processDataRcvd(tcps_sess_t *pThis,
	char c,
	struct syslogTime *stTime,
	const time_t ttGenTime,
	multi_submit_t *pMultiSub,
	unsigned *const __restrict__ pnMsgs)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, tcps_sess);
	int iMaxLine = glbl.GetMaxLine();
	uchar *propPeerName = NULL;
	int lenPeerName = 0;
	uchar *propPeerIP = NULL;
	int lenPeerIP = 0;

	if(pThis->inputState == eAtStrtFram) {
		if(pThis->bSuppOctetFram && c >= '0' && c <= '9') {
			pThis->inputState = eInOctetCnt;
			pThis->iOctetsRemain = 0;
			pThis->eFraming = TCP_FRAMING_OCTET_COUNTING;
		} else if(pThis->bSPFramingFix && c == ' ') {
			/* Cisco ASA very occasionally sends a SP after a LF, which
			 * thrashes framing if not taken special care of. Here,
			 * we permit space *in front of the next frame* and
			 * ignore it.
			 */
			 FINALIZE;
		} else {
			pThis->inputState = eInMsg;
			pThis->eFraming = TCP_FRAMING_OCTET_STUFFING;
		}
	}

	if(pThis->inputState == eInOctetCnt) {
		if(c >= '0' && c <= '9') { /* isdigit() the faster way */
			if(pThis->iOctetsRemain <= 200000000) {
				pThis->iOctetsRemain = pThis->iOctetsRemain * 10 + c - '0';
			}
			*(pThis->pMsg + pThis->iMsg++) = c;
		} else { /* done with the octet count, so this must be the SP terminator */
			DBGPRINTF("TCP Message with octet-counter, size %d.\n", pThis->iOctetsRemain);
			prop.GetString(pThis->fromHost, &propPeerName, &lenPeerName);
			prop.GetString(pThis->fromHost, &propPeerIP, &lenPeerIP);
			if(c != ' ') {
				LogError(0, NO_ERRCODE, "imtcp %s: Framing Error in received TCP message from "
					"peer: (hostname) %s, (ip) %s: delimiter is not SP but has "
					"ASCII value %d.", pThis->pSrv->pszInputName, propPeerName, propPeerIP, c);
			}
			if(pThis->iOctetsRemain < 1) {
				/* TODO: handle the case where the octet count is 0! */
				LogError(0, NO_ERRCODE, "imtcp %s: Framing Error in received TCP message from "
					"peer: (hostname) %s, (ip) %s: invalid octet count %d.",
					pThis->pSrv->pszInputName, propPeerName, propPeerIP, pThis->iOctetsRemain);
				pThis->eFraming = TCP_FRAMING_OCTET_STUFFING;
			} else if(pThis->iOctetsRemain > iMaxLine) {
				/* while we can not do anything against it, we can at least log an indication
				 * that something went wrong) -- rgerhards, 2008-03-14
				 */
				LogError(0, NO_ERRCODE, "imtcp %s: received oversize message from peer: "
					"(hostname) %s, (ip) %s: size is %d bytes, max msg size "
					"is %d, truncating...", pThis->pSrv->pszInputName, propPeerName,
					propPeerIP, pThis->iOctetsRemain, iMaxLine);
			}
			if(pThis->iOctetsRemain > pThis->pSrv->maxFrameSize) {
				LogError(0, NO_ERRCODE, "imtcp %s: Framing Error in received TCP message from "
					"peer: (hostname) %s, (ip) %s: frame too large: %d, change "
					"to octet stuffing", pThis->pSrv->pszInputName, propPeerName, propPeerIP,
						pThis->iOctetsRemain);
				pThis->eFraming = TCP_FRAMING_OCTET_STUFFING;
			} else {
				pThis->iMsg = 0;
			}
			pThis->inputState = eInMsg;
		}
	} else if(pThis->inputState == eInMsgTruncating) {
		if((   ((c == '\n') && !pThis->pSrv->bDisableLFDelim)
		   || ((pThis->pSrv->addtlFrameDelim != TCPSRV_NO_ADDTL_DELIMITER)
		        && (c == pThis->pSrv->addtlFrameDelim))
		   ) && pThis->eFraming == TCP_FRAMING_OCTET_STUFFING) {
			pThis->inputState = eAtStrtFram;
		}
	} else {
		assert(pThis->inputState == eInMsg);
		if(pThis->iMsg >= iMaxLine) {
			/* emergency, we now need to flush, no matter if we are at end of message or not... */
			DBGPRINTF("error: message received is larger than max msg size, we split it\n");
			defaultDoSubmitMessage(pThis, stTime, ttGenTime, pMultiSub);
			++(*pnMsgs);
			if(pThis->pSrv->discardTruncatedMsg == 1) {
				pThis->inputState = eInMsgTruncating;
			}
			/* configuration parameter discardTruncatedMsg controlls
			 * if rest of message is being processed
			 * 0 = off
			 * 1 = on
			 * Pascal Withopf, 2017-04-21
			 */
		}

		if((   ((c == '\n') && !pThis->pSrv->bDisableLFDelim)
		   || ((pThis->pSrv->addtlFrameDelim != TCPSRV_NO_ADDTL_DELIMITER)
		        && (c == pThis->pSrv->addtlFrameDelim))
		   ) && pThis->eFraming == TCP_FRAMING_OCTET_STUFFING) { /* record delimiter? */
			defaultDoSubmitMessage(pThis, stTime, ttGenTime, pMultiSub);
			++(*pnMsgs);
			pThis->inputState = eAtStrtFram;
		} else {
			/* IMPORTANT: here we copy the actual frame content to the message - for BOTH framing modes!
			 * If we have a message that is larger than the max msg size, we truncate it. This is the best
			 * we can do in light of what the engine supports. -- rgerhards, 2008-03-14
			 */
			if(pThis->iMsg < iMaxLine) {
				*(pThis->pMsg + pThis->iMsg++) = c;
			}
		}

		if(pThis->eFraming == TCP_FRAMING_OCTET_COUNTING) {
			/* do we need to find end-of-frame via octet counting? */
			pThis->iOctetsRemain--;
			if(pThis->iOctetsRemain < 1) {
				/* we have end of frame! */
				defaultDoSubmitMessage(pThis, stTime, ttGenTime, pMultiSub);
				++(*pnMsgs);
				pThis->inputState = eAtStrtFram;
			}
		}
	}

finalize_it:
	RETiRet;
}


/* Processes the data received via a TCP session. If there
 * is no other way to handle it, data is discarded.
 * Input parameter data is the data received, iLen is its
 * len as returned from recv(). iLen must be 1 or more (that
 * is errors must be handled by caller!). iTCPSess must be
 * the index of the TCP session that received the data.
 * rgerhards 2005-07-04
 * And another change while generalizing. We now return either
 * RS_RET_OK, which means the session should be kept open
 * or anything else, which means it must be closed.
 * rgerhards, 2008-03-01
 * As a performance optimization, we pick up the timestamp here. Acutally,
 * this *is* the *correct* reception step for all the data we received, because
 * we have just received a bunch of data! -- rgerhards, 2009-06-16
 */
#define NUM_MULTISUB 1024
static rsRetVal
DataRcvd(tcps_sess_t *pThis, char *pData, const size_t iLen)
{
	multi_submit_t multiSub;
	smsg_t *pMsgs[NUM_MULTISUB];
	struct syslogTime stTime;
	time_t ttGenTime;
	char *pEnd;
	unsigned nMsgs = 0;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, tcps_sess);
	assert(pData != NULL);
	assert(iLen > 0);

	datetime.getCurrTime(&stTime, &ttGenTime, TIME_IN_LOCALTIME);
	multiSub.ppMsgs = pMsgs;
	multiSub.maxElem = NUM_MULTISUB;
	multiSub.nElem = 0;

	 /* We now copy the message to the session buffer. */
	pEnd = pData + iLen; /* this is one off, which is intensional */

	while(pData < pEnd) {
		CHKiRet(processDataRcvd(pThis, *pData++, &stTime, ttGenTime, &multiSub, &nMsgs));
	}
	iRet = multiSubmitFlush(&multiSub);

	if(glblSenderKeepTrack)
		statsRecordSender(propGetSzStr(pThis->fromHost), nMsgs, ttGenTime);

finalize_it:
	RETiRet;
}
#undef NUM_MULTISUB


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(tcps_sess)
CODESTARTobjQueryInterface(tcps_sess)
	if(pIf->ifVersion != tcps_sessCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->DebugPrint = tcps_sessDebugPrint;
	pIf->Construct = tcps_sessConstruct;
	pIf->ConstructFinalize = tcps_sessConstructFinalize;
	pIf->Destruct = tcps_sessDestruct;

	pIf->PrepareClose = PrepareClose;
	pIf->Close = Close;
	pIf->DataRcvd = DataRcvd;

	pIf->SetUsrP = SetUsrP;
	pIf->SetTcpsrv = SetTcpsrv;
	pIf->SetLstnInfo = SetLstnInfo;
	pIf->SetHost = SetHost;
	pIf->SetHostIP = SetHostIP;
	pIf->SetStrm = SetStrm;
	pIf->SetMsgIdx = SetMsgIdx;
	pIf->SetOnMsgReceive = SetOnMsgReceive;
finalize_it:
ENDobjQueryInterface(tcps_sess)


/* exit our class
 * rgerhards, 2008-03-10
 */
BEGINObjClassExit(tcps_sess, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(tcps_sess)
	/* release objects we no longer need */
	objRelease(netstrm, LM_NETSTRMS_FILENAME);
	objRelease(datetime, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
ENDObjClassExit(tcps_sess)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-29
 */
BEGINObjClassInit(tcps_sess, 1, OBJ_IS_CORE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	/* request objects we use */
	CHKiRet(objUse(netstrm, LM_NETSTRMS_FILENAME));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	CHKiRet(objUse(glbl, CORE_COMPONENT));
	objRelease(glbl, CORE_COMPONENT);

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, tcps_sessDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, tcps_sessConstructFinalize);
ENDObjClassInit(tcps_sess)

/* vim:set ai:
 */
