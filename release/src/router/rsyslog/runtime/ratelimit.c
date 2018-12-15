/* ratelimit.c
 * support for rate-limiting sources, including "last message
 * repeated n times" processing.
 *
 * Copyright 2012-2016 Rainer Gerhards and Adiscon GmbH.
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

#include "rsyslog.h"
#include "errmsg.h"
#include "ratelimit.h"
#include "datetime.h"
#include "parser.h"
#include "unicode-helper.h"
#include "msg.h"
#include "rsconf.h"
#include "dirty.h"

/* definitions for objects we access */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(datetime)
DEFobjCurrIf(parser)

/* static data */

/* generate a "repeated n times" message */
static smsg_t *
ratelimitGenRepMsg(ratelimit_t *ratelimit)
{
	smsg_t *repMsg;
	size_t lenRepMsg;
	uchar szRepMsg[1024];

	if(ratelimit->nsupp == 1) { /* we simply use the original message! */
		repMsg = MsgAddRef(ratelimit->pMsg);
	} else {/* we need to duplicate, original message may still be in use in other
		 * parts of the system!  */
		if((repMsg = MsgDup(ratelimit->pMsg)) == NULL) {
			DBGPRINTF("Message duplication failed, dropping repeat message.\n");
			goto done;
		}
		lenRepMsg = snprintf((char*)szRepMsg, sizeof(szRepMsg),
					" message repeated %d times: [%.800s]",
					ratelimit->nsupp, getMSG(ratelimit->pMsg));
		MsgReplaceMSG(repMsg, szRepMsg, lenRepMsg);
	}

done:	return repMsg;
}

static rsRetVal
doLastMessageRepeatedNTimes(ratelimit_t *ratelimit, smsg_t *pMsg, smsg_t **ppRepMsg)
{
	int bNeedUnlockMutex = 0;
	DEFiRet;

	if(ratelimit->bThreadSafe) {
		pthread_mutex_lock(&ratelimit->mut);
		bNeedUnlockMutex = 1;
	}

	if( ratelimit->pMsg != NULL &&
	    getMSGLen(pMsg) == getMSGLen(ratelimit->pMsg) &&
	    !ustrcmp(getMSG(pMsg), getMSG(ratelimit->pMsg)) &&
	    !strcmp(getHOSTNAME(pMsg), getHOSTNAME(ratelimit->pMsg)) &&
	    !strcmp(getPROCID(pMsg, LOCK_MUTEX), getPROCID(ratelimit->pMsg, LOCK_MUTEX)) &&
	    !strcmp(getAPPNAME(pMsg, LOCK_MUTEX), getAPPNAME(ratelimit->pMsg, LOCK_MUTEX))) {
		ratelimit->nsupp++;
		DBGPRINTF("msg repeated %d times\n", ratelimit->nsupp);
		/* use current message, so we have the new timestamp
		 * (means we need to discard previous one) */
		msgDestruct(&ratelimit->pMsg);
		ratelimit->pMsg = pMsg;
		ABORT_FINALIZE(RS_RET_DISCARDMSG);
	} else {/* new message, do "repeat processing" & save it */
		if(ratelimit->pMsg != NULL) {
			if(ratelimit->nsupp > 0) {
				*ppRepMsg = ratelimitGenRepMsg(ratelimit);
				ratelimit->nsupp = 0;
			}
			msgDestruct(&ratelimit->pMsg);
		}
		ratelimit->pMsg = MsgAddRef(pMsg);
	}

finalize_it:
	if(bNeedUnlockMutex)
		pthread_mutex_unlock(&ratelimit->mut);
	RETiRet;
}


/* helper: tell how many messages we lost due to linux-like ratelimiting */
static void
tellLostCnt(ratelimit_t *ratelimit)
{
	uchar msgbuf[1024];
	if(ratelimit->missed) {
		snprintf((char*)msgbuf, sizeof(msgbuf),
			 "%s: %u messages lost due to rate-limiting",
			 ratelimit->name, ratelimit->missed);
		ratelimit->missed = 0;
		logmsgInternal(RS_RET_RATE_LIMITED, LOG_SYSLOG|LOG_INFO, msgbuf, 0);
	}
}

/* Linux-like ratelimiting, modelled after the linux kernel
 * returns 1 if message is within rate limit and shall be
 * processed, 0 otherwise.
 * This implementation is NOT THREAD-SAFE and must not
 * be called concurrently.
 */
