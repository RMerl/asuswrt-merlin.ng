/* wti.c
 *
 * This file implements the worker thread instance (wti) class.
 *
 * File begun on 2008-01-20 by RGerhards based on functions from the
 * previous queue object class (the wti functions have been extracted)
 *
 * There is some in-depth documentation available in doc/dev_queue.html
 * (and in the web doc set on http://www.rsyslog.com/doc). Be sure to read it
 * if you are getting aquainted to the object.
 *
 * Copyright 2008-2017 Adiscon GmbH.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "rsyslog.h"
#include "stringbuf.h"
#include "srUtils.h"
#include "errmsg.h"
#include "wtp.h"
#include "wti.h"
#include "obj.h"
#include "glbl.h"
#include "action.h"
#include "atomic.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)

pthread_key_t thrd_wti_key;


/* forward-definitions */

/* methods */

/* get the header for debug messages
 * The caller must NOT free or otherwise modify the returned string!
 */
static uchar *
wtiGetDbgHdr(wti_t *pThis)
{
	ISOBJ_TYPE_assert(pThis, wti);

	if(pThis->pszDbgHdr == NULL)
		return (uchar*) "wti"; /* should not normally happen */
	else
		return pThis->pszDbgHdr;
}


/* return the current worker processing state. For the sake of
 * simplicity, we do not use the iRet interface. -- rgerhards, 2009-07-17
 */
int ATTR_NONNULL()
wtiGetState(wti_t *pThis)
{
	return ATOMIC_FETCH_32BIT(&pThis->bIsRunning, &pThis->mutIsRunning);
}


/* Set this thread to "always running" state (can not be unset)
 * rgerhards, 2009-07-20
 */
rsRetVal ATTR_NONNULL()
wtiSetAlwaysRunning(wti_t *pThis)
{
	ISOBJ_TYPE_assert(pThis, wti);
	pThis->bAlwaysRunning = RSTRUE;
	return RS_RET_OK;
}

/* Set status (thread is running or not), actually an property of
 * use for wtp, but we need to have it per thread instance (thus it
 * is inside wti). -- rgerhards, 2009-07-17
 */
rsRetVal ATTR_NONNULL()
wtiSetState(wti_t *pThis, const int newVal)
{
	ISOBJ_TYPE_assert(pThis, wti);
	if(newVal == WRKTHRD_STOPPED) {
		ATOMIC_STORE_0_TO_INT(&pThis->bIsRunning, &pThis->mutIsRunning);
	} else {
		ATOMIC_OR_INT_TO_INT(&pThis->bIsRunning, &pThis->mutIsRunning, newVal);
	}
	return RS_RET_OK;
}


/* advise all workers to start by interrupting them. That should unblock all srSleep()
 * calls.
 */
rsRetVal
wtiWakeupThrd(wti_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, wti);


	if(wtiGetState(pThis)) {
		/* we first try the cooperative "cancel" interface */
		pthread_kill(pThis->thrdID, SIGTTIN);
		DBGPRINTF("sent SIGTTIN to worker thread %p\n", (void*) pThis->thrdID);
	}

	RETiRet;
}


/* Cancel the thread. If the thread is not running. But it is save and legal to
 * call wtiCancelThrd() in such situations. This function only returns when the
 * thread has terminated. Else we may get race conditions all over the code...
 * Note that when waiting for the thread to terminate, we do a busy wait, checking
 * progress every 10ms. It is very unlikely that we will ever cancel a thread
 * and, if so, it will only happen at the end of the rsyslog run. So doing this
 * kind of non-optimal wait is considered preferable over using condition variables.
 * rgerhards, 2008-02-26
 */
rsRetVal ATTR_NONNULL()
wtiCancelThrd(wti_t *pThis, const uchar *const cancelobj)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, wti);

	if(wtiGetState(pThis)) {
		LogMsg(0, RS_RET_ERR, LOG_WARNING, "%s: need to do cooperative cancellation "
			"- some data may be lost, increase timeout?", cancelobj);
		/* we first try the cooperative "cancel" interface */
		pthread_kill(pThis->thrdID, SIGTTIN);
		DBGPRINTF("sent SIGTTIN to worker thread %p, giving it a chance to terminate\n",
			(void *) pThis->thrdID);
		srSleep(0, 10000);
	}

	if(wtiGetState(pThis)) {
		LogMsg(0, RS_RET_ERR, LOG_WARNING, "%s: need to do hard cancellation", cancelobj);
		if(dbgTimeoutToStderr) {
			fprintf(stderr, "rsyslogd debug: %s: need to do hard cancellation\n",
				cancelobj);
		}
		pthread_cancel(pThis->thrdID);
		DBGPRINTF("cooperative worker termination failed, using cancellation...\n");
		DBGOPRINT((obj_t*) pThis, "canceling worker thread\n");
		pthread_cancel(pThis->thrdID);
		/* now wait until the thread terminates... */
		while(wtiGetState(pThis)) {
			srSleep(0, 10000);
		}
	}

	RETiRet;
}

