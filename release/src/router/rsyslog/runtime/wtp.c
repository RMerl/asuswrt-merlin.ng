/* wtp.c
 *
 * This file implements the worker thread pool (wtp) class.
 *
 * File begun on 2008-01-20 by RGerhards
 *
 * There is some in-depth documentation available in doc/dev_queue.html
 * (and in the web doc set on http://www.rsyslog.com/doc). Be sure to read it
 * if you are getting aquainted to the object.
 *
 * Copyright 2008-2017 Rainer Gerhards and Adiscon GmbH.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <atomic.h>
#ifdef HAVE_SYS_PRCTL_H
#  include <sys/prctl.h>
#endif

/// TODO: check on solaris if this is any longer needed - I don't think so - rgerhards, 2009-09-20
//#ifdef OS_SOLARIS
//#	include <sched.h>
//#endif

#include "rsyslog.h"
#include "stringbuf.h"
#include "srUtils.h"
#include "wtp.h"
#include "wti.h"
#include "obj.h"
#include "unicode-helper.h"
#include "glbl.h"
#include "errmsg.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)

/* forward-definitions */

/* methods */

/* get the header for debug messages
 * The caller must NOT free or otherwise modify the returned string!
 */
static uchar *
wtpGetDbgHdr(wtp_t *pThis)
{
	ISOBJ_TYPE_assert(pThis, wtp);

	if(pThis->pszDbgHdr == NULL)
		return (uchar*) "wtp"; /* should not normally happen */
	else
		return pThis->pszDbgHdr;
}



/* Not implemented dummy function for constructor */
static rsRetVal NotImplementedDummy_voidp_int(__attribute__((unused)) void* p1, __attribute__((unused)) int p2) {
	return RS_RET_NOT_IMPLEMENTED; }
static rsRetVal NotImplementedDummy_voidp_intp(__attribute__((unused)) void* p1, __attribute__((unused)) int* p2) {
	return RS_RET_NOT_IMPLEMENTED; }
static rsRetVal NotImplementedDummy_voidp_voidp(__attribute__((unused)) void* p1, __attribute__((unused)) void* p2) {
	return RS_RET_NOT_IMPLEMENTED; }
static rsRetVal NotImplementedDummy_voidp_wti_tp(__attribute__((unused)) void* p1, __attribute__((unused)) wti_t* p2) {
	return RS_RET_NOT_IMPLEMENTED; }
/* Standard-Constructor for the wtp object
 */
BEGINobjConstruct(wtp) /* be sure to specify the object type also in END macro! */
	pthread_mutex_init(&pThis->mutWtp, NULL);
	pthread_cond_init(&pThis->condThrdInitDone, NULL);
	pthread_cond_init(&pThis->condThrdTrm, NULL);
	pthread_attr_init(&pThis->attrThrd);
	/* Set thread scheduling policy to default */
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
	pthread_attr_setschedpolicy(&pThis->attrThrd, default_thr_sched_policy);
	pthread_attr_setschedparam(&pThis->attrThrd, &default_sched_param);
	pthread_attr_setinheritsched(&pThis->attrThrd, PTHREAD_EXPLICIT_SCHED);
#endif
	pthread_attr_setdetachstate(&pThis->attrThrd, PTHREAD_CREATE_DETACHED);
	/* set all function pointers to "not implemented" dummy so that we can safely call them */
	pThis->pfChkStopWrkr = (rsRetVal (*)(void*,int))NotImplementedDummy_voidp_int;
	pThis->pfGetDeqBatchSize = (rsRetVal (*)(void*,int*))NotImplementedDummy_voidp_intp;
	pThis->pfDoWork = (rsRetVal (*)(void*,void*))NotImplementedDummy_voidp_voidp;
	pThis->pfObjProcessed = (rsRetVal (*)(void*,wti_t*))NotImplementedDummy_voidp_wti_tp;
	INIT_ATOMIC_HELPER_MUT(pThis->mutCurNumWrkThrd);
	INIT_ATOMIC_HELPER_MUT(pThis->mutWtpState);
