/* action.c
 *
 * Implementation of the action object.
 *
 * File begun on 2007-08-06 by RGerhards (extracted from syslogd.c)
 *
 * Some notes on processing (this hopefully makes it easier to find
 * the right code in question): For performance reasons, this module
 * uses different methods of message submission based on the user-selected
 * configuration. This code is similar, but can not be abstracted because
 * of the performance-affecting differences in it. As such, it is often
 * necessary to triple-check that everything works well in *all* modes.
 * The different modes (and calling sequence) are:
 *
 * if set iExecEveryNthOccur > 1 || iSecsExecOnceInterval
 * - doSubmitToActionQComplex
 *   handles mark message reduction, but in essence calls
 * - actionWriteToAction
 * - qqueueEnqObj
 *   (now queue engine processing)
 * if(pThis->bWriteAllMarkMsgs == RSFALSE)
 * - doSubmitToActionQNotAllMark
 * - doSubmitToActionQ (and from here like in the else case below!)
 * else
 * - doSubmitToActionQ
 * - qqueueEnqObj
 *   (now queue engine processing)
 *
 * Note that bWriteAllMakrMsgs on or off creates almost the same processing.
 * The difference ist that if WriteAllMarkMsgs is not set, we need to
 * preprocess the batch and drop mark messages which are not yet due for
 * writing.
 *
 * After dequeue, processing is as follows:
 * - processBatchMain
 * - ...
 *
 * MORE ON PROCESSING, QUEUES and FILTERING
 * All filtering needs to be done BEFORE messages are enqueued to an
 * action. In previous code, part of the filtering was done at the
 * "remote end" of the action queue, which lead to problems in
 * non-direct mode (because then things run asynchronously). In order
 * to solve this problem once and for all, I have changed the code so
 * that all filtering is done before enq, and processing on the
 * dequeue side of action processing now always executes whatever is
 * enqueued. This is the only way to handle things consistently and
 * (as much as possible) in a queue-type agnostic way. However, it is
 * a rather radical change, which I unfortunately needed to make from
 * stable version 5.8.1 to 5.8.2. If new problems pop up, you now know
 * what may be their cause. In any case, the way it is done now is the
 * only correct one.
 * A problem is that, under fortunate conditions, we use the current
 * batch for the output system as well. This is very good from a performance
 * point of view, but makes the distinction between enq and deq side of
 * the queue a bit hard. The current idea is that the filter condition
 * alone is checked at the deq side of the queue (seems to be unavoidable
 * to do it that way), but all other complex conditons (like failover
 * handling) go into the computation of the filter condition. For
 * non-direct queues, we still enqueue only what is acutally necessary.
 * Note that in this case the rest of the code must ensure that the filter
 * is set to "true". While this is not perfect and not as simple as
 * we would like to see it, it looks like the best way to tackle that
 * beast.
 * rgerhards, 2011-06-15
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef _AIX
#include <pthread.h>
#endif
#include <json.h>

#include "rsyslog.h"
#include "dirty.h"
#include "template.h"
#include "action.h"
#include "modules.h"
#include "cfsysline.h"
#include "srUtils.h"
#include "errmsg.h"
#include "batch.h"
#include "wti.h"
#include "rsconf.h"
#include "datetime.h"
#include "unicode-helper.h"
#include "atomic.h"
#include "ruleset.h"
#include "parserif.h"
#include "statsobj.h"

/* AIXPORT : cs renamed to legacy_cs as clashes with libpthreads variable in complete file*/
#ifdef _AIX
#define cs legacy_cs
#endif
#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

#define NO_TIME_PROVIDED 0 /* indicate we do not provide any cached time */

/* forward definitions */
static rsRetVal processBatchMain(void *pVoid, batch_t *pBatch, wti_t * const pWti);
static rsRetVal doSubmitToActionQ(action_t * const pAction, wti_t * const pWti, smsg_t*);
static rsRetVal doSubmitToActionQComplex(action_t * const pAction, wti_t * const pWti, smsg_t*);
static rsRetVal doSubmitToActionQNotAllMark(action_t * const pAction, wti_t * const pWti, smsg_t*);

/* object static data (once for all instances) */
DEFobjCurrIf(obj)
DEFobjCurrIf(datetime)
DEFobjCurrIf(module)
DEFobjCurrIf(statsobj)
DEFobjCurrIf(ruleset)


typedef struct configSettings_s {
	int bActExecWhenPrevSusp;			/* execute action only when previous one was suspended? */
	int bActionWriteAllMarkMsgs;			/* should all mark messages be unconditionally written? */
	int iActExecOnceInterval;			/* execute action once every nn seconds */
	int iActExecEveryNthOccur;			/* execute action every n-th occurence (0,1=always) */
	time_t iActExecEveryNthOccurTO;			/* timeout for n-occurence setting (in seconds, 0=never) */
	int glbliActionResumeInterval;
	int glbliActionResumeRetryCount;		/* how often should suspended actions be retried? */
	int bActionRepMsgHasMsg;			/* last messsage repeated... has msg fragment in it */
	uchar *pszActionName;				/* short name for the action */
	/* action queue and its configuration parameters */
	queueType_t ActionQueType;			/* type of the main message queue above */
	int iActionQueueSize;				/* size of the main message queue above */
	int iActionQueueDeqBatchSize;			/* batch size for action queues */
	int iActionQHighWtrMark;			/* high water mark for disk-assisted queues */
	int iActionQLowWtrMark;				/* low water mark for disk-assisted queues */
	int iActionQDiscardMark;			/* begin to discard messages */
	int iActionQDiscardSeverity;
	/* by default, discard nothing to prevent unintentional loss */
	int iActionQueueNumWorkers;			/* number of worker threads for the mm queue above */
	uchar *pszActionQFName;				/* prefix for the main message queue file */
	int64 iActionQueMaxFileSize;
	int iActionQPersistUpdCnt;			/* persist queue info every n updates */
	int bActionQSyncQeueFiles;			/* sync queue files */
	int iActionQtoQShutdown;			/* queue shutdown */
	int iActionQtoActShutdown;			/* action shutdown (in phase 2) */
	int iActionQtoEnq;				/* timeout for queue enque */
	int iActionQtoWrkShutdown;			/* timeout for worker thread shutdown */
	int iActionQWrkMinMsgs;				/* minimum messages per worker needed to start a new one */
	int bActionQSaveOnShutdown;			/* save queue on shutdown (when DA enabled)? */
	int64 iActionQueMaxDiskSpace;			/* max disk space allocated 0 ==> unlimited */
	int iActionQueueDeqSlowdown;			/* dequeue slowdown (simple rate limiting) */
	int iActionQueueDeqtWinFromHr;			/* hour begin of time frame when queue is to be dequeued */
	int iActionQueueDeqtWinToHr;			/* hour begin of time frame when queue is to be dequeued */
} configSettings_t;


static configSettings_t cs;					/* our current config settings */

/* the counter below counts actions created. It is used to obtain unique IDs for the action. They
 * should not be relied on for any long-term activity (e.g. disk queue names!), but they are nice
 * to have during one instance of an rsyslogd run. For example, I use them to name actions when there
 * is no better name available.
 */
int iActionNbr = 0;
int bActionReportSuspension = 1;
int bActionReportSuspensionCont = 0;

/* tables for interfacing with the v6 config system */
static struct cnfparamdescr cnfparamdescr[] = {
	{ "name", eCmdHdlrGetWord, 0 }, /* legacy: actionname */
	{ "type", eCmdHdlrString, CNFPARAM_REQUIRED }, /* legacy: actionname */
	{ "action.errorfile", eCmdHdlrString, 0 },
	{ "action.writeallmarkmessages", eCmdHdlrBinary, 0 }, /* legacy: actionwriteallmarkmessages */
	{ "action.execonlyeverynthtime", eCmdHdlrInt, 0 }, /* legacy: actionexeconlyeverynthtime */
	{ "action.execonlyeverynthtimetimeout", eCmdHdlrInt, 0 }, /* legacy: actionexeconlyeverynthtimetimeout */
	{ "action.execonlyonceeveryinterval", eCmdHdlrInt, 0 }, /* legacy: actionexeconlyonceeveryinterval */
	{ "action.execonlywhenpreviousissuspended", eCmdHdlrBinary, 0 },
	/* legacy: actionexeconlywhenpreviousissuspended */
	{ "action.repeatedmsgcontainsoriginalmsg", eCmdHdlrBinary, 0 }, /* legacy: repeatedmsgcontainsoriginalmsg */
	{ "action.resumeretrycount", eCmdHdlrInt, 0 }, /* legacy: actionresumeretrycount */
	{ "action.reportsuspension", eCmdHdlrBinary, 0 },
	{ "action.reportsuspensioncontinuation", eCmdHdlrBinary, 0 },
	{ "action.resumeinterval", eCmdHdlrInt, 0 },
	{ "action.copymsg", eCmdHdlrBinary, 0 }
};
static struct cnfparamblk pblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(cnfparamdescr)/sizeof(struct cnfparamdescr),
	  cnfparamdescr
	};


/* primarily a helper for debug purposes, get human-readble name of state */
/* currently not needed, but may be useful in the future!
static const char *
batchState2String(const batch_state_t state)
{
	switch(state) {
	case BATCH_STATE_RDY:
		return "BATCH_STATE_RDY";
	case BATCH_STATE_BAD:
		return "BATCH_STATE_BAD";
	case BATCH_STATE_SUB:
		return "BATCH_STATE_SUB";
	case BATCH_STATE_COMM:
		return "BATCH_STATE_COMM";
	case BATCH_STATE_DISC:
		return "BATCH_STATE_DISC";
	default:
		return "ERROR, batch state not known!";
	}
}
*/

/* ------------------------------ methods ------------------------------ */

/* This function returns the "current" time for this action. Current time
 * is not necessarily real-time. In order to enhance performance, current
 * system time is obtained the first time an action needs to know the time
 * and then kept cached inside the action structure. Later requests will
 * always return that very same time. Wile not totally accurate, it is far
 * accurate in most cases and considered "acurate enough" for all cases.
 * When changing the threading model, please keep in mind that this
 * logic needs to be changed should we once allow more than one parallel
 * call into the same action (object). As this is currently not supported,
 * we simply cache the time inside the action object itself, after it
 * is under mutex protection.
 * Side-note: the value -1 is used as tActNow, because it also is the
 * error return value of time(). So we would do a retry with the next
 * invocation if time() failed. Then, of course, we would probably already
 * be in trouble, but for the sake of performance we accept this very,
 * very slight risk.
 * This logic has been added as part of an overall performance improvment
 * effort inspired by David Lang. -- rgerhards, 2008-09-16
 * Note: this function does not use the usual iRet call conventions
 * because that would provide little to no benefit but complicate things
 * a lot. So we simply return the system time.
 */
static time_t
getActNow(action_t * const pThis)
{
	assert(pThis != NULL);
	if(pThis->tActNow == -1) {
		pThis->tActNow = datetime.GetTime(NULL); /* good time call - the only one done */
		if(pThis->tLastExec > pThis->tActNow) {
			/* if we are traveling back in time, reset tLastExec */
			pThis->tLastExec = (time_t) 0;
		}
	}

	return pThis->tActNow;
}


/* resets action queue parameters to their default values. This happens
 * after each action has been created in order to prevent any wild defaults
 * to be used. It is somewhat against the original spirit of the config file
 * reader, but I think it is a good thing to do.
 * rgerhards, 2008-01-29
 */
