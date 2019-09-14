/* Definition of the queue support module.
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
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

#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include <pthread.h>
#include "obj.h"
#include "wtp.h"
#include "batch.h"
#include "stream.h"
#include "statsobj.h"
#include "cryprov.h"

/* support for the toDelete list */
typedef struct toDeleteLst_s toDeleteLst_t;
struct toDeleteLst_s {
	qDeqID	deqID;
	int	nElemDeq;	/* numbe of elements that were dequeued and as such must now be discarded */
	struct toDeleteLst_s *pNext;
};


/* queue types */
typedef enum {
	QUEUETYPE_FIXED_ARRAY = 0,/* a simple queue made out of a fixed (initially malloced) array fast but memoryhog */
	QUEUETYPE_LINKEDLIST = 1, /* linked list used as buffer, lower fixed memory overhead but slower */
	QUEUETYPE_DISK = 2, 	  /* disk files used as buffer */
	QUEUETYPE_DIRECT = 3 	  /* no queuing happens, consumer is directly called */
} queueType_t;

/* list member definition for linked list types of queues: */
typedef struct qLinkedList_S {
	struct qLinkedList_S *pNext;
	smsg_t *pMsg;
} qLinkedList_t;


/* the queue object */
struct queue_s {
	BEGINobjInstance;
	queueType_t	qType;
	int	nLogDeq;	/* number of elements currently logically dequeued */
	int	bShutdownImmediate; /* should all workers cease processing messages? */
	sbool	bEnqOnly;	/* does queue run in enqueue-only mode (1) or not (0)? */
	sbool	bSaveOnShutdown;/* persists everthing on shutdown (if DA!)? 1-yes, 0-no */
	sbool	bQueueStarted;	/* has queueStart() been called on this queue? 1-yes, 0-no */
	int	iQueueSize;	/* Current number of elements in the queue */
	int	iMaxQueueSize;	/* how large can the queue grow? */
	int 	iNumWorkerThreads;/* number of worker threads to use */
	int 	iCurNumWrkThrd;/* current number of active worker threads */
	int	iMinMsgsPerWrkr;
	/* minimum nbr of msgs per worker thread, if more, a new worker is started until max wrkrs */
	wtp_t	*pWtpDA;
	wtp_t	*pWtpReg;
	action_t *pAction;	/* for action queues, ptr to action object; for main queues unused */
	int	iUpdsSincePersist;/* nbr of queue updates since the last persist call */
	int	iPersistUpdCnt;	/* persits queue info after this nbr of updates - 0 -> persist only on shutdown */
	sbool	bSyncQueueFiles;/* if working with files, sync them after each write? */
	int	iHighWtrMrk;	/* high water mark for disk-assisted memory queues */
	int	iLowWtrMrk;	/* low water mark for disk-assisted memory queues */
	int	iDiscardMrk;	/* if the queue is above this mark, low-severity messages are discarded */
	int	iFullDlyMrk;	/* if the queue is above this mark, FULL_DELAYable message are put on hold */
	int	iLightDlyMrk;	/* if the queue is above this mark, LIGHT_DELAYable message are put on hold */
	int	iDiscardSeverity;/* messages of this severity above are discarded on too-full queue */
	sbool	bNeedDelQIF;	/* does the QIF file need to be deleted when queue becomes empty? */
	int	toQShutdown;	/* timeout for regular queue shutdown in ms */
	int	toActShutdown;	/* timeout for long-running action shutdown in ms */
	int	toWrkShutdown;	/* timeout for idle workers in ms, -1 means indefinite (0 is immediate) */
	toDeleteLst_t *toDeleteLst;/* this queue's to-delete list */
	int	toEnq;		/* enqueue timeout */
	int	iDeqBatchSize;	/* max number of elements that shall be dequeued at once */
	/* rate limiting settings (will be expanded) */
	int	iDeqSlowdown; /* slow down dequeue by specified nbr of microseconds */
	/* end rate limiting */
	/* dequeue time window settings (may also be expanded) */
	int iDeqtWinFromHr;	/* begin of dequeue time window (hour only) */
	int iDeqtWinToHr;	/* end of dequeue time window (hour only), set to 25 to disable deq window! */
	/* note that begin and end have specific semantics. It is a big difference if we have
	 * begin 4, end 22 or begin 22, end 4. In the later case, dequeuing will run from 10p,
	 * throughout the night and stop at 4 in the morning. In the first case, it will start
	 * at 4am, run throughout the day, and stop at 10 in the evening! So far, not logic is
	 * applied to detect user configuration errors (and tell me how should we detect what
	 * the user really wanted...). -- rgerhards, 2008-04-02
	 */
	/* end dequeue time window */
	rsRetVal (*pConsumer)(void *,batch_t*, wti_t*); /* user-supplied consumer function for dequeued messages */
	/* calling interface for pConsumer: arg1 is the global user pointer from this structure, arg2 is the
	 * user pointer array that was dequeued (actual sample: for actions, arg1 is the pAction and arg2
	 * is pointer to an array of message message pointers)
	 */
	/* type-specific handlers (set during construction) */
	rsRetVal (*qConstruct)(struct queue_s *pThis);
	rsRetVal (*qDestruct)(struct queue_s *pThis);
	rsRetVal (*qAdd)(struct queue_s *pThis, smsg_t *pMsg);
	rsRetVal (*qDeq)(struct queue_s *pThis, smsg_t **ppMsg);
	rsRetVal (*qDel)(struct queue_s *pThis);
	/* end type-specific handler */
	/* public entry points (set during construction, permit to set best algorithm for params selected) */
	rsRetVal (*MultiEnq)(qqueue_t *pThis, multi_submit_t *pMultiSub);
	/* end public entry points */
	/* synchronization variables */
	pthread_mutex_t mutThrdMgmt; /* mutex for the queue's thread management */
	pthread_mutex_t *mut; /* mutex for enqueing and dequeueing messages */
	pthread_cond_t notFull;
	pthread_cond_t belowFullDlyWtrMrk; /* below eFLOWCTL_FULL_DELAY watermark */
	pthread_cond_t belowLightDlyWtrMrk; /* below eFLOWCTL_FULL_DELAY watermark */
	int bThrdStateChanged;		/* at least one thread state has changed if 1 */
	/* end sync variables */
	/* the following variables are always present, because they
	 * are not only used for the "disk" queueing mode but also for
	 * any other queueing mode if it is set to "disk assisted".
	 * rgerhards, 2008-01-09
	 */
	uchar *pszSpoolDir;
	size_t lenSpoolDir;
	uchar *pszFilePrefix;
	size_t lenFilePrefix;
	uchar *pszQIFNam;	/* full .qi file name, based on parts above */
	size_t lenQIFNam;
	int iNumberFiles;	/* how many files make up the queue? */
	int64 iMaxFileSize;	/* max size for a single queue file */
	int64 sizeOnDiskMax;    /* maximum size on disk allowed */
	qDeqID deqIDAdd;	/* next dequeue ID to use during add to queue store */
	qDeqID deqIDDel;	/* queue store delete position */
	int bIsDA;		/* is this queue disk assisted? */
	struct queue_s *pqDA;	/* queue for disk-assisted modes */
	struct queue_s *pqParent;/* pointer to the parent (if this is a child queue) */
	int	bDAEnqOnly;	/* EnqOnly setting for DA queue */
	/* now follow queueing mode specific data elements */
	//union {			/* different data elements based on queue type (qType) */
	struct {			/* different data elements based on queue type (qType) */
		struct {
			long deqhead, head, tail;
			void** pBuf;		/* the queued user data structure */
		} farray;
		struct {
			qLinkedList_t *pDeqRoot;
			qLinkedList_t *pDelRoot;
			qLinkedList_t *pLast;
		} linklist;
		struct {
			int64 sizeOnDisk; /* current amount of disk space used */
			int64 deqOffs; /* offset after dequeue batch - used for file deleter */
			int deqFileNumIn; /* same for the circular file numbers, mainly for  */
			int deqFileNumOut;/* deleting finished files */
			strm_t *pWrite;   /* current file to be written */
			strm_t *pReadDeq; /* current file for dequeueing */
			strm_t *pReadDel; /* current file for deleting */
			int nForcePersist;/* force persist of .qi file the next "n" times */
		} disk;
	} tVars;
	sbool	useCryprov;	/* quicker than checkig ptr (1 vs 8 bytes!) */
	uchar *cryprovName; /* crypto provider to use */
	cryprov_if_t cryprov;	/* ptr to crypto provider interface */
	void *cryprovData; /* opaque data ptr for provider use */
	uchar 	*cryprovNameFull;/* full internal crypto provider name */
	DEF_ATOMIC_HELPER_MUT(mutQueueSize)
	DEF_ATOMIC_HELPER_MUT(mutLogDeq)
	/* for statistics subsystem */
	statsobj_t *statsobj;
	STATSCOUNTER_DEF(ctrEnqueued, mutCtrEnqueued)
	STATSCOUNTER_DEF(ctrFull, mutCtrFull)
	STATSCOUNTER_DEF(ctrFDscrd, mutCtrFDscrd)
	STATSCOUNTER_DEF(ctrNFDscrd, mutCtrNFDscrd)
	int ctrMaxqsize; /* NOT guarded by a mutex */
	int iSmpInterval; /* line interval of sampling logs */
};


