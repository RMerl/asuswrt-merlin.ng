/* queue.c
 *
 * This file implements the queue object and its several queueing methods.
 *
 * File begun on 2008-01-03 by RGerhards
 *
 * There is some in-depth documentation available in doc/dev_queue.html
 * (and in the web doc set on http://www.rsyslog.com/doc). Be sure to read it
 * if you are getting aquainted to the object.
 *
 * NOTE: as of 2009-04-22, I have begin to remove the qqueue* prefix from static
 * function names - this makes it really hard to read and does not provide much
 * benefit, at least I (now) think so...
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
#include <sys/stat.h>	 /* required for HP UX */
#include <time.h>
#include <errno.h>
#include <inttypes.h>

#include "rsyslog.h"
#include "queue.h"
#include "stringbuf.h"
#include "srUtils.h"
#include "obj.h"
#include "wtp.h"
#include "wti.h"
#include "msg.h"
#include "obj.h"
#include "atomic.h"
#include "errmsg.h"
#include "datetime.h"
#include "unicode-helper.h"
#include "statsobj.h"
#include "parserif.h"

#ifdef OS_SOLARIS
#	include <sched.h>
#endif

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(strm)
DEFobjCurrIf(datetime)
DEFobjCurrIf(statsobj)

#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wcast-function-type" // TODO: investigate further!
#endif /* if __GNUC__ >= 8 */

#ifdef ENABLE_IMDIAG
unsigned int iOverallQueueSize = 0;
#endif

/* forward-definitions */
static rsRetVal doEnqSingleObj(qqueue_t *pThis, flowControl_t flowCtlType, smsg_t *pMsg);
static rsRetVal qqueueChkPersist(qqueue_t *pThis, int nUpdates);
static rsRetVal RateLimiter(qqueue_t *pThis);
/*  AIXPORT : return type mismatch corrected */
#if defined (_AIX)
static rsRetVal qqueueChkStopWrkrDA(qqueue_t *pThis);
#else
static int qqueueChkStopWrkrDA(qqueue_t *pThis);
#endif
static rsRetVal GetDeqBatchSize(qqueue_t *pThis, int *pVal);
static rsRetVal ConsumerDA(qqueue_t *pThis, wti_t *pWti);
static rsRetVal batchProcessed(qqueue_t *pThis, wti_t *pWti);
static rsRetVal qqueueMultiEnqObjNonDirect(qqueue_t *pThis, multi_submit_t *pMultiSub);
static rsRetVal qqueueMultiEnqObjDirect(qqueue_t *pThis, multi_submit_t *pMultiSub);
static rsRetVal qAddDirect(qqueue_t *pThis, smsg_t *pMsg);
static rsRetVal qDestructDirect(qqueue_t __attribute__((unused)) *pThis);
static rsRetVal qConstructDirect(qqueue_t __attribute__((unused)) *pThis);
static rsRetVal qDestructDisk(qqueue_t *pThis);
rsRetVal qqueueSetSpoolDir(qqueue_t *pThis, uchar *pszSpoolDir, int lenSpoolDir);

/* some constants for queuePersist () */
#define QUEUE_CHECKPOINT	1
#define QUEUE_NO_CHECKPOINT	0

/* tables for interfacing with the v6 config system */
static struct cnfparamdescr cnfpdescr[] = {
	{ "queue.filename", eCmdHdlrGetWord, 0 },
	{ "queue.spooldirectory", eCmdHdlrGetWord, 0 },
	{ "queue.size", eCmdHdlrSize, 0 },
	{ "queue.dequeuebatchsize", eCmdHdlrInt, 0 },
	{ "queue.maxdiskspace", eCmdHdlrSize, 0 },
	{ "queue.highwatermark", eCmdHdlrInt, 0 },
	{ "queue.lowwatermark", eCmdHdlrInt, 0 },
	{ "queue.fulldelaymark", eCmdHdlrInt, 0 },
	{ "queue.lightdelaymark", eCmdHdlrInt, 0 },
	{ "queue.discardmark", eCmdHdlrInt, 0 },
	{ "queue.discardseverity", eCmdHdlrFacility, 0 },
	{ "queue.checkpointinterval", eCmdHdlrInt, 0 },
	{ "queue.syncqueuefiles", eCmdHdlrBinary, 0 },
	{ "queue.type", eCmdHdlrQueueType, 0 },
	{ "queue.workerthreads", eCmdHdlrInt, 0 },
	{ "queue.timeoutshutdown", eCmdHdlrInt, 0 },
	{ "queue.timeoutactioncompletion", eCmdHdlrInt, 0 },
	{ "queue.timeoutenqueue", eCmdHdlrInt, 0 },
	{ "queue.timeoutworkerthreadshutdown", eCmdHdlrInt, 0 },
	{ "queue.workerthreadminimummessages", eCmdHdlrInt, 0 },
	{ "queue.maxfilesize", eCmdHdlrSize, 0 },
	{ "queue.saveonshutdown", eCmdHdlrBinary, 0 },
	{ "queue.dequeueslowdown", eCmdHdlrInt, 0 },
	{ "queue.dequeuetimebegin", eCmdHdlrInt, 0 },
	{ "queue.dequeuetimeend", eCmdHdlrInt, 0 },
	{ "queue.cry.provider", eCmdHdlrGetWord, 0 },
	{ "queue.samplinginterval", eCmdHdlrInt, 0 }
};
static struct cnfparamblk pblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(cnfpdescr)/sizeof(struct cnfparamdescr),
	  cnfpdescr
	};

/* debug aid */
#if 0
static inline void displayBatchState(batch_t *pBatch)
{
	int i;
	for(i = 0 ; i < pBatch->nElem ; ++i) {
		DBGPRINTF("displayBatchState %p[%d]: %d\n", pBatch, i, pBatch->eltState[i]);
	}
}
#endif
static rsRetVal qqueuePersist(qqueue_t *pThis, int bIsCheckpoint);

/***********************************************************************
 * we need a private data structure, the "to-delete" list. As C does
 * not provide any partly private data structures, we implement this
 * structure right here inside the module.
 * Note that this list must always be kept sorted based on a unique
 * dequeue ID (which is monotonically increasing).
 * rgerhards, 2009-05-18
 ***********************************************************************/

/* generate next uniqueue dequeue ID. Note that uniqueness is only required
 * on a per-queue basis and while this instance runs. So a stricly monotonically
 * increasing counter is sufficient (if enough bits are used).
 */
static inline qDeqID getNextDeqID(qqueue_t *pQueue)
{
	ISOBJ_TYPE_assert(pQueue, qqueue);
	return pQueue->deqIDAdd++;
}


/* return the top element of the to-delete list or NULL, if the
 * list is empty.
 */
static toDeleteLst_t *tdlPeek(qqueue_t *pQueue)
{
	ISOBJ_TYPE_assert(pQueue, qqueue);
	return pQueue->toDeleteLst;
}


/* remove the top element of the to-delete list. Nothing but the
 * element itself is destroyed. Must not be called when the list
 * is empty.
 */
static rsRetVal tdlPop(qqueue_t *pQueue)
{
	toDeleteLst_t *pRemove;
	DEFiRet;

	ISOBJ_TYPE_assert(pQueue, qqueue);
	assert(pQueue->toDeleteLst != NULL);

	pRemove = pQueue->toDeleteLst;
	pQueue->toDeleteLst = pQueue->toDeleteLst->pNext;
	free(pRemove);

	RETiRet;
}


/* Add a new to-delete list entry. The function allocates the data
 * structure, populates it with the values provided and links the new
 * element into the correct place inside the list.
 */
static rsRetVal
tdlAdd(qqueue_t *pQueue, qDeqID deqID, int nElemDeq)
{
	toDeleteLst_t *pNew;
	toDeleteLst_t *pPrev;
	DEFiRet;

	ISOBJ_TYPE_assert(pQueue, qqueue);
	assert(pQueue->toDeleteLst != NULL);

	CHKmalloc(pNew = MALLOC(sizeof(toDeleteLst_t)));
	pNew->deqID = deqID;
	pNew->nElemDeq = nElemDeq;

	/* now find right spot */
	for(  pPrev = pQueue->toDeleteLst
	    ; pPrev != NULL && deqID > pPrev->deqID
	    ; pPrev = pPrev->pNext) {
		/*JUST SEARCH*/;
	}

	if(pPrev == NULL) {
		pNew->pNext = pQueue->toDeleteLst;
		pQueue->toDeleteLst = pNew;
	} else {
		pNew->pNext = pPrev->pNext;
		pPrev->pNext = pNew;
	}

finalize_it:
	RETiRet;
}


/* methods */

static const char *
getQueueTypeName(queueType_t t)
{
	const char *r;

	switch(t) {
	case QUEUETYPE_FIXED_ARRAY:
		r = "FixedArray";
		break;
	case QUEUETYPE_LINKEDLIST:
		r = "LinkedList";
		break;
	case QUEUETYPE_DISK:
		r = "Disk";
		break;
	case QUEUETYPE_DIRECT:
		r = "Direct";
		break;
	default:
		r = "invalid/unknown queue mode";
		break;
	}
	return r;
}

void
qqueueDbgPrint(qqueue_t *pThis)
{
	dbgoprint((obj_t*) pThis, "parameter dump:\n");
	dbgoprint((obj_t*) pThis, "queue.filename '%s'\n",
		(pThis->pszFilePrefix == NULL) ? "[NONE]" : (char*)pThis->pszFilePrefix);
	dbgoprint((obj_t*) pThis, "queue.size: %d\n", pThis->iMaxQueueSize);
	dbgoprint((obj_t*) pThis, "queue.dequeuebatchsize: %d\n", pThis->iDeqBatchSize);
	dbgoprint((obj_t*) pThis, "queue.maxdiskspace: %lld\n", pThis->sizeOnDiskMax);
	dbgoprint((obj_t*) pThis, "queue.highwatermark: %d\n", pThis->iHighWtrMrk);
	dbgoprint((obj_t*) pThis, "queue.lowwatermark: %d\n", pThis->iLowWtrMrk);
	dbgoprint((obj_t*) pThis, "queue.fulldelaymark: %d\n", pThis->iFullDlyMrk);
	dbgoprint((obj_t*) pThis, "queue.lightdelaymark: %d\n", pThis->iLightDlyMrk);
	dbgoprint((obj_t*) pThis, "queue.discardmark: %d\n", pThis->iDiscardMrk);
	dbgoprint((obj_t*) pThis, "queue.discardseverity: %d\n", pThis->iDiscardSeverity);
	dbgoprint((obj_t*) pThis, "queue.checkpointinterval: %d\n", pThis->iPersistUpdCnt);
	dbgoprint((obj_t*) pThis, "queue.syncqueuefiles: %d\n", pThis->bSyncQueueFiles);
	dbgoprint((obj_t*) pThis, "queue.type: %d [%s]\n", pThis->qType, getQueueTypeName(pThis->qType));
	dbgoprint((obj_t*) pThis, "queue.workerthreads: %d\n", pThis->iNumWorkerThreads);
	dbgoprint((obj_t*) pThis, "queue.timeoutshutdown: %d\n", pThis->toQShutdown);
	dbgoprint((obj_t*) pThis, "queue.timeoutactioncompletion: %d\n", pThis->toActShutdown);
	dbgoprint((obj_t*) pThis, "queue.timeoutenqueue: %d\n", pThis->toEnq);
	dbgoprint((obj_t*) pThis, "queue.timeoutworkerthreadshutdown: %d\n", pThis->toWrkShutdown);
	dbgoprint((obj_t*) pThis, "queue.workerthreadminimummessages: %d\n", pThis->iMinMsgsPerWrkr);
	dbgoprint((obj_t*) pThis, "queue.maxfilesize: %lld\n", pThis->iMaxFileSize);
	dbgoprint((obj_t*) pThis, "queue.saveonshutdown: %d\n", pThis->bSaveOnShutdown);
	dbgoprint((obj_t*) pThis, "queue.dequeueslowdown: %d\n", pThis->iDeqSlowdown);
	dbgoprint((obj_t*) pThis, "queue.dequeuetimebegin: %d\n", pThis->iDeqtWinFromHr);
	dbgoprint((obj_t*) pThis, "queue.dequeuetimeend: %d\n", pThis->iDeqtWinToHr);
}


/* get the physical queue size. Must only be called
 * while mutex is locked!
 * rgerhards, 2008-01-29
 */
static int
getPhysicalQueueSize(qqueue_t *pThis)
{
	return (int) PREFER_FETCH_32BIT(pThis->iQueueSize);
}


/* get the logical queue size (that is store size minus logically dequeued elements).
 * Must only be called while mutex is locked!
 * rgerhards, 2009-05-19
 */
static int
getLogicalQueueSize(qqueue_t *pThis)
{
	return pThis->iQueueSize - pThis->nLogDeq;
}



/* This function drains the queue in cases where this needs to be done. The most probable
 * reason is a HUP which needs to discard data (because the queue is configured to be lossy).
 * During a shutdown, this is typically not needed, as the OS frees up ressources and does
 * this much quicker than when we clean up ourselvs. -- rgerhards, 2008-10-21
 * This function returns void, as it makes no sense to communicate an error back, even if
 * it happens.
 * This functions works "around" the regular deque mechanism, because it is only used to
 * clean up (in cases where message loss is acceptable).
 */
static void queueDrain(qqueue_t *pThis)
{
	smsg_t *pMsg;
	ASSERT(pThis != NULL);

	BEGINfunc
	DBGOPRINT((obj_t*) pThis, "queue (type %d) will lose %d messages, destroying...\n",
		pThis->qType, pThis->iQueueSize);
	/* iQueueSize is not decremented by qDel(), so we need to do it ourselves */
	while(ATOMIC_DEC_AND_FETCH(&pThis->iQueueSize, &pThis->mutQueueSize) > 0) {
		pThis->qDeq(pThis, &pMsg);
		if(pMsg != NULL) {
			msgDestruct(&pMsg);
		}
		pThis->qDel(pThis);
	}
	ENDfunc
}


/* --------------- code for disk-assisted (DA) queue modes -------------------- */


/* returns the number of workers that should be advised at
 * this point in time. The mutex must be locked when
 * ths function is called. -- rgerhards, 2008-01-25
 */
static rsRetVal
qqueueAdviseMaxWorkers(qqueue_t *pThis)
{
	DEFiRet;
	int iMaxWorkers;

	ISOBJ_TYPE_assert(pThis, qqueue);

	if(!pThis->bEnqOnly) {
		if(pThis->bIsDA && getLogicalQueueSize(pThis) >= pThis->iHighWtrMrk) {
			DBGOPRINT((obj_t*) pThis, "(re)activating DA worker\n");
			wtpAdviseMaxWorkers(pThis->pWtpDA, 1); /* disk queues have always one worker */
		}
		if(getLogicalQueueSize(pThis) == 0) {
			iMaxWorkers = 0;
		} else if(pThis->qType == QUEUETYPE_DISK || pThis->iMinMsgsPerWrkr == 0) {
			iMaxWorkers = 1;
		} else {
			iMaxWorkers = getLogicalQueueSize(pThis) / pThis->iMinMsgsPerWrkr + 1;
		}
		wtpAdviseMaxWorkers(pThis->pWtpReg, iMaxWorkers);
	}

	RETiRet;
}


/* check if we run in disk-assisted mode and record that
 * setting for easy (and quick!) access in the future. This
 * function must only be called from constructors and only
 * from those that support disk-assisted modes (aka memory-
 * based queue drivers).
 * rgerhards, 2008-01-14
 */
static rsRetVal
qqueueChkIsDA(qqueue_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	if(pThis->pszFilePrefix != NULL) {
		pThis->bIsDA = 1;
		DBGOPRINT((obj_t*) pThis, "is disk-assisted, disk will be used on demand\n");
	} else {
		DBGOPRINT((obj_t*) pThis, "is NOT disk-assisted\n");
	}

	RETiRet;
}


/* Start disk-assisted queue mode.
 * rgerhards, 2008-01-15
 */
static rsRetVal
StartDA(qqueue_t *pThis)
{
	DEFiRet;
	uchar pszDAQName[128];

	ISOBJ_TYPE_assert(pThis, qqueue);

	/* create message queue */
	CHKiRet(qqueueConstruct(&pThis->pqDA, QUEUETYPE_DISK , 1, 0, pThis->pConsumer));

	/* give it a name */
	snprintf((char*) pszDAQName, sizeof(pszDAQName), "%s[DA]", obj.GetName((obj_t*) pThis));
	obj.SetName((obj_t*) pThis->pqDA, pszDAQName);

	/* as the created queue is the same object class, we take the
	 * liberty to access its properties directly.
	 */
	pThis->pqDA->pqParent = pThis;

	CHKiRet(qqueueSetpAction(pThis->pqDA, pThis->pAction));
	CHKiRet(qqueueSetsizeOnDiskMax(pThis->pqDA, pThis->sizeOnDiskMax));
	CHKiRet(qqueueSetiDeqSlowdown(pThis->pqDA, pThis->iDeqSlowdown));
	CHKiRet(qqueueSetMaxFileSize(pThis->pqDA, pThis->iMaxFileSize));
	CHKiRet(qqueueSetFilePrefix(pThis->pqDA, pThis->pszFilePrefix, pThis->lenFilePrefix));
	CHKiRet(qqueueSetSpoolDir(pThis->pqDA, pThis->pszSpoolDir, pThis->lenSpoolDir));
	CHKiRet(qqueueSetiPersistUpdCnt(pThis->pqDA, pThis->iPersistUpdCnt));
	CHKiRet(qqueueSetbSyncQueueFiles(pThis->pqDA, pThis->bSyncQueueFiles));
	CHKiRet(qqueueSettoActShutdown(pThis->pqDA, pThis->toActShutdown));
	CHKiRet(qqueueSettoEnq(pThis->pqDA, pThis->toEnq));
	CHKiRet(qqueueSetiDeqtWinFromHr(pThis->pqDA, pThis->iDeqtWinFromHr));
	CHKiRet(qqueueSetiDeqtWinToHr(pThis->pqDA, pThis->iDeqtWinToHr));
	CHKiRet(qqueueSettoQShutdown(pThis->pqDA, pThis->toQShutdown));
	CHKiRet(qqueueSetiHighWtrMrk(pThis->pqDA, 0));
	CHKiRet(qqueueSetiDiscardMrk(pThis->pqDA, 0));

	iRet = qqueueStart(pThis->pqDA);
	/* file not found is expected, that means it is no previous QIF available */
	if(iRet != RS_RET_OK && iRet != RS_RET_FILE_NOT_FOUND) {
		errno = 0; /* else an errno is shown in errmsg! */
		LogError(errno, iRet, "error starting up disk queue, using pure in-memory mode");
		pThis->bIsDA = 0;	/* disable memory mode */
		FINALIZE; /* something is wrong */
	}

	DBGOPRINT((obj_t*) pThis, "DA queue initialized, disk queue 0x%lx\n",
		  qqueueGetID(pThis->pqDA));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pThis->pqDA != NULL) {
			qqueueDestruct(&pThis->pqDA);
		}
		LogError(0, iRet, "%s: error creating disk queue - giving up.",
			obj.GetName((obj_t*)pThis));
		pThis->bIsDA = 0;
	}

	RETiRet;
}