static rsRetVal
actionResetQueueParams(void)
{
	DEFiRet;

	cs.ActionQueType = QUEUETYPE_DIRECT;		/* type of the main message queue above */
	cs.iActionQueueSize = 1000;			/* size of the main message queue above */
	cs.iActionQueueDeqBatchSize = 16;		/* default batch size */
	cs.iActionQHighWtrMark = -1;			/* high water mark for disk-assisted queues */
	cs.iActionQLowWtrMark = -1;			/* low water mark for disk-assisted queues */
	cs.iActionQDiscardMark = 980;			/* begin to discard messages */
	cs.iActionQDiscardSeverity = 8;			/* discard warning and above */
	cs.iActionQueueNumWorkers = 1;			/* number of worker threads for the mm queue above */
	cs.iActionQueMaxFileSize = 1024*1024;
	cs.iActionQPersistUpdCnt = 0;			/* persist queue info every n updates */
	cs.bActionQSyncQeueFiles = 0;
	cs.iActionQtoQShutdown = 0;			/* queue shutdown */
	cs.iActionQtoActShutdown = 1000;		/* action shutdown (in phase 2) */
	cs.iActionQtoEnq = 50;				/* timeout for queue enque */
	cs.iActionQtoWrkShutdown = 60000;		/* timeout for worker thread shutdown */
	cs.iActionQWrkMinMsgs = -1;			/* minimum messages per worker needed to start a new one */
	cs.bActionQSaveOnShutdown = 1;			/* save queue on shutdown (when DA enabled)? */
	cs.iActionQueMaxDiskSpace = 0;
	cs.iActionQueueDeqSlowdown = 0;
	cs.iActionQueueDeqtWinFromHr = 0;
	cs.iActionQueueDeqtWinToHr = 25;		/* 25 disables time windowed dequeuing */

	cs.glbliActionResumeRetryCount = 0;		/* I guess it is smart to reset this one, too */

	d_free(cs.pszActionQFName);
	cs.pszActionQFName = NULL;			/* prefix for the main message queue file */

	RETiRet;
}


/* destructs an action descriptor object
 * rgerhards, 2007-08-01
 */
rsRetVal actionDestruct(action_t * const pThis)
{
	DEFiRet;
	ASSERT(pThis != NULL);

	if(!strcmp((char*)modGetName(pThis->pMod), "builtin:omdiscard")) {
		/* discard actions will be optimized out */
		FINALIZE;
	}

	if(pThis->pQueue != NULL) {
		qqueueDestruct(&pThis->pQueue);
	}

	/* destroy stats object, if we have one (may not always be
	 * be the case, e.g. if turned off)
	 */
	if(pThis->statsobj != NULL)
		statsobj.Destruct(&pThis->statsobj);

	if(pThis->pModData != NULL)
		pThis->pMod->freeInstance(pThis->pModData);

	if(pThis->fdErrFile != -1)
		close(pThis->fdErrFile);
	pthread_mutex_destroy(&pThis->mutErrFile);
	pthread_mutex_destroy(&pThis->mutAction);
	pthread_mutex_destroy(&pThis->mutWrkrDataTable);
	free((void*)pThis->pszErrFile);
	d_free(pThis->pszName);
	d_free(pThis->ppTpl);
	d_free(pThis->peParamPassing);
	d_free(pThis->wrkrDataTable);

finalize_it:
	d_free(pThis);
	RETiRet;
}


/* Disable action, this means it will never again be usable
 * until rsyslog is reloaded. Use only as a last resort, but
 * depends on output module.
 * rgerhards, 2007-08-02
 */
static inline void
actionDisable(action_t *__restrict__ const pThis)
{
	pThis->bDisabled = 1;
}



/* create a new action descriptor object
 * rgerhards, 2007-08-01
 * Note that it is vital to set proper initial values as the v6 config
 * system depends on these!
 */
rsRetVal actionConstruct(action_t **ppThis)
{
	DEFiRet;
	action_t *pThis;

	ASSERT(ppThis != NULL);
	
	CHKmalloc(pThis = (action_t*) calloc(1, sizeof(action_t)));
	pThis->iResumeInterval = 30;
	pThis->iResumeRetryCount = 0;
	pThis->pszName = NULL;
	pThis->pszErrFile = NULL;
	pThis->fdErrFile = -1;
	pThis->bWriteAllMarkMsgs = 1;
	pThis->iExecEveryNthOccur = 0;
	pThis->iExecEveryNthOccurTO = 0;
	pThis->iSecsExecOnceInterval = 0;
	pThis->bExecWhenPrevSusp = 0;
	pThis->bRepMsgHasMsg = 0;
	pThis->bDisabled = 0;
	pThis->isTransactional = 0;
	pThis->bReportSuspension = -1; /* indicate "not yet set" */
	pThis->bReportSuspensionCont = -1; /* indicate "not yet set" */
	pThis->bCopyMsg = 0;
	pThis->tLastOccur = datetime.GetTime(NULL);	/* done once per action on startup only */
	pThis->iActionNbr = iActionNbr;
	pthread_mutex_init(&pThis->mutErrFile, NULL);
	pthread_mutex_init(&pThis->mutAction, NULL);
	pthread_mutex_init(&pThis->mutWrkrDataTable, NULL);
	INIT_ATOMIC_HELPER_MUT(pThis->mutCAS);

	/* indicate we have a new action */
	++iActionNbr;

finalize_it:
	*ppThis = pThis;
	RETiRet;
}


/* action construction finalizer
 */