ENDobjConstruct(wtp)


/* Construction finalizer
 * rgerhards, 2008-01-17
 */
rsRetVal
wtpConstructFinalize(wtp_t *pThis)
{
	DEFiRet;
	int i;
	uchar pszBuf[64];
	size_t lenBuf;
	wti_t *pWti;

	ISOBJ_TYPE_assert(pThis, wtp);

	DBGPRINTF("%s: finalizing construction of worker thread pool (numworkerThreads %d)\n",
		  wtpGetDbgHdr(pThis), pThis->iNumWorkerThreads);
	/* alloc and construct workers - this can only be done in finalizer as we previously do
	 * not know the max number of workers
	 */
	CHKmalloc(pThis->pWrkr = MALLOC(sizeof(wti_t*) * pThis->iNumWorkerThreads));
	
	for(i = 0 ; i < pThis->iNumWorkerThreads ; ++i) {
		CHKiRet(wtiConstruct(&pThis->pWrkr[i]));
		pWti = pThis->pWrkr[i];
		lenBuf = snprintf((char*)pszBuf, sizeof(pszBuf), "%s/w%d", wtpGetDbgHdr(pThis), i);
		CHKiRet(wtiSetDbgHdr(pWti, pszBuf, lenBuf));
		CHKiRet(wtiSetpWtp(pWti, pThis));
		CHKiRet(wtiConstructFinalize(pWti));
	}


finalize_it:
	RETiRet;
}


/* Destructor */
BEGINobjDestruct(wtp) /* be sure to specify the object type also in END and CODESTART macros! */
	int i;
CODESTARTobjDestruct(wtp)
	d_pthread_mutex_lock(&pThis->mutWtp); /* make sure nobody is still using the mutex */
	assert(pThis->iCurNumWrkThrd == 0);

	/* destruct workers */
	for(i = 0 ; i < pThis->iNumWorkerThreads ; ++i)
		wtiDestruct(&pThis->pWrkr[i]);

	free(pThis->pWrkr);
	pThis->pWrkr = NULL;

	/* actual destruction */
	d_pthread_mutex_unlock(&pThis->mutWtp);
	pthread_cond_destroy(&pThis->condThrdTrm);
	pthread_cond_destroy(&pThis->condThrdInitDone);
	pthread_mutex_destroy(&pThis->mutWtp);
	pthread_attr_destroy(&pThis->attrThrd);
	DESTROY_ATOMIC_HELPER_MUT(pThis->mutCurNumWrkThrd);
	DESTROY_ATOMIC_HELPER_MUT(pThis->mutWtpState);

	free(pThis->pszDbgHdr);
ENDobjDestruct(wtp)


/* Sent a specific state for the worker thread pool. -- rgerhards, 2008-01-21
 * We do not need to do atomic instructions as set operations are only
 * called when terminating the pool, and then in strict sequence. So we
 * can never overwrite each other. On the other hand, it also doesn't
 * matter if the read operation obtains an older value, as we then simply
 * do one more iteration, what is perfectly legal (during shutdown
 * they are awoken in any case). -- rgerhards, 2009-07-20
 */
rsRetVal
wtpSetState(wtp_t *pThis, wtpState_t iNewState)
{
	ISOBJ_TYPE_assert(pThis, wtp);
	pThis->wtpState = iNewState; // TODO: do we need a mutex here? 2010-04-26
	return RS_RET_OK;
}


/* check if the worker shall shutdown (1 = yes, 0 = no)
 * Note: there may be two mutexes locked, the bLockUsrMutex is the one in our "user"
 * (e.g. the queue clas)
 * rgerhards, 2008-01-21
 */
