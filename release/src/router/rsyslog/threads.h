/* Definition of the threading support module.
 *
 * Copyright 2007-2012 Adiscon GmbH.
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

#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED

/* the thread object */
struct thrdInfo {
	pthread_mutex_t mutThrd;/* mutex for handling long-running operations and shutdown */
	pthread_cond_t condThrdTerm;/* condition: thread terminates (used just for shutdown loop) */
	int bIsActive;		/* Is thread running? */
	int bShallStop;		/* set to 1 if the thread should be stopped ? */
	rsRetVal (*pUsrThrdMain)(struct thrdInfo*); /* user thread main to be called in new thread */
	rsRetVal (*pAfterRun)(struct thrdInfo*);   /* cleanup function */
	pthread_t thrdID;
	sbool bNeedsCancel;	/* must input be terminated by pthread_cancel()? */
	uchar *name;		/* a thread name, mainly for user interaction */
};

/* prototypes */
rsRetVal thrdExit(void);
rsRetVal thrdInit(void);
rsRetVal thrdTerminate(thrdInfo_t *pThis);
rsRetVal thrdTerminateAll(void);
rsRetVal thrdCreate(rsRetVal (*thrdMain)(thrdInfo_t*), rsRetVal(*afterRun)(thrdInfo_t *), sbool, uchar*);

/* macros (replace inline functions) */

#endif /* #ifndef THREADS_H_INCLUDED */
