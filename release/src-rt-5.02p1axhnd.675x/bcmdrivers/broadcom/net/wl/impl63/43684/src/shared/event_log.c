/*
 * EVENT_LOG system
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: event_log.c 692339 2017-03-27 17:04:27Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <event_log.h>

#ifdef EVENT_LOG_COMPILE

static event_log_top_t *event_log_top = NULL;

uint8 *event_log_tag_sets = NULL;

#ifdef EVENT_LOG_TS
bool enable_ts = TRUE;
#else
bool enable_ts = FALSE;
#endif /* EVENT_LOG_TS */

extern uint32 logstrs_size;

/* Timestamp should be synced every 1-sec or more */
#define EL_SYNC_TIME 1000
/* Lengths of some mandatory fields in words in event log record */
#define EVENT_LOG_RECORD_HDR_LEN		1
#define EVENT_LOG_RECORD_TIMESTAMP_LEN		1

int
BCMATTACHFN(event_log_init)(osl_t *osh)
{
	int i;
	event_log_set_t **setpp;

	if ((event_log_top = MALLOC(osh, sizeof(event_log_top_t))) == NULL) {
		return -1;
	}

	event_log_top->magic = EVENT_LOG_TOP_MAGIC;
	event_log_top->version = EVENT_LOG_VERSION;

	/* Init the sets */
	event_log_top->num_sets = NUM_EVENT_LOG_SETS;
	if ((event_log_top->sets = MALLOC(osh, sizeof(uint32) * NUM_EVENT_LOG_SETS)) == NULL) {
		goto err1;
	}

	setpp = event_log_top->sets;
	for (i = 0; i < NUM_EVENT_LOG_SETS; i++) {
		*setpp++ = NULL;
	}

	event_log_top->logstrs_size = (uint32) &logstrs_size;

	/* Init the tag flags */
	if ((event_log_tag_sets = MALLOC(osh, sizeof(uint8) * (EVENT_LOG_TAG_MAX + 1))) == NULL) {
		goto err2;
	}

	for (i = 0; i <= EVENT_LOG_TAG_MAX; i++) {
		*(event_log_tag_sets + i) = EVENT_LOG_TAG_FLAG_NONE;
	}

	/* Init the timestamp */
	event_log_time_sync(OSL_SYSUPTIME());

	/* Put a dummy in the logstrs so that it isn't empty */
	EVENT_LOG(EVENT_LOG_TAG_MAX, " ");

	return 0;

err2:
	MFREE(osh, event_log_top->sets, sizeof(uint32) * NUM_EVENT_LOG_SETS);

err1:
	MFREE(osh, event_log_top, sizeof(event_log_top_t));
	return -1;
}

/*
 * This function should only be used by dongle services that need to log very early in the
 * boot sequence where the event_log API has not been initialised.
 */
bool event_log_is_ready(void)
{
	return event_log_top != NULL;
}

int
event_log_set_init(osl_t *osh, int set_num, int size)
{
	event_log_set_t *ts;
	event_log_set_t **setpp;

	if ((event_log_top == NULL) || (event_log_top->sets == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS) || (size == 0)) {
		return -1;
	}

	/* See if the set has already been initialized (will re-initialize if it exists) */
	setpp = event_log_top->sets + set_num;
	if ((ts = *setpp) != NULL) {
		event_log_set_shrink(osh, set_num, ts->size * ts->blockcount);
		ASSERT(ts->blockcount == 0);
	} else if ((ts = MALLOC(osh, sizeof(event_log_set_t))) == NULL) {
		return -1;
	}

	/* Init to empty */
	ts->first_block = NULL;
	ts->last_block = NULL;
	ts->cur_block = NULL;
	ts->logtrace_block = NULL;
	ts->blockcount = 0;
	ts->timestamp = event_log_top->timestamp;
	ts->cyclecount = event_log_top->cyclecount;
	ts->size = MIN(size, EVENT_LOG_MAX_BLOCK_SIZE);
	ts->size = ALIGN_SIZE(ts->size, sizeof(uint32));
	ts->destination = SET_DESTINATION_HOST;

	*setpp = ts;

	/* Expand the empty set */
	return (event_log_set_expand(osh, set_num, size));
}