rsRetVal
actionConstructFinalize(action_t *__restrict__ const pThis, struct nvlst *lst)
{
	DEFiRet;
	uchar pszAName[64]; /* friendly name of our action */

	if(!strcmp((char*)modGetName(pThis->pMod), "builtin:omdiscard")) {
		/* discard actions will be optimized out */
		FINALIZE;
	}
	/* generate a friendly name for us action stats */
	if(pThis->pszName == NULL) {
		snprintf((char*) pszAName, sizeof(pszAName), "action-%d-%s",
			pThis->iActionNbr, pThis->pMod->pszName);
		pThis->pszName = ustrdup(pszAName);
	}

	/* cache transactional attribute */
	pThis->isTransactional = pThis->pMod->mod.om.supportsTX;
	if(pThis->isTransactional) {
		int i;
		for(i = 0 ; i < pThis->iNumTpls ; ++i) {
			if(pThis->peParamPassing[i] != ACT_STRING_PASSING) {
				LogError(0, RS_RET_INVLD_OMOD, "action '%s'(%d) is transactional but "
						"parameter %d "
						"uses invalid parameter passing mode -- disabling "
						"action. This is probably caused by a pre-v7 "
						"output module that needs upgrade.",
						pThis->pszName, pThis->iActionNbr, i);
				actionDisable(pThis);
				ABORT_FINALIZE(RS_RET_INVLD_OMOD);

			}
		}
	}


	/* support statistics gathering */
	CHKiRet(statsobj.Construct(&pThis->statsobj));
	CHKiRet(statsobj.SetName(pThis->statsobj, pThis->pszName));
	CHKiRet(statsobj.SetOrigin(pThis->statsobj, (uchar*)"core.action"));

	STATSCOUNTER_INIT(pThis->ctrProcessed, pThis->mutCtrProcessed);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("processed"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrProcessed));

	STATSCOUNTER_INIT(pThis->ctrFail, pThis->mutCtrFail);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("failed"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrFail));

	STATSCOUNTER_INIT(pThis->ctrSuspend, pThis->mutCtrSuspend);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("suspended"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrSuspend));
	STATSCOUNTER_INIT(pThis->ctrSuspendDuration, pThis->mutCtrSuspendDuration);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("suspended.duration"),
		ctrType_IntCtr, 0, &pThis->ctrSuspendDuration));

	STATSCOUNTER_INIT(pThis->ctrResume, pThis->mutCtrResume);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("resumed"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrResume));

	CHKiRet(statsobj.ConstructFinalize(pThis->statsobj));

	/* create our queue */

	/* generate a friendly name for the queue */
	snprintf((char*) pszAName, sizeof(pszAName), "%s queue",
		 pThis->pszName);

	/* now check if we can run the action in "firehose mode" during stage one of
	 * its processing (that is before messages are enqueued into the action q).
	 * This is only possible if some features, which require strict sequence, are
	 * not used. Thankfully, that is usually the case. The benefit of firehose
	 * mode is much faster processing (and simpler code) -- rgerhards, 2010-06-08
	 */
	if(   pThis->iExecEveryNthOccur > 1
	   || pThis->iSecsExecOnceInterval
	  ) {
		DBGPRINTF("info: firehose mode disabled for action because "
		          "iExecEveryNthOccur=%d, iSecsExecOnceInterval=%d\n",
			  pThis->iExecEveryNthOccur, pThis->iSecsExecOnceInterval);
		pThis->submitToActQ = doSubmitToActionQComplex;
	} else if(pThis->bWriteAllMarkMsgs) {
		/* full firehose submission mode, default case*/
		pThis->submitToActQ = doSubmitToActionQ;
	} else {
		/* nearly full-speed submission mode */
		pThis->submitToActQ = doSubmitToActionQNotAllMark;
	}

	/* create queue */
	/* action queues always (for now) have just one worker. This may change when
	 * we begin to implement an interface the enable output modules to request
	 * to be run on multiple threads. So far, this is forbidden by the interface
	 * spec. -- rgerhards, 2008-01-30
	 */
	CHKiRet(qqueueConstruct(&pThis->pQueue, cs.ActionQueType, 1, cs.iActionQueueSize,
					processBatchMain));
	obj.SetName((obj_t*) pThis->pQueue, pszAName);
	qqueueSetpAction(pThis->pQueue, pThis);

	if(lst == NULL) { /* use legacy params? */
		/* ... set some properties ... */
#		define setQPROP(func, directive, data) \
		CHKiRet_Hdlr(func(pThis->pQueue, data)) { \
			LogError(0, NO_ERRCODE, "Invalid " #directive ", \
				error %d. Ignored, running with default setting", iRet); \
		}
#		define setQPROPstr(func, directive, data) \
		CHKiRet_Hdlr(func(pThis->pQueue, data, (data == NULL)? 0 : strlen((char*) data))) { \
			LogError(0, NO_ERRCODE, "Invalid " #directive ", \
				error %d. Ignored, running with default setting", iRet); \
		}
		setQPROP(qqueueSetsizeOnDiskMax, "$ActionQueueMaxDiskSpace", cs.iActionQueMaxDiskSpace);
		setQPROP(qqueueSetiDeqBatchSize, "$ActionQueueDequeueBatchSize", cs.iActionQueueDeqBatchSize);
		setQPROP(qqueueSetMaxFileSize, "$ActionQueueFileSize", cs.iActionQueMaxFileSize);
		setQPROPstr(qqueueSetFilePrefix, "$ActionQueueFileName", cs.pszActionQFName);
		setQPROP(qqueueSetiPersistUpdCnt, "$ActionQueueCheckpointInterval", cs.iActionQPersistUpdCnt);
		setQPROP(qqueueSetbSyncQueueFiles, "$ActionQueueSyncQueueFiles", cs.bActionQSyncQeueFiles);
		setQPROP(qqueueSettoQShutdown, "$ActionQueueTimeoutShutdown", cs.iActionQtoQShutdown );
		setQPROP(qqueueSettoActShutdown, "$ActionQueueTimeoutActionCompletion", cs.iActionQtoActShutdown);
		setQPROP(qqueueSettoWrkShutdown, "$ActionQueueWorkerTimeoutThreadShutdown", cs.iActionQtoWrkShutdown);
		setQPROP(qqueueSettoEnq, "$ActionQueueTimeoutEnqueue", cs.iActionQtoEnq);
		setQPROP(qqueueSetiHighWtrMrk, "$ActionQueueHighWaterMark", cs.iActionQHighWtrMark);
		setQPROP(qqueueSetiLowWtrMrk, "$ActionQueueLowWaterMark", cs.iActionQLowWtrMark);
		setQPROP(qqueueSetiDiscardMrk, "$ActionQueueDiscardMark", cs.iActionQDiscardMark);
		setQPROP(qqueueSetiDiscardSeverity, "$ActionQueueDiscardSeverity", cs.iActionQDiscardSeverity);
		setQPROP(qqueueSetiMinMsgsPerWrkr, "$ActionQueueWorkerThreadMinimumMessages", cs.iActionQWrkMinMsgs);
		setQPROP(qqueueSetiNumWorkerThreads, "$ActionQueueWorkerThreads", cs.iActionQueueNumWorkers);
		setQPROP(qqueueSetbSaveOnShutdown, "$ActionQueueSaveOnShutdown", cs.bActionQSaveOnShutdown);
		setQPROP(qqueueSetiDeqSlowdown,    "$ActionQueueDequeueSlowdown", cs.iActionQueueDeqSlowdown);
		setQPROP(qqueueSetiDeqtWinFromHr,  "$ActionQueueDequeueTimeBegin", cs.iActionQueueDeqtWinFromHr);
		setQPROP(qqueueSetiDeqtWinToHr,    "$ActionQueueDequeueTimeEnd", cs.iActionQueueDeqtWinToHr);
	} else {
		/* we have v6-style config params */
		qqueueSetDefaultsActionQueue(pThis->pQueue);
		qqueueApplyCnfParam(pThis->pQueue, lst);
	}

#	undef setQPROP
#	undef setQPROPstr

	qqueueDbgPrint(pThis->pQueue);

	DBGPRINTF("Action %p: queue %p created\n", pThis, pThis->pQueue);

	if(pThis->bUsesMsgPassingMode && pThis->pQueue->qType != QUEUETYPE_DIRECT) {
		parser_warnmsg("module %s with message passing mode uses "
			"non-direct queue. This most probably leads to undesired "
			"results. For message modificaton modules (mm*), this means "
			"that they will have no effect - "
			"see https://www.rsyslog.com/mm-no-queue/", (char*)modGetName(pThis->pMod));
	}
	
	/* and now reset the queue params (see comment in its function header!) */
	actionResetQueueParams();

finalize_it:
	RETiRet;
}



/* set the global resume interval
 */
rsRetVal actionSetGlobalResumeInterval(int iNewVal)
{
	cs.glbliActionResumeInterval = iNewVal;
	return RS_RET_OK;
}


/* returns the action state name in human-readable form
 * returned string must not be modified.
 * rgerhards, 2009-05-07
 */
static uchar *getActStateName(action_t * const pThis, wti_t * const pWti)
{
	switch(getActionState(pWti, pThis)) {
		case ACT_STATE_RDY:
			return (uchar*) "rdy";
		case ACT_STATE_ITX:
			return (uchar*) "itx";
		case ACT_STATE_RTRY:
			return (uchar*) "rtry";
		case ACT_STATE_SUSP:
			return (uchar*) "susp";
		case ACT_STATE_DATAFAIL:
			return (uchar*) "datafail";
		default:
			return (uchar*) "ERROR/UNKNWON";
	}
}


/* returns a suitable return code based on action state
 * rgerhards, 2009-05-07
 */
static rsRetVal getReturnCode(action_t * const pThis, wti_t * const pWti)
{
	DEFiRet;

	switch(getActionState(pWti, pThis)) {
		case ACT_STATE_RDY:
			iRet = RS_RET_OK;
			break;
		case ACT_STATE_ITX:
			if(pWti->actWrkrInfo[pThis->iActionNbr].bHadAutoCommit) {
				pWti->actWrkrInfo[pThis->iActionNbr].bHadAutoCommit = 0; /* auto-reset */
				iRet = RS_RET_PREVIOUS_COMMITTED;
			} else {
				iRet = RS_RET_DEFER_COMMIT;
			}
			break;
		case ACT_STATE_RTRY:
			iRet = RS_RET_SUSPENDED;
			break;
		case ACT_STATE_SUSP:
			iRet = RS_RET_ACTION_FAILED;
			break;
		case ACT_STATE_DATAFAIL:
			iRet = RS_RET_DATAFAIL;
			break;
		default:
			DBGPRINTF("Invalid action engine state %u, program error\n",
				  getActionState(pWti, pThis));
			iRet = RS_RET_ERR;
			break;
	}

	RETiRet;
}


/* set the action to a new state
 * rgerhards, 2007-08-02
 */
static void
actionSetState(action_t * const pThis, wti_t * const pWti, uint8_t newState)
{
	setActionState(pWti, pThis, newState);
	DBGPRINTF("action[%s] transitioned to state: %s\n",
		  pThis->pszName, getActStateName(pThis, pWti));
}

/* Handles the transient commit state. So far, this is
 * mostly a dummy...
 * rgerhards, 2007-08-02
 */
static void actionCommitted(action_t * const pThis, wti_t * const pWti)
{
	actionSetState(pThis, pWti, ACT_STATE_RDY);
}


/* we need to defer setting the action's own bReportSuspension state until
 * after the full config has been processed. So the most simple case to do
 * that is here. It's not a performance problem, as it happens infrequently.
 * it's not a threading race problem, as always the same value will be written.
 * As we need to do this in several places, we have moved the code to its own
 * helper function.
 */
static void
setSuspendMessageConfVars(action_t *__restrict__ const pThis)
{
	if(pThis->bReportSuspension == -1)
		pThis->bReportSuspension = bActionReportSuspension;
	if(pThis->bReportSuspensionCont == -1) {
		pThis->bReportSuspensionCont = bActionReportSuspensionCont;
		if(pThis->bReportSuspensionCont == -1)
			pThis->bReportSuspensionCont = 1;
	}
}


/* set action to "rtry" state.
 * rgerhards, 2007-08-02
 */
static void actionRetry(action_t * const pThis, wti_t * const pWti)
{
	setSuspendMessageConfVars(pThis);
	actionSetState(pThis, pWti, ACT_STATE_RTRY);
	LogMsg(0, RS_RET_SUSPENDED, LOG_WARNING,
	      "action '%s' suspended (module '%s'), retry %d. There should "
	      "be messages before this one giving the reason for suspension.",
	      pThis->pszName, pThis->pMod->pszName,
	      getActionNbrResRtry(pWti, pThis));
	incActionResumeInRow(pWti, pThis);
}

/* Suspend action, this involves changing the action state as well
 * as setting the next retry time.
 * if we have more than 10 retries, we prolong the
 * retry interval. If something is really stalled, it will
 * get re-tried only very, very seldom - but that saves
 * CPU time. TODO: maybe a config option for that?
 * rgerhards, 2007-08-02
 */
static void
actionSuspend(action_t * const pThis, wti_t * const pWti)
{
	time_t ttNow;
	int suspendDuration;
	char timebuf[32];

	setSuspendMessageConfVars(pThis);

	/* note: we can NOT use a cached timestamp, as time may have evolved
	 * since caching, and this would break logic (and it actually did so!)
	 */
	datetime.GetTime(&ttNow);
	suspendDuration = pThis->iResumeInterval * (getActionNbrResRtry(pWti, pThis) / 10 + 1);
	pThis->ttResumeRtry = ttNow + suspendDuration;
	actionSetState(pThis, pWti, ACT_STATE_SUSP);
	pThis->ctrSuspendDuration += suspendDuration;
	if(getActionNbrResRtry(pWti, pThis) == 0) {
		STATSCOUNTER_INC(pThis->ctrSuspend, pThis->mutCtrSuspend);
	}

	if(   pThis->bReportSuspensionCont
	   || (pThis->bReportSuspension && getActionNbrResRtry(pWti, pThis) == 0) ) {
		ctime_r(&pThis->ttResumeRtry, timebuf);
		timebuf[strlen(timebuf)-1] = '\0'; /* strip LF */
		LogMsg(0, RS_RET_SUSPENDED, LOG_WARNING,
			      "action '%s' suspended (module '%s'), next retry is %s, retry nbr %d. "
			      "There should be messages before this one giving the reason for suspension.",
			      pThis->pszName, pThis->pMod->pszName, timebuf,
			      getActionNbrResRtry(pWti, pThis));
	}
	DBGPRINTF("action '%s' suspended, earliest retry=%lld (now %lld), iNbrResRtry %d, "
		  "duration %d\n",
		  pThis->pszName, (long long) pThis->ttResumeRtry, (long long) ttNow,
		  getActionNbrResRtry(pWti, pThis), suspendDuration);
}


/* actually do retry processing. Note that the function receives a timestamp so
 * that we do not need to call the (expensive) time() API.
 * Note that we do the full retry processing here, doing the configured number of
 * iterations. -- rgerhards, 2009-05-07
 * We need to guard against module which always return RS_RET_OK from their tryResume()
 * entry point. This is invalid, but has harsh consequences: it will cause the rsyslog
 * engine to go into a tight loop. That obviously is not acceptable. As such, we track the
 * count of iterations that a tryResume returning RS_RET_OK is immediately followed by
 * an unsuccessful call to doAction(). If that happens more than 10 times, we assume
 * the return acutally is a RS_RET_SUSPENDED. In order to go through the various
 * resumption stages, we do this for every 10 requests. This magic number 10 may
 * not be the most appropriate, but it should be thought of a "if nothing else helps"
 * kind of facility: in the first place, the module should return a proper indication
 * of its inability to recover. -- rgerhards, 2010-04-26.
 */
static rsRetVal
actionDoRetry(action_t * const pThis, wti_t * const pWti)
{
	int iRetries;
	int iSleepPeriod;
	int bTreatOKasSusp;
	DEFiRet;

	ASSERT(pThis != NULL);

	iRetries = 0;
	while((*pWti->pbShutdownImmediate == 0) && getActionState(pWti, pThis) == ACT_STATE_RTRY) {
		DBGPRINTF("actionDoRetry: %s enter loop, iRetries=%d, ResumeInRow %d\n",
			pThis->pszName, iRetries, getActionResumeInRow(pWti, pThis));
		iRet = pThis->pMod->tryResume(pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData);
		DBGPRINTF("actionDoRetry: %s action->tryResume returned %d\n", pThis->pszName, iRet);
		if((getActionResumeInRow(pWti, pThis) > 9) && (getActionResumeInRow(pWti, pThis) % 10 == 0)) {
			bTreatOKasSusp = 1;
			setActionResumeInRow(pWti, pThis, 0);
			iRet = RS_RET_SUSPENDED;
		} else {
			bTreatOKasSusp = 0;
		}
		if((iRet == RS_RET_OK) && (!bTreatOKasSusp)) {
			DBGPRINTF("actionDoRetry: %s had success RDY again (iRet=%d)\n",
				  pThis->pszName, iRet);
			if(pThis->bReportSuspension) {
				LogMsg(0, RS_RET_RESUMED, LOG_INFO, "action '%s' "
					      "resumed (module '%s')",
					      pThis->pszName, pThis->pMod->pszName);
			}
			actionSetState(pThis, pWti, ACT_STATE_RDY);
		} else if(iRet == RS_RET_SUSPENDED || bTreatOKasSusp) {
			/* max retries reached? */
			DBGPRINTF("actionDoRetry: %s check for max retries, iResumeRetryCount "
				  "%d, iRetries %d\n",
				  pThis->pszName, pThis->iResumeRetryCount, iRetries);
			if((pThis->iResumeRetryCount != -1 && iRetries >= pThis->iResumeRetryCount)) {
				actionSuspend(pThis, pWti);
				if(getActionNbrResRtry(pWti, pThis) < 20)
					incActionNbrResRtry(pWti, pThis);
			} else {
				++iRetries;
				iSleepPeriod = pThis->iResumeInterval;
				srSleep(iSleepPeriod, 0);
				if(*pWti->pbShutdownImmediate) {
					ABORT_FINALIZE(RS_RET_FORCE_TERM);
				}
			}
		} else if(iRet == RS_RET_DISABLE_ACTION) {
			actionDisable(pThis);
		}
	}

	if(getActionState(pWti, pThis) == ACT_STATE_RDY) {
		setActionNbrResRtry(pWti, pThis, 0);
	}

finalize_it:
	RETiRet;
}


static rsRetVal
actionCheckAndCreateWrkrInstance(action_t * const pThis, const wti_t *const pWti)
{
	int locked = 0;
	DEFiRet;
	if(pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData == NULL) {
		DBGPRINTF("wti %p: we need to create a new action worker instance for "
			  "action %d\n", pWti, pThis->iActionNbr);
		CHKiRet(pThis->pMod->mod.om.createWrkrInstance(&(pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData),
						               pThis->pModData));
		pWti->actWrkrInfo[pThis->iActionNbr].pAction = pThis;
		setActionState(pWti, pThis, ACT_STATE_RDY); /* action is enabled */

		/* maintain worker data table -- only needed if wrkrHUP is requested! */

		pthread_mutex_lock(&pThis->mutWrkrDataTable);
		locked = 1;
		int freeSpot;
		for(freeSpot = 0 ; freeSpot < pThis->wrkrDataTableSize ; ++freeSpot)
			if(pThis->wrkrDataTable[freeSpot] == NULL)
				break;
		if(pThis->nWrkr == pThis->wrkrDataTableSize) {
			void *const newTable = realloc(pThis->wrkrDataTable,
				(pThis->wrkrDataTableSize + 1) * sizeof(void*));
			if(newTable == NULL) {
				DBGPRINTF("actionCheckAndCreateWrkrInstance: out of "
					"memory realloc wrkrDataTable\n")
				ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
			}
			pThis->wrkrDataTable = newTable;
			pThis->wrkrDataTableSize++;
		}
		pThis->wrkrDataTable[freeSpot] = pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData;
		pThis->nWrkr++;
		DBGPRINTF("wti %p: created action worker instance %d for "
			  "action %d\n", pWti, pThis->nWrkr, pThis->iActionNbr);
	}
finalize_it:
	if(locked) {
		pthread_mutex_unlock(&pThis->mutWrkrDataTable);
	}
	RETiRet;
}

/* try to resume an action -- rgerhards, 2007-08-02
 * changed to new action state engine -- rgerhards, 2009-05-07
 */
static rsRetVal
actionTryResume(action_t * const pThis, wti_t * const pWti)
{
	DEFiRet;
	time_t ttNow = NO_TIME_PROVIDED;

	if(getActionState(pWti, pThis) == ACT_STATE_SUSP) {
		/* if we are suspended, we need to check if the timeout expired.
		 * for this handling, we must always obtain a fresh timestamp. We used
		 * to use the action timestamp, but in this case we will never reach a
		 * point where a resumption is actually tried, because the action timestamp
		 * is always in the past. So we can not avoid doing a fresh time() call
		 * here. -- rgerhards, 2009-03-18
		 */
		datetime.GetTime(&ttNow); /* cache "now" */
		if(ttNow >= pThis->ttResumeRtry) {
			actionSetState(pThis, pWti, ACT_STATE_RTRY); /* back to retries */
		}
	}

	if(getActionState(pWti, pThis) == ACT_STATE_RTRY) {
		CHKiRet(actionDoRetry(pThis, pWti));
	}

	if(Debug && (getActionState(pWti, pThis) == ACT_STATE_RTRY ||
		getActionState(pWti, pThis) == ACT_STATE_SUSP)) {
		if(ttNow == NO_TIME_PROVIDED) /* use cached result if we have it */
			datetime.GetTime(&ttNow);
		dbgprintf("actionTryResume: action[%s] state: %s, next retry (if applicable): %u [now %u]\n",
			pThis->pszName, getActStateName(pThis, pWti),
			(unsigned) pThis->ttResumeRtry, (unsigned) ttNow);
	}

finalize_it:
	RETiRet;
}


/* prepare an action for performing work. This involves trying to recover it,
 * depending on its current state.
 * rgerhards, 2009-05-07
 */
static rsRetVal
actionPrepare(action_t *__restrict__ const pThis, wti_t *__restrict__ const pWti)
{
	DEFiRet;

	CHKiRet(actionCheckAndCreateWrkrInstance(pThis, pWti));
	CHKiRet(actionTryResume(pThis, pWti));

	/* if we are now ready, we initialize the transaction and advance
	 * action state accordingly
	 */
	if(getActionState(pWti, pThis) == ACT_STATE_RDY) {
		iRet = pThis->pMod->mod.om.beginTransaction(pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData);
		switch(iRet) {
			case RS_RET_OK:
				actionSetState(pThis, pWti, ACT_STATE_ITX);
				break;
			case RS_RET_SUSPENDED:
				actionRetry(pThis, pWti);
				break;
			case RS_RET_DISABLE_ACTION:
				actionDisable(pThis);
				break;
			default:FINALIZE;
		}
	}

finalize_it:
	RETiRet;
}


/* prepare the calling parameters for doAction()
 * rgerhards, 2009-05-07
 */
static rsRetVal
prepareDoActionParams(action_t * __restrict__ const pAction,
		      wti_t * __restrict__ const pWti,
		      smsg_t *__restrict__ const pMsg,
		      struct syslogTime *ttNow)
{
	int i;
	struct json_object *json;
	actWrkrIParams_t *iparams;
	actWrkrInfo_t *__restrict__ pWrkrInfo;
	DEFiRet;

	pWrkrInfo = &(pWti->actWrkrInfo[pAction->iActionNbr]);
	if(pAction->isTransactional) {
		CHKiRet(wtiNewIParam(pWti, pAction, &iparams));
		for(i = 0 ; i < pAction->iNumTpls ; ++i) {
			CHKiRet(tplToString(pAction->ppTpl[i], pMsg,
					    &actParam(iparams, pAction->iNumTpls, 0, i),
				            ttNow));
		}
	} else {
		for(i = 0 ; i < pAction->iNumTpls ; ++i) {
			switch(pAction->peParamPassing[i]) {
			case ACT_STRING_PASSING:
				CHKiRet(tplToString(pAction->ppTpl[i], pMsg,
					   &(pWrkrInfo->p.nontx.actParams[i]),
					   ttNow));
				break;
			/* note: ARRAY_PASSING mode has been removed in 8.26.0; if it
			 * is ever needed again, it can be found in 8.25.0.
			 * rgerhards 2017-03-06
			 */
			case ACT_MSG_PASSING:
				pWrkrInfo->p.nontx.actParams[i].param = (void*) pMsg;
				break;
			case ACT_JSON_PASSING:
				CHKiRet(tplToJSON(pAction->ppTpl[i], pMsg, &json, ttNow));
				pWrkrInfo->p.nontx.actParams[i].param = (void*) json;
				break;
			default:dbgprintf("software bug/error: unknown "
				"pAction->peParamPassing[%d] %d in prepareDoActionParams\n",
					  i, (int) pAction->peParamPassing[i]);
				break;
			}
		}
	}

finalize_it:
	RETiRet;
}


/* the #pragmas can go away when we have disable array-passing mode */
#if !defined(_AIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif
void
releaseDoActionParams(action_t *__restrict__ const pAction, wti_t *__restrict__ const pWti, int action_destruct)
{
	int j;
	actWrkrInfo_t *__restrict__ pWrkrInfo;

	pWrkrInfo = &(pWti->actWrkrInfo[pAction->iActionNbr]);
	for(j = 0 ; j < pAction->iNumTpls ; ++j) {
		if (action_destruct) {
			if (ACT_STRING_PASSING == pAction->peParamPassing[j]) {
				free(pWrkrInfo->p.nontx.actParams[j].param);
				pWrkrInfo->p.nontx.actParams[j].param = NULL;
			}
		} else {
			switch(pAction->peParamPassing[j]) {
			case ACT_ARRAY_PASSING:
				LogError(0, RS_RET_ERR, "plugin error: no longer supported "
					"ARRAY_PASSING mode is used (see action.c)");
				return;
			case ACT_JSON_PASSING:
				json_object_put((struct json_object*)
								pWrkrInfo->p.nontx.actParams[j].param);
				pWrkrInfo->p.nontx.actParams[j].param = NULL;
				break;
			case ACT_STRING_PASSING:
			case ACT_MSG_PASSING:
				/* no need to do anything with these */
				break;
			}
		}
	}

	return;
}
#if !defined(_AIX)
#pragma GCC diagnostic pop
#endif


/* This is used in resume processing. We only finally know that a resume
 * worked when we have been able to actually process a messages. As such,
 * we need to do some cleanup and status tracking in that case.
 */
static void
actionSetActionWorked(action_t *__restrict__ const pThis, wti_t *__restrict__ const pWti)
{
	setActionResumeInRow(pWti, pThis, 0);
}

static rsRetVal
handleActionExecResult(action_t *__restrict__ const pThis,
	wti_t *__restrict__ const pWti,
	const rsRetVal ret)
{
	DEFiRet;
	switch(ret) {
		case RS_RET_OK:
			actionCommitted(pThis, pWti);
			actionSetActionWorked(pThis, pWti); /* we had a successful call! */
			break;
		case RS_RET_DEFER_COMMIT:
			actionSetActionWorked(pThis, pWti); /* we had a successful call! */
			/* we are done, action state remains the same */
			break;
		case RS_RET_PREVIOUS_COMMITTED:
			/* action state remains the same, but we had a commit. */
			pWti->actWrkrInfo[pThis->iActionNbr].bHadAutoCommit = 1;
			actionSetActionWorked(pThis, pWti); /* we had a successful call! */
			break;
		case RS_RET_DISABLE_ACTION:
			actionDisable(pThis);
			break;
		case RS_RET_SUSPENDED:
			actionRetry(pThis, pWti);
			break;
		default:/* error happened - if it hits us here, we assume the message cannot
			 * be processed but an retry makes no sense. Usually, this should be
			 * return code RS_RET_DATAFAIL. -- rgerhards, 2017-10-06
			 */
			LogError(0, ret, "action '%s' (module '%s') "
				"message lost, could not be processed. Check for "
				"additional error messages before this one.",
				pThis->pszName, pThis->pMod->pszName);
			actionSetState(pThis, pWti, ACT_STATE_DATAFAIL);
			break;
	}
	iRet = getReturnCode(pThis, pWti);

	RETiRet;
}

/* call the DoAction output plugin entry point
 * rgerhards, 2008-01-28
 */
static rsRetVal
actionCallDoAction(action_t *__restrict__ const pThis,
	actWrkrIParams_t *__restrict__ const iparams,
	wti_t *__restrict__ const pWti)
{
	void *param[CONF_OMOD_NUMSTRINGS_MAXSIZE];
	int i;
	DEFiRet;

	DBGPRINTF("entering actionCalldoAction(), state: %s, actionNbr %d\n",
		  getActStateName(pThis, pWti), pThis->iActionNbr);

	pWti->actWrkrInfo[pThis->iActionNbr].bHadAutoCommit = 0;
	/* for this interface, we need to emulate the old style way
	 * of parameter passing.
	 */
	for(i = 0 ; i < pThis->iNumTpls ; ++i) {
		param[i] = actParam(iparams, pThis->iNumTpls, 0, i).param;
	}

	iRet = pThis->pMod->mod.om.doAction(param,
				            pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData);
	iRet = handleActionExecResult(pThis, pWti, iRet);
	RETiRet;
}


/* call the commitTransaction output plugin entry point */
static rsRetVal
actionCallCommitTransaction(action_t * const pThis,
	wti_t *const pWti,
	actWrkrIParams_t *__restrict__ const iparams, const int nparams)
{
	DEFiRet;

	DBGPRINTF("entering actionCallCommitTransaction[%s], state: %s, nMsgs %u\n",
		  pThis->pszName, getActStateName(pThis, pWti), nparams);

	iRet = pThis->pMod->mod.om.commitTransaction(
		    pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData,
		    iparams, nparams);
	DBGPRINTF("actionCallCommitTransaction[%s] state: %s "
		"mod commitTransaction returned %d\n",
		pThis->pszName, getActStateName(pThis, pWti), iRet);
	iRet = handleActionExecResult(pThis, pWti, iRet);
	RETiRet;
}


/* process a message
 * this readies the action and then calls doAction()
 * rgerhards, 2008-01-28
 */
static rsRetVal
actionProcessMessage(action_t * const pThis, void *actParams, wti_t * const pWti)
{
	DEFiRet;

	CHKiRet(actionPrepare(pThis, pWti));
	if(pThis->pMod->mod.om.SetShutdownImmdtPtr != NULL)
		pThis->pMod->mod.om.SetShutdownImmdtPtr(pThis->pModData, pWti->pbShutdownImmediate);
	if(getActionState(pWti, pThis) == ACT_STATE_ITX)
		CHKiRet(actionCallDoAction(pThis, actParams, pWti));

	iRet = getReturnCode(pThis, pWti);
finalize_it:
	RETiRet;
}


/* the following function uses the new-style transactional interface */
static rsRetVal
doTransaction(action_t *__restrict__ const pThis, wti_t *__restrict__ const pWti,
	actWrkrIParams_t *__restrict__ const iparams, const int nparams)
{
	actWrkrInfo_t *wrkrInfo;
	int i;
	DEFiRet;

	wrkrInfo = &(pWti->actWrkrInfo[pThis->iActionNbr]);
	if(pThis->pMod->mod.om.commitTransaction != NULL) {
		DBGPRINTF("doTransaction: have commitTransaction IF, using that, pWrkrInfo %p\n", wrkrInfo);
		CHKiRet(actionCallCommitTransaction(pThis, pWti, iparams, nparams));
	} else { /* note: this branch is for compatibility with old TX modules */
		DBGPRINTF("doTransaction: action '%s', currIParam %d\n",
			   pThis->pszName, wrkrInfo->p.tx.currIParam);
		for(i = 0 ; i < nparams ; ++i) {
			/* Note: we provide the message's base iparam - actionProcessMessage()
			 * uses this as *base* address.
			 */
			iRet = actionProcessMessage(pThis,
				&actParam(iparams, pThis->iNumTpls, i, 0), pWti);
			DBGPRINTF("doTransaction: action %d, processing msg %d, result %d\n",
			   pThis->iActionNbr, i,iRet);
			if(iRet == RS_RET_SUSPENDED) {
				--i; /* we need to re-submit */
				/* note: we are suspended and need to retry. In order not to
				 * hammer the CPU, we now do a voluntarly wait of 1 second.
				 * The rest will be handled by the standard retry handler.
				 */
				srSleep(1, 0);
			} else if(iRet != RS_RET_DEFER_COMMIT && iRet != RS_RET_PREVIOUS_COMMITTED &&
			   iRet != RS_RET_OK) {
				FINALIZE; /* let upper peer handle the error condition! */
			}
		}
	}
finalize_it:
	if(iRet == RS_RET_DEFER_COMMIT || iRet == RS_RET_PREVIOUS_COMMITTED)
		iRet = RS_RET_OK; /* this is expected for transactional action! */
	RETiRet;
}


/* Commit try committing (do not handle retry processing and such) */
static rsRetVal ATTR_NONNULL()
actionTryCommit(action_t *__restrict__ const pThis, wti_t *__restrict__ const pWti,
	actWrkrIParams_t *__restrict__ const iparams, const int nparams)
{
	DEFiRet;

	DBGPRINTF("actionTryCommit[%s] enter\n", pThis->pszName);
	CHKiRet(actionPrepare(pThis, pWti));

	CHKiRet(doTransaction(pThis, pWti, iparams, nparams));

	if(getActionState(pWti, pThis) == ACT_STATE_ITX) {
		iRet = pThis->pMod->mod.om.endTransaction(pWti->actWrkrInfo[pThis->iActionNbr].actWrkrData);
		switch(iRet) {
			case RS_RET_OK:
				actionCommitted(pThis, pWti);
				break;
			case RS_RET_SUSPENDED:
				actionRetry(pThis, pWti);
				break;
			case RS_RET_DISABLE_ACTION:
				actionDisable(pThis);
				break;
			case RS_RET_DEFER_COMMIT:
				DBGPRINTF("output plugin error: endTransaction() returns RS_RET_DEFER_COMMIT "
					  "- ignored\n");
				actionCommitted(pThis, pWti);
				break;
			case RS_RET_PREVIOUS_COMMITTED:
				DBGPRINTF("output plugin error: endTransaction() returns RS_RET_PREVIOUS_COMMITTED "
					  "- ignored\n");
				actionCommitted(pThis, pWti);
				break;
			default:/* permanent failure of this message - no sense in retrying. This is
				 * not yet handled (but easy TODO)
				 */
				DBGPRINTF("action[%s]: actionTryCommit receveived iRet %d\n",
					pThis->pszName, iRet);
				FINALIZE;
		}
	}
	iRet = getReturnCode(pThis, pWti);

finalize_it:
	RETiRet;
}

/* If a transcation failed, we write the error file (if configured).
 */
static void ATTR_NONNULL()
actionWriteErrorFile(action_t *__restrict__ const pThis, const rsRetVal ret,
	actWrkrIParams_t *__restrict__ const iparams, const int nparams)
{
	fjson_object *etry=NULL;
	int bNeedUnlock = 0;

	if(pThis->pszErrFile == NULL) {
		DBGPRINTF("action %s: commit failed, no error file set, silently "
			"discarding %d messages\n", pThis->pszName, nparams);
		goto done;
	}

	DBGPRINTF("action %d commit failed, writing %u messages (%d tpls) to error file\n",
		pThis->iActionNbr, nparams, pThis->iNumTpls);

	pthread_mutex_lock(&pThis->mutErrFile);
	bNeedUnlock = 1;

	if(pThis->fdErrFile == -1) {
		pThis->fdErrFile = open(pThis->pszErrFile,
					O_WRONLY|O_CREAT|O_APPEND|O_LARGEFILE|O_CLOEXEC,
					S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
		if(pThis->fdErrFile == -1) {
			LogError(errno, RS_RET_ERR, "action %s: error opening error file %s",
				pThis->pszName, pThis->pszErrFile);
			goto done;
		}
	}

	for(int i = 0 ; i < nparams ; ++i) {
		if((etry = fjson_object_new_object()) == NULL)
			goto done;
		fjson_object_object_add(etry, "action", fjson_object_new_string((char*)pThis->pszName));
		fjson_object_object_add(etry, "status", fjson_object_new_int(ret));
		for(int j = 0 ; j < pThis->iNumTpls ; ++j) {
			char tplname[20];
			snprintf(tplname, sizeof(tplname), "template%d", j);
			tplname[sizeof(tplname)-1] = '\0';
			fjson_object_object_add(etry, tplname,
				fjson_object_new_string((char*)actParam(iparams, 1, i, j).param));
		}

		char *const rendered = strdup((char*)fjson_object_to_json_string(etry));
		if(rendered == NULL)
			goto done;
		const size_t toWrite = strlen(rendered) + 1;
		/* note: we use the '\0' inside the string to store a LF - we do not
		 * otherwise need it and it safes us a copy/realloc.
		 */
		rendered[toWrite-1] = '\n'; /* NO LONGER A STRING! */
		const ssize_t wrRet = write(pThis->fdErrFile, rendered, toWrite);
		if(wrRet != (ssize_t) toWrite) {
			LogError(errno, RS_RET_IO_ERROR,
				"action %s: error writing errorFile %s, write returned %lld",
				pThis->pszName, pThis->pszErrFile, (long long) wrRet);
		}
		free(rendered);

		fjson_object_put(etry);
		etry = NULL;
	}
done:
	if(bNeedUnlock) {
		pthread_mutex_unlock(&pThis->mutErrFile);
	}
	fjson_object_put(etry);
	return;
}


static rsRetVal
actionTryRemoveHardErrorsFromBatch(action_t *__restrict__ const pThis, wti_t *__restrict__ const pWti,
	actWrkrIParams_t *const new_iparams, unsigned *new_nMsgs)
{
	actWrkrInfo_t *const wrkrInfo = &(pWti->actWrkrInfo[pThis->iActionNbr]);
	const unsigned nMsgs = wrkrInfo->p.tx.currIParam;
	actWrkrIParams_t oneParamSet[CONF_OMOD_NUMSTRINGS_MAXSIZE];
	rsRetVal ret;
	DEFiRet;

	*new_nMsgs = 0;
	for(unsigned i = 0 ; i < nMsgs ; ++i) {
		setActionResumeInRow(pWti, pThis, 0); // make sure we do not trigger OK-as-SUSPEND handling
		memcpy(&oneParamSet, &actParam(wrkrInfo->p.tx.iparams, pThis->iNumTpls, i, 0),
			sizeof(actWrkrIParams_t) * pThis->iNumTpls);
		ret = actionTryCommit(pThis, pWti, oneParamSet, 1);
		if(ret == RS_RET_SUSPENDED) {
			memcpy(new_iparams + *new_nMsgs, &oneParamSet,
				sizeof(actWrkrIParams_t) * pThis->iNumTpls);
			++(*new_nMsgs);
		} else if(ret != RS_RET_OK) {
			actionWriteErrorFile(pThis, ret, oneParamSet, 1);
		}
	}
	RETiRet;
}

/* Note: we currently need to return an iRet, as this is used in
 * direct mode. TODO: However, it may be worth further investigating this,
 * as it looks like there is no ultimate consumer of this code.
 * rgerhards, 2013-11-06
 */
static rsRetVal ATTR_NONNULL()
actionCommit(action_t *__restrict__ const pThis, wti_t *__restrict__ const pWti)
{
	actWrkrInfo_t *const wrkrInfo = &(pWti->actWrkrInfo[pThis->iActionNbr]);
	/* Variables that permit us to override the batch of messages */
	unsigned nMsgs;
	actWrkrIParams_t *iparams = NULL;
	int needfree_iparams = 0; // work-around for clang static analyzer false positive
	DEFiRet;

	DBGPRINTF("actionCommit[%s]: enter, %d msgs\n", pThis->pszName, wrkrInfo->p.tx.currIParam);
	if(!pThis->isTransactional ||
	   pWti->actWrkrInfo[pThis->iActionNbr].p.tx.currIParam == 0 ||
	   getActionState(pWti, pThis) == ACT_STATE_SUSP
	   ) {
		FINALIZE;
	}
	DBGPRINTF("actionCommit[%s]: processing...\n", pThis->pszName);

	/* we now do one try at commiting the whole batch. Usually, this will
	 * succeed. If so, we are happy and done. If not, we dig into the details
	 * of finding out if we have a non-temporary error and try to handle this
	 * as well as retry processing. Due to this logic we do a bit more retries
	 * than configured (if temporary failure), but this unavoidable and should
	 * do no real harm. - rgerhards, 2017-10-06
	 */
	iRet = actionTryCommit(pThis, pWti, wrkrInfo->p.tx.iparams, wrkrInfo->p.tx.currIParam);
DBGPRINTF("actionCommit[%s]: return actionTryCommit %d\n", pThis->pszName, iRet);
	if(iRet == RS_RET_OK) {
		FINALIZE;
	}

	/* check if this was a single-message batch. If it had a datafail error, we
	 * are done. If it is a multi-message batch, we need to sort out the individual
	 * message states.
	 */
	if(wrkrInfo->p.tx.currIParam == 1) {
		needfree_iparams = 0;
		iparams = wrkrInfo->p.tx.iparams;
		nMsgs = wrkrInfo->p.tx.currIParam;
		if(iRet == RS_RET_DATAFAIL) {
			FINALIZE;
		}
	} else {
		DBGPRINTF("actionCommit[%s]: somewhat unhappy, full batch of %d msgs returned "
			"status %d. Trying messages as individual actions.\n",
			pThis->pszName, wrkrInfo->p.tx.currIParam, iRet);
		CHKmalloc(iparams = malloc(sizeof(actWrkrIParams_t) * pThis->iNumTpls
			* wrkrInfo->p.tx.currIParam));
		needfree_iparams = 1;
		actionTryRemoveHardErrorsFromBatch(pThis, pWti, iparams, &nMsgs);
	}

	if(nMsgs == 0) {
		ABORT_FINALIZE(RS_RET_OK); // here, we consider everyting OK
	}

	/* We still have some messages with suspend error. So now let's do our
	 * "regular" retry and suspend processing.
	 */
	DBGPRINTF("actionCommit[%s]: unhappy, we still have %d uncommited messages.\n",
		pThis->pszName, nMsgs);
	int bDone = 0;
	do {
		iRet = actionTryCommit(pThis, pWti, iparams, nMsgs);
		DBGPRINTF("actionCommit[%s]: in retry loop, iRet %d\n",
			pThis->pszName, iRet);
		if(iRet == RS_RET_FORCE_TERM) {
			ABORT_FINALIZE(RS_RET_FORCE_TERM);
		} else if(iRet == RS_RET_SUSPENDED) {
			iRet = actionDoRetry(pThis, pWti);
			DBGPRINTF("actionCommit[%s]: actionDoRetry returned %d\n",
				pThis->pszName, iRet);
			if(iRet == RS_RET_FORCE_TERM) {
				ABORT_FINALIZE(RS_RET_FORCE_TERM);
			} else if(iRet != RS_RET_OK) {
				actionWriteErrorFile(pThis, iRet, iparams, nMsgs);
				bDone = 1;
			}
			continue;
		} else if(iRet == RS_RET_OK ||
		          iRet == RS_RET_SUSPENDED ||
			  iRet == RS_RET_ACTION_FAILED) {
			bDone = 1;
		}
		if(getActionState(pWti, pThis) == ACT_STATE_RDY  ||
		   getActionState(pWti, pThis) == ACT_STATE_SUSP) {
			bDone = 1;
		}
	} while(!bDone);
finalize_it:
	DBGPRINTF("actionCommit[%s]: done, iRet %d\n", pThis->pszName, iRet);
	if(needfree_iparams) {
		free(iparams);
	}
	wrkrInfo->p.tx.currIParam = 0; /* reset to beginning */
	RETiRet;
}

/* Commit all active transactions in *DIRECT mode* */
void ATTR_NONNULL()
actionCommitAllDirect(wti_t *__restrict__ const pWti)
{
	int i;
	action_t *pAction;

	for(i = 0 ; i < iActionNbr ; ++i) {
		pAction = pWti->actWrkrInfo[i].pAction;
		if(pAction == NULL)
			continue;
		DBGPRINTF("actionCommitAllDirect: action %d, state %u, nbr to commit %d "
			  "isTransactional %d\n",
			  i, getActionStateByNbr(pWti, i), pWti->actWrkrInfo->p.tx.currIParam,
			  pAction->isTransactional);
		if(pAction->pQueue->qType == QUEUETYPE_DIRECT)
			actionCommit(pAction, pWti);
	}
}

/* process a single message. This is both called if we run from the
 * cosumer side of an action queue as well as directly from the main
 * queue thread if the action queue is set to "direct".
 */
static rsRetVal
processMsgMain(action_t *__restrict__ const pAction,
	wti_t *__restrict__ const pWti,
	smsg_t *__restrict__ const pMsg,
	struct syslogTime *ttNow)
{
	DEFiRet;

	CHKiRet(prepareDoActionParams(pAction, pWti, pMsg, ttNow));

	if(pAction->isTransactional) {
		pWti->actWrkrInfo[pAction->iActionNbr].pAction = pAction;
		DBGPRINTF("action '%s': is transactional - executing in commit phase\n", pAction->pszName);
		actionPrepare(pAction, pWti);
		iRet = getReturnCode(pAction, pWti);
		FINALIZE;
	}

	iRet = actionProcessMessage(pAction,
				    pWti->actWrkrInfo[pAction->iActionNbr].p.nontx.actParams,
				    pWti);
	if(pAction->bNeedReleaseBatch)
		releaseDoActionParams(pAction, pWti, 0);
finalize_it:
	if(iRet == RS_RET_OK) {
		if(pWti->execState.bDoAutoCommit)
			iRet = actionCommit(pAction, pWti);
	}
	RETiRet;
}

/* This entry point is called by the ACTION queue (not main queue!)
 */
static rsRetVal
processBatchMain(void *__restrict__ const pVoid,
	batch_t *__restrict__ const pBatch,
	wti_t *__restrict__ const pWti)
{
	action_t *__restrict__ const pAction = (action_t*__restrict__ const) pVoid;
	int i;
	struct syslogTime ttNow;
	DEFiRet;

	wtiResetExecState(pWti, pBatch);
	/* indicate we have not yet read the date */
	ttNow.year = 0;

	for(i = 0 ; i < batchNumMsgs(pBatch) && !*pWti->pbShutdownImmediate ; ++i) {
		if(batchIsValidElem(pBatch, i)) {
			/* we do not check error state below, because aborting would be
			 * more harmful than continuing.
			 */
			rsRetVal localRet = processMsgMain(pAction, pWti, pBatch->pElem[i].pMsg, &ttNow);
			DBGPRINTF("processBatchMain: i %d, processMsgMain iRet %d\n", i, localRet);
			if(   localRet == RS_RET_OK
			   || localRet == RS_RET_DEFER_COMMIT
			   || localRet == RS_RET_ACTION_FAILED
			   || localRet == RS_RET_PREVIOUS_COMMITTED ) {
				batchSetElemState(pBatch, i, BATCH_STATE_COMM);
				DBGPRINTF("processBatchMain: i %d, COMM state set\n", i);
			}
		}
	}

	iRet = actionCommit(pAction, pWti);
	RETiRet;
}


/* remove an action worker instance from our table of
 * workers. To be called from worker handler (wti).
 */
void
actionRemoveWorker(action_t *const __restrict__ pAction,
	void *const __restrict__ actWrkrData)
{
	pthread_mutex_lock(&pAction->mutWrkrDataTable);
	pAction->nWrkr--;
	for(int w = 0 ; w < pAction->wrkrDataTableSize ; ++w) {
		if(pAction->wrkrDataTable[w] == actWrkrData) {
			pAction->wrkrDataTable[w] = NULL;
			break; /* done */
		}
	}
	pthread_mutex_unlock(&pAction->mutWrkrDataTable);
}


/* call the HUP handler for a given action, if such a handler is defined.
 * Note that the action must be able to service HUP requests concurrently
 * to any current doAction() processing.
 */
rsRetVal
actionCallHUPHdlr(action_t * const pAction)
{
	DEFiRet;

	ASSERT(pAction != NULL);
	DBGPRINTF("Action %p checks HUP hdlr, act level: %p, wrkr level %p\n",
		pAction, pAction->pMod->doHUP, pAction->pMod->doHUPWrkr);

	if(pAction->pMod->doHUP != NULL) {
		CHKiRet(pAction->pMod->doHUP(pAction->pModData));
	}

	if(pAction->pMod->doHUPWrkr != NULL) {
		pthread_mutex_lock(&pAction->mutWrkrDataTable);
		for(int i = 0 ; i < pAction->wrkrDataTableSize ; ++i) {
			dbgprintf("HUP: table entry %d: %p %s\n", i,
				pAction->wrkrDataTable[i],
				pAction->wrkrDataTable[i] == NULL ? "[unused]" : "");
			if(pAction->wrkrDataTable[i] != NULL) {
				const rsRetVal localRet
					= pAction->pMod->doHUPWrkr(pAction->wrkrDataTable[i]);
				if(localRet != RS_RET_OK) {
					DBGPRINTF("HUP handler returned error state %d - "
						  "ignored\n", localRet);
				}
			}
		}
		pthread_mutex_unlock(&pAction->mutWrkrDataTable);
	}

finalize_it:
	RETiRet;
}


/* set the action message queue mode
 * TODO: probably move this into queue object, merge with MainMsgQueue!
 * rgerhards, 2008-01-28
 */
static rsRetVal setActionQueType(void __attribute__((unused)) *pVal, uchar *pszType)
{
	DEFiRet;

	if (!strcasecmp((char *) pszType, "fixedarray")) {
		cs.ActionQueType = QUEUETYPE_FIXED_ARRAY;
		DBGPRINTF("action queue type set to FIXED_ARRAY\n");
	} else if (!strcasecmp((char *) pszType, "linkedlist")) {
		cs.ActionQueType = QUEUETYPE_LINKEDLIST;
		DBGPRINTF("action queue type set to LINKEDLIST\n");
	} else if (!strcasecmp((char *) pszType, "disk")) {
		cs.ActionQueType = QUEUETYPE_DISK;
		DBGPRINTF("action queue type set to DISK\n");
	} else if (!strcasecmp((char *) pszType, "direct")) {
		cs.ActionQueType = QUEUETYPE_DIRECT;
		DBGPRINTF("action queue type set to DIRECT (no queueing at all)\n");
	} else {
		LogError(0, RS_RET_INVALID_PARAMS, "unknown actionqueue parameter: %s", (char *) pszType);
		iRet = RS_RET_INVALID_PARAMS;
	}
	d_free(pszType); /* no longer needed */

	RETiRet;
}


/* This submits the message to the action queue in case we do NOT need to handle repeat
 * message processing. That case permits us to gain lots of freedom during processing
 * and thus speed. This is also utilized to submit messages in more complex cases once
 * the complex logic has been applied ;)
 * rgerhards, 2010-06-08
 */
static rsRetVal
doSubmitToActionQ(action_t * const pAction, wti_t * const pWti, smsg_t *pMsg)
{
	struct syslogTime ttNow; // TODO: think if we can buffer this in pWti
	DEFiRet;

	DBGPRINTF("action '%s': called, logging to %s (susp %d/%d, direct q %d)\n",
		pAction->pszName, module.GetStateName(pAction->pMod),
		pAction->bExecWhenPrevSusp, pWti->execState.bPrevWasSuspended,
		pAction->pQueue->qType == QUEUETYPE_DIRECT);

	if(   pAction->bExecWhenPrevSusp
	   && !pWti->execState.bPrevWasSuspended) {
		DBGPRINTF("action '%s': NOT executing, as previous action was "
			  "not suspended\n", pAction->pszName);
		FINALIZE;
	}

	STATSCOUNTER_INC(pAction->ctrProcessed, pAction->mutCtrProcessed);
	if(pAction->pQueue->qType == QUEUETYPE_DIRECT) {
		ttNow.year = 0;
		iRet = processMsgMain(pAction, pWti, pMsg, &ttNow);
	} else {/* in this case, we do single submits to the queue.
		 * TODO: optimize this, we may do at least a multi-submit!
		 */
		iRet = qqueueEnqMsg(pAction->pQueue, eFLOWCTL_NO_DELAY,
			pAction->bCopyMsg ? MsgDup(pMsg) : MsgAddRef(pMsg));
	}
	pWti->execState.bPrevWasSuspended
		= (iRet == RS_RET_SUSPENDED || iRet == RS_RET_ACTION_FAILED);

	if (iRet == RS_RET_ACTION_FAILED)	/* Increment failed counter */
		STATSCOUNTER_INC(pAction->ctrFail, pAction->mutCtrFail);

	DBGPRINTF("action '%s': set suspended state to %d\n",
		pAction->pszName, pWti->execState.bPrevWasSuspended);

finalize_it:
	RETiRet;
}


/* This function builds up a batch of messages to be (later)
 * submitted to the action queue.
 * Important: this function MUST not be called with messages that are to
 * be discarded due to their "prevWasSuspended" state. It will not check for
 * this and submit all messages to the queue for execution. So these must
 * be filtered out before calling us (what is done currently!).
 */
rsRetVal
actionWriteToAction(action_t * const pAction, smsg_t *pMsg, wti_t * const pWti)
{
	DEFiRet;

	/* first, we check if the action should actually be called. The action-specific
	 * $ActionExecOnlyEveryNthTime permits us to execute an action only every Nth
	 * time. So we need to check if we need to drop the (otherwise perfectly executable)
	 * action for this reason. Note that in case we need to drop it, we return RS_RET_OK
	 * as the action was properly "passed to execution" from the upper layer's point
	 * of view. -- rgerhards, 2008-08-07.
	 */
	if(pAction->iExecEveryNthOccur > 1) {
		/* we need to care about multiple occurences */
		if(   pAction->iExecEveryNthOccurTO > 0
		   && (getActNow(pAction) - pAction->tLastOccur) > pAction->iExecEveryNthOccurTO) {
		  	DBGPRINTF("n-th occurence handling timed out (%d sec), restarting from 0\n",
				  (int) (getActNow(pAction) - pAction->tLastOccur));
			pAction->iNbrNoExec = 0;
			pAction->tLastOccur = getActNow(pAction);
		   }
		if(pAction->iNbrNoExec < pAction->iExecEveryNthOccur - 1) {
			++pAction->iNbrNoExec;
			DBGPRINTF("action %p passed %d times to execution - less than neded - discarding\n",
			  pAction, pAction->iNbrNoExec);
			FINALIZE;
		} else {
			pAction->iNbrNoExec = 0; /* we execute the action now, so the number of no execs is down to */
		}
	}

	DBGPRINTF("Called action(complex case), logging to %s\n", module.GetStateName(pAction->pMod));

	/* now check if we need to drop the message because otherwise the action would be too
	 * frequently called. -- rgerhards, 2008-04-08
	 * Note that the check for "pAction->iSecsExecOnceInterval > 0" is not necessary from
	 * a purely logical point of view. However, if safes us to check the system time in
	 * (those common) cases where ExecOnceInterval is not used. -- rgerhards, 2008-09-16
	 */
	if(pAction->iSecsExecOnceInterval > 0 &&
	   pAction->iSecsExecOnceInterval + pAction->tLastExec > getActNow(pAction)) {
		/* in this case we need to discard the message - its not yet time to exec the action */
		DBGPRINTF("action not yet ready again to be executed, onceInterval %d, tCurr %d, tNext %d\n",
			  (int) pAction->iSecsExecOnceInterval, (int) getActNow(pAction),
			  (int) (pAction->iSecsExecOnceInterval + pAction->tLastExec));
		FINALIZE;
	}

	/* we use reception time, not dequeue time - this is considered more appropriate and also faster ;)
	 * rgerhards, 2008-09-17 */
	pAction->tLastExec = getActNow(pAction); /* re-init time flags */
	pAction->f_time = pMsg->ttGenTime;

	/* When we reach this point, we have a valid, non-disabled action.
	 * So let's enqueue our message for execution. -- rgerhards, 2007-07-24
	 */
	iRet = doSubmitToActionQ(pAction, pWti, pMsg);

finalize_it:
	RETiRet;
}


/* Call configured action, most complex case with all features supported (and thus slow).
 * rgerhards, 2010-06-08
 */
#ifndef _AIX
#pragma GCC diagnostic ignored "-Wempty-body"
#endif
static rsRetVal
doSubmitToActionQComplex(action_t * const pAction, wti_t * const pWti, smsg_t *pMsg)
{
	DEFiRet;

	d_pthread_mutex_lock(&pAction->mutAction);
	pthread_cleanup_push(mutexCancelCleanup, &pAction->mutAction);
	DBGPRINTF("Called action %p (complex case), logging to %s\n",
		  pAction, module.GetStateName(pAction->pMod));

	pAction->tActNow = -1; /* we do not yet know our current time (clear prev. value) */
	// TODO: can we optimize the "now" handling again (was batch, I guess...)?

	/* don't output marks to recently written outputs */
	if(pAction->bWriteAllMarkMsgs == 0
	   && (pMsg->msgFlags & MARK) && (getActNow(pAction) - pAction->f_time) < MarkInterval / 2) {
		ABORT_FINALIZE(RS_RET_OK);
	}

	/* call the output driver */
	iRet = actionWriteToAction(pAction, pMsg, pWti);

finalize_it:
	d_pthread_mutex_unlock(&pAction->mutAction);
	pthread_cleanup_pop(0); /* remove mutex cleanup handler */

	RETiRet;
}
#ifndef _AIX
#pragma GCC diagnostic warning "-Wempty-body"
#endif


/* helper to activateActions, it activates a specific action.
 */
DEFFUNC_llExecFunc(doActivateActions)
{
	rsRetVal localRet;
	action_t * const pThis = (action_t*) pData;
	BEGINfunc
	localRet = qqueueStart(pThis->pQueue);
	if(localRet != RS_RET_OK) {
		LogError(0, localRet, "error starting up action queue");
		if(localRet == RS_RET_FILE_PREFIX_MISSING) {
			LogError(0, localRet, "file prefix (work directory?) "
					"is missing");
		}
		actionDisable(pThis);
	}
	DBGPRINTF("Action %s[%p]: queue %p started\n", modGetName(pThis->pMod),
		  pThis, pThis->pQueue);
	ENDfunc
	return RS_RET_OK; /* we ignore errors, we can not do anything either way */
}


/* This function "activates" the action after privileges have been dropped. Currently,
 * this means that the queues are started.
 * rgerhards, 2011-05-02
 */
rsRetVal
activateActions(void)
{
	DEFiRet;
	iRet = ruleset.IterateAllActions(ourConf, doActivateActions, NULL);
	RETiRet;
}



/* This submits the message to the action queue in case where we need to handle
 * bWriteAllMarkMessage == RSFALSE only. Note that we use a non-blocking CAS loop
 * for the synchronization. Here, we just modify the filter condition to be false when
 * a mark message must not be written. However, in this case we must save the previous
 * filter as we may need it in the next action (potential future optimization: check if this is
 * the last action TODO).
 * rgerhards, 2010-06-08
 */
static rsRetVal
doSubmitToActionQNotAllMark(action_t * const pAction, wti_t * const pWti, smsg_t * const pMsg)
{
	int doProcess = 1;
	time_t lastAct;
	DEFiRet;

	/* TODO: think about the whole logic. If messages come in out of order, things
	 * tend to become a bit unreliable. On the other hand, this only happens if we have
	 * very high traffic, in which this use case here is not really affected (as the
	 * MarkInterval is pretty corase).
	 */
	/* CAS loop, we write back a bit early, but that's OK... */
	/* we use reception time, not dequeue time - this is considered more appropriate and
	 * also faster ;) -- rgerhards, 2008-09-17 */
	do {
		lastAct = pAction->f_time;
		if(pMsg->msgFlags & MARK) {
			if((pMsg->ttGenTime - lastAct) < MarkInterval / 2) {
				doProcess = 0;
				DBGPRINTF("action was recently called, ignoring mark message\n");
				break; /* do not update timestamp for non-written mark messages */
			}
		}
	} while(ATOMIC_CAS_time_t(&pAction->f_time, lastAct,
		pMsg->ttGenTime, &pAction->mutCAS) == 0);

	if(doProcess) {
		DBGPRINTF("Called action(NotAllMark), processing via '%s'\n",
			  module.GetStateName(pAction->pMod));
		iRet = doSubmitToActionQ(pAction, pWti, pMsg);
	}

	RETiRet;
}


/* apply all params from param block to action. This supports the v6 config system.
 * Defaults must have been set appropriately during action construct!
 * rgerhards, 2011-08-01
 */
static rsRetVal
actionApplyCnfParam(action_t * const pAction, struct cnfparamvals * const pvals)
{
	int i;
	
	for(i = 0 ; i < pblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(pblk.descr[i].name, "name")) {
			pAction->pszName = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblk.descr[i].name, "type")) {
			continue; /* this is handled seperately during module select! */
		} else if(!strcmp(pblk.descr[i].name, "action.errorfile")) {
			pAction->pszErrFile = es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblk.descr[i].name, "action.writeallmarkmessages")) {
			pAction->bWriteAllMarkMsgs = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.execonlyeverynthtime")) {
			pAction->iExecEveryNthOccur = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.execonlyeverynthtimetimeout")) {
			pAction->iExecEveryNthOccurTO = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.execonlyonceeveryinterval")) {
			pAction->iSecsExecOnceInterval = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.execonlywhenpreviousissuspended")) {
			pAction->bExecWhenPrevSusp = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.repeatedmsgcontainsoriginalmsg")) {
			pAction->bRepMsgHasMsg = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.resumeretrycount")) {
			pAction->iResumeRetryCount = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.reportsuspension")) {
			pAction->bReportSuspension = (int) pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.reportsuspensioncontinuation")) {
			pAction->bReportSuspensionCont = (int) pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.copymsg")) {
			pAction->bCopyMsg = (int) pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "action.resumeinterval")) {
			pAction->iResumeInterval = pvals[i].val.d.n;
		} else {
			dbgprintf("action: program error, non-handled "
			  "param '%s'\n", pblk.descr[i].name);
		}
	}
	return RS_RET_OK;
}