/* the define below is an "eternal" timeout for the timeout settings which require a value.
 * It is one day, which is not really eternal, but comes close to it if we think about
 * rsyslog (e.g.: do you want to wait on shutdown for more than a day? ;))
 * rgerhards, 2008-01-17
 */
#define QUEUE_TIMEOUT_ETERNAL 24 * 60 * 60 * 1000

/* prototypes */
rsRetVal qqueueDestruct(qqueue_t **ppThis);
rsRetVal qqueueEnqMsg(qqueue_t *pThis, flowControl_t flwCtlType, smsg_t *pMsg);
rsRetVal qqueueStart(qqueue_t *pThis);
rsRetVal qqueueSetMaxFileSize(qqueue_t *pThis, size_t iMaxFileSize);
rsRetVal qqueueSetFilePrefix(qqueue_t *pThis, uchar *pszPrefix, size_t iLenPrefix);
rsRetVal qqueueConstruct(qqueue_t **ppThis, queueType_t qType, int iWorkerThreads,
		        int iMaxQueueSize, rsRetVal (*pConsumer)(void*,batch_t*, wti_t *));
int queueCnfParamsSet(struct nvlst *lst);
rsRetVal qqueueApplyCnfParam(qqueue_t *pThis, struct nvlst *lst);
void qqueueSetDefaultsRulesetQueue(qqueue_t *pThis);
void qqueueSetDefaultsActionQueue(qqueue_t *pThis);
void qqueueDbgPrint(qqueue_t *pThis);
rsRetVal qqueueShutdownWorkers(qqueue_t *pThis);

