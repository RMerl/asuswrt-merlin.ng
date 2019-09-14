/* Definition of the worker thread instance (wti) class.
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

#ifndef WTI_H_INCLUDED
#define WTI_H_INCLUDED

#include <pthread.h>
#include <stdlib.h>
#include "wtp.h"
#include "obj.h"
#include "batch.h"
#include "action.h"


#define ACT_STATE_RDY  0	/* action ready, waiting for new transaction */
#define ACT_STATE_ITX  1	/* transaction active, waiting for new data or commit */
/* 2 currently not being used */
#define ACT_STATE_RTRY 3	/* failure occured, trying to restablish ready state */
#define ACT_STATE_SUSP 4	/* suspended due to failure (return fail until timeout expired) */
#define ACT_STATE_DATAFAIL 5	/* suspended due to failure in data, which means the message in
				   questions needs to be dropped as it will always fail. The
				   action must still do a "normal" retry in order to bring
				   it back to regular state. */
/* note: 3 bit bit field --> highest value is 7! */

typedef struct actWrkrInfo {
	action_t *pAction;
	void *actWrkrData;
	uint16_t uResumeOKinRow;/* number of times in a row that resume said OK with an
				   immediate failure following */
	int	iNbrResRtry;	/* number of retries since last suspend */
	sbool	bHadAutoCommit;	/* did an auto-commit happen during doAction()? */
	struct {
		unsigned actState : 3;
	} flags;
	union {
		struct {
			actWrkrIParams_t *iparams;/* dynamically sized array for transactional outputs */
			int currIParam;
			int maxIParams;	/* current max */
		} tx;
		struct {
			actWrkrIParams_t actParams[CONF_OMOD_NUMSTRINGS_MAXSIZE];
		} nontx;
	} p; /* short name for "parameters" */
} actWrkrInfo_t;

/* the worker thread instance class */
struct wti_s {
	BEGINobjInstance;
	pthread_t thrdID; 	/* thread ID */
	int bIsRunning;	/* is this thread currently running? (must be int for atomic op!) */
	sbool bAlwaysRunning;	/* should this thread always run? */
	int *pbShutdownImmediate;/* end processing of this batch immediately if set to 1 */
	wtp_t *pWtp; /* my worker thread pool (important if only the work thread instance is passed! */
	batch_t batch; /* pointer to an object array meaningful for current user
			  pointer (e.g. queue pUsr data elemt) */
	uchar *pszDbgHdr;	/* header string for debug messages */
	actWrkrInfo_t *actWrkrInfo; /* *array* of action wrkr infos for all actions
				      (sized for max nbr of actions in config!) */
	pthread_cond_t pcondBusy; /* condition to wake up the worker, protected by pmutUsr in wtp */
	DEF_ATOMIC_HELPER_MUT(mutIsRunning)
	struct {
		uint8_t	script_errno; /* errno-type interface for RainerScript functions */
		uint8_t bPrevWasSuspended;
		uint8_t bDoAutoCommit; /* do a commit after each message
		                        * this is usually set for batches with 0 element, but may
					* also be added as a user-selectable option (not implemented yet)
					*/
	} execState;	/* state for the execution engine */
};


/* prototypes */
rsRetVal wtiConstruct(wti_t **ppThis);
rsRetVal wtiConstructFinalize(wti_t * const pThis);
rsRetVal wtiDestruct(wti_t **ppThis);
rsRetVal wtiWorker(wti_t * const pThis);
rsRetVal wtiSetDbgHdr(wti_t * const pThis, uchar *pszMsg, size_t lenMsg);
rsRetVal wtiCancelThrd(wti_t * const pThis, const uchar *const cancelobj);
rsRetVal wtiSetAlwaysRunning(wti_t * const pThis);
rsRetVal wtiSetState(wti_t * const pThis, int bNew);
rsRetVal wtiWakeupThrd(wti_t * const pThis);
int wtiGetState(wti_t * const pThis);
wti_t *wtiGetDummy(void);
PROTOTYPEObjClassInit(wti);
PROTOTYPEObjClassExit(wti);
PROTOTYPEpropSetMeth(wti, pszDbgHdr, uchar*);
PROTOTYPEpropSetMeth(wti, pWtp, wtp_t*);

#define getActionStateByNbr(pWti, iActNbr) ((uint8_t) ((pWti)->actWrkrInfo[(iActNbr)].flags.actState))
#define getActionState(pWti, pAction) (((uint8_t) (pWti)->actWrkrInfo[(pAction)->iActionNbr].flags.actState))
#define setActionState(pWti, pAction, newState) ((pWti)->actWrkrInfo[(pAction)->iActionNbr].flags.actState = \
(newState))
#define getActionResumeInRow(pWti, pAction) (((pWti)->actWrkrInfo[(pAction)->iActionNbr].uResumeOKinRow))
#define setActionResumeInRow(pWti, pAction, val) ((pWti)->actWrkrInfo[(pAction)->iActionNbr].uResumeOKinRow = (val))
#define incActionResumeInRow(pWti, pAction) ((pWti)->actWrkrInfo[(pAction)->iActionNbr].uResumeOKinRow++)
#define getActionNbrResRtry(pWti, pAction) (((pWti)->actWrkrInfo[(pAction)->iActionNbr].iNbrResRtry))
#define setActionNbrResRtry(pWti, pAction, val) ((pWti)->actWrkrInfo[(pAction)->iActionNbr].iNbrResRtry = (val))
#define incActionNbrResRtry(pWti, pAction) ((pWti)->actWrkrInfo[(pAction)->iActionNbr].iNbrResRtry++)
#define wtiInitIParam(piparams) (memset((piparams), 0, sizeof(actWrkrIParams_t)))

#define wtiGetScriptErrno(pWti) ((pWti)->execState.script_errno)
#define wtiSetScriptErrno(pWti, newval) (pWti)->execState.script_errno = (newval)

static inline uint8_t ATTR_UNUSED ATTR_NONNULL(1)
wtiGetPrevWasSuspended(const wti_t * const pWti)
{
	assert(pWti != NULL);
	return pWti->execState.bPrevWasSuspended;
}

static inline void __attribute__((unused))
wtiResetExecState(wti_t * const pWti, batch_t * const pBatch)
{
	wtiSetScriptErrno(pWti, 0);
	pWti->execState.bPrevWasSuspended = 0;
	pWti->execState.bDoAutoCommit = (batchNumMsgs(pBatch) == 1);
}


rsRetVal wtiNewIParam(wti_t *const pWti, action_t *const pAction, actWrkrIParams_t **piparams);
#endif /* #ifndef WTI_H_INCLUDED */