int
event_log_set_destination_set(int set, event_log_set_destination_t dest)
{
	event_log_set_t **setpp;
	int ret = BCME_ERROR;

	if ((event_log_top == NULL) ||
	    (set >= NUM_EVENT_LOG_SETS) ||
	    (event_log_top->sets == NULL)) {
		ret = BCME_ERROR;
	} else {

		/* See if the set has already been initialized */
		setpp = event_log_top->sets + set;
		if (*setpp != NULL) {
			(*setpp)->destination = dest;
			ret = BCME_OK;
		}
	}
	return ret;
}

event_log_set_destination_t
event_log_set_destination_get(int set)
{
	event_log_set_t **setpp;
	event_log_set_destination_t ret = SET_DESTINATION_INVALID;
	if ((event_log_top == NULL) ||
	    (set >= NUM_EVENT_LOG_SETS) ||
	    (event_log_top->sets == NULL)) {
		ret = SET_DESTINATION_INVALID;
	} else {
		/* See if the set has already been initialized */
		setpp = event_log_top->sets + set;
		if (*setpp != NULL) {
			ret = (*setpp)->destination;
		}
	}
	return ret;
}

int
event_log_set_expand(osl_t *osh, int set_num, int size)
{
	/* Add a event_log block before the first one */
	event_log_set_t *ts;
	event_log_block_t *tb;
	int err = 0;

	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS) ||
	    (event_log_top->sets == NULL)) {
		return -1;
	}

	ts = *(event_log_top->sets + set_num);
	if (ts == NULL) {
		return -1;
	}

	/* Allocate multiple blocks if greater than max blocksize */
	while (size > 0) {
		int this_size = ts->size;

		/* Allocate and the clear the block + logs in one chunk */
		tb = MALLOCZ_NOPERSIST(osh, sizeof(event_log_block_t) + this_size);
		if (tb == NULL) {
			err = -1;
			break;
		}

		/* Set the end ptr saving one slot for the end count */
		tb->end_ptr = (&tb->event_logs) + (this_size >> 2) - 1;
		tb->pktlen = (uint32) tb->end_ptr - (uint32) &tb->pktlen;

		/* This becomes the last block */
		if (ts->first_block == NULL) {
			/* This is the first one */
			ts->first_block = tb;
			tb->next_block = tb;
			tb->prev_block = tb;
			ts->last_block = tb;
		} else {
			ts->first_block->prev_block = tb;
			ts->last_block->next_block = tb;
			tb->next_block = ts->first_block;
			tb->prev_block = ts->last_block;
			ts->last_block = tb;
		}

		ts->blockcount++;

		size -= this_size;
	}

	/* Reset the set logging to the first block */
	ts->cur_block = ts->first_block;

	/* Reset the counts for logtrace */
	ts->blockfill_count = 0;
	ts->logtrace_block = ts->first_block;
	if (ts->first_block) {
		ts->cur_ptr = &(ts->first_block->event_logs);
		ts->logtrace_block->count = ts->blockfill_count++;
		ts->logtrace_count = ts->logtrace_block->count;
	}

	return err;
}

