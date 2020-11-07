/*
 * bcmseclib_timer.c -- timer library
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
 * $Id: bcmseclib_timer.c,v 1.7 2011-01-11 19:03:26 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <typedefs.h>
#include <bcm_osl.h>
#include <bcm_llist.h>
#include <bcmseclib_timer.h>
#include <bcmseclib_timer_os.h>
#include <debug.h>

/* Timer manager. */
struct bcmseclib_timer_mgr
{
	bcmseclib_timer_t *timerlist;
	bcmseclib_timer_t *timer_list_mark;
	bcmseclib_timer_t *timer_freelist;
	int maxtimers;
};

/* Global timer manager. */
static bcmseclib_timer_mgr_t *g_timer_mgr = NULL;

static int bcmseclib_set_expiration(uint time, exp_time_t * expiry_time);
static int bcmseclib_compare_times(exp_time_t *t1, exp_time_t *t2);
static bool bcmseclib_has_timer_expired(exp_time_t *t);
static void bcmseclib_translate_expiry_to_absolute(exp_time_t *t);

/* Init the timer library
 * Allocate memory, init free list
 * Return zero for success, non-zero otherwise
 */
int
bcmseclib_init_timer_utilities_ex(int ntimers, bcmseclib_timer_mgr_t **mgrp)
{
	int i;
	bcmseclib_timer_mgr_t *mgr;

	mgr = (bcmseclib_timer_mgr_t *) OS_MALLOC(sizeof(bcmseclib_timer_mgr_t));
	if (mgr == NULL) {
		return (-1);
	}
	memset(mgr, 0, sizeof(bcmseclib_timer_mgr_t));

	mgr->maxtimers = ntimers;
	mgr->timer_freelist = (bcmseclib_timer_t *) OS_MALLOC(ntimers * sizeof(bcmseclib_timer_t));
	memset(mgr->timer_freelist, 0, ntimers * sizeof(bcmseclib_timer_t));

	/* save, we'll need this to cleanup when we shutdown */
	mgr->timer_list_mark = mgr->timer_freelist;

	for (i = 0; i < (ntimers-1); i++)
		mgr->timer_freelist[i].next = &mgr->timer_freelist[i+1];

	mgr->timer_freelist[i].next = NULL;

	if (mgrp == NULL) {
		mgrp = &g_timer_mgr;
	}
	*mgrp = mgr;
	return 0;
}
/* Clean it up:
 * Free the allocated memory
 * All users of timer utilities should have freed their timers
 * via bcmseclib_free_timer() before this
 */
int
bcmseclib_deinit_timer_utilities_ex(bcmseclib_timer_mgr_t *mgr)
{
	if (mgr == NULL) {
		mgr = g_timer_mgr;
	}

	OS_FREE(mgr->timer_list_mark);
	OS_FREE(mgr);
	mgr = NULL;

	return 0;
}

/* Setup an existing timer with specified parms
 * and add it to the active list
 * If it's already on the active list relocate it
 * to the appropriate position
 */
void
bcmseclib_add_timer(bcmseclib_timer_t *t, uint ms, bool periodic)
{
	bcmseclib_timer_t *plist, *pprev;
	bcmseclib_timer_mgr_t *mgr;
	char *funstr = "bcmseclib_add_timer";

	(void) funstr;
	PRINT_TRACE(("%s(0x%x): duration %d periodic %d\n", funstr, (int)t->mgr, ms, periodic));

	mgr = t->mgr;

	/* If already in active list: remove it! */
	/* this call may fail: ok, just means t was not on list */
	bcm_llist_del_member(&mgr->timerlist, t);

	/* set time value and periodic flag */
	t->ms = ms;
	t->periodic = periodic;
	bcmseclib_set_expiration(ms, &t->expiry_time);

	/* place in correct position (by ascending expiry time) in active list */

	/* if list is NULL, we're the one & only */
	if (mgr->timerlist == NULL) {
		t->next = NULL;
		mgr->timerlist = t;
		goto done;
	}

	/* walk list until we find a time greater than ours, insert before that member */
	pprev = NULL;
	for (plist = mgr->timerlist; plist; ) {
		if (bcmseclib_compare_times(&t->expiry_time, &plist->expiry_time) <= 0) {
			t->next = plist;
			if (pprev == NULL)
				mgr->timerlist = t;
			else
				pprev->next = t;
			break;
		}
		pprev = plist;
		plist = plist->next;
	}

	/* end of list */
	if (plist == NULL) {
		pprev->next = t;
		t->next = NULL;
	}

done:
	return;
}