/* initiate DA mode
 * param bEnqOnly tells if the disk queue is to be run in enqueue-only mode. This may
 * be needed during shutdown of memory queues which need to be persisted to disk.
 * If this function fails (should not happen), DA mode is not turned on.
 * rgerhards, 2008-01-16
 */
static rsRetVal
InitDA(qqueue_t *pThis, int bLockMutex)
{
	DEFiRet;
	DEFVARS_mutexProtection;
	uchar pszBuf[64];
	size_t lenBuf;

	BEGIN_MTX_PROTECTED_OPERATIONS(pThis->mut, bLockMutex);
	/* check if we already have a DA worker pool. If not, initiate one. Please note that the
	 * pool is created on first need but never again destructed (until the queue is). This
	 * is intentional. We assume that when we need it once, we may also need it on another
	 * occasion. Ressources used are quite minimal when no worker is running.
	 * rgerhards, 2008-01-24
	 * NOTE: this is the DA worker *pool*, not the DA queue!
	 */
	lenBuf = snprintf((char*)pszBuf, sizeof(pszBuf), "%s:DAwpool", obj.GetName((obj_t*) pThis));
	CHKiRet(wtpConstruct		(&pThis->pWtpDA));
	CHKiRet(wtpSetDbgHdr		(pThis->pWtpDA, pszBuf, lenBuf));
	CHKiRet(wtpSetpfChkStopWrkr	(pThis->pWtpDA, (rsRetVal (*)(void *pUsr, int)) qqueueChkStopWrkrDA));
	CHKiRet(wtpSetpfGetDeqBatchSize	(pThis->pWtpDA, (rsRetVal (*)(void *pUsr, int*)) GetDeqBatchSize));
	CHKiRet(wtpSetpfDoWork		(pThis->pWtpDA, (rsRetVal (*)(void *pUsr, void *pWti)) ConsumerDA));
	CHKiRet(wtpSetpfObjProcessed	(pThis->pWtpDA, (rsRetVal (*)(void *pUsr, wti_t *pWti)) batchProcessed));
	CHKiRet(wtpSetpmutUsr		(pThis->pWtpDA, pThis->mut));
	CHKiRet(wtpSetiNumWorkerThreads	(pThis->pWtpDA, 1));
	CHKiRet(wtpSettoWrkShutdown	(pThis->pWtpDA, pThis->toWrkShutdown));
	CHKiRet(wtpSetpUsr		(pThis->pWtpDA, pThis));
	CHKiRet(wtpConstructFinalize	(pThis->pWtpDA));
	/* if we reach this point, we have a "good" DA worker pool */

	/* now construct the actual queue (if it does not already exist) */
	if(pThis->pqDA == NULL) {
		CHKiRet(StartDA(pThis));
	}

finalize_it:
	END_MTX_PROTECTED_OPERATIONS(pThis->mut);
	RETiRet;
}


/* --------------- end code for disk-assisted queue modes -------------------- */


/* Now, we define type-specific handlers. The provide a generic functionality,
 * but for this specific type of queue. The mapping to these handlers happens during
 * queue construction. Later on, handlers are called by pointers present in the
 * queue instance object.
 */

/* -------------------- fixed array -------------------- */
static rsRetVal qConstructFixedArray(qqueue_t *pThis)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	if(pThis->iMaxQueueSize == 0)
		ABORT_FINALIZE(RS_RET_QSIZE_ZERO);

	if((pThis->tVars.farray.pBuf = MALLOC(sizeof(void *) * pThis->iMaxQueueSize)) == NULL) {
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}

	pThis->tVars.farray.deqhead = 0;
	pThis->tVars.farray.head = 0;
	pThis->tVars.farray.tail = 0;

	qqueueChkIsDA(pThis);

finalize_it:
	RETiRet;
}


static rsRetVal qDestructFixedArray(qqueue_t *pThis)
{
	DEFiRet;
	
	ASSERT(pThis != NULL);

	queueDrain(pThis); /* discard any remaining queue entries */
	free(pThis->tVars.farray.pBuf);

	RETiRet;
}


static rsRetVal qAddFixedArray(qqueue_t *pThis, smsg_t* in)
{
	DEFiRet;

	ASSERT(pThis != NULL);
	pThis->tVars.farray.pBuf[pThis->tVars.farray.tail] = in;
	pThis->tVars.farray.tail++;
	if (pThis->tVars.farray.tail == pThis->iMaxQueueSize)
		pThis->tVars.farray.tail = 0;

	RETiRet;
}


static rsRetVal qDeqFixedArray(qqueue_t *pThis, smsg_t **out)
{
	DEFiRet;

	ASSERT(pThis != NULL);
	*out = (void*) pThis->tVars.farray.pBuf[pThis->tVars.farray.deqhead];

	pThis->tVars.farray.deqhead++;
	if (pThis->tVars.farray.deqhead == pThis->iMaxQueueSize)
		pThis->tVars.farray.deqhead = 0;

	RETiRet;
}


static rsRetVal qDelFixedArray(qqueue_t *pThis)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	pThis->tVars.farray.head++;
	if (pThis->tVars.farray.head == pThis->iMaxQueueSize)
		pThis->tVars.farray.head = 0;

	RETiRet;
}


/* -------------------- linked list  -------------------- */


static rsRetVal qConstructLinkedList(qqueue_t *pThis)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	pThis->tVars.linklist.pDeqRoot = NULL;
	pThis->tVars.linklist.pDelRoot = NULL;
	pThis->tVars.linklist.pLast = NULL;

	qqueueChkIsDA(pThis);

	RETiRet;
}


static rsRetVal qDestructLinkedList(qqueue_t __attribute__((unused)) *pThis)
{
	DEFiRet;

	queueDrain(pThis); /* discard any remaining queue entries */

	/* with the linked list type, there is nothing left to do here. The
	 * reason is that there are no dynamic elements for the list itself.
	 */

	RETiRet;
}

static rsRetVal qAddLinkedList(qqueue_t *pThis, smsg_t* pMsg)
{
	qLinkedList_t *pEntry;
	DEFiRet;

	CHKmalloc((pEntry = (qLinkedList_t*) MALLOC(sizeof(qLinkedList_t))));

	pEntry->pNext = NULL;
	pEntry->pMsg = pMsg;

	if(pThis->tVars.linklist.pDelRoot == NULL) {
		pThis->tVars.linklist.pDelRoot = pThis->tVars.linklist.pDeqRoot = pThis->tVars.linklist.pLast
		= pEntry;
	} else {
		pThis->tVars.linklist.pLast->pNext = pEntry;
		pThis->tVars.linklist.pLast = pEntry;
	}

	if(pThis->tVars.linklist.pDeqRoot == NULL) {
		pThis->tVars.linklist.pDeqRoot = pEntry;
	}

finalize_it:
	RETiRet;
}


static rsRetVal qDeqLinkedList(qqueue_t *pThis, smsg_t **ppMsg)
{
	qLinkedList_t *pEntry;
	DEFiRet;

	pEntry = pThis->tVars.linklist.pDeqRoot;
	*ppMsg = pEntry->pMsg;
	pThis->tVars.linklist.pDeqRoot = pEntry->pNext;

	RETiRet;
}


static rsRetVal qDelLinkedList(qqueue_t *pThis)
{
	qLinkedList_t *pEntry;
	DEFiRet;

	pEntry = pThis->tVars.linklist.pDelRoot;

	if(pThis->tVars.linklist.pDelRoot == pThis->tVars.linklist.pLast) {
		pThis->tVars.linklist.pDelRoot = pThis->tVars.linklist.pDeqRoot = pThis->tVars.linklist.pLast = NULL;
	} else {
		pThis->tVars.linklist.pDelRoot = pEntry->pNext;
	}

	free(pEntry);

	RETiRet;
}


/* -------------------- disk  -------------------- */


/* The following function is used to "save" ourself from being killed by
 * a fatally failed disk queue. A fatal failure is, for example, if no
 * data can be read or written. In that case, the disk support is disabled,
 * with all on-disk structures kept as-is as much as possible. Instead, the
 * queue is switched to direct mode, so that at least
 * some processing can happen. Of course, this may still have lots of
 * undesired side-effects, but is probably better than aborting the
 * syslogd. Note that this function *must* succeed in one way or another, as
 * we can not recover from failure here. But it may emit different return
 * states, which can trigger different processing in the higher layers.
 * rgerhards, 2011-05-03
 */
static rsRetVal
queueSwitchToEmergencyMode(qqueue_t *pThis, rsRetVal initiatingError)
{
	pThis->iQueueSize = 0;
	pThis->nLogDeq = 0;
	qDestructDisk(pThis); /* free disk structures */

	pThis->qType = QUEUETYPE_DIRECT;
	pThis->qConstruct = qConstructDirect;
	pThis->qDestruct = qDestructDirect;
	/* these entry points shall not be used in direct mode
	 * To catch program errors, make us abort if that happens!
	 * rgerhards, 2013-11-05
	 */
	pThis->qAdd = qAddDirect;
	pThis->MultiEnq = qqueueMultiEnqObjDirect;
	pThis->qDel = NULL;
	if(pThis->pqParent != NULL) {
		DBGOPRINT((obj_t*) pThis, "DA queue is in emergency mode, disabling DA in parent\n");
		pThis->pqParent->bIsDA = 0;
		pThis->pqParent->pqDA = NULL;
		/* This may have undesired side effects, not sure if I really evaluated
		 * all. So you know where to look at if you come to this point during
		 * troubleshooting ;) -- rgerhards, 2011-05-03
		 */
	}

	LogError(0, initiatingError, "fatal error on disk queue '%s', "
		"emergency switch to direct mode", obj.GetName((obj_t*) pThis));
	return RS_RET_ERR_QUEUE_EMERGENCY;
}


static rsRetVal
qqueueLoadPersStrmInfoFixup(strm_t *pStrm, qqueue_t __attribute__((unused)) *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_TYPE_assert(pThis, qqueue);
	CHKiRet(strm.SetDir(pStrm, pThis->pszSpoolDir, pThis->lenSpoolDir));
	CHKiRet(strm.SetbSync(pStrm, pThis->bSyncQueueFiles));
finalize_it:
	RETiRet;
}


/* The method loads the persistent queue information.
 * rgerhards, 2008-01-11
 */
