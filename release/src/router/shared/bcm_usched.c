/*
 * Broadcom micro scheduler library definitions
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcm_usched.c 647940 2016-07-08 10:38:09Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <bcmnvram.h>
#include "typedefs.h"
#include "bcmutils.h"
#include "bcm_usched.h"

#define USCHED_ERROR_PREFIX	"BCMUSCHED - "

/* This will be the default timeout for select call if there is no timers present */
#define USCHED_TIEMOUT_DEFAULT	5 /* In seconds */

/* If FD_COPY is not defined, use our own copy to copy the fd_set */
#ifndef FD_COPY
#define FD_COPY(src, dest) memcpy((dest), (src), sizeof(*(dest)))
#endif /* FD_COPY */

/* To convert microseconds to timeval struct */
#define USCHED_MICROSEC_TO_TIMEVAL(microsec, tv) (tv)->tv_sec = microsec / 1000000; \
	(tv)->tv_usec = microsec % 1000000;

/* To swap any basic data */
#define USCHED_SWAP(a, b, T) do { T t; t = a; a = b; b = t; } while (0)

/* Structure to hold one timer entry. Note change the bcm_usched_swap_timers_data function
 * If any changes in this structure
 */
typedef struct usched_timers {
	dll_t node;			/* dll_t struct node for doubly linked list */
	short int repeat_flag;		/* Timer should be repeated or not */
	short int remove_flag;		/* Flag to notify the process function to remove
		* the timer from list
		*/

	struct timeval timeout;		/* Next timeout */
	struct timeval interval;	/* Interval between timer wakeup */
	bcm_usched_timerscbfn *cbfn;	/* Call back function data passed by the user */
	void *arg;			/* Optional argument to be passed to call back function */
} usched_timers_t;

/* Structure to hold one FD entry */
typedef struct usched_fds {
	dll_t node;			/* dll_t struct node for doubly linked list */
	short int remove_flag;		/* Flag to notify the process function to remove the
		* scheduler from list
		*/
	int fd;				/* FD to be watched in select */
	int fdbits;			/* READ, WRITE or EXCEPTION to be watched */
	bcm_usched_fdscbfn *cbfn;	/* Users call back function */
	void *arg;			/* Optional argument to be passed to call back function */
} usched_fds_t;

/* Structure for micro scheduler handle */
typedef struct usched_handle {
	BCM_USCHED_STATUS status;	/* Status of the micro scheduler */
	int ref_count;			/* Number of handles created */
	int stop_schedule;		/* Flag to notify the process function return from run() */

	dll_t timers_list_head;		/* Head node of All Timers */
	dll_t fds_list_head;			/* Head node of All FD's */

	/* We are keeping copy of the fd_set because no need to calculate the fd_set for
	 * every select call as select call modifies the fd_set on return
	 */
	fd_set read_fds;		/* All the read FD's set */
	fd_set write_fds;		/* All the write FD's set */
	fd_set exc_fds;			/* All the Exception FD's set */

	int max_fd;			/* MAX of all the FD's */
} usched_handle_t;

/* For debug prints */
#define USCHED_DEBUG_ERROR		0x0001
#define USCHED_DEBUG_WARNING		0x0002
#define USCHED_DEBUG_INFO		0x0004
#define USCHED_DEBUG_DETAIL		0x0008

#define USCHED_DEFAULT_DEBUG_LEVEL	USCHED_DEBUG_ERROR

#define USCHED_PRINT(fmt, arg...) printf("USCHED >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg)

