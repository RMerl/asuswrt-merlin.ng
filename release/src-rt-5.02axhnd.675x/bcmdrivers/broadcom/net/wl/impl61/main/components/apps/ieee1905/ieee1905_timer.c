/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

/*
 * Timers
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "ieee1905_timer.h"
#include "ieee1905_linkedlist.h"
#include "ieee1905_trace.h"
#if defined(USEBCMUSCHED)
#include "bcm_usched.h"
#endif /* USEBCMUSCHED */

#define I5_TRACE_MODULE i5TraceTimer

timer_elem_type i5_timer_list = {};

#if defined(USEBCMUSCHED)
/* Callback function called fm scheduler lib on timer expiry */
static void
i5CommonTimerCallback(bcm_usched_handle *hdl, void *arg)
{
	timer_elem_type *ptmr;

	ptmr = (timer_elem_type*)arg;
	ptmr->process(ptmr->arg);
	if (i5DmDeviceTopologyChangeProcess(&i5_config.i5_mac_address[0])) {
    i5MessageTopologyNotificationSend(NULL, NULL, 0);
  }
}

/* Add timers to micro scheduler */
static int
i5AddTimerToSchedule(bcm_usched_handle *hdl, void *arg, unsigned long timeout,
	bcm_usched_timerscbfn *cbfn,
	int repeat_flag)
{
	BCM_USCHED_STATUS status = 0;

	status = bcm_usched_add_timer(hdl, (I5_MSEC_USEC((unsigned long long)timeout)), repeat_flag, cbfn, arg);
	if (status != BCM_USCHEDE_OK) {
		i5TraceError("Timeout[%ld]Msec arg[%p] cbfn[%p] Failed to add Timer. Error : %s\n",
			timeout, arg, cbfn, bcm_usched_strerror(status));
		goto end;
	}

end:
	return status;
}
#endif /* USEBCMUSCHED */

timer_elem_type *i5TimerNew(int msecs, void (*process)(void *arg), void *arg)
{
  struct timespec now_ts;
  timer_elem_type *ptmr;
  timer_elem_type *newtmr;
  struct timeval  now_tv, add_tv;

  newtmr = (timer_elem_type *)malloc(sizeof(timer_elem_type));
  if (newtmr != NULL) {
#if defined(USEBCMUSCHED)
    if (i5AddTimerToSchedule(i5_config.usched_hdl, newtmr, msecs, i5CommonTimerCallback, 1)
      != BCM_USCHEDE_OK) {
      free(newtmr);
      return NULL;
    }
#endif /* USEBCMUSCHED */

    add_tv.tv_sec = msecs/1000;
    add_tv.tv_usec = (msecs%1000)*1000;

    clock_gettime(CLOCK_MONOTONIC, &now_ts);
    now_tv.tv_sec = now_ts.tv_sec;
    now_tv.tv_usec = now_ts.tv_nsec/1000;

    timeradd(&now_tv, &add_tv, &newtmr->tv);
    newtmr->process = process;
    newtmr->arg = arg;

    ptmr = &i5_timer_list;
    while ((ptmr->ll.next != NULL) && timercmp(&((timer_elem_type *)(ptmr->ll.next))->tv, &newtmr->tv, <)) {
      ptmr = ptmr->ll.next;
    }
    i5LlItemAdd(NULL, ptmr, newtmr);
  }
  return newtmr;
}

int i5TimerFree(timer_elem_type *ptmr)
{
  timer_elem_type *prevItem;

  if (!ptmr) {
    return -1;
  }

#if defined(USEBCMUSCHED)
  bcm_usched_remove_timer(i5_config.usched_hdl, i5CommonTimerCallback, ptmr);
#endif /* USEBCMUSCHED */

  prevItem = &i5_timer_list;
  while ((prevItem->ll.next != NULL) && ((timer_elem_type *)(prevItem->ll.next) != ptmr)) {
    prevItem = prevItem->ll.next;
  }
  if (prevItem->ll.next != NULL) {
      return i5LlItemFree(prevItem, prevItem->ll.next);
  } else {
      i5TraceError("could not delete item (%p) from list...\n", ptmr);
      return -1;
  }
}

struct timeval *i5TimerExpires(struct timeval *ptv) {
  struct timespec now_ts;
  struct timeval  now_tv;
  timer_elem_type *ptmr = i5_timer_list.ll.next;

  while (ptmr != NULL) {
    clock_gettime(CLOCK_MONOTONIC, &now_ts);
    now_tv.tv_sec = now_ts.tv_sec;
    now_tv.tv_usec = now_ts.tv_nsec/1000;

    if (timercmp(&ptmr->tv, &now_tv, <)) {
      /* Expired */
      (ptmr->process)(ptmr->arg);
      ptmr = i5_timer_list.ll.next;
    } else {
      timersub(&ptmr->tv, &now_tv, ptv);
      return ptv;
    }
  }
  /* No timeout */
  return NULL;
}