static int ATTR_NONNULL()
withinRatelimit(ratelimit_t *__restrict__ const ratelimit,
	time_t tt,
	const char*const appname)
{
	int ret;
	uchar msgbuf[1024];

	if(ratelimit->bThreadSafe) {
		pthread_mutex_lock(&ratelimit->mut);
	}

	if(ratelimit->interval == 0) {
		ret = 1;
		goto finalize_it;
	}

	/* we primarily need "NoTimeCache" mode for imjournal, as it
	 * sets the message generation time to the journal timestamp.
	 * As such, we do not get a proper indication of the actual
	 * message rate. To prevent this, we need to query local
	 * system time ourselvs.
	 */
	if(ratelimit->bNoTimeCache)
		tt = time(NULL);

	assert(ratelimit->burst != 0);

	if(ratelimit->begin == 0)
		ratelimit->begin = tt;

	/* resume if we go out of time window or if time has gone backwards */
	if((tt > ratelimit->begin + ratelimit->interval) || (tt < ratelimit->begin) ) {
		ratelimit->begin = 0;
		ratelimit->done = 0;
		tellLostCnt(ratelimit);
	}

	/* do actual limit check */
	if(ratelimit->burst > ratelimit->done) {
		ratelimit->done++;
		ret = 1;
	} else {
		ratelimit->missed++;
		if(ratelimit->missed == 1) {
			snprintf((char*)msgbuf, sizeof(msgbuf),
				"%s from <%s>: begin to drop messages due to rate-limiting",
				ratelimit->name, appname);
			logmsgInternal(RS_RET_RATE_LIMITED, LOG_SYSLOG|LOG_INFO, msgbuf, 0);
		}
		ret = 0;
	}

finalize_it:
	if(ratelimit->bThreadSafe) {
		pthread_mutex_unlock(&ratelimit->mut);
	}
	return ret;
}


/* ratelimit a message, that means:
 * - handle "last message repeated n times" logic
 * - handle actual (discarding) rate-limiting
 * This function returns RS_RET_OK, if the caller shall process
 * the message regularly and RS_RET_DISCARD if the caller must
 * discard the message. The caller should also discard the message
 * if another return status occurs. This places some burden on the
 * caller logic, but provides best performance. Demanding this
 * cooperative mode can enable a faulty caller to thrash up part
 * of the system, but we accept that risk (a faulty caller can
 * always do all sorts of evil, so...)
 * If *ppRepMsg != NULL on return, the caller must enqueue that
 * message before the original message.
 */
rsRetVal
ratelimitMsg(ratelimit_t *__restrict__ const ratelimit, smsg_t *pMsg, smsg_t **ppRepMsg)
{
	DEFiRet;
	rsRetVal localRet;

	*ppRepMsg = NULL;

	if((pMsg->msgFlags & NEEDS_PARSING) != 0) {
		if((localRet = parser.ParseMsg(pMsg)) != RS_RET_OK)  {
			DBGPRINTF("Message discarded, parsing error %d\n", localRet);
			ABORT_FINALIZE(RS_RET_DISCARDMSG);
		}
	}

	/* Only the messages having severity level at or below the
	 * treshold (the value is >=) are subject to ratelimiting. */
	if(ratelimit->interval && (pMsg->iSeverity >= ratelimit->severity)) {
		char namebuf[512]; /* 256 for FGDN adn 256 for APPNAME should be enough */
		snprintf(namebuf, sizeof namebuf, "%s:%s", getHOSTNAME(pMsg),
			getAPPNAME(pMsg, 0));
		if(withinRatelimit(ratelimit, pMsg->ttGenTime, namebuf) == 0) {
			msgDestruct(&pMsg);
			ABORT_FINALIZE(RS_RET_DISCARDMSG);
		}
	}
	if(ratelimit->bReduceRepeatMsgs) {
		CHKiRet(doLastMessageRepeatedNTimes(ratelimit, pMsg, ppRepMsg));
	}
finalize_it:
	if(Debug) {
		if(iRet == RS_RET_DISCARDMSG)
			DBGPRINTF("message discarded by ratelimiting\n");
	}
	RETiRet;
}

/* returns 1, if the ratelimiter performs any checks and 0 otherwise */
int
ratelimitChecked(ratelimit_t *ratelimit)
{
	return ratelimit->interval || ratelimit->bReduceRepeatMsgs;
}


/* add a message to a ratelimiter/multisubmit structure.
 * ratelimiting is automatically handled according to the ratelimit
 * settings.
 * if pMultiSub == NULL, a single-message enqueue happens (under reconsideration)
 */