/* add an Action to the current selector
 * The pOMSR is freed, as it is not needed after this function.
 * Note: this function pulls global data that specifies action config state.
 * rgerhards, 2007-07-27
 */
rsRetVal
addAction(action_t **ppAction, modInfo_t *pMod, void *pModData,
	  omodStringRequest_t *pOMSR, struct cnfparamvals *actParams,
	  struct nvlst * const lst)
{
	DEFiRet;
	int i;
	int iTplOpts;
	uchar *pTplName;
	action_t *pAction;
	char errMsg[512];

	assert(ppAction != NULL);
	assert(pMod != NULL);
	assert(pOMSR != NULL);
	DBGPRINTF("Module %s processes this action.\n", module.GetName(pMod));

	CHKiRet(actionConstruct(&pAction)); /* create action object first */
	pAction->pMod = pMod;
	pAction->pModData = pModData;
	if(actParams == NULL) { /* use legacy systemn */
		pAction->pszName = cs.pszActionName;
		pAction->iResumeInterval = cs.glbliActionResumeInterval;
		pAction->iResumeRetryCount = cs.glbliActionResumeRetryCount;
		pAction->bWriteAllMarkMsgs = cs.bActionWriteAllMarkMsgs;
		pAction->bExecWhenPrevSusp = cs.bActExecWhenPrevSusp;
		pAction->iSecsExecOnceInterval = cs.iActExecOnceInterval;
		pAction->iExecEveryNthOccur = cs.iActExecEveryNthOccur;
		pAction->iExecEveryNthOccurTO = cs.iActExecEveryNthOccurTO;
		pAction->bRepMsgHasMsg = cs.bActionRepMsgHasMsg;
		cs.iActExecEveryNthOccur = 0; /* auto-reset */
		cs.iActExecEveryNthOccurTO = 0; /* auto-reset */
		cs.bActionWriteAllMarkMsgs = 1; /* auto-reset */
		cs.pszActionName = NULL;	/* free again! */
	} else {
		actionApplyCnfParam(pAction, actParams);
	}