static rsRetVal
qqueueTryLoadPersistedInfo(qqueue_t *pThis)
{
	DEFiRet;
	strm_t *psQIF = NULL;
	struct stat stat_buf;

	ISOBJ_TYPE_assert(pThis, qqueue);

	/* check if the file exists */
	if(stat((char*) pThis->pszQIFNam, &stat_buf) == -1) {
		if(errno == ENOENT) {
			DBGOPRINT((obj_t*) pThis, "clean startup, no .qi file found\n");
		} else {
			LogError(errno, RS_RET_IO_ERROR, "queue: %s: error %d could not access .qi file",
					obj.GetName((obj_t*) pThis), errno);
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
	}

	/* If we reach this point, we have a .qi file */

	CHKiRet(strm.Construct(&psQIF));
	CHKiRet(strm.SettOperationsMode(psQIF, STREAMMODE_READ));
	CHKiRet(strm.SetsType(psQIF, STREAMTYPE_FILE_SINGLE));
	CHKiRet(strm.SetFName(psQIF, pThis->pszQIFNam, pThis->lenQIFNam));
	CHKiRet(strm.ConstructFinalize(psQIF));

	/* first, we try to read the property bag for ourselfs */
	CHKiRet(obj.DeserializePropBag((obj_t*) pThis, psQIF));
	
	/* then the stream objects (same order as when persisted!) */
	CHKiRet(obj.Deserialize(&pThis->tVars.disk.pWrite, (uchar*) "strm", psQIF,
			       (rsRetVal(*)(obj_t*,void*))qqueueLoadPersStrmInfoFixup, pThis));
	CHKiRet(obj.Deserialize(&pThis->tVars.disk.pReadDel, (uchar*) "strm", psQIF,
			       (rsRetVal(*)(obj_t*,void*))qqueueLoadPersStrmInfoFixup, pThis));
	/* create a duplicate for the read "pointer". */
	CHKiRet(strm.Dup(pThis->tVars.disk.pReadDel, &pThis->tVars.disk.pReadDeq));
	CHKiRet(strm.SetbDeleteOnClose(pThis->tVars.disk.pReadDeq, 0)); /* deq must NOT delete the files! */
	CHKiRet(strm.ConstructFinalize(pThis->tVars.disk.pReadDeq));
	/* if we use a crypto provider, we need to amend the objects with it's info */
	if(pThis->useCryprov) {
		CHKiRet(strm.Setcryprov(pThis->tVars.disk.pWrite, &pThis->cryprov));
		CHKiRet(strm.SetcryprovData(pThis->tVars.disk.pWrite, pThis->cryprovData));
		CHKiRet(strm.Setcryprov(pThis->tVars.disk.pReadDeq, &pThis->cryprov));
		CHKiRet(strm.SetcryprovData(pThis->tVars.disk.pReadDeq, pThis->cryprovData));
		CHKiRet(strm.Setcryprov(pThis->tVars.disk.pReadDel, &pThis->cryprov));
		CHKiRet(strm.SetcryprovData(pThis->tVars.disk.pReadDel, pThis->cryprovData));
	}

	CHKiRet(strm.SeekCurrOffs(pThis->tVars.disk.pWrite));
	CHKiRet(strm.SeekCurrOffs(pThis->tVars.disk.pReadDel));
	CHKiRet(strm.SeekCurrOffs(pThis->tVars.disk.pReadDeq));

	/* OK, we could successfully read the file, so we now can request that it be
	 * deleted when we are done with the persisted information.
	 */
	pThis->bNeedDelQIF = 1;

finalize_it:
	if(psQIF != NULL)
		strm.Destruct(&psQIF);

	if(iRet != RS_RET_OK) {
		DBGOPRINT((obj_t*) pThis, "state %d reading .qi file - can not read persisted info (if any)\n",
			  iRet);
	}

	RETiRet;
}


/* disk queue constructor.
 * Note that we use a file limit of 10,000,000 files. That number should never pose a
 * problem. If so, I guess the user has a design issue... But of course, the code can
 * always be changed (though it would probably be more appropriate to increase the
 * allowed file size at this point - that should be a config setting...
 * rgerhards, 2008-01-10
 */
static rsRetVal qConstructDisk(qqueue_t *pThis)
{
	DEFiRet;
	int bRestarted = 0;

	ASSERT(pThis != NULL);

	/* and now check if there is some persistent information that needs to be read in */
	iRet = qqueueTryLoadPersistedInfo(pThis);
	if(iRet == RS_RET_OK)
		bRestarted = 1;
	else if(iRet != RS_RET_FILE_NOT_FOUND)
			FINALIZE;

	if(bRestarted == 1) {
		;
	} else {
		CHKiRet(strm.Construct(&pThis->tVars.disk.pWrite));
		CHKiRet(strm.SetbSync(pThis->tVars.disk.pWrite, pThis->bSyncQueueFiles));
		CHKiRet(strm.SetDir(pThis->tVars.disk.pWrite, pThis->pszSpoolDir, pThis->lenSpoolDir));
		CHKiRet(strm.SetiMaxFiles(pThis->tVars.disk.pWrite, 10000000));
		CHKiRet(strm.SettOperationsMode(pThis->tVars.disk.pWrite, STREAMMODE_WRITE));
		CHKiRet(strm.SetsType(pThis->tVars.disk.pWrite, STREAMTYPE_FILE_CIRCULAR));
		if(pThis->useCryprov) {
			CHKiRet(strm.Setcryprov(pThis->tVars.disk.pWrite, &pThis->cryprov));
			CHKiRet(strm.SetcryprovData(pThis->tVars.disk.pWrite, pThis->cryprovData));
		}
		CHKiRet(strm.ConstructFinalize(pThis->tVars.disk.pWrite));

		CHKiRet(strm.Construct(&pThis->tVars.disk.pReadDeq));
		CHKiRet(strm.SetbDeleteOnClose(pThis->tVars.disk.pReadDeq, 0));
		CHKiRet(strm.SetDir(pThis->tVars.disk.pReadDeq, pThis->pszSpoolDir, pThis->lenSpoolDir));
		CHKiRet(strm.SetiMaxFiles(pThis->tVars.disk.pReadDeq, 10000000));
		CHKiRet(strm.SettOperationsMode(pThis->tVars.disk.pReadDeq, STREAMMODE_READ));
		CHKiRet(strm.SetsType(pThis->tVars.disk.pReadDeq, STREAMTYPE_FILE_CIRCULAR));
		if(pThis->useCryprov) {
			CHKiRet(strm.Setcryprov(pThis->tVars.disk.pReadDeq, &pThis->cryprov));
			CHKiRet(strm.SetcryprovData(pThis->tVars.disk.pReadDeq, pThis->cryprovData));
		}
		CHKiRet(strm.ConstructFinalize(pThis->tVars.disk.pReadDeq));

		CHKiRet(strm.Construct(&pThis->tVars.disk.pReadDel));
		CHKiRet(strm.SetbSync(pThis->tVars.disk.pReadDel, pThis->bSyncQueueFiles));
		CHKiRet(strm.SetbDeleteOnClose(pThis->tVars.disk.pReadDel, 1));
		CHKiRet(strm.SetDir(pThis->tVars.disk.pReadDel, pThis->pszSpoolDir, pThis->lenSpoolDir));
		CHKiRet(strm.SetiMaxFiles(pThis->tVars.disk.pReadDel, 10000000));
		CHKiRet(strm.SettOperationsMode(pThis->tVars.disk.pReadDel, STREAMMODE_READ));
		CHKiRet(strm.SetsType(pThis->tVars.disk.pReadDel, STREAMTYPE_FILE_CIRCULAR));
		if(pThis->useCryprov) {
			CHKiRet(strm.Setcryprov(pThis->tVars.disk.pReadDel, &pThis->cryprov));
			CHKiRet(strm.SetcryprovData(pThis->tVars.disk.pReadDel, pThis->cryprovData));
		}
		CHKiRet(strm.ConstructFinalize(pThis->tVars.disk.pReadDel));

		CHKiRet(strm.SetFName(pThis->tVars.disk.pWrite,   pThis->pszFilePrefix, pThis->lenFilePrefix));
		CHKiRet(strm.SetFName(pThis->tVars.disk.pReadDeq, pThis->pszFilePrefix, pThis->lenFilePrefix));
		CHKiRet(strm.SetFName(pThis->tVars.disk.pReadDel, pThis->pszFilePrefix, pThis->lenFilePrefix));
	}

	/* now we set (and overwrite in case of a persisted restart) some parameters which
	 * should always reflect the current configuration variables. Be careful by doing so,
	 * for example file name generation must not be changed as that would break the
	 * ability to read existing queue files. -- rgerhards, 2008-01-12
	 */
	CHKiRet(strm.SetiMaxFileSize(pThis->tVars.disk.pWrite, pThis->iMaxFileSize));
	CHKiRet(strm.SetiMaxFileSize(pThis->tVars.disk.pReadDeq, pThis->iMaxFileSize));
	CHKiRet(strm.SetiMaxFileSize(pThis->tVars.disk.pReadDel, pThis->iMaxFileSize));

finalize_it:
	RETiRet;
}


static rsRetVal qDestructDisk(qqueue_t *pThis)
{
	DEFiRet;
	
	ASSERT(pThis != NULL);

	free(pThis->pszQIFNam);
	if(pThis->tVars.disk.pWrite != NULL) {
		int64 currOffs;
		strm.GetCurrOffset(pThis->tVars.disk.pWrite, &currOffs);
		if(currOffs == 0) {
			/* if no data is present, we can (and must!) delete this
			 * file. Else we can leave garbagge after termination.
			 */
			strm.SetbDeleteOnClose(pThis->tVars.disk.pWrite, 1);
		}
		strm.Destruct(&pThis->tVars.disk.pWrite);
	}
	if(pThis->tVars.disk.pReadDeq != NULL)
		strm.Destruct(&pThis->tVars.disk.pReadDeq);
	if(pThis->tVars.disk.pReadDel != NULL)
		strm.Destruct(&pThis->tVars.disk.pReadDel);

	RETiRet;
}

static rsRetVal qAddDisk(qqueue_t *pThis, smsg_t* pMsg)
{
	DEFiRet;
	number_t nWriteCount;
	const int oldfile = strmGetCurrFileNum(pThis->tVars.disk.pWrite);

	ASSERT(pThis != NULL);

	CHKiRet(strm.SetWCntr(pThis->tVars.disk.pWrite, &nWriteCount));
	CHKiRet((objSerialize(pMsg))(pMsg, pThis->tVars.disk.pWrite));
	CHKiRet(strm.Flush(pThis->tVars.disk.pWrite));
	CHKiRet(strm.SetWCntr(pThis->tVars.disk.pWrite, NULL)); /* no more counting for now... */

	pThis->tVars.disk.sizeOnDisk += nWriteCount;

	/* we have enqueued the user element to disk. So we now need to destruct
	 * the in-memory representation. The instance will be re-created upon
	 * dequeue. -- rgerhards, 2008-07-09
	 */
	msgDestruct(&pMsg);

	DBGOPRINT((obj_t*) pThis, "write wrote %lld octets to disk, queue disk size now %lld octets, EnqOnly:%d\n",
		   nWriteCount, pThis->tVars.disk.sizeOnDisk, pThis->bEnqOnly);

	/* Did we have a change in the on-disk file? If so, we
	 * should do a "robustness sync" of the .qi file to guard
	 * against the most harsh consequences of kill -9 and power off.
	 */
	int newfile;
	newfile = strmGetCurrFileNum(pThis->tVars.disk.pWrite);
	if(newfile != oldfile) {
		DBGOPRINT((obj_t*) pThis, "current to-be-written-to file has changed from "
			"number %d to number %d - requiring a .qi write for robustness\n",
			oldfile, newfile);
		pThis->tVars.disk.nForcePersist = 2;
	}

finalize_it:
	RETiRet;
}


static rsRetVal
qDeqDisk(qqueue_t *pThis, smsg_t **ppMsg)
{
	DEFiRet;
	iRet = objDeserializeWithMethods(ppMsg, (uchar*) "msg", 3,
		pThis->tVars.disk.pReadDeq, NULL,
		NULL, msgConstructForDeserializer, NULL, MsgDeserialize);
	if(iRet != RS_RET_OK) {
		LogError(0, iRet, "%s: qDeqDisk error happened at around offset %lld",
			obj.GetName((obj_t*)pThis),
			(long long) pThis->tVars.disk.pReadDeq->iCurrOffs);
	}
	RETiRet;
}


/* -------------------- direct (no queueing) -------------------- */
static rsRetVal qConstructDirect(qqueue_t __attribute__((unused)) *pThis)
{
	return RS_RET_OK;
}


static rsRetVal qDestructDirect(qqueue_t __attribute__((unused)) *pThis)
{
	return RS_RET_OK;
}

static rsRetVal qAddDirectWithWti(qqueue_t *pThis, smsg_t* pMsg, wti_t *pWti)
{
	batch_t singleBatch;
	batch_obj_t batchObj;
	batch_state_t batchState = BATCH_STATE_RDY;
	DEFiRet;

	//TODO: init batchObj (states _OK and new fields -- CHECK)
	ASSERT(pThis != NULL);

	/* calling the consumer is quite different here than it is from a worker thread */
	/* we need to provide the consumer's return value back to the caller because in direct
	 * mode the consumer probably has a lot to convey (which get's lost in the other modes
	 * because they are asynchronous. But direct mode is deliberately synchronous.
	 * rgerhards, 2008-02-12
	 * We use our knowledge about the batch_t structure below, but without that, we
	 * pay a too-large performance toll... -- rgerhards, 2009-04-22
	 */
	memset(&batchObj, 0, sizeof(batch_obj_t));
	memset(&singleBatch, 0, sizeof(batch_t));
	batchObj.pMsg = pMsg;
	singleBatch.nElem = 1; /* there always is only one in direct mode */
	singleBatch.pElem = &batchObj;
	singleBatch.eltState = &batchState;
	iRet = pThis->pConsumer(pThis->pAction, &singleBatch, pWti);
	msgDestruct(&pMsg);

	RETiRet;
}

/* this is called if we do not have a pWti. This currently only happens
 * when we are called from a main queue in direct mode. If so, we need
 * to obtain a dummy pWti.
 */
static rsRetVal
qAddDirect(qqueue_t *pThis, smsg_t* pMsg)
{
	wti_t *pWti;
	DEFiRet;

	pWti = wtiGetDummy();
	pWti->pbShutdownImmediate = &pThis->bShutdownImmediate;
	iRet = qAddDirectWithWti(pThis, pMsg, pWti);
	RETiRet;
}


/* --------------- end type-specific handlers -------------------- */


/* generic code to add a queue entry
 * We use some specific code to most efficiently support direct mode
 * queues. This is justified in spite of the gain and the need to do some
 * things truely different. -- rgerhards, 2008-02-12
 */
static rsRetVal
qqueueAdd(qqueue_t *pThis, smsg_t *pMsg)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	static int msgCnt = 0;

	if(pThis->iSmpInterval > 0)
	{
		msgCnt = (msgCnt + 1) % (pThis->iSmpInterval);
		if(msgCnt != 0)
		{
		        msgDestruct(&pMsg);
			goto finalize_it;
		}
	}

	CHKiRet(pThis->qAdd(pThis, pMsg));

	if(pThis->qType != QUEUETYPE_DIRECT) {
		ATOMIC_INC(&pThis->iQueueSize, &pThis->mutQueueSize);
#		ifdef ENABLE_IMDIAG
#			ifdef HAVE_ATOMIC_BUILTINS
				/* mutex is never used due to conditional compilation */
				ATOMIC_INC(&iOverallQueueSize, &NULL);
#			else
				++iOverallQueueSize; /* racy, but we can't wait for a mutex! */
#			endif
#		endif
		DBGOPRINT((obj_t*) pThis, "qqueueAdd: entry added, size now log %d, phys %d entries\n",
			  getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));
	}

finalize_it:
	RETiRet;
}


/* generic code to dequeue a queue entry
 */
static rsRetVal
qqueueDeq(qqueue_t *pThis, smsg_t **ppMsg)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	/* we do NOT abort if we encounter an error, because otherwise the queue
	 * will not be decremented, what will most probably result in an endless loop.
	 * If we decrement, however, we may lose a message. But that is better than
	 * losing the whole process because it loops... -- rgerhards, 2008-01-03
	 */
	iRet = pThis->qDeq(pThis, ppMsg);
	ATOMIC_INC(&pThis->nLogDeq, &pThis->mutLogDeq);

//	DBGOPRINT((obj_t*) pThis, "entry deleted, size now log %d, phys %d entries\n",
//		  getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));

	RETiRet;
}


/* Try to shut down regular and DA queue workers, within the queue timeout
 * period. That means processing continues as usual. This is the expected
 * usual case, where during shutdown those messages remaining are being
 * processed. At this point, it is acceptable that the queue can not be
 * fully depleted, that case is handled in the next step. During this phase,
 * we first shut down the main queue DA worker to prevent new data to arrive
 * at the DA queue, and then we ask the regular workers of both the Regular
 * and DA queue to try complete processing.
 * rgerhards, 2009-10-14
 */
static rsRetVal ATTR_NONNULL(1)
tryShutdownWorkersWithinQueueTimeout(qqueue_t *const pThis)
{
	struct timespec tTimeout;
	rsRetVal iRetLocal;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ASSERT(pThis->pqParent == NULL); /* detect invalid calling sequence */

	if(pThis->bIsDA) {
		/* We need to lock the mutex, as otherwise we may have a race that prevents
		 * us from awaking the DA worker. */
		d_pthread_mutex_lock(pThis->mut);

		/* tell regular queue DA worker to stop shuffling messages to DA queue... */
		DBGOPRINT((obj_t*) pThis, "setting EnqOnly mode for DA worker\n");
		pThis->pqDA->bEnqOnly = 1;
		wtpSetState(pThis->pWtpDA, wtpState_SHUTDOWN_IMMEDIATE);
		wtpAdviseMaxWorkers(pThis->pWtpDA, 1);
		DBGOPRINT((obj_t*) pThis, "awoke DA worker, told it to shut down.\n");

		/* also tell the DA queue worker to shut down, so that it already knows... */
		wtpSetState(pThis->pqDA->pWtpReg, wtpState_SHUTDOWN);
		wtpAdviseMaxWorkers(pThis->pqDA->pWtpReg, 1); /* awake its lone worker */
		DBGOPRINT((obj_t*) pThis, "awoke DA queue regular worker, told it to shut down when done.\n");

		d_pthread_mutex_unlock(pThis->mut);
	}


	/* first calculate absolute timeout - we need the absolute value here, because we need to coordinate
	 * shutdown of both the regular and DA queue on *the same* timeout.
	 */
	timeoutComp(&tTimeout, pThis->toQShutdown);
	DBGOPRINT((obj_t*) pThis, "trying shutdown of regular workers\n");
	iRetLocal = wtpShutdownAll(pThis->pWtpReg, wtpState_SHUTDOWN, &tTimeout);
	if(iRetLocal == RS_RET_TIMED_OUT) {
		LogMsg(0, RS_RET_TIMED_OUT, LOG_INFO,
			"%s: regular queue shutdown timed out on primary queue "
			"(this is OK, timeout was %d)",
			objGetName((obj_t*) pThis), pThis->toQShutdown);
	} else {
		DBGOPRINT((obj_t*) pThis, "regular queue workers shut down.\n");
	}

	/* OK, the worker for the regular queue is processed, on the the DA queue regular worker. */
	if(pThis->pqDA != NULL) {
		DBGOPRINT((obj_t*) pThis, "we have a DA queue (0x%lx), requesting its shutdown.\n",
			 qqueueGetID(pThis->pqDA));
		/* we use the same absolute timeout as above, so we do not use more than the configured
		 * timeout interval!
		 */
		DBGOPRINT((obj_t*) pThis, "trying shutdown of regular worker of DA queue\n");
		iRetLocal = wtpShutdownAll(pThis->pqDA->pWtpReg, wtpState_SHUTDOWN, &tTimeout);
		if(iRetLocal == RS_RET_TIMED_OUT) {
			LogMsg(0, RS_RET_TIMED_OUT, LOG_INFO,
				"%s: regular queue shutdown timed out on DA queue (this is OK, "
				"timeout was %d)", objGetName((obj_t*) pThis), pThis->toQShutdown);
		} else {
			DBGOPRINT((obj_t*) pThis, "DA queue worker shut down.\n");
		}
	}

	RETiRet;
}


/* Try to shut down regular and DA queue workers, within the action timeout
 * period. This aborts processing, but at the end of the current action, in
 * a well-defined manner. During this phase, we terminate all three worker
 * pools, including the regular queue DA worker if it not yet has terminated.
 * Not finishing processing all messages is OK (and expected) at this stage
 * (they may be preserved later, depending * on bSaveOnShutdown setting).
 * rgerhards, 2009-10-14
 */
static rsRetVal
tryShutdownWorkersWithinActionTimeout(qqueue_t *pThis)
{
	struct timespec tTimeout;
	rsRetVal iRetLocal;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ASSERT(pThis->pqParent == NULL); /* detect invalid calling sequence */

	/* instruct workers to finish ASAP, even if still work exists */
	DBGOPRINT((obj_t*) pThis, "trying to shutdown workers within Action Timeout");
	DBGOPRINT((obj_t*) pThis, "setting EnqOnly mode\n");
	pThis->bEnqOnly = 1;
	pThis->bShutdownImmediate = 1;
	/* now DA queue */
	if(pThis->bIsDA) {
		pThis->pqDA->bEnqOnly = 1;
		pThis->pqDA->bShutdownImmediate = 1;
	}

// TODO: make sure we have at minimum a 10ms timeout - workers deserve a chance...
	/* now give the queue workers a last chance to gracefully shut down (based on action timeout setting) */
	timeoutComp(&tTimeout, pThis->toActShutdown);
	DBGOPRINT((obj_t*) pThis, "trying immediate shutdown of regular workers (if any)\n");
	iRetLocal = wtpShutdownAll(pThis->pWtpReg, wtpState_SHUTDOWN_IMMEDIATE, &tTimeout);
	if(iRetLocal == RS_RET_TIMED_OUT) {
		LogMsg(0, RS_RET_TIMED_OUT, LOG_INFO,
			"%s: immediate shutdown timed out on primary queue (this is acceptable and "
			  "triggers cancellation)", objGetName((obj_t*) pThis));
	} else if(iRetLocal != RS_RET_OK) {
		LogMsg(0, iRetLocal, LOG_WARNING,
			"%s: potential internal error: unexpected return state after trying "
			"immediate shutdown of the primary queue in disk save mode. "
			"Continuing, but results are unpredictable", objGetName((obj_t*) pThis));
	}

	if(pThis->pqDA != NULL) {
		/* and now the same for the DA queue */
		DBGOPRINT((obj_t*) pThis, "trying immediate shutdown of DA queue workers\n");
		iRetLocal = wtpShutdownAll(pThis->pqDA->pWtpReg, wtpState_SHUTDOWN_IMMEDIATE, &tTimeout);
		if(iRetLocal == RS_RET_TIMED_OUT) {
			LogMsg(0, RS_RET_TIMED_OUT, LOG_INFO,
				"%s: immediate shutdown timed out on DA queue (this is acceptable and "
				  "triggers cancellation)", objGetName((obj_t*) pThis));
		} else if(iRetLocal != RS_RET_OK) {
			LogMsg(0, iRetLocal, LOG_WARNING,
				"%s: potential internal error: unexpected return state after trying "
				"immediate shutdown of the DA queue in disk save mode. "
				"Continuing, but results are unpredictable", objGetName((obj_t*) pThis));
		}

		/* and now we need to terminate the DA worker itself. We always grant it a 100ms timeout,
		 * which should be sufficient and usually not be required (it is expected to have finished
		 * long before while we were processing the queue timeout in shutdown phase 1).
		 * rgerhards, 2009-10-14
		 */
		timeoutComp(&tTimeout, 100);
		DBGOPRINT((obj_t*) pThis, "trying regular shutdown of main queue DA worker pool\n");
		iRetLocal = wtpShutdownAll(pThis->pWtpDA, wtpState_SHUTDOWN_IMMEDIATE, &tTimeout);
		if(iRetLocal == RS_RET_TIMED_OUT) {
			LogMsg(0, iRetLocal, LOG_WARNING,
				"%s: shutdown timed out on main queue DA worker pool "
				"(this is not good, but possibly OK)",
				objGetName((obj_t*) pThis));
		} else {
			DBGOPRINT((obj_t*) pThis, "main queue DA worker pool shut down.\n");
		}
	}

	RETiRet;
}


