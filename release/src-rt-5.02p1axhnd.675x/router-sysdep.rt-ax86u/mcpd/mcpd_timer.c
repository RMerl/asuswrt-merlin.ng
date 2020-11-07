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
#include <unistd.h>
#include <sys/time.h>
#include "mcpd_timer.h"
#include "mcpd.h"

/* Internal Variables */
mcpd_timer_elem_type mcpd_timer_list = {};

void mcpd_timer_ListItemAdd(void *parent, void *list, void *child)
{
  mcpd_timer_listitem *childitem = (mcpd_timer_listitem *)child;
  mcpd_timer_listitem *listitem = (mcpd_timer_listitem *)list;

  childitem->parent = (mcpd_timer_listitem *)parent;
  childitem->next = listitem->next;
  listitem->next = childitem;

  return;
}

int mcpd_timer_ListItemRemove(void *list, void *child)
{
  mcpd_timer_listitem *childitem = (mcpd_timer_listitem *)child;
  mcpd_timer_listitem *listitem = (mcpd_timer_listitem *)list;

  while ((listitem != NULL) && (listitem->next != child)) {
    listitem = listitem->next;
  }
  if ((listitem != NULL) && (listitem->next == child)) {
    listitem->next = childitem->next;
    return 0;
  }
  return -1;
}

int mcpd_timer_ListItemFree(void *list, void *child)
{
  if (mcpd_timer_ListItemRemove(list, child) == 0) {
    free(child);
    return 0;
  }
  return -1;
}

void mcpd_timer_ListSearchItemPush(mcpd_timer_search_item_type *search_list, void *item)
{
  mcpd_timer_search_item_type *search_item;
  
  search_item = (mcpd_timer_search_item_type *)malloc(sizeof(mcpd_timer_search_item_type));
  search_item->item = (void *) item;
  mcpd_timer_ListItemAdd(NULL, search_list, search_item);
}

void *mcpd_timer_ListSearchItemPop(mcpd_timer_search_item_type *search_list)
{
  mcpd_timer_search_item_type *search_item;
  void *item = NULL;

  search_item = (mcpd_timer_search_item_type *)search_list->ll.next;
  if (search_item) {
    item = search_item->item;
    if (mcpd_timer_ListItemFree(search_list, search_item) == 0) {
      free(search_item);
    }
  }
  return item;
}

#if 0
static inline void mcpd_timer_Dump()
{
    if (i5TraceGet(i5TraceTimer) >= i5TraceLevelInfo) {
        timer_elem_type *ptmr = mcpd_timer_list.ll.next;
        while(ptmr != NULL){
            printf("[%p %p(%p) %ld.%04ld]->", ptmr, ptmr->process, ptmr->arg, ptmr->tv.tv_sec, ptmr->tv.tv_usec/1000);
            ptmr = ptmr->ll.next;
        }
        printf("\n");
    }
}
#endif

void mcpd_timer_cleanup()
{
  mcpd_timer_elem_type *ptmr;

  ptmr = &mcpd_timer_list;
  while (ptmr->ll.next != NULL) {
    mcpd_timer_ListItemFree(ptmr, ptmr->ll.next);
  }
}

mcpd_timer_elem_type *mcpd_timer_new(int msecs, McpdEventHandler process, void *arg)
{
  struct timespec now_ts;
  mcpd_timer_elem_type *ptmr;
  mcpd_timer_elem_type *newtmr = (mcpd_timer_elem_type *)malloc(sizeof(mcpd_timer_elem_type));
  struct timeval  now_tv, add_tv;

  if (newtmr != NULL) {
    add_tv.tv_sec = msecs/1000;
    add_tv.tv_usec = (msecs%1000)*1000;

    clock_gettime(CLOCK_MONOTONIC, &now_ts);
    now_tv.tv_sec = now_ts.tv_sec;
    now_tv.tv_usec = now_ts.tv_nsec/1000;

    timeradd(&now_tv, &add_tv, &newtmr->tv);
    newtmr->process = process;
    //if ( NULL == arg) {
    //  newtmr->arg = newtmr;
    //}
    //else {
      newtmr->arg = arg;
    //}

    ptmr = &mcpd_timer_list;
    while ((ptmr->ll.next != NULL) && timercmp(&((mcpd_timer_elem_type *)(ptmr->ll.next))->tv, &newtmr->tv, <)) {
      ptmr = ptmr->ll.next;
    }
    mcpd_timer_ListItemAdd(NULL, ptmr, newtmr);
  }
  return newtmr;
}

mcpd_timer_elem_type *mcpd_get_timer(McpdEventHandler process, void *arg)
{
  mcpd_timer_elem_type *ptmr;

  ptmr = &mcpd_timer_list;
  while (ptmr->ll.next != NULL) {
    mcpd_timer_elem_type *currTimer = (mcpd_timer_elem_type*)ptmr->ll.next;
    if ((currTimer->process == process) && (currTimer->arg == arg)) {
      return currTimer;
    }
    ptmr = ptmr->ll.next;
  }
  return NULL;
}

int mcpd_timer_free(mcpd_timer_elem_type *ptmr)
{
  mcpd_timer_elem_type *prevItem;

  prevItem = &mcpd_timer_list;  
  while ((prevItem->ll.next != NULL) && ((mcpd_timer_elem_type *)(prevItem->ll.next) != ptmr)) {
    prevItem = prevItem->ll.next;
  }
  if (prevItem->ll.next != NULL) {
      return mcpd_timer_ListItemFree(prevItem, prevItem->ll.next);
  } else {
      return -1;
  }  
}

