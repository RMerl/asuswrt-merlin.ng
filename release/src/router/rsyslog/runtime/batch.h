/* Definition of the batch_t data structure.
 * I am not sure yet if this will become a full-blown object. For now, this header just
 * includes the object definition and is not accompanied by code.
 *
 * Copyright 2009-2013 by Rainer Gerhards and Adiscon GmbH.
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

#ifndef BATCH_H_INCLUDED
#define BATCH_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include "msg.h"

/* enum for batch states. Actually, we violate a layer here, in that we assume that a batch is used
 * for action processing. So far, this seems acceptable, the status is simply ignored inside the
 * main message queue. But over time, it could potentially be useful to split the two.
 * rgerhad, 2009-05-12
 */
#define BATCH_STATE_RDY  0	/* object ready for processing */
#define BATCH_STATE_BAD  1	/* unrecoverable failure while processing, do NOT resubmit to same action */
#define BATCH_STATE_SUB  2	/* message submitted for processing, outcome yet unknown */
#define BATCH_STATE_COMM 3	/* message successfully commited */
#define BATCH_STATE_DISC 4 	/* discarded - processed OK, but do not submit to any other action */
typedef unsigned char batch_state_t;


/* an object inside a batch, including any information (state!) needed for it to "life".
 */
struct batch_obj_s {
	smsg_t *pMsg;
};

/* the batch
 * This object is used to dequeue multiple user pointers which are than handed over
 * to processing. The size of elements is fixed after queue creation, but may be
 * modified by config variables (better said: queue properties).
 * Note that a "user pointer" in rsyslog context so far always is a message
 * object. We stick to the more generic term because queues may potentially hold
 * other types of objects, too.
 * rgerhards, 2009-05-12
 * Note that nElem is not necessarily equal to nElemDeq. This is the case when we
 * discard some elements (because of configuration) during dequeue processing. As
 * all Elements are only deleted when the batch is processed, we can not immediately
 * delete them. So we need to keep their number that we can delete them when the batch
 * is completed (else, the whole process does not work correctly).
 */
struct batch_s {
	int maxElem;		/* maximum number of elements that this batch supports */
	int nElem;		/* actual number of element in this entry */
	int nElemDeq;		/* actual number of elements dequeued (and thus to be deleted) - see comment above! */
	qDeqID	deqID;		/* ID of dequeue operation that generated this batch */
	batch_obj_t *pElem;	/* batch elements */
	batch_state_t *eltState;/* state (array!) for individual objects.
	   			   NOTE: we have moved this out of batch_obj_t because we
				         get a *much* better cache hit ratio this way. So do not
					 move it back into this structure! Note that this is really
					 a HUGE saving, even if it doesn't look so (both profiler
					 data as well as practical tests indicate that!).
				*/
};


/* get number of msgs for this batch */
#define batchNumMsgs(pBatch) ((pBatch)->nElem)


/* set the status of the i-th batch element. Note that once the status is
 * DISC, it will never be reset. So this function can NOT be used to initialize
 * the state table. -- rgerhards, 2010-06-10
 */
static inline void __attribute__((unused))
batchSetElemState(batch_t * const pBatch, const int i, const batch_state_t newState) {
	if(pBatch->eltState[i] != BATCH_STATE_DISC)
		pBatch->eltState[i] = newState;
}


/* check if an element is a valid entry. We do NOT verify if the
 * element index is valid. -- rgerhards, 2010-06-10
 */
#define batchIsValidElem(pBatch, i) ((pBatch)->eltState[(i)] != BATCH_STATE_DISC)


/* free members of a batch "object". Note that we can not do the usual
 * destruction as the object typically is allocated on the stack and so the
 * object itself cannot be freed! -- rgerhards, 2010-06-15
 */
static inline void __attribute__((unused))
batchFree(batch_t * const pBatch) {
	free(pBatch->pElem);
	free(pBatch->eltState);
}


/* initialiaze a batch "object". The record must already exist,
 * we "just" initialize it. The max number of elements must be
 * provided. -- rgerhards, 2010-06-15
 */
static inline rsRetVal __attribute__((unused))
batchInit(batch_t *const pBatch, const int maxElem)
{
	DEFiRet;
	pBatch->maxElem = maxElem;
	CHKmalloc(pBatch->pElem = calloc((size_t)maxElem, sizeof(batch_obj_t)));
	CHKmalloc(pBatch->eltState = calloc((size_t)maxElem, sizeof(batch_state_t)));
finalize_it:
	RETiRet;
}

#ifdef _AIX
#endif

#endif /* #ifndef BATCH_H_INCLUDED */