/* This function cancels all remaining regular workers for both the main and the DA
 * queue.
 * rgerhards, 2009-05-29
 */
static rsRetVal
cancelWorkers(qqueue_t *pThis)
{
	rsRetVal iRetLocal;
	DEFiRet;

	assert(pThis->qType != QUEUETYPE_DIRECT);

	/* Now queue workers should have terminated. If not, we need to cancel them as we have applied
	 * all timeout setting. If any worker in any queue still executes, its consumer is possibly
	 * long-running and cancelling is the only way to get rid of it.
	 */
	DBGOPRINT((obj_t*) pThis, "checking to see if we need to cancel any worker threads of the primary queue\n");
	iRetLocal = wtpCancelAll(pThis->pWtpReg, objGetName((obj_t*) pThis));
		/* ^-- returns immediately if all threads already have terminated */
	if(iRetLocal != RS_RET_OK) {
		DBGOPRINT((obj_t*) pThis, "unexpected iRet state %d trying to cancel primary queue worker "
			  "threads, continuing, but results are unpredictable\n", iRetLocal);
	}

	/* ... and now the DA queue, if it exists (should always be after the primary one) */
	if(pThis->pqDA != NULL) {
		DBGOPRINT((obj_t*) pThis, "checking to see if we need to cancel any worker threads of "
			"the DA queue\n");
		iRetLocal = wtpCancelAll(pThis->pqDA->pWtpReg, objGetName((obj_t*) pThis));
		/* returns immediately if all threads already have terminated */
		if(iRetLocal != RS_RET_OK) {
			DBGOPRINT((obj_t*) pThis, "unexpected iRet state %d trying to cancel DA queue worker "
				  "threads, continuing, but results are unpredictable\n", iRetLocal);
		}

		/* finally, we cancel the main queue's DA worker pool, if it still is running. It may be
		 * restarted later to persist the queue. But we stop it, because otherwise we get into
		 * big trouble when resetting the logical dequeue pointer. This operation can only be
		 * done when *no* worker is running. So time for a shutdown... -- rgerhards, 2009-05-28
		 */
		DBGOPRINT((obj_t*) pThis, "checking to see if main queue DA worker pool needs to be cancelled\n");
		wtpCancelAll(pThis->pWtpDA, objGetName((obj_t*) pThis));
			/* returns immediately if all threads already have terminated */
	}

	RETiRet;
}


/* This function shuts down all worker threads and waits until they
 * have terminated. If they timeout, they are cancelled.
 * rgerhards, 2008-01-24
 * Please note that this function shuts down BOTH the parent AND the child queue
 * in DA case. This is necessary because their timeouts are tightly coupled. Most
 * importantly, the timeouts would be applied twice (or logic be extremely
 * complex) if each would have its own shutdown. The function does not self check
 * this condition - the caller must make sure it is not called with a parent.
 * rgerhards, 2009-05-26: we do NO longer persist the queue here if bSaveOnShutdown
 * is set. This must be handled by the caller. Not doing that cleans up the queue
 * shutdown considerably. Also, older engines had a potential hang condition when
 * the DA queue was already started and the DA worker configured for infinite
 * retries and the action was during retry processing. This was a design issue,
 * which is solved as of now. Note that the shutdown now may take a little bit
 * longer, because we no longer can persist the queue in parallel to waiting
 * on worker timeouts.
 */
rsRetVal
qqueueShutdownWorkers(qqueue_t *const pThis)
{
	DEFiRet;

	if(pThis->qType == QUEUETYPE_DIRECT) {
		FINALIZE;
	}

	ISOBJ_TYPE_assert(pThis, qqueue);
	ASSERT(pThis->pqParent == NULL); /* detect invalid calling sequence */

	DBGOPRINT((obj_t*) pThis, "initiating worker thread shutdown sequence %p\n", pThis);

	CHKiRet(tryShutdownWorkersWithinQueueTimeout(pThis));

	pthread_mutex_lock(pThis->mut);
	int physQueueSize;
	physQueueSize = getPhysicalQueueSize(pThis);
	pthread_mutex_unlock(pThis->mut);
	if(physQueueSize > 0) {
		CHKiRet(tryShutdownWorkersWithinActionTimeout(pThis));
	}

	CHKiRet(cancelWorkers(pThis));

	/* ... finally ... all worker threads have terminated :-)
	 * Well, more precisely, they *are in termination*. Some cancel cleanup handlers
	 * may still be running. Note that the main queue's DA worker may still be running.
	 */
	DBGOPRINT((obj_t*) pThis, "worker threads terminated, remaining queue size log %d, phys %d.\n",
		  getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));

finalize_it:
	RETiRet;
}

/* Constructor for the queue object
 * This constructs the data structure, but does not yet start the queue. That
 * is done by queueStart(). The reason is that we want to give the caller a chance
 * to modify some parameters before the queue is actually started.
 */
rsRetVal qqueueConstruct(qqueue_t **ppThis, queueType_t qType, int iWorkerThreads,
		        int iMaxQueueSize, rsRetVal (*pConsumer)(void*, batch_t*, wti_t*))
{
	DEFiRet;
	qqueue_t *pThis;
	const uchar *const workDir = glblGetWorkDirRaw();

	ASSERT(ppThis != NULL);
	ASSERT(pConsumer != NULL);
	ASSERT(iWorkerThreads >= 0);

	CHKmalloc(pThis = (qqueue_t *)calloc(1, sizeof(qqueue_t)));

	/* we have an object, so let's fill the properties */
	objConstructSetObjInfo(pThis);

	if(workDir != NULL) {
		if((pThis->pszSpoolDir = ustrdup(workDir)) == NULL)
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
		pThis->lenSpoolDir = ustrlen(pThis->pszSpoolDir);
	}
	/* set some water marks so that we have useful defaults if none are set specifically */
	pThis->iFullDlyMrk  = -1;
	pThis->iLightDlyMrk = -1;
	pThis->iMaxFileSize = 1024 * 1024; /* default is 1 MiB */
	pThis->iQueueSize = 0;
	pThis->nLogDeq = 0;
	pThis->useCryprov = 0;
	pThis->iMaxQueueSize = iMaxQueueSize;
	pThis->pConsumer = pConsumer;
	pThis->iNumWorkerThreads = iWorkerThreads;
	pThis->iDeqtWinToHr = 25; /* disable time-windowed dequeuing by default */
	pThis->iDeqBatchSize = 8; /* conservative default, should still provide good performance */

	pThis->pszFilePrefix = NULL;
	pThis->qType = qType;


	INIT_ATOMIC_HELPER_MUT(pThis->mutQueueSize);
	INIT_ATOMIC_HELPER_MUT(pThis->mutLogDeq);

finalize_it:
	OBJCONSTRUCT_CHECK_SUCCESS_AND_CLEANUP
	RETiRet;
}


/* set default inside queue object suitable for action queues.
 * This shall be called directly after queue construction. This functions has
 * been added in support of the new v6 config system. It expect properly pre-initialized
 * objects, but we need to differentiate between ruleset main and action queues.
 * In order to avoid unnecessary complexity, we provide the necessary defaults
 * via specific function calls.
 */
void
qqueueSetDefaultsActionQueue(qqueue_t *pThis)
{
	pThis->qType = QUEUETYPE_DIRECT;	/* type of the main message queue above */
	pThis->iMaxQueueSize = 1000;		/* size of the main message queue above */
	pThis->iDeqBatchSize = 128; 		/* default batch size */
	pThis->iHighWtrMrk = -1;		/* high water mark for disk-assisted queues */
	pThis->iLowWtrMrk = -1;			/* low water mark for disk-assisted queues */
	pThis->iDiscardMrk = -1;		/* begin to discard messages */
	pThis->iDiscardSeverity = 8;		/* turn off */
	pThis->iNumWorkerThreads = 1;		/* number of worker threads for the mm queue above */
	pThis->iMaxFileSize = 1024*1024;
	pThis->iPersistUpdCnt = 0;		/* persist queue info every n updates */
	pThis->bSyncQueueFiles = 0;
	pThis->toQShutdown = 0;			/* queue shutdown */
	pThis->toActShutdown = 1000;		/* action shutdown (in phase 2) */
	pThis->toEnq = 2000;			/* timeout for queue enque */
	pThis->toWrkShutdown = 60000;		/* timeout for worker thread shutdown */
	pThis->iMinMsgsPerWrkr = -1;		/* minimum messages per worker needed to start a new one */
	pThis->bSaveOnShutdown = 1;		/* save queue on shutdown (when DA enabled)? */
	pThis->sizeOnDiskMax = 0;		/* unlimited */
	pThis->iDeqSlowdown = 0;
	pThis->iDeqtWinFromHr = 0;
	pThis->iDeqtWinToHr = 25;		 /* disable time-windowed dequeuing by default */
	pThis->iSmpInterval = 0;                 /* disable sampling */
}


/* set defaults inside queue object suitable for main/ruleset queues.
 * See queueSetDefaultsActionQueue() for more details and background.
 */
void
qqueueSetDefaultsRulesetQueue(qqueue_t *pThis)
{
	pThis->qType = QUEUETYPE_FIXED_ARRAY;	/* type of the main message queue above */
	pThis->iMaxQueueSize = 50000;		/* size of the main message queue above */
	pThis->iDeqBatchSize = 1024; 		/* default batch size */
	pThis->iHighWtrMrk = -1;		/* high water mark for disk-assisted queues */
	pThis->iLowWtrMrk = -1;			/* low water mark for disk-assisted queues */
	pThis->iDiscardMrk = -1;		/* begin to discard messages */
	pThis->iDiscardSeverity = 8;		/* turn off */
	pThis->iNumWorkerThreads = 1;		/* number of worker threads for the mm queue above */
	pThis->iMaxFileSize = 16*1024*1024;
	pThis->iPersistUpdCnt = 0;		/* persist queue info every n updates */
	pThis->bSyncQueueFiles = 0;
	pThis->toQShutdown = 1500;			/* queue shutdown */
	pThis->toActShutdown = 1000;		/* action shutdown (in phase 2) */
	pThis->toEnq = 2000;			/* timeout for queue enque */
	pThis->toWrkShutdown = 60000;		/* timeout for worker thread shutdown */
	pThis->iMinMsgsPerWrkr = -1;		/* minimum messages per worker needed to start a new one */
	pThis->bSaveOnShutdown = 1;		/* save queue on shutdown (when DA enabled)? */
	pThis->sizeOnDiskMax = 0;		/* unlimited */
	pThis->iDeqSlowdown = 0;
	pThis->iDeqtWinFromHr = 0;
	pThis->iDeqtWinToHr = 25;		 /* disable time-windowed dequeuing by default */
	pThis->iSmpInterval = 0;                 /* disable sampling */
}


/* This function checks if the provided message shall be discarded and does so, if needed.
 * In DA mode, we do not discard any messages as we assume the disk subsystem is fast enough to
 * provide real-time creation of spool files.
 * Note: cached copies of iQueueSize is provided so that no mutex locks are required.
 * The caller must have obtained them while the mutex was locked. Of course, these values may no
 * longer be current, but that is OK for the discard check. At worst, the message is either processed
 * or discarded when it should not have been. As discarding is in itself somewhat racy and erratic,
 * that is no problems for us. This function MUST NOT lock the queue mutex, it could result in
 * deadlocks!
 * If the message is discarded, it can no longer be processed by the caller. So be sure to check
 * the return state!
 * rgerhards, 2008-01-24
 */
static int qqueueChkDiscardMsg(qqueue_t *pThis, int iQueueSize, smsg_t *pMsg)
{
	DEFiRet;
	rsRetVal iRetLocal;
	int iSeverity;

	ISOBJ_TYPE_assert(pThis, qqueue);

	if(pThis->iDiscardMrk > 0 && iQueueSize >= pThis->iDiscardMrk) {
		iRetLocal = MsgGetSeverity(pMsg, &iSeverity);
		if(iRetLocal == RS_RET_OK && iSeverity >= pThis->iDiscardSeverity) {
			DBGOPRINT((obj_t*) pThis, "queue nearly full (%d entries), discarded severity %d message\n",
				  iQueueSize, iSeverity);
			STATSCOUNTER_INC(pThis->ctrNFDscrd, pThis->mutCtrNFDscrd);
			msgDestruct(&pMsg);
			ABORT_FINALIZE(RS_RET_QUEUE_FULL);
		} else {
			DBGOPRINT((obj_t*) pThis, "queue nearly full (%d entries), but could not drop msg "
				  "(iRet: %d, severity %d)\n", iQueueSize, iRetLocal, iSeverity);
		}
	}

finalize_it:
	RETiRet;
}


/* Finally remove n elements from the queue store.
 */
static rsRetVal
DoDeleteBatchFromQStore(qqueue_t *pThis, int nElem)
{
	int i;
	off64_t bytesDel = 0; /* keep CLANG static anaylzer happy */
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);

	/* now send delete request to storage driver */
	if(pThis->qType == QUEUETYPE_DISK) {
		strmMultiFileSeek(pThis->tVars.disk.pReadDel, pThis->tVars.disk.deqFileNumOut,
				  pThis->tVars.disk.deqOffs, &bytesDel);
		/* We need to correct the on-disk file size. This time it is a bit tricky:
		 * we free disk space only upon file deletion. So we need to keep track of what we
		 * have read until we get an out-offset that is lower than the in-offset (which
		 * indicates file change). Then, we can subtract the whole thing from the on-disk
		 * size. -- rgerhards, 2008-01-30
		 */
		 if(bytesDel != 0) {
			pThis->tVars.disk.sizeOnDisk -= bytesDel;
			DBGOPRINT((obj_t*) pThis, "doDeleteBatch: a %lld octet file has been deleted, now %lld "
				"octets disk space used\n", (long long) bytesDel, pThis->tVars.disk.sizeOnDisk);
			/* awake possibly waiting enq process */
			pthread_cond_signal(&pThis->notFull); /* we hold the mutex while we are in here! */
		}
	} else { /* memory queue */
		for(i = 0 ; i < nElem ; ++i) {
			pThis->qDel(pThis);
		}
	}

	/* iQueueSize is not decremented by qDel(), so we need to do it ourselves */
	ATOMIC_SUB(&pThis->iQueueSize, nElem, &pThis->mutQueueSize);
#	ifdef ENABLE_IMDIAG
#		ifdef HAVE_ATOMIC_BUILTINS
			/* mutex is never used due to conditional compilation */
			ATOMIC_SUB(&iOverallQueueSize, nElem, &NULL);
#		else
			iOverallQueueSize -= nElem; /* racy, but we can't wait for a mutex! */
#		endif
#	endif
	ATOMIC_SUB(&pThis->nLogDeq, nElem, &pThis->mutLogDeq);
	DBGPRINTF("doDeleteBatch: delete batch from store, new sizes: log %d, phys %d\n",
		  getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));
	++pThis->deqIDDel; /* one more batch dequeued */

	if((pThis->qType == QUEUETYPE_DISK) && (bytesDel != 0)) {
		qqueuePersist(pThis, QUEUE_CHECKPOINT); /* robustness persist .qi file */
	}

	RETiRet;
}


/* remove messages from the physical queue store that are fully processed. This is
 * controlled via the to-delete list.
 */
static rsRetVal
DeleteBatchFromQStore(qqueue_t *pThis, batch_t *pBatch)
{
	toDeleteLst_t *pTdl;
	qDeqID	deqIDDel;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	assert(pBatch != NULL);

	pTdl = tdlPeek(pThis); /* get current head element */
	if(pTdl == NULL) { /* to-delete list empty */
		DoDeleteBatchFromQStore(pThis, pBatch->nElem);
	} else if(pBatch->deqID == pThis->deqIDDel) {
		deqIDDel = pThis->deqIDDel;
		pTdl = tdlPeek(pThis);
		while(pTdl != NULL && deqIDDel == pTdl->deqID) {
			DoDeleteBatchFromQStore(pThis, pTdl->nElemDeq);
			tdlPop(pThis);
			++deqIDDel;
			pTdl = tdlPeek(pThis);
		}
		/* old entries deleted, now delete current ones... */
		DoDeleteBatchFromQStore(pThis, pBatch->nElem);
	} else {
		/* can not delete, insert into to-delete list */
		DBGPRINTF("not at head of to-delete list, enqueue %d\n", (int) pBatch->deqID);
		CHKiRet(tdlAdd(pThis, pBatch->deqID, pBatch->nElem));
	}

finalize_it:
	RETiRet;
}


/* Delete a batch of processed user objects from the queue, which includes
 * destructing the objects themself. Any entries not marked as finally
 * processed are enqueued again. The new enqueue is necessary because we have a
 * rgerhards, 2009-05-13
 */
static rsRetVal
DeleteProcessedBatch(qqueue_t *pThis, batch_t *pBatch)
{
	int i;
	smsg_t *pMsg;
	int nEnqueued = 0;
	rsRetVal localRet;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	assert(pBatch != NULL);

	for(i = 0 ; i < pBatch->nElem ; ++i) {
		pMsg = pBatch->pElem[i].pMsg;
		DBGPRINTF("DeleteProcessedBatch: etry %d state %d\n", i, pBatch->eltState[i]);
		if(   pBatch->eltState[i] == BATCH_STATE_RDY
		   || pBatch->eltState[i] == BATCH_STATE_SUB) {
			localRet = doEnqSingleObj(pThis, eFLOWCTL_NO_DELAY, MsgAddRef(pMsg));
			++nEnqueued;
			if(localRet != RS_RET_OK) {
				DBGPRINTF("DeleteProcessedBatch: error %d re-enqueuing unprocessed "
						"data element - discarded\n", localRet);
			}
		}
		msgDestruct(&pMsg);
	}

	DBGPRINTF("DeleteProcessedBatch: we deleted %d objects and enqueued %d objects\n", i-nEnqueued, nEnqueued);

	if(nEnqueued > 0)
		qqueueChkPersist(pThis, nEnqueued);

	iRet = DeleteBatchFromQStore(pThis, pBatch);

	pBatch->nElem = pBatch->nElemDeq = 0; /* reset batch */ // TODO: more fine init, new fields! 2010-06-14

	RETiRet;
}