/* De-activate timer: remove from active list, but maintain settings,
 * application is maintaining ownership
 */
bool
bcmseclib_del_timer(bcmseclib_timer_t *t)
{
	bcmseclib_timer_mgr_t *mgr = t->mgr;

	/* Unlink from active list */
	/* this call may fail: ok, just means t was not on list */
	if (bcm_llist_del_member(&mgr->timerlist, t) == 0)
		return TRUE;

	return FALSE;
}

/* Application is surrendering this timer:
 * Remove from active list and clear all members
 */
void
bcmseclib_free_timer(bcmseclib_timer_t *t)
{
	bcmseclib_timer_mgr_t *mgr;

	if (t == NULL)
		return;

	mgr = t->mgr;

	/* unlink from active list if necessary */
	bcm_llist_del_member(&mgr->timerlist, t);

	/* place in free list */
	t->next = mgr->timer_freelist;
	mgr->timer_freelist = t;
}

/* Create the data structures, fill in callback args, but do NOT
 * add to active list
 */
bcmseclib_timer_t *
bcmseclib_init_timer_ex(bcmseclib_timer_mgr_t *mgr, void (*fn)(void *arg),
	void *arg, const char *name)
{
	bcmseclib_timer_t *pnew;
	char *funstr = "bcmseclib_init_timer";

	if (mgr == NULL) {
		mgr = g_timer_mgr;
	}

	/* Find a free timer, if none complain and return NULL */
	if (mgr->timer_freelist == NULL) {
		PRINT_ERR(("%s: No timer blocks availavble\n", funstr));
		return NULL;
	}

	pnew = mgr->timer_freelist;
	mgr->timer_freelist = pnew->next;
	pnew->next = NULL;

	/* Fill in cb fun, arg, name */
	ASSERT(fn);
	if (fn == NULL) {
		PRINT_ERR(("%s: NULL cb function arg!\n", funstr));
		return NULL;
	}
	pnew->fn = fn;
	pnew->arg = arg;
	pnew->mgr = mgr;

#ifdef BCMDBG
	if ((pnew->name = OS_MALLOC(strlen(name) + 1)))
		strcpy(pnew->name, name);
#endif // endif

	/* return pointer to timer */
	return pnew;
}

/* Check the active timer list
 * Return:
 * TRUE if we've got a timeout to consider, set value of t appropriately
 * FALSE otherwise, value of t irrelevant
 * Primary user should be the select/waitfor loop in the dispatcher.
 */
bool
bcmseclib_get_timeout_ex(bcmseclib_timer_mgr_t *mgr, exp_time_t *t)
{
	char *funstr = "bcmseclib_get_timeout";

	if (mgr == NULL) {
		mgr = g_timer_mgr;
	}

	/* no timers pending */
	if (mgr->timerlist == NULL)
		return FALSE;

	if (t == NULL) {
		PRINT_ERR(("%s: can't get timeout into null pointer\n", funstr));
		return FALSE;
	}

	memcpy(t, &mgr->timerlist->expiry_time, sizeof(exp_time_t));

	/* Translate expiry time which is a literal time of day to
	 * time remaining to expiry for use by select/waitfor functions
	 */
	bcmseclib_translate_expiry_to_absolute(t);

	return TRUE;
}