/* note: this function is only called once in action.c */
rsRetVal
wtiNewIParam(wti_t *const pWti, action_t *const pAction, actWrkrIParams_t **piparams)
{
	actWrkrInfo_t *const wrkrInfo = &(pWti->actWrkrInfo[pAction->iActionNbr]);
	actWrkrIParams_t *iparams;
	int newMax;
	DEFiRet;

	if(wrkrInfo->p.tx.currIParam == wrkrInfo->p.tx.maxIParams) {
		/* we need to extend */
		newMax = (wrkrInfo->p.tx.maxIParams == 0) ? CONF_IPARAMS_BUFSIZE
							  : 2 * wrkrInfo->p.tx.maxIParams;
		CHKmalloc(iparams = realloc(wrkrInfo->p.tx.iparams,
					    sizeof(actWrkrIParams_t) * pAction->iNumTpls * newMax));
		memset(iparams + (wrkrInfo->p.tx.currIParam * pAction->iNumTpls), 0,
		       sizeof(actWrkrIParams_t) * pAction->iNumTpls * (newMax - wrkrInfo->p.tx.maxIParams));
		wrkrInfo->p.tx.iparams = iparams;
		wrkrInfo->p.tx.maxIParams = newMax;
	}
	*piparams = wrkrInfo->p.tx.iparams + wrkrInfo->p.tx.currIParam * pAction->iNumTpls;
	++wrkrInfo->p.tx.currIParam;

finalize_it:
	RETiRet;
}



/* Destructor */
BEGINobjDestruct(wti) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(wti)
	/* actual destruction */
	batchFree(&pThis->batch);
	free(pThis->actWrkrInfo);
	pthread_cond_destroy(&pThis->pcondBusy);
	DESTROY_ATOMIC_HELPER_MUT(pThis->mutIsRunning);
	free(pThis->pszDbgHdr);
ENDobjDestruct(wti)


/* Standard-Constructor for the wti object
 */
BEGINobjConstruct(wti) /* be sure to specify the object type also in END macro! */
	INIT_ATOMIC_HELPER_MUT(pThis->mutIsRunning);
	pthread_cond_init(&pThis->pcondBusy, NULL);
ENDobjConstruct(wti)


/* Construction finalizer
 * rgerhards, 2008-01-17
 */
rsRetVal
wtiConstructFinalize(wti_t *pThis)
{
	DEFiRet;
	int iDeqBatchSize;

	ISOBJ_TYPE_assert(pThis, wti);

	DBGPRINTF("%s: finalizing construction of worker instance data (for %d actions)\n",
		  wtiGetDbgHdr(pThis), iActionNbr);

	/* initialize our thread instance descriptor (no concurrency here) */
	pThis->bIsRunning = WRKTHRD_STOPPED;

	/* must use calloc as we need zero-init */
	CHKmalloc(pThis->actWrkrInfo = calloc(iActionNbr, sizeof(actWrkrInfo_t)));

	if(pThis->pWtp == NULL) {
		dbgprintf("wtiConstructFinalize: pWtp not set, this may be intentional\n");
		FINALIZE;
	}

	/* we now alloc the array for user pointers. We obtain the max from the queue itself. */
	CHKiRet(pThis->pWtp->pfGetDeqBatchSize(pThis->pWtp->pUsr, &iDeqBatchSize));
	CHKiRet(batchInit(&pThis->batch, iDeqBatchSize));

finalize_it:
	RETiRet;
}


/* cancellation cleanup handler for queueWorker ()
 * Most importantly, it must bring back the batch into a consistent state.
 * Keep in mind that cancellation is disabled if we run into
 * the cancel cleanup handler (and have been cancelled).
 * rgerhards, 2008-01-16
 */
static void
wtiWorkerCancelCleanup(void *arg)
{
	wti_t *pThis = (wti_t*) arg;
	wtp_t *pWtp;

	BEGINfunc
	ISOBJ_TYPE_assert(pThis, wti);
	pWtp = pThis->pWtp;
	ISOBJ_TYPE_assert(pWtp, wtp);

	DBGPRINTF("%s: cancelation cleanup handler called.\n", wtiGetDbgHdr(pThis));
	pWtp->pfObjProcessed(pWtp->pUsr, pThis);
	DBGPRINTF("%s: done cancelation cleanup handler.\n", wtiGetDbgHdr(pThis));
	
	ENDfunc
}