rsRetVal
wtpChkStopWrkr(wtp_t *pThis, int bLockUsrMutex)
{
	DEFiRet;
	wtpState_t wtpState;

	ISOBJ_TYPE_assert(pThis, wtp);
	/* we need a consistent value, but it doesn't really matter if it is changed
	 * right after the fetch - then we simply do one more iteration in the worker
	 */
	wtpState = (wtpState_t) ATOMIC_FETCH_32BIT((int*)&pThis->wtpState, &pThis->mutWtpState);

	if(wtpState == wtpState_SHUTDOWN_IMMEDIATE) {
		ABORT_FINALIZE(RS_RET_TERMINATE_NOW);
	} else if(wtpState == wtpState_SHUTDOWN) {
		ABORT_FINALIZE(RS_RET_TERMINATE_WHEN_IDLE);
	}

	/* try customer handler if one was set and we do not yet have a definite result */
	if(pThis->pfChkStopWrkr != NULL) {
		iRet = pThis->pfChkStopWrkr(pThis->pUsr, bLockUsrMutex);
	}

finalize_it:
	RETiRet;
}


#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wempty-body"
#endif
/* Send a shutdown command to all workers and see if they terminate.
 * A timeout may be specified. This function may also be called with
 * the current number of workers being 0, in which case it does not
 * shut down any worker.
 * rgerhards, 2008-01-14
 */
rsRetVal ATTR_NONNULL()
wtpShutdownAll(wtp_t *pThis, wtpState_t tShutdownCmd, struct timespec *ptTimeout)
{
	DEFiRet;
	int bTimedOut;
	int i;

	ISOBJ_TYPE_assert(pThis, wtp);

	/* lock mutex to prevent races (may otherwise happen during idle processing and such...) */
	d_pthread_mutex_lock(pThis->pmutUsr);
	wtpSetState(pThis, tShutdownCmd);
	/* awake workers in retry loop */
	for(i = 0 ; i < pThis->iNumWorkerThreads ; ++i) {
		pthread_cond_signal(&pThis->pWrkr[i]->pcondBusy);
		wtiWakeupThrd(pThis->pWrkr[i]);
	}
	d_pthread_mutex_unlock(pThis->pmutUsr);

	/* wait for worker thread termination */
	d_pthread_mutex_lock(&pThis->mutWtp);
	pthread_cleanup_push(mutexCancelCleanup, &pThis->mutWtp);
	bTimedOut = 0;
	while(pThis->iCurNumWrkThrd > 0 && !bTimedOut) {
		DBGPRINTF("%s: waiting %ldms on worker thread termination, %d still running\n",
			   wtpGetDbgHdr(pThis), timeoutVal(ptTimeout),
			   ATOMIC_FETCH_32BIT(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd));

		if(d_pthread_cond_timedwait(&pThis->condThrdTrm, &pThis->mutWtp, ptTimeout) != 0) {
			DBGPRINTF("%s: timeout waiting on worker thread termination\n",
				wtpGetDbgHdr(pThis));
			bTimedOut = 1;	/* we exit the loop on timeout */
		}

		/* awake workers in retry loop */
		for(i = 0 ; i < pThis->iNumWorkerThreads ; ++i) {
			wtiWakeupThrd(pThis->pWrkr[i]);
		}

	}
	pthread_cleanup_pop(1);

	if(bTimedOut)
		iRet = RS_RET_TIMED_OUT;
	
	RETiRet;
}
#if !defined(_AIX)
#pragma GCC diagnostic warning "-Wempty-body"
#endif


/* Unconditionally cancel all running worker threads.
 * rgerhards, 2008-01-14
 */
rsRetVal ATTR_NONNULL()
wtpCancelAll(wtp_t *pThis, const uchar *const cancelobj)
{
	DEFiRet;
	int i;

	ISOBJ_TYPE_assert(pThis, wtp);

	/* go through all workers and cancel those that are active */
	for(i = 0 ; i < pThis->iNumWorkerThreads ; ++i) {
		wtiCancelThrd(pThis->pWrkr[i], cancelobj);
	}

	RETiRet;
}


