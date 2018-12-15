/* header for ratelimit.c
 *
 * Copyright 2012-2016 Adiscon GmbH.
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
#ifndef INCLUDED_RATELIMIT_H
#define INCLUDED_RATELIMIT_H

struct ratelimit_s {
	char *name;	/**< rate limiter name, e.g. for user messages */
	/* support for Linux kernel-type ratelimiting */
	unsigned short interval;
	unsigned burst;
	intTiny severity; /**< ratelimit only equal or lower severity levels (eq or higher values) */
	unsigned done;
	unsigned missed;
	time_t begin;
	/* support for "last message repeated n times */
	int bReduceRepeatMsgs; /**< shall we do "last message repeated n times" processing? */
	unsigned nsupp;		/**< nbr of msgs suppressed */
	smsg_t *pMsg;
	sbool bThreadSafe;	/**< do we need to operate in Thread-Safe mode? */
	sbool bNoTimeCache;	/**< if we shall not used cached reception time */
	pthread_mutex_t mut;	/**< mutex if thread-safe operation desired */
};

/* prototypes */
rsRetVal ratelimitNew(ratelimit_t **ppThis, const char *modname, const char *dynname);
void ratelimitSetThreadSafe(ratelimit_t *ratelimit);
void ratelimitSetLinuxLike(ratelimit_t *ratelimit, unsigned short interval, unsigned burst);
void ratelimitSetNoTimeCache(ratelimit_t *ratelimit);
void ratelimitSetSeverity(ratelimit_t *ratelimit, intTiny severity);
rsRetVal ratelimitMsg(ratelimit_t *ratelimit, smsg_t *pMsg, smsg_t **ppRep);
rsRetVal ratelimitAddMsg(ratelimit_t *ratelimit, multi_submit_t *pMultiSub, smsg_t *pMsg);
void ratelimitDestruct(ratelimit_t *pThis);
int ratelimitChecked(ratelimit_t *ratelimit);
rsRetVal ratelimitModInit(void);
void ratelimitModExit(void);

#endif /* #ifndef INCLUDED_RATELIMIT_H */