/* dequeue as many user pointers as are available, until we hit the configured
 * upper limit of pointers. Note that this function also deletes all processed
 * objects from the previous batch. However, it is perfectly valid that the
 * previous batch contained NO objects at all. For example, this happens
 * immediately after system startup or when a queue was exhausted and the queue
 * worker needed to wait for new data.
 * This must only be called when the queue mutex is LOOKED, otherwise serious
 * malfunction will happen.
 */
static rsRetVal
DequeueConsumableElements(qqueue_t *pThis, wti_t *pWti, int *piRemainingQueueSize, int *const pSkippedMsgs)
{
	int nDequeued;
	int nDiscarded;
	int nDeleted;
	int iQueueSize;
	smsg_t *pMsg;
	rsRetVal localRet;
	DEFiRet;

	nDeleted = pWti->batch.nElemDeq;
	DeleteProcessedBatch(pThis, &pWti->batch);

	nDequeued = nDiscarded = 0;
	if(pThis->qType == QUEUETYPE_DISK) {
		pThis->tVars.disk.deqFileNumIn = strmGetCurrFileNum(pThis->tVars.disk.pReadDeq);
	}

	while((iQueueSize = getLogicalQueueSize(pThis)) > 0 && nDequeued < pThis->iDeqBatchSize) {
		int rd_fd = -1;
		int64_t rd_offs = 0;
		int wr_fd = -1;
		int64_t wr_offs = 0;
		if(pThis->tVars.disk.pReadDeq != NULL) {
			rd_fd = strmGetCurrFileNum(pThis->tVars.disk.pReadDeq);
			rd_offs = pThis->tVars.disk.pReadDeq->iCurrOffs;
		}
		if(pThis->tVars.disk.pWrite != NULL) {
			wr_fd = strmGetCurrFileNum(pThis->tVars.disk.pWrite);
			wr_offs = pThis->tVars.disk.pWrite->iCurrOffs;
		}
		if(rd_fd != -1 && rd_fd == wr_fd && rd_offs == wr_offs) {
			DBGPRINTF("problem on disk queue '%s': "
					//"queue size log %d, phys %d, but rd_fd=wr_rd=%d and offs=%lld\n",
					"queue size log %d, phys %d, but rd_fd=wr_rd=%d and offs=%" PRId64 "\n",
					obj.GetName((obj_t*) pThis), iQueueSize, pThis->iQueueSize,
					rd_fd, rd_offs);
			*pSkippedMsgs = iQueueSize;
#			ifdef ENABLE_IMDIAG
			iOverallQueueSize -= iQueueSize;
#			endif
			pThis->iQueueSize -= iQueueSize;
			iQueueSize = 0;
			break;
		}

		localRet = qqueueDeq(pThis, &pMsg);
		if(localRet == RS_RET_FILE_NOT_FOUND) {
			DBGPRINTF("fatal error on disk queue '%s': file '%s' "
				"not found, queue size said to be %d",
				obj.GetName((obj_t*) pThis), "...", iQueueSize);
		}
		CHKiRet(localRet);

		/* check if we should discard this element */
		localRet = qqueueChkDiscardMsg(pThis, pThis->iQueueSize, pMsg);
		if(localRet == RS_RET_QUEUE_FULL) {
			++nDiscarded;
			continue;
		} else if(localRet != RS_RET_OK) {
			ABORT_FINALIZE(localRet);
		}

		/* all well, use this element */
		pWti->batch.pElem[nDequeued].pMsg = pMsg;
		pWti->batch.eltState[nDequeued] = BATCH_STATE_RDY;
		++nDequeued;
	}

	if(pThis->qType == QUEUETYPE_DISK) {
		strm.GetCurrOffset(pThis->tVars.disk.pReadDeq, &pThis->tVars.disk.deqOffs);
		pThis->tVars.disk.deqFileNumOut = strmGetCurrFileNum(pThis->tVars.disk.pReadDeq);
	}

	/* it is sufficient to persist only when the bulk of work is done */
	qqueueChkPersist(pThis, nDequeued+nDiscarded+nDeleted);

	DBGOPRINT((obj_t*) pThis, "dequeued %d consumable elements, szlog %d sz phys %d\n",
		nDequeued, getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));
	pWti->batch.nElem = nDequeued;
	pWti->batch.nElemDeq = nDequeued + nDiscarded;
	pWti->batch.deqID = getNextDeqID(pThis);
	*piRemainingQueueSize = iQueueSize;
finalize_it:
	RETiRet;
}


/* dequeue the queued object for the queue consumers.
 * rgerhards, 2008-10-21
 * I made a radical change - we now dequeue multiple elements, and store these objects in
 * an array of user pointers. We expect that this increases performance.
 * rgerhards, 2009-04-22
 */
static rsRetVal
DequeueConsumable(qqueue_t *pThis, wti_t *pWti, int *const pSkippedMsgs)
{
	DEFiRet;
	int iQueueSize = 0; /* keep the compiler happy... */

	*pSkippedMsgs = 0;
	/* dequeue element batch (still protected from mutex) */
	iRet = DequeueConsumableElements(pThis, pWti, &iQueueSize, pSkippedMsgs);
	if(*pSkippedMsgs > 0) {
		LogError(0, RS_RET_ERR, "%s: lost %d messages from diskqueue (invalid .qi file)",
			obj.GetName((obj_t*)pThis), *pSkippedMsgs);
	}

	/* awake some flow-controlled sources if we can do this right now */
	/* TODO: this could be done better from a performance point of view -- do it only if
	 * we have someone waiting for the condition (or only when we hit the watermark right
	 * on the nail [exact value]) -- rgerhards, 2008-03-14
	 * now that we dequeue batches of pointers, this is much less an issue...
	 * rgerhards, 2009-04-22
	 */
	if(iQueueSize < pThis->iFullDlyMrk / 2 || glbl.GetGlobalInputTermState() == 1) {
		pthread_cond_broadcast(&pThis->belowFullDlyWtrMrk);
	}

	if(iQueueSize < pThis->iLightDlyMrk / 2) {
		pthread_cond_broadcast(&pThis->belowLightDlyWtrMrk);
	}

	pthread_cond_signal(&pThis->notFull);
	/* WE ARE NO LONGER PROTECTED BY THE MUTEX */

	if(iRet != RS_RET_OK && iRet != RS_RET_DISCARDMSG) {
		LogError(0, iRet, "%s: error dequeueing element - ignoring, "
			"but strange things may happen", obj.GetName((obj_t*)pThis));
	}

	RETiRet;
}


/* The rate limiter
 *
 * IMPORTANT: the rate-limiter MUST unlock and re-lock the queue when
 * it actually delays processing. Otherwise inputs are stalled.
 *
 * Here we may wait if a dequeue time window is defined or if we are
 * rate-limited. TODO: If we do so, we should also look into the
 * way new worker threads are spawned. Obviously, it doesn't make much
 * sense to spawn additional worker threads when none of them can do any
 * processing. However, it is deemed acceptable to allow this for an initial
 * implementation of the timeframe/rate limiting feature.
 * Please also note that these feature could also be implemented at the action
 * level. However, that would limit them to be used together with actions. We have
 * taken the broader approach, moving it right into the queue. This is even
 * necessary if we want to prevent spawning of multiple unnecessary worker
 * threads as described above. -- rgerhards, 2008-04-02
 *
 *
 * time window: tCurr is current time; tFrom is start time, tTo is end time (in mil 24h format).
 * We may have tFrom = 4, tTo = 10 --> run from 4 to 10 hrs. nice and happy
 * we may also have tFrom= 22, tTo = 4 -> run from 10pm to 4am, which is actually two
 *     windows: 0-4; 22-23:59
 * so when to run? Let's assume we have 3am
 *
 * if(tTo < tFrom) {
 * 	if(tCurr < tTo [3 < 4] || tCurr > tFrom [3 > 22])
 * 		do work
 * 	else
 * 		sleep for tFrom - tCurr "hours" [22 - 5 --> 17]
 * } else {
 * 	if(tCurr >= tFrom [3 >= 4] && tCurr < tTo [3 < 10])
 * 		do work
 * 	else
 * 		sleep for tTo - tCurr "hours" [4 - 3 --> 1]
 * }
 *
 * Bottom line: we need to check which type of window we have and need to adjust our
 * logic accordingly. Of course, sleep calculations need to be done up to the minute,
 * but you get the idea from the code above.
 */
static rsRetVal
RateLimiter(qqueue_t *pThis)
{
	DEFiRet;
	int iDelay;
	int iHrCurr;
	time_t tCurr;
	struct tm m;

	ISOBJ_TYPE_assert(pThis, qqueue);

	iDelay = 0;
	if(pThis->iDeqtWinToHr != 25) { /* 25 means disabled */
		/* time calls are expensive, so only do them when needed */
		datetime.GetTime(&tCurr);
		localtime_r(&tCurr, &m);
		iHrCurr = m.tm_hour;

		if(pThis->iDeqtWinToHr < pThis->iDeqtWinFromHr) {
			if(iHrCurr < pThis->iDeqtWinToHr || iHrCurr > pThis->iDeqtWinFromHr) {
				; /* do not delay */
			} else {
				iDelay = (pThis->iDeqtWinFromHr - iHrCurr) * 3600;
				/* this time, we are already into the next hour, so we need
				 * to subtract our current minute and seconds.
				 */
				iDelay -= m.tm_min * 60;
				iDelay -= m.tm_sec;
			}
		} else {
			if(iHrCurr >= pThis->iDeqtWinFromHr && iHrCurr < pThis->iDeqtWinToHr) {
				; /* do not delay */
			} else {
				if(iHrCurr < pThis->iDeqtWinFromHr) {
					iDelay = (pThis->iDeqtWinFromHr - iHrCurr - 1) * 3600;
						/* -1 as we are already in the hour */
					iDelay += (60 - m.tm_min) * 60;
					iDelay += 60 - m.tm_sec;
				} else {
					iDelay = (24 - iHrCurr + pThis->iDeqtWinFromHr) * 3600;
					/* this time, we are already into the next hour, so we need
					 * to subtract our current minute and seconds.
					 */
					iDelay -= m.tm_min * 60;
					iDelay -= m.tm_sec;
				}
			}
		}
	}

	if(iDelay > 0) {
		pthread_mutex_unlock(pThis->mut);
		DBGOPRINT((obj_t*) pThis, "outside dequeue time window, delaying %d seconds\n", iDelay);
		srSleep(iDelay, 0);
		pthread_mutex_lock(pThis->mut);
	}

	RETiRet;
}


/* This dequeues the next batch. Note that this function must not be
 * cancelled, else it will leave back an inconsistent state.
 * rgerhards, 2009-05-20
 */
static rsRetVal
DequeueForConsumer(qqueue_t *pThis, wti_t *pWti, int *const pSkippedMsgs)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ISOBJ_TYPE_assert(pWti, wti);

	CHKiRet(DequeueConsumable(pThis, pWti, pSkippedMsgs));

	if(pWti->batch.nElem == 0)
		ABORT_FINALIZE(RS_RET_IDLE);


finalize_it:
	RETiRet;
}


/* This is called when a batch is processed and the worker does not
 * ask for another batch (e.g. because it is to be terminated)
 * Note that we must not be terminated while we delete a processed
 * batch. Otherwise, we may not complete it, and then the cancel
 * handler also tries to delete the batch. But then it finds some of
 * the messages already destructed. This was a bug we have seen, especially
 * with disk mode, where a delete takes rather long. Anyhow, the coneptual
 * problem exists in all queue modes.
 * rgerhards, 2009-05-27
 */
static rsRetVal
batchProcessed(qqueue_t *pThis, wti_t *pWti)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ISOBJ_TYPE_assert(pWti, wti);

	int iCancelStateSave;
	/* at this spot, we must not be cancelled */
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &iCancelStateSave);
	DeleteProcessedBatch(pThis, &pWti->batch);
	qqueueChkPersist(pThis, pWti->batch.nElemDeq);
	pthread_setcancelstate(iCancelStateSave, NULL);

	RETiRet;
}


/* This is the queue consumer in the regular (non-DA) case. It is
 * protected by the queue mutex, but MUST release it as soon as possible.
 * rgerhards, 2008-01-21
 */
static rsRetVal
ConsumerReg(qqueue_t *pThis, wti_t *pWti)
{
	int iCancelStateSave;
	int bNeedReLock = 0;	/**< do we need to lock the mutex again? */
	int skippedMsgs = 0;	/**< did the queue loose any messages (can happen with
	                         ** disk queue if .qi file is corrupt */
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ISOBJ_TYPE_assert(pWti, wti);

	iRet = DequeueForConsumer(pThis, pWti, &skippedMsgs);
	if(iRet == RS_RET_FILE_NOT_FOUND) {
		/* This is a fatal condition and means the queue is almost unusable */
		d_pthread_mutex_unlock(pThis->mut);
		DBGOPRINT((obj_t*) pThis, "got 'file not found' error %d, queue defunct\n", iRet);
		iRet = queueSwitchToEmergencyMode(pThis, iRet);
		// TODO: think about what to return as iRet -- keep RS_RET_FILE_NOT_FOUND?
		d_pthread_mutex_lock(pThis->mut);
	}
	if (iRet != RS_RET_OK) {
		FINALIZE;
	}

	/* we now have a non-idle batch of work, so we can release the queue mutex and process it */
	d_pthread_mutex_unlock(pThis->mut);
	bNeedReLock = 1;

	/* report errors, now that we are outside of queue lock */
	if(skippedMsgs > 0) {
		LogError(0, 0, "problem on disk queue '%s': "
				"queue files contain %d messages fewer than specified "
				"in .qi file -- we lost those messages. That's all we know.",
				obj.GetName((obj_t*) pThis), skippedMsgs);
	}

	/* at this spot, we may be cancelled */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &iCancelStateSave);


	pWti->pbShutdownImmediate = &pThis->bShutdownImmediate;
	CHKiRet(pThis->pConsumer(pThis->pAction, &pWti->batch, pWti));

	/* we now need to check if we should deliberately delay processing a bit
	 * and, if so, do that. -- rgerhards, 2008-01-30
	 */
//TODO: MULTIQUEUE: the following setting is no longer correct - need to think about how to do that...
	if(pThis->iDeqSlowdown) {
		DBGOPRINT((obj_t*) pThis, "sleeping %d microseconds as requested by config params\n",
			  pThis->iDeqSlowdown);
		srSleep(pThis->iDeqSlowdown / 1000000, pThis->iDeqSlowdown % 1000000);
	}

	/* but now cancellation is no longer permitted */
	pthread_setcancelstate(iCancelStateSave, NULL);

finalize_it:
	DBGPRINTF("regular consumer finished, iret=%d, szlog %d sz phys %d\n", iRet,
	          getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));

	/* now we are done, but potentially need to re-aquire the mutex */
	if(bNeedReLock)
		d_pthread_mutex_lock(pThis->mut);

	RETiRet;
}


/* This is a special consumer to feed the disk-queue in disk-assisted mode.
 * When active, our own queue more or less acts as a memory buffer to the disk.
 * So this consumer just needs to drain the memory queue and submit entries
 * to the disk queue. The disk queue will then call the actual consumer from
 * the app point of view (we chain two queues here).
 * When this method is entered, the mutex is always locked and needs to be unlocked
 * as part of the processing.
 * rgerhards, 2008-01-14
 */
static rsRetVal
ConsumerDA(qqueue_t *pThis, wti_t *pWti)
{
	int i;
	int iCancelStateSave;
	int bNeedReLock = 0;	/**< do we need to lock the mutex again? */
	int skippedMsgs = 0;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ISOBJ_TYPE_assert(pWti, wti);

	CHKiRet(DequeueForConsumer(pThis, pWti, &skippedMsgs));

	/* we now have a non-idle batch of work, so we can release the queue mutex and process it */
	d_pthread_mutex_unlock(pThis->mut);
	bNeedReLock = 1;

	/* at this spot, we may be cancelled */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &iCancelStateSave);

	/* iterate over returned results and enqueue them in DA queue */
	for(i = 0 ; i < pWti->batch.nElem && !pThis->bShutdownImmediate ; i++) {
		iRet = qqueueEnqMsg(pThis->pqDA, eFLOWCTL_NO_DELAY, MsgAddRef(pWti->batch.pElem[i].pMsg));
		if(iRet != RS_RET_OK) {
			if(iRet == RS_RET_ERR_QUEUE_EMERGENCY) {
				/* Queue emergency error occured */
				DBGOPRINT((obj_t*) pThis, "ConsumerDA:qqueueEnqMsg caught RS_RET_ERR_QUEUE_EMERGENCY,"
						"aborting loop.\n");
				FINALIZE;
			} else {
				DBGOPRINT((obj_t*) pThis, "ConsumerDA:qqueueEnqMsg item (%d) returned "
						"with error state: '%d'\n", i, iRet);
			}
		}
		pWti->batch.eltState[i] = BATCH_STATE_COMM; /* commited to other queue! */
	}

	/* but now cancellation is no longer permitted */
	pthread_setcancelstate(iCancelStateSave, NULL);

finalize_it:
	/*	Check the last return state of qqueueEnqMsg. If an error was returned, we acknowledge it only.
	*	Unless the error code is RS_RET_ERR_QUEUE_EMERGENCY, we reset the return state to RS_RET_OK.
	*	Otherwise the Caller functions would run into an infinite Loop trying to enqueue the
	*	same messages over and over again.
	*
	*	However we do NOT overwrite positive return states like
	*		RS_RET_TERMINATE_NOW,
	*		RS_RET_NO_RUN,
	*		RS_RET_IDLE,
	*		RS_RET_TERMINATE_WHEN_IDLE
	*	These return states are important for Queue handling of the upper laying functions.
	*	RGer: Note that checking for iRet < 0 is a bit bold. In theory, positive iRet
	*	values are "OK" states, and things that the caller shall deal with. However,
	*	this has not been done so consistently. Andre convinced me that the current
	*	code is an elegant solution. However, if problems with queue workers and/or
	*	shutdown come up, this code here should be looked at suspiciously. In those
	*	cases it may work out to check all status codes explicitely, just to avoid
	*	a pitfall due to unexpected states being passed on to the caller.
	*/
	if(	iRet != RS_RET_OK &&
		iRet != RS_RET_ERR_QUEUE_EMERGENCY &&
		iRet < 0) {
		DBGOPRINT((obj_t*) pThis, "ConsumerDA:qqueueEnqMsg Resetting iRet from %d back to RS_RET_OK\n", iRet);
		iRet = RS_RET_OK;
	} else {
		DBGOPRINT((obj_t*) pThis, "ConsumerDA:qqueueEnqMsg returns with iRet %d\n", iRet);
	}

	/* now we are done, but potentially need to re-aquire the mutex */
	if(bNeedReLock)
		d_pthread_mutex_lock(pThis->mut);

	RETiRet;
}