#define USCHED_ERROR(fmt, arg...) \
	do { if (g_bcm_usched_debug_level & USCHED_DEBUG_ERROR) \
		USCHED_PRINT(fmt, ##arg); } while (0)

#define USCHED_WARNING(fmt, arg...) \
	do { if (g_bcm_usched_debug_level & USCHED_DEBUG_WARNING) \
		USCHED_PRINT(fmt, ##arg); } while (0)

#define USCHED_INFO(fmt, arg...) \
	do { if (g_bcm_usched_debug_level & USCHED_DEBUG_INFO) \
		USCHED_PRINT(fmt, ##arg); } while (0)

#define USCHED_DEBUG(fmt, arg...) \
	do { if (g_bcm_usched_debug_level & USCHED_DEBUG_DETAIL) \
		USCHED_PRINT(fmt, ##arg); } while (0)

#define USCHED_PRINT_TIMEVAL(arg, tv) \
	USCHED_DEBUG("%s : {%d, %d}\n", arg, (int)(tv)->tv_sec, (int)(tv)->tv_usec);

unsigned int g_bcm_usched_debug_level;


/* Gets the config val from NVARM, if not found applies the default value */
uint16
bcm_usched_get_config_val_int(const char *c, uint16 def)
{
	uint16 ret = def;
#ifndef PCTARGET
	char *val;

	val = nvram_safe_get(c);
	if (val)
		ret = strtoul(val, NULL, 0);
#endif
	return ret;
}

/* Remove all the timers from the list */
static void
bcm_usched_remove_all_timers(usched_handle_t *hdl)
{
	dll_t *item_p, *next_p;

	for (item_p = dll_head_p(&hdl->timers_list_head); !dll_end(&hdl->timers_list_head, item_p);
		item_p = next_p) {
		usched_timers_t *tmp = (usched_timers_t*)item_p;

		USCHED_DEBUG("Removing timer arg[%p] cb[%p]\n", tmp->arg, tmp->cbfn);
		next_p = dll_next_p(item_p);
		dll_delete(item_p);
		free(((usched_timers_t*)item_p));
	}

	USCHED_DEBUG("Removed All timers\n");
}

/* Remove all the FD's from the list */
static void
bcm_usched_remove_all_fds(usched_handle_t *hdl)
{
	dll_t *item_p, *next_p;

	for (item_p = dll_head_p(&hdl->fds_list_head); !dll_end(&hdl->fds_list_head, item_p);
		item_p = next_p) {
		usched_fds_t *tmp = (usched_fds_t*)item_p;

		USCHED_DEBUG("Removing FD[%d]\n", tmp->fd);
		next_p = dll_next_p(item_p);
		dll_delete(item_p);
		free(((usched_fds_t*)item_p));
	}
	USCHED_DEBUG("Removed all FD's\n");
}

/* Check whether timer already exists in the list or not */
static int
bcm_usched_is_timer_exists(usched_handle_t *hdl, bcm_usched_timerscbfn *cbfn, void *arg)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hdl->timers_list_head); !dll_end(&hdl->timers_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		usched_timers_t *tmp = (usched_timers_t*)item_p;

		/* Check if the timer exists by comparing the argument address and callback address
		 * Also, dont return timer exists if the remove_flag is set to 1
		 */
		if ((tmp->remove_flag == 0) && (tmp->cbfn == cbfn) && (tmp->arg == arg)) {
			USCHED_DEBUG("Timer arg[%p] cb[%p] already exists\n", tmp->arg, tmp->cbfn);
			return BCM_USCHEDE_TIMER_EXISTS;
		}
	}

	return BCM_USCHEDE_OK;
}

/* Check whether FD already exists in the list or not */
static int
bcm_usched_is_fd_exists(usched_handle_t *hdl, int fd)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hdl->fds_list_head); !dll_end(&hdl->fds_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		usched_fds_t *tmp = (usched_fds_t*)item_p;

		/* Check if the FD exists by comparing the FD. Also, dont return FD exists if
		 * the remove_flag is set to 1
		 */
		if ((tmp->remove_flag == 0) && (tmp->fd == fd)) {
			USCHED_DEBUG("FD[%d] Already exists\n", tmp->fd);
			return BCM_USCHEDE_FD_EXISTS;
		}
	}

	return BCM_USCHEDE_OK;
}

/* Add the newly created node to the timers list. Add in a sorted manner depending on interval */
static int
bcm_usched_add_timer_node(usched_handle_t *hdl, usched_timers_t *new)
{
	dll_t *item_p, *prev = NULL;

	for (item_p = dll_head_p(&hdl->timers_list_head); !dll_end(&hdl->timers_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		/* CSTYLED */
		if (timercmp(&((usched_timers_t*)item_p)->timeout, &new->timeout, >)) {
			break;
		}
		prev = item_p;
	}

	if (prev == NULL) {
		dll_prepend(&hdl->timers_list_head, (dll_t*)new);
	} else {
		dll_insert((dll_t*)new, prev);
	}

	return BCM_USCHEDE_OK;
}

/* Add the newly created node to the fds list */
static int
bcm_usched_add_fd_node(usched_handle_t *hdl, usched_fds_t *new)
{
	dll_append(&hdl->fds_list_head, (dll_t*)new);

	return BCM_USCHEDE_OK;
}

/* This will reassign the FD sets of main handle also update the max_fd. This is required in
 * case of some FD's got deleted
 */
static void
bcm_usched_reassign_fdsets(usched_handle_t *hdl)
{
	dll_t *item_p;
	usched_fds_t *curr = NULL;

	/* Initialize the FD sets */
	FD_ZERO(&hdl->read_fds);
	FD_ZERO(&hdl->write_fds);
	FD_ZERO(&hdl->exc_fds);

	hdl->max_fd = -1;

	for (item_p = dll_head_p(&hdl->fds_list_head); !dll_end(&hdl->fds_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		curr = (usched_fds_t*)item_p;

		/* Set the FD sets of main handle based on the MASK(READ, WRITE or EXCEPTION)
		 * sent by user
		 */
		if (BCM_USCHED_ISFDMASKSET(curr->fdbits, BCM_USCHED_MASK_READFD))
			FD_SET(curr->fd, &hdl->read_fds);
		if (BCM_USCHED_ISFDMASKSET(curr->fdbits, BCM_USCHED_MASK_WRITEFD))
			FD_SET(curr->fd, &hdl->write_fds);
		if (BCM_USCHED_ISFDMASKSET(curr->fdbits, BCM_USCHED_MASK_EXCEPTIONFD))
			FD_SET(curr->fd, &hdl->exc_fds);

		/* Now update the max_fd */
		if (hdl->max_fd < curr->fd)
			hdl->max_fd = curr->fd;
	}

	USCHED_DEBUG("Reassigned the FD sets\n");
}

/* This will delete the timer node with remove_flag = 1 */
static int
bcm_usched_remove_unwanted_timers(usched_handle_t *hdl)
{
	dll_t *item_p, *next_p;
	int is_deleted = 0;

	for (item_p = dll_head_p(&hdl->timers_list_head); !dll_end(&hdl->timers_list_head, item_p);
		item_p = next_p) {
		usched_timers_t *tmp = (usched_timers_t*)item_p;

		next_p = dll_next_p(item_p);
		if (tmp->remove_flag) {
			USCHED_DEBUG("Removing timer arg[%p] cb[%p]\n", tmp->arg, tmp->cbfn);
			dll_delete(item_p);
			free(((usched_timers_t*)item_p));
			is_deleted = 1;
		}
	}

	return is_deleted;
}

/* This will delete the fd node with remove_flag = 1 */
static int
bcm_usched_remove_unwanted_fds(usched_handle_t *hdl)
{
	dll_t *item_p, *next_p;
	int is_deleted = 0;

	for (item_p = dll_head_p(&hdl->fds_list_head); !dll_end(&hdl->fds_list_head, item_p);
		item_p = next_p) {
		usched_fds_t *tmp = (usched_fds_t*)item_p;

		next_p = dll_next_p(item_p);
		if (tmp->remove_flag) {
			USCHED_DEBUG("Removing the FD[%d]\n", tmp->fd);
			dll_delete(item_p);
			free(((usched_fds_t*)item_p));
			is_deleted = 1;
		}
	}

	return is_deleted;
}

/* Swaps the data of two timer nodes Note: change this function if the usched_timers_t is changed */
static void
bcm_usched_swap_timers_data(usched_timers_t *a, usched_timers_t *b)
{
	USCHED_SWAP(a->repeat_flag, b->repeat_flag, short int);
	USCHED_SWAP(a->remove_flag, b->remove_flag, short int);

	USCHED_SWAP(a->timeout, b->timeout, struct timeval);
	USCHED_SWAP(a->interval, b->interval, struct timeval);

	USCHED_SWAP(a->arg, b->arg, void*);
	USCHED_SWAP(a->cbfn, b->cbfn, bcm_usched_timerscbfn*);
}

/* Sorts the timers linked list */
static void
bcm_usched_sort_timers_list(usched_handle_t *hdl)
{
	dll_t *item_p, *in_item_p;

	/* If there is no node or if there is only one node, don't sort */
	if ((dll_empty(&hdl->timers_list_head)) ||
		(dll_end(dll_head_p(&hdl->timers_list_head), dll_tail_p(&hdl->timers_list_head)))) {
		return;
	}

	for (item_p = dll_head_p(&hdl->timers_list_head); !dll_end(&hdl->timers_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		for (in_item_p = dll_next_p(item_p); !dll_end(&hdl->timers_list_head, in_item_p);
			in_item_p = dll_next_p(in_item_p)) {
			usched_timers_t *curr, *next;

			curr = (usched_timers_t*)item_p;
			next = (usched_timers_t*)in_item_p;
			/* CSTYLED */
			if (timercmp(&curr->timeout, &next->timeout, >)) {
				bcm_usched_swap_timers_data(curr, next);
			}
		}
	}
}

/* Get the timeout for select call. This calculates the timeout from timers */
static BCM_USCHED_STATUS
bcm_usched_calculate_timeout(usched_handle_t *hdl, struct timeval *tv)
{
	struct timeval now;
	dll_t *item_p;
	usched_timers_t *firsttimer = NULL;

	/* If there is no timers use default time */
	if (dll_empty(&hdl->timers_list_head)) {
		timerclear(tv);
		tv->tv_sec = USCHED_TIEMOUT_DEFAULT;
		return BCM_USCHEDE_OK;
	}

	/* Get the current time */
	gettimeofday(&now, NULL);

	item_p = dll_head_p(&hdl->timers_list_head);
	firsttimer = (usched_timers_t*)item_p;
	/* If the timeout is already expired, set timeout as zero so that select will return
	 * immediately
	 */
	/* CSTYLED */
	if (!timercmp(&firsttimer->timeout, &now, >)) {
		timerclear(tv);
		return BCM_USCHEDE_OK;
	}

	/* Substract the current time from timeout to get the next timeout for select call */
	timersub(&firsttimer->timeout, &now, tv);

	return BCM_USCHEDE_OK;
}

/* Process all the timers */
static void
bcm_usched_process_timers(usched_handle_t *hdl)
{
	dll_t *item_p;
	struct timeval curr_time;
	int is_changed = 0, is_deleted = 0;

	gettimeofday(&curr_time, NULL);

	for (item_p = dll_head_p(&hdl->timers_list_head);
		!hdl->stop_schedule && !dll_end(&hdl->timers_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		usched_timers_t *curr = (usched_timers_t*)item_p;

		/* If remove flag is set don't process */
		if (curr->remove_flag) {
			USCHED_DEBUG("Remove_flag is set arg[%p] cb[%p]. So dont process\n",
				curr->arg, curr->cbfn);
			continue;
		}

		/* As all the timers in sorted order, if we find any timeout greater than
		 * curr_time break the loop
		 */
		/* CSTYLED */
		if (timercmp(&curr->timeout, &curr_time, >))
			break;

		curr->cbfn(hdl, curr->arg);
		is_changed = 1;

		/* If repeat_flag is set update the timeout for next callback */
		if (curr->repeat_flag) {
			timeradd(&curr_time, &curr->interval, &curr->timeout);
		} else {
			/* Update the remove_flag to 1 to indicate for deletion */
			curr->remove_flag = 1;
			USCHED_DEBUG("Timer not repetitive Setting remove_flag arg[%p] cb[%p]\n",
				curr->arg, curr->cbfn);
		}
	}

	/* Delete some unwanted timers with repeat_flag 1 */
	is_deleted = bcm_usched_remove_unwanted_timers(hdl);

	/* If the callback is called or any timer is deleted sort the list */
	if (is_changed || is_deleted)
		bcm_usched_sort_timers_list(hdl);
}

/* Checks whether the FD is set or not */
static int
bcm_usched_isfdset(usched_handle_t *hdl, usched_fds_t *curr, fd_set *fdset, int bit)
{
	if (BCM_USCHED_ISFDMASKSET(curr->fdbits, bit)) {
		if (FD_ISSET(curr->fd, fdset)) {
			/* Call the registered callback */
			bcm_usched_fds_entry_t fdentry;

			memset(&fdentry, 0, sizeof(fdentry));
			fdentry.fd = curr->fd;
			BCM_USCHED_SETFDMASK(fdentry.fdbits, bit);
			curr->cbfn(hdl, curr->arg, &fdentry);
			return 1;
		}
	}

	return 0;
}

/* Checks whether the FD is set or not for all in the list */
static void
bcm_usched_checkfdset(usched_handle_t *hdl, fd_set *read_fds, fd_set *write_fds, fd_set *exc_fds)
{
	dll_t *item_p;
	usched_fds_t *curr = NULL;

	for (item_p = dll_head_p(&hdl->fds_list_head);
		!hdl->stop_schedule && !dll_end(&hdl->fds_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		int found = 0;
		curr = (usched_fds_t*)item_p;

		/* If remove flag is set don't process */
		if (curr->remove_flag) {
			USCHED_DEBUG("Remove_flag is set fd[%d]. So dont process\n", curr->fd);
			continue;
		}

		/* Check for read, write and exception bits to call the callback */
		found = bcm_usched_isfdset(hdl, curr, read_fds, BCM_USCHED_MASK_READFD);
		if (!found) {
			found = bcm_usched_isfdset(hdl, curr, write_fds,
				BCM_USCHED_MASK_WRITEFD);
		}
		if (!found) {
			found = bcm_usched_isfdset(hdl, curr, exc_fds,
				BCM_USCHED_MASK_EXCEPTIONFD);
		}
	}
}

/* Processes timers and FD's in the list */
static void
bcm_usched_process_fds_and_timers(usched_handle_t *hdl)
{
	int status, is_fd_removed = 0;
	struct timeval tv;
	fd_set read_fds, write_fds, exc_fds;

	/* Get the timeout for the select from the timers timeout */
	bcm_usched_calculate_timeout(hdl, &tv);
	USCHED_PRINT_TIMEVAL("Timeout for select", &tv);
	/* If there is no timeout set, then its time to process timers */
	if (!timerisset(&tv)) {
		/* Now check for the timers which are expired */
		bcm_usched_process_timers(hdl);
		return;
	}

	/* Initialize the FD sets */
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&exc_fds);

	/* Now copy the FD sets from handle */
	FD_COPY(&hdl->read_fds, &read_fds);
	FD_COPY(&hdl->write_fds, &write_fds);
	FD_COPY(&hdl->exc_fds, &exc_fds);

	/* listen to data available on all FD's */
	status = select(hdl->max_fd+1, &read_fds, &write_fds, &exc_fds, &tv);
	if (status == -1) {
		if (errno != EINTR) {
			/* No event */
			USCHED_WARNING("Error from select : %s\n", strerror(errno));
		}
		return;
	}

	if (status > 0) {
		/* Now check which FD is set */
		bcm_usched_checkfdset(hdl, &read_fds, &write_fds, &exc_fds);
	}

	/* Now check for the timers which are expired */
	bcm_usched_process_timers(hdl);

	/* Now delete unwanted fds */
	is_fd_removed = bcm_usched_remove_unwanted_fds(hdl);

	/* If any of the FD is removed recalculate the fd_set's */
	if (is_fd_removed)
		bcm_usched_reassign_fdsets(hdl);
}

/* Initializes the library */
bcm_usched_handle*
bcm_usched_init()
{
	static usched_handle_t *usched_new_hdl = NULL;

	if (usched_new_hdl != NULL) {
		usched_new_hdl->ref_count++;
		USCHED_DEBUG("USCHED is already initialized. Increasing the reference count[%d]\n",
			usched_new_hdl->ref_count);
		return usched_new_hdl;
	}

	g_bcm_usched_debug_level = bcm_usched_get_config_val_int("bcm_usched_debug_level",
		USCHED_DEFAULT_DEBUG_LEVEL);

	/* Allocate handle */
	usched_new_hdl = (usched_handle_t*)malloc(sizeof(*usched_new_hdl));
	if (usched_new_hdl == NULL) {
		USCHED_ERROR("Failed to allocate memory\n");
		return NULL;
	}

	memset(usched_new_hdl, 0, sizeof(*usched_new_hdl));

	usched_new_hdl->max_fd = -1;
	usched_new_hdl->ref_count = 1;
	usched_new_hdl->status = BCM_USCHED_INITIALIZED;
	dll_init(&usched_new_hdl->timers_list_head);
	dll_init(&usched_new_hdl->fds_list_head);

	return (bcm_usched_handle*)usched_new_hdl;
}

/* DeInitializes the library */
BCM_USCHED_STATUS
bcm_usched_deinit(bcm_usched_handle *handle)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	/* Now check the reference count. If the other module is using this handle dont delete */
	if ((--hdl->ref_count) > 0) {
		USCHED_DEBUG("%d Modules are using\n", hdl->ref_count);
		return BCM_USCHEDE_OK;
	}

	/* First remove all the timers */
	bcm_usched_remove_all_timers(hdl);

	/* Remove all the FD's */
	bcm_usched_remove_all_fds(hdl);

	/* Free handle */
	free(hdl);
	hdl = NULL;

	USCHED_DEBUG("Deinitialized successfully\n");

	return BCM_USCHEDE_OK;
}

/* create the new timer */
BCM_USCHED_STATUS
bcm_usched_add_timer(bcm_usched_handle *hdl, unsigned long interval, short int repeat_flag,
	bcm_usched_timerscbfn *cbfn, void *arg)
{
	usched_timers_t *new = NULL;
	struct timeval curr_time;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	if (bcm_usched_is_timer_exists((usched_handle_t*)hdl, cbfn, arg) != BCM_USCHEDE_OK)
		return BCM_USCHEDE_TIMER_EXISTS;

	/* Allocate Timer */
	new = (usched_timers_t*)malloc(sizeof(*new));
	if (new == NULL) {
		USCHED_ERROR("Failed to allocate memory\n");
		return BCM_USCHEDE_MEMORY;
	}

	memset(new, 0, sizeof(*new));

	/* Convert interval to timeval */
	USCHED_MICROSEC_TO_TIMEVAL(interval, &new->interval);
	USCHED_PRINT_TIMEVAL("new->interval", &new->interval);

	/* Now get the timeout by adding curr_time and interval */
	gettimeofday(&curr_time, NULL);
	timeradd(&curr_time, &new->interval, &new->timeout);
	USCHED_PRINT_TIMEVAL("curr_time", &curr_time);
	USCHED_PRINT_TIMEVAL("new->timeout", &new->timeout);

	new->repeat_flag = repeat_flag;
	new->remove_flag = 0;
	new->arg = arg;
	new->cbfn = cbfn;

	return bcm_usched_add_timer_node((usched_handle_t*)hdl, new);
}

/* create the new FD scheduler */
BCM_USCHED_STATUS
bcm_usched_add_fd_schedule(bcm_usched_handle *handle, int fd, int fdbits,
	bcm_usched_fdscbfn *cbfn, void *arg)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;
	usched_fds_t *new = NULL;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	if (bcm_usched_is_fd_exists((usched_handle_t*)hdl, fd) != BCM_USCHEDE_OK)
		return BCM_USCHEDE_FD_EXISTS;

	/* Allocate for FD */
	new = (usched_fds_t*)malloc(sizeof(*new));
	if (new == NULL) {
		USCHED_ERROR("Failed to allocate memory\n");
		return BCM_USCHEDE_MEMORY;
	}

	new->remove_flag = 0;
	new->fd = fd;
	new->fdbits = fdbits;
	new->arg = arg;
	new->cbfn = cbfn;

	/* Now set the FD sets of main handle based on the MASK(READ, WRITE or EXCEPTION)
	 * sent by user
	 */
	if (BCM_USCHED_ISFDMASKSET(fdbits, BCM_USCHED_MASK_READFD))
		FD_SET(fd, &hdl->read_fds);
	if (BCM_USCHED_ISFDMASKSET(fdbits, BCM_USCHED_MASK_WRITEFD))
		FD_SET(fd, &hdl->write_fds);
	if (BCM_USCHED_ISFDMASKSET(fdbits, BCM_USCHED_MASK_EXCEPTIONFD))
		FD_SET(fd, &hdl->exc_fds);

	/* Now update the max_fd */
	if (hdl->max_fd < fd)
		hdl->max_fd = fd;

	return bcm_usched_add_fd_node((usched_handle_t*)hdl, new);
}

/* Remove an timer from the list of timers provided the timer ID */
BCM_USCHED_STATUS
bcm_usched_remove_timer(bcm_usched_handle *handle, bcm_usched_timerscbfn *cbfn, void *arg)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;
	dll_t *item_p;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	/* Check the timers */
	if (dll_empty(&hdl->timers_list_head)) {
		USCHED_WARNING("No Timers to delete\n");
		return BCM_USCHEDE_NO_TIMERS;
	}

	/* Just update the remove flag. Actual removal is handled in process_timers */
	for (item_p = dll_head_p(&hdl->timers_list_head); !dll_end(&hdl->timers_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		usched_timers_t *tmp = (usched_timers_t*)item_p;

		if ((tmp->cbfn == cbfn) && (tmp->arg == arg)) {
			tmp->remove_flag = 1;
			USCHED_DEBUG("Setting remove flag for timer arg[%p] cb[%p]\n",
				tmp->arg, tmp->cbfn);
			return BCM_USCHEDE_OK;
		}
	}

	return BCM_USCHEDE_NOT_FOUND;
}

/* Remove an FD from the list of FD's provided the schedule ID */
BCM_USCHED_STATUS
bcm_usched_remove_fd_schedule(bcm_usched_handle *handle, int fd)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;
	dll_t *item_p;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	/* Check the FD's */
	if (dll_empty(&hdl->fds_list_head)) {
		USCHED_WARNING("No FD's Found\n");
		return BCM_USCHEDE_NO_FDS;
	}

	/* Just update the remove flag. Actual removal is handled in process_fds */
	for (item_p = dll_head_p(&hdl->fds_list_head); !dll_end(&hdl->fds_list_head, item_p);
		item_p = dll_next_p(item_p)) {
		usched_fds_t *tmp = (usched_fds_t*)item_p;

		if (tmp->fd == fd) {
			tmp->remove_flag = 1;
			USCHED_DEBUG("Setting remove flag for FD[%d]\n", tmp->fd);
			return BCM_USCHEDE_OK;
		}
	}

	return BCM_USCHEDE_NOT_FOUND;
}

/* Main loop which keeps on checking for the timers and fd's */
BCM_USCHED_STATUS
bcm_usched_run(bcm_usched_handle *handle)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	hdl->status = BCM_USCHED_RUNNING;
	/* loop through to check the timer and fd's */
	while (!hdl->stop_schedule) {
		/* If there is no timers or fds to process return */
		if ((dll_empty(&hdl->timers_list_head)) && (dll_empty(&hdl->fds_list_head))) {
			USCHED_ERROR("No item to schedule so stopping...\n");
			return BCM_USCHEDE_SCHEDULER_EMPTY;
		}
		bcm_usched_process_fds_and_timers(hdl);
	}
	hdl->status = BCM_USCHED_STOPPED;
	USCHED_DEBUG("Micro scheduler stopped\n");

	return BCM_USCHEDE_OK;
}

/* Stop the scheduler */
BCM_USCHED_STATUS
bcm_usched_stop(bcm_usched_handle *handle)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}

	/* If other modules are using it don't stop */
	if (hdl->ref_count > 1) {
		USCHED_DEBUG("%d Modules are using\n", hdl->ref_count);
		return BCM_USCHEDE_OK;
	}

	/* Just update the flag to stop the main loop */
	hdl->stop_schedule = 1;
	USCHED_DEBUG("Stopping the scheduler\n");

	return BCM_USCHEDE_OK;
}

/* Is scheduler stopped or not */
BCM_USCHED_STATUS
bcm_usched_get_status(bcm_usched_handle *handle)
{
	usched_handle_t *hdl = (usched_handle_t*)handle;

	/* Check the handle */
	if (!hdl) {
		USCHED_ERROR("Invalid Handle\n");
		return BCM_USCHEDE_INV_HDL;
	}
	USCHED_DEBUG("Status of the scheduler : %d\n", hdl->status);

	return hdl->status;
}

/* Return error string for the error code */
const char*
bcm_usched_strerror(BCM_USCHED_STATUS errorcode)
{
	switch (errorcode) {
		case BCM_USCHEDE_OK:
			return USCHED_ERROR_PREFIX"Success";

		case BCM_USCHEDE_FAIL:
			return USCHED_ERROR_PREFIX"Failed";

		case BCM_USCHEDE_INV_HDL:
			return USCHED_ERROR_PREFIX"Invalid USCHED Handle";

		case BCM_USCHEDE_MEMORY:
			return USCHED_ERROR_PREFIX"Memory Allocation Failed";

		case BCM_USCHEDE_SCHEDULER_EMPTY:
			return USCHED_ERROR_PREFIX"Scheduler is Empty";

		case BCM_USCHEDE_NO_TIMERS:
			return USCHED_ERROR_PREFIX"No Timers Present";

		case BCM_USCHEDE_NO_FDS:
			return USCHED_ERROR_PREFIX"No FDs Present";

		case BCM_USCHEDE_TIMER_EXISTS:
			return USCHED_ERROR_PREFIX"Timer Already Exists";

		case BCM_USCHEDE_FD_EXISTS:
			return USCHED_ERROR_PREFIX"FD Already Exists";

		case BCM_USCHEDE_NOT_FOUND:
			return USCHED_ERROR_PREFIX"Not Found";

		default:
			return USCHED_ERROR_PREFIX"Unknown Error";
	}
}