	/* check if we can obtain the template pointers - TODO: move to separate function? */
	pAction->iNumTpls = OMSRgetEntryCount(pOMSR);
	assert(pAction->iNumTpls >= 0); /* only debug check because this "can not happen" */
	/* please note: iNumTpls may validly be zero. This is the case if the module
	 * does not request any templates. This sounds unlikely, but an actual example is
	 * the discard action, which does not require a string. -- rgerhards, 2007-07-30
	 */
	if(pAction->iNumTpls > 0) {
		/* we first need to create the template arrays */
		CHKmalloc(pAction->ppTpl = (struct template **)calloc(pAction->iNumTpls, sizeof(struct template *)));
		CHKmalloc(pAction->peParamPassing = (paramPassing_t*)calloc(pAction->iNumTpls,
			sizeof(paramPassing_t)));
	}
	
	pAction->bUsesMsgPassingMode = 0;
	pAction->bNeedReleaseBatch = 0;
	for(i = 0 ; i < pAction->iNumTpls ; ++i) {
		CHKiRet(OMSRgetEntry(pOMSR, i, &pTplName, &iTplOpts));
		/* Ok, we got everything, so it now is time to look up the template
		 * (Hint: templates MUST be defined before they are used!)
		 */
		if(!(iTplOpts & OMSR_TPL_AS_MSG)) {
		   	if((pAction->ppTpl[i] =
				tplFind(ourConf, (char*)pTplName, strlen((char*)pTplName))) == NULL) {
				snprintf(errMsg, sizeof(errMsg),
					 " Could not find template %d '%s' - action disabled",
					 i, pTplName);
				errno = 0;
				LogError(0, RS_RET_NOT_FOUND, "%s", errMsg);
				ABORT_FINALIZE(RS_RET_NOT_FOUND);
			}
			/* check required template options */
			if(   (iTplOpts & OMSR_RQD_TPL_OPT_SQL)
			   && (pAction->ppTpl[i]->optFormatEscape == 0)) {
				errno = 0;
				LogError(0, RS_RET_RQD_TPLOPT_MISSING, "Action disabled."
						" To use this action, you have to specify "
						"the SQL or stdSQL option in your template!\n");
				ABORT_FINALIZE(RS_RET_RQD_TPLOPT_MISSING);
			}
		}

		/* set parameter-passing mode */
		if(iTplOpts & OMSR_TPL_AS_ARRAY) {
			ABORT_FINALIZE(RS_RET_ERR);
		} else if(iTplOpts & OMSR_TPL_AS_MSG) {
			pAction->peParamPassing[i] = ACT_MSG_PASSING;
			pAction->bUsesMsgPassingMode = 1;
		} else if(iTplOpts & OMSR_TPL_AS_JSON) {
			pAction->peParamPassing[i] = ACT_JSON_PASSING;
			pAction->bNeedReleaseBatch = 1;
		} else {
			pAction->peParamPassing[i] = ACT_STRING_PASSING;
		}

		DBGPRINTF("template: '%s' assigned\n", pTplName);
	}

