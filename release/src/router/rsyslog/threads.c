/* threads.c
 *
 * This file implements threading support helpers (and maybe the thread object)
 * for rsyslog.
 *
 * File begun on 2007-12-14 by RGerhards
 *
 * Copyright 2007-2016 Adiscon GmbH.
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
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#ifdef HAVE_SYS_PRCTL_H
#  include <sys/prctl.h>
#endif

#include "rsyslog.h"
#include "dirty.h"
#include "linkedlist.h"
#include "threads.h"
#include "srUtils.h"
#include "errmsg.h"
#include "glbl.h"
#include "unicode-helper.h"

/* linked list of currently-known threads */
static linkedList_t llThrds;

/* methods */

/* Construct a new thread object
 */
static rsRetVal
thrdConstruct(thrdInfo_t **ppThis)
{
	DEFiRet;
	thrdInfo_t *pThis;

	assert(ppThis != NULL);

	CHKmalloc(pThis = calloc(1, sizeof(thrdInfo_t)));
	pthread_mutex_init(&pThis->mutThrd, NULL);
	pthread_cond_init(&pThis->condThrdTerm, NULL);
	*ppThis = pThis;

finalize_it:
	RETiRet;
}


/* Destructs a thread object. The object must not be linked to the
 * linked list of threads. Please note that the thread should have been
 * stopped before. If not, we try to do it.
 */
static rsRetVal thrdDestruct(thrdInfo_t *pThis)
{
	DEFiRet;
	assert(pThis != NULL);

	pthread_mutex_lock(&pThis->mutThrd);
	if(pThis->bIsActive == 1) {
		pthread_mutex_unlock(&pThis->mutThrd);
		thrdTerminate(pThis);
	} else {
		pthread_mutex_unlock(&pThis->mutThrd);
		pthread_join(pThis->thrdID, NULL);
	}

	/* call cleanup function, if any */
	if(pThis->pAfterRun != NULL)
		pThis->pAfterRun(pThis);

	pthread_mutex_destroy(&pThis->mutThrd);
	pthread_cond_destroy(&pThis->condThrdTerm);
	free(pThis->name);
	free(pThis);

	RETiRet;
}


/* terminate a thread via the non-cancel interface
 * This is a separate function as it involves a bit more of code.
 * rgerhads, 2009-10-15
 */
static rsRetVal
thrdTerminateNonCancel(thrdInfo_t *pThis)
{
	struct timespec tTimeout;
	int ret;
	int was_active;
	DEFiRet;
	assert(pThis != NULL);

	DBGPRINTF("request term via SIGTTIN for input thread '%s' %p\n",
		  pThis->name, (void*) pThis->thrdID);

	pThis->bShallStop = RSTRUE;
	d_pthread_mutex_lock(&pThis->mutThrd);
	timeoutComp(&tTimeout, glblInputTimeoutShutdown);
	was_active = pThis->bIsActive;
	while(was_active) {
		if(dbgTimeoutToStderr) {
			fprintf(stderr, "rsyslogd debug: info: trying to cooperatively stop "
				"input %s, timeout %d ms\n", pThis->name, glblInputTimeoutShutdown);
		}
		DBGPRINTF("thread %s: initiating termination, timeout %d ms\n",
			pThis->name, glblInputTimeoutShutdown);
		pthread_kill(pThis->thrdID, SIGTTIN);
		ret = d_pthread_cond_timedwait(&pThis->condThrdTerm, &pThis->mutThrd, &tTimeout);
		if(ret == ETIMEDOUT) {
			DBGPRINTF("input thread term: timeout expired waiting on thread %s "
				"termination - canceling\n", pThis->name);
			if(dbgTimeoutToStderr) {
				fprintf(stderr, "rsyslogd debug: input thread term: "
					"timeout expired waiting on thread %s "
					"termination - canceling\n", pThis->name);
			}
			pthread_cancel(pThis->thrdID);
			break;
		} else if(ret != 0) {
			char errStr[1024];
			int err = errno;
			rs_strerror_r(err, errStr, sizeof(errStr));
			DBGPRINTF("input thread term: cond_wait returned with error %d: %s\n",
				  err, errStr);
		}
		was_active = pThis->bIsActive;
	}
	d_pthread_mutex_unlock(&pThis->mutThrd);

	if(was_active) {
		DBGPRINTF("non-cancel input thread termination FAILED for thread %s %p\n",
			  pThis->name, (void*) pThis->thrdID);
	} else {
		DBGPRINTF("non-cancel input thread termination succeeded for thread %s %p\n",
			  pThis->name, (void*) pThis->thrdID);
	}

	RETiRet;
}


/* terminate a thread gracefully.
 */