int
event_log_set_shrink(osl_t *osh, int set_num, int size)
{
	/* Delete the last block(s) - even if it is the only one */
	event_log_set_t *ts;
	event_log_block_t *lb;

	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS)) {
		return -1;
	}

	ts = *(event_log_top->sets + set_num);
	if (ts == NULL) {
		return -1;
	}

	while (size > 0) {
		int this_size = ts->size;

		lb = ts->last_block;
		if (lb == NULL) {           /* Check for all empty */
			ASSERT(ts->blockcount == 0);
			break;
		}

		size -= this_size;

		if (lb == ts->first_block) {  /* Deleting last block */
			ts->cur_block = NULL;
			ts->last_block = NULL;
			ts->first_block = NULL;
			ts->cur_ptr = NULL;

		} else {
			/* Set up the circular list of sets */
			ts->last_block = lb->prev_block;
			ts->first_block->prev_block = ts->last_block;
			ts->last_block->next_block = ts->first_block;

			/* Start event_log over */
			ts->cur_block = ts->first_block;
			ts->cur_ptr = &ts->first_block->event_logs;
		}

		/* Release the block */
		MFREE(osh, lb, sizeof(event_log_block_t) + this_size);
		ts->blockcount--;
	}

	/* Reset the set logging to the first block */
	ts->cur_block = ts->first_block;

	/* Reset the counts for logtrace */
	ts->blockfill_count = 0;
	ts->logtrace_block = ts->first_block;
	if (ts->first_block) {
		ts->cur_ptr = &(ts->first_block->event_logs);
		ts->logtrace_block->count = ts->blockfill_count++;
		ts->logtrace_count = ts->logtrace_block->count;
	}

	return 0;
}

int
event_log_tag_start(int tag, int set_num, int flags)
{
	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS) ||
	    (tag > EVENT_LOG_TAG_MAX)) {
		return -1;
	}

	*(event_log_tag_sets + tag) = flags | set_num;

	return 0;
}

int
event_log_tag_set_retrieve(int tag)
{
	if ((event_log_top == NULL) ||
	    (tag > EVENT_LOG_TAG_MAX) ||
	    /* if there is no tag atribute defined. */
	    (!(event_log_tag_sets[tag] & ~EVENT_LOG_TAG_FLAG_SET_MASK))) {
		return -1;
	}

	return (event_log_tag_sets[tag] & EVENT_LOG_TAG_FLAG_SET_MASK);
}

int
event_log_tag_stop(int tag)
{
	if ((event_log_top == NULL) ||
	    (tag > EVENT_LOG_TAG_MAX)) {
		return -1;
	}

	*(event_log_tag_sets + tag) = EVENT_LOG_TAG_FLAG_NONE;

	return 0;
}

int
event_log_get(int set_num, int buflen, void *buf)
{
	int size;
	uint8 *pkt = event_log_next_logtrace(set_num);
	if (pkt == NULL) {
		*((uint32 *) buf) = 0;
		return -1;
	}

	/* Compute how much to move in */
	size = *((uint16 *) pkt);

	/* Copy as much as possible */
	bcopy(pkt, buf, (buflen < size) ? buflen : size);

	return 0;
}