	pAction->pMod = pMod;
	pAction->pModData = pModData;

	CHKiRet(actionConstructFinalize(pAction, lst));
	
	*ppAction = pAction; /* finally store the action pointer */

finalize_it:
	if(iRet == RS_RET_OK)
		iRet = OMSRdestruct(pOMSR);
	else {
		/* do not overwrite error state! */
		OMSRdestruct(pOMSR);
		if(pAction != NULL)
			actionDestruct(pAction);
	}

	RETiRet;
}


/* Reset config variables to default values.
 * rgerhards, 2009-11-12
 */
static rsRetVal
resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	cs.iActExecOnceInterval = 0;
	cs.bActExecWhenPrevSusp = 0;
	return RS_RET_OK;
}


/* initialize (current) config variables.
 * Used at program start and when a new scope is created.
 */
static void
initConfigVariables(void)
{
	cs.bActionWriteAllMarkMsgs = 1;
	cs.glbliActionResumeRetryCount = 0;
	cs.bActExecWhenPrevSusp = 0;
	cs.iActExecOnceInterval = 0;
	cs.iActExecEveryNthOccur = 0;
	cs.iActExecEveryNthOccurTO = 0;
	cs.glbliActionResumeInterval = 30;
	cs.glbliActionResumeRetryCount = 0;
	cs.bActionRepMsgHasMsg = 0;
	if(cs.pszActionName != NULL) {
		free(cs.pszActionName);
		cs.pszActionName = NULL;
	}
	actionResetQueueParams();
}


