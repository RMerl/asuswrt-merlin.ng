/* Definition of the worker thread pool (wtp) object.
 *
 * Copyright 2008-2012 Adiscon GmbH.
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

#ifndef WTP_H_INCLUDED
#define WTP_H_INCLUDED

#include <pthread.h>
#include "obj.h"
#include "atomic.h"

/* states for worker threads.
 * important: they need to be increasing with all previous state bits
 * set. That is because we can only atomically or a value!
 */
#define WRKTHRD_STOPPED  	0
#define WRKTHRD_INITIALIZING	1
#define WRKTHRD_RUNNING		3


/* possible states of a worker thread pool */
typedef enum {
	wtpState_RUNNING = 0,		/* runs in regular mode */
	wtpState_SHUTDOWN = 1,		/* worker threads shall shutdown when idle */
	wtpState_SHUTDOWN_IMMEDIATE = 2	/* worker threads shall shutdown ASAP, even if not idle */
} wtpState_t;


/* the worker thread pool (wtp) object */
struct wtp_s {
	BEGINobjInstance;
	wtpState_t wtpState;
	int 	iNumWorkerThreads;/* number of worker threads to use */
	int 	iCurNumWrkThrd;/* current number of active worker threads */
	struct wti_s **pWrkr;/* array with control structure for the worker thread(s) associated with this wtp */
	int	toWrkShutdown;	/* timeout for idle workers in ms, -1 means indefinite (0 is immediate) */
	rsRetVal (*pConsumer)(void *); /* user-supplied consumer function for dewtpd messages */
	/* synchronization variables */
	pthread_mutex_t mutWtp; /* mutex for the wtp's thread management */
	pthread_cond_t condThrdInitDone; /* signalled when a new thread is ready for work */
	pthread_cond_t condThrdTrm;/* signalled when threads terminate */
	/* end sync variables */
	/* user objects */
	void *pUsr;		/* pointer to user object (in this case, the queue the wtp belongs to) */
	pthread_attr_t attrThrd;/* attribute for new threads (created just once and cached here) */
	pthread_mutex_t *pmutUsr;
	rsRetVal (*pfChkStopWrkr)(void *pUsr, int);
	rsRetVal (*pfGetDeqBatchSize)(void *pUsr, int*); /* obtains max dequeue count from queue config */
	rsRetVal (*pfObjProcessed)(void *pUsr, wti_t *pWti); /* indicate user object is processed */
	rsRetVal (*pfRateLimiter)(void *pUsr);
	rsRetVal (*pfDoWork)(void *pUsr, void *pWti);
	/* end user objects */
	uchar *pszDbgHdr;	/* header string for debug messages */
	DEF_ATOMIC_HELPER_MUT(mutCurNumWrkThrd)
	DEF_ATOMIC_HELPER_MUT(mutWtpState)
};

/* some symbolic constants for easier reference */


/* prototypes */
rsRetVal wtpConstruct(wtp_t **ppThis);
rsRetVal wtpConstructFinalize(wtp_t *pThis);
rsRetVal wtpDestruct(wtp_t **ppThis);
rsRetVal wtpAdviseMaxWorkers(wtp_t *pThis, int nMaxWrkr);
rsRetVal wtpProcessThrdChanges(wtp_t *pThis);
rsRetVal wtpChkStopWrkr(wtp_t *pThis, int bLockUsrMutex);
rsRetVal wtpSetState(wtp_t *pThis, wtpState_t iNewState);
rsRetVal wtpWakeupAllWrkr(wtp_t *pThis);
rsRetVal wtpCancelAll(wtp_t *pThis, const uchar *const cancelobj);
rsRetVal wtpSetDbgHdr(wtp_t *pThis, uchar *pszMsg, size_t lenMsg);
rsRetVal wtpShutdownAll(wtp_t *pThis, wtpState_t tShutdownCmd, struct timespec *ptTimeout);
PROTOTYPEObjClassInit(wtp);
PROTOTYPEObjClassExit(wtp);
PROTOTYPEpropSetMethFP(wtp, pfChkStopWrkr, rsRetVal(*pVal)(void*, int));
PROTOTYPEpropSetMethFP(wtp, pfRateLimiter, rsRetVal(*pVal)(void*));
PROTOTYPEpropSetMethFP(wtp, pfGetDeqBatchSize, rsRetVal(*pVal)(void*, int*));
PROTOTYPEpropSetMethFP(wtp, pfDoWork, rsRetVal(*pVal)(void*, void*));
PROTOTYPEpropSetMethFP(wtp, pfObjProcessed, rsRetVal(*pVal)(void*, wti_t*));
PROTOTYPEpropSetMeth(wtp, toWrkShutdown, long);
PROTOTYPEpropSetMeth(wtp, wtpState, wtpState_t);
PROTOTYPEpropSetMeth(wtp, iMaxWorkerThreads, int);
PROTOTYPEpropSetMeth(wtp, pUsr, void*);
PROTOTYPEpropSetMeth(wtp, iNumWorkerThreads, int);
PROTOTYPEpropSetMethPTR(wtp, pmutUsr, pthread_mutex_t);

#endif /* #ifndef WTP_H_INCLUDED */