uint8 *
event_log_next_logtrace(int set_num)
{
	event_log_set_t *ts;
	event_log_block_t *lb;

	/* Check if active, valid set */
	if ((event_log_top == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS)) {
		return NULL;
	}

	/* See if this set has been initialized and has blocks */
	ts = *(event_log_top->sets + set_num);
	if ((ts == NULL) || (ts->logtrace_block == NULL)) {
		return NULL;
	}

	/* Still send the block because event log asked it in the first place.
	 * Asking to send first and then reverting decision not to send just because
	 * we have one block doesn't look correct.
	 * There could be cases where a log set has only 1 buffer (ecounters).
	 * The buffer needs to be sent when there is a trigger.
	 * The application using the log set needs to be aware that if its set uses only
	 * 1 buffer, then it may overwrite its own buffer while the underlying transport
	 * mechanism is performing a copy operation.
	 */
	lb = ts->logtrace_block;
	if ((lb == ts->cur_block) &&
	    (ts->cur_block->count == ts->logtrace_count)) {
		/* Block still in use */
		return NULL;
	}

	/* If number of blocks < 2, this may make logtrace_count == blockfillcount
	 * logtrace_count and blockfillcount need to be 1 away from each other
	 * with blockfillcount leading.
	 */
	if ((ts->logtrace_count != lb->count) && (ts->blockcount > 1)) {
		/* Overran the block so things wrapped all the way
		 * around.  Start over with the first clean block
		 * which is the one after the current block.
		 */
		lb = ts->cur_block->next_block;
		ts->logtrace_block = lb;
		ts->logtrace_count = lb->count;
	}

	/* if blockcount == 1  logtrace is lagging by more than one block,
	 * correct the logcount and return NULL.
	 * This means that single block wrapped around too fast causing
	 * older entries to be overwritten. They are lost anyway, so simply
	 * adjust logtrace_count and move ahead. Mark the current block as
	 * in use.
	 * If the write pointer wrapped around, check if log block is really
	 * not being written to. We cannot send an incomplete block.
	 * Since block count == 1, lb = ts->logtrace_block = ts->cur_block
	 */
	if ((ts->blockcount == 1) &&
		/* did we wrap around more than once? */
		((lb->count != (ts->logtrace_count+1)) ||
		 /* Is this block being written?  */
		(ts->cur_ptr != &lb->event_logs)))
	{
		ts->logtrace_count = lb->count;
		return NULL;
	}

	/* On to the next block */
	ts->logtrace_count++;
	ts->logtrace_block = lb->next_block;

	/* Reuse the timestamp field. Put new information in this field
	 * We want to put set_num to indicate which set generated this block
	 * We don't have many sets. SO just use last 6 bits of the timestamp.
	 */
	lb->extra_hdr_info = (uint32)(set_num & 0x3F);

	return (uint8 *) &(lb->pktlen);
}

/* Define the macros used to generate event log entries */

#ifdef LOGTRACE
static event_log_logtrace_trigger_fn_t logtrace_trigger_fn = NULL;
static void *logtrace_trigger_ctx = NULL;
#define LOGTRACE_TRIGGER() do { \
		if (logtrace_trigger_fn != NULL) { \
			logtrace_trigger_fn(logtrace_trigger_ctx); \
		} \
	} while (FALSE)
#else
#define LOGTRACE_TRIGGER() do {} while (FALSE)
#endif // endif

#define _EVENT_LOG_END_CHECK(n)				\
	if ((ts->cur_ptr + (n)) >= cb->end_ptr) {	\
		do {					\
			/* Clear the rest */		\
			*ts->cur_ptr++ = 0;		\
		} while (ts->cur_ptr < cb->end_ptr);	\
		/* Mark the end for matching */		\
		*ts->cur_ptr = cb->count;		\
							\
		/* Advance to the next block */		\
		cb = cb->next_block;			\
		ts->cur_block = cb;			\
		ts->cur_ptr = &cb->event_logs;		\
		cb->count = ts->blockfill_count++;	\
		cb->extra_hdr_info = ts->timestamp;	\
		LOGTRACE_TRIGGER();			\
	}

#define _EVENT_LOG_STORE(el)						\
	*ts->cur_ptr++ = el

#define _EVENT_LOG_START(tag, fmt_num, num)				\
	event_log_set_t *ts = *(event_log_top->sets +			\
				(flag & EVENT_LOG_TAG_FLAG_SET_MASK));	\
	event_log_block_t *cb;						\
									\
	if ((ts != NULL) && ((cb = ts->cur_block) != NULL)) {		\
		event_log_hdr_t th = {{tag, num + 1, fmt_num}};		\
		if (enable_ts && event_log_top->timestamp != ts->timestamp) {	\
			/* Must put a timestamp in the log */		\
			event_log_hdr_t tsh = {{EVENT_LOG_TAG_TS, 3, 0}}; \
			_EVENT_LOG_END_CHECK(4);			\
			ts->timestamp = event_log_top->timestamp;	\
			ts->cyclecount = event_log_top->cyclecount;	\
			*ts->cur_ptr++ = ts->timestamp;			\
			*ts->cur_ptr++ = ts->cyclecount;		\
			*ts->cur_ptr++ = get_arm_cyclecount();		\
			*ts->cur_ptr++ = tsh.t;				\
		}							\
	        _EVENT_LOG_END_CHECK(num + 2);				\
		/* Note missing close paren */

#define _EVENT_LOG_END							\
	        /* Note missing open paren */				\
	        *ts->cur_ptr++ = get_arm_cyclecount();			\
	        *ts->cur_ptr++ = th.t;					\
	}