/* wait for queue to become non-empty or timeout
 * helper to wtiWorker. Note the the predicate is
 * re-tested by the caller, so it is OK to NOT do it here.
 * rgerhards, 2009-05-20
 */
static void
doIdleProcessing(wti_t *pThis, wtp_t *pWtp, int *pbInactivityTOOccured)
{
	struct timespec t;

	BEGINfunc
	DBGPRINTF("%s: worker IDLE, waiting for work.\n", wtiGetDbgHdr(pThis));

	if(pThis->bAlwaysRunning) {
		/* never shut down any started worker */
		d_pthread_cond_wait(&pThis->pcondBusy, pWtp->pmutUsr);
	} else {
		timeoutComp(&t, pWtp->toWrkShutdown);/* get absolute timeout */
		if(d_pthread_cond_timedwait(&pThis->pcondBusy, pWtp->pmutUsr, &t) != 0) {
			DBGPRINTF("%s: inactivity timeout, worker terminating...\n", wtiGetDbgHdr(pThis));
			*pbInactivityTOOccured = 1; /* indicate we had a timeout */
		}
	}
	DBGOPRINT((obj_t*) pThis, "worker awoke from idle processing\n");
	ENDfunc
}


/* generic worker thread framework. Note that we prohibit cancellation
 * during almost all times, because it can have very undesired side effects.
 * However, we may need to cancel a thread if the consumer blocks for too
 * long (during shutdown). So what we do is block cancellation, and every
 * consumer must enable it during the periods where it is safe.
 */
#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wempty-body"
#endif
rsRetVal
wtiWorker(wti_t *__restrict__ const pThis)
{
	wtp_t *__restrict__ const pWtp = pThis->pWtp; /* our worker thread pool -- shortcut */
	action_t *__restrict__ pAction;
	int bInactivityTOOccured = 0;
	rsRetVal localRet;
	rsRetVal terminateRet;
	actWrkrInfo_t *__restrict__ wrkrInfo;
	int iCancelStateSave;
	int i, j, k;
	DEFiRet;

	dbgSetThrdName(pThis->pszDbgHdr);
	pthread_cleanup_push(wtiWorkerCancelCleanup, pThis);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &iCancelStateSave);
	DBGPRINTF("wti %p: worker starting\n", pThis);
	/* now we have our identity, on to real processing */

	/* note: in this loop, the mutex is "never" unlocked. Of course,
	 * this is not true: it actually is unlocked when the actual processing
	 * is done, as part of pWtp->pfDoWork() processing. Note that this
	 * function is required to re-lock it when done. We cannot do the
	 * lock/unlock here ourselfs, as pfDoWork() needs to access queue
	 * structures itself.
	 * The same goes for pfRateLimiter(). While we could unlock/lock when
	 * we call it, in practice the function is often called without any
	 * ratelimiting actually done. Only the rate limiter itself knows
	 * that. As such, it needs to bear the burden of doing the locking
	 * when required. -- rgerhards, 2013-11-20
	 */
	d_pthread_mutex_lock(pWtp->pmutUsr);
	while(1) { /* loop will be broken below */
		if(pWtp->pfRateLimiter != NULL) { /* call rate-limiter, if defined */
			pWtp->pfRateLimiter(pWtp->pUsr);
		}

		/* first check if we are in shutdown process (but evaluate a bit later) */
		terminateRet = wtpChkStopWrkr(pWtp, MUTEX_ALREADY_LOCKED);
		if(terminateRet == RS_RET_TERMINATE_NOW) {
			/* we now need to free the old batch */
			localRet = pWtp->pfObjProcessed(pWtp->pUsr, pThis);
			DBGOPRINT((obj_t*) pThis, "terminating worker because of "
				  "TERMINATE_NOW mode, del iRet %d\n", localRet);
			break;
		}

		/* try to execute and process whatever we have */
		localRet = pWtp->pfDoWork(pWtp->pUsr, pThis);

		if(localRet == RS_RET_ERR_QUEUE_EMERGENCY) {
			break;	/* end of loop */
		} else if(localRet == RS_RET_IDLE) {
			if(terminateRet == RS_RET_TERMINATE_WHEN_IDLE || bInactivityTOOccured) {
				DBGOPRINT((obj_t*) pThis, "terminating worker terminateRet=%d, "
					"bInactivityTOOccured=%d\n", terminateRet, bInactivityTOOccured);
				break;	/* end of loop */
			}
			doIdleProcessing(pThis, pWtp, &bInactivityTOOccured);
			continue; /* request next iteration */
		}

		bInactivityTOOccured = 0; /* reset for next run */
	}

	d_pthread_mutex_unlock(pWtp->pmutUsr);

	DBGPRINTF("DDDD: wti %p: worker cleanup action instances\n", pThis);
	for(i = 0 ; i < iActionNbr ; ++i) {
		wrkrInfo = &(pThis->actWrkrInfo[i]);
		dbgprintf("wti %p, action %d, ptr %p\n", pThis, i, wrkrInfo->actWrkrData);
		if(wrkrInfo->actWrkrData != NULL) {
			pAction = wrkrInfo->pAction;
			actionRemoveWorker(pAction, wrkrInfo->actWrkrData);
			pAction->pMod->mod.om.freeWrkrInstance(wrkrInfo->actWrkrData);
			if(pAction->isTransactional) {
				/* free iparam "cache" - we need to go through to max! */
				for(j = 0 ; j < wrkrInfo->p.tx.maxIParams ; ++j) {
					for(k = 0 ; k < pAction->iNumTpls ; ++k) {
						free(actParam(wrkrInfo->p.tx.iparams,
							      pAction->iNumTpls, j, k).param);
					}
				}
				free(wrkrInfo->p.tx.iparams);
				wrkrInfo->p.tx.iparams = NULL;
				wrkrInfo->p.tx.currIParam = 0;
				wrkrInfo->p.tx.maxIParams = 0;
			} else {
				releaseDoActionParams(pAction, pThis, 1);
			}
			wrkrInfo->actWrkrData = NULL; /* re-init for next activation */
		}
	}

	/* indicate termination */
	pthread_cleanup_pop(0); /* remove cleanup handler */
	pthread_setcancelstate(iCancelStateSave, NULL);
	dbgprintf("wti %p: worker exiting\n", pThis);

	RETiRet;
}
#if !defined(_AIX)
#pragma GCC diagnostic warning "-Wempty-body"
#endif