rsRetVal
actionNewInst(struct nvlst *lst, action_t **ppAction)
{
	struct cnfparamvals *paramvals;
	modInfo_t *pMod;
	uchar *cnfModName = NULL;
	omodStringRequest_t *pOMSR;
	void *pModData;
	action_t *pAction;
	DEFiRet;

	paramvals = nvlstGetParams(lst, &pblk, NULL);
	if(paramvals == NULL) {
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);
	}
	dbgprintf("action param blk after actionNewInst:\n");
	cnfparamsPrint(&pblk, paramvals);
	cnfModName = (uchar*)es_str2cstr(paramvals[cnfparamGetIdx(&pblk, ("type"))].val.d.estr, NULL);
	if((pMod = module.FindWithCnfName(loadConf, cnfModName, eMOD_OUT)) == NULL) {
		LogError(0, RS_RET_MOD_UNKNOWN, "module name '%s' is unknown", cnfModName);
		ABORT_FINALIZE(RS_RET_MOD_UNKNOWN);
	}
	CHKiRet(pMod->mod.om.newActInst(cnfModName, lst, &pModData, &pOMSR));

	if((iRet = addAction(&pAction, pMod, pModData, pOMSR, paramvals, lst)) == RS_RET_OK) {
		/* check if the module is compatible with select features
		 * (currently no such features exist) */
		loadConf->actions.nbrActions++;	/* one more active action! */
		*ppAction = pAction;
	} else {
		// TODO: cleanup
	}