/*
 * The first 5 event log variants have explicit variables.  This is
 * slightly more efficient than using va_args because it removes the
 * requirement that the parameters get saved in memory.  The tradeoff
 * is the size of the generated code.  Since only the first few
 * parameters get passed in registers anyways there isn't much point
 * beyond 4 parameters so we use a loop for 5 or more params
 */

void
event_log0(int tag, int fmt_num)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x\n", tag, fmt_num);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 0);
		_EVENT_LOG_END;
	}
}

void
event_log1(int tag, int fmt_num, uint32 t1)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x\n", tag,
		       fmt_num, t1);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 1);
		*ts->cur_ptr++ = t1;
		_EVENT_LOG_END;
	}
}

void
event_log2(int tag, int fmt_num, uint32 t1, uint32 t2)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x %x\n", tag,
		       fmt_num, t1, t2);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 2);
		*ts->cur_ptr++ = t1;
		*ts->cur_ptr++ = t2;
		_EVENT_LOG_END;
	}
}

void
event_log3(int tag, int fmt_num, uint32 t1, uint32 t2, uint32 t3)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x %x %x\n", tag,
		       fmt_num, t1, t2, t3);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 3);
		*ts->cur_ptr++ = t1;
		*ts->cur_ptr++ = t2;
		*ts->cur_ptr++ = t3;
		_EVENT_LOG_END;
	}
}

void
event_log4(int tag, int fmt_num, uint32 t1, uint32 t2, uint32 t3, uint32 t4)
{
	uint8 flag = *(event_log_tag_sets + tag);
	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x %x %x %x %x\n", tag,
		       fmt_num, t1, t2, t3, t4);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, 4);
		*ts->cur_ptr++ = t1;
		*ts->cur_ptr++ = t2;
		*ts->cur_ptr++ = t3;
		*ts->cur_ptr++ = t4;
		_EVENT_LOG_END;
	}
}

void
event_logn(int num_args, int tag, int fmt_num, ...)
{
	uint8 flag = *(event_log_tag_sets + tag);
	va_list ap;
	int i;

	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {

		printf("EL: %x %x", tag, fmt_num);
		va_start(ap, fmt_num);
		for (i = num_args; i != 0; i--) {
			printf(" %x", va_arg(ap, uint32));
		}
		printf("\n");
		va_end(ap);
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, fmt_num, num_args);
		va_start(ap, fmt_num);
		for (i = num_args; i != 0; i--) {
			*ts->cur_ptr++ = va_arg(ap, uint32);
		}
		_EVENT_LOG_END;
		va_end(ap);
	}
}

/* Sync the timestamp with the PMU timer */
void event_log_time_sync(uint32 ms)
{
	if ((ms - event_log_top->timestamp) >= EL_SYNC_TIME) {
		event_log_top->timestamp = ms;
		event_log_top->cyclecount = get_arm_cyclecount();
	}
}

void event_log_caller_return_address(int tag)
{
	EVENT_LOG(tag, "Error occurred. Logged return address = 0x%x",
		(uint32)__builtin_return_address(0));
}

void
BCMATTACHFN(event_log_set_logtrace_trigger_fn)(event_log_logtrace_trigger_fn_t fn, void *ctx)
{
#ifdef LOGTRACE
	logtrace_trigger_fn = fn;
	logtrace_trigger_ctx = ctx;
#endif // endif
}

event_log_top_t *
BCMATTACHFN(event_log_get_top)(void)
{
	return event_log_top;
}