void mcpd_timer_cancel(McpdEventHandler process, void *arg)
{
  mcpd_timer_elem_type *timer = mcpd_get_timer(process, arg);
  if (timer) {
    mcpd_timer_free(timer);
  }
}

void mcpd_igmp_v2_bckcomp_tmr(void *handle);
void mcpd_igmp_v1_bckcomp_tmr(void *handle);
void mcpd_igmp_timer_reporter(void *handle);
void mcpd_igmp_timer_source(void *handle);
void mpcd_igmpv2_last_member_query_tmr(void *handle);
void mcpd_process_query_timer(void *handle);
void mcpd_mld_v1_bckcomp_tmr(void *handle);
void mcpd_mld_timer_reporter(void *handle);
void mcpd_mld_timer_source(void *handle);
void mpcd_mld_last_member_query_tmr(void *handle);
void mcpd_igmp_timer_group(void *handle);
void mcpd_mld_timer_group(void *handle);

char* mcpd_timer_getProcName(McpdEventHandler process)
{
    if (process == mcpd_igmp_v2_bckcomp_tmr)
      return "mcpd_igmp_v2_bckcomp_tmr";
    if (process ==  mcpd_igmp_v1_bckcomp_tmr)
      return "mcpd_igmp_v1_bckcomp_tmr";
    if (process ==  mcpd_igmp_timer_reporter)
      return "mcpd_igmp_timer_reporter";
    if (process ==  mcpd_igmp_timer_source)
      return "mcpd_igmp_timer_source";
    if (process ==  mpcd_igmpv2_last_member_query_tmr)
      return "mpcd_igmpv2_last_member_query_tmr";
    if (process ==  mcpd_process_query_timer)
      return "mcpd_process_query_timer";
    if (process ==  mcpd_igmp_timer_group)
      return "mcpd_igmp_timer_group";
#if 0    
    if (process ==  mcpd_mld_v1_bckcomp_tmr)
      return "mcpd_mld_v1_bckcomp_tmr";
    if (process ==  mcpd_mld_timer_reporter)
      return "mcpd_mld_timer_reporter";
    if (process ==  mcpd_mld_timer_source)
      return "mcpd_mld_timer_source";
    if (process ==  mpcd_mld_last_member_query_tmr)
      return "mpcd_mld_last_member_query_tmr";
    if (process ==  mcpd_mld_timer_group)
      return "mcpd_mld_timer_group";
#endif
  
  return "Unknown MCPD Timer";
}



void mcpd_timer_expires()
{
  struct timespec now_ts;
  struct timeval  now_tv;
  mcpd_timer_elem_type *ptmr;

  clock_gettime(CLOCK_MONOTONIC, &now_ts);
  now_tv.tv_sec = now_ts.tv_sec;
  now_tv.tv_usec = now_ts.tv_nsec/1000;
  
  ptmr = mcpd_timer_list.ll.next;  
  while (ptmr != NULL) {
    if (timercmp(&ptmr->tv, &now_tv, <)) {
      mcpd_timer_elem_process process = ptmr->process;
      void *arg = ptmr->arg;
      MCPD_TRACE(MCPD_TRC_LOG, "Expiring: (%s)", mcpd_timer_getProcName(process) );
      /* free the timer first to allow the call to 'process'
         to add the same timer */
      mcpd_timer_free(ptmr);
      /* Expired */
      (process)(arg);
      ptmr = mcpd_timer_list.ll.next;
    }
    else {
      ptmr = (mcpd_timer_elem_type*)ptmr->ll.next;
    }
  }
}

struct timeval *mcpd_timer_timeToNextEvent_msec(struct timeval *tv)
{
  struct timespec now_ts;
  struct timeval  now_tv;
  mcpd_timer_elem_type *earliestItem;

  earliestItem = mcpd_timer_list.ll.next;  
  if (NULL == earliestItem) {
    /* no timers set - return NULL to indicate indefinite */
    return NULL;
  }
  
  clock_gettime(CLOCK_MONOTONIC, &now_ts);
  now_tv.tv_sec = now_ts.tv_sec;
  now_tv.tv_usec = now_ts.tv_nsec/1000;

  if (timercmp(&earliestItem->tv, &now_tv, <))
  {
    /* timer has already expired */
    tv->tv_sec = 0;
    tv->tv_usec = 10000;
  }
  else
  {
    timersub(&earliestItem->tv, &now_tv, tv);
  }
  return tv;
}

void mcpd_timer_dumpEvents()
{
  mcpd_timer_elem_type *ptmr;

  ptmr = &mcpd_timer_list;
  printf("MCPD Timer List\n");
  while (ptmr->ll.next != NULL) {
    mcpd_timer_elem_type *currTimer = (mcpd_timer_elem_type*)ptmr->ll.next;
    printf("(%s) %ld s %ld us\n", mcpd_timer_getProcName(currTimer->process),
                                      currTimer->tv.tv_sec, currTimer->tv.tv_usec);
    ptmr = ptmr->ll.next;
  }
}

unsigned int mcpd_timer_getTimeRemaining_msec(McpdEventHandler process, void *arg)
{
  struct timespec now_ts;
  struct timeval  now_tv;
  struct timeval sub_tv;
  mcpd_timer_elem_type *timer = mcpd_get_timer(process, arg);

  if (NULL == timer) {
    return 0xffffffff;
  }
  clock_gettime(CLOCK_MONOTONIC, &now_ts);
  now_tv.tv_sec = now_ts.tv_sec;
  now_tv.tv_usec = now_ts.tv_nsec/1000;

  timersub(&timer->tv, &now_tv, &sub_tv);
  return (sub_tv.tv_sec * 1000 + sub_tv.tv_usec / 1000);
}