PROTOTYPEObjClassInit(qqueue);
PROTOTYPEpropSetMeth(qqueue, iPersistUpdCnt, int);
PROTOTYPEpropSetMeth(qqueue, bSyncQueueFiles, int);
PROTOTYPEpropSetMeth(qqueue, iDeqtWinFromHr, int);
PROTOTYPEpropSetMeth(qqueue, iDeqtWinToHr, int);
PROTOTYPEpropSetMeth(qqueue, toQShutdown, long);
PROTOTYPEpropSetMeth(qqueue, toActShutdown, long);
PROTOTYPEpropSetMeth(qqueue, toWrkShutdown, long);
PROTOTYPEpropSetMeth(qqueue, toEnq, long);
PROTOTYPEpropSetMeth(qqueue, iLightDlyMrk, int);
PROTOTYPEpropSetMeth(qqueue, iHighWtrMrk, int);
PROTOTYPEpropSetMeth(qqueue, iLowWtrMrk, int);
PROTOTYPEpropSetMeth(qqueue, iDiscardMrk, int);
PROTOTYPEpropSetMeth(qqueue, iDiscardSeverity, int);
PROTOTYPEpropSetMeth(qqueue, iMinMsgsPerWrkr, int);
PROTOTYPEpropSetMeth(qqueue, iNumWorkerThreads, int);
PROTOTYPEpropSetMeth(qqueue, bSaveOnShutdown, int);
PROTOTYPEpropSetMeth(qqueue, pAction, action_t*);
PROTOTYPEpropSetMeth(qqueue, iDeqSlowdown, int);
PROTOTYPEpropSetMeth(qqueue, sizeOnDiskMax, int64);
PROTOTYPEpropSetMeth(qqueue, iDeqBatchSize, int);
#define qqueueGetID(pThis) ((unsigned long) pThis)

#ifdef ENABLE_IMDIAG
extern unsigned int iOverallQueueSize;
#endif

#endif /* #ifndef QUEUE_H_INCLUDED */