/* this function contains shared code for both regular worker shutdown as
 * well as shutdown via cancellation. We can not simply use pthread_cleanup_pop(1)
 * as this introduces a race in the debug system (RETiRet system).
 * rgerhards, 2009-10-26
 */
static void
wtpWrkrExecCleanup(wti_t *pWti)
{
	wtp_t *pThis;

	BEGINfunc
	ISOBJ_TYPE_assert(pWti, wti);
	pThis = pWti->pWtp;
	ISOBJ_TYPE_assert(pThis, wtp);

	/* the order of the next two statements is important! */
	wtiSetState(pWti, WRKTHRD_STOPPED);
	ATOMIC_DEC(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd);

	/* note: numWorkersNow is only for message generation, so we do not try
	 * hard to get it 100% accurate (as curently done, it is not).
	 */
	const int numWorkersNow = ATOMIC_FETCH_32BIT(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd);
	DBGPRINTF("%s: Worker thread %lx, terminated, num workers now %d\n",
		wtpGetDbgHdr(pThis), (unsigned long) pWti, numWorkersNow);
	if(numWorkersNow > 0) {
		LogMsg(0, RS_RET_OPERATION_STATUS, LOG_INFO,
			"%s: worker thread %lx terminated, now %d active worker threads",
			wtpGetDbgHdr(pThis), (unsigned long) pWti, numWorkersNow);
	}

	ENDfunc
}


/* cancellation cleanup handler for executing worker decrements the worker counter.
 * rgerhards, 2009-07-20
 */
static void
wtpWrkrExecCancelCleanup(void *arg)
{
	wti_t *pWti = (wti_t*) arg;
	wtp_t *pThis;

	BEGINfunc
	ISOBJ_TYPE_assert(pWti, wti);
	pThis = pWti->pWtp;
	ISOBJ_TYPE_assert(pThis, wtp);
	DBGPRINTF("%s: Worker thread %lx requested to be cancelled.\n",
		  wtpGetDbgHdr(pThis), (unsigned long) pWti);

	wtpWrkrExecCleanup(pWti);

	ENDfunc
	/* NOTE: we must call ENDfunc FIRST, because otherwise the schedule may activate the main
	 * thread after the broadcast, which could destroy the debug class, resulting in a potential
	 * segfault. So we need to do the broadcast as actually the last action in our processing
	 */
	pthread_cond_broadcast(&pThis->condThrdTrm); /* activate anyone waiting on thread shutdown */
}


/* wtp worker shell. This is started and calls into the actual
 * wti worker.
 * rgerhards, 2008-01-21
 */