/* Check the timer list, process expirations */
void
bcmseclib_process_timer_expiry_ex(bcmseclib_timer_mgr_t *mgr)
{
	bcmseclib_timer_t *ptimer;
	char *funstr = "bcmseclib_process_timer_expiry";

	if (mgr == NULL) {
		mgr = g_timer_mgr;
	}

	ptimer = mgr->timerlist;
	if (ptimer == NULL)
		return;

	/* process all expired timers */
	for (; ptimer && bcmseclib_has_timer_expired(&ptimer->expiry_time);
		ptimer = mgr->timerlist) {
		/* Remove from the active list */
		/* Caution: this modifies timerlist: recheck against NULL afterwards */
		bcmseclib_del_timer(ptimer);

		/* add timer back if periodic */
		/* Caution: do this before the cb is fired in case it is delete
		 *          in the handler
		*/
		if (ptimer->periodic)
			bcmseclib_add_timer(ptimer, ptimer->ms, ptimer->periodic);

		/* exec cb function */
		if (!ptimer->fn) {
			PRINT_ERR(("%s: NULL cb function\n", funstr));
		} else {

			(*ptimer->fn)(ptimer->arg);
		}
	}
}

/* Theory of operation:
 * All timers that are added specify their expiration time in milliseconds (msec)
 * relative to now. This 'msec' expiration time is converted to an exp_time_t
 * structure that stores the expiry time relative to a fixed reference point.
 * The resulting expiration time can be compared to the current time (relative
 * to the fixed reference point) to determine if the timer has expired.
 */

/* Convert the expiration time (msec) to an exp_time_t structure that
 * stores the expiry time relative to a fixed reference point.
 */
static int
bcmseclib_set_expiration(uint msec, exp_time_t * expiry_time)
{
	bcmseclib_time_t now;
	int sec = msec/1000;
	int usec = (msec % 1000) * 1000;

	memset(&now, 0, sizeof(now));
	bcmseclib_os_get_time(&now);

	expiry_time->sec = now.sec + sec;
	expiry_time->usec = now.usec + usec;

	/* Handle overflow of usec. */
	while (expiry_time->usec >= 1000000) {
		expiry_time->sec += 1;
		expiry_time->usec -= 1000000;
	}

	return 0;
}

/* Return:
 * < 0 for t1 < t2
 *   0 for t1 == t2
 * > 0 for t1 > t2
 *
 * Where: "less than" means sooner, "greater than" means later
 */
static int
bcmseclib_compare_times(exp_time_t *t1, exp_time_t *t2)
{

	if (t1->sec > t2->sec)
		return 1;

	if (t1->sec < t2->sec)
		return -1;

	/* sec parts are equal */
	if (t1->usec > t2->usec)
		return 1;
	if (t1->usec < t2->usec)
		return -1;

	return 0;
}

/* Return:
 * TRUE if time has expired
 * FALSE otherwise
 * TODO: should we advance slightly, ie return TRUE if timer will
 * expire "soon"?
 */
static bool
bcmseclib_has_timer_expired(exp_time_t *t)
{
	bcmseclib_time_t now;

	memset(&now, 0, sizeof(now));
	bcmseclib_os_get_time(&now);

	if (bcmseclib_compare_times(&now, t) >= 0)
		return TRUE;

	return FALSE;
}

/* Input: time value in t which stores the expiry time relative to a fixed reference point.
 * Output: time remaining to expiration as absolute number of sec/usec relative to now.
 */
static void
bcmseclib_translate_expiry_to_absolute(exp_time_t *t)
{
	bcmseclib_time_t now;

	memset(&now, 0, sizeof(now));
	bcmseclib_os_get_time(&now);
	if (bcmseclib_compare_times(&now, t) >= 0) {
		t->sec = t->usec = 0;
		return;
	}

	PRINT_TRACE(("t->sec %ld t->usec %ld now.sec %ld now.usec %ld\n",
		t->sec, t->usec, now.sec, now.usec));

	t->sec -= now.sec;
	t->usec -= now.usec;

	/* Handle underflow. */
	if (t->usec < 0) {
		t->sec -= 1;
		t->usec += 1000000;
	}

	ASSERT(t->sec >= 0);
	ASSERT(t->usec >= 0);

}