finalize_it:
	free(cnfModName);
	cnfparamvalsDestruct(paramvals, &pblk);
	RETiRet;
}

rsRetVal actionClassInit(void)
{
	DEFiRet;
	/* request objects we use */
	CHKiRet(objGetObjInterface(&obj)); /* this provides the root pointer for all other queries */
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(module, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));

	CHKiRet(regCfSysLineHdlr((uchar *)"actionname", 0, eCmdHdlrGetWord, NULL, &cs.pszActionName, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuefilename", 0, eCmdHdlrGetWord, NULL,
		&cs.pszActionQFName, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuesize", 0, eCmdHdlrInt, NULL, &cs.iActionQueueSize,
		NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionwriteallmarkmessages", 0, eCmdHdlrBinary, NULL,
		&cs.bActionWriteAllMarkMsgs, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuedequeuebatchsize", 0, eCmdHdlrInt, NULL,
		&cs.iActionQueueDeqBatchSize, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuemaxdiskspace", 0, eCmdHdlrSize, NULL,
		&cs.iActionQueMaxDiskSpace, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuehighwatermark", 0, eCmdHdlrInt, NULL,
		&cs.iActionQHighWtrMark, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuelowwatermark", 0, eCmdHdlrInt, NULL,
		&cs.iActionQLowWtrMark, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuediscardmark", 0, eCmdHdlrInt, NULL,
		&cs.iActionQDiscardMark, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuediscardseverity", 0, eCmdHdlrInt, NULL,
		&cs.iActionQDiscardSeverity, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuecheckpointinterval", 0, eCmdHdlrInt, NULL,
		&cs.iActionQPersistUpdCnt, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuesyncqueuefiles", 0, eCmdHdlrBinary, NULL,
		&cs.bActionQSyncQeueFiles, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuetype", 0, eCmdHdlrGetWord, setActionQueType, NULL, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueueworkerthreads", 0, eCmdHdlrInt, NULL,
		&cs.iActionQueueNumWorkers, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuetimeoutshutdown", 0, eCmdHdlrInt, NULL,
		&cs.iActionQtoQShutdown, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuetimeoutactioncompletion", 0, eCmdHdlrInt, NULL,
		&cs.iActionQtoActShutdown, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuetimeoutenqueue", 0, eCmdHdlrInt, NULL,
		&cs.iActionQtoEnq, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueueworkertimeoutthreadshutdown", 0, eCmdHdlrInt, NULL,
		&cs.iActionQtoWrkShutdown, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueueworkerthreadminimummessages", 0, eCmdHdlrInt, NULL,
		&cs.iActionQWrkMinMsgs, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuemaxfilesize", 0, eCmdHdlrSize, NULL,
		&cs.iActionQueMaxFileSize, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuesaveonshutdown", 0, eCmdHdlrBinary, NULL,
		&cs.bActionQSaveOnShutdown, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuedequeueslowdown", 0, eCmdHdlrInt, NULL,
		&cs.iActionQueueDeqSlowdown, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuedequeuetimebegin", 0, eCmdHdlrInt, NULL,
		&cs.iActionQueueDeqtWinFromHr, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionqueuedequeuetimeend", 0, eCmdHdlrInt, NULL,
		&cs.iActionQueueDeqtWinToHr, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionexeconlyeverynthtime", 0, eCmdHdlrInt, NULL,
		&cs.iActExecEveryNthOccur, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionexeconlyeverynthtimetimeout", 0, eCmdHdlrInt, NULL,
		&cs.iActExecEveryNthOccurTO, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionexeconlyonceeveryinterval", 0, eCmdHdlrInt, NULL,
		&cs.iActExecOnceInterval, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"repeatedmsgcontainsoriginalmsg", 0, eCmdHdlrBinary, NULL,
		&cs.bActionRepMsgHasMsg, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionexeconlywhenpreviousissuspended", 0, eCmdHdlrBinary, NULL,
		&cs.bActExecWhenPrevSusp, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"actionresumeretrycount", 0, eCmdHdlrInt, NULL,
		&cs.glbliActionResumeRetryCount, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, NULL));

	initConfigVariables(); /* first-time init of config setings */

finalize_it:
	RETiRet;
}

/* vi:set ai:
 */