void
event_log_buffer(int tag, uint8 *buf, int size)
{
	uint8 flag = *(event_log_tag_sets + tag);
	int i, num_args;
	event_log_set_t *ts1;

	ts1 = *(event_log_top->sets + (flag & EVENT_LOG_TAG_FLAG_SET_MASK));
	if ((ts1 == NULL) || (size > ts1->size))
		return;

	num_args = (size + 3) / 4;

	if (flag & EVENT_LOG_TAG_FLAG_PRINT) {
		printf("EL: %x %x", tag, 0xffff);
		for (i = 0; i < num_args; i++) {
			printf(" %8.8x", *((uint32*)buf + i));
		}
		printf("\n");
	}

	if (flag & EVENT_LOG_TAG_FLAG_LOG) {
		_EVENT_LOG_START(tag, 0xffff, num_args);
		memcpy((uint8 *)ts1->cur_ptr, (uint8*)buf, size);
		ts1->cur_ptr += num_args;
		_EVENT_LOG_END;
	}
}

/*
 * Some applications using event log may keep on writing data. If these filled buffers are
 * not flushed, they may overwrite their own data due to buffer pointer wraparound. The following
 * API can provide maximum size one shot write that can be done in the next write after calling
 * this API.
 */
uint16
event_log_get_available_space(int set)
{
	int available_space;
	event_log_block_t *cb;
	event_log_set_t *ts;
	uint32* cur_ptr;

	ASSERT(event_log_top->sets);

	if (set >= NUM_EVENT_LOG_SETS)
		return 0;
	ts = *(event_log_top->sets + set);

	if ((ts == NULL) || ((cb = ts->cur_block) == NULL))
		return 0;

	/* Counter mismatch, the buffer is yet to be flushed. So return 0. */
	if ((cb->count - ts->logtrace_count) >= ts->blockcount)
		return 0;

	/* The end_ptr points to the last word of an event log buffer. cur_ptr always points
	 * to the next empty word that needs to be written. The _EVENT_LOG_END_CHECK()
	 * checks if cur_ptr + (X number of words) >= end_ptr and makes a decision on moving to
	 * the next buffer. SO if this function give exactly X number of words as available space
	 * in current buffer, the check above may kick in and cause the current pointer to
	 * move to the buffer. In single buffer case, it will overwrite any counters that are
	 * already logged. To prevent this, this function returns smaller size
	 * so the caller of this funciton can decide what to do.
	 * the correct way would be to remove >= with > but doing that would cause
	 * 1K of ROM invalidation.
	 * if advertized size = X+1, and X words are to be written, cur_ptr would point to
	 * end_ptr after write is complete. The above macro will trigger a decision to move to
	 * next buffer.
	 * If advertized size = X and X words are to be written, cur_ptr would still point to
	 * end_ptr after write is complete. The above macro will still trigger a decision to
	 * move to next buffer.
	 * Advertize size smaller by 2 words to get past the above problem.
	 */
	available_space = cb->end_ptr - ts->cur_ptr - 1;

	/* Check if next block is free. logtrace points to the first log buffer ready
	 * for sending or being written to.
	 * Note: Assume all blocks in a log set are of the same size until a scheme to
	 * managing buffer sizes is properly decided.
	 */
	if (cb->next_block != ts->logtrace_block)
	{
		cb = cb->next_block;
		cur_ptr = &cb->event_logs;
		/* Next buffer is completely empty. The maximum size is the size of buffer. */
		available_space = (cb->end_ptr - cur_ptr - 1);
	}

	/* Take out space required for time sync. time sync size = 4 words */
	if (event_log_top->timestamp != ts->timestamp)
		available_space -= 4;

	/* The length field in event log record = 8 bits = 256 words.
	 * The length field = number of words ( 4 bytes each). If payload = 1024 bytes
	 * length = 256 words = 0 in the header. This is a problem.
	 * So valid length range = 1..255 words. 1 word is reserved for timestamp
	 * that brings valid range = 1..254 words. Note that min length of an event log
	 * record = 1 due to timestamp
	 * So 254 words of payload + 1 word for timestamp + 1 word for header = 256 words
	 * event log record size.
	 * length = 0 is not allowed.
	 */
	if (available_space >
		(EVENT_LOG_MAX_RECORD_PAYLOAD_SIZE + EVENT_LOG_RECORD_HDR_LEN +
		EVENT_LOG_RECORD_TIMESTAMP_LEN)) {
		available_space = EVENT_LOG_MAX_RECORD_PAYLOAD_SIZE;
	} else {
		/* event log record header size = 2 words. This includes 4 bytes of timestamp */
		available_space -= (EVENT_LOG_RECORD_HDR_LEN + EVENT_LOG_RECORD_TIMESTAMP_LEN);
	}

	/* Convert words to number of bytes */
	return (uint16)((available_space < 0) ? 0 : (available_space << 2));
}