rsRetVal thrdTerminate(thrdInfo_t *pThis)
{
	DEFiRet;
	assert(pThis != NULL);

	if(pThis->bNeedsCancel) {
		DBGPRINTF("request term via canceling for input thread %s\n", pThis->name);
		if(dbgTimeoutToStderr) {
			fprintf(stderr, "rsyslogd debug: request term via canceling for "
				"input thread %s\n", pThis->name);
		}
		pthread_cancel(pThis->thrdID);
	} else {
		thrdTerminateNonCancel(pThis);
	}
	pthread_join(pThis->thrdID, NULL); /* wait for input thread to complete */

	RETiRet;
}


/* terminate all known threads gracefully.
 */
rsRetVal thrdTerminateAll(void)
{
	DEFiRet;
	llDestroy(&llThrds);
	RETiRet;
}


/* This is an internal wrapper around the user thread function. Its
 * purpose is to handle all the necessary housekeeping stuff so that the
 * user function needs not to be aware of the threading calls. The user
 * function call has just "normal", non-threading semantics.
 * rgerhards, 2007-12-17
 */
static ATTR_NORETURN void*
thrdStarter(void *const arg)
{
	DEFiRet;
	thrdInfo_t *const pThis = (thrdInfo_t*) arg;
#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	uchar thrdName[32] = "in:";
#	endif

	assert(pThis != NULL);
	assert(pThis->pUsrThrdMain != NULL);

#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	ustrncpy(thrdName+3, pThis->name, 20);
	dbgOutputTID((char*)thrdName);

	/* set thread name - we ignore if the call fails, has no harsh consequences... */
	if(prctl(PR_SET_NAME, thrdName, 0, 0, 0) != 0) {
		DBGPRINTF("prctl failed, not setting thread name for '%s'\n", pThis->name);
	} else {
		DBGPRINTF("set thread name to '%s'\n", thrdName);
	}
#	endif

	/* block all signals except SIGTTIN and SIGSEGV */
	sigset_t sigSet;
	sigfillset(&sigSet);
	sigdelset(&sigSet, SIGTTIN);
	sigdelset(&sigSet, SIGSEGV);
	pthread_sigmask(SIG_BLOCK, &sigSet, NULL);

	/* setup complete, we are now ready to execute the user code. We will not
	 * regain control until the user code is finished, in which case we terminate
	 * the thread.
	 */
	iRet = pThis->pUsrThrdMain(pThis);

	if(iRet == RS_RET_OK) {
		dbgprintf("thrdStarter: usrThrdMain %s - 0x%lx returned with iRet %d, exiting now.\n",
			  pThis->name, (unsigned long) pThis->thrdID, iRet);
	} else {
		LogError(0, iRet, "main thread of %s terminated abnormally", pThis->name);
	}

	/* signal master control that we exit (we do the mutex lock mostly to
	 * keep the thread debugger happer, it would not really be necessary with
	 * the logic we employ...)
	 */
	d_pthread_mutex_lock(&pThis->mutThrd);
	pThis->bIsActive = 0;
	pthread_cond_signal(&pThis->condThrdTerm);
	d_pthread_mutex_unlock(&pThis->mutThrd);

	ENDfunc
	pthread_exit(0);
}
/* Start a new thread and add it to the list of currently
 * executing threads. It is added at the end of the list.
 * rgerhards, 2007-12-14
 */
rsRetVal thrdCreate(rsRetVal (*thrdMain)(thrdInfo_t*), rsRetVal(*afterRun)(thrdInfo_t *),
	sbool bNeedsCancel, uchar *name)
{
	DEFiRet;
	thrdInfo_t *pThis;
#if defined (_AIX)
	pthread_attr_t aix_attr;
#endif

	assert(thrdMain != NULL);

	CHKiRet(thrdConstruct(&pThis));
	pThis->bIsActive = 1;
	pThis->pUsrThrdMain = thrdMain;
	pThis->pAfterRun = afterRun;
	pThis->bNeedsCancel = bNeedsCancel;
	pThis->name = ustrdup(name);
#if defined (_AIX)
	pthread_attr_init(&aix_attr);
	pthread_attr_setstacksize(&aix_attr, 4096*512);
	pthread_create(&pThis->thrdID, &aix_attr, thrdStarter, pThis);
#else
	pthread_create(&pThis->thrdID, &default_thread_attr, thrdStarter, pThis);
#endif
	CHKiRet(llAppend(&llThrds, NULL, pThis));

finalize_it:
	RETiRet;
}


/* initialize the thread-support subsystem
 * must be called once at the start of the program
 */
rsRetVal thrdInit(void)
{
	DEFiRet;
	iRet = llInit(&llThrds, thrdDestruct, NULL, NULL);
	RETiRet;
}


/* de-initialize the thread subsystem
 * must be called once at the end of the program
 */
rsRetVal thrdExit(void)
{
	DEFiRet;
	iRet = llDestroy(&llThrds);
	RETiRet;
}


/* vi:set ai:
 */