/* must only be called when the queue mutex is locked, else results
 * are not stable!
 */
static rsRetVal
qqueueChkStopWrkrDA(qqueue_t *pThis)
{
	DEFiRet;

	/*DBGPRINTF("XXXX: chkStopWrkrDA called, low watermark %d, log Size %d, phys Size %d, bEnqOnly %d\n",
	pThis->iLowWtrMrk, getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis), pThis->bEnqOnly);*/
	if(pThis->bEnqOnly) {
		iRet = RS_RET_TERMINATE_WHEN_IDLE;
	}
	if(getPhysicalQueueSize(pThis) <= pThis->iLowWtrMrk) {
		iRet = RS_RET_TERMINATE_NOW;
	}

	RETiRet;
}


/* must only be called when the queue mutex is locked, else results
 * are not stable!
 * If we are a child, we have done our duty when the queue is empty. In that case,
 * we can terminate. Version for the regular worker thread.
 */
static rsRetVal
ChkStopWrkrReg(qqueue_t *pThis)
{
	DEFiRet;
	/*DBGPRINTF("XXXX: chkStopWrkrReg called, low watermark %d, log Size %d, phys Size %d, bEnqOnly %d\n",
	pThis->iLowWtrMrk, getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis), pThis->bEnqOnly);*/
	if(pThis->bEnqOnly) {
		iRet = RS_RET_TERMINATE_NOW;
	} else if(pThis->pqParent != NULL) {
		iRet = RS_RET_TERMINATE_WHEN_IDLE;
	}

	RETiRet;
}


/* return the configured "deq max at once" interval
 * rgerhards, 2009-04-22
 */
static rsRetVal
GetDeqBatchSize(qqueue_t *pThis, int *pVal)
{
	DEFiRet;
	assert(pVal != NULL);
	*pVal = pThis->iDeqBatchSize;
if(pThis->pqParent != NULL) // TODO: check why we actually do this!
	*pVal = 16;
	RETiRet;
}


/* start up the queue - it must have been constructed and parameters defined
 * before.
 */
rsRetVal
qqueueStart(qqueue_t *pThis) /* this is the ConstructionFinalizer */
{
	DEFiRet;
	uchar pszBuf[64];
	uchar pszQIFNam[MAXFNAME];
	int wrk;
	int goodval; /* a "good value" to use for comparisons (different objects) */
	uchar *qName;
	size_t lenBuf;

	ASSERT(pThis != NULL);

	dbgoprint((obj_t*) pThis, "starting queue\n");

	if(pThis->pszSpoolDir == NULL) {
		/* note: we need to pick the path so late as we do not have
		 *       the workdir during early config load
		 */
		if((pThis->pszSpoolDir = (uchar*) strdup((char*)glbl.GetWorkDir())) == NULL)
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
		pThis->lenSpoolDir = ustrlen(pThis->pszSpoolDir);
	}
	/* set type-specific handlers and other very type-specific things
	 * (we can not totally hide it...)
	 */
	switch(pThis->qType) {
		case QUEUETYPE_FIXED_ARRAY:
			pThis->qConstruct = qConstructFixedArray;
			pThis->qDestruct = qDestructFixedArray;
			pThis->qAdd = qAddFixedArray;
			pThis->qDeq = qDeqFixedArray;
			pThis->qDel = qDelFixedArray;
			pThis->MultiEnq = qqueueMultiEnqObjNonDirect;
			break;
		case QUEUETYPE_LINKEDLIST:
			pThis->qConstruct = qConstructLinkedList;
			pThis->qDestruct = qDestructLinkedList;
			pThis->qAdd = qAddLinkedList;
			pThis->qDeq = qDeqLinkedList;
			pThis->qDel = qDelLinkedList;
			pThis->MultiEnq = qqueueMultiEnqObjNonDirect;
			break;
		case QUEUETYPE_DISK:
			pThis->qConstruct = qConstructDisk;
			pThis->qDestruct = qDestructDisk;
			pThis->qAdd = qAddDisk;
			pThis->qDeq = qDeqDisk;
			pThis->qDel = NULL; /* delete for disk handled via special code! */
			pThis->MultiEnq = qqueueMultiEnqObjNonDirect;
			/* special handling */
			pThis->iNumWorkerThreads = 1; /* we need exactly one worker */
			/* pre-construct file name for .qi file */
			pThis->lenQIFNam = snprintf((char*)pszQIFNam, sizeof(pszQIFNam),
				"%s/%s.qi", (char*) pThis->pszSpoolDir, (char*)pThis->pszFilePrefix);
			pThis->pszQIFNam = ustrdup(pszQIFNam);
			DBGOPRINT((obj_t*) pThis, ".qi file name is '%s', len %d\n", pThis->pszQIFNam,
				(int) pThis->lenQIFNam);
			break;
		case QUEUETYPE_DIRECT:
			pThis->qConstruct = qConstructDirect;
			pThis->qDestruct = qDestructDirect;
			/* these entry points shall not be used in direct mode
			 * To catch program errors, make us abort if that happens!
			 * rgerhards, 2013-11-05
			 */
			pThis->qAdd = qAddDirect;
			pThis->MultiEnq = qqueueMultiEnqObjDirect;
			pThis->qDel = NULL;
			break;
	}

	if(pThis->iMaxQueueSize < 100
	   && (pThis->qType == QUEUETYPE_LINKEDLIST || pThis->qType == QUEUETYPE_FIXED_ARRAY)) {
		LogMsg(0, RS_RET_OK_WARN, LOG_WARNING, "Note: queue.size=\"%d\" is very "
			"low and can lead to unpredictable results. See also "
			"http://www.rsyslog.com/lower-bound-for-queue-sizes/",
			pThis->iMaxQueueSize);
	}

	/* we need to do a quick check if our water marks are set plausible. If not,
	 * we correct the most important shortcomings.
	 */
	goodval = (pThis->iMaxQueueSize / 100) * 60;
	if(pThis->iHighWtrMrk != -1 && pThis->iHighWtrMrk < goodval) {
		LogMsg(0, RS_RET_CONF_PARSE_WARNING, LOG_WARNING, "queue \"%s\": high water mark "
				"is set quite low at %d. You should only set it below "
				"60%% (%d) if you have a good reason for this.",
				obj.GetName((obj_t*) pThis), pThis->iHighWtrMrk, goodval);
	}

	if(pThis->iNumWorkerThreads > 1) {
		goodval = (pThis->iMaxQueueSize / 100) * 10;
		if(pThis->iMinMsgsPerWrkr != -1 && pThis->iMinMsgsPerWrkr < goodval) {
			LogMsg(0, RS_RET_CONF_PARSE_WARNING, LOG_WARNING, "queue \"%s\": "
					"queue.workerThreadMinimumMessage "
					"is set quite low at %d. You should only set it below "
					"10%% (%d) if you have a good reason for this.",
					obj.GetName((obj_t*) pThis), pThis->iMinMsgsPerWrkr, goodval);
		}
	}

	if(pThis->iDiscardMrk > pThis->iMaxQueueSize) {
		LogError(0, RS_RET_PARAM_ERROR, "error: queue \"%s\": "
				"queue.discardMark %d is set larger than queue.size",
				obj.GetName((obj_t*) pThis), pThis->iDiscardMrk);
	}

	goodval = (pThis->iMaxQueueSize / 100) * 80;
	if(pThis->iDiscardMrk != -1 && pThis->iDiscardMrk < goodval) {
		LogMsg(0, RS_RET_CONF_PARSE_WARNING, LOG_WARNING,
				"queue \"%s\": queue.discardMark "
				"is set quite low at %d. You should only set it below "
				"80%% (%d) if you have a good reason for this.",
				obj.GetName((obj_t*) pThis), pThis->iDiscardMrk, goodval);
	}

	if(pThis->pszFilePrefix != NULL) { /* This means we have a potential DA queue */
		if(pThis->iFullDlyMrk != -1 && pThis->iFullDlyMrk < pThis->iHighWtrMrk) {
			LogMsg(0, RS_RET_CONF_WRN_FULLDLY_BELOW_HIGHWTR, LOG_WARNING,
					"queue \"%s\": queue.fullDelayMark "
					"is set below high water mark. This will result in DA mode "
					" NOT being activated for full delayable messages: In many "
					"cases this is a configuration error, please check if this "
					"is really what you want",
					obj.GetName((obj_t*) pThis));
		}
	}

	/* now come parameter corrections and defaults */
	if(pThis->iHighWtrMrk < 2 || pThis->iHighWtrMrk > pThis->iMaxQueueSize) {
		pThis->iHighWtrMrk  = (pThis->iMaxQueueSize / 100) * 90;
		if(pThis->iHighWtrMrk == 0) { /* guard against very low max queue sizes! */
			pThis->iHighWtrMrk = pThis->iMaxQueueSize;
		}
	}
	if(   pThis->iLowWtrMrk < 2
	   || pThis->iLowWtrMrk > pThis->iMaxQueueSize
	   || pThis->iLowWtrMrk > pThis->iHighWtrMrk ) {
		pThis->iLowWtrMrk  = (pThis->iMaxQueueSize / 100) * 70;
		if(pThis->iLowWtrMrk == 0) {
			pThis->iLowWtrMrk = 1;
		}
	}

	if(   pThis->iMinMsgsPerWrkr < 1
	   || pThis->iMinMsgsPerWrkr > pThis->iMaxQueueSize ) {
		pThis->iMinMsgsPerWrkr  = pThis->iMaxQueueSize / pThis->iNumWorkerThreads;
	}

	if(pThis->iFullDlyMrk == -1 || pThis->iFullDlyMrk > pThis->iMaxQueueSize) {
		pThis->iFullDlyMrk  = (pThis->iMaxQueueSize / 100) * 97;
		if(pThis->iFullDlyMrk == 0) {
			pThis->iFullDlyMrk =
				(pThis->iMaxQueueSize == 1) ? 1 : pThis->iMaxQueueSize - 1;
		}
	}
	if(pThis->iLightDlyMrk == -1 || pThis->iLightDlyMrk > pThis->iMaxQueueSize) {
		pThis->iLightDlyMrk = (pThis->iMaxQueueSize / 100) * 70;
		if(pThis->iLightDlyMrk == 0) {
			pThis->iLightDlyMrk =
				(pThis->iMaxQueueSize == 1) ? 1 : pThis->iMaxQueueSize - 1;
		}
	}

	if(pThis->iDiscardMrk < 1 || pThis->iDiscardMrk > pThis->iMaxQueueSize) {
		pThis->iDiscardMrk  = (pThis->iMaxQueueSize / 100) * 98;
		if(pThis->iDiscardMrk == 0) {
			/* for very small queues, we disable this by default */
			pThis->iDiscardMrk = pThis->iMaxQueueSize;
		}
	}

	if(pThis->iMaxQueueSize > 0 && pThis->iDeqBatchSize > pThis->iMaxQueueSize) {
		pThis->iDeqBatchSize = pThis->iMaxQueueSize;
	}

	/* finalize some initializations that could not yet be done because it is
	 * influenced by properties which might have been set after queueConstruct ()
	 */
	if(pThis->pqParent == NULL) {
		CHKmalloc(pThis->mut = (pthread_mutex_t *) MALLOC (sizeof (pthread_mutex_t)));
		pthread_mutex_init(pThis->mut, NULL);
	} else {
		/* child queue, we need to use parent's mutex */
		DBGOPRINT((obj_t*) pThis, "I am a child\n");
		pThis->mut = pThis->pqParent->mut;
	}

	pthread_mutex_init(&pThis->mutThrdMgmt, NULL);
	pthread_cond_init (&pThis->notFull, NULL);
	pthread_cond_init (&pThis->belowFullDlyWtrMrk, NULL);
	pthread_cond_init (&pThis->belowLightDlyWtrMrk, NULL);

	/* call type-specific constructor */
	CHKiRet(pThis->qConstruct(pThis)); /* this also sets bIsDA */

	/* re-adjust some params if required */
	if(pThis->bIsDA) {
		/* if we are in DA mode, we must make sure full delayable messages do not
		 * initiate going to disk!
		 */
		wrk = pThis->iHighWtrMrk - (pThis->iHighWtrMrk / 100) * 50; /* 50% of high water mark */
		if(wrk < pThis->iFullDlyMrk)
			pThis->iFullDlyMrk = wrk;
	}

	DBGOPRINT((obj_t*) pThis, "params: type %d, enq-only %d, disk assisted %d, spoolDir '%s', maxFileSz %lld, "
			          "maxQSize %d, lqsize %d, pqsize %d, child %d, full delay %d, "
				  "light delay %d, deq batch size %d, high wtrmrk %d, low wtrmrk %d, "
				  "discardmrk %d, max wrkr %d, min msgs f. wrkr %d\n",
		  pThis->qType, pThis->bEnqOnly, pThis->bIsDA, pThis->pszSpoolDir,
		  pThis->iMaxFileSize, pThis->iMaxQueueSize,
		  getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis),
		  pThis->pqParent == NULL ? 0 : 1, pThis->iFullDlyMrk, pThis->iLightDlyMrk,
		  pThis->iDeqBatchSize, pThis->iHighWtrMrk, pThis->iLowWtrMrk,
		  pThis->iDiscardMrk, pThis->iNumWorkerThreads, pThis->iMinMsgsPerWrkr);

	pThis->bQueueStarted = 1;
	if(pThis->qType == QUEUETYPE_DIRECT)
		FINALIZE;	/* with direct queues, we are already finished... */

	/* create worker thread pools for regular and DA operation.
	 */
	lenBuf = snprintf((char*)pszBuf, sizeof(pszBuf), "%s:Reg", obj.GetName((obj_t*) pThis));
	CHKiRet(wtpConstruct		(&pThis->pWtpReg));
	CHKiRet(wtpSetDbgHdr		(pThis->pWtpReg, pszBuf, lenBuf));
	CHKiRet(wtpSetpfRateLimiter	(pThis->pWtpReg, (rsRetVal (*)(void *pUsr)) RateLimiter));
	CHKiRet(wtpSetpfChkStopWrkr	(pThis->pWtpReg, (rsRetVal (*)(void *pUsr, int)) ChkStopWrkrReg));
	CHKiRet(wtpSetpfGetDeqBatchSize	(pThis->pWtpReg, (rsRetVal (*)(void *pUsr, int*)) GetDeqBatchSize));
	CHKiRet(wtpSetpfDoWork		(pThis->pWtpReg, (rsRetVal (*)(void *pUsr, void *pWti)) ConsumerReg));
	CHKiRet(wtpSetpfObjProcessed	(pThis->pWtpReg, (rsRetVal (*)(void *pUsr, wti_t *pWti)) batchProcessed));
	CHKiRet(wtpSetpmutUsr		(pThis->pWtpReg, pThis->mut));
	CHKiRet(wtpSetiNumWorkerThreads	(pThis->pWtpReg, pThis->iNumWorkerThreads));
	CHKiRet(wtpSettoWrkShutdown	(pThis->pWtpReg, pThis->toWrkShutdown));
	CHKiRet(wtpSetpUsr		(pThis->pWtpReg, pThis));
	CHKiRet(wtpConstructFinalize	(pThis->pWtpReg));

	/* set up DA system if we have a disk-assisted queue */
	if(pThis->bIsDA)
		InitDA(pThis, LOCK_MUTEX); /* initiate DA mode */

	DBGOPRINT((obj_t*) pThis, "queue finished initialization\n");

	/* if the queue already contains data, we need to start the correct number of worker threads. This can be
	 * the case when a disk queue has been loaded. If we did not start it here, it would never start.
	 */
	qqueueAdviseMaxWorkers(pThis);

	/* support statistics gathering */
	qName = obj.GetName((obj_t*)pThis);
	CHKiRet(statsobj.Construct(&pThis->statsobj));
	CHKiRet(statsobj.SetName(pThis->statsobj, qName));
	CHKiRet(statsobj.SetOrigin(pThis->statsobj, (uchar*)"core.queue"));
	/* we need to save the queue size, as the stats module initializes it to 0! */
	/* iQueueSize is a dual-use counter: no init, no mutex! */
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("size"),
		ctrType_Int, CTR_FLAG_NONE, &pThis->iQueueSize));

	STATSCOUNTER_INIT(pThis->ctrEnqueued, pThis->mutCtrEnqueued);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("enqueued"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrEnqueued));

	STATSCOUNTER_INIT(pThis->ctrFull, pThis->mutCtrFull);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("full"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrFull));

	STATSCOUNTER_INIT(pThis->ctrFDscrd, pThis->mutCtrFDscrd);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("discarded.full"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrFDscrd));
	STATSCOUNTER_INIT(pThis->ctrNFDscrd, pThis->mutCtrNFDscrd);
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("discarded.nf"),
		ctrType_IntCtr, CTR_FLAG_RESETTABLE, &pThis->ctrNFDscrd));

	pThis->ctrMaxqsize = 0; /* no mutex needed, thus no init call */
	CHKiRet(statsobj.AddCounter(pThis->statsobj, UCHAR_CONSTANT("maxqsize"),
		ctrType_Int, CTR_FLAG_NONE, &pThis->ctrMaxqsize));

	CHKiRet(statsobj.ConstructFinalize(pThis->statsobj));