rsRetVal
ratelimitAddMsg(ratelimit_t *ratelimit, multi_submit_t *pMultiSub, smsg_t *pMsg)
{
	rsRetVal localRet;
	smsg_t *repMsg;
	DEFiRet;

	localRet = ratelimitMsg(ratelimit, pMsg, &repMsg);
	if(pMultiSub == NULL) {
		if(repMsg != NULL)
			CHKiRet(submitMsg2(repMsg));
		CHKiRet(localRet);
		CHKiRet(submitMsg2(pMsg));
	} else {
		if(repMsg != NULL) {
			pMultiSub->ppMsgs[pMultiSub->nElem++] = repMsg;
			if(pMultiSub->nElem == pMultiSub->maxElem)
				CHKiRet(multiSubmitMsg2(pMultiSub));
		}
		CHKiRet(localRet);
		if(pMsg->iLenRawMsg > glblGetMaxLine()) {
			/* oversize message needs special processing. We keep
			 * at least the previous batch as batch...
			 */
			if(pMultiSub->nElem > 0) {
				CHKiRet(multiSubmitMsg2(pMultiSub));
			}
			CHKiRet(submitMsg2(pMsg));
			FINALIZE;
		}
		pMultiSub->ppMsgs[pMultiSub->nElem++] = pMsg;
		if(pMultiSub->nElem == pMultiSub->maxElem)
			CHKiRet(multiSubmitMsg2(pMultiSub));
	}

finalize_it:
	RETiRet;
}


/* modname must be a static name (usually expected to be the module
 * name and MUST be present. dynname may be NULL and can be used for
 * dynamic information, e.g. PID or listener IP, ...
 * Both values should be kept brief.
 */
rsRetVal
ratelimitNew(ratelimit_t **ppThis, const char *modname, const char *dynname)
{
	ratelimit_t *pThis;
	char namebuf[256];
	DEFiRet;

	CHKmalloc(pThis = calloc(1, sizeof(ratelimit_t)));
	if(modname == NULL)
		modname ="*ERROR:MODULE NAME MISSING*";

	if(dynname == NULL) {
		pThis->name = strdup(modname);
	} else {
		snprintf(namebuf, sizeof(namebuf), "%s[%s]",
			 modname, dynname);
		namebuf[sizeof(namebuf)-1] = '\0'; /* to be on safe side */
		pThis->name = strdup(namebuf);
	}
	/* pThis->severity == 0 - all messages are ratelimited */
	pThis->bReduceRepeatMsgs = loadConf->globals.bReduceRepeatMsgs;
	DBGPRINTF("ratelimit:%s:new ratelimiter:bReduceRepeatMsgs %d\n",
		  pThis->name, pThis->bReduceRepeatMsgs);
	*ppThis = pThis;
finalize_it:
	RETiRet;
}


/* enable linux-like ratelimiting */
void
ratelimitSetLinuxLike(ratelimit_t *ratelimit, unsigned short interval, unsigned burst)
{
	ratelimit->interval = interval;
	ratelimit->burst = burst;
	ratelimit->done = 0;
	ratelimit->missed = 0;
	ratelimit->begin = 0;
}


/* enable thread-safe operations mode. This make sure that
 * a single ratelimiter can be called from multiple threads. As
 * this causes some overhead and is not always required, it needs
 * to be explicitely enabled. This operation cannot be undone
 * (think: why should one do that???)
 */
void
ratelimitSetThreadSafe(ratelimit_t *ratelimit)
{
	ratelimit->bThreadSafe = 1;
	pthread_mutex_init(&ratelimit->mut, NULL);
}
void
ratelimitSetNoTimeCache(ratelimit_t *ratelimit)
{
	ratelimit->bNoTimeCache = 1;
	pthread_mutex_init(&ratelimit->mut, NULL);
}

/* Severity level determines which messages are subject to
 * ratelimiting. Default (no value set) is all messages.
 */
void
ratelimitSetSeverity(ratelimit_t *ratelimit, intTiny severity)
{
	ratelimit->severity = severity;
}

void
ratelimitDestruct(ratelimit_t *ratelimit)
{
	smsg_t *pMsg;
	if(ratelimit->pMsg != NULL) {
		if(ratelimit->nsupp > 0) {
			pMsg = ratelimitGenRepMsg(ratelimit);
			if(pMsg != NULL)
				submitMsg2(pMsg);
		}
		msgDestruct(&ratelimit->pMsg);
	}
	tellLostCnt(ratelimit);
	if(ratelimit->bThreadSafe)
		pthread_mutex_destroy(&ratelimit->mut);
	free(ratelimit->name);
	free(ratelimit);
}

void
ratelimitModExit(void)
{
	objRelease(datetime, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(parser, CORE_COMPONENT);
}

rsRetVal
ratelimitModInit(void)
{
	DEFiRet;
	CHKiRet(objGetObjInterface(&obj));
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(parser, CORE_COMPONENT));
finalize_it:
	RETiRet;
}
