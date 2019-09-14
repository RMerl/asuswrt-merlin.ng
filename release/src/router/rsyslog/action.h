/* action.h
 * Header file for the action object
 *
 * File begun on 2007-08-06 by RGerhards (extracted from syslogd.c, which
 * was under BSD license at the time of rsyslog fork)
 *
 * Copyright 2007-2018 Adiscon GmbH.
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
#ifndef	ACTION_H_INCLUDED
#define	ACTION_H_INCLUDED 1

#include "syslogd-types.h"
#include "queue.h"

/* external data */
extern int glbliActionResumeRetryCount;
extern int bActionReportSuspension;
extern int bActionReportSuspensionCont;


/* the following struct defines the action object data structure
 */
struct action_s {
	time_t	f_time;		/* used for "max. n messages in m seconds" processing */
	time_t	tActNow;	/* the current time for an action execution. Initially set to -1 and
				   populated on an as-needed basis. This is a performance optimization. */
	time_t	tLastExec;	/* time this action was last executed */
	int	iActionNbr;	/* this action's number (ID) */
	sbool	bExecWhenPrevSusp;/* execute only when previous action is suspended? */
	sbool	bWriteAllMarkMsgs;
	/* should all mark msgs be written (not matter how recent the action was executed)? */
	sbool	bReportSuspension;/* should suspension (and reactivation) of the action reported */
	sbool	bReportSuspensionCont;
	sbool	bDisabled;
	sbool	isTransactional;
	sbool	bCopyMsg;
	int	iSecsExecOnceInterval; /* if non-zero, minimum seconds to wait until action is executed again */
	time_t	ttResumeRtry;	/* when is it time to retry the resume? */
	int	iResumeInterval;/* resume interval for this action */
	int	iResumeRetryCount;/* how often shall we retry a suspended action? (-1 --> eternal) */
	int	iNbrNoExec;	/* number of matches that did not yet yield to an exec */
	int	iExecEveryNthOccur;/* execute this action only every n-th occurence (with n=0,1 -> always) */
	int  	iExecEveryNthOccurTO;/* timeout for n-th occurence feature */
	time_t  tLastOccur;	/* time last occurence was seen (for timing them out) */
	struct modInfo_s *pMod;/* pointer to output module handling this selector */
	void	*pModData;	/* pointer to module data - content is module-specific */
	sbool	bRepMsgHasMsg;	/* "message repeated..." has msg fragment in it (0-no, 1-yes) */
	rsRetVal (*submitToActQ)(action_t *, wti_t*, smsg_t*);/* function submit message to action queue */
	rsRetVal (*qConstruct)(struct queue_s *pThis);
	sbool	bUsesMsgPassingMode;
	sbool	bNeedReleaseBatch; /* do we need to release batch ressources? Depends on ParamPassig modes... */
	int	iNumTpls;	/* number of array entries for template element below */
	struct template **ppTpl;/* array of template to use - strings must be passed to doAction
				 * in this order. */
	paramPassing_t *peParamPassing;	/* mode of parameter passing to action for that template */
	qqueue_t *pQueue;	/* action queue */
	pthread_mutex_t mutAction; /* primary action mutex */
	uchar *pszName;		/* action name */
	DEF_ATOMIC_HELPER_MUT(mutCAS)
	/* error file */
	const char *pszErrFile;
	int fdErrFile;
	pthread_mutex_t mutErrFile;
	/* for per-worker HUP processing */
	pthread_mutex_t mutWrkrDataTable; /* protects table structures */
	void **wrkrDataTable;
	int wrkrDataTableSize;
	int nWrkr;
	/* for statistics subsystem */
	statsobj_t *statsobj;
	STATSCOUNTER_DEF(ctrProcessed, mutCtrProcessed)
	STATSCOUNTER_DEF(ctrFail, mutCtrFail)
	STATSCOUNTER_DEF(ctrSuspend, mutCtrSuspend)
	STATSCOUNTER_DEF(ctrSuspendDuration, mutCtrSuspendDuration)
	STATSCOUNTER_DEF(ctrResume, mutCtrResume)
};


/* function prototypes
 */
rsRetVal actionConstruct(action_t **ppThis);
rsRetVal actionConstructFinalize(action_t *pThis, struct nvlst *lst);
rsRetVal actionDestruct(action_t *pThis);
//rsRetVal actionDbgPrint(action_t *pThis);
rsRetVal actionSetGlobalResumeInterval(int iNewVal);
rsRetVal actionDoAction(action_t *pAction);
rsRetVal actionWriteToAction(action_t *pAction, smsg_t *pMsg, wti_t*);
rsRetVal actionCallHUPHdlr(action_t *pAction);
rsRetVal actionClassInit(void);
rsRetVal addAction(action_t **ppAction, modInfo_t *pMod, void *pModData, omodStringRequest_t *pOMSR,
	struct cnfparamvals *actParams, struct nvlst *lst);
rsRetVal activateActions(void);
rsRetVal actionNewInst(struct nvlst *lst, action_t **ppAction);
rsRetVal actionProcessCnf(struct cnfobj *o);
void actionCommitAllDirect(wti_t *pWti);
void actionRemoveWorker(action_t *const pAction, void *const actWrkrData);
void releaseDoActionParams(action_t * const pAction, wti_t * const pWti, int action_destruct);

/* external data */
extern int iActionNbr;

#endif /* #ifndef ACTION_H_INCLUDED */