#if !defined(_AIX)
#pragma GCC diagnostic ignored "-Wempty-body"
#endif
static void *
wtpWorker(void *arg) /* the arg is actually a wti object, even though we are in wtp! */
{
	wti_t *pWti = (wti_t*) arg;
	wtp_t *pThis;
	sigset_t sigSet;
#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	uchar *pszDbgHdr;
	uchar thrdName[32] = "rs:";
#	endif

	BEGINfunc
	ISOBJ_TYPE_assert(pWti, wti);
	pThis = pWti->pWtp;
	ISOBJ_TYPE_assert(pThis, wtp);

	/* block all signals except SIGTTIN and SIGSEGV */
	sigfillset(&sigSet);
	sigdelset(&sigSet, SIGTTIN);
	sigdelset(&sigSet, SIGSEGV);
	pthread_sigmask(SIG_BLOCK, &sigSet, NULL);

#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	/* set thread name - we ignore if the call fails, has no harsh consequences... */
	pszDbgHdr = wtpGetDbgHdr(pThis);
	ustrncpy(thrdName+3, pszDbgHdr, 20);
	if(prctl(PR_SET_NAME, thrdName, 0, 0, 0) != 0) {
		DBGPRINTF("prctl failed, not setting thread name for '%s'\n", wtpGetDbgHdr(pThis));
	}
	dbgOutputTID((char*)thrdName);
#	endif

	/* let the parent know we're done with initialization */
	d_pthread_mutex_lock(&pThis->mutWtp);
	wtiSetState(pWti, WRKTHRD_RUNNING);
	pthread_cond_broadcast(&pThis->condThrdInitDone);
	d_pthread_mutex_unlock(&pThis->mutWtp);

	pthread_cleanup_push(wtpWrkrExecCancelCleanup, pWti);

	wtiWorker(pWti);
	pthread_cleanup_pop(0);
	d_pthread_mutex_lock(&pThis->mutWtp);
	pthread_cleanup_push(mutexCancelCleanup, &pThis->mutWtp);
	wtpWrkrExecCleanup(pWti);

	ENDfunc
	/* NOTE: we must call ENDfunc FIRST, because otherwise the schedule may activate the main
	 * thread after the broadcast, which could destroy the debug class, resulting in a potential
	 * segfault. So we need to do the broadcast as actually the last action in our processing
	 */
	pthread_cond_broadcast(&pThis->condThrdTrm); /* activate anyone waiting on thread shutdown */
	pthread_cleanup_pop(1); /* unlock mutex */
	pthread_exit(0);
}
#if !defined(_AIX)
#pragma GCC diagnostic warning "-Wempty-body"
#endif


/* start a new worker */
static rsRetVal
wtpStartWrkr(wtp_t *pThis)
{
	wti_t *pWti;
	int i;
	int iState;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, wtp);

	d_pthread_mutex_lock(&pThis->mutWtp);

	/* find free spot in thread table. */
	for(i = 0 ; i < pThis->iNumWorkerThreads ; ++i) {
		if(wtiGetState(pThis->pWrkr[i]) == WRKTHRD_STOPPED) {
			break;
		}
	}

	if(i == pThis->iNumWorkerThreads)
		ABORT_FINALIZE(RS_RET_NO_MORE_THREADS);

	if(i == 0 || pThis->toWrkShutdown == -1) {
		wtiSetAlwaysRunning(pThis->pWrkr[i]);
	}

	pWti = pThis->pWrkr[i];
	wtiSetState(pWti, WRKTHRD_INITIALIZING);
	iState = pthread_create(&(pWti->thrdID), &pThis->attrThrd,wtpWorker, (void*) pWti);
	ATOMIC_INC(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd); /* we got one more! */

	DBGPRINTF("%s: started with state %d, num workers now %d\n",
		wtpGetDbgHdr(pThis), iState,
		ATOMIC_FETCH_32BIT(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd));

	/* wait for the new thread to initialize its signal mask and
	 * cancelation cleanup handler before proceeding
	 */
	do {
		d_pthread_cond_wait(&pThis->condThrdInitDone, &pThis->mutWtp);
	} while(wtiGetState(pWti) != WRKTHRD_RUNNING);

finalize_it:
	d_pthread_mutex_unlock(&pThis->mutWtp);
	RETiRet;
}


/* set the number of worker threads that should be running. If less than currently running,
 * a new worker may be started. Please note that there is no guarantee the number of workers
 * said will be running after we exit this function. It is just a hint. If the number is
 * higher than one, and no worker is started, the "busy" condition is signaled to awake a worker.
 * So the caller can assume that there is at least one worker re-checking if there is "work to do"
 * after this function call.
 * rgerhards, 2008-01-21
 */