int
event_log_flush_log_buffer(int set)
{
	int rc = BCME_OK;
	event_log_block_t *cb;
	event_log_set_t *ts = *(event_log_top->sets + set);

	if (set >= NUM_EVENT_LOG_SETS)
		return BCME_ERROR;

	/* Make sure that the block is not empty.
	 * Currently flush non empty buffers only.
	 * In case of single buffer, it is possible that log buffer may get full and cur_ptr
	 * may wrap around and point to the beginning. That may give an impression
	 * that the buffer is empty. Sure it is empty for the condition below but
	 * the wrap around action should have trigerred logtrace to pick this buffer.
	 */
	if ((ts != NULL) && ((cb = ts->cur_block) != NULL) &&
		(ts->cur_ptr != &cb->event_logs)) {
		_EVENT_LOG_END_CHECK(EVENT_LOG_MAX_BLOCK_SIZE);
		rc = BCME_OK;
	}
	else {
		/* This block cannot be flushed yet as it is not ready due to
		 * conditions above
		 */
		rc = BCME_NOTREADY;
	}

	/* return status to the caller */
	return rc;
}

bool
event_log_is_set_configured(int set_num)
{
	event_log_set_t *ts;

	if ((event_log_top == NULL) || (event_log_top->sets == NULL) ||
	    (set_num >= NUM_EVENT_LOG_SETS)) {
		return FALSE;
	}

	/* See if this set has been initialized and has blocks */
	ts = *(event_log_top->sets + set_num);
	if ((ts == NULL) || (ts->first_block == NULL)) {
		return FALSE;
	}

	return TRUE;
}

bool
event_log_is_tag_valid(int tag)
{
	return (tag != EVENT_LOG_TAG_NULL) &&
		(tag != EVENT_LOG_TAG_TS) &&
		(tag <= EVENT_LOG_TAG_MAX);
}

int
event_log_free_blocks_get(int set)
{
	event_log_block_t *cb;
	event_log_set_t *ts;

	ASSERT(event_log_top->sets);

	if (set >= NUM_EVENT_LOG_SETS)
		return 0;

	ts = *(event_log_top->sets + set);

	if ((ts == NULL) || ((cb = ts->cur_block) == NULL))
		return 0;

	/* Block is partially filled. Then there is no completely
	 * empty block available.
	 */
	if ((ts != NULL) && ((cb = ts->cur_block) != NULL) &&
		(ts->cur_ptr != &cb->event_logs)) {
		return 0;
	}

	/* Counter mismatch, the buffer is yet to be flushed. So return 0. */
	/* Logtrace count is lagging more than available number of blocks
	 * associated with this set => no buffer has been taken out yet.
	 * So return 0.
	 */

	return ((cb->count - ts->logtrace_count) >= ts->blockcount) ? 0 :
		(ts->blockcount - (cb->count - ts->logtrace_count));
}
#endif /* EVENT_LOG_COMPILE */