finalize_it:
	if(iRet != RS_RET_OK) {
		/* note: a child uses it's parent mutex, so do not delete it! */
		if(pThis->pqParent == NULL && pThis->mut != NULL)
			free(pThis->mut);
	}
	RETiRet;
}


/* persist the queue to disk (write the .qi file). If we have something to persist, we first
 * save the information on the queue properties itself and then we call
 * the queue-type specific drivers.
 * Variable bIsCheckpoint is set to 1 if the persist is for a checkpoint,
 * and 0 otherwise.
 * rgerhards, 2008-01-10
 */
static rsRetVal
qqueuePersist(qqueue_t *pThis, int bIsCheckpoint)
{
	DEFiRet;
	char *tmpQIFName = NULL;
	strm_t *psQIF = NULL; /* Queue Info File */
	char errStr[1024];

	ASSERT(pThis != NULL);

	if(pThis->qType != QUEUETYPE_DISK) {
		if(getPhysicalQueueSize(pThis) > 0) {
			/* This error code is OK, but we will probably not implement this any time
			 * The reason is that persistence happens via DA queues. But I would like to
			 * leave the code as is, as we so have a hook in case we need one.
			 * -- rgerhards, 2008-01-28
			 */
			ABORT_FINALIZE(RS_RET_NOT_IMPLEMENTED);
		} else
			FINALIZE; /* if the queue is empty, we are happy and done... */
	}

	DBGOPRINT((obj_t*) pThis, "persisting queue to disk, %d entries...\n", getPhysicalQueueSize(pThis));

	if((bIsCheckpoint != QUEUE_CHECKPOINT) && (getPhysicalQueueSize(pThis) == 0)) {
		if(pThis->bNeedDelQIF) {
			unlink((char*)pThis->pszQIFNam);
			pThis->bNeedDelQIF = 0;
		}
		/* indicate spool file needs to be deleted */
		if(pThis->tVars.disk.pReadDel != NULL) /* may be NULL if we had a startup failure! */
			CHKiRet(strm.SetbDeleteOnClose(pThis->tVars.disk.pReadDel, 1));
		FINALIZE; /* nothing left to do, so be happy */
	}

	int lentmpQIFName;
#ifdef _AIX
	lentmpQIFName = strlen( pThis->pszQIFNam) + strlen(".tmp") + 1;
	tmpQIFName = malloc(sizeof(char)*lentmpQIFName);
	if(tmpQIFName == NULL)
		tmpQIFName = (char*)pThis->pszQIFNam;
	snprintf(tmpQIFName, lentmpQIFName, "%s.tmp", pThis->pszQIFNam);
#else
	lentmpQIFName = asprintf((char **)&tmpQIFName, "%s.tmp", pThis->pszQIFNam);
	if(tmpQIFName == NULL)
		tmpQIFName = (char*)pThis->pszQIFNam;
#endif

	CHKiRet(strm.Construct(&psQIF));
	CHKiRet(strm.SettOperationsMode(psQIF, STREAMMODE_WRITE_TRUNC));
	CHKiRet(strm.SetbSync(psQIF, pThis->bSyncQueueFiles));
	CHKiRet(strm.SetsType(psQIF, STREAMTYPE_FILE_SINGLE));
	CHKiRet(strm.SetFName(psQIF, (uchar*) tmpQIFName, lentmpQIFName));
	CHKiRet(strm.ConstructFinalize(psQIF));

	/* first, write the property bag for ourselfs
	 * And, surprisingly enough, we currently need to persist only the size of the
	 * queue. All the rest is re-created with then-current config parameters when the
	 * queue is re-created. Well, we'll also save the current queue type, just so that
	 * we know when somebody has changed the queue type... -- rgerhards, 2008-01-11
	 */
	CHKiRet(obj.BeginSerializePropBag(psQIF, (obj_t*) pThis));
	objSerializeSCALAR(psQIF, iQueueSize, INT);
	objSerializeSCALAR(psQIF, tVars.disk.sizeOnDisk, INT64);
	CHKiRet(obj.EndSerialize(psQIF));

	/* now persist the stream info */
	if(pThis->tVars.disk.pWrite != NULL)
		CHKiRet(strm.Serialize(pThis->tVars.disk.pWrite, psQIF));
	if(pThis->tVars.disk.pReadDel != NULL)
		CHKiRet(strm.Serialize(pThis->tVars.disk.pReadDel, psQIF));

	strm.Destruct(&psQIF);
	if(tmpQIFName != (char*)pThis->pszQIFNam) { /* pointer, not string comparison! */
		if(rename(tmpQIFName, (char*)pThis->pszQIFNam) != 0) {
			rs_strerror_r(errno, errStr, sizeof(errStr));
			DBGOPRINT((obj_t*) pThis,
				"FATAL error: renaming temporary .qi file failed: %s\n",
				errStr);
			ABORT_FINALIZE(RS_RET_RENAME_TMP_QI_ERROR);
		}
	}
	
	/* tell the input file object that it must not delete the file on close if the queue
	 * is non-empty - but only if we are not during a simple checkpoint
	 */
	if(bIsCheckpoint != QUEUE_CHECKPOINT
	   && pThis->tVars.disk.pReadDel != NULL) {
		CHKiRet(strm.SetbDeleteOnClose(pThis->tVars.disk.pReadDel, 0));
	}

	/* we have persisted the queue object. So whenever it comes to an empty queue,
	 * we need to delete the QIF. Thus, we indicte that need.
	 */
	pThis->bNeedDelQIF = 1;

finalize_it:
	if(tmpQIFName != (char*)pThis->pszQIFNam) /* pointer, not string comparison! */
		free(tmpQIFName);
	if(psQIF != NULL)
		strm.Destruct(&psQIF);

	RETiRet;
}


/* check if we need to persist the current queue info. If an
 * error occurs, this should be ignored by caller (but we still
 * abide to our regular call interface)...
 * rgerhards, 2008-01-13
 * nUpdates is the number of updates since the last call to this function.
 * It may be > 1 due to batches. -- rgerhards, 2009-05-12
 */
static rsRetVal qqueueChkPersist(qqueue_t *const pThis, const int nUpdates)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, qqueue);
	assert(nUpdates >= 0);

	if(nUpdates == 0)
		FINALIZE;

	pThis->iUpdsSincePersist += nUpdates;
	if(pThis->iPersistUpdCnt && pThis->iUpdsSincePersist >= pThis->iPersistUpdCnt) {
		qqueuePersist(pThis, QUEUE_CHECKPOINT);
		pThis->iUpdsSincePersist = 0;
	}

finalize_it:
	RETiRet;
}


/* persist a queue with all data elements to disk - this is used to handle
 * bSaveOnShutdown. We utilize the DA worker to do this. This must only
 * be called after all workers have been shut down and if bSaveOnShutdown
 * is actually set. Note that this function may potentially run long,
 * depending on the queue configuration (e.g. store on remote machine).
 * rgerhards, 2009-05-26
 */
static rsRetVal
DoSaveOnShutdown(qqueue_t *pThis)
{
	struct timespec tTimeout;
	rsRetVal iRetLocal;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);

	/* we reduce the low water mark, otherwise the DA worker would terminate when
	 * it is reached.
	 */
	DBGOPRINT((obj_t*) pThis, "bSaveOnShutdown set, restarting DA worker...\n");
	pThis->bShutdownImmediate = 0; /* would termiante the DA worker! */
	pThis->iLowWtrMrk = 0;
	wtpSetState(pThis->pWtpDA, wtpState_SHUTDOWN);	/* shutdown worker (only) when done (was _IMMEDIATE!) */
	wtpAdviseMaxWorkers(pThis->pWtpDA, 1);		/* restart DA worker */

	DBGOPRINT((obj_t*) pThis, "waiting for DA worker to terminate...\n");
	timeoutComp(&tTimeout, QUEUE_TIMEOUT_ETERNAL);
	/* and run the primary queue's DA worker to drain the queue */
	iRetLocal = wtpShutdownAll(pThis->pWtpDA, wtpState_SHUTDOWN, &tTimeout);
	DBGOPRINT((obj_t*) pThis, "end queue persistence run, iRet %d, queue size log %d, phys %d\n",
		  iRetLocal, getLogicalQueueSize(pThis), getPhysicalQueueSize(pThis));
	if(iRetLocal != RS_RET_OK) {
		DBGOPRINT((obj_t*) pThis, "unexpected iRet state %d after trying to shut down primary "
			"queue in disk save mode, continuing, but results are unpredictable\n", iRetLocal);
	}

	RETiRet;
}


/* destructor for the queue object */
BEGINobjDestruct(qqueue) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(qqueue)
	DBGOPRINT((obj_t*) pThis, "shutdown: begin to destruct queue\n");
	if(pThis->bQueueStarted) {
		/* shut down all workers
		 * We do not need to shutdown workers when we are in enqueue-only mode or we are a
		 * direct queue - because in both cases we have none... ;)
		 * with a child! -- rgerhards, 2008-01-28
		 */
		if(pThis->qType != QUEUETYPE_DIRECT && !pThis->bEnqOnly && pThis->pqParent == NULL
		   && pThis->pWtpReg != NULL)
			qqueueShutdownWorkers(pThis);

		if(pThis->bIsDA && getPhysicalQueueSize(pThis) > 0 && pThis->bSaveOnShutdown) {
			CHKiRet(DoSaveOnShutdown(pThis));
		}

		/* finally destruct our (regular) worker thread pool
		 * Note: currently pWtpReg is never NULL, but if we optimize our logic, this may happen,
		 * e.g. when they are not created in enqueue-only mode. We already check the condition
		 * as this may otherwise be very hard to find once we optimize (and have long forgotten
		 * about this condition here ;)
		 * rgerhards, 2008-01-25
		 */
		if(pThis->qType != QUEUETYPE_DIRECT && pThis->pWtpReg != NULL) {
			wtpDestruct(&pThis->pWtpReg);
		}

		/* Now check if we actually have a DA queue and, if so, destruct it.
		 * Note that the wtp must be destructed first, it may be in cancel cleanup handler
		 * *right now* and actually *need* to access the queue object to persist some final
		 * data (re-queueing case). So we need to destruct the wtp first, which will make
		 * sure all workers have terminated. Please note that this also generates a situation
		 * where it is possible that the DA queue has a parent pointer but the parent has
		 * no WtpDA associated with it - which is perfectly legal thanks to this code here.
		 */
		if(pThis->pWtpDA != NULL) {
			wtpDestruct(&pThis->pWtpDA);
		}
		if(pThis->pqDA != NULL) {
			qqueueDestruct(&pThis->pqDA);
		}

		/* persist the queue (we always do that - queuePersits() does cleanup if the queue is empty)
		 * This handler is most important for disk queues, it will finally persist the necessary
		 * on-disk structures. In theory, other queueing modes may implement their other (non-DA)
		 * methods of persisting a queue between runs, but in practice all of this is done via
		 * disk queues and DA mode. Anyhow, it doesn't hurt to know that we could extend it here
		 * if need arises (what I doubt...) -- rgerhards, 2008-01-25
		 */
		CHKiRet_Hdlr(qqueuePersist(pThis, QUEUE_NO_CHECKPOINT)) {
			DBGOPRINT((obj_t*) pThis, "error %d persisting queue - data lost!\n", iRet);
		}

		/* finally, clean up some simple things... */
		if(pThis->pqParent == NULL) {
			/* if we are not a child, we allocated our own mutex, which we now need to destroy */
			pthread_mutex_destroy(pThis->mut);
			free(pThis->mut);
		}
		pthread_mutex_destroy(&pThis->mutThrdMgmt);
		pthread_cond_destroy(&pThis->notFull);
		pthread_cond_destroy(&pThis->belowFullDlyWtrMrk);
		pthread_cond_destroy(&pThis->belowLightDlyWtrMrk);

		DESTROY_ATOMIC_HELPER_MUT(pThis->mutQueueSize);
		DESTROY_ATOMIC_HELPER_MUT(pThis->mutLogDeq);

		/* type-specific destructor */
		iRet = pThis->qDestruct(pThis);
	}

	free(pThis->pszFilePrefix);
	free(pThis->pszSpoolDir);
	if(pThis->useCryprov) {
		pThis->cryprov.Destruct(&pThis->cryprovData);
		obj.ReleaseObj(__FILE__, pThis->cryprovNameFull+2, pThis->cryprovNameFull,
			       (void*) &pThis->cryprov);
		free(pThis->cryprovName);
		free(pThis->cryprovNameFull);
	}

	/* some queues do not provide stats and thus have no statsobj! */
	if(pThis->statsobj != NULL)
		statsobj.Destruct(&pThis->statsobj);
ENDobjDestruct(qqueue)


/* set the queue's spool directory. The directory MUST NOT be NULL.
 * The passed-in string is duplicated. So if the caller does not need
 * it any longer, it must free it.
 */
rsRetVal
qqueueSetSpoolDir(qqueue_t *pThis, uchar *pszSpoolDir, int lenSpoolDir)
{
	DEFiRet;

	free(pThis->pszSpoolDir);
	CHKmalloc(pThis->pszSpoolDir = ustrdup(pszSpoolDir));
	pThis->lenSpoolDir = lenSpoolDir;

finalize_it:
	RETiRet;
}


/* set the queue's file prefix
 * The passed-in string is duplicated. So if the caller does not need
 * it any longer, it must free it.
 * rgerhards, 2008-01-09
 */
rsRetVal
qqueueSetFilePrefix(qqueue_t *pThis, uchar *pszPrefix, size_t iLenPrefix)
{
	DEFiRet;

	free(pThis->pszFilePrefix);
	pThis->pszFilePrefix = NULL;

	if(pszPrefix == NULL) /* just unset the prefix! */
		ABORT_FINALIZE(RS_RET_OK);

	if((pThis->pszFilePrefix = MALLOC(iLenPrefix + 1)) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	memcpy(pThis->pszFilePrefix, pszPrefix, iLenPrefix + 1);
	pThis->lenFilePrefix = iLenPrefix;

finalize_it:
	RETiRet;
}

/* set the queue's maximum file size
 * rgerhards, 2008-01-09
 */
rsRetVal
qqueueSetMaxFileSize(qqueue_t *pThis, size_t iMaxFileSize)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	
	if(iMaxFileSize < 1024) {
		ABORT_FINALIZE(RS_RET_VALUE_TOO_LOW);
	}

	pThis->iMaxFileSize = iMaxFileSize;

finalize_it:
	RETiRet;
}


/* enqueue a single data object.
 * Note that the queue mutex MUST already be locked when this function is called.
 * rgerhards, 2009-06-16
 */