/* some simple object access methods */
DEFpropSetMeth(wti, pWtp, wtp_t*)

/* set the debug header message
 * The passed-in string is duplicated. So if the caller does not need
 * it any longer, it must free it. Must be called only before object is finalized.
 * rgerhards, 2008-01-09
 */
rsRetVal
wtiSetDbgHdr(wti_t *pThis, uchar *pszMsg, size_t lenMsg)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, wti);
	assert(pszMsg != NULL);
	
	if(lenMsg < 1)
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);

	if(pThis->pszDbgHdr != NULL) {
		free(pThis->pszDbgHdr);
	}

	if((pThis->pszDbgHdr = MALLOC(lenMsg + 1)) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	memcpy(pThis->pszDbgHdr, pszMsg, lenMsg + 1); /* always think about the \0! */

finalize_it:
	RETiRet;
}


/* This function returns (and creates if necessary) a dummy wti suitable
 * for use by the rule engine. It is intended to be used for direct-mode
 * main queues (folks, don't do that!). Once created, data is stored in
 * thread-specific storage.
 * Note: we do NOT do error checking -- if this functions fails, all the
 * rest will fail as well... (also, it will only fail under OOM, so...).
 * Memleak: we leak pWti's when run in direct mode. However, this is only
 * a cosmetic leak, as we need them until all inputs are terminated,
 * what means essentially until rsyslog itself is terminated. So we
 * don't care -- it's just not nice in valgrind, but that's it.
 */
wti_t *
wtiGetDummy(void)
{
	wti_t *pWti;

	pWti = (wti_t*) pthread_getspecific(thrd_wti_key);
	if(pWti == NULL) {
		wtiConstruct(&pWti);
		if(pWti != NULL)
			wtiConstructFinalize(pWti);
		if(pthread_setspecific(thrd_wti_key, pWti) != 0) {
			DBGPRINTF("wtiGetDummy: error setspecific thrd_wti_key\n");
		}
	}
	return pWti;
}

/* dummy */
static rsRetVal wtiQueryInterface(interface_t __attribute__((unused)) *i) { return RS_RET_NOT_IMPLEMENTED; }

/* exit our class
 */
BEGINObjClassExit(wti, OBJ_IS_CORE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nsdsel_gtls)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
	pthread_key_delete(thrd_wti_key);
ENDObjClassExit(wti)


/* Initialize the wti class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-01-09
 */
BEGINObjClassInit(wti, 1, OBJ_IS_CORE_MODULE) /* one is the object version (most important for persisting) */
	int r;
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	r = pthread_key_create(&thrd_wti_key, NULL);
	if(r != 0) {
		dbgprintf("wti.c: pthread_key_create failed\n");
		ABORT_FINALIZE(RS_RET_ERR);
	}
ENDObjClassInit(wti)

/* vi:set ai:
 */