rsRetVal
wtpAdviseMaxWorkers(wtp_t *pThis, int nMaxWrkr)
{
	DEFiRet;
	int nMissing; /* number workers missing to run */
	int i, nRunning;

	ISOBJ_TYPE_assert(pThis, wtp);

	if(nMaxWrkr == 0)
		FINALIZE;

	if(nMaxWrkr > pThis->iNumWorkerThreads) /* limit to configured maximum */
		nMaxWrkr = pThis->iNumWorkerThreads;

	nMissing = nMaxWrkr - ATOMIC_FETCH_32BIT(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd);

	if(nMissing > 0) {
		if(ATOMIC_FETCH_32BIT(&pThis->iCurNumWrkThrd, &pThis->mutCurNumWrkThrd) > 0) {
			LogMsg(0, RS_RET_OPERATION_STATUS, LOG_INFO,
				"%s: high activity - starting %d additional worker thread(s), "
				"currently %d active worker threads.",
				wtpGetDbgHdr(pThis), nMissing,
				ATOMIC_FETCH_32BIT(&pThis->iCurNumWrkThrd,
					&pThis->mutCurNumWrkThrd) );
		}
		/* start the rqtd nbr of workers */
		for(i = 0 ; i < nMissing ; ++i) {
			CHKiRet(wtpStartWrkr(pThis));
		}
	} else {
		/* we have needed number of workers, but they may be sleeping */
		for(i = 0, nRunning = 0; i < pThis->iNumWorkerThreads && nRunning < nMaxWrkr; ++i) {
			if (wtiGetState(pThis->pWrkr[i]) != WRKTHRD_STOPPED) {
				pthread_cond_signal(&pThis->pWrkr[i]->pcondBusy);
				nRunning++;
			}
		}
	}

	
finalize_it:
	RETiRet;
}


/* some simple object access methods */
DEFpropSetMeth(wtp, toWrkShutdown, long)
DEFpropSetMeth(wtp, wtpState, wtpState_t)
DEFpropSetMeth(wtp, iNumWorkerThreads, int)
DEFpropSetMeth(wtp, pUsr, void*)
DEFpropSetMethPTR(wtp, pmutUsr, pthread_mutex_t)
DEFpropSetMethFP(wtp, pfChkStopWrkr, rsRetVal(*pVal)(void*, int))
DEFpropSetMethFP(wtp, pfRateLimiter, rsRetVal(*pVal)(void*))
DEFpropSetMethFP(wtp, pfGetDeqBatchSize, rsRetVal(*pVal)(void*, int*))
DEFpropSetMethFP(wtp, pfDoWork, rsRetVal(*pVal)(void*, void*))
DEFpropSetMethFP(wtp, pfObjProcessed, rsRetVal(*pVal)(void*, wti_t*))


/* set the debug header message
 * The passed-in string is duplicated. So if the caller does not need
 * it any longer, it must free it. Must be called only before object is finalized.
 * rgerhards, 2008-01-09
 */
rsRetVal
wtpSetDbgHdr(wtp_t *pThis, uchar *pszMsg, size_t lenMsg)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, wtp);
	assert(pszMsg != NULL);
	
	if(lenMsg < 1)
		ABORT_FINALIZE(RS_RET_PARAM_ERROR);

	if(pThis->pszDbgHdr != NULL) {
		free(pThis->pszDbgHdr);
		pThis->pszDbgHdr = NULL;
	}

	if((pThis->pszDbgHdr = MALLOC(lenMsg + 1)) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	memcpy(pThis->pszDbgHdr, pszMsg, lenMsg + 1); /* always think about the \0! */

finalize_it:
	RETiRet;
}

/* dummy */
static rsRetVal wtpQueryInterface(interface_t __attribute__((unused)) *i) { return RS_RET_NOT_IMPLEMENTED; }

/* exit our class
 */
BEGINObjClassExit(wtp, OBJ_IS_CORE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nsdsel_gtls)
	/* release objects we no longer need */
	objRelease(glbl, CORE_COMPONENT);
ENDObjClassExit(wtp)


/* Initialize the stream class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-01-09
 */
BEGINObjClassInit(wtp, 1, OBJ_IS_CORE_MODULE)
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
ENDObjClassInit(wtp)
