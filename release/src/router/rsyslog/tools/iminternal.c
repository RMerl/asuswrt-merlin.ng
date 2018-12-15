/* iminternal.c
 * This file set implements the internal messages input module for rsyslog.
 * Note: we currently do not have an input module spec, but
 * we will have one in the future. This module needs then to be
 * adapted.
 *
 * File begun on 2007-08-03 by RGerhards
 *
 * Copyright 2007-2017 Rainer Gerhards and Adiscon GmbH.
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
#include "rsyslog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>

#include "syslogd.h"
#include "linkedlist.h"
#include "iminternal.h"

static linkedList_t llMsgs;
static pthread_mutex_t mutList = PTHREAD_MUTEX_INITIALIZER;


/* destructs an iminternal object
 */
static rsRetVal iminternalDestruct(iminternal_t *pThis)
{
	DEFiRet;

	if(pThis->pMsg != NULL)
		msgDestruct(&pThis->pMsg);

	free(pThis);

	RETiRet;
}


/* Construct an iminternal object
 */
static rsRetVal iminternalConstruct(iminternal_t **ppThis)
{
	DEFiRet;
	if((*ppThis = (iminternal_t*) calloc(1, sizeof(iminternal_t))) == NULL) {
		iRet = RS_RET_OUT_OF_MEMORY;
	}
	RETiRet;
}


/* add a message to the linked list
 * Note: the pMsg reference counter is not incremented. Consequently,
 * the caller must NOT decrement it. The caller actually hands over
 * full ownership of the pMsg object.
 */
rsRetVal iminternalAddMsg(smsg_t *pMsg)
{
	DEFiRet;
	iminternal_t *pThis = NULL;
	struct timespec to;
	int r;
	int is_locked = 0;

	/* we guard against deadlock, so we can guarantee rsyslog will never
	 * block due to internal messages. The 1 second timeout should be
	 * sufficient under all circumstances.
	 */
	to.tv_sec = time(NULL) + 1;
	to.tv_nsec = 0;
	#if !defined(__APPLE__)
	r = pthread_mutex_timedlock(&mutList, &to);
	#else
	r = pthread_mutex_trylock(&mutList); // must check
	#endif
	is_locked = 1;
	if(r != 0) {
		dbgprintf("iminternalAddMsg: timedlock for mutex failed with %d, msg %s\n",
			r, getMSG(pMsg));
		/* the message is lost, nothing we can do against this! */
		msgDestruct(&pMsg);
		ABORT_FINALIZE(RS_RET_ERR);
	}
	CHKiRet(iminternalConstruct(&pThis));
	pThis->pMsg = pMsg;
	CHKiRet(llAppend(&llMsgs,  NULL, (void*) pThis));

	if(bHaveMainQueue) {
		DBGPRINTF("signaling new internal message via SIGTTOU: '%s'\n",
			pThis->pMsg->pszRawMsg);
		kill(glblGetOurPid(), SIGTTOU);
	}

finalize_it:
	if(is_locked) {
		pthread_mutex_unlock(&mutList);
	}
	if(iRet != RS_RET_OK) {
		dbgprintf("iminternalAddMsg() error %d - can not otherwise report this error, message lost\n", iRet);
		if(pThis != NULL)
			iminternalDestruct(pThis);
	}

	RETiRet;
}


/* pull the first error message from the linked list, remove it
 * from the list and return it to the caller. The caller is
 * responsible for freeing the message!
 */
rsRetVal iminternalRemoveMsg(smsg_t **ppMsg)
{
	DEFiRet;
	iminternal_t *pThis;
	linkedListCookie_t llCookie = NULL;

	pthread_mutex_lock(&mutList);
	CHKiRet(llGetNextElt(&llMsgs, &llCookie, (void*)&pThis));
	*ppMsg = pThis->pMsg;
	pThis->pMsg = NULL; /* we do no longer own it - important for destructor */

	if(llDestroyRootElt(&llMsgs) != RS_RET_OK) {
		dbgprintf("Root element of iminternal linked list could not be destroyed - there is "
			"nothing we can do against it, we ignore it for now. Things may go wild "
			"from here on. This is most probably a program logic error.\n");
	}

finalize_it:
	pthread_mutex_unlock(&mutList);
	RETiRet;
}

/* tell the caller if we have any messages ready for processing.
 * 0 means we have none, everything else means there is at least
 * one message ready.
 */
rsRetVal iminternalHaveMsgReady(int* pbHaveOne)
{
	pthread_mutex_lock(&mutList);
	const rsRetVal iRet = llGetNumElts(&llMsgs, pbHaveOne);
	pthread_mutex_unlock(&mutList);
	return iRet;
}


/* initialize the iminternal subsystem
 * must be called once at the start of the program
 */
rsRetVal modInitIminternal(void)
{
	DEFiRet;
	iRet = llInit(&llMsgs, iminternalDestruct, NULL, NULL);
	RETiRet;
}


/* de-initialize the iminternal subsystem
 * must be called once at the end of the program
 * Note: the error list must have been pulled first. We do
 * NOT care if there are any errors left - we simply destroy
 * them.
 */
rsRetVal modExitIminternal(void)
{
	DEFiRet;
	iRet = llDestroy(&llMsgs);
	RETiRet;
}