static rsRetVal
doEnqSingleObj(qqueue_t *pThis, flowControl_t flowCtlType, smsg_t *pMsg)
{
	DEFiRet;
	int err;
	struct timespec t;

	STATSCOUNTER_INC(pThis->ctrEnqueued, pThis->mutCtrEnqueued);
	/* first check if we need to discard this message (which will cause CHKiRet() to exit)
	 */
	CHKiRet(qqueueChkDiscardMsg(pThis, pThis->iQueueSize, pMsg));

	/* handle flow control
	 * There are two different flow control mechanisms: basic and advanced flow control.
	 * Basic flow control has always been implemented and protects the queue structures
	 * in that it makes sure no more data is enqueued than the queue is configured to
	 * support. Enhanced flow control is being added today. There are some sources which
	 * can easily be stopped, e.g. a file reader. This is the case because it is unlikely
	 * that blocking those sources will have negative effects (after all, the file is
	 * continued to be written). Other sources can somewhat be blocked (e.g. the kernel
	 * log reader or the local log stream reader): in general, nothing is lost if messages
	 * from these sources are not picked up immediately. HOWEVER, they can not block for
	 * an extended period of time, as this either causes message loss or - even worse - some
	 * other bad effects (e.g. unresponsive system in respect to the main system log socket).
	 * Finally, there are some (few) sources which can not be blocked at all. UDP syslog is
	 * a prime example. If a UDP message is not received, it is simply lost. So we can't
	 * do anything against UDP sockets that come in too fast. The core idea of advanced
	 * flow control is that we take into account the different natures of the sources and
	 * select flow control mechanisms that fit these needs. This also means, in the end
	 * result, that non-blockable sources like UDP syslog receive priority in the system.
	 * It's a side effect, but a good one ;) -- rgerhards, 2008-03-14
	 */
	if(flowCtlType == eFLOWCTL_FULL_DELAY) {
		while(pThis->iQueueSize >= pThis->iFullDlyMrk&& ! glbl.GetGlobalInputTermState()) {
			/* We have a problem during shutdown if we block eternally. In that
			 * case, the the input thread cannot be terminated. So we wake up
			 * from time to time to check for termination.
			 * TODO/v6(at earliest): check if we could signal the condition during
			 * shutdown. However, this requires new queue registries and thus is
			 * far to much change for a stable version (and I am still not sure it
			 * is worth the effort, given how seldom this situation occurs and how
			 * few resources the wakeups need). -- rgerhards, 2012-05-03
			 * In any case, this was the old code (if we do the TODO):
			 * pthread_cond_wait(&pThis->belowFullDlyWtrMrk, pThis->mut);
			 */
			DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: FullDelay mark reached for full "
				"delayable message - blocking, queue size is %d.\n", pThis->iQueueSize);
			timeoutComp(&t, 1000);
			err = pthread_cond_timedwait(&pThis->belowLightDlyWtrMrk, pThis->mut, &t);
			if(err != 0 && err != ETIMEDOUT) {
				/* Something is really wrong now. Report to debug log and abort the
				 * wait. That keeps us running, even though we may lose messages.
				 */
				DBGOPRINT((obj_t*) pThis, "potential program bug: pthread_cond_timedwait()"
				          "/fulldelay returned %d\n", err);
				break;

			}
			DBGPRINTF("wti worker in full delay timed out, checking termination...\n");
		}
	} else if(flowCtlType == eFLOWCTL_LIGHT_DELAY && !glbl.GetGlobalInputTermState()) {
		if(pThis->iQueueSize >= pThis->iLightDlyMrk) {
			DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: LightDelay mark reached for light "
			          "delayable message - blocking a bit.\n");
			timeoutComp(&t, 1000); /* 1000 millisconds = 1 second TODO: make configurable */
			err = pthread_cond_timedwait(&pThis->belowLightDlyWtrMrk, pThis->mut, &t);
			if(err != 0 && err != ETIMEDOUT) {
				/* Something is really wrong now. Report to debug log */
				DBGOPRINT((obj_t*) pThis, "potential program bug: pthread_cond_timedwait()"
				          "/lightdelay returned %d\n", err);

			}
		}
	}

	/* from our regular flow control settings, we are now ready to enqueue the object.
	 * However, we now need to do a check if the queue permits to add more data. If that
	 * is not the case, basic flow control enters the field, which means we wait for
	 * the queue to become ready or drop the new message. -- rgerhards, 2008-03-14
	 */
	while(   (pThis->iMaxQueueSize > 0 && pThis->iQueueSize >= pThis->iMaxQueueSize)
	      || ((pThis->qType == QUEUETYPE_DISK || pThis->bIsDA) && pThis->sizeOnDiskMax != 0
	      	  && pThis->tVars.disk.sizeOnDisk > pThis->sizeOnDiskMax)) {
		STATSCOUNTER_INC(pThis->ctrFull, pThis->mutCtrFull);
		if(pThis->toEnq == 0 || pThis->bEnqOnly) {
			DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: queue FULL - configured for immediate "
					"discarding QueueSize=%d MaxQueueSize=%d sizeOnDisk=%lld "
					"sizeOnDiskMax=%lld\n", pThis->iQueueSize, pThis->iMaxQueueSize,
					pThis->tVars.disk.sizeOnDisk, pThis->sizeOnDiskMax);
			STATSCOUNTER_INC(pThis->ctrFDscrd, pThis->mutCtrFDscrd);
			msgDestruct(&pMsg);
			ABORT_FINALIZE(RS_RET_QUEUE_FULL);
		} else {
			DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: queue FULL - waiting %dms to drain.\n",
				pThis->toEnq);
			if(glbl.GetGlobalInputTermState()) {
				DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: queue FULL, discard due to "
					"FORCE_TERM.\n");
				ABORT_FINALIZE(RS_RET_FORCE_TERM);
			}
			timeoutComp(&t, pThis->toEnq);
			const int r = pthread_cond_timedwait(&pThis->notFull, pThis->mut, &t);
			if(dbgTimeoutToStderr && r != 0) {
				fprintf(stderr, "%lld: queue timeout(%dms), error %d%s, "
					"lost message %s\n", (long long) time(NULL), pThis->toEnq,
					r, ( r == ETIMEDOUT) ? "[ETIMEDOUT]" : "", pMsg->pszRawMsg);
			}
			if(r == ETIMEDOUT) {
				DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: cond timeout, dropping message!\n");
				STATSCOUNTER_INC(pThis->ctrFDscrd, pThis->mutCtrFDscrd);
				msgDestruct(&pMsg);
				ABORT_FINALIZE(RS_RET_QUEUE_FULL);
			} else if(r != 0) {
				DBGOPRINT((obj_t*) pThis, "doEnqSingleObject: cond error %d, dropping message!\n", r);
				STATSCOUNTER_INC(pThis->ctrFDscrd, pThis->mutCtrFDscrd);
				msgDestruct(&pMsg);
				ABORT_FINALIZE(RS_RET_QUEUE_FULL);
			}
		dbgoprint((obj_t*) pThis, "doEnqSingleObject: wait solved queue full condition, enqueing\n");
		}
	}

	/* and finally enqueue the message */
	CHKiRet(qqueueAdd(pThis, pMsg));
	STATSCOUNTER_SETMAX_NOMUT(pThis->ctrMaxqsize, pThis->iQueueSize);

	/* check if we had a file rollover and need to persist
	 * the .qi file for robustness reasons.
	 * Note: the n=2 write is required for closing the old file and
	 * the n=1 write is required after opening and writing to the new
	 * file.
	 */
	if(pThis->tVars.disk.nForcePersist > 0) {
		DBGOPRINT((obj_t*) pThis, ".qi file write required for robustness reasons (n=%d)\n",
			pThis->tVars.disk.nForcePersist);
		pThis->tVars.disk.nForcePersist--;
		qqueuePersist(pThis, QUEUE_CHECKPOINT);
	}

finalize_it:
	RETiRet;
}

/* ------------------------------ multi-enqueue functions ------------------------------ */
/* enqueue multiple user data elements at once. The aim is to provide a faster interface
 * for object submission. Uses the multi_submit_t helper object.
 * Please note that this function is not cancel-safe and consequently
 * sets the calling thread's cancelibility state to PTHREAD_CANCEL_DISABLE
 * during its execution. If that is not done, race conditions occur if the
 * thread is canceled (most important use case is input module termination).
 * rgerhards, 2009-06-16
 * Note: there now exists multiple different functions implementing specially
 * optimized algorithms for different config cases. -- rgerhards, 2010-06-09
 */
/* now the function for all modes but direct */
static rsRetVal
qqueueMultiEnqObjNonDirect(qqueue_t *pThis, multi_submit_t *pMultiSub)
{
	int iCancelStateSave;
	int i;
	rsRetVal localRet;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	assert(pMultiSub != NULL);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &iCancelStateSave);
	d_pthread_mutex_lock(pThis->mut);
	for(i = 0 ; i < pMultiSub->nElem ; ++i) {
		localRet = doEnqSingleObj(pThis, pMultiSub->ppMsgs[i]->flowCtlType, (void*)pMultiSub->ppMsgs[i]);
		if(localRet != RS_RET_OK && localRet != RS_RET_QUEUE_FULL)
			ABORT_FINALIZE(localRet);
	}
	qqueueChkPersist(pThis, pMultiSub->nElem);

finalize_it:
	/* make sure at least one worker is running. */
	qqueueAdviseMaxWorkers(pThis);
	/* and release the mutex */
	d_pthread_mutex_unlock(pThis->mut);
	pthread_setcancelstate(iCancelStateSave, NULL);
	DBGOPRINT((obj_t*) pThis, "MultiEnqObj advised worker start\n");

	RETiRet;
}

/* now, the same function, but for direct mode */
static rsRetVal
qqueueMultiEnqObjDirect(qqueue_t *pThis, multi_submit_t *pMultiSub)
{
	int i;
	wti_t *pWti;
	DEFiRet;

	pWti = wtiGetDummy();
	pWti->pbShutdownImmediate = &pThis->bShutdownImmediate;

	for(i = 0 ; i < pMultiSub->nElem ; ++i) {
		CHKiRet(qAddDirectWithWti(pThis, (void*)pMultiSub->ppMsgs[i], pWti));
	}

finalize_it:
	RETiRet;
}
/* ------------------------------ END multi-enqueue functions ------------------------------ */


/* enqueue a new user data element
 * Enqueues the new element and awakes worker thread.
 */
rsRetVal
qqueueEnqMsg(qqueue_t *pThis, flowControl_t flowCtlType, smsg_t *pMsg)
{
	DEFiRet;
	int iCancelStateSave;
	ISOBJ_TYPE_assert(pThis, qqueue);

	const int isNonDirectQ = pThis->qType != QUEUETYPE_DIRECT;

	if(isNonDirectQ) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &iCancelStateSave);
		d_pthread_mutex_lock(pThis->mut);
	}

	CHKiRet(doEnqSingleObj(pThis, flowCtlType, pMsg));

	qqueueChkPersist(pThis, 1);

finalize_it:
	if(isNonDirectQ) {
		/* make sure at least one worker is running. */
		qqueueAdviseMaxWorkers(pThis);
		/* and release the mutex */
		d_pthread_mutex_unlock(pThis->mut);
		pthread_setcancelstate(iCancelStateSave, NULL);
		DBGOPRINT((obj_t*) pThis, "EnqueueMsg advised worker start\n");
	}

	RETiRet;
}


/* are any queue params set at all? 1 - yes, 0 - no
 * We need to evaluate the param block for this function, which is somewhat
 * inefficient. HOWEVER, this is only done during config load, so we really
 * don't care... -- rgerhards, 2013-05-10
 */
int
queueCnfParamsSet(struct nvlst *lst)
{
	int r;
	struct cnfparamvals *pvals;

	pvals = nvlstGetParams(lst, &pblk, NULL);
	r = cnfparamvalsIsSet(&pblk, pvals);
	cnfparamvalsDestruct(pvals, &pblk);
	return r;
}


static rsRetVal
initCryprov(qqueue_t *pThis, struct nvlst *lst)
{
	uchar szDrvrName[1024];
	DEFiRet;

	if(snprintf((char*)szDrvrName, sizeof(szDrvrName), "lmcry_%s", pThis->cryprovName)
		== sizeof(szDrvrName)) {
		LogError(0, RS_RET_ERR, "queue: crypto provider "
				"name is too long: '%s' - encryption disabled",
				pThis->cryprovName);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	pThis->cryprovNameFull = ustrdup(szDrvrName);

	pThis->cryprov.ifVersion = cryprovCURR_IF_VERSION;
	/* The pDrvrName+2 below is a hack to obtain the object name. It
	 * safes us to have yet another variable with the name without "lm" in
	 * front of it. If we change the module load interface, we may re-think
	 * about this hack, but for the time being it is efficient and clean enough.
	 */
	if(obj.UseObj(__FILE__, szDrvrName, szDrvrName, (void*) &pThis->cryprov)
		!= RS_RET_OK) {
		LogError(0, RS_RET_LOAD_ERROR, "queue: could not load "
				"crypto provider '%s' - encryption disabled",
				szDrvrName);
		ABORT_FINALIZE(RS_RET_CRYPROV_ERR);
	}

	if(pThis->cryprov.Construct(&pThis->cryprovData) != RS_RET_OK) {
		LogError(0, RS_RET_CRYPROV_ERR, "queue: error constructing "
				"crypto provider %s dataset - encryption disabled",
				szDrvrName);
		ABORT_FINALIZE(RS_RET_CRYPROV_ERR);
	}
	CHKiRet(pThis->cryprov.SetCnfParam(pThis->cryprovData, lst, CRYPROV_PARAMTYPE_DISK));

	dbgprintf("loaded crypto provider %s, data instance at %p\n",
		  szDrvrName, pThis->cryprovData);
	pThis->useCryprov = 1;
finalize_it:
	RETiRet;
}

/* apply all params from param block to queue. Must be called before
 * finalizing. This supports the v6 config system. Defaults were already
 * set during queue creation. The pvals object is destructed by this
 * function.
 */
rsRetVal
qqueueApplyCnfParam(qqueue_t *pThis, struct nvlst *lst)
{
	int i;
	struct cnfparamvals *pvals;
	DEFiRet;

	pvals = nvlstGetParams(lst, &pblk, NULL);
	if(pvals == NULL) {
		parser_errmsg("error processing queue config parameters");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}
	if(Debug) {
		dbgprintf("queue param blk:\n");
		cnfparamsPrint(&pblk, pvals);
	}
	for(i = 0 ; i < pblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(pblk.descr[i].name, "queue.filename")) {
			pThis->pszFilePrefix = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
			pThis->lenFilePrefix = es_strlen(pvals[i].val.d.estr);
		} else if(!strcmp(pblk.descr[i].name, "queue.cry.provider")) {
			pThis->cryprovName = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(pblk.descr[i].name, "queue.spooldirectory")) {
			free(pThis->pszSpoolDir);
			pThis->pszSpoolDir = (uchar*) es_str2cstr(pvals[i].val.d.estr, NULL);
			pThis->lenSpoolDir = es_strlen(pvals[i].val.d.estr);
			if(pThis->pszSpoolDir[pThis->lenSpoolDir-1] == '/') {
				pThis->pszSpoolDir[pThis->lenSpoolDir-1] = '\0';
				--pThis->lenSpoolDir;
				parser_errmsg("queue.spooldirectory must not end with '/', "
					      "corrected to '%s'", pThis->pszSpoolDir);
			}
		} else if(!strcmp(pblk.descr[i].name, "queue.size")) {
			pThis->iMaxQueueSize = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.dequeuebatchsize")) {
			pThis->iDeqBatchSize = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.maxdiskspace")) {
			pThis->sizeOnDiskMax = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.highwatermark")) {
			pThis->iHighWtrMrk = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.lowwatermark")) {
			pThis->iLowWtrMrk = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.fulldelaymark")) {
			pThis->iFullDlyMrk = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.lightdelaymark")) {
			pThis->iLightDlyMrk = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.discardmark")) {
			pThis->iDiscardMrk = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.discardseverity")) {
			pThis->iDiscardSeverity = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.checkpointinterval")) {
			pThis->iPersistUpdCnt = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.syncqueuefiles")) {
			pThis->bSyncQueueFiles = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.type")) {
			pThis->qType = (queueType_t) pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.workerthreads")) {
			pThis->iNumWorkerThreads = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.timeoutshutdown")) {
			pThis->toQShutdown = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.timeoutactioncompletion")) {
			pThis->toActShutdown = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.timeoutenqueue")) {
			pThis->toEnq = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.timeoutworkerthreadshutdown")) {
			pThis->toWrkShutdown = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.workerthreadminimummessages")) {
			pThis->iMinMsgsPerWrkr = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.maxfilesize")) {
			pThis->iMaxFileSize = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.saveonshutdown")) {
			pThis->bSaveOnShutdown = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.dequeueslowdown")) {
			pThis->iDeqSlowdown = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.dequeuetimebegin")) {
			pThis->iDeqtWinFromHr = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.dequeuetimeend")) {
			pThis->iDeqtWinToHr = pvals[i].val.d.n;
		} else if(!strcmp(pblk.descr[i].name, "queue.samplinginterval")) {
			pThis->iSmpInterval = pvals[i].val.d.n;
		} else {
			DBGPRINTF("queue: program error, non-handled "
			  "param '%s'\n", pblk.descr[i].name);
		}
	}
	if(pThis->qType == QUEUETYPE_DISK) {
		if(pThis->pszFilePrefix == NULL) {
			LogError(0, RS_RET_QUEUE_DISK_NO_FN, "error on queue '%s', disk mode selected, but "
					"no queue file name given; queue type changed to 'linkedList'",
					obj.GetName((obj_t*) pThis));
			pThis->qType = QUEUETYPE_LINKEDLIST;
		}
	}

	if(pThis->pszFilePrefix == NULL && pThis->cryprovName != NULL) {
		LogError(0, RS_RET_QUEUE_CRY_DISK_ONLY, "error on queue '%s', crypto provider can "
				"only be set for disk or disk assisted queue - ignored",
				obj.GetName((obj_t*) pThis));
		free(pThis->cryprovName);
		pThis->cryprovName = NULL;
	}

	if(pThis->cryprovName != NULL) {
		initCryprov(pThis, lst);
	}

	cnfparamvalsDestruct(pvals, &pblk);
finalize_it:
	RETiRet;
}


/* some simple object access methods */
DEFpropSetMeth(qqueue, bSyncQueueFiles, int)
DEFpropSetMeth(qqueue, iPersistUpdCnt, int)
DEFpropSetMeth(qqueue, iDeqtWinFromHr, int)
DEFpropSetMeth(qqueue, iDeqtWinToHr, int)
DEFpropSetMeth(qqueue, toQShutdown, long)
DEFpropSetMeth(qqueue, toActShutdown, long)
DEFpropSetMeth(qqueue, toWrkShutdown, long)
DEFpropSetMeth(qqueue, toEnq, long)
DEFpropSetMeth(qqueue, iHighWtrMrk, int)
DEFpropSetMeth(qqueue, iLowWtrMrk, int)
DEFpropSetMeth(qqueue, iDiscardMrk, int)
DEFpropSetMeth(qqueue, iDiscardSeverity, int)
DEFpropSetMeth(qqueue, iLightDlyMrk, int)
DEFpropSetMeth(qqueue, iNumWorkerThreads, int)
DEFpropSetMeth(qqueue, iMinMsgsPerWrkr, int)
DEFpropSetMeth(qqueue, bSaveOnShutdown, int)
DEFpropSetMeth(qqueue, pAction, action_t*)
DEFpropSetMeth(qqueue, iDeqSlowdown, int)
DEFpropSetMeth(qqueue, iDeqBatchSize, int)
DEFpropSetMeth(qqueue, sizeOnDiskMax, int64)
DEFpropSetMeth(qqueue, iSmpInterval, int)


/* This function can be used as a generic way to set properties. Only the subset
 * of properties required to read persisted property bags is supported. This
 * functions shall only be called by the property bag reader, thus it is static.
 * rgerhards, 2008-01-11
 */
#define isProp(name) !rsCStrSzStrCmp(pProp->pcsName, (uchar*) name, sizeof(name) - 1)
static rsRetVal qqueueSetProperty(qqueue_t *pThis, var_t *pProp)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, qqueue);
	ASSERT(pProp != NULL);

	if(isProp("iQueueSize")) {
		pThis->iQueueSize = pProp->val.num;
#		ifdef ENABLE_IMDIAG
			iOverallQueueSize += pThis->iQueueSize;
#		endif
	} else if(isProp("tVars.disk.sizeOnDisk")) {
		pThis->tVars.disk.sizeOnDisk = pProp->val.num;
	} else if(isProp("qType")) {
		if(pThis->qType != pProp->val.num)
			ABORT_FINALIZE(RS_RET_QTYPE_MISMATCH);
	}

finalize_it:
	RETiRet;
}
#undef	isProp

/* dummy */
static rsRetVal qqueueQueryInterface(interface_t __attribute__((unused)) *i) { return RS_RET_NOT_IMPLEMENTED; }

/* Initialize the stream class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-01-09
 */
BEGINObjClassInit(qqueue, 1, OBJ_IS_CORE_MODULE)
	/* request objects we use */
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(strm, CORE_COMPONENT));
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(statsobj, CORE_COMPONENT));

	/* now set our own handlers */
	OBJSetMethodHandler(objMethod_SETPROPERTY, qqueueSetProperty);
ENDObjClassInit(qqueue)
